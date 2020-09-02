#pragma once
#include "rtpsrbase.hpp"

namespace rtpsr {
	struct RtpSender : public RtpSRBase {
		explicit RtpSender(RtpSRSetting& s, Url& url, Codec codec, std::ostream& logger = std::cerr);
		~RtpSender() {
			av_write_trailer(output->ctx);
			if (output->ctx->pb != nullptr) {
				avio_close(output->ctx->pb);
			}
		}

		Encoder          encoder;
		AVDictionary*    params    = nullptr;
		int64_t          timecount = 0;
		std::string      url_tmp;
		std::future<int> wait_connection;
		void             sendData();
		static void      setCtxParams(AVDictionary** dict);
		auto&            getInput() {
            return *std::dynamic_pointer_cast<CustomCbInFormat>(input);
		}
		auto& getBuffer() {
			return getInput().buffer;
		}
		auto* getBufPointer() {
			return reinterpret_cast<uint8_t*>(getBuffer().data());
		}
	};

}    // namespace rtpsr