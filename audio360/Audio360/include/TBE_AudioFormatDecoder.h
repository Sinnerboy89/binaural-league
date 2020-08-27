#ifndef FBA_TBE_AUDIOFORMATDECODER_H
#define FBA_TBE_AUDIOFORMATDECODER_H

#pragma once
#include <stddef.h>
#include <stdint.h>
#include "TBE_AudioEngineDefinitions.h"
#include "TBE_IOStream.h"

namespace TBE {
/// A class for decoding compressed and uncompressed audio formats. The source could be a file or
/// data stream. Supported codecs:
/// 1. Opus stream (opus packets from demuxers, use TBE_CreateAudioFormatDecoderFromHeader)
/// 2. Opus file (*.opus files)
/// 3. WAV files (including broadcast WAV)
/// 4. Native codecs on Android, iOS, macOS and Windows
class AudioFormatDecoder {
 public:
  enum class Info {
    PRE_SKIP /// Some codecs such as opus have a decoding latency. AudioFormatDecoder already
             /// compensates for this.
  };

  virtual ~AudioFormatDecoder(){};

  /// Returns the number of channels in the audio format
  /// @return Number of channels in the audio format
  virtual int32_t getNumOfChannels() const = 0;

  /// Returns the number of total samples, including all channels.
  /// @return Total number of samples, including all channels.
  virtual size_t getNumTotalSamples() const = 0;

  /// Returns the number of samples in a single channel;
  /// @return  Number of samples in a single channel
  virtual size_t getNumSamplesPerChannel() const = 0;

  /// Returns the number of milliseconds in a single channel;
  /// @return  Number of milliseconds in a single channel
  virtual double getMsPerChannel() const = 0;

  /// Returns the current sample position. In some cases this will always return zero if the audio
  /// packets do not have timing information (such as opus).
  /// @return The current sample position.
  virtual size_t getSamplePosition() = 0;

  /// Seek to a sample position. This is only applicable when using an audio file or stream.
  /// @return EngineError::OK on success or corresponding error
  virtual EngineError seekToSample(size_t samplePosition) = 0;

  /// Decode a packet of audio data into pcm float samples. This method must only be used for
  /// in-place processing, such as when you are managing a file stream yourself.
  /// @param data Input audio data
  /// @param dataSize Size of input audio data in bytes
  /// @param bufferOut Audio buffer to write decoded samples into (interleaved for multichannel
  /// audio)
  /// @param numOfSamplesInBuffer Number of total samples in buffer (including all channels for
  /// multichannel audio). The number of samples should not be greater getMaxBufferSizePerChannel()
  /// * getNumOfChannels()
  /// @return Number of samples decoded and written into bufferOut
  virtual size_t
  decode(const char* data, size_t dataSize, float* bufferOut, int32_t numOfSamplesInBuffer) = 0;

  /// Decode an open stream/file into pcm float samples. This method must only be used when
  /// AudioFormatDecoder is also managing the stream (such as a file).
  /// @param bufferOut Audio buffer to write decoded samples into (interleaved for multichannel
  /// audio)
  /// @param numOfSamplesInBuffer Number of total samples in buffer (including all channels for
  /// multichannel audio). The number of samples should not be greater getMaxBufferSizePerChannel()
  /// * getNumOfChannels()
  /// @return Number of samples decoded and written into bufferOut
  virtual size_t decode(float* bufferOut, int32_t numOfSamplesInBuffer) = 0;

  /// Returns the sample rate of the audio format
  /// @return The sample rate of the audio format in Hz.
  virtual float getSampleRate() const = 0;

  /// Returns the output sample rate. The audio would be resampled to this rate if the AudioFormat
  /// allows for it.
  /// @return The output sample rate in Hz.
  virtual float getOutputSampleRate() const = 0;

  /// Returns the number of bits in the audio format. (Eg: 24 bit or 16 bit wav file)
  /// @return The number of bits in the audio format.
  virtual int32_t getNumBits() const = 0;

  /// Returns true if the end of the stream has been reached. This method must only be used when
  /// AudioFormatDecoder is also managing the stream (such as a file).
  /// @return True if the end of
  /// the stream has been reached
  virtual bool endOfStream() = 0;

  /// Returns true if there was an error while decoding. If decode(..) returns 0, it is likely that
  /// either endOfStream() or decoderError() will return true.
  virtual bool decoderError() = 0;

  /// @return The maximum number of samples allocated for each channel in internal buffers.
  virtual int32_t getMaxBufferSizePerChannel() const = 0;

  /// @return The name of audio format (Eg: opus, wav)
  virtual const char* getName() const = 0;

  /// Flushes the internal buffers and resets the state of the decoder. resetToZero must be set to
  /// true if the audio format stream was reset to its initial position (i.e rewound to the start of
  /// the stream).
  virtual void flush(bool resetToZero = false) = 0;

  /// @return Returns information related to the audio format/codec.
  virtual int32_t getInfo(Info info) = 0;

  /// @return ChannelMap if detected, if not ChannelMap::INVALID
  virtual ChannelMap getChannelMap() const = 0;

  /// Set the Java VM for Android. Only required for API level < 21
  /// @vm Valid instance to the Java VM
  API_EXPORT static void setJavaVM(void* vm);

  /// @return the current Java VM if specified using setJavaVM or nullptr
  API_EXPORT static void* getJavaVM();
};
} // namespace TBE

extern "C" {
/// Create an instance of AudioFormatDecoder from header information. The object can be de-allocated
/// with delete.
/// @param decoder A null reference to AudioFormatDecoder
/// @param headerData Header data
/// @param headerDataSize Size of the header data in bytes
/// @return Relevant error or EngineError::OK
API_EXPORT TBE::EngineError TBE_CreateAudioFormatDecoderFromHeader(
    TBE::AudioFormatDecoder*& decoder,
    const char* headerData,
    size_t headerDataSize);

/// Create an instance of AudioFormatDecoder from a file. The object can be de-allocated with
/// delete. Formats currently supported: wav, .tbe.
/// @param decoder A null reference to
/// AudioFormatDecoder
/// @param file Path to file
/// @param maxBufferSizePerChannel Maximum buffer size
/// per channel in samples. This is used to allocate internal buffers and should typically match the
/// size specified in the decode(..) method.
/// @param outputSampleRate Output sample rate. The audio
/// format will be resampled to match this rate, if required. Setting this to 0 would disable
/// resampling and set the output sample rate to match the sample rate of the audio format.
/// @return Relevant error or EngineError::OK
API_EXPORT TBE::EngineError TBE_CreateAudioFormatDecoder(
    TBE::AudioFormatDecoder*& decoder,
    const char* file,
    int maxBufferSizePerChannel,
    float outputSampleRate);

/// Create an instance of AudioFormatDecoder from an IOStream.
API_EXPORT TBE::EngineError TBE_CreateAudioFormatDecoderFromStream(
    TBE::AudioFormatDecoder*& decoder,
    TBE::IOStream* stream,
    bool decoderOwnsStream,
    int maxBufferSizePerChannel,
    float outputSampleRate);
}

#endif // FBA_TBE_AUDIOFORMATDECODER_H
