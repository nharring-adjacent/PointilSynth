cmake_minimum_required(VERSION 3.22)

# Version is needed by JUCE.
project(PointillisticSynth VERSION 0.1.0)
set(POINTILSYNTH_VERSION ${PROJECT_VERSION})
set(POINTILSYNTH_PRODUCT_NAME "PointillisticSynth")
set(POINTILSYNTH_COMPANY_NAME "HHI")
set(POINTILSYNTH_MANUFACTURER_CODE "HHIS")
set(POINTILSYNTH_PLUGIN_CODE "Pntl")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/include/Pointilsynth/ProjectInfo.h.in"
  "${CMAKE_BINARY_DIR}/include/Pointilsynth/ProjectInfo.h"
)
# Adding a directory with the library/application name as a subfolder of the
# include folder is a good practice. It helps avoid name clashes later on.
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/Pointilsynth/")

# Adds a plugin target (that's basically what the Projucer does).
juce_add_plugin(
  ${PROJECT_NAME}
  COMPANY_NAME
  "${POINTILSYNTH_COMPANY_NAME}"
  IS_SYNTH
  TRUE # may change this
  NEEDS_MIDI_INPUT
  TRUE # may change this
  NEEDS_MIDI_OUTPUT
  FALSE # may change this
  PLUGIN_MANUFACTURER_CODE
  ${POINTILSYNTH_MANUFACTURER_CODE}
  PLUGIN_CODE
  ${POINTILSYNTH_PLUGIN_CODE}
  FORMATS
  Standalone
  #VST3
  AU
  AAX #
  PRODUCT_NAME
  "${POINTILSYNTH_PRODUCT_NAME}"
)

# Sets the source files of the plugin project.
set(SOURCE_FILES
    source/AudioEngine.cpp
    source/InertialHistoryManager.cpp
    source/ConfigManager.cpp
    source/DebugUIPanel.cpp
    source/PluginProcessor.cpp
    source/PluginEditor.cpp
    source/PresetManager.cpp
    source/UI/PresetBrowserComponent.cpp
    source/UI/VisualizationComponent.cpp
    source/PodComponent.cpp
    source/StochasticModel.cpp
)
# Optional; includes header files in the project file tree in Visual Studio
set(HEADER_FILES
    ${INCLUDE_DIR}/DebugUIPanel.h
    ${INCLUDE_DIR}/ConfigManager.h
    ${INCLUDE_DIR}/GrainEnvelope.h
    ${INCLUDE_DIR}/Oscillator.h
    ${INCLUDE_DIR}/PluginEditor.h
    ${INCLUDE_DIR}/PluginProcessor.h
    ${INCLUDE_DIR}/InertialHistoryManager.h
    ${INCLUDE_DIR}/PointilismInterfaces.h
    ${INCLUDE_DIR}/PresetManager.h
    ${INCLUDE_DIR}/Resampler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/UI/PresetBrowserComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/UI/VisualizationComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/PodComponent.h
)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES} ${HEADER_FILES})

# Sets the include directories of the plugin project.
target_include_directories(
  ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/Pointilsynth
                         ${CMAKE_BINARY_DIR}/include
)

# Links to all necessary dependencies. The present ones are recommended by JUCE.
# If you use one of the additional modules, like the DSP module, you need to specify it here.
target_link_libraries_system(
  ${PROJECT_NAME}
  PUBLIC
  juce::juce_audio_utils
  juce::juce_audio_basics
  juce::juce_audio_processors
  juce::juce_core
  juce::juce_dsp
  juce::juce_opengl
  juce::juce_gui_basics
  nlohmann_json::nlohmann_json
)

target_link_libraries(${PROJECT_NAME} PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags)
#                         juce::juce_recommended_warning_flags

# These definitions are recommended by JUCE.
target_compile_definitions(${PROJECT_NAME} PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)

# Enables strict C++ warnings and treats warnings as errors.
# This needs to be set up only for your projects, not 3rd party
set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_OPTIONS "${PROJECT_WARNINGS_CXX}")

# In Visual Studio this command provides a nice grouping of source files in "filters".
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/..)
