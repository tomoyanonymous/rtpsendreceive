//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.

#pragma once

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"

}
#include <array>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <utility>

namespace rtpsr {
using sample_t = int16_t;

struct buffer_data {
  sample_t* ptr;
  size_t size;
};

using buffertype = std::vector<sample_t>;

using readfn_type = int (*)(void*, uint8_t*, int);
using seekfn_type = int64_t(*)(void*, int64_t, int);

}  // namespace rtpsr
