#ifndef FBA_TBE_AUDIOASSETMANAGER_H
#define FBA_TBE_AUDIOASSETMANAGER_H

/*
 * Copyright (c) 2018-present, Facebook, Inc.
 */

#pragma once

#include <string>
#include "TBE_AudioEngineDefinitions.h"
#include "TBE_AudioFormat.h"
#include "TBE_AudioFormatDecoder.h"
#include "TBE_IOStream.h"

namespace TBE {
static size_t const uninitializedHandle = SIZE_MAX;
typedef struct {
  size_t index{uninitializedHandle};
  size_t id{uninitializedHandle};
} AudioAssetHandle;
const AudioAssetHandle InvalidAudioAssetHandle;

///
/// AudioAssetManager is to open file or allocate memory and load audio data
/// from file to memory only once to read audio data efficiently.
/// It uses the audio file name and path to create a unique handle from which
/// new IOStream objects can be created.
/// There are three access modes to the data:
///   FILE: access data from file
///   MEMORY: load data from file to memory and access data from memory
///   DECODED_MEMORY: load data from file and then decode the compressed data
///                   before storing to memory, and access decoded data from
///                   memory.
///
class AudioAssetManager {
 public:
  virtual ~AudioAssetManager() {}

  ///
  /// loadAudio does nothing and only returns file handle when AssetAccessMode is
  /// FILE.
  /// It performs the audio data copy from file to memory when AssetAccessMode is
  /// other than FILE.
  /// If the file is not loaded yet, this function will allocate memory,
  /// copy the audio data from the file to this memory, and then return a
  /// handle of the memory.
  /// If the file is already loaded, it only returns the handle of the
  /// memory that is allocated to store the data. Duplicate files are detected
  /// by the path.
  /// When AssetAccessMode is set to DECODED_MEMORY, the audio data will be decoded
  /// before storing to the memory.
  /// @param handle AudioAssetHandle that can be used to access the data in the memory
  /// @param fileNameAndPath Input audio file name as well as its path
  /// @param ad AssetDescriptor to specify the offset and size for the file access,
  ///        both of which can be zero.
  /// @param mode Audio data access mode
  /// @return error message
  ///
  virtual EngineError loadAudio(
      AudioAssetHandle& handle,
      const char* fileNameAndPath,
      AssetDescriptor ad,
      AssetAccessMode mode) = 0;

  ///
  /// This function returns a new IOStream that can be used to read audio
  /// data in the memory or file. This IOStream object can be used with
  /// AudioObject::open and similar methods.
  /// @param handle handle to access data in the memory/file.
  /// @return newly created IOStream for following data streaming
  ///
  virtual IOStream* getNewStream(AudioAssetHandle& handle) = 0;

  /// This function returns a new AudioFormatDecoder that can be used to read audio
  /// data in the memory or file. This object can be used with AudioObject::open and similar
  /// methods.
  /// @param handle handle to access data in the memory/file.
  /// @param bufferSize the buffer size with which to initialize the decoder.
  /// This is usually the engine's buffer size/numSamples.
  /// @param sampleRate The sampleRate with which to initialize the decoder.
  /// This is usually the engine's sample rate.
  /// @return newly created IOStream for following data streaming
  virtual TBE::AudioFormatDecoder*
  getNewDecoder(TBE::AudioAssetHandle& handle, int32_t bufferSize, float sampleRate) = 0;

  ///
  /// This function does clean up of the memory and the map from file name to the handle.
  /// @param handle handle to access data in the memory
  /// @return true if the audio asset is unloaded synchronously, or false if the asset
  /// will be freed unsynchronously later.
  ///
  virtual bool unloadAudio(AudioAssetHandle& handle) = 0;

  ///
  /// This function returns audio data access mode.
  /// @param handle handle to access data in the memory
  /// @return audio data access mode
  ///
  virtual AssetAccessMode getMode(AudioAssetHandle& handle) = 0;

  ///
  /// This function returns the format of the audio data from the file.
  /// @param handle handle to access data in the memory
  /// @return audio format string of the audio data in the memory
  ///
  virtual const char* getFormat(AudioAssetHandle& handle) = 0;

  /// Returns the total size in bytes of assets loaded in memory (via AssetAccessMode::MEMORY and
  /// AssetAccessMode::DECODED_MEMORY)
  /// @return The total size in bytes of assets loaded in memory
  virtual size_t getBytesInMemory() = 0;
};

} // namespace TBE

extern "C" {
/// Create an object to load audio data to memory/file. It is preferable to use the default asset
/// manager in the audio engine (audioEngine->getAudioAssetManager()). This method should only be
/// used if you want to set your own instance of the audio asset manager.
/// @param engine Pointer to the asset manager instance (must be null)
/// @return Relevant error or EngineError::OK
API_EXPORT TBE::EngineError TBE_CreateAudioAssetManager(TBE::AudioAssetManager*& assetManager);
}

#endif // FBA_TBE_AUDIOASSETMANAGER_H
