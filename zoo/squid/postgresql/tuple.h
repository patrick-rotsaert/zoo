//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/postgresql/field.h"
#include "zoo/squid/postgresql/resultsetdatafwd.h"

#include <vector>

namespace zoo {
namespace squid {
namespace postgresql {

class tuple final
{
	const resultset_data* data_;
	std::vector<field>    fields_;

public:
	explicit tuple(const resultset_data* data, std::size_t tuple_index);

	~tuple();

	tuple(tuple&&) noexcept;
	tuple(const tuple&);

	tuple& operator=(tuple&&) noexcept;
	tuple& operator=(const tuple&);

	std::vector<field>::const_iterator begin() const noexcept;
	std::vector<field>::const_iterator end() const noexcept;

	const std::vector<field>& fields() const noexcept;

	std::size_t size() const noexcept;
	bool        empty() const noexcept;

	const field& get_field(std::size_t field_index) const;
	const field& get_field(std::string_view field_name) const;
	const field* find_field(std::string_view field_name) const;

	const field& operator[](std::size_t field_index) const;
	const field& operator[](std::string_view field_name) const;
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
