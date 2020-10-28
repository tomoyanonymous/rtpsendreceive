#include "rtpreceiver.hpp"

namespace rtpsr {
	RtpReceiver::RtpReceiver(std::unique_ptr<RtpSRSetting> s, Url const& url, Codec codec, std::ostream& logger)
	: RtpSRBase(*s, logger) {
		auto option = std::make_unique<RtspInOption>(url, setting_ref.samplerate, setting_ref.channels, setting_ref.framesize);
		input       = std::make_unique<RtspInFormat>(std::move(option));
		output      = std::make_unique<CustomCbAsyncOutFormat>(setting_ref, frame->nb_samples * 2);
		this->codec = std::make_unique<Decoder>(setting_ref, codec);
		init();
	}
	RtpReceiver::RtpReceiver(std::unique_ptr<RtpInOption> s, Codec codec, std::ostream& logger)
	: RtpSRBase(*s, logger) {
		input       = std::make_unique<RtpInFormat>(std::move(s));
		output      = std::make_unique<CustomCbAsyncOutFormat>(setting_ref, frame->nb_samples * 2);
		this->codec = std::make_unique<Decoder>(setting_ref, codec);
		init();
	}
	RtpReceiver::RtpReceiver(std::unique_ptr<RtspInOption> s, Codec codec, std::ostream& logger)
	: RtpSRBase(*s, logger) {
		input       = std::make_unique<RtspInFormat>(std::move(s));
		output      = std::make_unique<CustomCbAsyncOutFormat>(setting_ref, frame->nb_samples * 2);
		this->codec = std::make_unique<Decoder>(setting_ref, codec);
		init();
	}
	RtpReceiver::~RtpReceiver() {
		std::cerr << "rtpreceiver destructor called" << std::endl;
		bool res1 = init_asyncloop.halt();
		bool res2 = asynclooper.halt();
		if (res1 && res2) {
			avformat_close_input(&input->ctx);
		}
	}

	void RtpReceiver::init() {
		tmpbuf.resize(frame->nb_samples * setting_ref.channels * 2);
		dtosbuffer.resize(frame->nb_samples * setting_ref.channels);
		input->ctx->max_delay = 1000000;
		init_asyncloop.launch([&]() {
			logger << "rtpreceiver waiting incoming connection..." << std::endl;
			while (init_asyncloop.isActive()) {
				// this start blocking...
				bool connection_res = dynamic_cast<RtpInFormatBase*>(input.get())->tryConnectInput();
				if (connection_res) {
					initStream();
					logger << "rtpreceiver connected" << std::endl;
					break;
				}
			}
			return true;
		});
	}
	// return value:stopped_index;
	bool RtpReceiver::pushToOutput() {
		auto* asyncoutput = dynamic_cast<CustomCbAsyncOutFormat*>(output.get());
		auto* frameref    = av_frame_get_plane_buffer(frame, 0);
		auto  size        = frame->nb_samples * setting_ref.channels;
		tmpbuf.resize(size);
		std::memcpy(tmpbuf.data(), frameref->data, size * sizeof(int16_t));
		bool res = asyncoutput->tryPushRingBuffer(tmpbuf);
		av_packet_unref(packet);
		return res;
	}
	bool RtpReceiver::readFromOutput(std::vector<sample_t>& dest) {
		auto* asyncoutput = dynamic_cast<CustomCbAsyncOutFormat*>(output.get());
		return asyncoutput->tryPopRingBuffer(dest);
	}
	bool RtpReceiver::readFromOutput(std::vector<double>& dest) {
		assert(dtosbuffer.size() == dest.size());
		int count = 0;
		std::generate(dtosbuffer.begin(), dtosbuffer.end(), [&]() { return rtpsr::convertDoubleToSample(dest.at(count++)); });
		return readFromOutput(dtosbuffer);
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
	void RtpReceiver::launchLoop() {
		asynclooper.launch([&]() {
			auto* decoder        = dynamic_cast<Decoder*>(codec.get());
			bool  isdecoderempty = false;
			bool  isdecoderfull  = false;
			bool  res            = true;
			bool  writeres       = true;
			if (input->ctx == nullptr) {
				return false;
			}
			try {
				init_asyncloop.wait();
				while (asynclooper.isActive()) {
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
				return false;
			}
			return true;
		});
	}

}    // namespace rtpsr