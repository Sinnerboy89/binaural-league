#ifndef FBA_TBE_AUDIOENGINE_C_H
#define FBA_TBE_AUDIOENGINE_C_H

/*
 * Copyright (c) 2018-present, Facebook, Inc.
 */

#pragma once

#include <stddef.h>

///
/// A subset of TBE::AudioEngine and related object implemented as C APIs
/// See the original C++ headers for API documentation
///

#ifndef API_EXPORT
#if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#if defined(STATIC_LINKED)
#define API_EXPORT
#else
#define API_EXPORT __declspec(dllexport)
#endif
#else
#if defined(__GNUC__)
#define API_EXPORT __attribute__((visibility("default")))
#else
#define API_EXPORT
#endif
#endif
#endif

extern "C" {

typedef void* TBEAudioEngine;
typedef void* TBEAudioObject;
typedef void (*TBEAudioObjectCallback)(
    float* channelBuffer,
    size_t numSamples,
    size_t numChannels,
    void* userData);

enum TBEChannelMap {
  CHANNEL_MAP_AMBIX_4, /// 4 channels of first order ambiX
  CHANNEL_MAP_AMBIX_9, /// 9 channels of second order ambiX
  CHANNEL_MAP_AMBIX_16, /// 16 channels of third order ambiX
  CHANNEL_MAP_MONO, // Mono audio
  CHANNEL_MAP_STEREO, /// Stereo audio
  CHANNEL_MAP_UNKNOWN, /// Unknown channel map
  CHANNEL_MAP_INVALID, /// Invalid/unknown map. This must always be last.
};

enum class TBEPlayState {
  PLAY_STATE_PLAYING,
  PLAY_STATE_PAUSED,
  PLAY_STATE_STOPPED,
  PLAY_STATE_INVALID,
};

/// @return A negative value on error (corresponding to TBE::EngineError) or 0 on success
API_EXPORT int TBEAudioEngine_createAudioObject(TBEAudioEngine engine, TBEAudioObject* object);
API_EXPORT void TBEAudioEngine_destroyAudioObject(TBEAudioEngine engine, TBEAudioObject* object);

/// @return A negative value on error (corresponding to TBE::EngineError) or 0 on success
API_EXPORT int TBEAudioObject_setAudioBufferCallback(
    TBEAudioObject object,
    TBEAudioObjectCallback callback,
    size_t numChannels,
    TBEChannelMap map,
    void* userData);
API_EXPORT void TBEAudioObject_close(TBEAudioObject object);
API_EXPORT int TBEAudioObject_play(TBEAudioObject object);
API_EXPORT int TBEAudioObject_pause(TBEAudioObject object);
API_EXPORT TBEPlayState TBEAudioObject_getPlayState(TBEAudioObject object);
/// @return A negative value on error (corresponding to TBE::EngineError) or 0 on success
API_EXPORT int TBEAudioObject_setPosition(TBEAudioObject object, float x, float y, float z);
API_EXPORT void TBEAudioObject_getPosition(TBEAudioObject object, float* x, float* y, float* z);
API_EXPORT void TBEAudioObject_shouldSpatialize(TBEAudioObject object, bool shouldSpatialize);
API_EXPORT bool TBEAudioObject_isSpatialized(TBEAudioObject object);
API_EXPORT void TBEAudioObject_setVolume(
    TBEAudioObject object,
    float linearGain,
    float rampTimeMs,
    bool forcePreviousRamp);
API_EXPORT float TBEAudioObject_getVolume(TBEAudioObject object);
}

#endif // FBA_TBE_AUDIOENGINE_C_H
