#pragma once

#include <variant>
#include <unordered_map>

#include "rtpsendreceive_lib.hpp"
#include "lockfree_ringbuffer.hpp"


namespace rtpsr {


	inline void checkAvError(int error_code) {
		if (error_code < 0) {
			char str[4096];
			av_make_error_string(str, 4096, error_code);
			throw std::runtime_error(str);
		}
	}

	struct AVOptionBase {
		using keytype     = std::string;
		using valtype     = std::variant<std::monostate, std::string, int>;
		using container_t = std::unordered_map<keytype, valtype>;
		AVOptionBase()    = delete;
		explicit AVOptionBase(container_t&& init) {
			for (auto& [key, val] : init) {
				assert(!std::holds_alternative<std::monostate>(val));
				if (std::holds_alternative<int>(val)) {
					checkAvError(av_dict_set_int(&this->params, key.c_str(), std::get<int>(val), 0));
					allocated |= true;
				}
				if (std::holds_alternative<std::string>(val)) {
					checkAvError(av_dict_set(&this->params, key.c_str(), std::get<std::string>(val).c_str(), 0));
					allocated |= true;
				}
			}
			container = std::move(init);
		}
		~AVOptionBase() {
			if (allocated) {
				av_dict_free(&params);
			}
		}
		AVDictionary** get() {
			return &params;
		}

	private:
		AVDictionary* params    = nullptr;
		container_t   container = {};
		bool          allocated = false;
	};

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
		virtual ~IOFormat() {
			avformat_free_context(ctx);
		}
		AVFormatContext* ctx;
		RtpSRSetting&    setting;

	protected:
	};

	struct CustomCbFormat {
		rtpsr::buffertype buffer;
	};
	class CustomCbAsyncFormat {
	public:
		explicit CustomCbAsyncFormat(size_t buffer_size)
		: buffer(buffer_size) { }
		virtual bool tryPushRingBuffer(std::vector<sample_t> const& input);
		virtual bool tryPopRingBuffer(std::vector<sample_t>& dest);

	private:
		LockFreeRingbuf<sample_t> buffer;
	};

	struct InFormat : public IOFormat {
		explicit InFormat(RtpSRSetting& s)
		: IOFormat(s) {};
		~InFormat() override = default;
		virtual void connectInput() {};
	};

	struct CustomCbInFormat : public InFormat, public CustomCbFormat {
		explicit CustomCbInFormat(RtpSRSetting& s);
		~CustomCbInFormat() = default;
		static int readPacket(void* userdata, uint8_t* avio_buf, int buf_size);

		void setBuffer(double sample, int pos, int channel) {
			buffer[pos * setting.channels + channel] = static_cast<rtpsr::sample_t>(sample * INT16_MAX);
		}
	};

	class CustomCbAsyncInFormat final : public InFormat, public CustomCbAsyncFormat {
	public:
		explicit CustomCbAsyncInFormat(RtpSRSetting& s, size_t buffer_size);
		~CustomCbAsyncInFormat() final = default;
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
	struct RtpInFormat final : public InFormat {
		RtpInFormat(Url& url, RtpSRSetting& s, std::unique_ptr<AVOptionBase> options)
		: url(url)
		, InFormat(s)
		, options(std::move(options)) {
			avformat_network_init();
		}
		~RtpInFormat() final = default;
		bool                          tryConnectInput();
		Url                           url;
		std::unique_ptr<AVOptionBase> options;
	};

	struct OutFormat : public IOFormat {
		explicit OutFormat(RtpSRSetting& s)
		: IOFormat(s) {};
		virtual ~OutFormat() = default;
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


	class CustomCbAsyncOutFormat final : public OutFormat, public CustomCbAsyncFormat {
	public:
		explicit CustomCbAsyncOutFormat(RtpSRSetting& s, size_t buffer_size);
		~CustomCbAsyncOutFormat() final = default;
	};

	struct RtpOutFormat : public OutFormat {
		explicit RtpOutFormat(Url& url, RtpSRSetting& s, std::unique_ptr<AVOptionBase> options);
		Url  url;
		bool tryConnect();

		std::unique_ptr<AVOptionBase> options;
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
	struct AsyncLoopState {
		std::atomic<bool> active = false;
		std::future<bool> future;
	};

	class AsyncLooper {
	public:
		AsyncLooper()      = default;
		using callbacktype = void (*)(AsyncLooper const&);

		template<class F>
		void launch(F&& fn) {
			active = true;
			future = std::async(std::launch::async, std::move(fn));
		}
		bool halt() {
			if (active) {
				active = false;
				wait();
			}
			return true;
		}
		void wait() {
			future.wait();
			bool res = future.get();
		}
		auto wait_for(int mills) {
			return future.wait_for(std::chrono::milliseconds(mills));
		}
		bool isActive() const {
			return active;
		}

	private:
		std::atomic<bool>        active = false;
		std::shared_future<bool> future;
	};

	struct RtpSRBase {
		explicit RtpSRBase(RtpSRSetting& s, std::ostream& logger = std::cerr);
		~RtpSRBase();
		virtual void launchLoop() = 0;
		AsyncLooper  asynclooper;
		AsyncLooper  init_asyncloop;

	protected:
		void                       initStream() const;
		RtpSRSetting               setting;
		std::unique_ptr<InFormat>  input;
		std::unique_ptr<OutFormat> output;
		std::unique_ptr<CodecBase> codec;
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