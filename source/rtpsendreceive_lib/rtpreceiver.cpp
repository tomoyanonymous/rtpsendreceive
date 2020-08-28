#include "rtpreceiver.hpp"

namespace rtpsr{
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
  while (!finish_read) {
    av_init_packet(packet);
    checkAvError(av_read_frame(input->ctx, packet));
    bool readflag = true;
    readflag = decoder.sendPacket(packet);
    readflag = decoder.receiveFrame(frame);
    int samples = frame->nb_samples;
    auto frameref = av_frame_get_plane_buffer(frame, 0);
    size_t offset = read_count * setting.channels * sizeof(int16_t);
    for (int i = read_count; i < read_count + samples; i++) {
      int16_t s = reinterpret_cast<int16_t*>(frameref->data)[i];
      getBuffer().at(i)=s;
    }
    read_count += samples;
    finish_read = (read_count >= setting.framesize);
    av_packet_unref(packet);
  }
}

}