/*
 * Copyright (c) 2018-present, Facebook, Inc.
 */

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "TBE_AudioEngine.h"

// for GetExePath()
#ifdef WIN32
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#elif __APPLE__
#include <mach-o/dyld.h>
#include <sys/param.h>
#else // Linux and Android
#include <limits.h>
#include <unistd.h>
#endif

#define THROW_IF_ERROR(error)                                                                    \
  do {                                                                                           \
    if (error != EngineError::OK) {                                                              \
      throw std::runtime_error("Error initialising AudioEngine: " + std::to_string((int)error)); \
    }                                                                                            \
  } while (0)

using namespace TBE;

/// An example for playing back a spatial audio file
class DecoderFileExample {
 public:
  DecoderFileExample(const std::string& file) : engine_(nullptr), spatFile_(nullptr) {
    // Create the audio engine object with default settings
    auto err = TBE_CreateAudioEngine(engine_);
    THROW_IF_ERROR(err);

    // A new instance SpatDecoderFile must be created from the engine's internal pool of objects
    err = engine_->createSpatDecoderFile(spatFile_);
    THROW_IF_ERROR(err);

    // The engine's audiod device and mixer must be started
    engine_->start();

    // Setup an event callback to know when the file is ready for playback
    spatFile_->setEventCallback(
        [](Event event, void* userData) {
          if (event == Event::DECODER_INIT) {
            static_cast<SpatDecoderFile*>(userData)->play();
          }
        },
        spatFile_);

    // Open the file, it should trigger the event callback right away
    err = spatFile_->open(file.c_str());
    THROW_IF_ERROR(err);
  }

  ~DecoderFileExample() {
    if (engine_) {
      TBE_DestroyAudioEngine(engine_);
    }
  }

 private:
  AudioEngine* engine_;
  SpatDecoderFile* spatFile_;
};

/// An example for playing back a spatial audio stream. In this case, it is a raw header-less 16 bit
/// audio file
class DecoderQueueExample {
 public:
  DecoderQueueExample(const std::string& rawFile)
      : enqueue_(true),
        rawFile_(rawFile, std::ios_base::in | std::ios_base::binary),
        engine_(nullptr),
        spatQueue_(nullptr),
        yawRotation_(0) {
    if (!rawFile_.is_open())
      throw std::runtime_error("Could not open " + rawFile);

    // Create the audio engine with default settings
    auto err = TBE_CreateAudioEngine(engine_);
    THROW_IF_ERROR(err);
    // Create the SpatDecoderQueue object from the engine's internal object pool
    err = engine_->createSpatDecoderQueue(spatQueue_);
    THROW_IF_ERROR(err);

    /// Start a thread to enqueue data
    enqueueThread_ = std::thread([this]() {
      while (enqueue_) {
        // The ChannelMap is used to specify the spatial audio format in the file.
        // TBE_8_2 is 8 channels of spatial audio and 2 channels of head-locked audio
        const ChannelMap map = ChannelMap::TBE_8_2;

        // Create a buffer to hold enough channels of interleaved data
        std::vector<int16_t> buffer;
        buffer.resize(512 * getNumChannelsForMap(map));

        if (!rawFile_.eof() && spatQueue_->getFreeSpaceInQueue(map) > buffer.size()) {
          // Enqueue data until the queue is full or the end of the file is reached
          rawFile_.read((char*)buffer.data(), buffer.size() * sizeof(int16_t));
          spatQueue_->enqueueData(buffer.data(), (int)buffer.size(), map);
        }
      }
    });

    // Start the audio engine's audio device and mixer
    engine_->start();

    // Fade the audio in!
    spatQueue_->playWithFade(10000 /* ms */);
  }

  bool rotate() {
    // Rotate the listener by specifying the yaw angle (-180 to 180 degrees)
    engine_->setListenerRotation((yawRotation_ > 180) ? yawRotation_ - 360 : yawRotation_, 0, 0);
    yawRotation_ = (yawRotation_ + 1);
    return (yawRotation_ < 360);
  }

  ~DecoderQueueExample() {
    enqueue_ = false;
    enqueueThread_.detach();

    if (engine_) {
      TBE_DestroyAudioEngine(engine_);
    }
  }

 private:
  bool enqueue_;
  std::ifstream rawFile_;
  AudioEngine* engine_;
  SpatDecoderQueue* spatQueue_;
  std::thread enqueueThread_;
  int yawRotation_;
};

std::string GetExePath();

int main(int argc, const char* argv[]) {
  std::cout
      << "\n\nThis is a simple example that that demonstrates playback of files and streams. \n\n"
      << std::endl;

  {
    try {
      DecoderFileExample decoder(GetExePath() + "HansDirectionsOnly.tbe");
      std::this_thread::sleep_for(std::chrono::seconds(31));
    } catch (std::exception& e) {
      std::cout << "FAILED! " << e.what() << "\n";
    }
  }

  std::cout << "\n\nNow, let's decode and enqueue a stream of data and fade it in \n" << std::endl;

  {
    try {
      DecoderQueueExample decoder(GetExePath() + "HansOneFish_44100_16bit_10ch.raw");

      std::cout << "\n\nAnd rotate the listener \n\n\n";
      while (decoder.rotate()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(75));
      }
    } catch (std::exception& e) {
      std::cout << "FAILED! " << e.what() << std::endl;
    }
  }

  return 0;
}

std::string GetExePath() {
#ifdef WIN32
  TCHAR dest[MAX_PATH];
  DWORD length = GetModuleFileName(NULL, dest, MAX_PATH);
  PathRemoveFileSpec(dest);
  std::string pathString(dest);
  std::replace(pathString.begin(), pathString.end(), '\\', '/');
  return (pathString + "/");
#elif __APPLE__
  char path[PATH_MAX];
  char realPath[PATH_MAX];
  memset(path, 0, PATH_MAX);
  memset(realPath, 0, PATH_MAX);
  uint32_t size = sizeof(path);
  _NSGetExecutablePath(path, &size);
  realpath(path, realPath);
  std::string exePath(realPath);
  return (exePath.substr(0, exePath.find_last_of("\\/")) + "/");
#else // Linux and Android
  char path[PATH_MAX];
  memset(path, 0, PATH_MAX);
  readlink("/proc/self/exe", path, PATH_MAX);
  std::string exePath(path);
  return (exePath.substr(0, exePath.find_last_of("\\/")) + "/");
#endif
}
