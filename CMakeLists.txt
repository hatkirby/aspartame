cmake_minimum_required (VERSION 3.1)
project (rawr-ebooks)

add_subdirectory(vendor/rawr-ebooks)

include_directories(vendor/rawr-ebooks)

add_executable(garnet dialogue.cpp histogram.cpp)
set_property(TARGET garnet PROPERTY CXX_STANDARD 17)
set_property(TARGET garnet PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(garnet rawr)
