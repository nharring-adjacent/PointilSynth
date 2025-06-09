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
add_test(
    NAME ${PROJECT_NAME}_RunTests
    COMMAND ${PROJECT_NAME}
)
