//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/postgresql/resultset.h"
#include "zoo/squid/postgresql/tuple.h"
#include "zoo/squid/postgresql/detail/conversions.h"
#include "zoo/squid/postgresql/detail/ipqapi.h"
#include "zoo/common/conversion/conversion.h"
#include "zoo/common/conversion/conversion.h"
#include "zoo/common/misc/quoted_c.h"

#include <fmt/format.h>

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <map>
#include <string_view>

namespace zoo {
namespace squid {
namespace postgresql {
namespace {

std::uint64_t get_affected_rows(ipq_api* api, PGresult* res)
{
	const auto num = api->cmdTuples(res);
	if (!num || !(*num))
	{
		return 0ull;
	}
	return conversion::string_to_number<std::uint64_t>(num);
}

void ensure_field_not_null(const field& f)
{
	if (f.is_null())
	{
		throw std::logic_error{ fmt::format("Field {} is null", quoted_c(f.name())) };
	}
}

bool string_to_bool(std::string_view value)
{
	if (value == "t")
	{
		return true;
	}
	else if (value == "f")
	{
		return false;
	}
	else
	{
		throw std::invalid_argument{ fmt::format("Value {} is not a boolean", quoted_c(value)) };
	}
}

} // namespace

//================//
// resultset_data //
//================//

struct resultset_data
{
	ipq_api*                                    api;
	std::shared_ptr<PGresult>                   pgresult;
	PGresult*                                   res;
	std::uint64_t                               affected_rows;
	std::size_t                                 tuple_count;
	std::size_t                                 field_count;
	std::map<std::string_view, std::size_t>     field_name_index_map;
	mutable std::vector<std::unique_ptr<tuple>> cache_;

	explicit resultset_data(ipq_api* api, std::shared_ptr<PGresult> pgresult)
	    : api{ api }
	    , pgresult{ std::move(pgresult) }
	    , res{ this->pgresult.get() }
	    , affected_rows{ get_affected_rows(api, res) }
	    , tuple_count{ static_cast<std::size_t>(std::max(0, api->ntuples(res))) }
	    , field_count{ static_cast<std::size_t>(std::max(0, api->nfields(res))) }
	    , field_name_index_map{}
	    , cache_{ tuple_count }
	{
		for (std::size_t field_index = 0; field_index < field_count; ++field_index)
		{
			field_name_index_map[api->fname(res, field_index)] = field_index;
		}
	}
};

//===========//
// resultset //
//===========//

resultset::resultset(ipq_api* api, std::shared_ptr<PGresult> pgresult)
    : data_{ std::make_unique<resultset_data>(api, std::move(pgresult)) }
{
}

resultset::~resultset() noexcept = default;

resultset::resultset(resultset&& src)        = default;
resultset& resultset::operator=(resultset&&) = default;

std::uint64_t resultset::affected_rows() const
{
	return data_->affected_rows;
}

std::size_t resultset::size() const
{
	return data_->tuple_count;
}

bool resultset::empty() const
{
	return size() == 0u;
}

const tuple& resultset::get_tuple(std::size_t tuple_index) const
{
	if (tuple_index < data_->tuple_count)
	{
		auto& elem = data_->cache_.at(tuple_index);
		if (!elem)
		{
			elem = std::make_unique<tuple>(data_.get(), tuple_index);
		}
		return *elem;
	}
	else if (data_->tuple_count)
	{
		throw std::out_of_range{ fmt::format("Tuple index {} is out of range [0, {}]", tuple_index, data_->tuple_count - 1u) };
	}
	else
	{
		throw std::out_of_range{ "Resultset is empty" };
	}
}

resultset_iterator resultset::begin() const
{
	return resultset_iterator{ this, 0 };
}

resultset_iterator resultset::end() const
{
	return resultset_iterator{ this, static_cast<resultset_iterator::difference_type>(size()) };
}

//=======//
// tuple //
//=======//

tuple::tuple(const resultset_data* data, std::size_t tuple_index)
    : data_{ data }
    , fields_{}
{
	fields_.reserve(data->field_count);
	for (std::size_t field_index = 0; field_index < data->field_count; ++field_index)
	{
		if (data->api->getisnull(data->res, tuple_index, field_index))
		{
			fields_.emplace_back(data->api->fname(data->res, field_index));
		}
		else
		{
			fields_.emplace_back(data->api->fname(data->res, field_index), data->api->getvalue(data->res, tuple_index, field_index));
		}
	}
}

tuple::~tuple() = default;

tuple::tuple(tuple&&) noexcept = default;
tuple::tuple(const tuple&)     = default;

tuple& tuple::operator=(tuple&&) noexcept = default;
tuple& tuple::operator=(const tuple&)     = default;

std::vector<field>::const_iterator tuple::begin() const noexcept
{
	return fields_.begin();
};

std::vector<field>::const_iterator tuple::end() const noexcept
{
	return fields_.end();
}

const std::vector<field>& tuple::fields() const noexcept
{
	return fields_;
}

std::size_t tuple::size() const noexcept
{
	return fields_.size();
}

bool tuple::empty() const noexcept
{
	return fields_.empty();
}

const field& tuple::get_field(std::size_t field_index) const
{
	if (field_index < fields_.size())
	{
		return fields_.at(field_index);
	}
	else if (fields_.size())
	{
		throw std::out_of_range{ fmt::format("Field index {} is out of range [0, {}]", field_index, fields_.size() - 1u) };
	}
	else
	{
		throw std::out_of_range{ "Field set is empty" };
	}
}

const field& tuple::get_field(std::string_view field_name) const
{
	const auto it = data_->field_name_index_map.find(field_name);
	if (it == data_->field_name_index_map.end())
	{
		throw std::out_of_range{ fmt::format("Field {} not found", quoted_c(field_name)) };
	}
	else
	{
		return get_field(it->second);
	}
}

const field* tuple::find_field(std::string_view field_name) const
{
	const auto it = data_->field_name_index_map.find(field_name);
	if (it == data_->field_name_index_map.end())
	{
		return nullptr;
	}
	else
	{
		return &get_field(it->second);
	}
}

const field& tuple::operator[](std::size_t field_index) const
{
	return get_field(field_index);
}

const field& tuple::operator[](std::string_view field_name) const
{
	return get_field(field_name);
}

//=======//
// field //
//=======//

field::field(std::string_view name)
    : name_{ name }
    , value_{}
{
}

field::field(std::string_view name, std::string_view value)
    : name_{ name }
    , value_{ value }
{
}

bool field::is_null() const noexcept
{
	return !value_.has_value();
}

field::operator bool() const noexcept
{
	return value_.has_value();
}

std::string_view field::name() const noexcept
{
	return name_;
}

const std::optional<std::string_view>& field::value() const noexcept
{
	return value_;
}

std::string field::to_string() const
{
	ensure_field_not_null(*this);
	return std::string{ value_.value() };
}

std::string_view field::to_string_view() const noexcept
{
	ensure_field_not_null(*this);
	return value_.value();
}

byte_string field::to_byte_string() const
{
	ensure_field_not_null(*this);
	return hex_string_to_binary(value_.value());
}

bool field::to_bool() const
{
	ensure_field_not_null(*this);
	return string_to_bool(value_.value());
}

int field::to_int() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_number<int>(value_.value());
}

std::int8_t field::to_int8() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_number<std::int8_t>(value_.value());
}

std::int16_t field::to_int16() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_number<std::int16_t>(value_.value());
}

