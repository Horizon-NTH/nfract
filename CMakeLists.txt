cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

project(nfract VERSION 0.0.0)

option(ENABLE_TESTS "Enable testing" OFF)

set(HEADER_FILES
        include/app/ArgumentsParser.hpp
)

set(SOURCE_FILES
        src/app/ArgumentsParser.cpp
)

set(PROJECT_LIB ${PROJECT_NAME}_lib)

add_library(${PROJECT_LIB} STATIC ${HEADER_FILES} ${SOURCE_FILES})

include_directories(${CMAKE_SOURCE_DIR}/include)

target_compile_features(${PROJECT_LIB} PUBLIC cxx_std_20)
target_compile_definitions(${PROJECT_LIB} PUBLIC PROJECT_VERSION="${PROJECT_VERSION}")
target_include_directories(${PROJECT_LIB} PRIVATE ${CMAKE_SOURCE_DIR}/deps)

add_executable(${PROJECT_NAME} src/Main.cpp)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIB} ${DEPS})

if (ENABLE_TESTS)
    add_subdirectory(tests)
endif ()