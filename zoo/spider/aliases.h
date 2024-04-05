//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/status.hpp>

#include <memory>

//
// Forward declarations
//

namespace boost {

namespace core {

template<class Ch>
class basic_string_view;

using string_view = basic_string_view<char>;

} // namespace core

namespace system {

class error_code;

}

namespace asio {

class io_context;

namespace ip {
class tcp;
}
} // namespace asio

namespace beast {

using string_view = boost::core::string_view;
using error_code  = boost::system::error_code;

namespace http {

class message_generator;

} // namespace http

namespace websocket {
}

} // namespace beast

namespace urls {
class url_view;
}

namespace filesystem {
class path;
}

} // namespace boost

//
// Aliases
//

namespace zoo {
namespace spider {

namespace beast     = boost::beast;
namespace http      = beast::http;
namespace websocket = beast::websocket;
namespace net       = boost::asio;
namespace fs        = boost::filesystem;

using error_code        = beast::error_code;
using string_view       = beast::string_view;
using url_view          = boost::urls::url_view;
using message_generator = http::message_generator;
using status            = http::status;
using verb              = http::verb;
using tcp               = net::ip::tcp;

} // namespace spider
} // namespace zoo
