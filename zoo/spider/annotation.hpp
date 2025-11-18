//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/aliases.h"

#include <boost/describe.hpp>
#include <string_view>

namespace zoo {
namespace spider {

template<typename D>
constexpr inline std::string_view annotation_v{};

namespace detail {

template<typename C, typename T>
constexpr C deduce_class(T(C::*));

template<auto Mem>
using Class = decltype(deduce_class(Mem));

} // namespace detail

template<auto Mem>
using Desc = bd::descriptor_by_pointer<bd::describe_members<detail::Class<Mem>, bd::mod_any_access>, Mem>;

} // namespace spider
} // namespace zoo

// NOTICE: This macro must be used at the global namespace
#define SPIDER_OAS_ANNOTATE_MEMBER(CLASSMEMBER, DESCRIPTION)                                                                               \
	template<>                                                                                                                             \
	constexpr inline std::string_view zoo::spider::annotation_v<zoo::spider::Desc<&CLASSMEMBER>>{ DESCRIPTION };
