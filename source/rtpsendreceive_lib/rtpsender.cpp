#include "rtpsender.hpp"

RtpSender::RtpSender(int framesize, int samplerate, int channels,
                     const std::string&  address, const int port,
                     rtpsr::readfn_type callback_read,
                     rtpsr::seekfn_type callback_seek, void* userdata)
    : rtp_ioctx(nullptr),
    address(std::move(address)),
    port(port),
      callback_read(callback_read),
      callback_seek(callback_seek),
      RtpSRBase(framesize,samplerate,channels) {
  if (userdata == nullptr) {
    userdata_address = (void*)this;
  }
  // todo
}
RtpSender::~RtpSender() {
  // todo
  av_packet_unref(packet);
  avio_close(rtp_ioctx);
  av_free(avio_buffer);
}

// void RtpSender::init() {
//   std::cerr<< "hogehoge"<<std::endl;
// }

void RtpSender::initFormatCtx() {
  // for audio driver->ffmpeg
  std::string destination = "rtp://" + address + ":" + std::to_string(port);
  avio_buffer = (uint8_t*)av_malloc(bufsize);
  avioctx = avio_alloc_context(avio_buffer, bufsize, 0, userdata_address,
                               callback_read, nullptr, callback_seek);
  // Determine the input-format:
  AVProbeData probeData{"", avio_buffer, 512, "audio/L16"};
  probeData.buf = avio_buffer;
  input_format_ctx = avformat_alloc_context();
  input_format_ctx->pb = avioctx;
  input_format_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
  input_format_ctx->iformat = av_probe_input_format(&probeData, 1);
  // input_format_ctx->iformat = av_find_input_format("audio/L16");
  int inputres = avformat_open_input(&input_format_ctx, "", nullptr, nullptr);
  if (inputres < 0) {
    dumpAvError(inputres);
  }
  // usleep(100000);
  fmt_output = av_guess_format("rtp", destination.c_str(), "audio/L16");
  output_format_ctx = avformat_alloc_context();
  output_format_ctx->pb = rtp_ioctx;
  auto res =
      avio_open(&output_format_ctx->pb, destination.c_str(), AVIO_FLAG_WRITE);
  if (res < 0) {
    std::cerr << "avio open error\n";
  }
  output_format_ctx->oformat = fmt_output;
  auto url = new char[destination.size() + 1];
  std::char_traits<char>::copy(url, destination.c_str(),
                               destination.size() + 1);
  output_format_ctx->url = url;
  // delete url;
  av_new_packet(packet, bufsize);
}

void RtpSender::setDestination(std::string& ad, int po) {
  address = ad;
  port = po;
}

void RtpSender::start() {
  // av_new_packet(packet, 4096);
  int count = 0;
  while (true) {
    auto ret = av_read_frame(input_format_ctx, packet);
    // int response = avcodec_send_packet(codecctx_dec, packet);
    // dumpAvError(response);
    // while (response >= 0) {
    //   response = avcodec_receive_frame(codecctx_dec, frame);
    //   if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
    //     break;
    //   } else if (response < 0) {
    //     dumpAvError(response);
    //     // throw std::runtime_error(errbuf);
    //   }
    //   if (response >= 0) {
    //     encode(output_format_ctx, instream, outstream, codecctx_dec, frame,
    //            packet, 0);
    //     usleep(100000);
    //   }
    //   av_frame_unref(frame);
    // }

    // // auto* stream = input_format_ctx->streams[packet->stream_index];
    packet->pts = av_rescale_q_rnd(packet->pts, instream->time_base,
                                   outstream->time_base, AV_ROUND_PASS_MINMAX);
    packet->dts = av_rescale_q_rnd(packet->dts, instream->time_base,
                                   outstream->time_base, AV_ROUND_PASS_MINMAX);
    packet->duration = av_rescale_q(packet->duration, instream->time_base,
                                    outstream->time_base);
    packet->pos = -1;

    // std::cout << "buffer count : " << count++ << "\n";
    av_packet_rescale_ts(packet, instream->time_base, outstream->time_base);

    av_interleaved_write_frame(output_format_ctx, packet);
    usleep(512 * 1000000. / 48000);
  }
}

void RtpSender::writeBuffer(double sample, int pos, int channel_idx) {
  buffer[pos*channels + channel_idx] = static_cast<rtpsr::sample_t>(sample* INT16_MAX);
}

void RtpSender::sendData() {
  av_init_packet(packet);
  auto ret = av_read_frame(input_format_ctx, packet);
  packet->pts = av_rescale_q_rnd(packet->pts, instream->time_base,
                                 outstream->time_base, AV_ROUND_PASS_MINMAX);
  packet->dts = av_rescale_q_rnd(packet->dts, instream->time_base,
                                 outstream->time_base, AV_ROUND_PASS_MINMAX);
  packet->duration =
      av_rescale_q(packet->duration, instream->time_base, outstream->time_base);
  packet->pos = -1;

  av_packet_rescale_ts(packet, instream->time_base, outstream->time_base);
  av_interleaved_write_frame(output_format_ctx, packet);

  av_packet_unref(packet);
}

int RtpSender::readPacketSelf(void* userdata, uint8_t* avio_buf, int buf_size) {
  auto* sender = reinterpret_cast<RtpSender*>(userdata);
  auto* address = sender->getBuffer_ptr();
  memcpy(avio_buf, address, buf_size);
  return buf_size;
}