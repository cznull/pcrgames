cmake_minimum_required(VERSION 3.15)
project(pcr_mini_game LANGUAGES CXX)
#add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>") #支持utf-8
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/permissive->") #一些安全性检查
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/sdl>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/GS>")
set(CMAKE_CXX_STANDARD 17)

add_subdirectory("kaori's karate field") #香织的灵魂空手道场
#add_subdirectory("kaori's karate field") #棒球的也可以用cmake管理，但是我不会opencv啊，要不您整一下？
