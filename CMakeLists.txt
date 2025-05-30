cmake_minimum_required(VERSION 3.14)
project(object_system)

option(OBJECT_SYSTEM_TESTS "Build the object system tests" OFF)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS Private/*.cpp)
add_library(Object ${SOURCE_FILES})
target_include_directories(Object
    PUBLIC
        Public
    PRIVATE
        Private
)

if (${OBJECT_SYSTEM_TESTS})
    add_subdirectory(ThirdParty/catch2)

    file(GLOB_RECURSE TEST_SOURCE_FILES CONFIGURE_DEPENDS Tests/*.cpp)
    add_executable(ObjectTests ${TEST_SOURCE_FILES})
    target_link_libraries(ObjectTests
        PRIVATE
            Object
            Catch2::Catch2WithMain
    )

    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
    include(CTest)
    include(Catch)
    catch_discover_tests(ObjectTests)
endif()
