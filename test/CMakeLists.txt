cmake_minimum_required(VERSION 3.22)

project(PointilSynthTests)

# Add the main JUCE module. This will transitively include other modules.
# However, it's good practice to explicitly list the ones you use.
# find_package(JUCE REQUIRED) # This might be needed depending on JUCE installation

# Define the source files for the test executable
set(SOURCE_FILES
    source/DebugUIPanelTest.cpp
    # Add other test files here if needed in the future
)

# Create the test executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link against the plugin library (PointillisticSynth)
# This assumes PointillisticSynth is a target defined in the parent CMakeLists.txt
# or has been built and can be found by CMake.
# We also need to link against the JUCE modules required for the tests.
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    PointillisticSynth # Link against the actual plugin
    juce::juce_core
    juce::juce_events # For the test runner
    juce::juce_gui_basics # For DebugUIPanel
    # Add other JUCE modules if test code requires them, e.g.:
    # juce::juce_audio_basics
    # juce::juce_audio_processors
)

# Add include directories
# Directory containing DebugUIPanel.h and PointilismInterfaces.h
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ../plugin/source/ # For DebugUIPanel.h, PointilismInterfaces.h
    ../plugin/include/ # For other plugin headers if needed
    # JUCE modules are typically handled by JUCE's CMake support,
    # but if not, you might need:
    # ${JUCE_INCLUDE_DIRS}
)

# JUCE specific: Define a console app entry point
target_compile_definitions(${PROJECT_NAME} PRIVATE JUCE_CONSOLE_APPLICATION=1)

# Enables strict C++ warnings and treats warnings as errors if desired
# set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_OPTIONS "${PROJECT_WARNINGS_CXX}")

# To run JUCE unit tests, the executable itself will run them.
# We don't need gtest_discover_tests here.
# We can add a custom test command using add_test if we want CTest integration.
enable_testing()
add_test(
    NAME ${PROJECT_NAME}_RunTests
    COMMAND ${PROJECT_NAME}
)
