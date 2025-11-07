cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

project(nfract VERSION 0.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

option(ENABLE_TESTS "Enable testing" OFF)
option(RUN_ON_CPU "Run rendering on CPU instead of on ISPC" OFF)

if (NOT RUN_ON_CPU)
    if (CMAKE_GENERATOR MATCHES "Visual Studio")
        message(FATAL_ERROR "ISPC is not supported by generator ${CMAKE_GENERATOR}. "
                "Re-run CMake with a generator that supports ISPC (e.g. Ninja) or configure with -DRUN_ON_CPU=ON to disable ISPC.")
    endif ()
    enable_language(ISPC)
endif ()

set(HEADER_FILES
        include/app/ArgumentsParser.hpp
        include/core/Image.hpp
        include/core/RootsTable.hpp
        include/core/RenderNewton.hpp
        include/app/Application.hpp
)

set(SOURCE_FILES
        src/app/ArgumentsParser.cpp
        src/core/Image.cpp
        src/core/RootsTable.cpp
        src/core/RenderNewton.cpp
        src/app/Application.cpp
)

if (NOT RUN_ON_CPU)
    add_library(ispc_lib STATIC src/kernel/Newton.ispc)
    target_include_directories(ispc_lib PUBLIC $<TARGET_PROPERTY:ISPC_HEADER_DIRECTORY>)
endif ()

set(PROJECT_LIB ${PROJECT_NAME}_lib)

add_library(${PROJECT_LIB} STATIC ${HEADER_FILES} ${SOURCE_FILES})

include_directories(${CMAKE_SOURCE_DIR}/include)

target_compile_features(${PROJECT_LIB} PUBLIC cxx_std_20)
target_compile_definitions(${PROJECT_LIB} PUBLIC PROJECT_VERSION="${PROJECT_VERSION}")
target_include_directories(${PROJECT_LIB} PRIVATE ${CMAKE_SOURCE_DIR}/deps)
if (NOT RUN_ON_CPU)
    target_link_libraries(${PROJECT_LIB} ispc_lib)
else ()
    target_compile_definitions(${PROJECT_LIB} PUBLIC RUN_ON_CPU)
endif ()

add_executable(${PROJECT_NAME} src/Main.cpp)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIB})

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(TARGETS ${PROJECT_LIB}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if (ENABLE_TESTS)
    add_subdirectory(tests)
endif ()
