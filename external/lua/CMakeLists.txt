cmake_minimum_required(VERSION 3.20)
project(lua_static C)

file(GLOB LUA_CORE_SOURCES src/*.c)
add_library(lua STATIC ${LUA_CORE_SOURCES})
target_include_directories(lua PUBLIC src)
target_compile_definitions(lua PRIVATE LUA_32BITS)
