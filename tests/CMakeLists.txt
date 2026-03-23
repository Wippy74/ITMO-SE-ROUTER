include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    FIND_PACKAGE_ARGS NAMES GTest
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

set(LIB_SOURCES
    ../src/api_handler.cpp
    ../src/cache.cpp
    ../src/route_builder.cpp
    ../src/http_interface.cpp
)

add_executable(test_cache test_cache.cpp)
target_link_libraries(
    test_cache
    PRIVATE
    labwork7_lib
    GTest::gtest
    GTest::gtest_main
)

add_executable(test_api_handler test_api.cpp)
target_link_libraries(
    test_api_handler
    PRIVATE
    labwork7_lib
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
)

add_test(NAME CacheTests COMMAND test_cache)
add_test(NAME ApiHandlerTests COMMAND test_api_handler)

gtest_discover_tests(test_cache)
gtest_discover_tests(test_api_handler)
