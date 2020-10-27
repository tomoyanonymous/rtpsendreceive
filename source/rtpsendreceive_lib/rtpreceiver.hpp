#pragma once
#include <optional>

#include "rtpsrbase.hpp"
#include "lockfree_ringbuffer.hpp"

namespace rtpsr {
	struct RtpReceiver : public RtpSRBase {
		explicit RtpReceiver(RtpSRSetting& s, Url& url, Codec codec, std::ostream& logger = std::cerr);
		~RtpReceiver();
		AVDictionary*             params   = nullptr;
		std::atomic<bool>         initloop = true;
		LockFreeRingbuf<sample_t> output_buf;
		std::vector<int16_t>      tmpbuf;
		std::string               url_tmp;
		duration_type             polling_rate_cache;
		bool                      receiveData();
		std::future<bool>&        launchloop();
		bool                      pushToOutput();
		void                      setCtxParams(AVDictionary** dict);
		CustomCbOutFormat&        getOutput() {
            return *dynamic_cast<CustomCbOutFormat*>(output.get());
		}

		auto& getBuffer() {
			return getOutput().buffer;
		}
		auto* getBufPointer() {
			return reinterpret_cast<uint8_t*>(getBuffer().data());
		}
	};
}    // namespace rtpsr
