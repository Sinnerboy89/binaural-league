#ifndef FBA_TBE_AUDIOOBJECT_H
#define FBA_TBE_AUDIOOBJECT_H

/*
 * Copyright (c) 2018-present, Facebook, Inc.
 */

#pragma once

#include "TBE_AudioAssetManager.h"
#include "TBE_AudioEngine.h"

namespace TBE {

typedef void* EffectHandle;

class AudioObject : public SpatDecoderInterface {
 public:
  /// Callback for providing PCM data to the AudioObject from the client rather than a file.
  /// If stereo data is provided, the data must be interleaved
  /// @param channelBuffer The audio channel buffer that the client needs to fill with data
  /// @param numSamples The number of samples the client must fill in the buffer
  /// @param numChannels The number of interleaved channels in the buffer
  /// @param userData pointer to custom user data.
  typedef void (
      *BufferCallback)(float* channelBuffer, size_t numSamples, size_t numChannels, void* userData);

  /// Use this function to set a callback for audio data to be provided by the client
  /// The client must ensure the callback pointer is valid for the lifetime of the AudioObject
  /// If a file has been previously opened with the @ref open() call it will be closed
  /// @param BufferCallback The callback pointer to be called for providing audio
  /// @param numChannels The number of channels of audio data the client wants to provide. 1, 2 and
  /// ambisonic channels are supported.
  /// @param map Channel map, if known. Mostly used to determine the ambisonics format, if known.
  /// @param userData Pointer to custom user data that will be passed to the client on each
  /// invocation
  virtual EngineError setAudioBufferCallback(
      BufferCallback callback,
      size_t numChannels,
      ChannelMap map,
      void* userData) = 0;

  /// Opens an asset for playback. Currently .wav, .opus and native platform codecs are supported
  /// across all platforms. While the asset is opened synchronously, it is loaded into the streaming
  /// buffer asynchronously. An Event::DECODER_INIT event will be dispatched to the event callback
  /// when the streaming buffer is ready for the asset to play. If a callback has been provided with
  /// @ref setAudioBufferCallback, the callback will be unset.
  /// @see  setEventCallback, close
  /// @param nameAndPath Name and path to the file
  /// @return Relevant error or EngineError::OK
  virtual EngineError open(const char* nameAndPath) = 0;

  /// Opens an asset for playback using an asset descriptor. Useful for playing back chunks within a
  /// larger file. Currently .wav, .opus and native platform codecs are supported across all
  /// platforms. While the asset is opened synchronously, it is loaded into the streaming buffer
  /// asynchronously. An Event::DECODER_INIT event will be dispatched to the event callback when the
  /// streaming buffer is ready for the asset to play. If a callback has been provided with @ref
  /// setAudioBufferCallback, the callback will be unset.
  /// @see  setEventCallback, close
  /// @param nameAndPath Name and path to the file
  /// @param ad AssetDescriptor for the file
  /// @param map The channel mapping / spatial audio format of the file
  /// @return Relevant error or EngineError::OK
  virtual EngineError open(const char* nameAndPath, AssetDescriptor ad) = 0;

  /// Opens IOStream object for playback. Currently .wav, .opus and native platform codecs are
  /// supported across all platforms. While the asset is opened synchronously, it is loaded into the
  /// streaming buffer asynchronously. An Event::DECODER_INIT event will be dispatched to the event
  /// callback when the streaming buffer is ready for the asset to play. If a callback has been
  /// provided with @ref setAudioBufferCallback, the callback will be unset.
  /// @see  setEventCallback, close
  /// @param stream An instance of IOStream for audio asset that must be played
  /// @param shouldOwnStream If the lifecycle of the stream must be owned by this object
  /// @return Relevant error or EngineError::OK
  virtual EngineError open(TBE::IOStream* stream, bool shouldOwnStream) = 0;

