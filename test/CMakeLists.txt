cmake_minimum_required(VERSION 3.22)
project(PointilSynthTests VERSION 0.1.0)

enable_testing()
include(GoogleTest) # For GTest::gtest_main and gtest_discover_tests

set(TEST_SOURCE_FILES
    source/AudioProcessorTest.cpp
    source/PresetManagerTest.cpp
)

juce_add_console_app(${PROJECT_NAME}
    SOURCES ${TEST_SOURCE_FILES}
    # Note: If TARGET_LIBRARIES is not a valid keyword for juce_add_console_app,
    # this will cause an error, and linking will need to be done via
    # a separate target_link_libraries command.
    TARGET_LIBRARIES PointillisticSynth GTest::gtest_main
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${GOOGLETEST_SOURCE_DIR}/googletest/include
    ${PROJECT_SOURCE_DIR}/Source
    ${PROJECT_SOURCE_DIR}/plugin/source
    ${PROJECT_SOURCE_DIR}/plugin/include
)

# Removed: set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_OPTIONS "${PROJECT_WARNINGS_CXX}")
# JUCE toolchain should handle appropriate warnings.

if(CMAKE_GENERATOR STREQUAL Xcode)
  gtest_discover_tests(${PROJECT_NAME} DISCOVERY_MODE PRE_TEST)
else()
  gtest_discover_tests(${PROJECT_NAME})
endif()
