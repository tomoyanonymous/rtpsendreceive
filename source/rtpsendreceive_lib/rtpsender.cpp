#include "rtpsender.hpp"

RtpSender::RtpSender(int framesize, int samplerate, int channels,
                     std::string address, int port,rtpsr::Codec codec,
                     rtpsr::readfn_type callback_read,
                     rtpsr::seekfn_type callback_seek, void* userdata)
    : rtp_ioctx(nullptr),
      address(std::move(address)),
      port(port),
      callback_read(callback_read),
      callback_seek(callback_seek),
      RtpSRBase(framesize, samplerate, channels,codec),
      avio_buffer(nullptr),
      timecount(0),
      iformat(std::make_unique<AVInputFormat>(ff_pcm_s16_custom_demuxer)) {
  if (userdata == nullptr) {
    userdata_address = reinterpret_cast<void*>(this);
  }
  // todo
  testbuf.resize(bufsize);
}
RtpSender::~RtpSender() {
  // todo
  av_packet_unref(packet);
  avio_close(rtp_ioctx);
  av_free(avio_buffer);
}

void RtpSender::initFormatCtx() {
  // for audio driver->ffmpeg
  avio_buffer = (uint8_t*)av_malloc(bufsize);
  avioctx = avio_alloc_context(avio_buffer, bufsize, 0, userdata_address,
                               callback_read, nullptr, callback_seek);
  avioctx->direct = 1;
  // Determine the input-format:
  AVProbeData probe_data{"", avio_buffer, 512, "audio/L16"};
  input_format_ctx = avformat_alloc_context();
  input_format_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
  input_format_ctx->pb = avioctx;
  // input_format_ctx->iformat = av_probe_input_format(&probe_data, 1);
  input_format_ctx->iformat = iformat.get();
  input_format_ctx->iformat->flags |= AVFMT_NOFILE;
  AVDictionary* options = NULL;
  av_dict_set(&options, "samplerate", std::to_string(samplerate).c_str(), 0);
  av_dict_set(&options, "", "rgb24", 0);

  // int inputres = avformat_open_input(&input_format_ctx, "", nullptr,
  // nullptr); if (inputres < 0) { dumpAvError(inputres);
  // }
  // output
  std::string destination = "rtp://" + address + ":" + std::to_string(port);
  fmt_output = av_guess_format("rtp", destination.c_str(), "audio/L16");
  fmt_output->mime_type = "audio/L16";
  fmt_output->audio_codec = AV_CODEC_ID_PCM_S16BE;
  fmt_output->data_codec = AV_CODEC_ID_PCM_S16BE;
  output_format_ctx = avformat_alloc_context();
  auto res =
      avio_open(&output_format_ctx->pb, destination.c_str(), AVIO_FLAG_WRITE);
  if (res < 0) {
    dumpAvError(res);
    // std::cerr << "avio open error\n";
  }

  output_format_ctx->oformat = fmt_output;
  char* url = new char[destination.size() + 1];
  std::char_traits<char>::copy(url, destination.c_str(),
                               destination.size() + 1);
  output_format_ctx->url = url;
  // delete[] url;
  av_new_packet(packet, bufsize);
}

void RtpSender::setDestination(std::string& ad, int po) {
  address = ad;
  port = po;
}
void RtpSender::writeBuffer(double sample, int pos, int channel_idx) {
  buffer[pos * channels + channel_idx] =
      static_cast<rtpsr::sample_t>(sample * INT16_MAX);
}

void RtpSender::sendData() {
  read_flag = true;
  remain = framesize;
  int64_t offset = 0;
  int fill_res = avcodec_fill_audio_frame(
      frame, channels, AV_SAMPLE_FMT_S16, getBufferPtr(),
      framesize * channels * sizeof(rtpsr::sample_t), 0);
  frame->pts = timecount;
  frame->format = AV_SAMPLE_FMT_S16;
  frame->channels = channels;
  frame->channel_layout = codecctx_enc->channel_layout;
  timecount += framesize;
  int res = avcodec_send_frame(codecctx_enc, frame);
  if (res < 0) dumpAvError(res);
  while (read_flag) {
    av_init_packet(packet);
    int res = avcodec_receive_packet(codecctx_enc, packet);
    if (res == AVERROR(EAGAIN)) {
      read_flag = false;  // frame is fully flushed, finish sending
    } else {
      // remain = timecount - (packet->pts + framesize);
      // read_offset = framesize - remain;
      auto itb = instream->time_base;
      auto otb = outstream->time_base;
      av_packet_rescale_ts(packet, itb, otb);
      packet->pos = -1;
      av_interleaved_write_frame(output_format_ctx, packet);
    }
    av_packet_unref(packet);
  }
}
void RtpSender::writeHeader() {
  auto headerres = avformat_write_header(output_format_ctx, nullptr);
  if (headerres < 0) {
    dumpAvError(headerres);
  }
}
int RtpSender::readPacketSelf(void* userdata, uint8_t* avio_buf, int buf_size) {
  auto* sender = reinterpret_cast<RtpSender*>(userdata);
  auto* address = sender->getBufferPtr();
  int8_t offset = getBytesFromSamples(sender->read_offset, sender->channels);
  int remain_bytes = getBytesFromSamples(sender->remain, sender->channels);
  int read_size = std::min(buf_size, remain_bytes);
  memcpy(avio_buf, address + offset, read_size);
  if (read_size - remain_bytes == 0) {  // finished reading in this term
    sender->read_flag = false;
  }
  return read_size;
}
void RtpSender::incrementTime(int64_t count) { timecount += count; }
// test function
