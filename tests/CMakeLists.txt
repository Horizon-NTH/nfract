enable_testing()

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(TEST_SOURCES
        src/app/ApplicationTest.cpp
        src/app/ArgumentsParserTest.cpp
        src/core/ImageTest.cpp
        src/core/RootsTableTest.cpp
)

set(TEST_TARGET runTests)
add_executable(${TEST_TARGET} ${TEST_SOURCES})
target_compile_features(${TEST_TARGET} PUBLIC cxx_std_20)
target_link_libraries(${TEST_TARGET} PRIVATE GTest::gtest_main ${PROJECT_LIB})

include(GoogleTest)
gtest_discover_tests(${TEST_TARGET}
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${TEST_TARGET}>
)
