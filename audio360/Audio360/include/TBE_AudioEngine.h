#ifndef FBA_TBE_AUDIOENGINE_H
#define FBA_TBE_AUDIOENGINE_H

/*
 * Copyright (c) 2018-present, Facebook, Inc.
 */

#pragma once

#include "TBE_AudioEngineDefinitions.h"
#include "TBE_AudioFormatDecoder.h"
#include "TBE_IOStream.h"
#include "TBE_Quat.hh"
#include "TBE_Vector.hh"

namespace TBE {
class AudioEngine;
typedef void* Bus;
class SpatDecoderQueue;
class SpatDecoderFile;
class AudioObject;
class SpeakersVirtualizer;
class EventTransport;
class VoiceManager;

/// Controls the global state of the engine: audio device setup, object pools for spatialisation
/// objects and listener properties. The AudioEngine must be initialised first and destroyed last.
class AudioEngine {
 public:
  /// Callback type used for interleaved audio
  /// @param interleavedAudio - a pointer to an interleaved buffer of audio samples
  /// @param numChannles - number of audio channels
  /// @param numSamplesPerChannel - number of samples for each channel
  /// @param userData - user provided data pointer
  typedef void (*AudioInterleavedCb)(
      float* interleavedAudio,
      size_t numChannels,
      size_t numSamplesPerChannel,
      void* userData);

  /// Callback type used for deinterleaved audio
  /// @param deinterleavedAudio - a pointer to an array of buffers holding audio samples
  /// @param numChannles - number of audio channels
  /// @param numSamplesPerChannel - number of samples for each channel
  /// @param userData - user provided data pointer
  typedef void (*AudioDeInterleavedCb)(
      float** deinterleavedAudio,
      size_t numChannels,
      size_t numSamplesPerChannel,
      void* userData);

  /// Callback type for getting the interleaved audio mix right before it is written to the device
  /// buffer
  typedef AudioInterleavedCb AudioMixCallback;

  /// Callback type for getting the deinterleaved audio mix right before it is written to the device
  /// buffer
  typedef AudioDeInterleavedCb AudioMixDeinterleaved;

  virtual ~AudioEngine() {}

  /// Start the audio device and all audio processing.
  /// start() and suspend() can be used to pause all processing when the app is in the background.
  /// This has no effect if the audio device is disabled.
  /// @return Relevant error or EngineError::OK
  virtual EngineError start() = 0;

  /// Suspend all audio processing.
  /// This has no effect if the audio device is disabled.
  /// @return Relevant error or EngineError::OK
  virtual EngineError suspend() = 0;

  /// Set how many objects should be rendered binaurally
  /// Defaults to 0
  /// @param numBinaural number of objects
  virtual EngineError setNumBinaural(int numBinaural) = 0;

  /// Returns how many objects should be rendered binaurally
  virtual int getNumBinaural() = 0;

  /// Set the orientation of the listener through direction vectors (using Unity's left-handed
  /// coordinate convention). If experimental.fba is enabled, update() must be called for the
  /// transform to be updated.
  /// @param forwardVector Forward vector of the listene
  /// @param upVector Up vector of the listener
  /// @see TBVector for the co-ordinate system used by the audio engine.
  /// @see setListenerRotation(TBQuat quat)
  virtual void setListenerRotation(TBVector forwardVector, TBVector upVector) = 0;

  /// Set the orientation of the listener through a quaternion (using Unity's left-handed coordinate
  /// convention). If experimental.fba is enabled, update() must be called for the transform to be
  /// updated.
  /// @param quat Listener quaternion
  /// @see TBVector for the co-ordinate system used by the audio engine.
  /// @see setListenerRotation(TBVector forward, TBVector upVector)
  virtual void setListenerRotation(TBQuat quat) = 0;

  /// !!! It is strongly recommended that you use one of the other functions to set listener
  /// rotation - set rotation using forward/up vectors or a quaternion in order to avoid gimbal lock
  /// problems. Set the listener rotation in degrees. If experimental.fba is enabled, update() must
  /// be called for the transform to be updated.
  /// @param yaw Range from -180 to 180 (negative is left)
  /// @param pitch Range from -180 to 180 (positive is up)
  /// @param roll Range from -180 to 180
  /// (negative is left)
  virtual void setListenerRotation(float yaw, float pitch, float roll) = 0;

  /// Set the position of the listener in space. If experimental.fba is enabled, update() must be
  /// called for the transform to be updated.
  /// @param position of the listener.
  /// @see TBVector for the co-ordinate system used by the audio engine.
  virtual void setListenerPosition(TBVector position) = 0;

  /// @return the position of the listener
  /// @see TBVector for the co-ordinate system used by the audio engine.
  virtual TBVector getListenerPosition() const = 0;

  /// @return the rotation of the listener as a quaternion.
  virtual TBQuat getListenerRotation() const = 0;

  /// @return The listener's forward vector
  virtual TBVector getListenerForward() const = 0;

  /// @return The listener's up vector
  virtual TBVector getListenerUp() const = 0;

  /// Set the scale of the listener. This is a scalar value applied to the position of all objects.
  /// Larger the scale, smaller the objects and vice-versa.  If experimental.fba is enabled,
  /// update() must be called for the transform to be updated.
  virtual void setListenerScale(float scale) = 0;

  /// @return The current scale of the listener
  virtual float getListenerScale() const = 0;

  /// Calculate and update all transforms. Must be called from the same thread as
  /// setListenerRotation, setListenerPosition and setListenerScale. NOTE: THIS MUST BE CALLED on
  /// every frame if experimental.fba is enabled
  virtual void update() = 0;

  /// Enable positional tracking by specifying the initial/default position of the listener.
  /// The current position of the listener is automatically queried from AudioEngine
  /// (AudioEngine::setListenerPosition(..)).
  /// The API works by subtracting the current listener position from the initial listener position
  /// to arrive at the delta. The positional tracking range is limited to 1 unit magnitude in each
  /// direction.
  /// @param enable Set to true to enable positional tracking
  /// @param initialListenerPosition Initial/default position of the listener
  /// @return EngineError::OK or EngineError::NOT_SUPPORTED if an invalid renderer is spacified on
  /// initialisation
  virtual EngineError enablePositionalTracking(bool enable, TBVector initialListenerPosition) = 0;

  /// @return true if positional tracking is enabled
  virtual bool positionalTrackingEnabled() const = 0;

  /// @return Buffer size of the engine in samples. In most cases this would be the value
  /// specified on initialisation. Some platforms might return a value different
  /// from what is specified on initialisation.
  virtual int getBufferSize() const = 0;

  /// @return Sample rate of the engine in Hz. In most cases this would be the value
  /// specified on initialisation. Some platforms might return a value different
  /// from what is specified on initialisation.
  virtual float getSampleRate() const = 0;

