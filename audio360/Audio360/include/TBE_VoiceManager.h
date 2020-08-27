#ifndef FBA_TBE_VOICEMANAGER_H
#define FBA_TBE_VOICEMANAGER_H

/*
 * Copyright (c) 2019-present, Facebook, Inc.
 */

#pragma once

#include "TBE_AudioAssetManager.h"
#include "TBE_AudioEngine.h"

namespace TBE {
typedef uintptr_t VoiceHandle;
const VoiceHandle InvalidVoiceHandle = 0;

const size_t kMaxTotalVoices = 1024;

enum class VoiceMode { Physical, Virtual, Invalid };

enum class VoiceParam {
  // Transport
  Loop = 0,

  // Sound params
  Volume,
  VolumeRampMs,
  Pitch,

  // Spatial params
  Spatialise,
  AttenuationMode,
  CustomAttenuation,
  AttenuationProps_MinDistance,
  AttenuationProps_MaxDistance,
  AttenuationProps_Factor,
  AttenuationProps_MaxDistanceMute,
  DirectionalityEnabled,
  DirectionalProps_EffectLevel,
  DirectionalProps_ConeArea,
  Position_X,
  Position_Y,
  Position_Z,
  Rotation_X,
  Rotation_Y,
  Rotation_Z,
  Rotation_W,

  // Voice management params
  Priority,

  Num_Params
};

struct VoiceParamDescription {
  float min{0};
  float max{0};
  float defaultValue{0};
  float stepSize{0};
};

enum class VoiceManagerEvent {
  Invalid = 0,

  // Voice-specific events
  VoiceCreated,
  VoiceOpened,
  VoiceDestroyed,
  VoicePlayStateChanged,
  VoiceFinishedPlaying,
  VoiceModeChanged,
  VoiceError
};

typedef void (
    *VoiceManagerEventCb)(VoiceManagerEvent event, VoiceHandle voiceHandle, void* userData);

class VoiceManager {
 public:
  virtual ~VoiceManager() {}

  // @returns the maximum number of physical voices
  virtual size_t getMaxPhysicalVoices() const = 0;

  // @returns the maximum number of virtual voices
  virtual size_t getMaxVirtualVoices() const = 0;

  // @returns the maximum number of voices of all kinds
  virtual size_t getMaxTotalVoices() const = 0;

  // @returns the current number of physical voices
  virtual size_t getNumPhysicalVoices() const = 0;

  // @returns the current number of virtual voices
  virtual size_t getNumVirtualVoices() const = 0;

  // @returns the current number of voices of all kinds
  virtual size_t getNumTotalVoices() const = 0;

  /// Get a voice (if available) and open a sound for playing.
  /// Creating the voice is synchronous, and the voice can be used immediately.
  /// Opening the file to play is asynchronous, and further operations will be enqueued after it is
  /// open.
  /// @param voiceHandle - A handle to be filled in with voice info
  /// @param assetHandle - The audio asset to open from an AudioAssetManager
  /// @returns PENDING or appropriate error if creating the voice failed
  virtual EngineError openVoice(VoiceHandle& voiceHandle, AudioAssetHandle assetHandle) = 0;

  /// Close the voice that corresponds to the provided handler.
  /// This will also stop the voice immediately if it is currently playing.
  /// @param voiceHandle - The handle to the voice that should be closed
  /// @returns PENDING or appropriate error if closing failed
  virtual EngineError closeVoice(VoiceHandle voiceHandle) = 0;

  /// Check if a voice exists and is open
  /// @param voiceHandle - The handle to the voice
  /// @returns true if the voice exists and is open, false otherwise
  virtual bool voiceIsOpen(VoiceHandle voiceHandle) = 0;

  /*
   * Voice-specific controls
   */

  /*
   * Transport
   */
  /// Play a voice.
  /// @param voiceHandle - A handle to a voice
  /// @param delayMs - milliseconds to wait before changing the play state
  /// @param fadeTimeMs - milliseconds to fade the sound
  /// @returns PENDING or appropriate error on failure
  virtual EngineError play(VoiceHandle voiceHandle, float delayMs, float fadeTimeMs) = 0;

