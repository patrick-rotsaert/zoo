//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/core/basicstatement.h"
#include "zoo/squid/core/ibackendstatement.h"

#include "zoo/common/misc/throw_exception.h"

#include <cassert>

namespace zoo {
namespace squid {

basic_statement::basic_statement(std::shared_ptr<ibackend_connection> connection, std::unique_ptr<ibackend_statement>&& statement)
    : parameters_{}
    , results_{}
    , named_results_{}
    , connection_{ connection }
    , statement_{ std::move(statement) }
{
}

basic_statement::basic_statement(std::shared_ptr<ibackend_connection> connection)
    : parameters_{}
    , results_{}
    , named_results_{}
    , connection_{ connection }
    , statement_{}
{
}

std::ostream& basic_statement::query()
{
	this->statement_.reset();
	if (this->query_)
	{
		return this->query_.value();
	}
	else
	{
		return this->query_.emplace();
	}
}

basic_statement::~basic_statement() noexcept
{
}

basic_statement& basic_statement::bind(std::string_view name, const unsigned char* value, std::size_t size)
{
	this->upsert_parameter(name, byte_string_view{ value, size }, parameter::by_value{});
	return *this;
}

void basic_statement::execute()
{
	if (!this->statement_)
	{
		if (!this->query_)
		{
			ZOO_THROW_EXCEPTION(error{ "No statement nor query have has been provided" });
		}
		else
		{
			this->statement_ = this->create_statement(this->connection_, this->query_.value().str());
		}
	}

	if (!this->results_.empty() && !this->named_results_.empty())
	{
		ZOO_THROW_EXCEPTION(error{ "Named result binding cannot be combined with sequential result binding" });
	}
	if (!this->named_results_.empty())
	{
		this->statement_->execute(this->parameters_, this->named_results_);
	}
	else
	{
		this->statement_->execute(this->parameters_, this->results_);
	}
}

void basic_statement::bind_execute(std::initializer_list<std::pair<std::string_view, parameter_by_value>> params)
{
	this->parameters_.clear();
	for (auto&& pair : params)
	{
		this->parameters_.insert_or_assign(std::string{ pair.first }, std::move(pair.second));
	}
	this->execute();
}

void basic_statement::bind_ref_execute(std::initializer_list<std::pair<std::string_view, parameter_by_reference>> params)
{
	this->parameters_.clear();
	for (auto&& pair : params)
	{
		this->parameters_.insert_or_assign(std::string{ pair.first }, std::move(pair.second));
	}
	this->execute();
}

bool basic_statement::fetch()
{
	if (this->statement_)
	{
		return this->statement_->fetch();
	}
	else
	{
		ZOO_THROW_EXCEPTION(error{ "No statement has been created" });
	}
}

std::size_t basic_statement::field_count()
{
	if (!this->statement_)
	{
		ZOO_THROW_EXCEPTION(error{ "No statement has been created" });
	}
	return this->statement_->field_count();
}

std::string basic_statement::field_name(std::size_t index)
{
	if (!this->statement_)
	{
		ZOO_THROW_EXCEPTION(error{ "No statement has been created" });
	}
	return this->statement_->field_name(index);
}

uint64_t basic_statement::affected_rows()
{
	if (!this->statement_)
	{
		ZOO_THROW_EXCEPTION(error{ "No statement has been created" });
	}
	return this->statement_->affected_rows();
}

ibackend_statement& basic_statement::backend_statement() const
{
	if (!this->statement_)
	{
		ZOO_THROW_EXCEPTION(error{ "No statement has been created" });
	}
	else
	{
		return *this->statement_;
	}
}

} // namespace squid
} // namespace zoo
