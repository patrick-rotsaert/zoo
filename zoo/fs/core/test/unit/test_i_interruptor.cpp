//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/core/iinterruptor.h"
#include <gtest/gtest.h>

namespace zoo {
namespace fs {

namespace {

class interruptor final : public iinterruptor
{
	bool interrupted_ = false;

public:
	void interrupt() override
	{
		this->interrupted_ = true;
	}

	bool is_interrupted() override
	{
		return this->interrupted_;
	}

	bool wait_for_interruption(std::chrono::milliseconds /*duration*/) override
	{
		return false;
	}
};

} // namespace

TEST(IInterruptorTests, test_throw_if_interrupted)
{
	auto i = interruptor{};
	EXPECT_NO_THROW(i.throw_if_interrupted());
	i.interrupt();
	EXPECT_ANY_THROW(i.throw_if_interrupted());
}

} // namespace fs
} // namespace zoo
