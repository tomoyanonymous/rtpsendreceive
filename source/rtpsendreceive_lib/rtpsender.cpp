#include "rtpsender.hpp"

namespace rtpsr{
void RtpSender::setCtxParams(AVDictionary** dict) {
  checkAvError(
      av_dict_set(dict, "protocol_whitelist", "file,udp,rtp,tcp,rtsp", 0));
  checkAvError(av_dict_set(dict, "rtsp_transport", "udp", 0));
  checkAvError(av_dict_set(dict, "enable-protocol", "rtp", 0));
  checkAvError(av_dict_set(dict, "enable-protocol", "udp", 0));
  checkAvError(av_dict_set(dict, "enable-muxer", "rtsp", 0));
  checkAvError(av_dict_set(dict, "enable-demuxer", "rtsp", 0));
  checkAvError(av_dict_set(dict, "stimeout", "1000000", 0));//wait up to 10 seconds until connect
}

  void RtpSender::sendData() {
  //   av_read_frame(input->ctx, packet);
  checkAvError(avcodec_fill_audio_frame(
      frame, setting.channels, AV_SAMPLE_FMT_S16, (uint8_t*)getBuffer().data(),
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
      packet->pos = -1;
      packet->stream_index = 0;
    //   packet->duration = setting.framesize;
      auto itb = input->ctx->streams[0]->time_base;
      auto otb = output->ctx->streams[0]->time_base;
      av_packet_rescale_ts(packet, otb, otb);  // maybe unnecessary
      av_interleaved_write_frame(output->ctx, packet);
    }
    av_packet_unref(packet);
  }
}

}