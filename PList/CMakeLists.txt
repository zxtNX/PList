cmake_minimum_required(VERSION 3.28)
project(PList C)

set(CMAKE_C_STANDARD 23)

# Include directories
include_directories(include)

# Source files
file(GLOB SOURCES "src/*.c")

# Specify the executable
add_executable(PList ${SOURCES})
