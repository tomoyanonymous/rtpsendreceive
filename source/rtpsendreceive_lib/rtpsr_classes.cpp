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



int CustomCbOutFormat::writePacket(void* userdata, uint8_t* avio_buf,
                                   int buf_size) {
  auto* receiver = reinterpret_cast<CustomCbOutFormat*>(userdata);
  auto* address = receiver->buffer.data();
  memcpy(address, avio_buf, buf_size);
  return buf_size;
}
rtpsr::sample_t CustomCbOutFormat::getBuffer(int pos, int channel) {
  return buffer.at(pos * setting.channels + channel);
}

void RtpSender::setCtxParams(AVDictionary** dict) {
  checkAvError(
      av_dict_set(dict, "protocol_whitelist", "file,udp,rtp,tcp,rtsp", 0));
  checkAvError(av_dict_set(dict, "rtsp_transport", "udp", 0));
  checkAvError(av_dict_set(dict, "enable-protocol", "rtp", 0));
  checkAvError(av_dict_set(dict, "enable-protocol", "udp", 0));
  checkAvError(av_dict_set(dict, "enable-muxer", "rtsp", 0));
  checkAvError(av_dict_set(dict, "enable-demuxer", "rtsp", 0));
  checkAvError(av_dict_set(dict, "timeout", "10", 0));
}

bool CodecBase::checkIsErrAgain(int error_code) {
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

void RtpReceiver::setCtxParams(AVDictionary** dict) {
  checkAvError(
      av_dict_set(dict, "protocol_whitelist", "file,udp,rtp,tcp,rtsp", 0));
  checkAvError(av_dict_set(dict, "rtsp_transport", "udp", 0));
  checkAvError(av_dict_set(dict, "enable-protocol", "rtp", 0));
  checkAvError(av_dict_set(dict, "enable-protocol", "udp", 0));
  checkAvError(av_dict_set(dict, "enable-muxer", "rtsp", 0));
  checkAvError(av_dict_set(dict, "enable-demuxer", "rtsp", 0));
  checkAvError(
      av_dict_set_int(dict, "reorder_queue_size", 50000, 0));  // 0.05sec
  checkAvError(av_dict_set(dict, "rtsp_flags", "listen", 0));
  checkAvError(av_dict_set(dict, "allowed_media_types", "audio", 0));
}

void RtpReceiver::receiveData() {
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