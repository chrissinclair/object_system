cmake_minimum_required(VERSION 3.14)

find_package(magic_enum 0.9.7 QUIET)

if (NOT magic_enum_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        magic_enum
        GIT_REPOSITORY https://github.com/Neargye/magic_enum
        GIT_TAG v0.9.7
    )
    FetchContent_MakeAvailable(magic_enum)
endif()