  /// If the audio is constructed with the audio device disabled, this method can be used to return
  /// mixed spatialised buffers within your app's audio callback. \b WARNING: calling this method
  /// when an audio device is active can result in undefined behaviour.
  /// @param buffer Pointer to existing interleaved float buffer
  /// @param numOfSamples In most cases this would the buffer size * 2 (since it is stereo)
  /// @param numOfChannels Number of channels in buffer. This should be currently set to 2.
  /// @return Relevant error or EngineError::OK
  virtual EngineError getAudioMix(float* buffer, int numOfSamples, int numOfChannels) = 0;

  /// Call this function to register a callback for getting the final audio mix data just before
  /// it is written to the device buffer. It is the caller's responsibility to ensure:
  ///   1) The callback pointer is valid for the lifetime of the engine
  ///   2) Any operations done in the callback do not block the audio thread. This includes
  ///      memory allocation and release, file system and network access and anything that
  ///      may take time and cause the audio thread to cause a buffer underrun.
  ///
  /// @param AudioMixCallback - the callback that will be executed on each audio frame
  /// @param userData - pointer to user specific data
  /// @return Relevant error or EngineError::OK
  virtual EngineError setAudioMixCallback(AudioMixCallback callback, void* userData) = 0;

  /// Call this function to register a callback for getting the final audio mix data summed with
  /// any input audio data coming in from the input device (such as a microphone)
  /// It is the caller's responsibility to ensure:
  ///   1) The callback pointer is valid for the lifetime of the engine
  ///   2) Any operations done in the callback do not block the audio thread. This includes
  ///      memory allocation and release, file system and network access and anything that
  ///      may take time and cause the audio thread to cause a buffer underrun.
  ///
  /// @param AudioMixCallback - the callback that will be executed on each audio frame
  /// @param userData - pointer to user specific data
  /// @return Relevant error or EngineError::OK
  virtual EngineError setAudioInputMixCallback(AudioMixCallback callback, void* userData) = 0;

  /// Same as setAudioMixCallback() but for use with deinterleaved audio
  /// DEPRECATED AND NOT SUPPORTED IF experimental.fba is enabled.
  virtual EngineError setAudioMixDeinterleavedCallback(
      AudioMixDeinterleaved callback,
      void* userData) = 0;

  /// Call this function to to register a callback to receive the interleaved audio input from the
  /// device. To open the audio device for input use @ref openAudioInput().
  ///
  /// @param callback - the callback you want receiving the data
  /// @param userData - pointer to user specific data
  /// @return - EngineError::OK on success or relevant error code
  virtual EngineError setAudioInputInterleavedCallback(
      AudioInterleavedCb callback,
      void* userData) = 0;

  /// Create a new SpatDecoderQueue object from an existing pool.
  /// @param spatDecoder Pointer to SpatDecoderQueue that must be initialised.
  /// @return Relevant error or EngineError::OK
  virtual EngineError createSpatDecoderQueue(SpatDecoderQueue*& spatDecoder) = 0;

  /// Create a new SpatDecoderQueue object. Currently only available through the
  /// experimental.useFBA = true flag.
  /// @param spatDecoder Pointer to SpatDecoderQueue that must be initialised.
  /// @param map The channel map to use for the new queue
  /// @param type The pcm sample type to use for the queue
  /// @return Relevant error or EngineError::OK
  virtual EngineError
  createSpatDecoderQueue(SpatDecoderQueue*& spatDecoder, ChannelMap map, PCMType type) = 0;

  /// Destroy an existing and valid instance to SpatDecoderQueue and return it to the pool.
  virtual void destroySpatDecoderQueue(SpatDecoderQueue*& spatDecoder) = 0;

  /// Creates a virtual speaker layout that can playback arbitrary speaker layout formats
  /// such as stereo, 5.0, 5.1, 7.0 and 7.1.
  ///
  /// The Speakers Virtualizer uses AudioObjects under the hood to create the virtual speakers.
  /// For this reason, this call will fail if you do not have enough objects left in the audio
  /// object pool. You can customize the number of audio objects available via @ref
  /// engineSettings.memorySettings.audioObjectPoolSize
  ///
  /// @param virtualizer A pointer to a SpeakersVirtualizer that will be filled in upon success
  /// @param layout a SpeakerPosition::EndEnum terminated array of SpeakerPositions specifying
  ///        the speaker layout to be used and the order with which they are interleaved e.g.
  ///        [SpeakerPosition::LEFT, SpeakerPosition::RIGHT, SpeakerPosition::CENTER,
  ///        SpeakerPosition::END_ENUM] specifies a 3 channel audio interleaved stream, where
  ///        channel 1 is the audio for the left speaker, channel 2 is the right speaker and channel
  ///        3 is center.
  /// @param channelBufferSizeInSamples The size of the audio buffer for each channel.
  ///
  /// @return EngineError::OK on success or EngineError::NO_OBJECTS_IN_POOL
  virtual EngineError createSpeakersVirtualizer(
      SpeakersVirtualizer*& virtualizer,
      SpeakerPosition const* layout,
      size_t channelBufferSizeInSamples = 8192) = 0;

  /// Destroys a SpeakersVirtualizer and returns its AudioObjects to the pool
  /// @param virtualizer The object to be destroyed. This MUST have been previously created with
  /// createSpeakersVirtualizer()
  virtual void destroySpeakersVirtualizer(SpeakersVirtualizer*& virtualizer) = 0;

  /// Create a new SpatDecoderFile object from an existing pool.
  /// @param spatDecoder Pointer to SpatDecoderFile that must be initialised.
  /// @param options Currently only Options::DEFAULT and Options::DECODE_IN_AUDIO_CALLBACK are
  /// supported, where Options::DECODE_IN_AUDIO_CALLBACK will decode all audio within the audio
  /// mixer callback rather than a separate thread. DECODE_IN_AUDIO_CALLBACK is useful for cases
  /// where you might use the audio engine as an in-place processor with no multi-threading or audio
  /// device support.
  /// @return Relevant error or EngineError::OK
  virtual EngineError createSpatDecoderFile(
      SpatDecoderFile*& spatDecoder,
      Options options = Options::DEFAULT) = 0;

  /// Destroy an existing and valid instance to SpatDecoderFile and return it to the pool.
  virtual void destroySpatDecoderFile(SpatDecoderFile*& spatDecoder) = 0;

