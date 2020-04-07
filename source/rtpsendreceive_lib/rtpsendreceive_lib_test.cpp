//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.
#include "c74_min_unittest.h"     // required unit test header
#include "rtpsendreceive_lib.hpp"

SCENARIO("instance is correctly created") {
    RtpSender sender;
    GIVEN("An instance of our object") {

        // check that default attr values are correct
        // REQUIRE((my_object.greeting == symbol("hello world")));
        // now proceed to testing various sequences of events
        WHEN("a 'bang' is received") {
            // my_object.bang();
            sender.init();
            THEN("our greeting is produced at the outlet") {
                // REQUIRE((output.size() == 1));

            }
        }
    }
}