std::int32_t field::to_int32() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_number<std::int32_t>(value_.value());
}

std::int64_t field::to_int64() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_number<std::int64_t>(value_.value());
}

unsigned field::to_unsigned() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_number<unsigned>(value_.value());
}

std::uint8_t field::to_uint8() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_number<std::uint8_t>(value_.value());
}

std::uint16_t field::to_uint16() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_number<std::uint16_t>(value_.value());
}

std::uint32_t field::to_uint32() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_number<std::uint32_t>(value_.value());
}

std::uint64_t field::to_uint64() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_number<std::uint64_t>(value_.value());
}

float field::to_float() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_number<float>(value_.value());
}

double field::to_double() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_number<double>(value_.value());
}

long double field::to_double128() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_number<long double>(value_.value());
}

boost::posix_time::ptime field::to_posix_ptime() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_boost_ptime(value_.value());
}

boost::posix_time::time_duration field::to_posix_time_duration() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_boost_time_duration(value_.value());
}

boost::gregorian::date field::to_gregorian_date() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_boost_date(value_.value());
}

time_point field::to_time_point() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_time_point(value_.value());
}

date field::to_date() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_date(value_.value());
}

time_of_day field::to_time_of_day() const
{
	ensure_field_not_null(*this);
	return conversion::string_to_time_of_day(value_.value());
}

std::optional<std::string> field::to_optional_string() const
{
	return value_.transform([](const std::string_view v) { return std::string{ v }; });
}

std::optional<std::string_view> field::to_optional_string_view() const noexcept
{
	return value_;
}

std::optional<byte_string> field::to_optional_byte_string() const
{
	return value_.transform([](const std::string_view v) { return hex_string_to_binary(v); });
}

std::optional<bool> field::to_optional_bool() const
{
	return value_.transform([this](const std::string_view v) { return string_to_bool(v); });
}

std::optional<int> field::to_optional_int() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_number<int>(v); });
}

std::optional<std::int8_t> field::to_optional_int8() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_number<std::int8_t>(v); });
}

std::optional<std::int16_t> field::to_optional_int16() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_number<std::int16_t>(v); });
}

std::optional<std::int32_t> field::to_optional_int32() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_number<std::int32_t>(v); });
}

std::optional<std::int64_t> field::to_optional_int64() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_number<std::int64_t>(v); });
}

std::optional<unsigned> field::to_optional_unsigned() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_number<unsigned>(v); });
}

std::optional<std::uint8_t> field::to_optional_uint8() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_number<std::uint8_t>(v); });
}

std::optional<std::uint16_t> field::to_optional_uint16() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_number<std::uint16_t>(v); });
}

std::optional<std::uint32_t> field::to_optional_uint32() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_number<std::uint32_t>(v); });
}

std::optional<std::uint64_t> field::to_optional_uint64() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_number<std::uint64_t>(v); });
}

std::optional<float> field::to_optional_float() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_number<float>(v); });
}

std::optional<double> field::to_optional_double() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_number<double>(v); });
}

std::optional<long double> field::to_optional_double128() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_number<long double>(v); });
}

std::optional<boost::posix_time::ptime> field::to_optional_posix_ptime() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_boost_ptime(v); });
}

std::optional<boost::posix_time::time_duration> field::to_optional_posix_time_duration() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_boost_time_duration(v); });
}

std::optional<boost::gregorian::date> field::to_optional_gregorian_date() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_boost_date(v); });
}

std::optional<time_point> field::to_optional_time_point() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_time_point(v); });
}

std::optional<date> field::to_optional_date() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_date(v); });
}

std::optional<time_of_day> field::to_optional_time_of_day() const
{
	return value_.transform([](const std::string_view v) { return conversion::string_to_time_of_day(v); });
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
