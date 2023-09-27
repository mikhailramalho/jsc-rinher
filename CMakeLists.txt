cmake_minimum_required(VERSION 3.5)

project(cpp-rinher-compiler)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
        STRING "Choose the type of build." FORCE)

    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo" "Sanitizer")
endif()

set(CXX_Clang_DEFAULTS -Wall -Wextra -pipe -pedantic -fno-rtti -fno-exceptions)
set(CXX_GNU_DEFAULTS -Wall -Wextra -pipe -pedantic -fno-rtti -fno-exceptions)
add_compile_options(${CXX_${CMAKE_CXX_COMPILER_ID}_DEFAULTS})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-flto)

elseif(CMAKE_BUILD_TYPE STREQUAL "Sanitizer")
    message(STATUS "Sanitizer Mode")
    set(SANITIZER_TYPE "ASAN" CACHE
        STRING "Choose the sanitizer to use.")

    set_property(CACHE SANITIZER_TYPE PROPERTY STRINGS
        "TSAN" "ASAN" "LSAN" "MSAN" "UBSAN")

    # ThreadSanitizer
    set(TSAN_FLAGS "-fsanitize=thread -g -O1")

    # AddressSanitizer
    set(ASAN_FLAGS "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1")

    # LeakSanitizer
    set(LSAN_FLAGS "-fsanitize=leak -fno-omit-frame-pointer -g -O1")

    # MemorySanitizer
    set(MSAN_FLAGS "-fsanitize=memory -fno-optimize-sibling-calls -fsanitize-memory-track-origins=2 -fno-omit-frame-pointer -g -O2")

    # UndefinedBehaviour
    set(UBSAN_FLAGS "-fsanitize=undefined -fno-sanitize=vptr")

    set(CMAKE_C_FLAGS_SANITIZER
        "${${SANITIZER_TYPE}_FLAGS}" CACHE
        STRING "C flags for sanitizer." FORCE)
    set(CMAKE_CXX_FLAGS_SANITIZER
        "${${SANITIZER_TYPE}_FLAGS}"
        CACHE STRING "C++ flags for sanitizer." FORCE)
else()
    unset(SANITIZER_TYPE CACHE)
    unset(CMAKE_C_FLAGS_SANITIZER CACHE)
    unset(CMAKE_CXX_FLAGS_SANITIZER CACHE)
endif()

add_executable(cpp-rinher-compiler
    main.cpp
    ast.cpp
)

target_link_libraries(cpp-rinher-compiler ${JSONCPP_LIBRARIES})
