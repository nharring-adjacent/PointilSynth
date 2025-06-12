#pragma once

#include <juce::juce_gui_basics.h>

// Forward declaration
class PresetManager;

//==============================================================================
class PresetBrowserComponent  : public juce::Component,
                                public juce::ListBoxModel
{
public:
    //==============================================================================
    PresetBrowserComponent(PresetManager& pm);
    ~PresetBrowserComponent() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    //==============================================================================
    // ListBoxModel overrides
    int getNumRows() override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& event) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent& event) override;
    // Removed getSelectedItemIndex() and setSelectedRow(int) as juce::ListBox handles selection directly.
    // We can get the selected row via presetListBox.getSelectedRow()
    // and set it via presetListBox.selectRow().

private:
    //==============================================================================
    void scanPresetDirectory();
    void filterPresetsByCategory();
    void loadButtonClicked();
    void saveButtonClicked();
    void categoryChanged();

    PresetManager& presetManager;

    juce::ListBox presetListBox;
    juce::TextButton loadButton { "Load" };
    juce::TextEditor nameEditor;
    juce::TextButton saveButton { "Save" };
    juce::ComboBox categoryComboBox;
    juce::Label categoryLabel { {}, "Category:" };

    juce::StringArray presetFiles; // Stores the names of discovered preset files (display names, without .json)
    juce::File presetDirectory;    // Default directory for presets
    juce::String selectedCategory; // Currently selected category for filtering

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBrowserComponent)
};
