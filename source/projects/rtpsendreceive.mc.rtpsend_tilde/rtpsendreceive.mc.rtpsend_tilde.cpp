/// @file
///	@ingroup 	rtpsendreceive
///	@copyright	Copyright 2020 Tomoya Matsuura. All rights reserved.
///	@license	Use of this source code is governed by the LGPL License found in the License.md file.

#include "c74_min.h"

using namespace c74::min;


class rtpsend_tilde : public object<rtpsend_tilde> , public mc_operator<>{
public:
    MIN_DESCRIPTION	{"send audio stream via rtp protocol"};
    MIN_TAGS		{"Audio"};
    MIN_AUTHOR		{"Tomoya Matsuura"};
    MIN_RELATED		{"rtpreceive_tilde"};

    inlet<>  m_inlet 			{ this, "(multichannelsignal) input to be transmited" };

    void operator()(audio_bundle input, audio_bundle output) {

        for (auto i = 0; i < input.frame_count(); ++i) {


            for (auto channel = 0; channel < input.channel_count(); ++channel) {
                auto in { input.samples(channel)[i] };

            }
        }
    }
    // define an optional argument for setting the message
    argument<symbol> greeting_arg { this, "greeting", "Initial value for the greeting attribute.",
        MIN_ARGUMENT_FUNCTION {
            greeting = arg;
        }
    };


    // the actual attribute for the message
    attribute<symbol> greeting { this, "greeting", "hello world",
        description {
            "Greeting to be posted. "
            "The greeting will be posted to the Max console when a bang is received."
        }
    };


    // respond to the bang message to do something
    message<> bang { this, "bang", "Post the greeting.",
        MIN_FUNCTION {
            symbol the_greeting = greeting;    // fetch the symbol itself from the attribute named greeting

            cout << the_greeting << endl;    // post to the max console
            return {};
        }
    };


    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        MIN_FUNCTION {
            cout << "hello world" << endl;
            return {};
        }
    };

};


MIN_EXTERNAL(rtpsend_tilde);
