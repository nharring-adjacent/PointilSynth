cmake_minimum_required(VERSION 3.22)
project(PointilSynthTests VERSION 0.1.0)

enable_testing()
include(GoogleTest) # For GTest::gtest_main and gtest_discover_tests

set(TEST_SOURCE_FILES
    source/AudioProcessorTest.cpp
    source/PresetManagerTest.cpp
    source/TestMain.cpp
)
set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_OPTIONS "${PROJECT_WARNINGS_CXX}")

juce_add_console_app(${PROJECT_NAME}
    SOURCES
	${TEST_SOURCE_FILES}
    MODULES
        juce_core
        juce_events
        juce_gui_basics         # For DebugUIPanel and ScopedJuceInitialiser_GUI
        juce_unittest         # For UnitTestRunner and test macros
        juce_audio_processors # For APVTS in PanelTestFixtureA
        juce_audio_basics
    LINK_LIBRARIES
	PointlisticSynth
	GTest::gtest
)


target_link_libraries(
  ${PROJECT_NAME} PRIVATE juce::juce_recommended_config_flags juce::juce_recommended_lto_flags
                         juce::juce_recommended_warning_flags
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${GOOGLETEST_SOURCE_DIR}/googletest/include
    "${ROJECT_DIR}/../plugin/include"
    "${ROJECT_DIR}/../plugin/source/"
)
target_sources(${PROJECT_NAME} PRIVATE ${TEST_SOURCE_FILES})

# Removed: set_source_files_properties(${TEST_SOURCE_FILES} PROPERTIES COMPILE_OPTIONS "${PROJECT_WARNINGS_CXX}")
# JUCE toolchain should handle appropriate warnings.

if(CMAKE_GENERATOR STREQUAL Xcode)
  gtest_discover_tests(${PROJECT_NAME} DISCOVERY_MODE PRE_TEST)
else()
  gtest_discover_tests(${PROJECT_NAME})
endif()
