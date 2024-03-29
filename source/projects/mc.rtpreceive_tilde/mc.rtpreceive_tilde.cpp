/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights
/// reserved.
///	@license	Use of this source code is governed by the MIT License
/// found in the License.md file.

#include "c74_min.h"
#include "min_debugutils.hpp"
#include "rtpreceiver.hpp"

using namespace c74::min;

class rtpreceive_tilde : public object<rtpreceive_tilde>, public mc_operator<> {
private:
	bool m_initialized {false};

public:

	MIN_DESCRIPTION {"Receive audio stream via rtp protocol."};
	MIN_TAGS {"Audio"};
	MIN_AUTHOR {"Tomoya Matsuura"};
	MIN_RELATED {"mc.rtpsend_tilde"};
	rtpreceive_tilde(const atoms& args = {}) {
		// resetReceiver();

		m_initialized = true;
	}

	attribute<symbol> address {this, "address", "127.0.0.1", description {"Your IP address for an appropriate network interface."}};
	attribute<int>    port {this, "port", 30000, description {"A main incoming network port number."}};
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
attribute<int> active {this, "active", 0, setter {MIN_FUNCTION {int res = args[0];
if (m_initialized && rtpreceiver != nullptr) { }
return {};
}
}
}
;
attribute<double> reorder_queue_size {this, "reorder_queue_size", 500.0, description {"number of packets for reorder queue"}};
attribute<double> ringbuf_framenum {
	this, "ringbuf_framenum", 4, range {1, 1000}, description {"Size of an internal ring buffer (multiplied with signal vector size.)"}};
attribute<int> max_delay {this, "max_delay", 500000, description {"maximum accepted delay for reordering(in microseconds)"}};
attribute<int, threadsafe::no, limit::clamp> min_port {
	this, "min_port", 5000, range {0, 1000000}, description {"minimum port number used for rtsp internal transport"}};
attribute<int, threadsafe::no, limit::clamp> max_port {
	this, "max_port", 65000, range {0, 1000000}, description {"minimum port number used for rtsp internal transport"}};

attribute<bool> use_rtsp {this,
	"use_rtsp",
	true,
	description {"if set to false, use raw rtp protocol instead of using rtsp mux/demuxer(Some options are ignored)."}};

// attribute<bool>                              play {this, "play", true};
attribute<int, threadsafe::no, limit::clamp> channels {this, "channels", 1, range {1, 16}};
inlet<>                                      input {this, "(int) toggle subscription"};
outlet<>                                     m_output {this, "(multichannelsignal) received output", "multichannelsignal"};
outlet<>                                     latency {this, "(double) current latency"};

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
resetReceiver(newvecsize, channels);
return {};
}
}
;
message<> getlatency {this, "getlatency", MIN_FUNCTION {if (rtpreceiver != nullptr) {latency.send(rtpreceiver->getLatency());
}
return {};
}
, description {
	"Outputs a packet latency in milliseconds at second outlet."
}
}
;

static long setDspState(void* obj, long state) {
	c74::max::object_attr_setlong(obj, c74::max::gensym("active"), state);
	return state;
}
void operator()(audio_bundle input, audio_bundle output) {
	output.clear();
	if (rtpreceiver == nullptr) {
		return;
	}
	if (!rtpreceiver->isConnected()) {
		return;
	}
	int  chs     = std::min<int>(output.channel_count(), channels.get());
	bool readres = rtpreceiver->readFromOutput(iarray);
	misscount.second++;
	if (!readres) {
		misscount.first++;
		cout << "stream underflow detected! " << std::to_string(misscount.first) << "/" << std::to_string(misscount.second) << std::endl;
		return;
	}
	for (auto i = 0; i < output.frame_count(); i++) {
		for (auto channel = 0; channel < chs; channel++) {
			output.samples(channel)[i] = rtpsr::convertSampleToDouble(iarray.at(i * channels.get() + channel));
		}
	}
}

private:
std::shared_ptr<rtpsr::RtpReceiver> rtpreceiver {nullptr};
std::vector<rtpsr::sample_t>        iarray;
std::pair<int, int>                 misscount;
double                              frame_size = vector_size();

void resetChannel(int channel) {
	resetReceiver(vector_size(), channels);
}

void resetReceiver(double newvecsize, int channel) {
	iarray.resize(newvecsize * channel);
	frame_size = newvecsize;
	misscount  = {0, 0};
	rtpsr::Url url {address.get(), port};
	if (use_rtsp) {
		auto option = std::make_unique<rtpsr::RtspInOption>(url, samplerate(), channel, (int)newvecsize);
		setOptions(*static_cast<rtpsr::RtpOptionsBase*>(option.get()));
		resetReceiver(std::move(option));
	}
	else {
		auto option = std::make_unique<rtpsr::RtpInOption>(url, samplerate(), channel, (int)newvecsize);
		setOptions(*static_cast<rtpsr::RtpOptionsBase*>(option.get()));
		resetReceiver(std::move(option));
	}
}
void resetReceiver(std::unique_ptr<rtpsr::RtspInOption> s) {
	rtpreceiver.reset();
	try {
		s->port_range = getPortRange();
		rtpreceiver   = std::make_unique<rtpsr::RtpReceiver>(std::move(s), rtpsr::getCodecByName(codec.get()), ringbuf_framenum, cout);
		rtpreceiver->launchLoop();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}
void resetReceiver(std::unique_ptr<rtpsr::RtpInOption> s) {
	rtpreceiver.reset();
	try {
		rtpreceiver = std::make_unique<rtpsr::RtpReceiver>(std::move(s), rtpsr::getCodecByName(codec.get()), ringbuf_framenum, cout);
		rtpreceiver->launchLoop();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

void setOptions(rtpsr::RtpOptionsBase& opt) {
	opt.reorder_queue_size = reorder_queue_size;
	opt.max_delay          = max_delay;
}
std::pair<int, int> getPortRange() {
	if (min_port > max_port) {
		min_port = max_port.get();
	}
	return std::pair(min_port.get(), max_port.get());
}
static long setOutChans(void* obj, long outletindex) {
	auto chs = c74::max::object_attr_getlong(obj, c74::max::gensym("channels"));
	return chs;
}
}
;

MIN_EXTERNAL(rtpreceive_tilde);
