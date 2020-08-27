#ifndef FBA_LIBFFMPEG_H
#define FBA_LIBFFMPEG_H

/*
 *  Copyright 2013-Present Two Big Ears Limited
 *  All rights reserved.
 *  http://twobigears.com
 */

#pragma once

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include "LibLoader.hh"

namespace TBE {

#ifdef WIN32
static const std::string lib_suffix = ".dll";
static const std::string kAvUtil = "avutil-55";
static const std::string kSwResample = "swresample-2";
static const std::string kAvCodec = "avcodec-57";
static const std::string kAvFormat = "avformat-57";
static const std::string kSwScale = "swscale-4";
#else // mac
const std::string lib_suffix = ".dylib";
const std::string kAvUtil = "libavutil.55";
const std::string kSwResample = "libswresample.2";
const std::string kAvCodec = "libavcodec.57";
const std::string kAvFormat = "libavformat.57";
const std::string kSwScale = "libswscale.4";
#endif

struct ScopedAVPacket {
  using unref = decltype(&av_packet_unref);
  ScopedAVPacket(unref ptr) : unref_ptr_(ptr) {}
  ~ScopedAVPacket() {
    unref_ptr_(&packet_);
  }
  AVPacket packet_;
  unref unref_ptr_;
};

/// Loads ffmpeg dybamic libraries and their symbols
class LibFfmpeg {
 public:
  /// Loads required ffmpeg libraries. Throws an exception (std::runtime_error) if libraries or
  /// symbols aren't found. \param ffmpegLibPath Path to the folder containing the libffmpeg
  /// libraries
  LibFfmpeg(const char* ffmpegLibPath)
      : avUtil_(LibLoader(ffmpegLibPath, kAvUtil, lib_suffix)),
        swResample_(LibLoader(ffmpegLibPath, kSwResample, lib_suffix)),
        avCodec_(LibLoader(ffmpegLibPath, kAvCodec, lib_suffix)),
        avFormat_(LibLoader(ffmpegLibPath, kAvFormat, lib_suffix)),
        swScale_(LibLoader(ffmpegLibPath, kSwScale, lib_suffix)) {
    // AvFormat
    av_register_all = (decltype(av_register_all))avFormat_.getFunction("av_register_all");
    avformat_open_input =
        (decltype(avformat_open_input))avFormat_.getFunction("avformat_open_input");
    avformat_find_stream_info =
        (decltype(avformat_find_stream_info))avFormat_.getFunction("avformat_find_stream_info");
    avformat_close_input =
        (decltype(avformat_close_input))avFormat_.getFunction("avformat_close_input");
    avformat_seek_file = (decltype(avformat_seek_file))avFormat_.getFunction("avformat_seek_file");
    avformat_flush = (decltype(avformat_flush))avFormat_.getFunction("avformat_flush");
    av_seek_frame = (decltype(av_seek_frame))avFormat_.getFunction("av_seek_frame");
    av_read_frame = (decltype(av_read_frame))avFormat_.getFunction("av_read_frame");

    // AvCodec
    av_packet_unref = (decltype(av_packet_unref))avCodec_.getFunction("av_packet_unref");

    avcodec_find_decoder =
        (decltype(avcodec_find_decoder))avCodec_.getFunction("avcodec_find_decoder");
    avcodec_alloc_context3 =
        (decltype(avcodec_alloc_context3))avCodec_.getFunction("avcodec_alloc_context3");
    avcodec_open2 = (decltype(avcodec_open2))avCodec_.getFunction("avcodec_open2");
    avcodec_close = (decltype(avcodec_close))avCodec_.getFunction("avcodec_close");
    avcodec_flush_buffers =
        (decltype(avcodec_flush_buffers))avCodec_.getFunction("avcodec_flush_buffers");
    avcodec_parameters_to_context = (decltype(avcodec_parameters_to_context))avCodec_.getFunction(
        "avcodec_parameters_to_context");
    avcodec_receive_frame =
        (decltype(avcodec_receive_frame))avCodec_.getFunction("avcodec_receive_frame");
    avcodec_send_packet =
        (decltype(avcodec_send_packet))avCodec_.getFunction("avcodec_send_packet");
    av_codec_get_codec_descriptor = (decltype(av_codec_get_codec_descriptor))avCodec_.getFunction(
        "av_codec_get_codec_descriptor");
    avcodec_string = (decltype(avcodec_string))avCodec_.getFunction("avcodec_string");

    // swsScale
    sws_getContext = (decltype(sws_getContext))swScale_.getFunction("sws_getContext");
    sws_scale = (decltype(sws_scale))swScale_.getFunction("sws_scale");

    // AvUtil
    av_rescale_q = (decltype(av_rescale_q))avUtil_.getFunction("av_rescale_q");
    av_frame_alloc = (decltype(av_frame_alloc))avUtil_.getFunction("av_frame_alloc");
    av_frame_free = (decltype(av_frame_free))avUtil_.getFunction("av_frame_free");
    av_frame_get_best_effort_timestamp =
        (decltype(av_frame_get_best_effort_timestamp))avUtil_.getFunction(
            "av_frame_get_best_effort_timestamp");
    av_strerror = (decltype(av_strerror))avUtil_.getFunction("av_strerror");
    av_dict_set_int = (decltype(av_dict_set_int))avUtil_.getFunction("av_dict_set_int");
    av_dict_set = (decltype(av_dict_set))avUtil_.getFunction("av_dict_set");
    av_dict_get = (decltype(av_dict_get))avUtil_.getFunction("av_dict_get");
  }

