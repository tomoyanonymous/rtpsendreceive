#include "rtpsrbase.hpp"

namespace rtpsr {
	void checkAvError(int error_code) {
		if (error_code < 0) {
			char str[4096];
			av_make_error_string(str, 4096, error_code);
			throw std::runtime_error(str);
		}
	}
	AVOptionBase::AVOptionBase(container_t&& init) {
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
	AVOptionBase::~AVOptionBase() {
		if (allocated) {
			av_dict_free(&params);
		}
	}
	std::string getSdpUrl(std::string const& address, int port) {
		return "udp://" + address + ":" + std::to_string(port) + "/live.sdp";
	}
	std::string getSdpUrl(Url& url) {
		return getSdpUrl(url.address, url.port);
	}
	RtpOptionsBase::RtpOptionsBase(Url&& url)
	: url(url) {
		dict.emplace("protocol_whitelist", "file,udp,rtp,tcp,rtsp");
	}
	void RtpOptionsBase::generateOptions() {
		dict.clear();
		dict.emplace("reorder_queue_size", reorder_queue_size);
		dict.emplace("packet_size", packet_size);
		dict.emplace("buffer_size", buffer_size);
	}
	RtpOption::RtpOption(Url&& url, double samplerate, int channels, int buffersize)
	: RtpOptionsBase(std::move(url))
	, RtpSRSetting({samplerate, channels, buffersize}) { }
	void RtpOption::generateOptions() {
		RtpOptionsBase::generateOptions();
		// todo::filter_source;
	}

	RtspOption::RtspOption(Url&& url, double samplerate, int channels, int buffersize)
	: RtpOptionsBase(std::move(url))
	, RtpSRSetting({samplerate, channels, buffersize}) {
		// dict.emplace("allowed_media_types",nullptr);
		dict.emplace("protocol_whitelist", "file,udp,rtp,tcp,rtsp");
	}
	void RtspOption::generateOptions() {
		RtpOptionsBase::generateOptions();
		dict.emplace("rtsp_transport", getProtocolString(rtsp_transport));
		dict.emplace("min_port", port_range.first);
		dict.emplace("max_port", port_range.second);
		dict.emplace("listen_timeout", listen_timeout);
		// for compatibility of old ffmpeg
		dict.emplace("timeout", listen_timeout);
		dict.emplace("stimeout", socket_timeout);
	}
	void RtspInOption::generateOptions() {
		RtspOption::generateOptions();
		dict.emplace("rtsp_flags", "listen");
	}
	std::string RtspOption::getProtocolString(TransPortProtocol p) {
		switch (p) {
			using Protocol = RtspOption::TransPortProtocol;
			case Protocol::UDP:
				return "udp";
			case Protocol::TCP:
				return "tcp";
			case Protocol::UDP_MULTICAST:
				return "udp_multicast";
			case Protocol::HTTP:
				return "http";
			case Protocol::HTTPS:
				return "https";
		}
	}

	std::string RtpInOption::makeDummySdp() {
		std::string sdp_content =
			R"(SDP:
v=0
o=- 0 0 IN IP4 $address$
s=DUMMY SDP
c=IN IP4 $address$
t=0 0
a=tool:libavformat 58.29.100
m=audio $port$ RTP/AVP 97
b=AS:$bitrate$
a=rtpmap:97 L16/$samplerate$/$channels$)";
		std::vector<std::pair<std::string, std::string>> replace_pairs = {{"\\$port\\$", std::to_string(url.port)},
			{"\\$address\\$", url.address},
			{"\\$channels\\$", std::to_string(channels)},
			{"\\$bitrate\\$", std::to_string((samplerate / 1000) * 16 * channels)},
			{"\\$samplerate\\$", std::to_string(samplerate)}};
		std::for_each(replace_pairs.begin(), replace_pairs.end(), [&](auto pair) {
			sdp_content = std::regex_replace(sdp_content, std::regex(pair.first), pair.second);
		});
		return sdp_content;
	}

	// IO Format

