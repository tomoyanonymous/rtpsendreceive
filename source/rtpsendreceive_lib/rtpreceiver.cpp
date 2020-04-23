#include "rtpreceiver.hpp"


RtpReceiver::RtpReceiver(int framesize, int samplerate, int channels,
                     std::string address, int port,
                     rtpsr::readfn_type callback_read,
                     rtpsr::seekfn_type callback_seek, void* userdata)
    : rtp_ioctx(nullptr),
      address(std::move(address)),
      port(port),
      callback_read(callback_read),
      callback_seek(callback_seek),
      RtpSRBase(framesize, samplerate, channels),
      avio_buffer(nullptr) {
  if (userdata == nullptr) {
    userdata_address = reinterpret_cast<void*>(this);
  }
  // todo
}
RtpReceiver::~RtpReceiver() {
  // todo
  av_packet_unref(packet);
  avio_close(rtp_ioctx);
  av_free(avio_buffer);
}

// void RtpReceiver::init() {
//   std::cerr<< "hogehoge"<<std::endl;
// }

void RtpReceiver::initFormatCtx() {
  // for audio driver->ffmpeg
  std::string destination = "rtp://" + address + ":" + std::to_string(port);
  avio_buffer = (uint8_t*)av_malloc(bufsize);
  avioctx = avio_alloc_context(avio_buffer, bufsize, 0, userdata_address,
                               callback_read, nullptr, callback_seek);
  // Determine the input-format:
  AVProbeData probe_data{"", avio_buffer, 512, "audio/L16"};
  probe_data.buf = avio_buffer;
  input_format_ctx = avformat_alloc_context();
  input_format_ctx->pb = avioctx;
  input_format_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
  input_format_ctx->iformat = av_probe_input_format(&probe_data, 1);
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
  char* url = new char[destination.size() + 1];
  std::char_traits<char>::copy(url, destination.c_str(),
                               destination.size() + 1);
  output_format_ctx->url = url;
  delete[] url;
  av_new_packet(packet, bufsize);
}

void RtpReceiver::setDestination(std::string& ad, int po) {
  address = ad;
  port = po;
}
void RtpReceiver::writeBuffer(double sample, int pos, int channel_idx) {
  buffer[pos * channels + channel_idx] =
      static_cast<rtpsr::sample_t>(sample * INT16_MAX);
}

void RtpReceiver::sendData() {
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

int RtpReceiver::readPacketSelf(void* userdata, uint8_t* avio_buf, int buf_size) {
  auto* sender = reinterpret_cast<RtpReceiver*>(userdata);
  auto* address = sender->getBufferPtr();
  memcpy(avio_buf, address, buf_size);
  return buf_size;
}
