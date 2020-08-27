#ifndef FBA_TBE_EVENTTRANSPORT_H
#define FBA_TBE_EVENTTRANSPORT_H

/*
 * Copyright (c) 2019-present, Facebook, Inc.
 */

#pragma once

#include "TBE_AudioEngine.h"
#include "TBE_AudioEngineDefinitions.h"

namespace TBE {

class EventTransport : public TransportControl {
 public:
  /// Opens an asset for playback. This is currently a MIDI file on disk.
  /// @see  setEventCallback, close
  /// @param nameAndPath Name and path to the file
  /// @return Relevant error or EngineError::OK
  virtual EngineError open(const char* nameAndPath) = 0;

  /// Creates a timeline using a buffer which represents a MIDI file.
  /// @see  setEventCallback, close
  /// @param buffer The MIDI file buffer
  /// @param size The size of the buffer in bytes
  /// @return Relevant error or EngineError::OK
  virtual EngineError open(void* buffer, size_t size) = 0;

  /// Creates an empty timeline with a tempo, time signature, and length
  /// @see  setEventCallback, close
  /// @param tempo The tempo in quarter notes per minute
  /// @param timeSigNumerator The numerator of the time signature (the 2 in 2/4)
  /// @param timeSigDenominator The denominator of the time signature (the 4 in 2/4)
  /// @param lengthInBars The number of bars in the timeline (can be fractional)
  /// @return Relevant error or EngineError::OK
  virtual EngineError
  open(float tempo, uint8_t timeSigNumerator, uint8_t timeSigDenominator, float lengthInBars) = 0;

  /// Close an open file and release resources
  virtual void close() = 0;

  /// @return Whether there is currently a timeline loaded
  virtual bool isOpen() const = 0;

  /// Set a function to receive events from this object.
  /// @param callback Event callback function
  /// @param userData User data. Can be nullptr
  /// @return Relevant error or EngineError::OK
  virtual EngineError setEventCallback(EventTransportCallback callback, void* userData) = 0;

  /// Get the events in the specified range.
  /// @param callback Event callback to collect events.
  /// This is not asynchronous and will be called on the same thread.
  /// @param userData User data. Can be nullptr
  /// @param startSample The start of the range in samples
  /// @param numSamples The number of samples in the range
  /// @return EngineError::OK or relevant error.
  virtual EngineError getEvents(
      EventTransportCallback callback,
      void* userData,
      size_t startSample,
      size_t numSamples) = 0;

  /// Set looping on/off.
  /// @param loop When true, the EventTransport will reset when reaching the end
  /// @return true (always)
  virtual bool enableLooping(bool loop) = 0;

  /// @return whether the object is looping or not
  virtual bool loopingEnabled() = 0;

  /// Set the speed multiplier.
  /// @param speed Speed multiplier, between a range of 0.001 and 4
  virtual void setSpeed(float speed) = 0;

  /// Returns the current speed
  /// @return the speed (as specified in setSpeed())
  virtual float getSpeed() const = 0;

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

  /// @return the elapsed playback time in samples. Thread safe.
  virtual size_t getElapsedTimeInSamples() const = 0;

  /// @return the elapsed playback time in milliseconds. Thread safe.
  virtual float getElapsedTimeInMs() const = 0;

  /// @return the elapsed playback time in bars (including fractional amount). Thread safe.
  virtual float getElapsedTimeInBars() const = 0;

  /// @return the duration of the asset in samples. Thread safe.
  virtual size_t getAssetDurationInSamples() const = 0;

  /// @return the duration of the asset in milliseconds. Thread safe.
  virtual float getAssetDurationInMs() const = 0;

  /// @return the duration of the asset in bars (can be fractional). Thread safe.
  virtual float getAssetDurationInBars() const = 0;

  /// @return the number of events in the timeline
  virtual size_t getEventCount() const = 0;

  /// @return the current tempo. Can return zero if the transport hasn't started.
  virtual float getTempo() const = 0;

  /// @return the current time signature numerator (the 2 in 2/4 time)
  virtual uint8_t getTimeSignatureNumerator() const = 0;

  /// @return the current time signature denominator (the 4 in 2/4 time)
  virtual uint8_t getTimeSignatureDenominator() const = 0;

 protected:
  virtual ~EventTransport() {}
};

} // namespace TBE

#endif // FBA_TBE_EVENTTRANSPORT_H
