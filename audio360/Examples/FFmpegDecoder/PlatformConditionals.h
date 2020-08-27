#ifndef FBA_PLATFORMCONDITIONALS_H
#define FBA_PLATFORMCONDITIONALS_H

/*
 *  Copyright 2013-Present Two Big Ears Limited
 *  All rights reserved.
 *  http://twobigears.com
 */

#pragma once

// NOTE: Make sure that this file doesn't have any C++
// code. It has to be C compatible

#if defined(__ANDROID__)
#define TBE_ANDROID 1
#elif defined(__APPLE__)
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR
#define TBE_IOS_SIM 1
#define TBE_IOS 1
#elif TARGET_OS_IPHONE
#define TBE_IOS 1
#else
#define TBE_OSX 1
#endif
#elif defined(_WIN32)
#include <stdarg.h>
#define TBE_WIN 1
#elif defined(TBE_EMSCRIPTEN)
#define TBE_WEB 1
#elif defined(__linux__)
#define TBE_LINUX 1
#elif defined(XROS)
#define TBE_XROS 1
#else
#error Target OS not specified
#endif

// Windows store/universal windows apps.
// TBE_CMAKE_WINDOWS_UWP should be defined in CMake/build system
#if defined(TBE_CMAKE_WINDOWS_UWP)
#define TBE_WIN_UWP 1
#endif

#if defined(TBE_WIN)
#if defined(_WIN64) || defined(__MINGW64__)
#define TBE_64BIT
#else
#define TBE_32BIT
#endif
#endif

#if defined(TBE_OSX) || defined(TBE_IOS)
#if defined(__LP64__)
#define TBE_64BIT
#else
#define TBE_32BIT
#endif
#endif

#if defined(TBE_ANDROID) || defined(TBE_LINUX) || defined(TBE_WEB) || defined(TBE_XROS)
#if defined(__LP64__) || defined(_LP64) || defined(__arm64__) || defined(XROS)
#define TBE_64BIT
#else
#define TBE_32BIT
#endif
#endif

#endif // FBA_PLATFORMCONDITIONALS_H
