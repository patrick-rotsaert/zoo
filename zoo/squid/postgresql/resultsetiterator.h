//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/postgresql/resultsetfwd.h"
#include "zoo/squid/postgresql/tuplefwd.h"

#include <iterator>

namespace zoo {
namespace squid {
namespace postgresql {

class resultset_iterator
{
public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type        = tuple;
	using difference_type   = std::ptrdiff_t;
	using pointer           = const tuple*;
	using reference         = const tuple&;

	explicit resultset_iterator(const resultset* c, difference_type i);

	reference operator*() const;
	pointer   operator->() const;

	resultset_iterator& operator++();
	resultset_iterator  operator++(int);
	resultset_iterator& operator--();
	resultset_iterator  operator--(int);
	resultset_iterator& operator+=(difference_type n);
	resultset_iterator& operator-=(difference_type n);
	resultset_iterator  operator+(difference_type n) const;
	resultset_iterator  operator-(difference_type n) const;
	difference_type     operator-(const resultset_iterator& other) const;

	reference operator[](difference_type n) const;

	bool operator==(const resultset_iterator& other) const;
	bool operator!=(const resultset_iterator& other) const;
	bool operator<(const resultset_iterator& other) const;
	bool operator>(const resultset_iterator& other) const;
	bool operator<=(const resultset_iterator& other) const;
	bool operator>=(const resultset_iterator& other) const;

private:
	const resultset* parent_;
	difference_type  index_;
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
