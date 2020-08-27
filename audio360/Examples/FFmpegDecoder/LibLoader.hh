#ifndef FBA_LIBLOADER_H
#define FBA_LIBLOADER_H

/*
 *  Copyright 2013-Present Two Big Ears Limited
 *  All rights reserved.
 *  http://twobigears.com
 */

#pragma once

#include <memory>
#include <string>
#include "PlatformConditionals.h"

/// Macro to stringify values
#define TBE_STRINGIFY(value) #value

/// Declares a function pointer type of the function that needs to be loaded
/// from the shared library.
/// @param symbolName the function that must be declared and eventually bound
/// Eg:
/// // Function in shared lib
///	void helloWorld()
///
/// // Declare a pointer to that function:
/// TBE_DECL_FUNCT_PTR(helloWorld)
///
/// Use TBE_LOAD_FUNC to find the symbol in the shared lib and bind it to
/// this function pointer.
/// The function can then be used by appending "_ptr" to the function name. Eg: helloWorld_ptr();
#define TBE_DECL_FUNC_PTR(symbolName) decltype(&symbolName) symbolName##_ptr

#ifdef TBE_WIN

#include <Windows.h>
namespace TBE {
using Handle = HMODULE;
}

/// Load a function from a shared library and bind it to the function pointer declared
/// using TBE_DECL_FUNC_PTR
/// @param handle The shared library handle
/// @param symbolName The function that must be found and bound
#define TBE_LOAD_FUNC(handle, symbolName) \
  symbolName##_ptr = (decltype(&symbolName))GetProcAddress(handle, TBE_STRINGIFY(symbolName))

#define TBE_LOAD_DLL(handle, path, name, suffix)                 \
  do {                                                           \
    handle = LoadLibrary((path + "//" + name + suffix).c_str()); \
  } while (0);

#define TBE_FREE_DLL(handle) \
  do {                       \
    FreeLibrary(handle_);    \
  } while (0);

#elif TBE_OSX
#include <dlfcn.h>
namespace TBE {
using Handle = void*;
}
#define TBE_LOAD_FUNC(handle, symbolName) \
  symbolName##_ptr = (decltype(&symbolName))dlsym(handle, TBE_STRINGIFY(symbolName))

#define TBE_LOAD_DLL(handle, path, name, suffix)                      \
  do {                                                                \
    handle = dlopen((path + "/" + name + suffix).c_str(), RTLD_LAZY); \
  } while (0);

#define TBE_FREE_DLL(handle) \
  do {                       \
    dlclose(handle_);        \
  } while (0);

#endif

/// Load a function from a shared library and bind it to the function pointer declared
/// using TBE_DECL_FUNC_PTR. Throws a std::runtime_exception if the symbol cannot be found
/// @param handle The shared library handle
/// @param symbolName The function that must be found and bound
/// @exception std::runtime_error
#define TBE_LOAD_FUNC_OR_THROW(handle, symbolName)                                                 \
  do {                                                                                             \
    TBE_LOAD_FUNC(handle, symbolName);                                                             \
    if (!symbolName##_ptr) {                                                                       \
      throw std::runtime_error("Failed to find symbol " + std::string(TBE_STRINGIFY(symbolName))); \
    }                                                                                              \
  } while (0)

namespace TBE {

class LibLoader {
 public:
  using UPtr = std::unique_ptr<LibLoader>;

  /// Construct object and load library. Throws std::runtime_error if library cannot be found.
  /// @param path Full path to library. Can be empty if the library is in the exe search path
  /// @param name Library name without suffix (eg: libavutil not libavutil.dll)
  /// @param suffix .dll, .so or .dylib
  /// @exception std::runtime_error
  LibLoader(
      const std::string& path,
      const std::string& name,
      const std::string& suffix = std::string("")) {
    TBE_LOAD_DLL(handle_, path, name, suffix);
    if (!handle_) {
      throw std::runtime_error("Failed to load " + name + suffix + " from " + path);
    }
  }

  ~LibLoader() {
    TBE_FREE_DLL(handle_);
    handle_ = nullptr;
  }

  Handle getHandle() {
    return handle_;
  }

  void* getFunction(const std::string& function) {
#ifdef WIN32
    void* res = GetProcAddress(handle_, function.c_str());
#else
    auto res = dlsym(handle_, function.c_str());
#endif

    return res;
  }

 private:
  Handle handle_;
};
} // namespace TBE

#endif // FBA_LIBLOADER_H
