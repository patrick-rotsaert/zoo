//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/throw_exception.hpp>

#define ZOO_THROW_EXCEPTION(...) BOOST_THROW_EXCEPTION((__VA_ARGS__))
