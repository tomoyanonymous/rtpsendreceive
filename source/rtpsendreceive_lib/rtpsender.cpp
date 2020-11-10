#include "rtpsender.hpp"

namespace rtpsr {
	RtpSender::RtpSender(
		std::unique_ptr<RtpSRSetting> s, Url const& url, Codec codec, std::chrono::milliseconds init_retry_rate, std::ostream& logger)
	: RtpSRBase(*s, logger)
	, init_retry_rate(init_retry_rate)
	, pollingrate(duration_type(static_cast<double>(s->framesize) * 0.5 * 48000 / s->samplerate)) {
		input       = std::make_unique<CustomCbAsyncInFormat>(setting_ref, setting_ref.framesize * 10.2);
		auto option = std::make_unique<RtspOutOption>(url, setting_ref.samplerate, setting_ref.channels, setting_ref.framesize);
		output      = std::make_unique<RtspOutFormat>(std::move(option));
		this->codec = std::make_unique<Encoder>(*s, codec);
		init();
	}
	RtpSender::RtpSender(std::unique_ptr<RtpOutOption> option, Codec codec, std::chrono::milliseconds init_retry_rate, std::ostream& logger)
	: RtpSRBase(*static_cast<RtpSRSetting*>(option.get()), logger)
	, init_retry_rate(init_retry_rate) {
		pollingrate = duration_type(static_cast<double>(setting_ref.framesize) * 0.5 * 48000 / setting_ref.samplerate);
		input       = std::make_unique<CustomCbAsyncInFormat>(setting_ref, setting_ref.framesize * 10.2);
		output      = std::make_unique<RtpOutFormat>(std::move(option));
		this->codec = std::make_unique<Encoder>(setting_ref, codec);
		init();
	}

	RtpSender::RtpSender(
		std::unique_ptr<RtspOutOption> option, Codec codec, std::chrono::milliseconds init_retry_rate, std::ostream& logger)
	: RtpSRBase(*static_cast<RtpSRSetting*>(option.get()), logger)
	, init_retry_rate(init_retry_rate) {
		pollingrate = duration_type(static_cast<double>(setting_ref.framesize) * 0.5 * 48000 / setting_ref.samplerate);
		input       = std::make_unique<CustomCbAsyncInFormat>(setting_ref, setting_ref.framesize * 10.2);
		output      = std::make_unique<RtspOutFormat>(std::move(option));
		this->codec = std::make_unique<Encoder>(setting_ref, codec);
		init();
	}


	RtpSender::~RtpSender() {
		bool res1 = init_asyncloop.halt();
		bool res2 = asynclooper.halt();
		if (res1 && res2) {
			checkAvError(av_write_trailer(output->ctx));

			if (output->ctx->pb != nullptr) {
				avio_close(output->ctx->pb);
			}
			avformat_close_input(&input->ctx);
		}
	}
	void RtpSender::init() {
		initStream();
		dtosbuffer.resize(setting_ref.framesize * 2 * setting_ref.channels);
		init_asyncloop.launch([&]() {
			while (init_asyncloop.isActive()) {
				auto* rtpout = dynamic_cast<RtpOutFormatBase*>(output.get());
				try {
					bool res = rtpout->tryConnect();
					if (res) {
						logger << "rtpsender: connected" << std::endl;
						break;
					}
					logger << "rtpsender: connection to " << rtpout->option->url.address << ":" << std::to_string(rtpout->option->url.port)
						   << " refused,retry in " << std::to_string(init_retry_rate.count()) << "ms." << std::endl;
					std::this_thread::sleep_for(init_retry_rate);
				}
				catch (std::runtime_error& e) {
					throw e;
					break;
				}
			}
			isconnected = true;
			return true;    // return result;
		});
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
		size_t packet_framesize = frame->nb_samples * setting_ref.channels;

		framebuf.resize(packet_framesize);
		bool res = asyncinput->tryPopRingBuffer(framebuf);
		if (res) {
			checkAvError(avcodec_fill_audio_frame(frame,
				setting_ref.channels,
				AV_SAMPLE_FMT_S16,
				(uint8_t*)framebuf.data(),
				sizeof(sample_t) * packet_framesize,
				0));
			return true;
		}
		return false;
	}
	void RtpSender::sendData() {
		packet->pos          = -1;
		packet->stream_index = 0;
		//   packet->duration = setting_ref.framesize;
		auto itb = codec->ctx->time_base;
		auto otb = output->ctx->streams[0]->time_base;
		av_packet_rescale_ts(packet, otb, otb);    // maybe unnecessary
		checkAvError(av_write_frame(output->ctx, packet));
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
				auto* encoder         = dynamic_cast<rtpsr::Encoder*>(codec.get());
				frame->format         = AV_SAMPLE_FMT_S16;
				frame->channels       = setting_ref.channels;
				frame->channel_layout = codec->ctx->channel_layout;
				frame->nb_samples     = setting_ref.framesize;
				while (asynclooper.isActive()) {
					bool hasinputframe = fillFrame();
					if (!hasinputframe) { }
					else {
						frame->pts = timecount;
						timecount += setting_ref.framesize;
						bool isfull = encoder->sendFrame(frame);
						if (isfull) {
							logger << "sender encoder is full" << std::endl;
						}
						while (true) {
							av_init_packet(packet);
							bool isempty = encoder->receivePacket(packet);
							if (isempty) {
								av_packet_unref(packet);
								break;    // frame is fully flushed, finish sending
							}
							sendData();
							av_packet_unref(packet);
						}
					}


					std::this_thread::sleep_for(duration_type(64));
				}
			}
			catch (std::exception& e) {
				std::cerr << "Error happend in polling loop:" << e.what() << std::endl;
				return false;
			}
			return true;
		});
	}
	bool RtpSender::isConnected() {
		return isconnected.load();
	}

}    // namespace rtpsr