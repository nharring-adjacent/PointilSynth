#include <juce/juce_core.h>
#include <juce/juce_unittest.h> // For UnitTestRunner

// This main function will run all JUCE unit tests linked into the executable.
// If DebugUIPanelTest.cpp (and any other JUCE test files) has a static instance
// of its test class (e.g., static DebugUIPanelTests debugUIPanelTestsInstance;),
// the UnitTestRunner will automatically find and run them.

int main(int argc, char* argv[])
{
    // Note: ScopedJuceInitialiser_GUI is needed if any tests use GUI components,
    // even indirectly. If tests are purely non-GUI, ScopedJuceInitialiser could be used.
    // Given DebugUIPanel is a GUI component, _GUI is safer.
    juce::ScopedJuceInitialiser_GUI libraryInitialiser;

    juce::UnitTestRunner runner;
    // No need to explicitly add tests if they use static global instances.
    // runner.addTest (new DebugUIPanelTests()); // This would be an alternative.

    runner.runAllTests();

    int numFailures = runner.getNumFailures();

    // Optional: Output results to console or file, though CTest captures pass/fail via return code.
    // For example, to log to console:
    // runner.logResults(); // This is deprecated.
    // Instead, iterate and print:
    // for (int i = 0; i < runner.getNumResults(); ++i)
    // {
    //     auto* result = runner.getResult(i);
    //     juce::Logger::outputDebugString (result->description);
    //     juce::Logger::outputDebugString (result->messages);
    // }


    return numFailures; // CTest interprets non-zero return as failure
}
