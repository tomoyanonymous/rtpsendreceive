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
 public:
  MIN_DESCRIPTION{"send audio stream via rtp protocol"};
  MIN_TAGS{"Audio"};
  MIN_AUTHOR{"Tomoya Matsuura"};
  MIN_RELATED{"rtpreceive_tilde"};

  attribute<int, threadsafe::no, limit::clamp> channels{this, "channels", 1,
                                                        range{1, 16}};
  attribute<symbol> address{this, "address", "127.0.0.1"};
  attribute<symbol> codec{this, "(symbol)codec (\"pcm_s16be, opus\")", "pcm_s16be",setter{
    MIN_FUNCTION{
      auto c = rtpsr::getCodecByName(args[0]);
      if(c==rtpsr::Codec::INVALID){
        cerr << "Invalid Codec Name.  Using pcm_s16be" << endl;
        c = rtpsr::Codec::PCM_s16BE;
      }
      return args;
    }
  }};

  attribute<int> port{this, "port", 30000};

  inlet<> m_inlet{this, "(multichannelsignal) input to be transmited"};

  // static long inputChanged(rtpsend_tilde* s, long index, long count) {
  //   if (count != s->channels) {
  //     s->channels = count;
  //     s->resetSender();
  //     return true;
  //   } else
  //     return false;
  // }
  message<> maxclass_setup{this, "maxclass_setup",
                           MIN_FUNCTION{//  c74::max::t_class* c = args[0];
                                        // c74::max::class_addmethod(c,
                                        // (c74::max::method)inputChanged,
                                        // "inputchanged", c74::max::A_CANT, 0);
                                        return {};
}
}
;
message<> setup{this, "setup", MIN_FUNCTION{double newvecsize = args[1];
resetSender(newvecsize);
return {};
}
}
;
message<> dspsetup{this, "dspsetup", MIN_FUNCTION{double newvecsize = args[1];
resetSender(newvecsize);
return {};
}
}
;

void operator()(audio_bundle input, audio_bundle output) {
  int chs = std::min<int>(input.channel_count(), channels.get());
  for (auto i = 0; i < input.frame_count(); ++i) {
    for (auto channel = 0; channel < chs; ++channel) {
      auto in{input.samples(channel)[i]};
      rtpsender->writeBuffer(in, i, channel);
    }
  }
  rtpsender->incrementTime(input.frame_count());
  rtpsender->sendData();
}

private:
std::unique_ptr<RtpSender> rtpsender;
void resetSender(double newvecsize, symbol codec = "pcm_s16be") {
  rtpsender.reset();
  rtpsender = std::make_unique<RtpSender>(newvecsize, samplerate(), channels,
                                          address.get(), port,
                                          rtpsr::getCodecByName(codec));
  rtpsender->init();
}};

MIN_EXTERNAL(rtpsend_tilde);
