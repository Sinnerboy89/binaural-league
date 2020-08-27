/*
 * Copyright (c) 2018-present, Facebook, Inc.
 */

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include "Audio360FfmpegDecoder.h"

// for GetExePath()
#ifdef WIN32
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#elif __APPLE__
#include <mach-o/dyld.h>
#else // Linux and Android
#include <limits.h>
#include <unistd.h>
#endif

using namespace TBE;

std::string GetExePath();
std::string GetFFmpegPath();

int main(int argc, char* argv[]) {
  //-------------------------------------
  // Load File
  //-------------------------------------

  auto file = GetExePath() + "HansVoice_FB360_H264_Opus.mkv";
  if (argc > 1) {
    file = argv[1];
  }

  //-------------------------------------
  // Load file, decode and playback
  //-------------------------------------

  using Status = Audio360FfmpegDecoder::Status;
  Status status{Status::OK};

  Audio360FfmpegDecoder decoder(GetFFmpegPath());

  // set this to false and call decoder.getAudioMix() if want access to spatialized buffers
  const auto use_audio_device = true;

  // Open file and set everything up
  if (decoder.open(file, use_audio_device) != Status::OK) {
    std::cerr << "Failed to load file: " << file << std::endl;
  }

  decoder.startAudioDevice();
  decoder.play();

  // You can set the listener/camera orientation using this method:
  // decoder.setListenerRotation

  // Get spatialized two channel buffers if the audio device is disabled above:
  // decoder.getAudioMix(buffer, numSamples);

  while (status == Status::OK) {
    status = decoder.decode();
    // Arbitrary sleep, for this example.
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  if (status == Status::END_OF_STREAM) {
    std::cout << "End of file reached, exiting!" << std::endl;
  }

  return 0;
}

std::string GetFFmpegPath() {
#ifdef WIN32
  return GetExePath() + "bin";
#else
  return "/usr/local/lib";
#endif
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
