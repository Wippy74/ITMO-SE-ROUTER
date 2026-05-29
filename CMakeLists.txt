cmake_minimum_required(VERSION 3.12)

project(
    labwork7
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)


FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.11.0
)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)

FetchContent_MakeAvailable(json cpr googletest)

enable_testing()


add_library(labwork7_lib
    src/cache.cpp
    src/api_handler.cpp
    src/route_builder.cpp
    src/http_interface.cpp
)

target_include_directories(labwork7_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(labwork7_lib PUBLIC nlohmann_json::nlohmann_json cpr::cpr)

add_executable(labwork7 ./bin/main.cpp)
target_link_libraries(labwork7 PRIVATE labwork7_lib)
add_subdirectory(tests)