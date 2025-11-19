//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <optional>
#include <string>
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>

#include <boost/json/conversion.hpp>

namespace zoo {
namespace spider {

class openapi_type_name_registry
{
public:
	template<typename T>
	static void register_type(std::string name)
	{
		type_name_map()[std::type_index(typeid(T))] = std::move(name);
	}

	template<typename T>
	static std::optional<std::string> get_type_name()
	{
		const auto& map = type_name_map();
		const auto  it  = map.find(std::type_index(typeid(T)));
		if (it == map.end())
		{
			return std::nullopt;
		}
		else
		{
			return it->second;
		}
	}

private:
	static auto& type_name_map()
	{
		static std::unordered_map<std::type_index, std::string> map{};
		return map;
	}
};

template<typename T>
struct openapi_type_name_registrar
{
	explicit openapi_type_name_registrar(std::string name)
	{
		openapi_type_name_registry::register_type<T>(std::move(name));
	}
};

#define SPIDER_OAS_REGISTER_TYPE_NAME(TYPE, NAME) zoo::spider::openapi_type_name_registrar<TYPE> TYPE##_type_name_registrar{ NAME };

class openapi_type_example_registry
{
public:
	using factory_type = std::function<boost::json::value()>;

	template<typename T>
	static void register_type(factory_type&& factory)
	{
		type_factory_map()[std::type_index(typeid(T))] = std::move(factory);
	}

	template<typename T>
	static std::optional<boost::json::value> get_type_example()
	{
		const auto& map = type_factory_map();
		const auto  it  = map.find(std::type_index(typeid(T)));
		if (it == map.end())
		{
			return std::nullopt;
		}
		else
		{
			return it->second();
		}
	}

private:
	static auto& type_factory_map()
	{
		static std::unordered_map<std::type_index, factory_type> map{};
		return map;
	}
};

template<typename T>
struct openapi_type_example_registrar
{
	explicit openapi_type_example_registrar(openapi_type_example_registry::factory_type&& factory)
	{
		openapi_type_example_registry::register_type<T>(std::move(factory));
	}
};

#define SPIDER_OAS_REGISTER_TYPE_EXAMPLE(TYPE, EXAMPLE)                                                                                    \
	zoo::spider::openapi_type_example_registrar<TYPE> TYPE##_type_example_registrar{ []() { return boost::json::value_from(EXAMPLE); } };

} // namespace spider
} // namespace zoo
