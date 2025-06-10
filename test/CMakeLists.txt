cmake_minimum_required(VERSION 3.22)
project(PointilSynthTests VERSION 0.1.0)

enable_testing()
include(GoogleTest) # For GTest::gtest_main and gtest_discover_tests

set(SOURCE_FILES
    source/AudioProcessorTest.cpp
    source/DebugUIPanelTest.cpp
    source/PresetManagerTest.cpp
    source/TestMain.cpp
)


set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_OPTIONS "${PROJECT_WARNINGS_CXX}")
# JUCE toolchain should handle appropriate warnings.

juce_add_console_app(${PROJECT_NAME}
    SOURCES 
        source/AudioProcessorTest.cpp
        source/DebugUIPanelTest.cpp
        source/PresetManagerTest.cpp
        source/TestMain.cpp
    MODULES
        juce_core
        juce_events
        juce_gui_basics         # For DebugUIPanel and ScopedJuceInitialiser_GUI
        juce_unittest         # For UnitTestRunner and test macros
        juce_audio_processors # For APVTS in PanelTestFixture
        juce_audio_basics   # Potentially needed by juce_audio_processors or tests
    LINK_LIBRARIES
        PointillisticSynth    # Link against the actual plugin
        GTest::gtest_main
)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${GOOGLETEST_SOURCE_DIR}/googletest/include
    ${PROJECT_SOURCE_DIR}/Source
    ${PROJECT_SOURCE_DIR}/plugin/source
    ${PROJECT_SOURCE_DIR}/plugin/include
)

if(CMAKE_GENERATOR STREQUAL Xcode)
  gtest_discover_tests(${PROJECT_NAME} DISCOVERY_MODE PRE_TEST)
else()
  gtest_discover_tests(${PROJECT_NAME})
endif()

