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
