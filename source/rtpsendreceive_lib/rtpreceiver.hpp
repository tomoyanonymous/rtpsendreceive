#pragma once
#include <optional>

#include "rtpsrbase.hpp"
#include "lockfree_ringbuffer.hpp"

namespace rtpsr {
	struct RtpReceiver : public RtpSRBase {
		explicit RtpReceiver(RtpSRSetting& s, Url& url, Codec codec, std::ostream& logger = std::cerr);
		~RtpReceiver();
		Decoder          decoder;
		AVDictionary*    params = nullptr;
		AVInputFormat*   ifmt;
		std::future<int> wait_connection;
		AsyncLoopState   loopstate;
    std::atomic<bool> initloop=true;
		LockFreeRingbuf<sample_t> output_buf;
    std::vector<int16_t> tmpbuf;
		std::string url_tmp;
    duration_type polling_rate_cache;
		bool        receiveData();
		void        receiveLoop();
    AsyncLoopState& launchloop();
		bool        pushToOutput();
		void setCtxParams(AVDictionary** dict);
		auto&       getOutput() {
            return *std::dynamic_pointer_cast<CustomCbOutFormat>(output);
		}

		auto& getBuffer() {
			return getOutput().buffer;
		}
		auto* getBufPointer() {
			return reinterpret_cast<uint8_t*>(getBuffer().data());
		}
	};
}    // namespace rtpsr
