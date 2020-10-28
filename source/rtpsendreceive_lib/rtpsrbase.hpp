#pragma once

#include <variant>
#include <unordered_map>
#include <regex>


#include "rtpsendreceive_lib.hpp"
#include "lockfree_ringbuffer.hpp"


namespace rtpsr {


	void checkAvError(int error_code);

	struct SdpOpaque {
		using Vector = std::vector<uint8_t>;
		Vector           data;
		Vector::iterator pos;
	};

	struct AVOptionBase {
		using keytype     = std::string;
		using valtype     = std::variant<std::monostate, std::string, int>;
		using container_t = std::unordered_map<keytype, valtype>;
		AVOptionBase()    = delete;
		explicit AVOptionBase(container_t&& init);
		~AVOptionBase();
		AVDictionary** get() {
			return &params;
		}

	private:
		AVDictionary* params    = nullptr;
		container_t   container = {};
		bool          allocated = false;
	};

	struct RtpSRSetting {
		double samplerate;
		int    channels;
		int    framesize;
	};

	inline size_t getBufSize(RtpSRSetting& s) {
		// how sample format?
		return sizeof(double) * s.framesize * s.channels;
	}
	struct Url {
		std::string address = "127.0.0.1";
		int         port    = 30000;
	};
	std::string getSdpUrl(std::string const& address, int port);
	std::string getSdpUrl(Url& url);

	enum class RtpFormatKind { RTP = 0, RTSP = 1 };

	inline std::string toString(RtpFormatKind k) {
		switch (k) {
			case RtpFormatKind::RTP:
				return "rtp";
			case RtpFormatKind::RTSP:
				return "rtsp";
			default:
				assert(false);
				return "";
		}
	}

	struct RtpOptionsBase {
		Url                           url;
		std::unique_ptr<AVOptionBase> avoptions;
		int                           max_delay;
		virtual RtpFormatKind         getKind() = 0;
	};
	class RtpOption : public RtpSRSetting, public RtpOptionsBase {
	public:
		RtpFormatKind getKind() override {
			return RtpFormatKind::RTP;
		};
	};
	class RtspOption : public RtpSRSetting, public RtpOptionsBase {
	public:
		RtpFormatKind getKind() override {
			return RtpFormatKind::RTSP;
		};
	};
	class RtpInOption : public RtpOption {
	public:
		std::string makeDummySdp();
	};
	class RtpOutOption : public RtpOption {
	public:
	};

	class RtspOutOption : public RtpSRSetting, public RtpOptionsBase {
	public:
	};
	class RtspInOption : public RtpSRSetting, public RtpOptionsBase {
	public:
	};

	// IO Format
	struct IOFormat {
		explicit IOFormat(RtpSRSetting& setting);
		virtual ~IOFormat();
		AVFormatContext* ctx;
		RtpSRSetting&    setting;
	};
	struct InFormat : public IOFormat {
		explicit InFormat(RtpSRSetting& s);
		~InFormat() override = default;
		virtual void connectInput() {};
	};
	struct OutFormat : public IOFormat {
		explicit OutFormat(RtpSRSetting& s);
		virtual ~OutFormat() = default;
		virtual void startOutput() {};
	};

	// Custom Callback Async Format
	// Uses LockFreeRingbuffer. This does not uses libavformat. Data are assumed to be taken from avframe directly.
	class CustomCbAsyncFormat {
	public:
		explicit CustomCbAsyncFormat(size_t buffer_size)
		: buffer(buffer_size) { }
		virtual bool tryPushRingBuffer(std::vector<sample_t> const& input);
		virtual bool tryPopRingBuffer(std::vector<sample_t>& dest);

	private:
		LockFreeRingbuf<sample_t> buffer;
	};

	class CustomCbAsyncInFormat final : public InFormat, public CustomCbAsyncFormat {
	public:
		explicit CustomCbAsyncInFormat(RtpSRSetting& s, size_t buffer_size);
		~CustomCbAsyncInFormat() final = default;
	};
	class CustomCbAsyncOutFormat final : public OutFormat, public CustomCbAsyncFormat {
	public:
		explicit CustomCbAsyncOutFormat(RtpSRSetting& s, size_t buffer_size);
		~CustomCbAsyncOutFormat() final = default;
	};

	// old classes for synchronous reading
	struct CustomCbFormat {
		rtpsr::buffertype buffer;
	};
	struct CustomCbInFormat : public InFormat, public CustomCbFormat {
		explicit CustomCbInFormat(RtpSRSetting& s);
		~CustomCbInFormat() = default;
		static int readPacket(void* userdata, uint8_t* avio_buf, int buf_size);

		void setBuffer(double sample, int pos, int channel) {
			buffer[pos * setting.channels + channel] = static_cast<rtpsr::sample_t>(sample * INT16_MAX);
		}
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

	// rtp in out format.
	struct RtpFormatBase {
		RtpFormatBase() = delete;
		explicit RtpFormatBase(std::unique_ptr<AVOptionBase> options);
		std::unique_ptr<AVOptionBase> options;
	};
	struct RtpInFormat final : public InFormat, public RtpFormatBase {
		RtpInFormat() = delete;
		explicit RtpInFormat(Url& url, RtpSRSetting& s, std::unique_ptr<AVOptionBase> options);
		~RtpInFormat() final = default;
		bool tryConnectInput();
		Url  url;
	};
	struct RtpOutFormat final : public OutFormat, public RtpFormatBase {
		explicit RtpOutFormat(Url& url, RtpSRSetting& s, std::unique_ptr<AVOptionBase> options);
		~RtpOutFormat() final = default;
		bool tryConnect();
		Url  url;
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
		bool               halt();
		void               wait();
		std::future_status wait_for(int mills);
		bool               isActive() const;

	private:
		std::atomic<bool>        active = false;
		std::shared_future<bool> future;
	};

	struct RtpSRBase {
		explicit RtpSRBase(std::unique_ptr<RtpSRSetting> s, std::ostream& logger = std::cerr);
		~RtpSRBase();
		virtual void launchLoop() = 0;
		AsyncLooper  asynclooper;
		AsyncLooper  init_asyncloop;

	protected:
		void                          initStream() const;
		std::unique_ptr<RtpSRSetting> setting;
		std::unique_ptr<InFormat>     input;
		std::unique_ptr<OutFormat>    output;
		std::unique_ptr<CodecBase>    codec;
		AVPacket*                     packet;
		AVFrame*                      frame;
		std::ostream&                 logger;
	};

}    // namespace rtpsr