	IOFormat::IOFormat(RtpSRSetting& setting)
	: setting(setting) {
		ctx = avformat_alloc_context();
	}
	IOFormat::~IOFormat() {
		avformat_free_context(ctx);
	}
	// InFormat
	InFormat::InFormat(RtpSRSetting& s)
	: IOFormat(s) { }
	// OutFormat
	OutFormat::OutFormat(RtpSRSetting& s)
	: IOFormat(s) { }

	// old cutomcallback format
	CustomCbInFormat::CustomCbInFormat(RtpSRSetting& s)
	: InFormat(s) {
		buffer.resize(setting.framesize * setting.channels);
		auto  bufsize = getBufSize(s);
		auto* aviobuf = static_cast<uint8_t*>(av_malloc(bufsize));
		auto* avioctx = avio_alloc_context(aviobuf, bufsize, 0, this, readPacket, nullptr, nullptr);
		ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
		ctx->pb = avioctx;
	}


	int CustomCbInFormat::readPacket(void* userdata, uint8_t* avio_buf, int buf_size) {
		auto* sender  = reinterpret_cast<CustomCbInFormat*>(userdata);
		auto* address = sender->buffer.data();
		getBufSize(sender->setting);
		memcpy(avio_buf, address, buf_size);
		return buf_size;
	}
	CustomCbOutFormat::CustomCbOutFormat(RtpSRSetting& s)
	: OutFormat(s) {
		buffer.resize(setting.framesize);

		auto  bufsize = getBufSize(s);
		auto* aviobuf = static_cast<uint8_t*>(av_malloc(bufsize));
		auto* avioctx = avio_alloc_context(aviobuf, bufsize, 1, this, nullptr, writePacket, nullptr);
		ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
		ctx->pb = avioctx;
	}
	int CustomCbOutFormat::writePacket(void* userdata, uint8_t* avio_buf, int buf_size) {
		auto* receiver = reinterpret_cast<CustomCbOutFormat*>(userdata);
		auto* address  = receiver->buffer.data();
		memcpy(address, avio_buf, buf_size);
		return buf_size;
	}

	// CustomCbAsyncFormat
	bool CustomCbAsyncFormat::tryPushRingBuffer(std::vector<sample_t> const& input) {
		auto size = input.size();
		if (buffer.getWriteMargin() < size) {
			return false;
		}
		buffer.writeRange(input, input.size());
		return true;
	}
	bool CustomCbAsyncFormat::tryPopRingBuffer(std::vector<sample_t>& dest) {
		auto size = dest.size();
		if (buffer.getReadMargin() < size) {
			return false;
		}
		buffer.readRange(dest, dest.size());
		return true;
	}
	CustomCbAsyncInFormat::CustomCbAsyncInFormat(RtpSRSetting& s, size_t buffer_size)
	: InFormat(s)
	, CustomCbAsyncFormat(std::max((int)buffer_size, s.framesize) * s.channels) { }
	CustomCbAsyncOutFormat::CustomCbAsyncOutFormat(RtpSRSetting& s, size_t buffer_size)
	: OutFormat(s)
	, CustomCbAsyncFormat(std::max((int)buffer_size, s.framesize) * s.channels) { }

	// Rtp Format base
	RtpFormatBase::RtpFormatBase(std::unique_ptr<AVOptionBase> options)
	: options(std::move(options)) { }

