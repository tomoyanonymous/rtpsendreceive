/// @file
///	@ingroup 	rtpsendreceive
///	@copyright	Copyright 2020 Tomoya Matsuura. All rights reserved.
///	@license	Use of this source code is governed by the LGPL License found in the License.md file.

#include "c74_min_unittest.h"     // required unit test header

#include "mc.rtpsend_tilde.cpp"    // need the source of our object so that we can access it

// Unit tests are written using the Catch framework as described at
// https://github.com/philsquared/Catch/blob/master/docs/tutorial.md

SCENARIO("object produces correct output") {
    ext_main(nullptr);    // every unit test must call ext_main() once to configure the class

    GIVEN("An instance of our object") {

        test_wrapper<rtpsend_tilde> an_instance;
        rtpsend_tilde& my_object = an_instance;
        
        // check that default attr values are correct
        // REQUIRE((my_object.greeting == symbol("hello world")));

        // now proceed to testing various sequences of events
        // WHEN("a 'bang' is received") {
        //     my_object.bang();
        //     THEN("our greeting is produced at the outlet") {
        //         auto& output = *c74::max::object_getoutput(my_object, 0);
        //         REQUIRE((output.size() == 1));
        //         REQUIRE((output[0].size() == 1));
        //         REQUIRE((output[0][0] == symbol("hello world")));
        //     }
        // }
    }
}