  /// Create an audio object that can position mono audio. This is currently experimental and might
  /// return a null object.
  /// @param audioObject Pointer to AudioObject that must be initialised.
  /// @param options Currently only Options::DEFAULT and Options::DECODE_IN_AUDIO_CALLBACK are
  /// supported, where Options::DECODE_IN_AUDIO_CALLBACK will decode all audio within the audio
  /// mixer callback rather than a separate thread. DECODE_IN_AUDIO_CALLBACK is useful for cases
  /// where you might use the audio engine as an in-place processor with no multi-threading or audio
  /// device support.
  /// @return Relevant error or EngineError::OK
  virtual EngineError createAudioObject(
      AudioObject*& audioObject,
      Options options = Options::DEFAULT) = 0;

  /// Destroy an existing and valid instance to AudioObject and return it to the pool.
  virtual void destroyAudioObject(AudioObject*& audioObject) = 0;

  /// Create a new EventTransport that can play back an event timeline.
  /// @param eventTransport Pointer to EventTransport to be initialized
  /// @return relevant error or EngineError::OK
  virtual EngineError createEventTransport(EventTransport*& eventTransport) = 0;

  /// Destroy an existing EventTransport
  virtual void destroyEventTransport(EventTransport*& eventTransport) = 0;

  /// Create a bus for routing audio.
  /// @return Relevant error or EngineError::OK
  virtual EngineError createBus(Bus& bus) = 0;

  /// Destroy bus object
  /// @param busToDestroy A reference to the Bus that will be destroyed
  /// @return Relevant error or EngineError::OK
  virtual EngineError destroyBus(Bus& busToDestroy) = 0;

  /// Connect a bus to the master output bus
  /// @param bus A handle to the bus to connect to master
  virtual EngineError connectToMasterBus(Bus srcBus) = 0;

  /// Connect an AudioObject to the master output bus
  /// @param bus A handle to the bus to connect to master
  virtual EngineError connectToMasterBus(AudioObject* audioObject) = 0;

  /// Connect a bus to a bus. See also connectToMasterBus()
  /// @param srcBus Handle to source bus object
  /// @param destBus Handle to destination bus object
  /// @return Relevant error or EngineError::OK
  virtual EngineError connect(Bus srcBus, Bus destBus) = 0;

  /// Connect an AudioObject to a bus. See also connectToMasterBus()
  /// @param audioObject Handle to source AudioObject object
  /// @param destBus Handle to destination bus object
  /// @return Relevant error or EngineError::OK
  virtual EngineError connect(AudioObject* audioObject, Bus destBus) = 0;

  /// Disconnect the bus output
  /// @param bus Handle to source bus object
  /// @return Relevant error or EngineError::OK
  virtual EngineError disconnectOutput(Bus bus) = 0;

  /// Disconnect the AudioObject output
  /// @param bus Handle to source bus object
  /// @return Relevant error or EngineError::OK
  virtual EngineError disconnectOutput(AudioObject* audioObject) = 0;

  /// Set the gain of the bus
  /// @param bus Handle to bus object
  /// @param gain Linear gain value in [0,1]
  /// @param rampTimeMs Ramp time to the new gain value in milliseconds. If 0, a very short ramp
  /// will be applied to avoid a click.
  /// @return Relevant error or EngineError::OK
  virtual EngineError setGain(Bus bus, float gain, float rampTimeMs) = 0;

  /// Get the gain of the bus
  /// @param bus Handle to bus object
  /// @param gain Linear gain value in [0,1]
  /// @return Relevant error or EngineError::OK
  virtual EngineError getGain(Bus bus, float& gain) = 0;

  static const int32_t AUDIO360_MAX_BUS_NAME_SIZE = 256;

  /// Set the human readable name of the bus
  /// @param bus Handle to bus object
  /// @param name The human readable name of the bus
  /// @return Relevant error or EngineError::OK
  virtual EngineError setName(Bus bus, const char name[AUDIO360_MAX_BUS_NAME_SIZE]) = 0;

  /// Get the human readable name of the bus
  /// @param bus Handle to bus object
  /// @param name The human readable name of the bus
  /// @return Relevant error or EngineError::OK
  virtual EngineError getName(Bus bus, char name[AUDIO360_MAX_BUS_NAME_SIZE]) = 0;

  /// Get the human readable name of the master bus
  /// @param name The human readable name of the bus
  /// @return Relevant error or EngineError::OK
  virtual EngineError getMasterBusName(char name[AUDIO360_MAX_BUS_NAME_SIZE]) = 0;

  /// Set a function to receive events from the audio engine.
  /// @param callback Event callback function
  /// @param userData User data. Can be nullptr
  /// @return Relevant error or EngineError::OK
  virtual EngineError setEventCallback(EventCallback callback, void* userData) = 0;

  /// Play a test sine tone at the specified frequency and gain (linear). This will overwrite
  /// any other audio that might be mixed or processed by the engine.
  virtual void enableTestTone(bool enable, float frequency = 440.f, float gain = 0.5f) = 0;

  /// @return Major version as int
  virtual int getVersionMajor() const = 0;

  /// @return Minor version as int
  virtual int getVersionMinor() const = 0;

  /// @return Patch version as int
  virtual int getVersionPatch() const = 0;

  /// @returns The version hash
  virtual const char* getVersionHash() const = 0;

  /// The rendered loudness statistics since construction or since the last
  /// call to resetIntegratedLoudness(). This is not a measure of the
  /// loudness of the ambisonic field, but rather a measure of the loudness
  /// as binaurally rendered for the listener (including head tracking and
  /// HRTFs).
  /// Threadsafe.
  /// Care must be taken not to call in parallel with enableLoudness().
  virtual LoudnessStatistics getRenderedLoudness() = 0;

  /// Reset loudness statistics state.
  /// Threadsafe.
  /// Care must be taken not to call in parallel with enableLoudness().
  virtual void resetLoudness() = 0;

  /// Default state: disabled. Threadsafe but not reentrant.
  /// Care must be taken not to call in parallel with getRenderedLoudness() or resetLoudness()
  virtual void enableLoudness(bool enabled = true) = 0;

  /// Returns the elapsed time in samples since the engine was initialised and start() was called.
  /// The DSP clock count will be suspended when suspend() is called and resumed when start() is
  /// called.
  virtual int64_t getDSPTime() const = 0;

  /// Sets the number of output buffers. Thread safe.
  /// Currently only applicable on Android when an audio device is used and can be used for
  /// increasing the buffer size when bluetooth audio devices are connected.
  /// @param numOfBuffers
  /// Number of buffers. Default is 1. Minimum 1, Maximum 12. A value greater than 12 will be
  /// clamped.
  /// @return EngineError::OK if successful or EngineError::NOT_SUPPORTED.
  virtual EngineError setNumOutputBuffers(unsigned int numOfBuffers) = 0;

  /// Returns the current number of output buffers. Default: 1. Thread safe.
  virtual unsigned int getNumOutputBuffers() const = 0;

