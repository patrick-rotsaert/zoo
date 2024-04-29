#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "lib${PROJECT_NAME}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ZOO - C++ library collection")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
# set(CPACK_RESOURCE_FILE_WELCOME ${CMAKE_CURRENT_LIST_DIR}/welcome.txt)
# set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_LIST_DIR}/description.txt)
# set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_LIST_DIR}/readme.txt)
set(CPACK_VERBATIM_VARIABLES YES)

set(GIT_URL)
find_program(GIT git)
if(GIT)
	execute_process(
		COMMAND git -C ${PROJECT_SOURCE_DIR} config --get remote.origin.url
		OUTPUT_VARIABLE GIT_URL
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif()
if(GIT_URL)
	set(CPACK_PACKAGE_VENDOR "${GIT_URL}")
else()
	set(CPACK_PACKAGE_VENDOR "Patrick Rotsaert")
endif()
set(CPACK_PACKAGE_CONTACT "${CPACK_PACKAGE_VENDOR}")


# NSIS generator
set(CPACK_NSIS_DISPLAY_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_NSIS_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_NSIS_MANIFEST_DPI_AWARE true)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL YES)
# set(CPACK_NSIS_MUI_ICON ${CMAKE_CURRENT_LIST_DIR}/nsis\\\\icon.ico)
# set(CPACK_NSIS_INSTALLED_ICON_NAME ${CMAKE_CURRENT_LIST_DIR}/nsis\\\\icon.ico)
# set(CPACK_NSIS_MUI_UNIICON ${CMAKE_CURRENT_LIST_DIR}/nsis\\\\uninstall_icon.ico)
# set(CPACK_NSIS_MUI_HEADERIMAGE ${CMAKE_CURRENT_LIST_DIR}/nsis\\\\nsis_header.bmp)
# set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP ${CMAKE_CURRENT_LIST_DIR}/nsis\\\\nsis_install_welcome.bmp)
# set(CPACK_NSIS_MUI_UNWELCOMEFINISHPAGE_BITMAP ${CMAKE_CURRENT_LIST_DIR}/nsis\\\\nsis_uninstall_welcome.bmp)
# Define how to install and uninstall start menu entries on Windows
# set(CPACK_NSIS_CREATE_ICONS_EXTRA
#   "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\${CMAKE_PROJECT_NAME}.lnk' '$INSTDIR\\\\bin\\\\App.exe'")
# set(CPACK_NSIS_DELETE_ICONS_EXTRA
#   "Delete '$SMPROGRAMS\\\\$START_MENU\\\\${CMAKE_PROJECT_NAME}.lnk'")


set(CPACK_COMPONENTS_ALL ${COMPONENT_RUNTIME} ${COMPONENT_DEVELOPMENT})

set(CPACK_COMPONENT_${COMPONENT_RUNTIME_UC}_DISPLAY_NAME "Libraries")
set(CPACK_COMPONENT_${COMPONENT_DEVELOPMENT_UC}_DISPLAY_NAME "Development files")

# Set the package descriptions for runtime and dev
set(CPACK_COMPONENT_${COMPONENT_RUNTIME_UC}_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} (${CPACK_COMPONENT_${COMPONENT_RUNTIME_UC}_DISPLAY_NAME})")
set(CPACK_COMPONENT_${COMPONENT_DEVELOPMENT_UC}_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} (${CPACK_COMPONENT_${COMPONENT_DEVELOPMENT_UC}_DISPLAY_NAME})")

if(UNIX)
	list(APPEND CPACK_GENERATOR TGZ)

	find_program(LSB_RELEASE lsb_release)
	if(LSB_RELEASE)
		execute_process(COMMAND ${LSB_RELEASE} -is
			OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		string(TOLOWER ${LSB_RELEASE_ID_SHORT} LSB_RELEASE_ID_SHORT_LC)

		execute_process(COMMAND lsb_release -cs
			OUTPUT_VARIABLE LSB_RELEASE_CODENAME_SHORT
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)

		find_program(DPKG dpkg)
		if(DPKG)
			# Assume debian based distro
			list(APPEND CPACK_GENERATOR DEB)

			# Get the architecture name
			execute_process(COMMAND ${DPKG} --print-architecture
				OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
				OUTPUT_STRIP_TRAILING_WHITESPACE
			)

			# The default CPACK_PACKAGE_FILE_NAME is ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}
			# Overide the last bit so we have distro, release and architecture in the file name
			# As a bonus, this also applies to the generated TGZ file.
			set(CPACK_SYSTEM_NAME "${LSB_RELEASE_ID_SHORT_LC}_${LSB_RELEASE_CODENAME_SHORT}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

			set(CPACK_DEB_COMPONENT_INSTALL ON)

			# Set the package names for runtime and dev
			set(CPACK_DEBIAN_${COMPONENT_RUNTIME_UC}_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
			set(CPACK_DEBIAN_${COMPONENT_DEVELOPMENT_UC}_PACKAGE_NAME ${CPACK_PACKAGE_NAME}-dev)

			# The development package depends on the runtime + other dev packages
			set(DEVELOPMENT_PACKAGE_DEPENDS
				${CPACK_DEBIAN_${COMPONENT_RUNTIME_UC}_PACKAGE_NAME}
				# FIXME: this depends if vcpkg is used or not
				# Perhaps it's better not to set this in the DEB file
				libfmt-dev
				libspdlog-dev
				libboost-dev
				libpq-dev
				libmysqlclient-dev
				libsqlite3-dev
				libssh-dev
			)

			string(JOIN ", " CPACK_DEBIAN_${COMPONENT_DEVELOPMENT_UC}_PACKAGE_DEPENDS ${DEVELOPMENT_PACKAGE_DEPENDS})

			# Set the package file names
			# By default, when using component based packaging, cpack appends the component name to the file name.
			# But since we have included the project name in the component names (see ../CMakeLists.txt for the reason why),
			# this leads to rather ugly file names.
			set(CPACK_DEBIAN_${COMPONENT_RUNTIME_UC}_FILE_NAME "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}-${CPACK_SYSTEM_NAME}-runtime.deb")
			set(CPACK_DEBIAN_${COMPONENT_DEVELOPMENT_UC}_FILE_NAME "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}-${CPACK_SYSTEM_NAME}-development.deb")

			# Detect and include dependencies
			set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
		endif()
		# TODO: other distros
	endif()
elseif(WIN32)
	list(APPEND CPACK_GENERATOR ZIP NSIS)
endif()

include(CPack)
