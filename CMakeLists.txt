cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(cpp-rinher-compiler)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

set(default_build_type "RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo" "Sanitizer")
endif()

set(ESBMC_CXX_Clang_DEFAULTS -Wall -Wextra -pipe -pedantic)
set(ESBMC_CXX_GNU_DEFAULTS -Wall -Wextra -pipe -pedantic)
add_compile_options(${ESBMC_CXX_${CMAKE_CXX_COMPILER_ID}_DEFAULTS})

add_executable(cpp-rinher-compiler
	main.cpp
	ast.cpp
)

target_link_libraries(cpp-rinher-compiler ${JSONCPP_LIBRARIES})
