#pragma once

#include "rtpsendreceive_lib.hpp"
#include "lockfree_ringbuffer.hpp"


namespace rtpsr {

	struct AsyncLoopState {
		std::atomic<bool> active = false;
		std::future<bool> future;
	};

	inline void checkAvError(int error_code) {
		if (error_code < 0) {
			char str[4096];
			av_make_error_string(str, 4096, error_code);
			throw std::runtime_error(str);
		}
	}

	struct RtpSRSetting {
		int samplerate;
		int channels;
		int framesize;
	};

	inline size_t getBufSize(RtpSRSetting& s) {
		// how sample format?
		return sizeof(double) * s.framesize * s.channels;
	}

	struct IOFormat {
		explicit IOFormat(RtpSRSetting& setting)
		: setting(setting) {
			ctx = avformat_alloc_context();
		}
		~IOFormat() {
			avformat_free_context(ctx);
		}
		AVFormatContext* ctx;
		RtpSRSetting&    setting;
	};

	struct CustomCbFormat {
		rtpsr::buffertype buffer;
	};
	class CustomCbAsyncFormat {
	public:
		explicit CustomCbAsyncFormat(int buffer_size)
		: buffer(buffer_size) { }
		virtual void pushRingBuffer(std::vector<sample_t> const& input);
		virtual void popRingBuffer(std::vector<sample_t>& dest);

	private:
		LockFreeRingbuf<sample_t> buffer;
	};

	struct InFormat : public IOFormat {
		explicit InFormat(RtpSRSetting& s)
		: IOFormat(s) {};
		virtual void startInput() {};
	};

	struct CustomCbInFormat : public InFormat, public CustomCbFormat {
		explicit CustomCbInFormat(RtpSRSetting& s);
		~CustomCbInFormat() = default;
		static int readPacket(void* userdata, uint8_t* avio_buf, int buf_size);

		void setBuffer(double sample, int pos, int channel) {
			buffer[pos * setting.channels + channel] = static_cast<rtpsr::sample_t>(sample * INT16_MAX);
		}
	};

	class CustomCbAsyncInFormat : public CustomCbAsyncFormat {
	public:
		explicit CustomCbAsyncInFormat(size_t buffer_size);
		~CustomCbAsyncInFormat() = default;
		void pushRingBuffer(std::vector<sample_t> const& input) final {
			CustomCbAsyncFormat::pushRingBuffer(input);
		}

	protected:
		void popRingBuffer(std::vector<sample_t>& dest) final {
			CustomCbAsyncFormat::popRingBuffer(dest);
		}
	};

	class CustomCbAsyncOutFormat : public CustomCbAsyncFormat {
	public:
		explicit CustomCbAsyncOutFormat(size_t buffer_size);
		~CustomCbAsyncOutFormat() = default;
		void popRingBuffer(std::vector<sample_t>& dest) final {
			CustomCbAsyncFormat::popRingBuffer(dest);
		}

	protected:
		void pushRingBuffer(std::vector<sample_t> const& input) final {
			CustomCbAsyncFormat::pushRingBuffer(input);
		}
	};


	struct SdpOpaque {
		using Vector = std::vector<uint8_t>;
		Vector           data;
		Vector::iterator pos;
	};

	struct Url {
		std::string address = "127.0.0.1";
		int         port    = 30000;
	};
	inline std::string getSdpUrl(std::string const& address, int port) {
		return "udp://" + address + ":" + std::to_string(port) + "/live.sdp";
	}

	inline std::string getSdpUrl(Url& url) {
		return getSdpUrl(url.address, url.port);
	}
	struct RtpInFormat : public InFormat {
		RtpInFormat(Url& url, RtpSRSetting& s)
		: url(url)
		, InFormat(s) {
			avformat_network_init();
		}
		~RtpInFormat() { }
		Url url;
	};

	struct OutFormat : public IOFormat {
		explicit OutFormat(RtpSRSetting& s)
		: IOFormat(s) {};
		virtual void startOutput() {};
	};

	struct CustomCbOutFormat : public OutFormat, public CustomCbFormat {
		explicit CustomCbOutFormat(RtpSRSetting& s);
		~CustomCbOutFormat() { }
		static int writePacket(void* userdata, uint8_t* avio_buf, int buf_size);
		template<typename T>
		T readBuffer(int pos, int channel) {
			return static_cast<T>(buffer.at(pos * setting.channels + channel));
		};
		template<>
		rtpsr::sample_t readBuffer(int pos, int channel) {
			return buffer.at(pos * setting.channels + channel);
		};
		template<>
		double readBuffer(int pos, int channel) {
			return static_cast<double>(buffer.at(pos * setting.channels + channel)) / INT16_MAX;
		}
	};
	struct RtpOutFormat : public OutFormat {
		explicit RtpOutFormat(Url& url, RtpSRSetting& s);
		Url url;
	};

	struct CodecBase {
		explicit CodecBase(RtpSRSetting& s, Codec c = Codec::PCM_s16BE, bool isencoder = true);
		virtual ~CodecBase() = default;
		AVCodecContext* ctx  = nullptr;
		rtpsr::Codec    codec;
		int64_t         bitrate = 192000;    // bitrate for lossy compression
		static bool     checkIsErrAgain(int error_code);
	};
	struct Decoder : public CodecBase {
		explicit Decoder(RtpSRSetting& s, Codec c);
		~Decoder() override {
			avcodec_free_context(&ctx);
		}
		bool sendPacket(AVPacket* packet);
		bool receiveFrame(AVFrame* frame);
	};
	struct Encoder : public CodecBase {
		explicit Encoder(RtpSRSetting& s, Codec c);
		~Encoder() override {
			avcodec_free_context(&ctx);
		}
		bool sendFrame(AVFrame* frame);
		bool receivePacket(AVPacket* packet);
	};


	class AsyncLooper {
	public:
		AsyncLooper()      = default;
		using callbacktype = void (*)(AsyncLoopState const&, std::future<int>&);
		AsyncLoopState& launch(callbacktype fn) {
			loopstate.active = true;
			loopstate.future = std::async(std::launch::async, [&]() {
				fn(loopstate, wait_connection);
				return true;
			});
			return loopstate;
		}

	private:
		std::future<int> wait_connection;
		AsyncLoopState   loopstate;
	};

	struct RtpSRBase {
		explicit RtpSRBase(RtpSRSetting& s, std::ostream& logger = std::cerr);
		~RtpSRBase();

	protected:
		void                       initStream() const;
		RtpSRSetting               setting;
		std::unique_ptr<InFormat>  input;
		std::unique_ptr<OutFormat> output;
		std::unique_ptr<CodecBase> codec;
		AsyncLooper                asynclooper;
		AVPacket*                  packet;
		AVFrame*                   frame;
		std::ostream&              logger;
	};

	struct ConnectionCb {
		void*                      opaque = nullptr;
		std::function<void(void*)> cb     = [](void* v) {};
	};
	inline void triggerCallback(ConnectionCb& cb) {
		cb.cb(cb.opaque);
	}

}    // namespace rtpsr