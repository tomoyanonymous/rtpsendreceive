#include "rtpsr_classes.hpp"

namespace rtpsr {

int CustomCbInFormat::readPacket(void* userdata, uint8_t* avio_buf,
                                 int buf_size) {
  auto* sender = reinterpret_cast<CustomCbInFormat*>(userdata);
  auto* address = sender->buffer.data();
  getBufSize(sender->setting);
  memcpy(avio_buf, address, buf_size);
  return buf_size;
}

void CustomCbInFormat::setBuffer(rtpsr::sample_t sample, int pos, int channel) {
  buffer.at(pos * setting.channels + channel) = sample;
}

// std::string& RtpInFormat::makeDummySdp() {
//   std::string sdpcontent = R"(SDP:
// v=0
// o=- 0 0 IN IP4 $address$
// s=DUMMY SDP
// c=IN IP4 $address$
// t=0 0
// a=tool:libavformat 58.29.100
// m=audio $port$ RTP/AVP 97
// b=AS:$bitrate$
// a=rtpmap:97 L16/$samplerate$/$channels$)";
//   std::vector<std::pair<std::string, std::string>> replace_pairs = {
//       {"\\$port\\$", std::to_string(port)},
//       {"\\$address\\$", "127.0.0.1"},
//       {"\\$channels\\$", std::to_string(setting.channels)},
//       {"\\$bitrate\\$",
//        std::to_string((setting.samplerate / 1000) * 16 * setting.channels)},
//       {"\\$samplerate\\$", std::to_string(setting.samplerate)}};
//   std::for_each(replace_pairs.begin(), replace_pairs.end(), [&](auto pair) {
//     dummysdp_content =
//         std::regex_replace(sdpcontent, std::regex(pair.first), pair.second);
//   });
//   return dummysdp_content;
// }
// int RtpInFormat::readDummySdp(void* userdata, uint8_t* avio_buf, int buf_size) {
//   auto octx = static_cast<SdpOpaque*>(userdata);
//   if (octx->pos == octx->data.end()) {
//     return AVERROR_EOF;
//   }
//   auto dist = static_cast<int>(std::distance(octx->pos, octx->data.end()));
//   auto count = std::min(buf_size, dist);
//   std::copy(octx->pos, octx->pos + count, avio_buf);
//   octx->pos += count;
//   return count;
// }

int CustomCbOutFormat::writePacket(void* userdata, uint8_t* avio_buf,
                                   int buf_size) {
  auto* receiver = reinterpret_cast<CustomCbOutFormat*>(userdata);
  auto* address = receiver->buffer.data();
  memcpy(address, avio_buf, buf_size);
  return buf_size;
}
rtpsr::sample_t CustomCbOutFormat::getBuffer(int pos, int channel) {
    return buffer.at(pos*setting.channels+channel);
}

bool checkIsErrAgain(int error_code) {
  if (error_code == AVERROR(EAGAIN)) {
    return true;  // return isempty
  }
  checkAvError(error_code);
  return false;
}

bool Decoder::sendPacket(AVPacket* packet) {
  return checkIsErrAgain(avcodec_send_packet(ctx, packet));
}
bool Decoder::receiveFrame(AVFrame* frame) {
  return checkIsErrAgain(avcodec_receive_frame(ctx, frame));
}

bool Encoder::sendFrame(AVFrame* frame) {
  return checkIsErrAgain(avcodec_send_frame(ctx, frame));
}
bool Encoder::receivePacket(AVPacket* packet) {
  return checkIsErrAgain(avcodec_receive_packet(ctx, packet));
}

void RtpSender::sendData() {
  checkAvError(avcodec_fill_audio_frame(frame, setting.channels,
                                        AV_SAMPLE_FMT_S16, getBufPointer(),
                                        getBufSize(setting), 0));
  frame->pts = timecount;
  frame->format = AV_SAMPLE_FMT_S16;
  frame->channels = setting.channels;
  frame->channel_layout = encoder.ctx->channel_layout;
  timecount += setting.framesize;
  bool isfull = encoder.sendFrame(frame);
  bool read_flag = true;
  while (read_flag) {
    av_init_packet(packet);
    bool isempty = encoder.receivePacket(packet);
    if (isempty) {
      read_flag = false;  // frame is fully flushed, finish sending
    } else {
      auto itb = input->ctx->streams[0]->time_base;
      auto otb = output->ctx->streams[0]->time_base;
      av_packet_rescale_ts(packet, itb, otb);
      packet->pos = -1;
      av_interleaved_write_frame(output->ctx, packet);
    }
    av_packet_unref(packet);
  }
}

void RtpReciever::receiveData() {
  int read_count = 0;
  bool finish_read = false;
  while (finish_read) {
    av_init_packet(packet);
    checkAvError(av_read_frame(input->ctx, packet));
    bool readflag = true;
    readflag = decoder.sendPacket(packet);
    readflag = decoder.receiveFrame(frame);
    int samples = frame->nb_samples;
    auto frameref = av_frame_get_plane_buffer(frame, 0);
    size_t offset = read_count * setting.channels * sizeof(int16_t);
    for (int i = read_count; i < read_count + samples; i++) {
      getBuffer().at(i) = reinterpret_cast<int16_t*>(frameref->data)[i];
    }
    read_count += samples;
    finish_read = (read_count >= setting.framesize);
    av_packet_unref(packet);
  }
}

}  // namespace rtpsr