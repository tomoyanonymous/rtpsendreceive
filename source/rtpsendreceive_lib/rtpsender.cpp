#include "rtpsender.hpp"

namespace rtpsr {
RtpSender::RtpSender(RtpSRSetting& s, Url& url, Codec codec, std::ostream& logger)
    : RtpSRBase(s,logger), encoder(s, codec) {
  input = std::static_pointer_cast<InFormat>(
      std::make_shared<CustomCbInFormat>(setting));
  output = std::static_pointer_cast<OutFormat>(
      std::make_shared<RtpOutFormat>(url, setting));
  auto* instream = avformat_new_stream(input->ctx, encoder.ctx->codec);
  auto* outstream = avformat_new_stream(output->ctx, encoder.ctx->codec);
  checkAvError(
      avcodec_parameters_from_context(instream->codecpar, encoder.ctx));
  checkAvError(
      avcodec_parameters_from_context(outstream->codecpar, encoder.ctx));
  instream->start_time = 0;
  outstream->start_time = 0;
  setCtxParams(&params);
  url_tmp = getSdpUrl(url);
  char* urlctx = (char*)av_malloc(url_tmp.size() + sizeof(char));
  av_strlcpy(urlctx, url_tmp.c_str(), url_tmp.size() + sizeof(char));
  output->ctx->url = urlctx;
  checkAvError(avformat_init_output(output->ctx, &params));
  checkAvError(avio_open(&output->ctx->pb, urlctx, AVIO_FLAG_WRITE));
  wait_connection = std::async(std::launch::async, [&]() {
    while (true) {
      int res = avformat_write_header(output->ctx, nullptr);
      if (res >= 0) {
        break;
      }
      if (res == -61 || res == -22) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        logger << "rtpsender: connection refused,retry in 200ms" << std::endl;
      } else {
        checkAvError(res);
        break;
      }
    }
    logger << "rtpsender: connected" << std::endl;
    return 1;
  });
}

void RtpSender::setCtxParams(AVDictionary** dict) {
  checkAvError(
      av_dict_set(dict, "protocol_whitelist", "file,udp,rtp,tcp,rtsp", 0));
  checkAvError(av_dict_set(dict, "rtsp_transport", "udp", 0));
  checkAvError(av_dict_set(dict, "enable-protocol", "rtp", 0));
  checkAvError(av_dict_set(dict, "enable-protocol", "udp", 0));
  checkAvError(av_dict_set(dict, "enable-muxer", "rtsp", 0));
  checkAvError(av_dict_set(dict, "enable-demuxer", "rtsp", 0));
  checkAvError(av_dict_set(dict, "stimeout", "1000000",
                           0));  // wait up to 10 seconds until connect
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

}  // namespace rtpsr