#include "rtpreceiver.hpp"

RtpReceiver::RtpReceiver(int framesize, int samplerate, int channels,
                         std::string address, int port,
                         rtpsr::readfn_type callback_write,
                         rtpsr::seekfn_type callback_seek, void* userdata)
    : sdp_ioctx(nullptr),
      address(std::move(address)),
      port(port),
      callback_write(callback_write),
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

  if(packet){
  av_packet_unref(packet);
  }
  auto opaque = static_cast<SdpOpaque*>(input_format_ctx->pb->opaque);
  delete opaque;
  input_format_ctx->pb->opaque =nullptr;
  avformat_close_input(&input_format_ctx);

  avio_close(sdp_ioctx);
  av_free(avio_buffer);

}

void RtpReceiver::initInputFormat() {
  input_format_ctx = avformat_alloc_context();
  avformat_network_init();
  sdpio_buffer = (uint8_t*)av_malloc(bufsize);

  auto opaque = new SdpOpaque();
  makeDummySdp();
  auto sdp = sdp_content.c_str();
  opaque->data = SdpOpaque::Vector(sdp, sdp + strlen(sdp));
  opaque->pos = opaque->data.begin();

  sdp_ioctx = avio_alloc_context(sdpio_buffer, bufsize, 0, opaque, readDummySdp,
                                 nullptr, nullptr);
  input_format_ctx->pb = sdp_ioctx;
  auto infmt = av_find_input_format("sdp");
  int inputres =
      avformat_open_input(&input_format_ctx, "internal.sdp", infmt, nullptr);
  if (inputres < 0) {
    dumpAvError(inputres);
  }

}
void RtpReceiver::initOutputFormat() {
  avio_buffer = (uint8_t*)av_malloc(bufsize);
  avioctx = avio_alloc_context(avio_buffer, bufsize, 1, userdata_address,
                               nullptr, callback_write, callback_seek);
  // Determine the input-format:
  AVProbeData probe_data{"", avio_buffer, 512, "audio/L16"};
  probe_data.buf = avio_buffer;
  output_format_ctx = avformat_alloc_context();
  output_format_ctx->pb = avioctx;
  output_format_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
  fmt_output = av_guess_format("s16be", "", "audio/L16");
  output_format_ctx->oformat = fmt_output;
  av_new_packet(packet, bufsize);

}

void RtpReceiver::initFormatCtx() {
  initInputFormat();
  initOutputFormat();
}

void RtpReceiver::setSource(std::string& ad, int po) {
  address = ad;
  port = po;
}

void RtpReceiver::play(){

  av_read_play(input_format_ctx);
}
void RtpReceiver::pause(){
  av_read_pause(input_format_ctx);
}

// void RtpReceiver::writeBuffer(double sample, int pos, int channel_idx) {
//   buffer[pos * channels + channel_idx] =
//       static_cast<rtpsr::sample_t>(sample * INT16_MAX);
// }


double RtpReceiver::readBuffer(int pos, int channel_idx){
  return static_cast<double>(buffer[pos*channels+channel_idx])/INT16_MAX;
}

//static functions


void RtpReceiver::receiveData() {
  av_init_packet(packet);
  auto ret = av_read_frame(input_format_ctx, packet);
  av_packet_rescale_ts(packet, instream->time_base, outstream->time_base);
  packet->pos = -1;
  auto res= av_interleaved_write_frame(output_format_ctx, packet);
  
  av_packet_unref(packet);
}

int RtpReceiver::writePacketSelf(void* userdata, uint8_t* avio_buf,
                                 int buf_size) {
  auto* receiver = reinterpret_cast<RtpReceiver*>(userdata);
  auto* address = receiver->getBufferPtr();
  memcpy(address, avio_buf,buf_size);
  return buf_size;
}

int RtpReceiver::readDummySdp(void* userdata, uint8_t* avio_buf, int buf_size) {
  auto octx = static_cast<SdpOpaque*>(userdata);
  if (octx->pos == octx->data.end()) {
    return 0;
  }
  auto dist = static_cast<int>(std::distance(octx->pos, octx->data.end()));
  auto count = std::min(buf_size, dist);
  std::copy(octx->pos, octx->pos + count, avio_buf);
  octx->pos += count;
  return count;
}

void RtpReceiver::makeDummySdp() {
  // AVFormatContext* dummyctx =avformat_alloc_context();
  // auto res = avio_open(&dummyctx->pb,
  // ("rtp://127.0.0.1:"+std::to_string(port)).c_str(), AVIO_FLAG_WRITE);
  // avformat_free_context(dummyctx);

  sdp_content = R"(SDP:
v=0
o=- 0 0 IN IP4 127.0.0.1
s=DUMMY SDP
c=IN IP4 127.0.0.1
t=0 0
a=tool:libavformat 58.29.100
m=audio $port$ RTP/AVP 97
b=AS:$bitrate$
a=rtpmap:97 L16/$samplerate$/$channels$)";
  std::vector<std::pair<std::string, std::string>> replace_pairs = {
      {"\\$port\\$", std::to_string(port)},
      {"\\$channels\\$", std::to_string(channels)},
      {"\\$bitrate\\$", std::to_string((samplerate / 1000) * 16 * channels)},
      {"\\$samplerate\\$", std::to_string(samplerate)}};
  std::for_each(replace_pairs.begin(), replace_pairs.end(), [&](auto pair) {
    sdp_content =
        std::regex_replace(sdp_content, std::regex(pair.first), pair.second);
  });
}