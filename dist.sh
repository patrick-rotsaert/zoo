#!/bin/bash

#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

THISDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
THISNAME=$(basename "${BASH_SOURCE[0]}")
THIS="${THISDIR}/${THISNAME}"

usage() {
	cat <<-EOF
	${THIS} [OPTIONS]

	OPTIONS:
	-h, --help          Print this help and exit.
	-V, --verbose       Be verbose, i.e. debug this script.

	--build-dir=arg     Set the build directory.
	                    Defaults to /tmp/zoo-build.
	--generator=arg     Set the CMake generator.
	                    Defaults to Ninja.
	--cxx=arg           Set the path to the C++ compiler.
	--vcpkg             Use vcpkg to get the dependencies.
	EOF
}

CMAKE_ARGS=()

options=$(getopt -n "${THISNAME}" -l "help,verbose,build-dir:,generator:,cxx:,vcpkg" -o "hV" -a -- "$@")
eval set -- "${options}"
while true; do
	case $1 in
	-h|--help)
		usage
		exit 0
		;;
	-V|--verbose)
		set -x
		CMAKE_ARGS+=("-DCOMMAND_ECHO=STDERR")
		;;
	--build-dir)
		shift
		CMAKE_ARGS+=("-DBUILD_DIR=$1")
		;;
	--generator)
		shift
		CMAKE_ARGS+=("-DGENERATOR=$1")
		;;
	--cxx)
		shift
		CMAKE_ARGS+=("-DCXX=$1")
		;;
	--vcpkg)
		CMAKE_ARGS+=("-DVCPKG=ON")
		;;
	--)
		shift
		break;;
	esac
	shift
done

[ $# -eq 0 ] || error_exit "${THISNAME}: Invalid number of arguments. Try \`$0 --help' for usage."
cmake "${CMAKE_ARGS[@]}" -P "${THISDIR}"/dist/dist.cmake
