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
 public:
  MIN_DESCRIPTION{"receive audio stream via rtp protocol"};
  MIN_TAGS{"Audio"};
  MIN_AUTHOR{"Tomoya Matsuura"};
  MIN_RELATED{"rtpsend_tilde"};

  attribute<int, threadsafe::no, limit::clamp> channels{this, "channels", 1,
                                                        range{1, 16}};
  // attribute<symbol> address{this, "address", "127.0.0.1"};
  attribute<int> port{this, "port", 30000};
  attribute<bool> play{this, "play", false};
  //   inlet<> input{this, "(int) toggle subscription"};
  outlet<> m_output{this, "(multichannelsignal) receivedoutput"};

  // post to max window == but only when the class is loaded the first time
  message<> maxclass_setup{this, "maxclass_setup",
                           MIN_FUNCTION{//  c74::max::t_class* c = args[0];
                                        // c74::max::class_addmethod(c,
                                        // (c74::max::method)inputChanged,
                                        // "inputchanged", c74::max::A_CANT, 0);
                                        return {};
}
}
;
message<> toggle{this,"int","toggle play and pause",MIN_FUNCTION{
    int num = args[0];
    bool state = num>0;
    bool changed = state!=play;
    if(changed&&state){
        rtpreceiver->play();
    }
    if(changed&&(!state)){
        rtpreceiver->pause();
    }
    play = state;
    return {};
}};
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
  int chs = std::min<int>(input.channel_count(), channels.get());
  if (play) {
    rtpreceiver->receiveData();
    for (auto i = 0; i < input.frame_count(); ++i) {
      for (auto channel = 0; channel < chs; ++channel) {
        output.samples(channels)[i] = rtpreceiver->readBuffer(i, channel);
      }
    }
  } else {
      output.clear();
  }
}

private:
std::unique_ptr<RtpReceiver> rtpreceiver;
void resetReceiver() {
  rtpreceiver.reset();
  rtpreceiver = std::make_unique<RtpReceiver>(vector_size(), samplerate(),
                                              channels, "127.0.0.1", port);
  rtpreceiver->init();
}
}
;

MIN_EXTERNAL(rtpreceive_tilde);
