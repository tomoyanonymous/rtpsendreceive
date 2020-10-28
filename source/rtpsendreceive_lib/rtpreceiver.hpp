#pragma once
#include <optional>

#include "rtpsrbase.hpp"
#include "lockfree_ringbuffer.hpp"

namespace rtpsr {
	struct RtpReceiver : public RtpSRBase {
		explicit RtpReceiver(RtpSRSetting& s, Url& url, Codec codec, std::ostream& logger = std::cerr);
		~RtpReceiver();

		std::future<bool>&        launchloop();
		bool                      pushToOutput();
		void                      setCtxParams(AVDictionary** dict);
		bool readFromOutput(std::vector<sample_t> & dest);
		bool readFromOutput(std::vector<double> & dest);
		private:
		std::vector<int16_t>      dtosbuffer;
		std::vector<int16_t>      tmpbuf;
		bool                      receiveData();

	};
}    // namespace rtpsr
