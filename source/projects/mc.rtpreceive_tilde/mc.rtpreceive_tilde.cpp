/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights
/// reserved.
///	@license	Use of this source code is governed by the MIT License
/// found in the License.md file.

#include "c74_min.h"
#include "rtpreceiver.hpp"

using namespace c74::min;

class rtpreceive_tilde : public object<rtpreceive_tilde>, public mc_operator<> {
private:
	bool m_initialized {false};

public:
	MIN_DESCRIPTION {"receive audio stream via rtp protocol"};
	MIN_TAGS {"Audio"};
	MIN_AUTHOR {"Tomoya Matsuura"};
	MIN_RELATED {"rtpsend_tilde"};
	rtpreceive_tilde(const atoms& args = {}) {
		// resetReceiver();

		m_initialized = true;
	}

	attribute<symbol> address {this, "address", "127.0.0.1"};
	attribute<symbol> codec {this, "codec", "pcm_s16be", setter {MIN_FUNCTION {auto c = rtpsr::getCodecByName(args[0]);
	if (c == rtpsr::Codec::INVALID) {
		cerr << "Invalid Codec Name.  Using pcm_s16be" << endl;
		std::ostream& out = cerr;
		c                 = rtpsr::Codec::PCM_s16BE;
	}
	return args;
}
}
}
;
attribute<int> port {this, "port", 30000};
attribute<int> active {this, "active", 0, setter {MIN_FUNCTION {int res = args[0];
if (m_initialized && rtpreceiver != nullptr) {
	if (res <= 0) {
		rtpreceiver->loopstate.active = false;
    rtpreceiver->initloop=false;
	}
	else {
		rtpreceiver->loopstate.active = true;
		rtpreceiver->launchloop();
	}
}
return {};
}
}
}
;

// attribute<bool>                              play {this, "play", true};
attribute<int, threadsafe::no, limit::clamp> channels {this, "channels", 1, range {1, 16}};
inlet<>                                      input {this, "(int) toggle subscription"};
outlet<>                                     m_output {this, "(multichannelsignal) received output", "multichannelsignal"};

// post to max window == but only when the class is loaded the first time
// message<> maxclass_setup {this, "maxclass_setup"};
message<> toggle {this, "int", "toggle play and pause", MIN_FUNCTION {int num = args[0];
// bool state = num > 0;
// bool changed = state != play;
// if (changed && state) {
//   resetReceiver(frame_size);
// }
// if (changed && !state) {
//   rtpreceiver->pause();
// }
// play = state;
return {};
}
}
;
message<> setup {this, "maxclass_setup", MIN_FUNCTION {auto thisclass = args[0];
c74::max::class_addmethod(thisclass, (c74::max::method)setOutChans, "multichanneloutputs", c74::max::A_CANT, 0);
c74::max::class_addmethod(thisclass, (c74::max::method)setDspState, "dspstate", c74::max::A_CANT, 0);
return {};
}
}
;
message<> dspsetup {this, "dspsetup", MIN_FUNCTION {double newvecsize = args[1];
resetReceiver(newvecsize);
return {};
}
}
;
static long setDspState(void* obj, long state) {
	c74::max::object_attr_setlong(obj, c74::max::gensym("active"), state);
	return state;
}
void operator()(audio_bundle input, audio_bundle output) {
	int  chs     = std::min<int>(output.channel_count(), channels.get());
	bool readres = rtpreceiver->output_buf.readRange(iarray, iarray.size());
	if (!readres) {
		// cerr << "stream underflow detected!" << std::endl;
		output.clear();
		return;
	}
	for (auto i = 0; i < output.frame_count(); i++) {
		for (auto channel = 0; channel < chs; channel++) {
			rtpsr::sample_t s          = iarray[i * chs + channel];
			double          d          = rtpsr::convertSampleToDouble(s);
			output.samples(channel)[i] = d;
		}
	}
}

private:
std::shared_ptr<rtpsr::RtpReceiver> rtpreceiver {nullptr};
std::vector<rtpsr::sample_t>        iarray;
double                              frame_size = vector_size();
void                                resetChannel(int channel) {
    symbol              c = codec;
    rtpsr::RtpSRSetting setting {(int)samplerate(), channel, (int)frame_size};
    rtpsr::Url          url {address.get(), port};
    resetReceiver(setting, url, rtpsr::getCodecByName(c));
}

void resetReceiver(double newvecsize) {
	iarray.resize(newvecsize * channels);
	frame_size            = newvecsize;
	symbol              c = codec;
	rtpsr::RtpSRSetting setting {(int)samplerate(), channels, (int)newvecsize};
	rtpsr::Url          url {address.get(), port};
	resetReceiver(setting, url, rtpsr::getCodecByName(c));
}
void resetReceiver(rtpsr::RtpSRSetting& s, rtpsr::Url& url, rtpsr::Codec c) {
	rtpreceiver.reset();

	try {
		rtpreceiver = std::make_unique<rtpsr::RtpReceiver>(s, url, c, cout);
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}
static long setOutChans(void* obj, long outletindex) {
	auto chs = c74::max::object_attr_getlong(obj, c74::max::gensym("channels"));
	return chs;
}
}
;

MIN_EXTERNAL(rtpreceive_tilde);
