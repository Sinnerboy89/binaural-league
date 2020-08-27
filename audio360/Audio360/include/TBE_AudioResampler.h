#ifndef FBA_TBE_AUDIORESAMPLER_H
#define FBA_TBE_AUDIORESAMPLER_H

/*
 * Copyright (c) 2018-present, Facebook, Inc.
 */

#pragma once

#include "TBE_AudioEngineDefinitions.h"

namespace TBE {
/// Audio resampler
class AudioResampler {
 public:
  enum class Quality { OPTIMAL, FAST };

  virtual ~AudioResampler() {}

  /// Resample a buffer. The input and output buffers must not overlap.
  /// @param input Input interleaved buffer
  /// @param totalInputSamples Total number of input samples
  /// @param output Resampled interleaved buffer
  /// @param totalOutputSamples Total number of samples available in the output buffer
  /// @param endOfStream Should be set to true if the input buffer is the last in the stream. This
  /// will cause the resampler to flush any pending samples.
  /// @return Total number of resampled samples
  virtual size_t process(
      const float* input,
      size_t totalInputSamples,
      float* output,
      size_t totalOutputSamples,
      bool endOfStream) = 0;

  /// @return The number of channels specified on initialisation
  virtual int getNumChannels() const = 0;

  /// @return The input sample rate specified on initialisation
  virtual float getInputSampleRate() const = 0;

  /// @return The output sample rate specified on initialisation
  virtual float getOutputSampleRate() const = 0;

  /// @return The quality specified on construction
  virtual Quality getQuality() const = 0;

  /// Set the resampling ratio (output sample rate / input sample rate)
  /// @param resamplingRatio Ratio used while resampling
  virtual void setRatio(double resamplingRatio) = 0;

  /// @return The current resampling ratio
  virtual double getRatio() const = 0;

  /// Flushes and resets the internal state of the resampler. Does not free or allocate any memory
  virtual void reset() = 0;
};
} // namespace TBE

extern "C" {
/// Create an object to resample a mono or multi-channel audio stream.
/// @param engine Pointer to the resampler instance (must be null)
/// @param numChannels Number of audio channels
/// @param inputSampleRate Input sample rate in Hz
/// @param outputSampleRate Output sample rate in Hz
/// @param maxBufferSizeSamples max size in samples per channel for the resampler
/// @param quality Quality/speed as specified in the Quality enum
/// @return Relevant error or EngineError::OK
API_EXPORT TBE::EngineError TBE_CreateAudioResampler(
    TBE::AudioResampler*& resampler,
    unsigned numChannels,
    float inputSampleRate,
    float outputSampleRate,
    size_t maxBufferSizeSamples,
    TBE::AudioResampler::Quality quality = TBE::AudioResampler::Quality::FAST);
}

#endif // FBA_TBE_AUDIORESAMPLER_H
