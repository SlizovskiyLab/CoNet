cmake_minimum_required(VERSION 3.10)
project(CoNet)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers
include_directories(include)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Build target
add_executable(CoNet ${SOURCES})