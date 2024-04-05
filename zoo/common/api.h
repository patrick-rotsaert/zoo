//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

// clang-format off

#if defined _WIN32 || defined __CYGWIN__
  #define ZOO_API_IMPORT __declspec(dllimport)
  #define ZOO_API_EXPORT __declspec(dllexport)
  #define ZOO_API_LOCAL
#else
  #if __GNUC__ >= 4 // TODO: clang?
    #define ZOO_API_IMPORT __attribute__ ((visibility ("default")))
    #define ZOO_API_EXPORT __attribute__ ((visibility ("default")))
    #define ZOO_API_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define ZOO_API_IMPORT
    #define ZOO_API_EXPORT
    #define ZOO_API_LOCAL
  #endif
#endif

#ifdef ZOO_SHARED // compiled as a shared library
  #ifdef ZOO_SHARED_EXPORTS // defined if we are building the shared library
    #define ZOO_EXPORT ZOO_API_EXPORT
  #else
    #define ZOO_EXPORT ZOO_API_IMPORT
  #endif // ZOO_SHARED_EXPORTS
  #define ZOO_LOCAL ZOO_API_LOCAL
#else // compiled as a static library
  #define ZOO_EXPORT
  #define ZOO_LOCAL
#endif // ZOO_SHARED
