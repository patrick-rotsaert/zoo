//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/postgresql/resultsetdatafwd.h"
#include "zoo/squid/postgresql/resultsetiterator.h"
#include "zoo/squid/postgresql/detail/libpqfwd.h"

#include <memory>

namespace zoo {
namespace squid {
namespace postgresql {

class ipq_api; //@@
class tuple;   //@@

class resultset final
{
	std::unique_ptr<resultset_data> data_;

public:
	explicit resultset(ipq_api* api, std::shared_ptr<PGresult> pgresult);
	~resultset() noexcept;

	resultset(const resultset&) = delete;
	resultset(resultset&& src);
	resultset& operator=(const resultset&) = delete;
	resultset& operator=(resultset&&);

	std::uint64_t affected_rows() const;

	std::size_t size() const;
	bool        empty() const;

	const tuple& get_tuple(std::size_t tuple_index) const;

	resultset_iterator begin() const;
	resultset_iterator end() const;
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
