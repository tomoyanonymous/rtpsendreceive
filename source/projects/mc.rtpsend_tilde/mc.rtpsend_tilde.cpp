/// @file
///	@ingroup 	rtpsendreceive
///	@copyright	Copyright 2020 Tomoya Matsuura. All rights reserved.
///	@license	Use of this source code is governed by the LGPL License
/// found in the License.md file.

#include <rtpsender.hpp>

#include "c74_max.h"
#include "c74_min.h"

using namespace c74::min;

class rtpsend_tilde : public object<rtpsend_tilde>, public mc_operator<> {
	bool m_initialized {false};

public:
	MIN_DESCRIPTION {"send audio stream via rtp protocol"};
	MIN_TAGS {"Audio"};
	MIN_AUTHOR {"Tomoya Matsuura"};
	MIN_RELATED {"rtpreceive_tilde"};
	rtpsend_tilde(const atoms& args = {}) {
		m_initialized = true;
	}
	attribute<int, threadsafe::no, limit::clamp> channels {this, "channels", 1, range {1, 16}};
	attribute<symbol>                            address {this, "address", "127.0.0.1"};
	attribute<symbol> codec {this, "codec", "pcm_s16be", setter {MIN_FUNCTION {auto c = rtpsr::getCodecByName(args[0]);
	if (c == rtpsr::Codec::INVALID) {
		cerr << "Invalid Codec Name.  Using pcm_s16be" << endl;
		c = rtpsr::Codec::PCM_s16BE;
	}
	return args;
}
}
}
;

attribute<int> port {this, "port", 30000};
attribute<int> active {this, "active", 0, setter {MIN_FUNCTION {int res = args[0];
if (m_initialized && rtpsender != nullptr) {
	if (res <= 0) {
		rtpsender->loopstate.active = false;
	}
	else {
		rtpsender->loopstate.active = true;
    rtpsender->wait_connection.wait();
		// rtpsender->launchLoop();
	}
}
return {};
}
}
}
;

inlet<> m_inlet {this, "(multichannelsignal) input to be transmited"};

// static long inputChanged(rtpsend_tilde* s, long index, long count) {
//   if (count != s->channels) {
//     s->channels = count;
//     s->resetSender();
//     return true;
//   } else
//     return false;
// }
message<> maxclass_setup {this, "maxclass_setup", MIN_FUNCTION {c74::max::t_class* c = args[0];
c74::max::class_addmethod(c, (c74::max::method)setDspState, "dspstate", c74::max::A_CANT, 0);
return {};
}
}
;
message<> setup {this, "setup", MIN_FUNCTION {double newvecsize = args[1];
return {};
}
}
;

message<> dspsetup {this, "dspsetup", MIN_FUNCTION {double newvecsize = args[1];
resetSender(newvecsize);
return {};
}
}
;

void operator()(audio_bundle input, audio_bundle output) {

	int chs = std::min<int>(input.channel_count(), channels.get());
	for (auto i = 0; i < input.frame_count(); i++) {
		for (auto channel = 0; channel < chs; channel++) {
			double  in {input.samples(channel)[i]};
			int16_t s                 = rtpsr::convertDoubleToSample(in);
			iarray[i * chs + channel] = s;
		}
	}
	bool write_success = rtpsender->input_buf.writeRange(iarray, iarray.size());
	if (!write_success) {
		// cerr << "ring buffer is full!" <<std::endl;
	}
  rtpsender->sendData();
}
static long setDspState(void* obj, long state) {
	c74::max::object_attr_setlong(obj, c74::max::gensym("active"), state);
	return state;
}

private:
std::unique_ptr<rtpsr::RtpSender> rtpsender;
std::vector<int16_t>              iarray;
void                              resetSender(double newvecsize) {
    rtpsender.reset();
    iarray.resize((int)newvecsize * channels);
    symbol              c = codec;
    rtpsr::RtpSRSetting setting {(int)samplerate(), channels, (int)newvecsize};
    rtpsr::Url          url {address.get(), port};

    try {
        rtpsender = std::make_unique<rtpsr::RtpSender>(setting, url, rtpsr::getCodecByName(c), cout);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
}
;

MIN_EXTERNAL(rtpsend_tilde);
