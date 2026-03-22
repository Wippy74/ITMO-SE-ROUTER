cmake_minimum_required(VERSION 3.12)

project(
    labwork7
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)

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

FetchContent_MakeAvailable(json cpr)

add_executable(labwork7
               ./bin/main.cpp
               ./src/cache.cpp
               ./src/api_handler.cpp
               ./src/route_builder.cpp
               ./src/http_interface.cpp
)

enable_testing()
add_subdirectory(tests)

target_link_libraries(labwork7 PRIVATE nlohmann_json::nlohmann_json cpr::cpr)

include(tests/coverage.cmake)
