/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights
/// reserved.
///	@license	Use of this source code is governed by the MIT License
/// found in the License.md file.

#include "c74_min.h"
#include "rtpreceiver.hpp"

using namespace c74::min;
namespace max = c74::max;

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

  // attribute<symbol> address{this, "address", "127.0.0.1"};
  attribute<int> port{this, "port", 30000};
  attribute<bool> play{this, "play", false};
  inlet<> input{this, "(int) toggle subscription"};
  outlet<> m_output{this, "(multichannelsignal) received output",
                    "multichannelsignal"};
  attribute<int> channels{
      this, "channels", 7,
      setter{MIN_FUNCTION{if (m_initialized){resetChannel(args[0]);

} return args;
}
}
}
;
// post to max window == but only when the class is loaded the first time
message<> maxclass_setup{this, "maxclass_setup",
                         MIN_FUNCTION{
    const auto ns = max::gensym("box");
    auto c = max::class_findbyname(ns, max::gensym("mc.rtpreceive~"));
    max::class_addmethod(c,(max::method)setOutChans,"multichanneloutputs",max::A_CANT, 0);
                                      return {};
}
}
;
message<> toggle{this, "int", "toggle play and pause",
                 MIN_FUNCTION{int num = args[0];
bool state = num > 0;
bool changed = state != play;
if (changed && state) {
  rtpreceiver->play();
}
if (changed && (!state)) {
  rtpreceiver->pause();
}
play = state;
return {};
}
}
;
message<> setup{this, "setup", MIN_FUNCTION{resetReceiver();
return {};
}
}
;
message<> dspsetup{this, "dspsetup", MIN_FUNCTION{resetReceiver();
return {};
}
}
;

void operator()(audio_bundle input, audio_bundle output) {
  int chs = std::min<int>(output.channel_count(), channels.get());
  if (play) {
    rtpreceiver->receiveData();
    for (auto i = 0; i < output.frame_count(); ++i) {
      for (auto channel = 0; channel < chs; ++channel) {
        output.samples(channel)[i] = rtpreceiver->readBuffer(i, channel);
      }
    }
  } else {
    output.clear();
  }
}

private:
std::shared_ptr<RtpReceiver> rtpreceiver{nullptr};

void resetChannel(int channel) {
  rtpreceiver = std::make_shared<RtpReceiver>(vector_size(), samplerate(),
                                              channel, "127.0.0.1", port.get());
  rtpreceiver->init();
}

void resetReceiver() {
  rtpreceiver = std::make_shared<RtpReceiver>(
      vector_size(), samplerate(), channels.get(), "127.0.0.1", port);
  rtpreceiver->init();
}
static long setOutChans(void* obj, long outletindex) {
    auto chs = max::object_attr_getlong(obj, max::gensym("channels"));
  return chs;
}
}
;

MIN_EXTERNAL(rtpreceive_tilde);
