/// @file
///	@ingroup 	minexamples
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights
/// reserved.
///	@license	Use of this source code is governed by the MIT License
/// found in the License.md file.
#include "c74_min_unittest.h"  // required unit test header

#include "mc.rtpreceive_tilde.cpp"  // need the source of our object so that we can access it

//Instantiation of mc.rtpreceive~ requires calling Max API directly, which is not available in min-unittest framework currently

// SCENARIO("object produces correct output") {
//   ext_main(nullptr);  // every unit test must call ext_main() once to configure
//                       // the class

//   GIVEN("An instance of our object") {
//     test_wrapper<rtpreceive_tilde> an_instance;
//     rtpreceive_tilde& my_object = an_instance;
//     // check that default attr values are correct
//     // REQUIRE((my_object.greeting == symbol("hello world")));

//     // now proceed to testing various sequences of events
//     WHEN("toggle 1 is received") {
//       my_object.toggle(1);

//       THEN("internal play flag is activated") {
//         c74::max::t_atom_long attr =
//             c74::max::object_attr_getlong(my_object.maxobj(), symbol("play"));
//         REQUIRE(attr == 1);
//       }
//     }
//   }
// }