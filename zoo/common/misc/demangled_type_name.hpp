//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/misc/demangle.h"

#include <typeinfo>

namespace zoo {

template<typename T>
inline std::string demangled_type_name()
{
	return demangle(typeid(T).name());
}

template<typename T>
inline std::string demangled_type_name(T* ptr)
{
	return demangle(typeid(*ptr).name());
}

template<typename T>
inline std::string demangled_type_name(const T& arg)
{
	return demangle(typeid(arg).name());
}

} // namespace zoo
