#include "zoo/tui/asiotimer.h"

#include <boost/asio/steady_timer.hpp>

#include <chrono>

namespace zoo {
namespace tui {

class asio_timer::impl
{
	boost::asio::steady_timer deadline_;
	bool                      cancelled_;

	void set_deadline(int msec)
	{
		this->deadline_.expires_after(std::chrono::milliseconds{ msec });
	}

public:
	explicit impl(boost::asio::io_context& ioc)
	    : deadline_{ ioc }
	    , cancelled_{}
	{
	}

	template<typename T>
	void start(const T& time, std::function<void()> work)
	{
		this->deadline_.cancel();
		this->cancelled_ = false;
		this->set_deadline(time);
		this->deadline_.async_wait([this, work](boost::system::error_code ec) {
			if (!ec)
			{
				if (!this->cancelled_)
				{
					if (this->deadline_.expiry() <= std::chrono::steady_clock::now())
					{
						work();
					}
				}
			}
		});
	}

	void cancel()
	{
		this->cancelled_ = true;
		this->deadline_.cancel();
	}
};

asio_timer::asio_timer(boost::asio::io_context& ioc)
    : pimpl_(std::make_unique<impl>(ioc))
{
}

asio_timer::~asio_timer() noexcept
{
}

void asio_timer::start(int msec, std::function<void()> work)
{
	pimpl_->start(msec, work);
}

void asio_timer::cancel()
{
	pimpl_->cancel();
}

} // namespace tui
} // namespace zoo
