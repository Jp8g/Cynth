cmake_minimum_required(VERSION 3.10)

project(Cynth C)
set(CMAKE_C_STANDARD 99)

add_compile_options(-O0)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
add_executable(cynth src/main.c)