  /// Returns the output latency in samples, if an audio device is in use. On most platforms this is
  /// likely to be a best guess. The latency can be used to compensate for sync in video players.
  virtual int32_t getOutputLatencySamples() const = 0;

  /// Returns the output latency in milliseconds, if an audio device is in use. On most platforms
  /// this is likely to be a best guess. The latency can be used to compensate for sync in video
  /// players.
  virtual double getOutputLatencyMs() const = 0;

  /// Returns the name of the output audio device. This could be the name of the default device as
  /// specified by the OS or a custom device specified on intialisation. On mobile devices this is
  /// likely to return "default".
  virtual const char* getOutputAudioDeviceName() const = 0;

  /// Opens the specified audio device for receiving input audio
  /// The user can set a callback to receive the input audio data by calling @ref
  /// setAudioInputCallback()
  /// @param name - the name of the device to open. Null or empty string will open the default
  /// device
  virtual EngineError openAudioInput(char const* name) = 0;

  /// Opens the audio device for input and mixes the received audio data into the output buffer
  virtual EngineError mixAudioInput(bool shouldMix) = 0;

  /// Sets the gain for the input audio data mixed in the output buffers if
  /// the device was opened with mixAudioInput
  virtual EngineError setInputMixGain(float gain) = 0;

  /// Closes the audio input device
  virtual EngineError closeAudioInput() = 0;

  /// Returns the object instance of AudioAssetManager owned by the AudioEngine to let the user
  /// to create audio asset handle and IOStreams.
  virtual AudioAssetManager* getAudioAssetManager() const = 0;

  // Gets the voice manager owned by the AudioEngine
  virtual VoiceManager* getVoiceManager() const = 0;

  /// Set the linear volume of the master bus
  /// @param linearVolume Master linear volume
  /// @param rampTimeMs Ramp time to the new gain value in milliseconds. If 0, a very short ramp
  /// will be applied to avoid a click.
  virtual void setMasterVolume(float linearVolume, float rampTimeMs) = 0;

  /// @return The current master linear volume
  virtual float getMasterVolume() const = 0;

  /// Mute the master bus
  /// @param shouldMute set to true if the master bus must be muted
  virtual void enableMasterMute(bool shouldMute) = 0;

  /// @return Returns true if the master bus is muted
  virtual bool isMasterMuteEnabled() const = 0;

  /// Get engine statistics. This will typically return data from the last process call
  /// (getAudioMix) or device callback. Thread safe.
  /// @return EngineStatistics will populated data
  virtual EngineStatistics getStats() = 0;

  /// Static method that returns the number of available audio devices.
  API_EXPORT static int32_t getNumAudioDevices();

  /// Static method that returns the name of a device. The name can used on initialisation of the
  /// AudioEngine to route audio to this device.
  /// @param index A value between 0 and getNumAudioDevices() - 1
  /// @return Name of the device.
  API_EXPORT static const char* getAudioDeviceName(int index);

  /// Static method that returns the name of an audio device based on its ID. Currently only
  /// applicable on Windows.
  /// @param id A string containing the GUID of the IMMDevice on Windows (IMMDevice->GetId).
  /// @return The name of the device, if found
  API_EXPORT static const char* getAudioDeviceNameFromId(wchar_t* id);

  /// Bypass the master reverb
  /// @param bypass Set to true to bypass the reverb
  /// @return Relevant error or EngineError::OK
  virtual EngineError setMasterReverbBypass(bool bypass) = 0;

  /// Get the bypass state of the master reverb
  /// @return True if the reverb is bypassed
  virtual bool getMasterReverbBypass() = 0;

  /// Set the master reverb wet mix level
  /// @param wetLevel The wet level between 0 and 1
  /// @return Relevant error or EngineError::OK
  virtual EngineError setMasterReverbWetLevel(float wetLevel) = 0;

  /// Get the master reverb wet mix level
  /// @return The master reverb wet mix level
  virtual float getMasterReverbWetLevel() = 0;

  /// Set the master reverb room size
  /// @param roomSize The room size parameter between 0 and 1 - 0 = no reverb, 1 = large reflective
  /// room
  /// @return Relevant error or EngineError::OK
  virtual EngineError setMasterReverbRoomSize(float roomSize) = 0;

  /// Get the master reverb room size
  /// @return The room size parameter between 0 and 1 - 0 = no reverb, 1 = large reflective room
  virtual float getMasterReverbRoomSize() = 0;

  /// Set the master reverb dampening factor
  /// @param dampening The dampening factor, between 0 and 1 - 0 = no dampening, 1 = highly damped
  /// @return Relevant error or EngineError::OK
  virtual EngineError setMasterReverbDampening(float dampening) = 0;

  /// Get the master reverb dampening factor
  /// @return The dampening factor, between 0 and 1 - 0 = no dampening, 1 = highly damped
  virtual float getMasterReverbDampening() = 0;

  /// Set the master reverb width
  /// @param width The stereo width parameter between 0 and 1. =1 gives highest decorrelation
  /// between left and right channels
  /// @return Relevant error or EngineError::OK
  virtual EngineError setMasterReverbWidth(float width) = 0;

  /// Get the master reverb width
  /// @return The stereo width parameter between 0 and 1. =1 gives highest decorrelation between
  /// left and right channels
  virtual float getMasterReverbWidth() = 0;

  /// Experimental, use with caution! Save the internal graph to a json file.
  /// @param path Path and file name
  /// @return true on success
  virtual bool saveGraph(const char* path) = 0;
};

class TransportControl {
 public:
  /// Begin dequeueing and playing back audio. Threadsafe but non reentrant, all transport methods
  /// must not be called in parallel.
  /// @return Relevant error or EngineError::OK
  virtual EngineError play() = 0;

  /// Schedule a play event after a period of time. Any subsequent call to this function or any play
  /// function will disregard this event if it hasn't already been triggered. Threadsafe but non
  /// reentrant, all transport methods must not be called in parallel.
  /// @param millisecondsFromNow
  /// Number of milliseconds from now after which playback will begin. Must be a positive number.
  /// @return Relevant error or EngineError::OK
  virtual EngineError playScheduled(float millisecondsFromNow) = 0;

  /// Schedule a play event after a period of time with a fadein. Any subsequent call to this
  /// function or any play function will disregard this event if it hasn't already been triggered.
  /// Threadsafe but non reentrant, all transport methods must not be called in parallel.
  /// @param millisecondsFromNow
  /// Number of milliseconds from now after which playback will begin. Must be a positive number.
  /// @param fadeDurationInMs Duration of the
  /// fadein in milliseconds. Must be a positive number.
  /// @return Relevant error or EngineError::OK
  virtual EngineError playScheduled(float millisecondsFromNow, float fadeDurationInMs) = 0;