  /// Pause a voice.
  /// @param voiceHandle - A handle to a voice
  /// @param delayMs - milliseconds to wait before changing the play state
  /// @param fadeTimeMs - milliseconds to fade the sound
  /// @returns PENDING or appropriate error on failure
  virtual EngineError pause(VoiceHandle voiceHandle, float delayMs, float fadeTimeMs) = 0;

  /// Stop a voice (and rewind).
  /// @param voiceHandle - A handle to a voice
  /// @param delayMs - milliseconds to wait before changing the play state
  /// @param fadeTimeMs - milliseconds to fade the sound
  /// @returns PENDING or appropriate error on failure
  virtual EngineError stop(VoiceHandle voiceHandle, float delayMs, float fadeTimeMs) = 0;

  /// Get a voice's current play state.
  /// NOTE: if there are scheduled changes in play state, this will still report the current state.
  /// @param voiceHandle - A handle to a voice
  /// @param playState - The voice's current play state (invalid on error)
  /// @returns OK or appropriate error on failure
  virtual EngineError getPlayState(VoiceHandle voiceHandle, PlayState& playState) = 0;

  /// Seek a voice to an absolute position in milliseconds
  /// @param voiceHandle - A handle to a voice
  /// @param posMs - the seek position, in milliseconds
  /// @returns PENDING or appropriate error if seeking failed
  virtual EngineError seekMs(VoiceHandle voiceHandle, float posMs) = 0;

  /// Get the current position in a voice in milliseconds
  /// @param voiceHandle - A handle to a voice
  /// @param timeMs - The current position in milliseconds
  /// @returns OK or appropriate error on failure
  virtual EngineError getElapsedTimeMs(VoiceHandle voiceHandle, float& timeMs) = 0;

  /// Get the duration of a voice in milliseconds
  /// @param voiceHandle - A handle to a voice
  /// @param timeMs - The duration in milliseconds
  /// @returns OK or appropriate error on failure
  virtual EngineError getDurationMs(VoiceHandle voiceHandle, float& timeMs) = 0;

  /*
   * Parameters
   */
  /// Set a parameter of a voice
  /// @param voiceHandle - A handle to a voice
  /// @param param - The param key (one of VoiceParam enum)
  /// @param value - The value of the param to set
  /// @returns OK or appropriate error on failure
  virtual EngineError setParam(VoiceHandle voiceHandle, VoiceParam param, float value) = 0;

  /// Get a parameter of a voice
  /// @param voiceHandle - A handle to a voice
  /// @param param - The param key (one of VoiceParam enum)
  /// @param value - A reference to a float to store the param value
  /// @returns OK or appropriate error on failure
  virtual EngineError getParam(VoiceHandle voiceHandle, VoiceParam param, float& value) = 0;

  /// Get info about a param, such as the range and default value
  /// @param param - The param key (one of VoiceParam enum)
  /// @param description - The description to be filled in
  /// @returns OK or error if param is not found
  virtual EngineError getParamDescription(VoiceParam param, VoiceParamDescription& description) = 0;

  /*
   * Buses
   */
  /// Set the output bus for a voice.
  /// @param voiceHandle - A handle to a voice
  /// @param bus - The bus to which this voice should be connected
  /// @returns PENDING or appropriate error
  virtual EngineError setBus(VoiceHandle voiceHandle, Bus bus) = 0;

  /// Get the output bus for a voice.
  /// @param voiceHandle - A handle to a voice
  /// @param bus - The output bus
  /// @returns OK or appropriate error
  virtual EngineError getBus(VoiceHandle voiceHandle, Bus& bus) = 0;

  /*
   * Virtualisation
   */
  /// Get the mode of a voice (Physical, Virtual, etc)
  /// @param voiceHandle - A handle to a voice
  /// @param voiceMode - The mode of the voice (one of VoiceMode enum)
  /// @returns OK or appropriate error
  virtual EngineError getVoiceMode(VoiceHandle voiceHandle, VoiceMode& mode) = 0;

  /*
   * Events
   */
  /// Set a function to receive events from the voice manager
  /// @param callback - Event callback function
  /// @param userData - User data. Can be nullptr
  /// @returns OK or appropriate error
  virtual EngineError setEventCallback(VoiceManagerEventCb callback, void* userData) = 0;
};
} // namespace TBE

#endif // FBA_TBE_VOICEMANAGER_H
