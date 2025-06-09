cmake_minimum_required(VERSION 3.22)

project(PointilSynthTests)

# Add the main JUCE module. This will transitively include other modules.
# However, it's good practice to explicitly list the ones you use.
# find_package(JUCE REQUIRED) # This might be needed depending on JUCE installation

# Define the source files for the test executable
set(SOURCE_FILES
    source/TestMain.cpp
    source/DebugUIPanelTest.cpp
    # Add other test files here if needed in the future
)

# Create the test executable using JUCE's helper function
juce_add_console_application(${PROJECT_NAME}
    SOURCES
        source/TestMain.cpp
        source/DebugUIPanelTest.cpp
    MODULES
        juce_core
        juce_events
        juce_gui_basics         # For DebugUIPanel and ScopedJuceInitialiser_GUI
        juce_unittest         # For UnitTestRunner and test macros
        juce_audio_processors # For APVTS in PanelTestFixture
        # juce_audio_basics   # Potentially needed by juce_audio_processors or tests
    LINK_LIBRARIES
        PointillisticSynth    # Link against the actual plugin
)

# Add include directories
# Note: juce_add_console_application should handle JUCE module includes automatically.
# We still need includes for our own project headers.
# Directory containing DebugUIPanel.h and PointilismInterfaces.h
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ../plugin/source/ # For DebugUIPanel.h, PointilismInterfaces.h (which includes StochasticModel.h)
    ../plugin/include/ # For other plugin headers if needed
)

# JUCE specific console app definitions are handled by juce_add_console_application.

# Enables strict C++ warnings and treats warnings as errors if desired
# set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_OPTIONS "${PROJECT_WARNINGS_CXX}")

# To run JUCE unit tests, the executable itself will run them.
# We don't need gtest_discover_tests here.
# We can add a custom test command using add_test if we want CTest integration.
enable_testing()

# Creates the test console application.
set(SOURCE_FILES
    source/StochasticModelTest.cpp
)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Sets the necessary include directories of googletest.
target_include_directories(${PROJECT_NAME} PRIVATE ${GOOGLETEST_SOURCE_DIR}/googletest/include)

# Thanks to the fact that we link against the gtest_main library, we don't have to write the main function ourselves.
target_link_libraries(${PROJECT_NAME} PRIVATE AudioPlugin GTest::gtest_main)

# Enables strict C++ warnings and treats warnings as errors.
# This needs to be set up only for your projects, not 3rd party
set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_OPTIONS "${PROJECT_WARNINGS_CXX}")

# Adds googletest-specific CMake commands at our disposal.
include(GoogleTest)

# Add all tests defined with googletest to the CMake metadata
# so that these tests are run upon a call to ctest in the test
# projects' binary directory.
if(CMAKE_GENERATOR STREQUAL Xcode)
  # On macOS arm64, all binaries have to be signed before running.
  # In local development, the linker adds an ad-hoc placeholder signature.
  # In Xcode however, the ad-hoc signature is delayed until after the “Run Script” build phase,
  # so the POST_BUILD command added by gtest_discover_tests cannot run.
  # Thus, we need to delay test discovery until run time.
  # Source: https://discourse.cmake.org/t/googletest-crash-when-using-cmake-xcode-arm64/5766/8
  gtest_discover_tests(${PROJECT_NAME} DISCOVERY_MODE PRE_TEST)
else()
  gtest_discover_tests(${PROJECT_NAME})
endif()
