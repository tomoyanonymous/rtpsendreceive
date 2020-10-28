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
		instance_count += 1;
		m_initialized = true;
	}
	~rtpsend_tilde() {
		instance_count -= 1;
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

attribute<int>    port {this, "port", 30000};
attribute<double> retry_rate {this, "retry_rate", 500.0, description {"Retry intervals in milliseconds at connection"}};
attribute<double> reorder_queue_size {this, "reorder_queue_size", 500.0, description {"number of packets for reorder queue"}};
attribute<bool>   use_rtsp {this,
    "use_rtsp",
    true,
    description {"if set to false, use raw rtp protocol instead of using rtsp mux/demuxer(Some options are ignored)."}};

attribute<int, threadsafe::no, limit::clamp> min_port {
	this, "min_port", 5000, range {0, 1000000}, description {"minimum port number used for rtsp internal transport"}};
attribute<int, threadsafe::no, limit::clamp> max_port {
	this, "max_port", 65000, range {0, 1000000}, description {"minimum port number used for rtsp internal transport"}};


attribute<int> active {this, "active", 0, setter {MIN_FUNCTION {int res = args[0];
if (m_initialized && rtpsender != nullptr) {
	if (res <= 0) {
		rtpsender->asynclooper.halt();
	}
	else {
		rtpsender->launchLoop();
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
if (m_initialized) {
	resetSender(newvecsize);
}
return {};
}
}
;

void operator()(audio_bundle input, audio_bundle output) {

	if (rtpsender != nullptr) {
		int chs = std::min<int>(input.channel_count(), channels.get());
		for (auto i = 0; i < input.frame_count(); i++) {
			for (auto channel = 0; channel < chs; channel++) {
				double  in {input.samples(channel)[i]};
				int16_t s                 = rtpsr::convertDoubleToSample(in);
				iarray[i * chs + channel] = s;
			}
		}
		bool write_success = rtpsender->writeToInput(iarray);
		if (!write_success) {
			// cerr << "ring buffer is full!" <<std::endl;
		}
	}
}
static long setDspState(void* obj, long state) {
	c74::max::object_attr_setlong(obj, c74::max::gensym("active"), state);
	return state;
}

private:
void resetSender(double newvecsize) {
	iarray.resize((int)newvecsize * channels);
	symbol c = codec;
	try {

		if (use_rtsp) {
			auto option = std::make_unique<rtpsr::RtspOutOption>(rtpsr::Url {address.get(), port}, samplerate(), channels, (int)newvecsize);
			option->port_range = getPortRange();
			setOptions(*static_cast<rtpsr::RtpOptionsBase*>(option.get()));

			rtpsender = std::make_unique<rtpsr::RtpSender>(std::move(option),
				rtpsr::getCodecByName(c),
				std::chrono::milliseconds((long long)retry_rate.get()),
				cout);
		}
		else {
			auto option = std::make_unique<rtpsr::RtpOutOption>(rtpsr::Url {address.get(), port}, samplerate(), channels, (int)newvecsize);
			setOptions(*static_cast<rtpsr::RtpOptionsBase*>(option.get()));
			rtpsender = std::make_unique<rtpsr::RtpSender>(std::move(option),
				rtpsr::getCodecByName(c),
				std::chrono::milliseconds((long long)retry_rate.get()),
				cout);
		}
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}
void setOptions(rtpsr::RtpOptionsBase& opt) {
	opt.reorder_queue_size = reorder_queue_size;
}
std::pair<int, int> getPortRange() {
	if (min_port > max_port) {
		min_port = max_port.get();
	}
	return std::pair(min_port.get(),max_port.get());
}

std::unique_ptr<rtpsr::RtpSender> rtpsender;
std::vector<int16_t>              iarray;
static int                        instance_count;
}
;
int rtpsend_tilde::instance_count = 0;
MIN_EXTERNAL(rtpsend_tilde);
