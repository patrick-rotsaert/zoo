//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/squid/core/connectionpool.h"
#include "zoo/squid/core/ibackendconnection.h"
#include "zoo/squid/core/ibackendconnectionfactory.h"
#include "zoo/squid/core/ibackendstatement.h"

#include <memory>

namespace zoo {
namespace squid {
namespace {

// Minimal backend connection that tracks how many instances are currently alive.
class mock_backend_connection final : public ibackend_connection
{
	int& live_count_;

public:
	explicit mock_backend_connection(int& live_count)
	    : live_count_{ live_count }
	{
		++this->live_count_;
	}

	~mock_backend_connection() noexcept override
	{
		--this->live_count_;
	}

	std::unique_ptr<ibackend_statement> create_statement(std::string_view) override
	{
		return nullptr;
	}

	std::unique_ptr<ibackend_statement> create_prepared_statement(std::string_view) override
	{
		return nullptr;
	}

	void execute(const std::string&) override
	{
	}
};

class mock_factory final : public ibackend_connection_factory
{
	int& live_count_;

public:
	explicit mock_factory(int& live_count)
	    : live_count_{ live_count }
	{
	}

	std::shared_ptr<ibackend_connection> create_backend_connection(std::string_view) const override
	{
		return std::make_shared<mock_backend_connection>(this->live_count_);
	}
};

} // namespace

TEST(ConnectionPoolTests, AcquireExhaustAndRelease)
{
	int          live = 0;
	mock_factory factory{ live };

	connection_pool pool{ factory, "", 2 };
	EXPECT_EQ(live, 2);

	auto c1 = pool.acquire();
	auto c2 = pool.acquire();
	ASSERT_TRUE(c1);
	ASSERT_TRUE(c2);

	// The pool is now exhausted.
	EXPECT_EQ(pool.try_acquire(), nullptr);

	// Releasing a connection returns it to the pool so it can be acquired again.
	c1.reset();
	auto c3 = pool.try_acquire();
	EXPECT_TRUE(c3);

	// The underlying connections are recycled, never destroyed while the pool lives.
	EXPECT_EQ(live, 2);
}

// Regression test: a connection acquired from the pool may outlive the pool itself.
// Releasing it after the pool is destroyed must not touch the destroyed pool.
TEST(ConnectionPoolTests, ConnectionOutlivesPool)
{
	int          live = 0;
	mock_factory factory{ live };

	auto pool = std::make_unique<connection_pool>(factory, "", 1);
	EXPECT_EQ(live, 1);

	auto conn = pool->acquire();
	ASSERT_TRUE(conn);

	// Destroy the pool while the connection is still checked out.
	pool.reset();

	// The connection is still alive and usable; its eventual release must be a safe no-op.
	EXPECT_EQ(live, 1);
	conn->execute("noop");
	EXPECT_NO_THROW(conn.reset());
	EXPECT_EQ(live, 0);
}

} // namespace squid
} // namespace zoo
