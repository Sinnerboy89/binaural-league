#ifndef FBA_TBE_IOSTREAM_H
#define FBA_TBE_IOSTREAM_H

/*
 * Copyright (c) 2018-present, Facebook, Inc.
 */

#pragma once

#include "TBE_AudioEngineDefinitions.h"

namespace TBE {
static const size_t IOSTREAM_OPERATION_FAIL = SIZE_MAX;

/// Abstract class for an input/output stream
class IOStream {
 public:
  enum class StreamOptions {
    READ_BINARY,
    WRITE_BINARY,
    READ_WRITE_BINARY,
  };

  virtual ~IOStream() {}

  /// Read data from a stream. Can return 0 if unsupported.
  /// @param data Buffer to write into
  /// @param numBytes Number of bytes to read
  /// @return Number of bytes written to data or OPERATION_FAIL
  virtual size_t read(void* data, size_t numBytes) = 0;

  /// Write data to the stream. Can return 0 if unsupported.
  /// @param data Buffer containing data to write to the stream
  /// @param numBytes Number of bytes to write
  /// @return Number of bytes written to the stream or OPERATION_FAIL
  virtual size_t write(void* data, size_t numBytes) = 0;

  /// @return Current position of the stream in bytes or OPERATION_FAIL
  virtual size_t getPosition() = 0;

  /// Set the absolute position of the stream in bytes
  /// @param pos Position in bytes
  /// @return True if successful
  virtual bool setPosition(int64_t pos) = 0;

  /// Set the position of the stream in bytes, similar to posix fseek.
  /// @param pos Position in bytes
  /// @param mode Same as posix fseek (SEEK_SET, SEEK_CUR)
  /// @return True if successful
  virtual bool setPosition(int64_t pos, int mode) = 0;

  /// Push byte back into input stream. Similar implementation to to posix ungetc.
  /// @param c Byte to be pushed back
  /// @return Upon successful completion it should return the byte pushed back after conversion.
  /// Otherwise, it shall return EOF.
  virtual int32_t pushBackByte(int c) = 0;

  /// @return Size of the stream in bytes or OPERATION_FAIL
  virtual size_t getSize() = 0;

  /// @return True if the stream supports seeking
  virtual bool canSeek() = 0;

  /// @return True if the stream is ready to be read from or written to
  virtual bool ready() const = 0;

  /// @return True if the end of the stream has been reached
  virtual bool endOfStream() = 0;

  /// @return the file descriptor, if available. Returns -1 if there's an error or unavailable.
  virtual int getFD() = 0;

  /// Create a basic file stream object to read/write files.
  /// @param file Path and name of file
  /// @param options If the file must read, write or both
  /// @param ad Optional AssetDecriptor for specifying a custom size and offset.
  /// @return New IOStream object
  API_EXPORT static IOStream*
  createFileStream(const char* file, StreamOptions options, AssetDescriptor ad = AssetDescriptor());

  /// Create a read-only stream from a buffer of memory
  /// @param sizeInBytes size of the memory in bytes
  /// @param offsetInBytes Offset in bytes from which to start reading the buffer
  /// @Return New IOStream object
  API_EXPORT static IOStream*
  createMemoryStream(void* buffer, size_t sizeInBytes, size_t offsetInBytes = 0);
};
} // namespace TBE

#endif // FBA_TBE_IOSTREAM_H