  /// Opens a sound for playback in cases where a decoder is already available.
  /// This method takes ownership of the decoder, and will destroy it if opening fails,
  /// so make sure not to do anything with the decoder afterward.
  /// This is primarily for use with an AudioAssetManager, which can provide a decoder
  /// for a sound much quicker than the other methods.
  /// While the asset is opened synchronously, it is loaded into the streaming buffer
  /// asynchronously. An Event::DECODER_INIT event will be dispatched to the event callback when the
  /// streaming buffer is ready for the asset to play. If a callback has been provided with @ref
  /// setAudioBufferCallback, the callback will be unset.
  /// @see  setEventCallback, close
  /// @param decoder The decoder to use. Ownership is transferred to this AudioObject.
  /// @return Relevant error or EngineError::OK
  virtual EngineError open(TBE::AudioFormatDecoder* decoder) = 0;

  /// Close an open file/IOStream and release resources
  virtual void close() = 0;

  /// Returns true if a file is open
  virtual bool isOpen() const = 0;

  /// Seek to a specific time in samples
  /// Thread safe.
  /// @param timeInSamples Time in samples to seek to
  /// @return OK if successful or FAIL otherwise
  virtual EngineError seekToSample(size_t timeInSamples) = 0;

  /// Seek to a specific time in milliseconds
  /// Thread safe.
  /// @param timeInMs Time in milliseconds to seek to
  /// @return OK if successful or FAIL otherwise
  virtual EngineError seekToMs(float timeInMs) = 0;

  /// Returns the elapsed playback time in samples
  /// Thread safe.
  virtual size_t getElapsedTimeInSamples() const = 0;

  /// Returns the elapsed playback time in milliseconds
  /// Thread safe.
  virtual double getElapsedTimeInMs() const = 0;

  /// Returns the duration of the asset in samples.
  /// Thread safe.
  virtual size_t getAssetDurationInSamples() const = 0;

  /// Returns the duration of the asset in milliseconds.
  /// Thread safe.
  virtual float getAssetDurationInMs() const = 0;

  /// Set a function to receive events from this object. Currently Event::DECODER_INIT is sent when
  /// the a file is loaded and ready for playback.
  /// @param callback Event callback function
  /// @param userData User data. Can be nullptr
  /// @return Relevant error or EngineError::OK
  virtual EngineError setEventCallback(EventCallback callback, void* userData) = 0;

  /// Set spatialisation on or off.
  /// When spatialisation is enabled, the setPosition(..) method has an effect on how the sound
  /// source is perceived. Default: spatialisation is enabled
  /// @param spatialise When false, the
  /// object is no longer spatialised
  virtual void shouldSpatialise(bool spatialise) = 0;

  /// Returns true if the object is spatialised
  virtual bool isSpatialised() = 0;

  /// Set whether or not you want to override the automatic ranking algorithm with a fixed
  /// spatialization type. Default: false
  /// @param override true to override
  virtual void overrideRanking(bool override) = 0;

  /// This feature is currently unsupported and will return an EngineError.
  /// Switch spatialization type between ambisonics and binaural.
  /// Default: ambisonics
  /// @param spatType ambisonics or binaural
  /// @return Relevant error or EngineError::OK
  virtual EngineError setSpatialisationType(SpatialisationType spatType) = 0;

  /// Returns spatialisation type, either ambisonics or binaural
  /// If nothing has been set in setSpatialisationType, the default is ambisonics
  /// @return one of the SpatializationType options: ambisonics or binaural
  virtual SpatialisationType getSpatialisationType() const = 0;

  /// Set looping on/off. Only applicable to file-backed AudioObjects, does nothing when a callback
  /// is used
  /// @param loop When true, the audio object will loop back to the beginning
  /// @return true
  /// if the object is cabpable of looping (e.g. it is streamed from a file), false if not
  virtual bool enableLooping(bool loop) = 0;

  /// Returns whether the object is looping or not
  virtual bool loopingEnabled() = 0;

  /// Set the distance attenuation mode.
  /// Default is AttenuationMode::LOGARITHMIC
  /// @param AttenuationMode Attenuation mode: logarithmic, linear, disabled
  virtual void setAttenuationMode(AttenuationMode mode) = 0;

  /// @return The current distance attenuation mode
  virtual AttenuationMode getAttenuationMode() const = 0;

  /// Set the attenuation properties, only applicable if the attenuation mode is either set to
  /// logarithmic or linear.
  /// @param AttenuationProps Attenuation properties including minimum distance, maximum distance,
  /// attenuation factor and max distance mute
  virtual void setAttenuationProperties(AttenuationProps props) = 0;