  // AvFormat
  decltype(&::av_register_all) av_register_all;
  decltype(&::avformat_open_input) avformat_open_input;
  decltype(&::avformat_find_stream_info) avformat_find_stream_info;
  decltype(&::avformat_close_input) avformat_close_input;
  decltype(&::avformat_seek_file) avformat_seek_file;
  decltype(&::avformat_flush) avformat_flush;
  decltype(&::av_seek_frame) av_seek_frame;
  decltype(&::av_read_frame) av_read_frame;

  // avcodec
  decltype(&::avcodec_find_decoder) avcodec_find_decoder;
  decltype(&::avcodec_alloc_context3) avcodec_alloc_context3;
  decltype(&::avcodec_open2) avcodec_open2;
  decltype(&::avcodec_close) avcodec_close;
  decltype(&::avcodec_flush_buffers) avcodec_flush_buffers;
  decltype(&::avcodec_parameters_to_context) avcodec_parameters_to_context;
  decltype(&::avcodec_receive_frame) avcodec_receive_frame;
  decltype(&::avcodec_send_packet) avcodec_send_packet;
  decltype(&::av_codec_get_codec_descriptor) av_codec_get_codec_descriptor;
  decltype(&::avcodec_string) avcodec_string;
  decltype(&::av_frame_alloc) av_frame_alloc;
  decltype(&::av_frame_free) av_frame_free;
  decltype(&::av_frame_get_best_effort_timestamp) av_frame_get_best_effort_timestamp;
  decltype(&::av_packet_unref) av_packet_unref;
  decltype(&::av_strerror) av_strerror;
  decltype(&::av_rescale_q) av_rescale_q;
  decltype(&::sws_getContext) sws_getContext;
  decltype(&::sws_scale) sws_scale;
  decltype(&::av_dict_set_int) av_dict_set_int;
  decltype(&::av_dict_set) av_dict_set;
  decltype(&::av_dict_get) av_dict_get;

 private:
  LibLoader avUtil_;
  LibLoader swResample_;
  LibLoader avCodec_;
  LibLoader avFormat_;
  LibLoader swScale_;
};
} // namespace TBE

#endif // FBA_LIBFFMPEG_H