  /// Begin playback with a fadein. Any subsequent call to this function or any play function will
  /// disregard this event if it hasn't already been triggered. Threadsafe but non reentrant, all
  /// transport methods must not be called in parallel.
  /// @param fadeDurationInMs Duration of the
  /// fadein in milliseconds. Must be a positive number.
  /// @return Relevant error or EngineError::OK
  virtual EngineError playWithFade(float fadeDurationInMs) = 0;

  /// Pause playback. Any subsequent call to this function or any pause function will disregard this
  /// event if it hasn't already been triggered. Threadsafe but non reentrant, all transport methods
  /// must not be called in parallel.
  /// @return Relevant error or EngineError::OK
  virtual EngineError pause() = 0;

  /// Schedule a pause event after a period of time. Any subsequent call to this function or any
  /// pause function will disregard this event if it hasn't already been triggered. Threadsafe but
  /// non reentrant, all transport methods must not be called in parallel.
  /// @param
  /// millisecondsFromNow Number of milliseconds from now after which playback pause. Must be a
  /// positive number.
  /// @return Relevant error or EngineError::OK
  virtual EngineError pauseScheduled(float millisecondsFromNow) = 0;

  /// Schedule a pause event after a period of time and fadeout. Any subsequent call to this
  /// function or any pause function will disregard this event if it hasn't already been triggered.
  /// Threadsafe but non reentrant, all transport methods must not be called in parallel.
  /// @param
  /// millisecondsFromNow Number of milliseconds from now after which playback pause. Must be a
  /// positive number.
  /// @param fadeDurationInMs Duration of the
  /// fadeout in milliseconds. Must be a positive number.
  /// @return Relevant error or EngineError::OK
  virtual EngineError pauseScheduled(float millisecondsFromNow, float fadeDurationInMs) = 0;

  /// Fadeout the audio and pause. Any subsequent call to this function or any pause function will
  /// disregard this event if it hasn't already been triggered. Threadsafe but non reentrant, all
  /// transport methods must not be called in parallel.
  /// @param fadeDurationInMs Duration of the
  /// fadeout in milliseconds. Must be a positive number.
  /// @return Relevant error or EngineError::OK
  virtual EngineError pauseWithFade(float fadeDurationInMs) = 0;

  /// Stops playback and resets the playhead to the start of the asset. Threadsafe but non
  /// reentrant, all transport methods must not be called in parallel.
  /// @return Relevant error or EngineError::OK
  virtual EngineError stop() = 0;

  /// Schedule a stop event after a period of time. Any subsequent call to this function or any stop
  /// function will disregard this event if it hasn't already been triggered. Threadsafe but non
  /// reentrant, all transport methods must not be called in parallel.
  /// @param millisecondsFromNow
  /// Number of milliseconds from now after which playback will be stopped. Must be a positive
  /// number.
  /// @return Relevant error or EngineError::OK
  virtual EngineError stopScheduled(float millisecondsFromNow) = 0;

  /// Schedule a stop event after a period of time and fadeout. Any subsequent call to this function
  /// or any stop function will disregard this event if it hasn't already been triggered. Threadsafe
  /// but non reentrant, all transport methods must not be called in parallel.
  /// @param millisecondsFromNow
  /// Number of milliseconds from now after which playback will be stopped. Must be a positive
  /// number.
  /// @param fadeDurationInMs Duration of the
  /// fadeout in milliseconds. Must be a positive number.
  /// @return Relevant error or EngineError::OK
  virtual EngineError stopScheduled(float millisecondsFromNow, float fadeDurationInMs) = 0;

  /// Fadeout and stop playback. Any subsequent call to this function or any stop function will
  /// disregard this event if it hasn't already been triggered. Threadsafe but non reentrant, all
  /// transport methods must not be called in parallel.
  /// @param fadeDurationInMs Duration of the
  /// fadeout in milliseconds. Must be a positive number.
  /// @return Relevant error or EngineError::OK
  virtual EngineError stopWithFade(float fadeDurationInMs) = 0;

  /// @return Playback state, either PlayState::PLAYING or PlayState::PAUSED
  virtual PlayState getPlayState() const = 0;

  /// Cancel all scheduled parameter changes. This is mainly used to cancel future play state
  /// changes.
  virtual void cancelScheduledParams() = 0;

 protected:
  virtual ~TransportControl(){};
};

/// An object with a 3D position and rotation
class Object3D : public TransportControl {
 public:
  /// Set the position of the object
  /// @param position Position of the object in world space
  /// @return  EngineError::OK or EngineError::NOT_SUPPORTED
  virtual EngineError setPosition(TBVector position) = 0;

  /// @return The current position of the object
  virtual TBVector getPosition() const = 0;

  /// Set the rotation of the object
  /// @param rotation Rotation of the object (local space)
  /// @return  EngineError::OK or EngineError::NOT_SUPPORTED
  virtual EngineError setRotation(TBQuat rotation) = 0;

  /// Set the rotation of the object
  /// @param forward Forward direction vector
  /// @param up Up direction vector
  /// @return  EngineError::OK or EngineError::NOT_SUPPORTED
  virtual EngineError setRotation(TBVector forward, TBVector up) = 0;

  /// @return The current rotation of the object
  virtual TBQuat getRotation() const = 0;

 protected:
  virtual ~Object3D(){};
};

/// Interface for spatial audio decoders.
class SpatDecoderInterface : public Object3D {
 public:
  /// Enable mix focus. This gets a specified area of the mix to be more audible than surrounding
  /// areas, by reducing the volume of the area that isn't in focus. The focus area uses a cosine
  /// bump.
  /// @param enableFocus Enable focus
  /// @param followListener The focus area follows the listener's gaze
  virtual void enableFocus(bool enableFocus, bool followListener) = 0;

  /// DEPRECATED: Use @ref setOffFocusLeveldB() and @ref setFocusWidthDegrees() instead!
  /// Set the properties of the focus effect. This will be audible only if
  /// enableFocus(..) is set to true.
  /// @param offFocusLevel The level of the area that isn't in focus. A clamped ranged between 0
  /// and 1. 1 is no focus. 0 is maximum focus (the off focus area is reduced by 24dB)
  /// @param focusWidth The focus area specified in degrees
  virtual void setFocusProperties(float offFocusLevel, float focusWidth) = 0; // deprecated

  /// Set the attenuation level outside of the area of focus. This will only take
  /// effect if focus has been enabled by enableFocus(...) and the value is negative.
  /// @param offFocusLevelDB The attenuation level in dB outside of the focus area.
  /// This value has a range between -24.0 and 0.0. Out of bounds values are clamped.
  virtual void setOffFocusLeveldB(float offFocusLevelDB) = 0;

