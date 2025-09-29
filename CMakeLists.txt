cmake_minimum_required(VERSION 3.5)
project(TB2820_Api_Tests C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(tb2820 STATIC
    v07/TB2820_Api.c
    v07/TB2820V07_ApiVersion.c
)

target_include_directories(tb2820 PUBLIC v07)

find_package(GTest)
if (NOT GTest_FOUND)
    add_subdirectory(third_party/gtest)
    set(GTEST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gtest/include)
    set(GTEST_BOTH_LIBRARIES gtest_main gtest)
endif()

add_executable(tb2820_api_tests
    v07/tests/TB2820_Api_test.cpp
)

target_include_directories(tb2820_api_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/v07/tests
    ${CMAKE_CURRENT_SOURCE_DIR}/v07
    ${GTEST_INCLUDE_DIRS}
)
target_link_libraries(tb2820_api_tests PRIVATE tb2820 ${GTEST_BOTH_LIBRARIES} pthread)
set_property(TARGET tb2820_api_tests APPEND_STRING PROPERTY LINK_FLAGS
    " -Wl,--wrap=read -Wl,--wrap=open -Wl,--wrap=close -Wl,--wrap=mmap -Wl,--wrap=munmap -Wl,--wrap=lseek")

enable_testing()
add_test(NAME TB2820_Api_GTest COMMAND tb2820_api_tests)
