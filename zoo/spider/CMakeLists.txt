#
# Copyright (C) 2022-2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

add_project_library(spider
	TARGET_OUTPUT_NAME
		${PROJECT_NAME}_spider
	SOURCES
		listener.cpp
		http_session.cpp
		http_session.h
		irequest_handler.cpp
		request_router.cpp
		json_response.cpp
		file_response.cpp
		error_response.cpp
		empty_response.cpp
		tracked_file.cpp
		tracked_file.h
		ifile_event_listener.cpp
		noop_file_event_listener.cpp
		controller.cpp
		log_response.cpp
		exception.cpp
		exception.h
		tag_invoke/time_point.cpp
		tag_invoke/date.cpp
		tag_invoke/time_duration.cpp
	PUBLIC_HEADERS
		aliases.h
		message.h
		listener.h
		irequest_handler.h
		request_router.h
		json_response.h
		file_response.h
		error_response.h
		empty_response.h
		response_wrapper.h
		ifile_event_listener.h
		noop_file_event_listener.h
		dispatcher.hpp
		controller.hpp
		log_response.h
		tag_invoke/time_point.h
		tag_invoke/date.h
		tag_invoke/time_duration.h
	PUBLIC_LIBRARIES
		zoo::common
		Boost::system
		Boost::url
		Boost::filesystem
		Boost::json
		Boost::date_time
		fmt::fmt
)

if(ZOO_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