  /// Set the width angle in degrees of the focus area, between 40 and 120 degrees.
  /// This will only take effect if focus has been enabled via enableFocus(...)
  /// @param focusWidthDegrees the focus angle in degrees
  virtual void setFocusWidthDegrees(float focusWidthDegrees) = 0;

  /// Set the orientation of the focus area as a quaternion. This orientation is from the
  /// perspective of the listener and is audible only if followListener in enableFocus(..) is set to
  /// false.
  /// @param focusQuat Orientation of the focus area as a quaternion.
  virtual void setFocusOrientationQuat(TBQuat focusQuat) = 0;

  /// Set the volume in linear gain, with an optional ramp time
  /// @param linearGain Linear gain, where 0 is mute and 1 is unity gain
  /// @param rampTimeMs Ramp time to the new gain value in milliseconds. If 0, no ramp will be
  /// applied.
  /// @param forcePreviousRamp This forces a previous ramp message (if any) to conclude right away
  virtual void setVolume(float linearGain, float rampTimeMs, bool forcePreviousRamp = false) = 0;

  /// Set the volume in decibels, with an optional ramp time
  /// @param dB Volume in decibels, where 0 is unity gain.
  /// @param rampTimeMs Ramp time to the new gain value in milliseconds. If 0, no ramp will be
  /// applied.
  /// @param forcePreviousRamp This forces a previous ramp message (if any) to conclude right away
  virtual void setVolumeDecibels(float dB, float rampTimeMs, bool forcePreviousRamp = false) = 0;

  /// @return The current volume
  /// @see setVolume, setVolumeDecibels
  virtual float getVolume() const = 0;

  /// @return The current volume in decibels
  /// @see setVolume, setVolumeDecibels
  virtual float getVolumeDecibels() const = 0;

  /// Set a function to receive events from this object. Currently Event::DECODER_INIT is sent when
  /// the a file is loaded and ready for playback.
  /// @param callback Event callback function
  /// @param userData User data. Can be nullptr
  /// @return Relevant error or EngineError::OK
  virtual EngineError setEventCallback(EventCallback callback, void* userData) = 0;

  /// Set bypassing of the reverb send
  /// @param bypass Set true to stop sending audio through to the reverb
  virtual EngineError bypassReverbSend(bool bypass) = 0;

  /// Check if the reverb send is bypassed
  /// @return True if reverb send is bypassed
  virtual bool isReverbSendBypassed() = 0;

  /// Set the level of the reverb send
  /// @param level The reverb send level, linear gain from 0 to 1
  virtual EngineError setReverbSendLevel(float level) = 0;

  /// Get the reverb send level
  /// @return The reverb send level, linear gain from 0 to 1
  virtual float getReverbSendLevel() = 0;

  /// NOTE: DEPRECATED AND UNAVAILABLE IF FBA IS ACTIVE. USE AudioObject::createEffect and similar
  /// Activate an insert effect. There are a fixed number of effect slots, indicated by the
  /// EffectIndex enum. Adding an insert to a slot will replace any existing effect in that slot.
  /// @param effectIndex Specify which slot index to put the effect in
  /// @param effectType Set the effect type
  /// @return Relevant error or EngineError::OK
  virtual EngineError addEffectInsert(EffectIndex effectIndex, EffectType effectType) = 0;

  /// NOTE: DEPRECATED AND UNAVAILABLE IF FBA IS ACTIVE. USE AudioObject::createEffect and similar
  /// Remove an existing effect instance from the processing chain.
  /// @param effectIndex Specify which effect slot to update
  /// @return Relevant error or EngineError::OK
  virtual EngineError removeEffectInsert(EffectIndex effectIndex) = 0;

  /// NOTE: DEPRECATED AND UNAVAILABLE IF FBA IS ACTIVE. USE AudioObject::createEffect and similar
  /// Bypass an effect. The effect parameters are preserved, but the effect isn't active.
  /// @param effectIndex Specify which effect slot to update
  /// @param bypass Set true to bypass the effect
  virtual EngineError bypassEffectInsert(EffectIndex effectIndex, bool bypass) = 0;

  /// NOTE: DEPRECATED AND UNAVAILABLE IF FBA IS ACTIVE. USE AudioObject::createEffect and similar
  /// Set an effect parameter for a particular effect.
  /// @param effectIndex Specify which effect slot to update
  /// @param effectParam Specify which effect parameter to update
  /// @param value Set the parameter value
  virtual EngineError
  setEffectInsertParam(EffectIndex effectIndex, EffectParam effectParam, float value) = 0;

  /// NOTE: DEPRECATED AND UNAVAILABLE IF FBA IS ACTIVE. USE AudioObject::createEffect and similar
  /// Get an effect parameter for a particular effect.
  /// @param effectIndex Specify which effect slot to update
  /// @param effectParam Specify which effect parameter to update
  /// @return Returns the effect parameter value as a float
  virtual float getEffectInsertParam(EffectIndex effectIndex, EffectParam effectParam) = 0;

  /// NOTE: DEPRECATED AND UNAVAILABLE IF FBA IS ACTIVE. USE AudioObject::createEffect and similar
  /// Check if an effect insert slot is active.
  /// @param effectIndex Specify which effect slot to check
  /// @return Returns true if effect is active
  virtual bool isEffectInsertActive(EffectIndex effectIndex) = 0;

  /// NOTE: DEPRECATED AND UNAVAILABLE IF FBA IS ACTIVE. USE AudioObject::createEffect and similar
  /// Check if an effect insert slot is bypassed.
  /// @param effectIndex Specify which effect slot to check
  /// @return Returns true if effect is bypassed
  virtual bool isEffectInsertBypassed(EffectIndex effectIndex) = 0;

  /// NOTE: DEPRECATED AND UNAVAILABLE IF FBA IS ACTIVE. USE AudioObject::createEffect and similar
  /// Check what kind of effect is in the insert slot.
  /// @param effectIndex Specify which effect slot to check
  /// @return The effect type
  virtual EffectType getEffectType(EffectIndex effectIndex) = 0;

 protected:
  virtual ~SpatDecoderInterface() {}
};

/// An object for enqueuing and processing spatial audio buffers.
/// Data that is enqueued will be dequeued and processed by the audio engine in the audio device
/// callback. If the audio device is disabled, the data will be processed when
/// AudioEngine::getAudioMix is called. The audio queue is implemented as a lock-free circular
/// buffer (thread safe for one producer and one consumer).
class SpatDecoderQueue : public SpatDecoderInterface {
 public:
  /// Get the free space in the queue (in number of samples) for the kind of data you are enqueueing
  /// @param channelMap Channel map for the data
  /// @return The free space of the queue in samples
  virtual int32_t getFreeSpaceInQueue(ChannelMap channelMap) const = 0;

