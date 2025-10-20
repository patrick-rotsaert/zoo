//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/postgresql/resultsetiterator.h"
#include "zoo/squid/postgresql/resultset.h"
#include "zoo/squid/postgresql/tuple.h"

namespace zoo {
namespace squid {
namespace postgresql {

resultset_iterator::resultset_iterator(const resultset* p, difference_type i)
    : parent_{ p }
    , index_{ i }
{
}

resultset_iterator::reference resultset_iterator::operator*() const
{
	return parent_->get_tuple(index_);
}

resultset_iterator::pointer resultset_iterator::operator->() const
{
	return &parent_->get_tuple(index_);
}

resultset_iterator& resultset_iterator::operator++()
{
	++index_;
	return *this;
}

resultset_iterator resultset_iterator::operator++(int)
{
	auto tmp = *this;
	++(*this);
	return tmp;
}

resultset_iterator& resultset_iterator::operator--()
{
	--index_;
	return *this;
}

resultset_iterator resultset_iterator::operator--(int)
{
	auto tmp = *this;
	--(*this);
	return tmp;
}

resultset_iterator& resultset_iterator::operator+=(difference_type n)
{
	index_ += n;
	return *this;
}

resultset_iterator& resultset_iterator::operator-=(difference_type n)
{
	index_ -= n;
	return *this;
}

resultset_iterator resultset_iterator::operator+(difference_type n) const
{
	return resultset_iterator{ parent_, index_ + n };
}

resultset_iterator resultset_iterator::operator-(difference_type n) const
{
	return resultset_iterator{ parent_, index_ - n };
}

resultset_iterator::difference_type resultset_iterator::operator-(const resultset_iterator& other) const
{
	return index_ - other.index_;
}

resultset_iterator::reference resultset_iterator::operator[](difference_type n) const
{
	return parent_->get_tuple(index_ + n);
}

bool resultset_iterator::operator==(const resultset_iterator& other) const
{
	return index_ == other.index_ && parent_ == other.parent_;
}

bool resultset_iterator::operator!=(const resultset_iterator& other) const
{
	return !(*this == other);
}

bool resultset_iterator::operator<(const resultset_iterator& other) const
{
	return index_ < other.index_;
}

bool resultset_iterator::operator>(const resultset_iterator& other) const
{
	return index_ > other.index_;
}

bool resultset_iterator::operator<=(const resultset_iterator& other) const
{
	return index_ <= other.index_;
}

bool resultset_iterator::operator>=(const resultset_iterator& other) const
{
	return index_ >= other.index_;
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
