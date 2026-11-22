cmake_minimum_required(VERSION 3.24)
project(LuauDesk LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    Luau
    GIT_REPOSITORY https://github.com/luau-lang/luau.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Luau)

add_executable(LuauDesk WIN32 src/main.cpp)
target_compile_features(LuauDesk PRIVATE cxx_std_17)
target_compile_definitions(LuauDesk PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(LuauDesk PRIVATE Luau.VM Luau.Compiler comctl32)
