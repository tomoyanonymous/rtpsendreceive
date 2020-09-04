#include "rtpreceiver.hpp"

namespace rtpsr {
	RtpReceiver::RtpReceiver(RtpSRSetting& s, Url& url, Codec codec, std::ostream& logger)
	: RtpSRBase(s, logger)
	, decoder(s, codec)
	, output_buf(s.framesize) {
		input  = std::static_pointer_cast<InFormat>(std::make_shared<RtpInFormat>(url, setting));
		output = std::static_pointer_cast<OutFormat>(std::make_shared<CustomCbOutFormat>(setting));

		ifmt             = av_find_input_format("rtsp");
		url_tmp          = getSdpUrl(url);
		loopstate.active = true;
		setCtxParams(&params);
		tmpbuf.resize(frame->nb_samples * setting.channels * 2);
		output_buf.resize(frame->nb_samples * setting.channels * 2);
		polling_rate_cache    = std::chrono::seconds(0);
		input->ctx->max_delay = 1000000;
		wait_connection       = std::future<int>();
		wait_connection       = std::async(std::launch::async, [&]() -> int {
            int res = 1;
            while (initloop) {
                res = avformat_open_input(&input->ctx, url_tmp.c_str(), ifmt,
                    &params);    // this start blocking...
                if (res >= 0) {
                    logger << "rtpreceiver waiting incoming connection..." << std::endl;
                    auto* instream  = avformat_new_stream(input->ctx, decoder.ctx->codec);
                    auto* outstream = avformat_new_stream(output->ctx, decoder.ctx->codec);
                    checkAvError(avcodec_parameters_from_context(instream->codecpar, decoder.ctx));
                    checkAvError(avcodec_parameters_from_context(outstream->codecpar, decoder.ctx));
                    instream->start_time  = 0;
                    outstream->start_time = 0;
                    logger << "rtpreceiver connected" << std::endl;
                    break;
                }
            }
            return res;
        });
	}
	RtpReceiver::~RtpReceiver() {
		std::cerr << "rtpreceiver destructor called" << std::endl;
		loopstate.active = false;
    initloop = false;
    if(wait_connection.valid()){
		auto state       = wait_connection.get();
		if (state >= 0) {
			avformat_close_input(&input->ctx);
		}
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
		checkAvError(av_dict_set_int(dict, "timeout", 1, 0));
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
	void RtpReceiver::receiveLoop() {
		bool isdecoderempty = false;
		bool isdecoderfull  = false;
		bool res            = true;
		bool writeres       = true;
		try {
			int status = wait_connection.get();
			if (status < 0) {
				checkAvError(status);
			}
			while (loopstate.active) {
				// block until data comes;
				res = receiveData();
				if (!res) { }
				isdecoderfull  = decoder.sendPacket(packet);
				isdecoderempty = decoder.receiveFrame(frame);
				if (isdecoderempty) { }
				writeres = pushToOutput();
			}
		}

		catch (std::exception& e) {
			std::cerr << "Error happend in receive polling loop:" << e.what() << std::endl;
		}
	}    // namespace rtpsr
	AsyncLoopState& RtpReceiver::launchloop() {
		loopstate.active = true;
		loopstate.future = std::async(std::launch::async, [&]() {
			receiveLoop();
			return true;
		});
		return loopstate;
	}

}    // namespace rtpsr