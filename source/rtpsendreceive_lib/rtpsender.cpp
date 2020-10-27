#include "rtpsender.hpp"

namespace rtpsr {
	RtpSender::RtpSender(RtpSRSetting& s, Url& url, Codec codec, std::ostream& logger)
	: RtpSRBase(s, logger)
	, input_buf(s.framesize) {
		input       = std::make_unique<CustomCbInFormat>(setting);
		output      = std::make_unique<RtpOutFormat>(url, setting);
		this->codec = std::make_unique<Encoder>(s, codec);
		initStream();
		setCtxParams(&params);
		input_buf.resize(frame->nb_samples * s.channels * 2);
		framebuf.resize(frame->nb_samples * s.channels * 2);
		url_tmp      = getSdpUrl(url);
		char* urlctx = (char*)av_malloc(url_tmp.size() + sizeof(char));
		av_strlcpy(urlctx, url_tmp.c_str(), url_tmp.size() + sizeof(char));
		output->ctx->url = urlctx;
		checkAvError(avformat_init_output(output->ctx, &params));
		auto& future = init_asyncloop.launch([&](std::atomic<bool> const& isactive) {
			checkAvError(avio_open(&output->ctx->pb, urlctx, AVIO_FLAG_WRITE));
			while (isactive) {
				int res = avformat_write_header(output->ctx, nullptr);
				if (res >= 0) {
					logger << "rtpsender: connected" << std::endl;
					break;
				}
				if (res == -60 || res == -61 || res == -22) {
					std::this_thread::sleep_for(std::chrono::milliseconds(2000));
					logger << "rtpsender: connection to " << url_tmp << " refused,retry in 2000ms" << std::endl;
				}
				else {
					checkAvError(res);
					break;
				}
			}
			// return result;
		});
	}

	void RtpSender::setCtxParams(AVDictionary** dict) {
		checkAvError(av_dict_set(dict, "protocol_whitelist", "file,udp,rtp,tcp,rtsp", 0));
		checkAvError(av_dict_set(dict, "rtsp_transport", "udp", 0));
		checkAvError(av_dict_set(dict, "enable-protocol", "rtp", 0));
		checkAvError(av_dict_set(dict, "enable-protocol", "udp", 0));
		checkAvError(av_dict_set(dict, "enable-muxer", "rtsp", 0));
		checkAvError(av_dict_set(dict, "enable-demuxer", "rtsp", 0));
		checkAvError(av_dict_set(dict, "stimeout", "1000000",
			0));    // wait up to 10 seconds until connect
	}
	bool RtpSender::fillFrame() {
		size_t packet_framesize = frame->nb_samples * setting.channels;
		if (input_buf.getReadMargin() < packet_framesize) {
			return false;
		}
		framebuf.resize(packet_framesize);
		input_buf.readRange(framebuf, packet_framesize);
		checkAvError(avcodec_fill_audio_frame(
			frame, setting.channels, AV_SAMPLE_FMT_S16, (uint8_t*)framebuf.data(), sizeof(sample_t) * packet_framesize, 0));
		return true;
	}
	void RtpSender::sendData() {
		//   av_read_frame(input->ctx, packet);
		auto* encoder         = dynamic_cast<rtpsr::Encoder*>(codec.get());
		frame->pts            = timecount;
		frame->format         = AV_SAMPLE_FMT_S16;
		frame->channels       = setting.channels;
		frame->channel_layout = codec->ctx->channel_layout;
		bool hasinputframe    = fillFrame();
		if (!hasinputframe) {
			return;
		}
		timecount += setting.framesize;
		bool isfull = encoder->sendFrame(frame);
		while (true) {
			av_init_packet(packet);
			bool isempty = encoder->receivePacket(packet);
			if (isempty) {
				av_packet_unref(packet);
				break;    // frame is fully flushed, finish sending
			}
			packet->pos          = -1;
			packet->stream_index = 0;
			//   packet->duration = setting.framesize;
			auto itb = codec->ctx->time_base;
			auto otb = output->ctx->streams[0]->time_base;
			av_packet_rescale_ts(packet, otb, otb);    // maybe unnecessary
			checkAvError(av_write_frame(output->ctx, packet));

			av_packet_unref(packet);
		}
	}
	std::future<bool>& RtpSender::launchLoop() {
		return asynclooper.launch([&](std::atomic<bool> const& isactive) {
			try {
				init_asyncloop.wait();
				while (isactive) {
					sendData();
					std::this_thread::sleep_for(pollingrate);
				}
			}
			catch (std::exception& e) {
				std::cerr << "Error happend in polling loop:" << e.what() << std::endl;
			}
		});
	}
}    // namespace rtpsr