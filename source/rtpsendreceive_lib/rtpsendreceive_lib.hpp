//  A simple ffmpeg wrapper to send/receive audio stream with rtp protocol
//  written in C++.
// Copyright 2020 Tomoya Matsuura All rights Reserved.
// This source code is released under LGPL lisence.

#pragma once

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/timestamp.h"

typedef struct PCMAudioDemuxerContext_C {
  AVClass *c;
  int sample_rate;
  int channels;
} PCMAudioDemuxerContext_C;

// copied from pcm_dec.c

static int pcm_read_header_custom(AVFormatContext *s) {
  auto *s1 = static_cast<PCMAudioDemuxerContext_C *>(s->priv_data);
  AVStream *st;
  uint8_t *mime_type = NULL;

  st = avformat_new_stream(s, NULL);
  if (!st) return AVERROR(ENOMEM);

  st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
  st->codecpar->codec_id = (AVCodecID)s->iformat->raw_codec_id;
  st->codecpar->sample_rate = s1->sample_rate;
  st->codecpar->channels = s1->channels;

  av_opt_get(s->pb, "mime_type", AV_OPT_SEARCH_CHILDREN, &mime_type);
  // if (mime_type && s->iformat->mime_type) {
  //     int rate = 0, channels = 0, little_endian = 0;
  //     size_t len = strlen(s->iformat->mime_type);
  //     if (!av_strncasecmp(s->iformat->mime_type, mime_type, len)) { /*
  //     audio/L16 */
  //         uint8_t *options = mime_type + len;
  //         len = strlen(mime_type);
  //         while (options < mime_type + len) {
  //             options = strstr(options, ";");
  //             if (!options++)
  //                 break;
  //             if (!rate)
  //                 sscanf(options, " rate=%d",     &rate);
  //             if (!channels)
  //                 sscanf(options, " channels=%d", &channels);
  //             if (!little_endian) {
  //                  char val[14]; /* sizeof("little-endian") == 14 */
  //                  if (sscanf(options, " endianness=%13s", val) == 1) {
  //                      little_endian = strcmp(val, "little-endian") == 0;
  //                  }
  //             }
  //         }
  //         if (rate <= 0) {
  //             av_log(s, AV_LOG_ERROR,
  //                    "Invalid sample_rate found in mime_type \"%s\"\n",
  //                    mime_type);
  //             av_freep(&mime_type);
  //             return AVERROR_INVALIDDATA;
  //         }
  //         st->codecpar->sample_rate = rate;
  //         if (channels > 0)
  //             st->codecpar->channels = channels;
  //         if (little_endian)
  //             st->codecpar->codec_id = AV_CODEC_ID_PCM_S16LE;
  //     }
  // }
  av_freep(&mime_type);

  st->codecpar->bits_per_coded_sample =
      av_get_bits_per_sample(st->codecpar->codec_id);

  av_assert0(st->codecpar->bits_per_coded_sample > 0);

  st->codecpar->block_align =
      st->codecpar->bits_per_coded_sample * st->codecpar->channels / 8;

  // avpriv_set_pts_info(st, 64, 1, st->codecpar->sample_rate);
  return 0;
}

// custom pcm read function
static int ff_pcm_read_packet_custom(AVFormatContext *s, AVPacket *pkt) {
  AVCodecParameters *par = s->streams[0]->codecpar;
  int ret, size;

  if (par->block_align <= 0) return AVERROR(EINVAL);

  size = FFMIN(par->frame_size * par->channels * sizeof(int16_t), 2048);

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
}

static const AVClass s16_custom_demuxer_class = {
    .class_name = "s16_custom_demuxer",
    .item_name = av_default_item_name,
    .option = pcm_options_C,
    .version = LIBAVUTIL_VERSION_INT,
};
static AVInputFormat ff_pcm_s16_custom_demuxer = {
    .name = "s16_custom",
    .long_name = "PCM signed 16-bit big-endian(rtpsendreceive custom io)",
    .flags = AVFMT_NOFILE,
    .extensions = NULL,
    .priv_class = &s16_custom_demuxer_class,
    .raw_codec_id = AV_CODEC_ID_PCM_S16BE,
    .priv_data_size = sizeof(PCMAudioDemuxerContext_C),
    .read_header = pcm_read_header_custom,
    .read_packet = ff_pcm_read_packet_custom,
    .read_seek = NULL};

#include <unistd.h>

#include <array>
#include <iostream>
#include <utility>
#include <vector>

namespace rtpsr {
using sample_t = int16_t;

struct buffer_data {
  sample_t *ptr;
  size_t size;
};

using buffertype = std::vector<sample_t>;

using readfn_type = int (*)(void *, uint8_t *, int);
using seekfn_type = int64_t (*)(void *, int64_t, int);

enum class Codec { PCM_s16BE, OPUS, INVALID = -1 };

inline std::string getCodecName(Codec c) {
  switch (c) {
    case Codec::PCM_s16BE:
      return "pcm_s16be";
      break;
    case Codec::OPUS:
      return "libopus";
      break;
    default:
      return "";
  }
}
inline size_t getCodecDataSize(Codec c) {
  if (c == Codec::PCM_s16BE) {
    return sizeof(int16_t);
  }
  return sizeof(double);
}

inline Codec getCodecByName(const std::string &name) {
  if (name == "pcm_s16be") {
    return Codec::PCM_s16BE;
  }
  if (name == "opus") {
    return Codec::OPUS;
  }
  return Codec::INVALID;
}
}  // namespace rtpsr