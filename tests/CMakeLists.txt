cmake_minimum_required(VERSION 3.14)
project(TimetableTests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(run_tests run_tests.cpp)
target_compile_options(run_tests PRIVATE -Wall -Wextra)
target_link_libraries(run_tests PRIVATE Threads::Threads)

enable_testing()
add_test(NAME timetable_checks COMMAND run_tests)
