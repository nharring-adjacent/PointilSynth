cmake_minimum_required(VERSION 3.22)
project(PointilSynthTests VERSION 0.1.0)

cpmaddpackage(
  NAME
  Catch2
  GITHUB_REPOSITORY
  catchorg/Catch2
  VERSION
  3.5.2
  OPTIONS
  "CATCH_INSTALL_DOCS OFF"
  "CATCH_INSTALL_EXTRAS OFF"
)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)

enable_testing()
include(CTest)
include(Catch)

set(TEST_SOURCE_FILES
    source/DebugUIPanelTest.cpp
    source/GrainEnvelopeTest.cpp
    source/StandaloneGrainEnvelopeTest.cpp
    source/OscillatorTest.cpp
    source/PluginEditorTest.cpp
    source/PluginProcessorTest.cpp
    source/PointilismInterfacesTest.cpp
    source/StochasticModelListenerTest.cpp
    source/PresetManagerTest.cpp
    source/UI/PresetBrowserComponentTest.cpp
    source/UI/VisualizationComponentTest.cpp
    source/ResamplerTest.cpp
    source/InertialHistoryManagerTest.cpp
)
set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_OPTIONS "${PROJECT_WARNINGS_CXX}")

juce_add_console_app(
  ${PROJECT_NAME}
  SOURCES
  ${TEST_SOURCE_FILES}
  MODULES
  juce_core
  juce_events
  juce_gui_basics # For DebugUIPanel and ScopedJuceInitialiser_GUI
  juce_audio_processors # For APVTS in PanelTestFixtureA
  juce_audio_basics
)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC juce::juce_recommended_config_flags
         juce::juce_recommended_lto_flags
         juce::juce_audio_processors
         nlohmann_json::nlohmann_json
  PRIVATE PointillisticSynth Catch2::Catch2WithMain
)
# juce::juce_recommended_warning_flags

target_include_directories(
  ${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../plugin/include" "${CMAKE_CURRENT_SOURCE_DIR}/.."
)
target_sources(${PROJECT_NAME} PRIVATE ${TEST_SOURCE_FILES})

# Removed: set_source_files_properties(${TEST_SOURCE_FILES} PROPERTIES COMPILE_OPTIONS "${PROJECT_WARNINGS_CXX}")
# JUCE toolchain should handle appropriate warnings.

catch_discover_tests(${PROJECT_NAME})
