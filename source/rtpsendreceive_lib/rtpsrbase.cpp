#include "rtpsrbase.hpp"

namespace rtpsr {
int CustomCbInFormat::readPacket(void* userdata, uint8_t* avio_buf,
                                 int buf_size) {
  auto* sender = reinterpret_cast<CustomCbInFormat*>(userdata);
  auto* address = sender->buffer.data();
  getBufSize(sender->setting);
  memcpy(avio_buf, address, buf_size);
  return buf_size;
}

int CustomCbOutFormat::writePacket(void* userdata, uint8_t* avio_buf,
                                   int buf_size) {
  auto* receiver = reinterpret_cast<CustomCbOutFormat*>(userdata);
  auto* address = receiver->buffer.data();
  memcpy(address, avio_buf, buf_size);
  return buf_size;
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

}  // namespace rtpsr