cmake_minimum_required(VERSION 3.0.0)
project(nes_emulator)

set(CMAKE_CXX_STANDARD 17)
find_package(Catch2 REQUIRED)

# CPU lib
add_library(cpu STATIC src/cpu.cc src/utils.cc)

# Main
add_executable(nes-emu src/main.cc)
target_link_libraries(nes-emu cpu)

# Tests
file(GLOB TEST_SOURCES "src/test/*.cc")
add_executable(nes-emu-test ${TEST_SOURCES})
target_link_libraries(nes-emu-test cpu Catch2::Catch2)
