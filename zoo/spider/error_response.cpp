//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/error_response.h"
#include "zoo/spider/message.h"

#include <boost/beast/version.hpp>

#include <string>
#include <sstream>
#include <map>

namespace zoo {
namespace spider {

namespace {

std::string make_stock_reply(http::status status)
{
	const auto         phrase = http::obsolete_reason(status);
	std::ostringstream out;
	out << "<html>"
	       "<head><title>"
	    << phrase
	    << "</title></head>"
	       "<body><h1>"
	    << static_cast<int>(status) << " " << phrase
	    << "</h1></body>"
	       "</html>\r\n";
	return out.str();
}

std::map<status, std::string> make_stock_reply_map()
{
	auto map                                        = std::map<status, std::string>{};
	map[status::continue_]                          = make_stock_reply(status::continue_);
	map[status::switching_protocols]                = make_stock_reply(status::switching_protocols);
	map[status::processing]                         = make_stock_reply(status::processing);
	map[status::ok]                                 = make_stock_reply(status::ok);
	map[status::created]                            = make_stock_reply(status::created);
	map[status::accepted]                           = make_stock_reply(status::accepted);
	map[status::non_authoritative_information]      = make_stock_reply(status::non_authoritative_information);
	map[status::no_content]                         = make_stock_reply(status::no_content);
	map[status::reset_content]                      = make_stock_reply(status::reset_content);
	map[status::partial_content]                    = make_stock_reply(status::partial_content);
	map[status::multi_status]                       = make_stock_reply(status::multi_status);
	map[status::already_reported]                   = make_stock_reply(status::already_reported);
	map[status::im_used]                            = make_stock_reply(status::im_used);
	map[status::multiple_choices]                   = make_stock_reply(status::multiple_choices);
	map[status::moved_permanently]                  = make_stock_reply(status::moved_permanently);
	map[status::found]                              = make_stock_reply(status::found);
	map[status::see_other]                          = make_stock_reply(status::see_other);
	map[status::not_modified]                       = make_stock_reply(status::not_modified);
	map[status::use_proxy]                          = make_stock_reply(status::use_proxy);
	map[status::temporary_redirect]                 = make_stock_reply(status::temporary_redirect);
	map[status::permanent_redirect]                 = make_stock_reply(status::permanent_redirect);
	map[status::bad_request]                        = make_stock_reply(status::bad_request);
	map[status::unauthorized]                       = make_stock_reply(status::unauthorized);
	map[status::payment_required]                   = make_stock_reply(status::payment_required);
	map[status::forbidden]                          = make_stock_reply(status::forbidden);
	map[status::not_found]                          = make_stock_reply(status::not_found);
	map[status::method_not_allowed]                 = make_stock_reply(status::method_not_allowed);
	map[status::not_acceptable]                     = make_stock_reply(status::not_acceptable);
	map[status::proxy_authentication_required]      = make_stock_reply(status::proxy_authentication_required);
	map[status::request_timeout]                    = make_stock_reply(status::request_timeout);
	map[status::conflict]                           = make_stock_reply(status::conflict);
	map[status::gone]                               = make_stock_reply(status::gone);
	map[status::length_required]                    = make_stock_reply(status::length_required);
	map[status::precondition_failed]                = make_stock_reply(status::precondition_failed);
	map[status::payload_too_large]                  = make_stock_reply(status::payload_too_large);
	map[status::uri_too_long]                       = make_stock_reply(status::uri_too_long);
	map[status::unsupported_media_type]             = make_stock_reply(status::unsupported_media_type);
	map[status::range_not_satisfiable]              = make_stock_reply(status::range_not_satisfiable);
	map[status::expectation_failed]                 = make_stock_reply(status::expectation_failed);
	map[status::misdirected_request]                = make_stock_reply(status::misdirected_request);
	map[status::unprocessable_entity]               = make_stock_reply(status::unprocessable_entity);
	map[status::locked]                             = make_stock_reply(status::locked);
	map[status::failed_dependency]                  = make_stock_reply(status::failed_dependency);
	map[status::upgrade_required]                   = make_stock_reply(status::upgrade_required);
	map[status::precondition_required]              = make_stock_reply(status::precondition_required);
	map[status::too_many_requests]                  = make_stock_reply(status::too_many_requests);
	map[status::request_header_fields_too_large]    = make_stock_reply(status::request_header_fields_too_large);
	map[status::connection_closed_without_response] = make_stock_reply(status::connection_closed_without_response);
	map[status::unavailable_for_legal_reasons]      = make_stock_reply(status::unavailable_for_legal_reasons);
	map[status::client_closed_request]              = make_stock_reply(status::client_closed_request);
	map[status::internal_server_error]              = make_stock_reply(status::internal_server_error);
	map[status::not_implemented]                    = make_stock_reply(status::not_implemented);
	map[status::bad_gateway]                        = make_stock_reply(status::bad_gateway);
	map[status::service_unavailable]                = make_stock_reply(status::service_unavailable);
	map[status::gateway_timeout]                    = make_stock_reply(status::gateway_timeout);
	map[status::http_version_not_supported]         = make_stock_reply(status::http_version_not_supported);
	map[status::variant_also_negotiates]            = make_stock_reply(status::variant_also_negotiates);
	map[status::insufficient_storage]               = make_stock_reply(status::insufficient_storage);
	map[status::loop_detected]                      = make_stock_reply(status::loop_detected);
	map[status::not_extended]                       = make_stock_reply(status::not_extended);
	map[status::network_authentication_required]    = make_stock_reply(status::network_authentication_required);
	map[status::network_connect_timeout_error]      = make_stock_reply(status::network_connect_timeout_error);
	return map;
}

boost::beast::string_view stock_reply(status status)
{
	static const auto map = make_stock_reply_map();
	const auto        it  = map.find(status);
	if (it != map.end())
	{
		return it->second;
	}
	else
	{
		return {};
	}
}

} // namespace

response error_response_factory::create(const request& req, status status, const std::optional<string_view>& html)
{
	auto res = response{ status, req.version() };
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, "text/html");
	res.keep_alive(req.keep_alive());
	res.body() = html.value_or(stock_reply(status));
	res.prepare_payload();
	return res;
}

response error_response_factory::create(http::status status, const std::optional<string_view>& html)
{
	auto res = response{};
	res.result(status);
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, "text/html");
	res.body() = html.value_or(stock_reply(status));
	res.prepare_payload();
	return res;
}

} // namespace spider
} // namespace zoo
