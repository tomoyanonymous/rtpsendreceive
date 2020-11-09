#pragma once

#include <variant>
#include <unordered_map>
#include <regex>
#include <cassert>

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

	inline size_t getBufSize(RtpSRSetting const& s) {
		// how sample format?
		return sizeof(double) * s.framesize * s.channels;
	}
	struct Url {
		std::string address = "127.0.0.1";
		int         port    = 30000;
	};
	std::string getSdpUrl(std::string const& address, int port);
	std::string getSdpUrl(Url const& url);

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
		explicit RtpOptionsBase(Url const& url);
		virtual ~RtpOptionsBase() = default;
		const Url url;

		// number of packet of reorder queue
		size_t reorder_queue_size = 1000;
		size_t packet_size        = 1000;
		// maximum reorder delay(in microseconds) to be set to audioformatcontext
		int    max_delay = 500000;
		size_t buffer_size;

		virtual RtpFormatKind getKind() = 0;
		virtual void          generateOptions();
		AVDictionary**        getParam() {
            avoptions = std::make_unique<AVOptionBase>(std::move(dict));
            return avoptions->get();
		}
		AVOptionBase::container_t     dict;
		std::unique_ptr<AVOptionBase> avoptions;
	};
	class RtpOption : public RtpSRSetting, public RtpOptionsBase {
	public:
		explicit RtpOption(Url const& url, double samplerate, int channels, int buffersize);
		RtpFormatKind getKind() override {
			return RtpFormatKind::RTP;
		};
		void generateOptions() override;

		bool filter_source = false;
	};
	class RtspOption : public RtpSRSetting, public RtpOptionsBase {
	public:
		explicit RtspOption(Url const& url, double samplerate, int channels, int buffersize);

		RtpFormatKind getKind() override {
			return RtpFormatKind::RTSP;
		};
		void generateOptions() override;
		enum class TransPortProtocol { UDP = 0, TCP, UDP_MULTICAST, HTTP, HTTPS } rtsp_transport = TransPortProtocol::UDP;
		int                 listen_timeout                                                       = 1000;       // unit:seconds
		int                 socket_timeout                                                       = 500000;    // unit:microseconds
		std::pair<int, int> port_range                                                           = {5000, 65000};

	private:
		static std::string getProtocolString(TransPortProtocol p);
	};
	class RtpInOption : public RtpOption {
	public:
		explicit RtpInOption(Url const& url, double samplerate, int channels, int buffersize)
		: RtpOption(url, samplerate, channels, buffersize) { }
		std::string makeDummySdp();
		static int  readDummySdp(void* userdata, uint8_t* avio_buf, int buf_size);
		bool        use_customio   = false;
		bool        rtcp_to_source = false;
	};
	class RtpOutOption : public RtpOption {
	public:
		explicit RtpOutOption(Url const& url, double samplerate, int channels, int buffersize)
		: RtpOption(url, samplerate, channels, buffersize) { }
	};

	class RtspOutOption : public RtspOption {
	public:
		explicit RtspOutOption(Url const& url, double samplerate, int channels, int buffersize)
		: RtspOption(url, samplerate, channels, buffersize) { }
	};
	class RtspInOption : public RtspOption {
	public:
		explicit RtspInOption(Url const& url, double samplerate, int channels, int buffersize)
		: RtspOption(url, samplerate, channels, buffersize) { }
		void generateOptions() override;
	};

	// IO Format
	struct IOFormat {
		IOFormat();
		virtual ~IOFormat();
		AVFormatContext* ctx;
		RtpSRSetting     setting;
	};
	struct InFormat : public IOFormat {
		InFormat();
		~InFormat() override = default;
		virtual void connectInput() {};
	};
	struct OutFormat : public IOFormat {
		OutFormat();
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
		explicit CustomCbAsyncInFormat(RtpSRSetting const& s, size_t buffer_size);
		~CustomCbAsyncInFormat() final = default;
	};
	class CustomCbAsyncOutFormat final : public OutFormat, public CustomCbAsyncFormat {
	public:
		explicit CustomCbAsyncOutFormat(RtpSRSetting const& s, size_t buffer_size);
		~CustomCbAsyncOutFormat() final = default;
	};

	// old classes for synchronous reading
	struct CustomCbFormat {
		rtpsr::buffertype buffer;
	};
	struct CustomCbInFormat : public InFormat, public CustomCbFormat {
		explicit CustomCbInFormat(RtpSRSetting const& s);
		~CustomCbInFormat() = default;
		static int readPacket(void* userdata, uint8_t* avio_buf, int buf_size);

		void setBuffer(double sample, int pos, int channel) {
			buffer[pos * setting.channels + channel] = static_cast<rtpsr::sample_t>(sample * INT16_MAX);
		}
	};
	struct CustomCbOutFormat : public OutFormat, public CustomCbFormat {
		explicit CustomCbOutFormat(RtpSRSetting const& s);
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
		RtpFormatBase(std::unique_ptr<RtpOptionsBase> opt);
		std::unique_ptr<RtpOptionsBase> option;
	};
	struct RtpInFormatBase : public InFormat, public RtpFormatBase {
		RtpInFormatBase(std::unique_ptr<RtpOptionsBase> opt)
		: RtpFormatBase(std::move(opt)) {

		};
		virtual bool tryConnectInput() = 0;
	};

	struct RtspInFormat : public RtpInFormatBase {
		RtspInFormat() = delete;
		explicit RtspInFormat(std::unique_ptr<RtspInOption> rtpoptions);
		bool tryConnectInput() override;
	};

	struct RtpInFormat : public RtpInFormatBase {
		RtpInFormat() = delete;
		explicit RtpInFormat(std::unique_ptr<RtpInOption> rtpoptions);
		~RtpInFormat() override;
		bool                           tryConnectInput() override;
		std::unique_ptr<SdpOpaque>     sdp_opaque;
		unsigned char*                 sdp_avio;
		inline static constexpr size_t aviobufsize = 4096;
	};

	struct RtpOutFormatBase : public OutFormat, public RtpFormatBase {
		RtpOutFormatBase(std::unique_ptr<RtpOptionsBase> opt)
		: RtpFormatBase(std::move(opt)) {};
		virtual bool tryConnect() = 0;
	};
	struct RtspOutFormat : public RtpOutFormatBase {
		RtspOutFormat() = delete;
		explicit RtspOutFormat(std::unique_ptr<RtspOutOption> rtpoptions);
		bool tryConnect() override;
	};

	struct RtpOutFormat : public RtpOutFormatBase {
		RtpOutFormat() = delete;
		explicit RtpOutFormat(std::unique_ptr<RtpOutOption> rtpoptions);
		bool tryConnect() override;
	};

	struct CodecBase {
		explicit CodecBase(RtpSRSetting const& s, Codec c = Codec::PCM_s16BE, bool isencoder = true);
		virtual ~CodecBase() = default;
		AVCodecContext* ctx  = nullptr;
		rtpsr::Codec    codec;
		int64_t         bitrate = 192000;    // bitrate for lossy compression
		static bool     checkIsErrAgain(int error_code);
	};
	struct Decoder : public CodecBase {
		explicit Decoder(RtpSRSetting const& s, Codec c);
		~Decoder() override {
			avcodec_free_context(&ctx);
		}
		bool sendPacket(AVPacket* packet);
		bool receiveFrame(AVFrame* frame);
	};
	struct Encoder : public CodecBase {
		explicit Encoder(RtpSRSetting const& s, Codec c);
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
		bool               forceHalt();
		void               wait();
		std::future_status wait_for(int mills);
		bool               isActive() const;

	private:
		std::atomic<bool>        active = false;
		std::shared_future<bool> future;
	};

	struct RtpSRBase {
		explicit RtpSRBase(RtpSRSetting const& s, std::ostream& logger = std::cerr);
		~RtpSRBase();
		virtual void launchLoop() = 0;
		AsyncLooper  asynclooper;
		AsyncLooper  init_asyncloop;

	protected:
		void                       initStream() const;
		const RtpSRSetting&        setting_ref;
		std::unique_ptr<InFormat>  input;
		std::unique_ptr<OutFormat> output;
		std::unique_ptr<CodecBase> codec;
		AVPacket*                  packet;
		AVFrame*                   frame;
		std::ostream&              logger;
	};

}    // namespace rtpsr