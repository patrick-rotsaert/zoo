//
// Copyright (C) 2024-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/assert/source_location.hpp>

#if defined(ZOO_LOGGING_NO_PRETTY_FUNCTION)
namespace zoo::detail {
inline void current_function_helper()
{
#define ZOO_CURRENT_FUNCTION __func__
}
} // namespace zoo::detail
#define ZOO_CURRENT_LOCATION ::boost::source_location(__FILE__, __LINE__, ZOO_CURRENT_FUNCTION)
#else
#define ZOO_CURRENT_LOCATION BOOST_CURRENT_LOCATION
#endif