	// Rtp Input
	RtpInFormat::RtpInFormat(Url& url, RtpSRSetting& s, std::unique_ptr<AVOptionBase> options)
	: url(url)
	, RtpFormatBase(std::move(options))
	, InFormat(s) {
		avformat_network_init();
	}
	bool RtpInFormat::tryConnectInput() {
		auto* ifmt = av_find_input_format("rtsp");
		auto  res  = avformat_open_input(&ctx, getSdpUrl(url).c_str(), ifmt, options->get());
		if (res == -60 || res == -61 || res == -22) {
			return false;
		}
		if (res < 0) {
			checkAvError(res);
		}
		return true;
	}
	RtpOutFormat::RtpOutFormat(Url& url, RtpSRSetting& s, std::unique_ptr<AVOptionBase> options)
	: url(url)
	, RtpFormatBase(std::move(options))
	, OutFormat(s) {
		avformat_network_init();
		auto  url_tmp    = getSdpUrl(url);
		auto* fmt_output = av_guess_format("rtsp", url_tmp.c_str(), nullptr);
		ctx->oformat     = fmt_output;
		ctx->url         = (char*)av_malloc(url_tmp.size() + sizeof(char));
		av_strlcpy(ctx->url, url_tmp.c_str(), url_tmp.size() + sizeof(char));
	}
	bool RtpOutFormat::tryConnect() {
		checkAvError(avformat_init_output(ctx, this->options->get()));
		checkAvError(avio_open(&ctx->pb, ctx->url, AVIO_FLAG_WRITE));
		int rcode = avformat_write_header(ctx, nullptr);
		if (rcode >= 0) {
			return true;
		}
		if (rcode == -60 || rcode == -61 || rcode == -22) {
			return false;
		}
		checkAvError(rcode);
		return false;
	}


	CodecBase::CodecBase(RtpSRSetting& s, Codec c, bool isencoder)
	: codec(c) {
		auto* avcodec = (isencoder) ? avcodec_find_encoder_by_name(getCodecName(codec).c_str())
									: avcodec_find_decoder_by_name(getCodecName(codec).c_str());
		ctx                 = avcodec_alloc_context3(avcodec);
		ctx->bit_rate       = bitrate;
		ctx->sample_rate    = s.samplerate;
		ctx->frame_size     = s.framesize;
		ctx->channels       = s.channels;
		ctx->channel_layout = av_get_default_channel_layout(s.channels);
		ctx->sample_fmt     = AV_SAMPLE_FMT_S16;
		avcodec_open2(ctx, avcodec, nullptr);
	}
	bool CodecBase::checkIsErrAgain(int error_code) {
		if (error_code == AVERROR(EAGAIN)) {
			return true;    // return isempty
		}
		checkAvError(error_code);
		return false;
	}

	Decoder::Decoder(RtpSRSetting& s, Codec c)
	: CodecBase(s, c, false) { }
	Encoder::Encoder(RtpSRSetting& s, Codec c)
	: CodecBase(s, c, true) { }
	bool Decoder::sendPacket(AVPacket* packet) {
		return checkIsErrAgain(avcodec_send_packet(ctx, packet));
	}
	bool Decoder::receiveFrame(AVFrame* frame) {
		return checkIsErrAgain(avcodec_receive_frame(ctx, frame));
	}

	bool Encoder::sendFrame(AVFrame* frame) {
		return checkIsErrAgain(avcodec_send_frame(ctx, frame));
	}
	bool Encoder::receivePacket(AVPacket* packet) {
		return checkIsErrAgain(avcodec_receive_packet(ctx, packet));
	}

	bool AsyncLooper::halt() {
		if (active) {
			active = false;
			wait();
		}
		return true;
	}
	void AsyncLooper::wait() {
		future.wait();
		bool res = future.get();
	}

	bool AsyncLooper::isActive() const {
		return active;
	}
	std::future_status AsyncLooper::wait_for(int mills) {
		return future.wait_for(std::chrono::milliseconds(mills));
	}

	// RtpSRBase

	RtpSRBase::RtpSRBase(std::unique_ptr<RtpSRSetting> s, std::ostream& logger)
	: setting(std::move(s))
	, logger(logger) {
		frame             = av_frame_alloc();
		frame->nb_samples = setting->framesize;
		packet            = av_packet_alloc();
		av_new_packet(packet, getBufSize(*setting));
	}

	RtpSRBase::~RtpSRBase() {
		av_frame_free(&frame);
		av_packet_free(&packet);
	}
	void RtpSRBase::initStream() const {
		auto* instream  = avformat_new_stream(input->ctx, codec->ctx->codec);
		auto* outstream = avformat_new_stream(output->ctx, codec->ctx->codec);
		checkAvError(avcodec_parameters_from_context(instream->codecpar, codec->ctx));
		checkAvError(avcodec_parameters_from_context(outstream->codecpar, codec->ctx));
		instream->start_time  = 0;
		outstream->start_time = 0;
	}
}    // namespace rtpsr