  /// The size of the queue in samples for the kind of data you are enqueueing
  /// @param channelMap Channel map for the data
  /// @return the size of the queue in samples
  virtual int32_t getQueueSize(ChannelMap channelMap) const = 0;

  /// Enqueue interleaved float buffers of data.
  /// @param interleavedBuffer Interleaved float buffer
  /// @param numTotalSamples Number of total samples in buffer (including all channels)
  /// @param channelMap The channel map for the data being enqueued
  /// @return Number of samples successfully been queued. Should be the same as numTotalSamples.
  virtual int32_t
  enqueueData(const float* interleavedBuffer, int32_t numTotalSamples, ChannelMap channelMap) = 0;

  /// Enqueue interleaved 16 bit int buffers of data.
  /// @param interleavedBuffer Interleaved 16 bit int buffer
  /// @param numTotalSamples Number of total samples in buffer (including all channels)
  /// @param channelMap The channel map for the data being enqueued
  /// @return Number of samples successfully been queued. Should be the same as numTotalSamples.
  virtual int32_t
  enqueueData(const int16_t* interleavedBuffer, int32_t numTotalSamples, ChannelMap channelMap) = 0;

  /// Enqueue silence for a specific channel map configuration
  /// @param channelMap The channel map for the data being enqueued
  /// @param numTotalSamples Number of total samples in buffer (including all channels)
  /// @return Number of samples successfully been queued. Should be the same as numTotalSamples.
  virtual int32_t enqueueSilence(int32_t numTotalSamples, ChannelMap channelMap) = 0;

  /// Flushes data / clears the audio queue. It also resets the endOfStream flag (if specified in
  /// enqueueData()) to false.
  virtual void flushQueue() = 0;

  /// @return The number of samples dequeued and processed per channel.
  virtual uint64_t getNumSamplesDequeuedPerChannel() const = 0;

  /// Specify if the end of the stream has been reached. This ensures that data that might be less
  /// than the buffer size is dequeued correctly
  virtual void setEndOfStream(bool endOfStream) = 0;

  /// Returns true if the end of stream flag has been set using setEndOfStream(..)
  virtual bool getEndOfStreamStatus() const = 0;

 protected:
  virtual ~SpatDecoderQueue() {}
};

class SpeakersVirtualizer : public TransportControl {
 public:
  /// Enqueue interleaved float buffers of data.
  /// This method MUST be called consistently on the same thread!
  /// @param interleavedBuffer Interleaved float buffer. The number of channels MUST be the same as
  /// the
  ///        number of speakers given to the @ref createSpeakersVirtualizer() function
  /// @param numTotalSamples Number of total samples in buffer (this includes all channels)
  /// @param numEnqueued Filled in with the number of samples successfully queued.
  /// @param endOfStream Optional parameter to indicate the end of stream, which will allow the
  /// audio callback
  ///        to consume the remaining samples in the audio queues, even if they are less than the
  ///        required number of samples for the callback
  /// @return EngineError::OK if all samples have been queued
  ///         EngineError::QUEUE_FULL if some samples have been queued (check numEnqueued)
  ///         EngineError::BAD_THREAD if this method is called from another thread
  ///         EngineError::INVALID_BUFFER_SIZE if the numTotalSamples is not divisible by the
  ///         channel count EngineError::FAIL if there are no speaker objects
  virtual EngineError enqueueData(
      const float* interleavedBuffer,
      int32_t numTotalSamples,
      int32_t& numEnqueued,
      bool endOfStream = false) = 0;

  /// Enqueue interleaved int16_t buffers of data.
  /// This method MUST be called consistently on the same thread!
  /// @param interleavedBuffer Interleaved float buffer. The number of channels MUST be the same as
  /// the
  ///        number of speakers given to the @ref createSpeakersVirtualizer() function
  /// @param numTotalSamples Number of total samples in buffer (this includes all channels)
  /// @param numEnqueued Filled in with the number of samples successfully queued.
  /// @param endOfStream Optional parameter to indicate the end of stream, which will allow the
  /// audio callback
  ///        to consume the remaining samples in the audio queues, even if they are less than the
  ///        required number of samples for the callback
  /// @return EngineError::OK if all samples have been queued
  ///         EngineError::QUEUE_FULL if some samples have been queued (check numEnqueued)
  ///         EngineError::BAD_THREAD if this method is called from another thread
  ///         EngineError::INVALID_BUFFER_SIZE if the numTotalSamples is not divisible by the
  ///         channel count EngineError::FAIL if there are no speaker objects
  virtual EngineError enqueueData(
      const int16_t* interleavedBuffer,
      int32_t numTotalSamples,
      int32_t& numEnqueued,
      bool endOfStream = false) = 0;

  /// Set a function to receive events from this object. Currently Event::ERROR_BUFFER_UNDERRUN
  /// is sent when the audio callback requested more samples that are enqueued and end of stream
  /// has not been signaled
  /// @param callback Event callback function
  /// @param userData User data. Can be nullptr
  /// @return Relevant error or EngineError::OK
  virtual EngineError setEventCallback(EventCallback callback, void* userData) = 0;

  /// Get the free space in the queue (in total number of samples for all channels)
  /// for the kind of data you are enqueueing
  /// @return The free space of the queue in samples
  virtual int32_t getFreeSpaceInQueue() const = 0;

  /// The size of the queue in samples for the kind of data you are enqueueing
  /// @return the size of the queue in samples
  virtual int32_t getQueueSize() const = 0;

  /// Flushes data / clears the audio queues. It also resets the endOfStream flag
  virtual void flushQueue() = 0;

  /// Specify if the end of the stream has been reached. This ensures that data that might be less
  /// than the buffer size is dequeued correctly
  virtual void setEndOfStream(bool endOfStream) = 0;

  /// Returns true if the end of stream flag has been set using setEndOfStream(..)
  virtual bool getEndOfStreamStatus() const = 0;

  /// @return The number of samples dequeued and processed per channel.
  virtual uint64_t getNumSamplesDequeuedPerChannel() const = 0;

  /// Set the volume in linear gain, with an optional ramp time
  /// @param linearGain Linear gain, where 0 is mute and 1 is unity gain
  /// @param rampTimeMs Ramp time to the new gain value in milliseconds. If 0, no ramp will be
  /// applied.
  /// @param forcePreviousRamp This forces a previous ramp message (if any) to conclude right away
  virtual void setVolume(float linearGain, float rampTimeMs, bool forcePreviousRamp = false) = 0;

