#ifndef FBA_TBE_AUDIOFORMAT_H
#define FBA_TBE_AUDIOFORMAT_H

/*
 * Copyright (c) 2018-present, Facebook, Inc.
 */

#pragma once

#include <cstddef>
#include "TBE_AudioEngineDefinitions.h"
#include "TBE_IOStream.h"

namespace TBE {
enum class AudioFormat {
  OPUS_FILE, /// Opus-ogg files, typically with a .opus extension
  WAV, /// BWAV/WAV files
  INVALID,
};

/// The quality of the audio format. The mapping of these options will vary based on the codec and
/// format. These options might not be applicable to some formats (such was WAV). Use
/// AudioFormatEncoder::getBitRateForQualityIndex and AudioFormatEncoder::getIndexForQuality to
/// query the actual bit rate for a format.
enum class AudioFormatQuality { LOW, MEDIUM, HIGH, VERY_HIGH };

/// An object for encoding raw PCM audio to an audio format. This object should be used and accessed
/// on a single thread.
class AudioFormatEncoder {
 public:
  virtual ~AudioFormatEncoder(){};

  /// Encode mono or multichannel PCM audio (float 32 bit) to the AudioFormat that was chosen on
  /// construction. The encoded data will be written to the IOStream object provided on
  /// construction.
  /// @param inputInterleavedAudio Input buffer of interleaved audio
  /// @param totalNumSamples Total number of samples (including all channels) in the input buffer.
  /// This value should be less than equal to the maxBufferSize * numChannels specified on
  /// construction.
  /// @param endOfStream Specify if the stream has ended and if no more audio is expected
  /// @return EngineError::OK if successful or corresponding error
  virtual size_t
  encode(const float* inputInterleavedAudio, size_t totalNumSamples, bool endOfStream) = 0;

  /// @return The AudioFormat specified on construction
  virtual AudioFormat getAudioFormat() const = 0;

  /// Get the quality index for the current AudioFormat, if applicable. The quality index is a range
  /// between 1 and 10 specified on construction.
  /// @param qualityIndex Return value of index
  /// @return EngineError::OK if successful or corresponding error
  virtual EngineError getQualityIndex(int& qualityIndex) = 0;

  /// @return The current bit rate. The bit rate will depend on the quality index provided on
  /// initialisation
  virtual EngineError getBitRate(int& bitRate) = 0;

  /// @return The output sample rate
  virtual float getOutputSampleRate() const = 0;

  /// @return The number of channels
  virtual int getNumOfChannels() = 0;

  /// @return The maximum buffer size specified on initialisation
  virtual size_t getMaxBufferSize() const = 0;

  /// Get the bit rate for a quality index and AudioFormat
  /// @param format The AudioFormat
  /// @param qualityIndex A value between 1 and 10
  /// @param bitRate The output bit rate for the specified index
  /// @return EngineError::OK or EngineError::NOT_SUPPORTED (for uncompressed formats)
  API_EXPORT static EngineError
  getBitRateForQualityIndex(AudioFormat format, int qualityIndex, int& bitRate);

  /// Get the index for a specified AudioFormat and AudioFormatQuality
  /// @param format The AudioFormat
  /// @param quality The AudioFormatQuality
  /// @param qualityIndex The output quality index that is mapped to the specified
  /// AudioFormatQuality
  /// @return EngineError::OK or EngineError::NOT_SUPPORTED (for uncompressed formats)
  API_EXPORT static EngineError
  getIndexForQuality(AudioFormat format, AudioFormatQuality quality, int& qualityIndex);
};
} // namespace TBE

extern "C" {
/// Create an AudioFormatEncoder object to encode PCM audio to an AudioFormat.
/// @param encoder Pointer to a null AudioFormatEncoder instance
/// @param outputStream A valid IOStream object to use to write the encoded audio. The
/// AudioFormatEncoder will not own this object. The user must ensure that the IOStream object is
/// always valid during use and deleted after.
/// @param format The target AudioFormat
/// @param inputSampleRate The sample rate of the input PCM data
/// @param outputSampleRate The target output sample rate. Resampling may be applied if the input
/// and output sample rates do not match. Some formats might ignore this value.
/// @param maxBufferSize The maximum buffer size expected per channel (when calling encode())
/// @param numChannels The number of channels
/// @param quality Desired encoding quality. May be ignored for some formats.
/// @return EngineError::OK or corresponding error.
API_EXPORT TBE::EngineError TBE_CreateAudioFormatEncoder(
    TBE::AudioFormatEncoder*& encoder,
    TBE::IOStream* outputStream,
    TBE::AudioFormat format,
    float inputSampleRate,
    float outputSampleRate,
    size_t maxBufferSize,
    int numChannels,
    TBE::AudioFormatQuality quality);

/// Create an AudioFormatEncoder object to encode PCM audio to an AudioFormat.
/// @param encoder Pointer to a null AudioFormatEncoder instance
/// @param outputStream A valid IOStream object to use to write the encoded audio. The
/// AudioFormatEncoder will not own this object. The user must ensure that the IOStream object is
/// always valid during use and deleted after.
/// @param format The target AudioFormat
/// @param inputSampleRate The sample rate of the input PCM data
/// @param outputSampleRate The target output sample rate. Resampling may be applied if the input
/// and output sample rates do not match. Some formats might ignore this value.
/// @param maxBufferSize The maximum buffer size expected per channel (when calling encode())
/// @param numChannels The number of channels
/// @param qualityIndex Desired encoding quality index -- a value between 1 and 10, with 10 being
/// the highest supported audio quality. See AudioFormatEncoder::getBitRateForQualityIndex() to
/// query the bit rate associated with an index. May be ignored for some formats.
/// @return EngineError::OK or corresponding error.
API_EXPORT TBE::EngineError TBE_CreateAudioFormatEncoderWithIndex(
    TBE::AudioFormatEncoder*& encoder,
    TBE::IOStream* outputStream,
    TBE::AudioFormat format,
    float inputSampleRate,
    float outputSampleRate,
    size_t maxBufferSize,
    int numChannels,
    int qualityIndex);
}

#endif // FBA_TBE_AUDIOFORMAT_H