  /// @return The attenuation properties
  virtual AttenuationProps getAttenuationProperties() const = 0;

  /// Enable/disable directionality based filtering. The sound will be filtered based on the
  /// orientation of the sound source with respect to the listener. Default: disabled.
  /// @param enable Toggle directionality
  virtual void setDirectionalityEnabled(bool enable) = 0;

  /// @return True if directionality is enabled
  virtual bool isDirectionalityEnabled() const = 0;

  /// Set the properties for the directional filter. The effect will be audible only if
  /// setDirectionalityEnabled is set to true.
  /// @param props Directional properties
  virtual void setDirectionalProperties(DirectionalProps props) = 0;

  /// @return Get the current directional properties
  virtual DirectionalProps getDirectionalProperties() const = 0;

  /// Set the pitch multiplier. E.g a value of 2 will get the sound to playback at twice the speed
  /// and twice the pitch. This feature only works for files/streams and will not have any effect if
  /// audio is provided via the buffer callback.
  /// @param pitch Pitch multiplier, between a range of 0.001 and 4
  virtual void setPitch(float pitch) = 0;

  /// Returns the current pitch
  /// @return the pitch (as specified in setPitch())
  virtual float getPitch() const = 0;

  /// Create a new effect that will be inserted post-gain. There is no limit to the number of
  /// effects that can be created. Each new effect will be added to the end of the existing chain.
  /// Any effects that aren't explicitly destroyed will be automatically destroyed when this object
  /// is deleted.
  /// NOTE: All effects are created with bypass enabled.
  /// NOTE: Only available if FBA is enabled
  /// @param type The type of effect
  /// @return The handle to the effect.
  virtual EffectHandle createEffect(EffectType type) = 0;

  /// Destroy and remove an existing effect from the chain. Any effects that aren't explicitly
  /// destroyed will be automatically destroyed when this object is deleted. NOTE: Only available if
  /// FBA is enabled
  /// @param handle The handle to the effect.
  virtual void destroyEffect(EffectHandle handle) = 0;

  /// Get the current effect type for a handle
  /// NOTE: Only available if FBA is enabled
  /// @param handle The handle to the effect.
  /// @return the effect type
  virtual EffectType getEffectTypeForHandle(EffectHandle handle) = 0;

  /// Set a new effect type for an existing effect
  /// NOTE: Only available if FBA is enabled
  /// @param handle The handle to the effect.
  /// @param type The new type of effect
  /// @return Relevant error or EngineError::OK
  virtual EngineError setEffectType(EffectHandle handle, EffectType type) = 0;

  /// Toggle bypass state. An effect is in a bypassed state when it is first created
  /// NOTE: Only available if FBA is enabled
  /// @param handle The handle to the effect.
  /// @param bypass Bypassed if true
  /// @return Relevant error or EngineError::OK
  virtual EngineError bypassEffect(EffectHandle handle, bool bypass) = 0;

  /// NOTE: Only available if FBA is enabled
  /// @param handle The handle to the effect.
  /// @return True if effect is bypassed
  virtual bool isEffectBypassed(EffectHandle handle) = 0;

  /// Set a parameter for an effect
  /// NOTE: Only available if FBA is enabled
  /// @param handle The handle to the effect.
  /// @param param The parameter
  /// @param value The value
  /// @return Relevant error or EngineError::OK
  virtual EngineError setEffectParam(EffectHandle handle, EffectParam param, float value) = 0;

  /// NOTE: Only available if FBA is enabled
  /// @param handle The handle to the effect.
  /// @param param The parameter
  /// @return The value of a parameter
  virtual float getEffectParam(EffectHandle handle, EffectParam param) = 0;

  /// NOTE: Only available if FBA is enabled
  /// @return number of effects active
  virtual size_t getNumberOfEffects() const = 0;

  /// NOTE: Only available if FBA is enabled
  /// @return the effect handle for an index where index is >= 0 and < getNumberOfEffects()
  virtual EffectHandle getEffect(size_t effectIndex) = 0;

  /// NOTE: Only available if FBA is enabled
  /// Get the output bus
  virtual Bus getOutputBus() = 0;

 protected:
  virtual ~AudioObject() {}
};
} // namespace TBE

#endif // FBA_TBE_AUDIOOBJECT_H
