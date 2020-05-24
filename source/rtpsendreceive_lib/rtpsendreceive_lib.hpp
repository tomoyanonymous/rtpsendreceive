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
#include "libavutil/opt.h"
}
#include <unistd.h>

#include <array>
#include <iostream>
#include <utility>
#include <vector>

typedef struct PCMAudioDemuxerContext_C {
  AVClass* c;
  int sample_rate;
  int channels;
} PCMAudioDemuxerContext_C;

//defined in pcmdec.c
int pcm_read_header(AVFormatContext *s);
//custom pcm read function
int ff_pcm_read_packet_custom(AVFormatContext *s, AVPacket *pkt)
{
    AVCodecParameters *par = s->streams[0]->codecpar;
    int ret, size;

    if (par->block_align <= 0)
        return AVERROR(EINVAL);

    size = FFMIN(par->frame_size, 2048);

    ret = av_get_packet(s->pb, pkt, size);

    pkt->flags &= ~AV_PKT_FLAG_CORRUPT;
    pkt->stream_index = 0;

    return ret;
}

static const AVOption pcm_options_C[] = {
    {"sample_rate",
     "",
     offsetof(PCMAudioDemuxerContext_C, sample_rate),
     AV_OPT_TYPE_INT,
     {.i64 = 44100},
     0,
     INT_MAX,
     AV_OPT_FLAG_DECODING_PARAM},
    {"channels",
     "",
     offsetof(PCMAudioDemuxerContext_C, channels),
     AV_OPT_TYPE_INT,
     {.i64 = 1},
     0,
     INT_MAX,
     AV_OPT_FLAG_DECODING_PARAM},
    {NULL},
};

static const AVClass s16_custom_demuxer_class = {
    .class_name = "s16_custom_demuxer",
    .item_name = av_default_item_name,
    .option = pcm_options_C,
    .version = LIBAVUTIL_VERSION_INT,
};
AVInputFormat ff_pcm_s16_custom_demuxer = {
    .name = "s16_custom",
    .long_name = "PCM signed 16-bit big-endian(rtpsendreceive custom io)",
    .priv_data_size = sizeof(PCMAudioDemuxerContext_C),
    .read_header = pcm_read_header,
    .read_packet = ff_pcm_read_packet_custom,
    .read_seek = NULL,
    .flags = AVFMT_NOFILE,
    .extensions = NULL,
    .raw_codec_id = AV_CODEC_ID_PCM_S16BE,
    .priv_class = &s16_custom_demuxer_class};

namespace rtpsr {
using sample_t = int16_t;

struct buffer_data {
  sample_t* ptr;
  size_t size;
};

using buffertype = std::vector<sample_t>;

using readfn_type = int (*)(void*, uint8_t*, int);
using seekfn_type = int64_t (*)(void*, int64_t, int);

}  // namespace rtpsr
