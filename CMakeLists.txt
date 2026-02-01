cmake_minimum_required(VERSION 3.10)

project(Cynth C)
set(CMAKE_C_STANDARD 99)

add_compile_options(-O3)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx512f -mavx512bw")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
add_executable(cynth src/main.c)
target_link_libraries(cynth m)