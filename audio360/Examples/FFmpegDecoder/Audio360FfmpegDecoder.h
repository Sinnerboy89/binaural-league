/*
 * Copyright (c) 2018-present, Facebook, Inc.
 */

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include "TBE_AudioEngine.h"
#include "utils/LibFfmpeg.h"

namespace TBE {
/// An example for using libffmpeg to decode and spatialise the FB360 audio format from a mkv/webm
/// file
class Audio360FfmpegDecoder {
 public:
  enum class Status {
    DECODER_ERROR = 0, // ERROR - seems to cause issues (reserved macro perhaps?)
    OK,
    END_OF_STREAM
  };

  /// Will throw an exception if any of the required ffmpeg libraries cannot be found
  /// \param ffmpegLibPath Path to directory containing ffmpeg libraries
  Audio360FfmpegDecoder(const std::string& ffmpegLibPath);
  ~Audio360FfmpegDecoder();

  /// Open an mkv/webm file using the FB360 audio format.
  /// \param file Path to mkv/webm file
  /// \param useAudioDevice Playback audio directly through the default audio device. If false,
  /// getAudioMix will need to be called instead. \param map Channel map for the audio, typically
  /// TBE_8_2 (for 8 channels spatial + 2 channels head-locked \return Status::OK on success or
  /// Status::Error
  Status open(const std::string& file, bool useAudioDevice, ChannelMap map = ChannelMap::TBE_8_2);

  /// Close an open file
  void close();

  /// \return True if a file is open and ready
  bool ready() const;

  /// Must be called in a loop to read the file, demux and enqueue for spatialisation.
  /// \return Status::OK if the decode loop was successful and can continue in a loop.
  /// Status::END_OF_STREAM if the end of the file was reached. Status::ERROR if there was an error.
  Status decode();

  /// Begin playback of decoded and enqueued data.
  /// \return EngineError::OK on success, or corresponding error
  EngineError play();

  /// Pause playback of decoded and enqueued data
  /// \return EngineError::OK on success, or corresponding error
  EngineError pause();

  /// Set focus params in the spatdecoder queue
  /// Ensure enableFocus(..) is called for the effect to be audible
  /// \return EngineError::OK on success, or corresponding error
  EngineError setFocusProperties(float offFocusLevelDecibels, float focusWidth);

  /// Enable or disable focus
  EngineError enableFocus(bool enable);

  /// Seek the file to a time value in milliseconds
  bool seek(double milliseconds);

  /// \return The elapsed playback time in milliseconds
  double getElapsedTimeMs();

  /// If configured to use the audio device, this will start the audio device.
  /// \return EngineError::OK on success, or corresponding error
  EngineError startAudioDevice();

  /// Suspends the audio device if running (useful if the application is in the background or
  /// paused) \return EngineError::OK on success, or corresponding error
  EngineError suspendAudioDevice();

  /// If open() is called with the audio device disabled, this method can be used to return mixed
  /// spatialised buffers within your app's audio callback.
  /// \b WARNING: calling this method when an audio device is active can result in undefined
  /// behaviour. \param buffer Pointer to existing interleaved float buffer \param numOfSamples In
  /// most cases this would the buffer size * 2 (since it is stereo) \return Relevant error or
  /// EngineError::OK
  EngineError getAudioMix(float* buffer, int numOfSamples);

  /// Set the orientation of the listener through direction vectors.
  /// \param forward Forward vector of the listener
  /// \param upVector Up vector of the listener
  /// \see TBVector for the co-ordinate system used by the audio engine.
  /// \see setListenerRotation(TBQuat quat)
  void setListenerRotation(TBVector forward, TBVector upVector);

  /// Set the orientation of the listener through a quaternion
  /// \param quat Listener quaternion
  /// \see TBVector for the co-ordinate system used by the audio engine.
  /// \see setListenerRotation(TBVector forward, TBVector upVector)
  void setListenerRotation(TBQuat quat);

  /// Set the listener rotation in degrees.
  /// \param yaw Range from -180 to 180 (negative is left)
  /// \param pitch Range from 90 to -90 (positive is up)
  /// \param roll Range from -180 to 180 (negative is left)
  void setListenerRotation(float yaw, float pitch, float roll);

 private:
  const LibFfmpeg ffmpeg_;
  AVFormatContext* context_{nullptr};

  bool ready_{false};
  int audioStreamId_{-1};
  double audioStreamTimeBaseMs_{0.0};
  std::atomic<double> lastTimeStampMs_{0.0};
  std::atomic<bool> didSeek_{true};

  float newSecondsToSeekTo_ = -1;
  std::atomic<bool> shouldSeek_{false};

  AudioEngine* engine_{nullptr};
  SpatDecoderQueue* spatQueue_{nullptr};
  AudioFormatDecoder* opusDecoder_{nullptr};
  ChannelMap channelMap_{ChannelMap::TBE_8_2};

  std::unique_ptr<float[]> pcmBuffer_;
  int pcmBufferSize_{0};

  bool initialiseOpusDecoder(AVFormatContext* context);
  bool initialiseAudio360Engine(bool useAudioDevice);
  Status enqueueNextPacket();
};

// static function to determine if a given video was encoded with our encoder
static inline bool checkVideoIsNative(char const* ffmpegPath, char const* videoFilePath) {
  // MKV found, checking (metadata) to see if it one of ours
  size_t len = strlen(videoFilePath);
  if (len >= 4 && strcmp(videoFilePath + len - 4, ".mkv") == 0) {
    LibFfmpeg ffmpeg(ffmpegPath);
    ffmpeg.av_register_all();

    AVFormatContext* formatCtx_ = nullptr;
    int result = ffmpeg.avformat_open_input(&formatCtx_, videoFilePath, nullptr, nullptr);

    if (result != 0)
      return false;

    AVDictionaryEntry* tag = NULL;
    tag = ffmpeg.av_dict_get(formatCtx_->metadata, "FB360", tag, AV_DICT_IGNORE_SUFFIX);

    if (tag) {
      return true;
    }
  }

  return false;
}
} // namespace TBE
