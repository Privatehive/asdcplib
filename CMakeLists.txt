cmake_minimum_required(VERSION 3.21.1)

# Read the version number from configure.ac
file(STRINGS configure.ac VERSION_STRING REGEX "AC_INIT.*")
string(REGEX MATCH "[0-9]*[\\.][0-9]*[\\.][0-9]*" VERSION_STRING ${VERSION_STRING})
string(REGEX REPLACE "([0-9]+)[.].*" "\\1" VERSION_MAJOR ${VERSION_STRING})
string(REGEX REPLACE ".*[.]([0-9]+)[.].*" "\\1" VERSION_MINOR ${VERSION_STRING})
string(REGEX REPLACE ".*[.]+([0-9]+)" "\\1" VERSION_PATCH ${VERSION_STRING})
message(STATUS "Version Number is ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

project(asdcplib VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Default Targets")

# Configure a header file to pass some of the CMake settings to the source code.
configure_file("${PROJECT_SOURCE_DIR}/src/info.in" "${PROJECT_BINARY_DIR}/src/info.h")

# Add the binary tree to the search path for include files so that we will find info.h.
include_directories("${PROJECT_BINARY_DIR}/src")
add_subdirectory(src)

set(CPACK_GENERATOR ZIP)
if(UNIX)
	set(CPACK_GENERATOR ${CPACK_GENERATOR};STGZ;TGZ)
endif(UNIX)
if(UNIX AND NOT APPLE)
	set(CPACK_GENERATOR ${CPACK_GENERATOR};DEB;RPM)
endif(UNIX AND NOT APPLE)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "asdcplib from cinecert.com")
set(CPACK_PACKAGE_VENDOR "CineCert LLC")
set(CPACK_PACKAGE_CONTACT "asdcplib@cinecert.com")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME}{VERSION_MAJOR}.${VERSION_MINOR})
include(CPack)
