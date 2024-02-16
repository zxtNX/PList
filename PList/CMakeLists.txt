cmake_minimum_required(VERSION 3.28)
project(PList C)

set(CMAKE_C_STANDARD 23)

# Include directories
include_directories(include)

# Source files
file(GLOB MAIN_SOURCE "src/main.c")
file(GLOB MODULES_SOURCE "src/modules/*.c")
file(GLOB UTILS_SOURCE "src/utils/*.c")

# Specify the executable
add_executable(PList ${MAIN_SOURCE} ${MODULES_SOURCE} ${UTILS_SOURCE})

target_link_libraries(PList PRIVATE kernel32.lib ntdll.lib user32.lib Advapi32.lib)
