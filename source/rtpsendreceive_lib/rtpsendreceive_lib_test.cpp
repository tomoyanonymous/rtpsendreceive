//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.


#include "c74_min_unittest.h"     // required unit test header
#include "rtpsendrecerive_lib.hpp"

SCENARIO("instance is correctly created") {
    ext_main(nullptr);    // every unit test must call ext_main() once to configure the class

    GIVEN("An instance of our object") {

        test_wrapper<hello_world> an_instance;
        hello_world&              my_object = an_instance;

        // check that default attr values are correct
        REQUIRE((my_object.greeting == symbol("hello world")));

        // now proceed to testing various sequences of events
        WHEN("a 'bang' is received") {
            my_object.bang();
            THEN("our greeting is produced at the outlet") {
                auto& output = *c74::max::object_getoutput(my_object, 0);
                REQUIRE((output.size() == 1));
                REQUIRE((output[0].size() == 1));
                REQUIRE((output[0][0] == symbol("hello world")));
            }
        }
    }
}
