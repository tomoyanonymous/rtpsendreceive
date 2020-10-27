#include "rtpreceiver.hpp"

namespace rtpsr {
	RtpReceiver::RtpReceiver(RtpSRSetting& s, Url& url, Codec codec, std::ostream& logger)
	: RtpSRBase(s, logger)
	, output_buf(s.framesize) {
		input            = std::make_unique<RtpInFormat>(url, setting);
		output           = std::make_unique<CustomCbOutFormat>(setting);
		this->codec      = std::make_unique<Decoder>(s, codec);
		auto* ifmt       = av_find_input_format("rtsp");
		url_tmp          = getSdpUrl(url);
		setCtxParams(&params);
		tmpbuf.resize(frame->nb_samples * setting.channels * 2);
		output_buf.resize(frame->nb_samples * setting.channels * 2);
		polling_rate_cache    = std::chrono::seconds(0);
		input->ctx->max_delay = 1000000;
		auto& future          = init_asyncloop.launch([&](std::atomic<bool> const& loopstate) {
            int res = 1;
            while (loopstate) {
                logger << "rtpreceiver waiting incoming connection..." << std::endl;
                // this start blocking...
                res = avformat_open_input(&input->ctx, url_tmp.c_str(), ifmt, &params);
                if (res >= 0) {
                    initStream();
                    logger << "rtpreceiver connected" << std::endl;
                    break;
                }
            }
            return res;
        });
	}
	RtpReceiver::~RtpReceiver() {
		std::cerr << "rtpreceiver destructor called" << std::endl;
		bool res1 = init_asyncloop.halt();
		bool res2 = asynclooper.halt();
		if (res1 && res2) {
			avformat_close_input(&input->ctx);
		}
		av_dict_free(&params);
	}
	void RtpReceiver::setCtxParams(AVDictionary** dict) {
		checkAvError(av_dict_set(dict, "protocol_whitelist", "file,udp,rtp,tcp,rtsp", 0));
		checkAvError(av_dict_set(dict, "rtsp_transport", "udp", 0));
		checkAvError(av_dict_set(dict, "enable-protocol", "rtp", 0));
		checkAvError(av_dict_set(dict, "enable-protocol", "udp", 0));
		checkAvError(av_dict_set(dict, "enable-muxer", "rtsp", 0));
		checkAvError(av_dict_set(dict, "enable-demuxer", "rtsp", 0));
		checkAvError(av_dict_set_int(dict, "timeout", 20000, 0));
		checkAvError(av_dict_set(dict, "stimeout", "1000000", 0));                                          // tcp connection
		checkAvError(av_dict_set_int(dict, "reorder_queue_size", 100000, 0));                               // 0.05sec
		checkAvError(av_dict_set_int(dict, "buffer_size", setting.framesize * setting.channels * 4, 0));    // 0.05sec

		checkAvError(av_dict_set(dict, "rtsp_flags", "listen", 0));
		checkAvError(av_dict_set(dict, "allowed_media_types", "audio", 0));
	}


	bool RtpReceiver::receiveData() {
		av_init_packet(packet);
		int res = av_read_frame(input->ctx, packet);
		if (res == -60 || res == AVERROR_EOF) {
			return false;    // timeout or eof(ignore)
		}
		checkAvError(res);
		return true;
	}
	// return value:stopped_index;
	bool RtpReceiver::pushToOutput() {
		auto* frameref = av_frame_get_plane_buffer(frame, 0);
		auto  size     = frame->nb_samples * setting.channels;
		tmpbuf.resize(size);
		std::memcpy(tmpbuf.data(), frameref->data, size * sizeof(int16_t));
		bool res = output_buf.writeRange(tmpbuf, frame->nb_samples * setting.channels);
		av_packet_unref(packet);
		return res;
	}
	std::future<bool>& RtpReceiver::launchloop() {
		return asynclooper.launch([&](std::atomic<bool> const& loopstate) {
			auto* decoder        = dynamic_cast<Decoder*>(codec.get());
			bool  isdecoderempty = false;
			bool  isdecoderfull  = false;
			bool  res            = true;
			bool  writeres       = true;
			try {
				init_asyncloop.wait();
				while (loopstate) {
					// block until data comes;
					res = receiveData();
					if (!res) { }
					isdecoderfull  = decoder->sendPacket(packet);
					isdecoderempty = decoder->receiveFrame(frame);
					if (isdecoderempty) { }
					writeres = pushToOutput();
				}
			}
			catch (std::exception& e) {
				std::cerr << "Error happend in receive polling loop:" << e.what() << std::endl;
			}
		}

		);
	}

}    // namespace rtpsr