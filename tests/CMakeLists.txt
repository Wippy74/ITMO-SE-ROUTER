add_executable(labwork7_tests
    test_api.cpp
    test_cache.cpp
    test_router.cpp
)

target_link_libraries(labwork7_tests
    PRIVATE
    labwork7_lib
    GTest::gtest_main
    GTest::gmock
)

include(GoogleTest)

gtest_discover_tests(labwork7_tests)
include(coverage.cmake)