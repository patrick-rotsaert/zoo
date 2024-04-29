//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/core/connectionpool.h"
#include "zoo/squid/core/ibackendconnection.h"
#include "zoo/squid/core/ibackendconnectionfactory.h"
#include "zoo/squid/core/ibackendstatement.h"
#include "zoo/squid/core/error.h"

#include "zoo/common/misc/throw_exception.h"

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace zoo {
namespace squid {

namespace {

class backend_connection_wrapper final : public ibackend_connection
{
	using release_function_type = std::function<void(std::shared_ptr<ibackend_connection>&&)>;

	std::shared_ptr<ibackend_connection> connection_;
	release_function_type                release_function_;

	std::unique_ptr<ibackend_statement> create_statement(std::string_view query) override
	{
		return this->connection_->create_statement(query);
	}

	std::unique_ptr<ibackend_statement> create_prepared_statement(std::string_view query) override
	{
		return this->connection_->create_prepared_statement(query);
	}

	void execute(const std::string& query) override
	{
		this->connection_->execute(query);
	}

public:
	explicit backend_connection_wrapper(std::shared_ptr<ibackend_connection>&& connection, release_function_type&& release_function)
	    : connection_{ std::move(connection) }
	    , release_function_{ std::move(release_function) }
	{
	}

	~backend_connection_wrapper()
	{
		this->release_function_(std::move(this->connection_));
	}
};

} // namespace

class connection_pool::impl final
{
	std::queue<std::shared_ptr<ibackend_connection>> queue_;
	std::mutex                                       mutex_;
	std::condition_variable                          cv_;

	using lock_type = std::unique_lock<std::mutex>;

public:
	impl(const ibackend_connection_factory& factory, std::string_view connection_info, std::size_t count)
	{
		if (count == 0)
		{
			ZOO_THROW_EXCEPTION(std::invalid_argument{ "count must be greater than zero" });
		}

		while (count--)
		{
			this->queue_.push(factory.create_backend_connection(connection_info));
		}
	}

	/// Waits indefinitely until the pool has a connection available.
	std::shared_ptr<ibackend_connection> acquire()
	{
		lock_type lock(mutex_);

		while (this->queue_.empty())
		{
			this->cv_.wait(lock);
		}

		return this->acquire_from_front_of_queue(lock);
	}

	/// Returns nullptr if no connection is available within the specified timeout.
	std::shared_ptr<ibackend_connection> acquire(const std::chrono::milliseconds& timeout)
	{
		lock_type lock(mutex_);

		while (this->queue_.empty())
		{
			if (this->cv_.wait_for(lock, timeout) == std::cv_status::timeout)
			{
				return nullptr;
			}
		}

		return this->acquire_from_front_of_queue(lock);
	}

	std::shared_ptr<ibackend_connection> try_acquire()
	{
		lock_type lock(mutex_);

		if (this->queue_.empty())
		{
			return nullptr;
		}
		else
		{
			return this->acquire_from_front_of_queue(lock);
		}
	}

private:
	std::shared_ptr<ibackend_connection> acquire_from_front_of_queue(const lock_type&)
	{
		auto connection = this->queue_.front();
		this->queue_.pop();

		return std::make_shared<backend_connection_wrapper>(
		    std::move(connection), [this](std::shared_ptr<ibackend_connection>&& connection) { this->release(std::move(connection)); });
	}

	void release(std::shared_ptr<ibackend_connection>&& connection)
	{
		{
			lock_type lock(mutex_);
			this->queue_.push(connection);
		}
		this->cv_.notify_one();
	}
};

connection_pool::connection_pool(const ibackend_connection_factory& factory, std::string_view connection_info, std::size_t count)
    : pimpl_{ std::make_unique<impl>(factory, connection_info, count) }
{
}

connection_pool::~connection_pool() noexcept = default;

connection_pool::connection_pool(connection_pool&&) = default;

connection_pool& connection_pool::operator=(connection_pool&&) = default;

std::shared_ptr<ibackend_connection> connection_pool::acquire()
{
	return this->pimpl_->acquire();
}

std::shared_ptr<ibackend_connection> connection_pool::acquire(const std::chrono::milliseconds& timeout)
{
	return this->pimpl_->acquire(timeout);
}

std::shared_ptr<ibackend_connection> connection_pool::try_acquire()
{
	return this->pimpl_->try_acquire();
}

} // namespace squid
} // namespace zoo
