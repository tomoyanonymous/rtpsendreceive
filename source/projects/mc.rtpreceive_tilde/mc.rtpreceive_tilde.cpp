/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights
/// reserved.
///	@license	Use of this source code is governed by the MIT License
/// found in the License.md file.

#include "c74_min.h"
#include "rtpsr_classes.hpp"

using namespace c74::min;

class rtpreceive_tilde : public object<rtpreceive_tilde>, public mc_operator<> {
 private:
  bool m_initialized{false};

 public:
  MIN_DESCRIPTION{"receive audio stream via rtp protocol"};
  MIN_TAGS{"Audio"};
  MIN_AUTHOR{"Tomoya Matsuura"};
  MIN_RELATED{"rtpsend_tilde"};
  rtpreceive_tilde(const atoms& args = {}) {
    // resetReceiver();

    m_initialized = true;
  }

  attribute<symbol> address{this, "address", "127.0.0.1"};
  attribute<symbol> codec{
      this, "codec", "pcm_s16be",
      setter{MIN_FUNCTION{auto c = rtpsr::getCodecByName(args[0]);
  if (c == rtpsr::Codec::INVALID) {
    cerr << "Invalid Codec Name.  Using pcm_s16be" << endl;
    c = rtpsr::Codec::PCM_s16BE;
  }
  return args;
}
}
}
;
attribute<int> port{this, "port", 30000};
attribute<bool> play{this, "play", false};
inlet<> input{this, "(int) toggle subscription"};
outlet<> m_output{this, "(multichannelsignal) received output",
                  "multichannelsignal"};
attribute<int> channels{
    this, "channels", 8,
    setter{MIN_FUNCTION{if (m_initialized) resetChannel(args[0]);
return args;
}
}
}
;

// post to max window == but only when the class is loaded the first time
message<> maxclass_setup{this, "maxclass_setup",
                         MIN_FUNCTION{const auto ns = c74::max::gensym("box");
auto c = c74::max::class_findbyname(ns, c74::max::gensym("mc.rtpreceive~"));
c74::max::class_addmethod(c, (c74::max::method)setOutChans,
                          "multichanneloutputs", c74::max::A_CANT, 0);
return {};
}
}
;
message<> toggle{this, "int", "toggle play and pause",
                 MIN_FUNCTION{int num = args[0];
bool state = num > 0;
bool changed = state != play;
if (changed && state) {
  resetReceiver(frame_size);
}
if (changed && !state) {
  // rtpreceiver->pause();
}
play = state;
return {};
}
}
;
message<> setup{this, "setup", MIN_FUNCTION{double newvecsize = args[1];
resetReceiver(newvecsize);
return {};
}
}
;
message<> dspsetup{this, "dspsetup", MIN_FUNCTION{double newvecsize = args[1];
resetReceiver(newvecsize);
return {};
}
}
;

void operator()(audio_bundle input, audio_bundle output) {
  int chs = std::min<int>(output.channel_count(), channels.get());
  if (play && rtpreceiver != nullptr) {
    rtpreceiver->receiveData();
    for (auto i = 0; i < output.frame_count(); ++i) {
      for (auto channel = 0; channel < chs; ++channel) {
        output.samples(channel)[i] =
            rtpreceiver->getOutput().readBuffer<double>(i, channel);
      }
    }
  } else {
    output.clear();
  }
}

private:
std::shared_ptr<rtpsr::RtpReceiver> rtpreceiver{nullptr};
double frame_size = vector_size();
void resetChannel(int channel) {
  symbol c = codec;
  rtpsr::RtpSRSetting setting{(int)samplerate(), channel, (int)frame_size};
  rtpsr::Url url{address.get(), port};
}

void resetReceiver(double newvecsize) {
  frame_size = newvecsize;
  symbol c = codec;
  rtpsr::RtpSRSetting setting{(int)samplerate(), channels, (int)newvecsize};
  rtpsr::Url url{address.get(), port};
  resetReceiver(setting, url, rtpsr::getCodecByName(c));
}
void resetReceiver(rtpsr::RtpSRSetting& s, rtpsr::Url& url, rtpsr::Codec c) {
  rtpreceiver.reset();
  rtpsr::ConnectionCb cb{ this, [](void* cb_opaque) {
                           auto* obj = (rtpreceive_tilde*)(cb_opaque);
                           obj->cout << "RtpReceiver Connection to "
                                     << obj->address.get() << ":"
                                     << std::to_string(obj->port.get())
                                     << "finished" << c74::min::endl;
                         }};
  try {
    rtpreceiver = std::make_unique<rtpsr::RtpReceiver>(s, url, c, cb);
  } catch (std::exception& e) {
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
