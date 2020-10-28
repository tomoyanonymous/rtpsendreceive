#include "rtpsender.hpp"

namespace rtpsr {
	RtpSender::RtpSender(std::unique_ptr<RtpSRSetting> s, Url& url, Codec codec, std::ostream& logger)
	: RtpSRBase(std::move(s), logger) {
		input       = std::make_unique<CustomCbAsyncInFormat>(*setting, setting->framesize * 2);
		auto option = std::make_unique<AVOptionBase>(makeCtxParams());
		output      = std::make_unique<RtpOutFormat>(url, *setting, std::move(option));
		this->codec = std::make_unique<Encoder>(*setting, codec);
		initStream();
		dtosbuffer.resize(setting->framesize * 2 * setting->channels);
		init_asyncloop.launch([&]() {
			while (init_asyncloop.isActive()) {
				auto* rtpout = dynamic_cast<RtpOutFormat*>(output.get());
				try {
					bool res = rtpout->tryConnect();
					if (res) {
						logger << "rtpsender: connected" << std::endl;
						break;
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(2000));
					logger << "rtpsender: connection to " << rtpout->url.address << ":" << std::to_string(rtpout->url.port)
						   << " refused,retry in 2000ms" << std::endl;
				}
				catch (std::runtime_error& e) {
					throw e;
					break;
				}
			}
			return true;    // return result;
		});
	}
	RtpSender::~RtpSender() {
		bool res1 = init_asyncloop.halt();
		bool res2 = asynclooper.halt();
		if (res1 && res2) {
			av_write_trailer(output->ctx);
			if (output->ctx->pb != nullptr) {
				avio_close(output->ctx->pb);
			}
			avformat_close_input(&input->ctx);
		}
	}
	AVOptionBase::container_t RtpSender::makeCtxParams() {
		return {
			{"protocol_whitelist", "file,udp,rtp,tcp,rtsp"},
			{"rtsp_transport", "udp"},
			{"enable-protocol", "rtp"},
			{"enable-protocol", "udp"},
			{"stimeout", "1000000"},
		};
	}
	bool RtpSender::writeToInput(std::vector<sample_t> const& input) {
		auto* asyncinput = dynamic_cast<CustomCbAsyncInFormat*>(this->input.get());
		return asyncinput->tryPushRingBuffer(input);
	}
	bool RtpSender::writeToInput(std::vector<double> const& input) {
		assert(dtosbuffer.size() == input.size());
		int count = 0;
		std::generate(dtosbuffer.begin(), dtosbuffer.end(), [&]() { return rtpsr::convertDoubleToSample(input.at(count++)); });
		return writeToInput(dtosbuffer);
	}
	bool RtpSender::fillFrame() {
		auto*  asyncinput       = dynamic_cast<CustomCbAsyncInFormat*>(input.get());
		size_t packet_framesize = frame->nb_samples * setting->channels;

		framebuf.resize(packet_framesize);
		bool res = asyncinput->tryPopRingBuffer(framebuf);
		if (res) {
			checkAvError(avcodec_fill_audio_frame(frame,
				setting->channels,
				AV_SAMPLE_FMT_S16,
				(uint8_t*)framebuf.data(),
				sizeof(sample_t) * packet_framesize,
				0));
			return true;
		}
		return false;
	}
	void RtpSender::sendData() {
		//   av_read_frame(input->ctx, packet);
		auto* encoder         = dynamic_cast<rtpsr::Encoder*>(codec.get());
		frame->pts            = timecount;
		frame->format         = AV_SAMPLE_FMT_S16;
		frame->channels       = setting->channels;
		frame->channel_layout = codec->ctx->channel_layout;
		bool hasinputframe    = fillFrame();
		if (!hasinputframe) {
			return;
		}
		timecount += setting->framesize;
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
			//   packet->duration = setting->framesize;
			auto itb = codec->ctx->time_base;
			auto otb = output->ctx->streams[0]->time_base;
			av_packet_rescale_ts(packet, otb, otb);    // maybe unnecessary
			checkAvError(av_write_frame(output->ctx, packet));

			av_packet_unref(packet);
		}
	}
	void RtpSender::launchLoop() {
		asynclooper.launch([&]() {
			try {
				while (true) {
					if (!asynclooper.isActive()) {
						return false;
					}
					auto res = init_asyncloop.wait_for(20);
					if (res == std::future_status::ready) {
						break;
					}
				}
				while (asynclooper.isActive()) {
					sendData();
					std::this_thread::sleep_for(pollingrate);
				}
			}
			catch (std::exception& e) {
				std::cerr << "Error happend in polling loop:" << e.what() << std::endl;
				return false;
			}
			return true;
		});
	}
}    // namespace rtpsr