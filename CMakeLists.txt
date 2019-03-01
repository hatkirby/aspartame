cmake_minimum_required (VERSION 3.1)
project (aspartame)

find_package(PkgConfig)
pkg_check_modules(yaml-cpp yaml-cpp REQUIRED)

add_subdirectory(vendor/rawr-ebooks EXCLUDE_FROM_ALL)

include_directories(
  vendor/rawr-ebooks
  vendor/rawr-ebooks/vendor/libtwittercpp/src
  ${yaml-cpp_INCLUDE_DIRS})

add_executable(aspartame dialogue.cpp)
set_property(TARGET aspartame PROPERTY CXX_STANDARD 17)
set_property(TARGET aspartame PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(aspartame rawr twitter++ ${yaml-cpp_LIBRARIES})