  /// Set the volume in decibels, with an optional ramp time
  /// @param dB Volume in decibels, where 0 is unity gain.
  /// @param rampTimeMs Ramp time to the new gain value in milliseconds. If 0, no ramp will be
  /// applied.
  /// @param forcePreviousRamp This forces a previous ramp message (if any) to conclude right away
  virtual void setVolumeDecibels(float dB, float rampTimeMs, bool forcePreviousRamp = false) = 0;

  /// @return The current volume
  /// @see setVolume, setVolumeDecibels
  virtual float getVolume() const = 0;

  /// @return The current volume in decibels
  /// @see setVolume, setVolumeDecibels
  virtual float getVolumeDecibels() const = 0;

 protected:
  virtual ~SpeakersVirtualizer(){};
};

class SpatDecoderFile : public SpatDecoderInterface {
 public:
  /// Opens an asset for playback. Currently .wav, .opus and .tbe files are supported.
  /// While the asset is opened synchronously, it is loaded into the streaming buffer
  /// asynchronously. An Event::DECODER_INIT event will be dispatched to the event callback when the
  /// streaming buffer is ready for the asset to play.
  /// @see  setEventCallback, close
  /// @param nameAndPath Name and path to the file
  /// @param map The channel mapping / spatial audio format of the file. If ChannelMap::UNKNOWN, the
  /// ChannelMap will be determined by parsing any available metadata
  /// @return Relevant error or EngineError::OK
  virtual EngineError open(const char* nameAndPath, ChannelMap map = ChannelMap::UNKNOWN) = 0;

  /// Opens IOStream object for playback. Currently .wav, .opus, and .tbe codecs are supported.
  /// Two instances of IOStream are required as SpatDecoderFile might use two streams for seamless
  /// synchronisation. While the asset is opened synchronously, it is loaded into the streaming
  /// buffer asynchronously. An Event::DECODER_INIT event will be dispatched to the event callback
  /// when the streaming buffer is ready for the asset to play.
  /// @see  setEventCallback, close
  /// @param streams Two instances of IOStream for the same audio asset that must be played
  /// @param shouldOwnStreams If the lifecycle of the streams must be owned by this object
  /// @param map The channel mapping / spatial audio format of the file. If ChannelMap::UNKNOWN, the
  /// ChannelMap will be determined by parsing any available metadata
  /// @return Relevant error or EngineError::OK
  virtual EngineError
  open(TBE::IOStream* streams[2], bool shouldOwnStreams, ChannelMap map = ChannelMap::UNKNOWN) = 0;

  /// Opens an asset for playback using an asset descriptor. Useful for playing back chunks within a
  /// larger file. Currently .wav, .opus, and .tbe codecs are supported. While the asset is opened
  /// synchronously, it is loaded into the streaming buffer asynchronously. An Event::DECODER_INIT
  /// event will be dispatched to the event callback when the streaming buffer is ready for the
  /// asset to play.
  /// @see  setEventCallback, close
  /// @param nameAndPath Name and path to the file
  /// @param ad AssetDescriptor for the file
  /// @param map The channel mapping / spatial audio format of the file. If ChannelMap::UNKNOWN, the
  /// ChannelMap will be determined by parsing any available metadata
  /// @return Relevant error or EngineError::OK
  virtual EngineError
  open(const char* nameAndPath, AssetDescriptor ad, ChannelMap map = ChannelMap::UNKNOWN) = 0;

  /// Close an open file or stream objects and release resources
  virtual void close() = 0;

  /// Returns true if a file or stream is open
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

  /// Set synchronisation mode. Use setExternalClockInMs() if using external sync to specify the
  /// time.
  /// @param syncMode SyncMode::INTERNAL for internal synchronisation or SyncMode::EXTERNAL for an
  /// external source.
  virtual void setSyncMode(SyncMode syncMode) = 0;

  /// Returns the current synchronisation mode. SyncMode::INTERNAL by default
  virtual SyncMode getSyncMode() const = 0;

  /// Set the external time source in milliseconds when using SyncMode::EXTERNAL
  /// @param externalClockInMs Elapsed time in milliseconds
  virtual void setExternalClockInMs(double externalClockInMs) = 0;

  /// Set how often the engine tries to synchronise to the external clock. Very low values can
  /// result in stutter, very high values can result in synchronisation latency This method along
  /// with setResyncThresholdMs() can be used to fine-tune synchronisation.
  /// @param freewheelInMs Freewheel time or how often the engine tries to synchronise to the
  /// external clock
  virtual void setFreewheelTimeInMs(double freewheelInMs) = 0;

  /// Returns the current freewheel time
  virtual double getFreewheelTimeInMs() = 0;

  /// The time threshold after which the engine will synchronise to the external clock.
  /// This method along with setFreewheelTimeInMs() can be used to fine-tune synchronisation.
  /// @param resyncThresholdMs Synchronisation threshold in milliseconds
  virtual void setResyncThresholdMs(double resyncThresholdMs) = 0;

  /// Returns current synchronisation threshold
  virtual double getResyncThresholdMs() const = 0;

  /// Apply a volume fade over a period of time. The ramp is applied immediately with the current
  /// volume being set to startLinearGain and then ramping to endLinearGain over fadeDurationMs
  /// @param startLinearGain Linear gain value at the start of the fade, where 0 is mute and 1 is
  /// unity gain
  /// @param endLinearGain Linear gain value at the end of the fade, where 0 is mute and 1 is unity
  /// gain
  /// @param fadeDurationMs Duration of the ramp in milliseconds
  virtual void
  applyVolumeFade(float startLinearGain, float endLinearGain, float fadeDurationMs) = 0;

  /// Enable looping of the currently loaded file. Use this method for sample accurate looping
  /// rather than manually seeking the file to 0 when it finishes playing.
  /// @param shouldLoop Looping is enabled if true
  virtual void enableLooping(bool shouldLoop) = 0;

  /// Returns true if looping is enabled
  virtual bool loopingEnabled() const = 0;

 protected:
  virtual ~SpatDecoderFile() {}
};
} // namespace TBE

extern "C" {

/// Create a new instance of the AudioEngine.
/// @param engine Pointer to the engine instance
/// @param initSettings Initialisation settings. Can be TBE::EngineInitSettings_default.
/// @see TBE::EngineInitSettings for all the initialisation settings.
/// @return Relevant error or EngineError::OK
API_EXPORT TBE::EngineError TBE_CreateAudioEngine(
    TBE::AudioEngine*& engine,
    TBE::EngineInitSettings initSettings = TBE::EngineInitSettings());

/// Destroy an existing instance of AudioEngine. All child objects (such as SpatDecoderQueue will be
/// destroyed and made invalid).
/// @param engine Pointer to the engine instance
API_EXPORT void TBE_DestroyAudioEngine(TBE::AudioEngine*& engine);
}

#endif // FBA_TBE_AUDIOENGINE_H
