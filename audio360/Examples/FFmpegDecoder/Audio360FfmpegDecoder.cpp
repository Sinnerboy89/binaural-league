/*
 * Copyright (c) 2018-present, Facebook, Inc.
 */

#include "Audio360FfmpegDecoder.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>

static const int numThreads = 4;

namespace TBE {
static void logError(const std::string& s) {
  std::cerr << s << std::endl;
}

Audio360FfmpegDecoder::Audio360FfmpegDecoder(const std::string& ffmpegLibPath)
    : ffmpeg_(ffmpegLibPath.c_str()) {}

Audio360FfmpegDecoder::~Audio360FfmpegDecoder() {
  close();
}

Audio360FfmpegDecoder::Status
Audio360FfmpegDecoder::open(const std::string& file, bool useAudioDevice, ChannelMap map) {
  channelMap_ = map;

  if (ready_) {
    close();
  }

  ffmpeg_.av_register_all();
  if (ffmpeg_.avformat_open_input(&context_, file.c_str(), nullptr, nullptr) != 0) {
    return Status::DECODER_ERROR;
  }

  // Find the audio stream ID, if available
  for (unsigned i = 0; i < context_->nb_streams; i++) {
    if (context_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      audioStreamId_ = i;
    }
  }

  if (audioStreamId_ < 0) {
    logError("Could not find audio stream in " + file);
    close();
    return Status::DECODER_ERROR;
  }

  // set the number of threads to use
  AVDictionary* opts = NULL;
  ffmpeg_.av_dict_set_int(&opts, "threads", numThreads, 0);

  // Cache the stream's time base which will be used for seek and time stamp calculations later.
  audioStreamTimeBaseMs_ = av_q2d(context_->streams[audioStreamId_]->time_base) * 1000.0;

  // Initialise the AudioFormatDecoder from the TBE Audio Engine
  if (!initialiseOpusDecoder(context_)) {
    close();
    return Status::DECODER_ERROR;
  }

  const auto opusChannels = opusDecoder_->getNumOfChannels();
  const auto expectedChannels = getNumChannelsForMap(channelMap_);
  if (opusChannels != expectedChannels) {
    logError(
        "Incorrect number of channels detected. Expected " + std::to_string(expectedChannels) +
        ", stream has " + std::to_string(opusChannels));
    close();
    return Status::DECODER_ERROR;
  }

  // Used to decode opus to pcm and then enqueue into the audio engine for spatialisation
  pcmBufferSize_ = opusDecoder_->getMaxBufferSizePerChannel() * opusDecoder_->getNumOfChannels();
  pcmBuffer_.reset(new float[pcmBufferSize_]);

  // Initialise the audio engine and related components
  if (!initialiseAudio360Engine(useAudioDevice)) {
    close();
    return Status::DECODER_ERROR;
  }

  ready_ = true;
  return Status::OK;
}

void Audio360FfmpegDecoder::close() {
  ready_ = false;
  didSeek_ = true;

  if (context_) {
    ffmpeg_.avformat_close_input(&context_);
  }
  if (opusDecoder_) {
    delete opusDecoder_;
  }
  if (engine_) {
    TBE_DestroyAudioEngine(engine_);
  }
}

bool Audio360FfmpegDecoder::seek(double milliseconds) {
  if (!ready_) {
    return false;
  }

  newSecondsToSeekTo_ = milliseconds / 1000.f;
  shouldSeek_.store(true);
  return true;
}

Audio360FfmpegDecoder::Status Audio360FfmpegDecoder::decode() {
  if (!ready_) {
    return Status::DECODER_ERROR;
  }

  // need to seek to a new location
  if (shouldSeek_.load()) {
    const AVRational time_base = context_->streams[audioStreamId_]->time_base;
    const int64_t seek_pos = static_cast<int64_t>((newSecondsToSeekTo_)*AV_TIME_BASE);
    const int64_t seek_target = ffmpeg_.av_rescale_q(seek_pos, {1, AV_TIME_BASE}, time_base);

    auto success = ffmpeg_.av_seek_frame(
        context_, audioStreamId_, seek_target, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY);

    if (success >= 0) {
      ffmpeg_.avformat_flush(context_);
      opusDecoder_->flush();
      spatQueue_->flushQueue();
      didSeek_ = true;
    }

    shouldSeek_.store(false);
  }

  // Only decode if the queue has enough space.
  if (spatQueue_->getFreeSpaceInQueue(channelMap_) < pcmBufferSize_) {
    return Status::OK;
  }

  auto status = Status::OK;

  while (spatQueue_->getFreeSpaceInQueue(channelMap_) > pcmBufferSize_ && status == Status::OK) {
    status = enqueueNextPacket();
  }
  return status;
}

Audio360FfmpegDecoder::Status Audio360FfmpegDecoder::enqueueNextPacket() {
  // 1. Demux the opus packet
  // 2. Decode the opus packet
  // 3. Enqueue decoded audio for spatialization

  // READ a frame
  TBE::ScopedAVPacket avPacket(ffmpeg_.av_packet_unref);
  auto err = ffmpeg_.av_read_frame(context_, &avPacket.packet_);

  if (err == AVERROR_EOF) {
    // Signalling the end of stream lets the queue know that it can eventually dequeue all samples.
    spatQueue_->setEndOfStream(true);
    return Status::END_OF_STREAM;
  }

  if (err >= 0 && avPacket.packet_.stream_index == audioStreamId_) {
    if (didSeek_) {
      lastTimeStampMs_ = std::max(0LL, avPacket.packet_.pts) * audioStreamTimeBaseMs_;
      didSeek_ = false;
    }

    // Decode opus packet
    const auto samps = opusDecoder_->decode(
        (const char*)avPacket.packet_.data,
        avPacket.packet_.size,
        pcmBuffer_.get(),
        pcmBufferSize_);
    // Enqueue decoded audio for spatialization
    const auto enq = spatQueue_->enqueueData(pcmBuffer_.get(), (int)samps, channelMap_);

    assert(samps == enq);
    return (samps == enq) ? Status::OK : Status::DECODER_ERROR;
  } else if (err >= 0) {
    return Status::OK;
  }

  return Status::DECODER_ERROR;
}

bool Audio360FfmpegDecoder::initialiseOpusDecoder(AVFormatContext* context) {
  // AvFormat gives us the audio stream's header as extra data
  const auto err = TBE_CreateAudioFormatDecoderFromHeader(
      opusDecoder_,
      (const char*)context->streams[audioStreamId_]->codecpar->extradata,
      context->streams[audioStreamId_]->codecpar->extradata_size);

  if (err != EngineError::OK || opusDecoder_->getName() != std::string("opus")) {
    logError("Failed to create opus decoder. Error code: " + std::to_string(int(err)));
    return false;
  }
  return true;
}

bool Audio360FfmpegDecoder::initialiseAudio360Engine(bool useAudioDevice) {
  EngineInitSettings settings = EngineInitSettings_default;
  settings.audioSettings.sampleRate = opusDecoder_->getSampleRate();
  settings.audioSettings.deviceType =
      (useAudioDevice) ? AudioDeviceType::DEFAULT : AudioDeviceType::DISABLED;

  auto err = TBE_CreateAudioEngine(engine_, settings);
  if (err != EngineError::OK) {
    logError("Failed to create audio engine. Error code: " + std::to_string(int(err)));
    return false;
  }

  // SpatDecoderQueue is used to enqueue decompressed audio.
  err = engine_->createSpatDecoderQueue(spatQueue_);
  if (err != EngineError::OK) {
    logError("Failed to create SpatDecoderQueue. Error code: " + std::to_string(int(err)));
    return false;
  }

  return true;
}

EngineError Audio360FfmpegDecoder::play() {
  if (ready_) {
    return spatQueue_->play();
  }
  return EngineError::FAIL;
}

EngineError Audio360FfmpegDecoder::pause() {
  if (ready_) {
    return spatQueue_->pause();
  }
  return EngineError::FAIL;
}

EngineError Audio360FfmpegDecoder::setFocusProperties(
    float offFocusLevelDecibels,
    float focusWidth) {
  if (!ready_ || !spatQueue_) {
    return EngineError::FAIL;
  }

  spatQueue_->setOffFocusLeveldB(offFocusLevelDecibels);
  spatQueue_->setFocusWidthDegrees(focusWidth);
  return EngineError::OK;
}

EngineError Audio360FfmpegDecoder::enableFocus(bool enable) {
  if (!ready_ || !spatQueue_) {
    return EngineError::FAIL;
  }

  spatQueue_->enableFocus(enable, true /* follow listenr*/);
  return EngineError::OK;
}

EngineError Audio360FfmpegDecoder::suspendAudioDevice() {
  if (ready_) {
    return engine_->suspend();
  }
  return EngineError::FAIL;
}

EngineError Audio360FfmpegDecoder::startAudioDevice() {
  if (ready_) {
    return engine_->start();
  }
  return EngineError::FAIL;
}

double Audio360FfmpegDecoder::getElapsedTimeMs() {
  if (ready_) {
    const auto ms =
        (spatQueue_->getNumSamplesDequeuedPerChannel() / opusDecoder_->getSampleRate()) * 1000.0;
    return lastTimeStampMs_ + ms;
  }
  return 0.0;
}

void Audio360FfmpegDecoder::setListenerRotation(TBVector forward, TBVector upVector) {
  if (ready_) {
    engine_->setListenerRotation(forward, upVector);
  }
}

void Audio360FfmpegDecoder::setListenerRotation(TBQuat quat) {
  if (ready_) {
    engine_->setListenerRotation(quat);
  }
}

void Audio360FfmpegDecoder::setListenerRotation(float yaw, float pitch, float roll) {
  if (ready_) {
    engine_->setListenerRotation(yaw, pitch, roll);
  }
}

EngineError Audio360FfmpegDecoder::getAudioMix(float* buffer, int numOfSamples) {
  if (ready_) {
    return engine_->getAudioMix(buffer, numOfSamples, 2 /* num channels */);
  }
  return EngineError::FAIL;
}
} // namespace TBE
