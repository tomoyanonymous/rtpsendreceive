#include "rtpreceiver.hpp"

namespace rtpsr{
  RtpReceiver::RtpReceiver(RtpSRSetting& s, Url& url, Codec codec,std::ostream& logger)
      : RtpSRBase(s,logger), decoder(s, codec){
    input = std::static_pointer_cast<InFormat>(
        std::make_shared<RtpInFormat>(url, setting));
    output = std::static_pointer_cast<OutFormat>(
        std::make_shared<CustomCbOutFormat>(setting));

    ifmt = av_find_input_format("rtsp");
    url_tmp = getSdpUrl(url);

    setCtxParams(&params);
    input->ctx->max_delay = 1000;
    wait_connection = std::async(std::launch::async, [&]() -> int {
      logger << "rtpreceiver started connecting..." << std::endl;
      int res = avformat_open_input(&input->ctx, url_tmp.c_str(), ifmt,
                                    &params);  // this start blocking...
      auto* instream = avformat_new_stream(input->ctx, decoder.ctx->codec);
      auto* outstream = avformat_new_stream(output->ctx, decoder.ctx->codec);
      checkAvError(
          avcodec_parameters_from_context(instream->codecpar, decoder.ctx));
      checkAvError(
          avcodec_parameters_from_context(outstream->codecpar, decoder.ctx));
      instream->start_time = 0;
      outstream->start_time = 0;
      logger << "rtpreceiver connected" << std::endl;
      return res;
    });
  }
  RtpReceiver::~RtpReceiver(){ 
          logger << "rtpreceiver destructor called" << std::endl;
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