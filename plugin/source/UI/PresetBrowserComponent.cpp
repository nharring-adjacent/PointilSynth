#include "PresetBrowserComponent.h"
#include "Data/PresetManager.h" // Assuming PresetManager.h is in the parent directory (Source/)

//==============================================================================
PresetBrowserComponent::PresetBrowserComponent(PresetManager& pm)
    : presetManager(pm),
      presetDirectory(juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("MyPluginPresets")) // Example directory
{
    // Create preset directory if it doesn't exist
    if (!presetDirectory.exists())
        presetDirectory.createDirectory();

    // Setup ListBox
    addAndMakeVisible(presetListBox);
    presetListBox.setModel(this);
    presetListBox.setOutlineThickness(1);

    // Setup Load Button
    addAndMakeVisible(loadButton);
    loadButton.onClick = [this] { loadButtonClicked(); };

    // Setup Name Editor
    addAndMakeVisible(nameEditor);
    nameEditor.setTextToShowWhenEmpty("New Preset Name", juce::Colours::grey);
    nameEditor.setReturnKeyStartsNewLine(false);

    // Setup Save Button
    addAndMakeVisible(saveButton);
    saveButton.onClick = [this] { saveButtonClicked(); };

    // Setup Category Label
    addAndMakeVisible(categoryLabel);
    categoryLabel.setFont (juce::Font (15.00f, juce::Font::bold));
    categoryLabel.setJustificationType (juce::Justification::centredLeft);

    // Setup Category ComboBox
    addAndMakeVisible(categoryComboBox);
    categoryComboBox.addItem("All", 1);
    categoryComboBox.addItem("Texture", 2);
    categoryComboBox.addItem("Rhythmic", 3);
    categoryComboBox.addItem("FX", 4);
    categoryComboBox.setSelectedId(1); // Default to "All"
    categoryComboBox.onChange = [this] { categoryChanged(); };

    // Initial scan and population
    scanPresetDirectory(); // this populates presetFiles initially
    filterPresetsByCategory(); // Ensure correct category is applied on init
    presetListBox.updateContent();
}

PresetBrowserComponent::~PresetBrowserComponent()
{
    // ListBoxModel automatically detaches, child components are managed by JUCE
}

//==============================================================================
void PresetBrowserComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void PresetBrowserComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    auto topRow = bounds.removeFromTop(30);
    categoryLabel.setBounds(topRow.removeFromLeft(80));
    categoryComboBox.setBounds(topRow.removeFromLeft(150));

    auto bottomRow = bounds.removeFromBottom(40);
    nameEditor.setBounds(bottomRow.removeFromLeft(bounds.getWidth() / 2).reduced(0, 5));
    saveButton.setBounds(bottomRow.removeFromLeft(bounds.getWidth() / 2).reduced(10, 5)); // Relative to remaining width
    loadButton.setBounds(bottomRow.reduced(10,5)); // Takes remaining space in bottom row


    presetListBox.setBounds(bounds.reduced(0, 10)); // List box takes the remaining middle space
}

//==============================================================================
// ListBoxModel overrides
int PresetBrowserComponent::getNumRows()
{
    return presetFiles.size();
}

void PresetBrowserComponent::paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll (juce::Colours::lightblue);

    g.setColour (juce::Colours::black);
    g.setFont (height * 0.7f);

    if (rowNumber < presetFiles.size()) // Ensure rowNumber is in bounds
        g.drawText (presetFiles[rowNumber], 5, 0, width - 10, height, juce::Justification::centredLeft, true);
    else // Should not happen if getNumRows() is correct
        g.drawText ("Error", 5, 0, width - 10, height, juce::Justification::centredLeft, true);

}

void PresetBrowserComponent::listBoxItemClicked (int row, const juce::MouseEvent& /*event*/)
{
    // Optional: Handle single click, e.g., update some preview info
    // For now, selection is handled by the ListBox itself.
    // We could update the nameEditor with the selected preset name for quick rename/save-as
    if (row >= 0 && row < presetFiles.size())
        nameEditor.setText(presetFiles[row]);
}

void PresetBrowserComponent::listBoxItemDoubleClicked (int row, const juce::MouseEvent& /*event*/)
{
    if (row >= 0 && row < presetFiles.size())
    {
        presetListBox.selectRow(row); // Ensure the row is selected visually
        loadButtonClicked();
    }
}

//==============================================================================
// Helper methods

void PresetBrowserComponent::scanPresetDirectory()
{
    presetFiles.clear();
    juce::Array<juce::File> files;
    presetDirectory.findChildFiles(files, juce::File::findFiles, false, "*.json");

    for (const auto& file : files)
    {
        presetFiles.add(file.getFileNameWithoutExtension());
    }
    presetFiles.sort(true); // Case-insensitive sort
}

// Basic JSON parsing to extract category.
// In a real scenario, a more robust JSON library might be used, or PresetManager might provide this.
juce::String getCategoryFromJson(const juce::File& presetFile)
{
    if (!presetFile.existsAsFile())
        return juce::String();

    juce::var parsedJson;
    juce::Result parseResult = juce::JSON::parse(presetFile.loadFileAsString(), parsedJson);

    if (parseResult.wasOk() && parsedJson.isObject())
    {
        if (parsedJson.getProperty("category", juce::var()).isString())
            return parsedJson.getProperty("category", juce::var()).toString();
    }
    return juce::String(); // Return empty if no category or parse error
}

void PresetBrowserComponent::filterPresetsByCategory()
{
    scanPresetDirectory(); // Start with all presets

    if (selectedCategory.isNotEmpty() && selectedCategory != "All")
    {
        juce::StringArray filteredFiles;
        for (const auto& presetName : presetFiles)
        {
            juce::File presetFile = presetDirectory.getChildFile(presetName + ".json");
            juce::String categoryInFile = getCategoryFromJson(presetFile);
            if (categoryInFile.equalsIgnoreCase(selectedCategory))
            {
                filteredFiles.add(presetName);
            }
        }
        presetFiles = filteredFiles;
    }
    // If "All" or empty, presetFiles already contains all scanned presets.

    presetListBox.updateContent();
    presetListBox.repaint();
    // Deselect any row if current selection is no longer valid, or select first
    if (presetFiles.isEmpty()) {
        presetListBox.deselectAllRows();
    } else {
        // If the previously selected item is not in the new list, deselect.
        // Or, optionally, select the first item by default: presetListBox.selectRow(0);
    }
}

void PresetBrowserComponent::loadButtonClicked()
{
    int selectedRow = presetListBox.getSelectedRow();
    if (selectedRow >= 0 && selectedRow < presetFiles.size())
    {
        juce::File fileToLoad = presetDirectory.getChildFile(presetFiles[selectedRow] + ".json");
        if (fileToLoad.existsAsFile())
        {
            presetManager.loadPreset(fileToLoad);
            // Potentially provide user feedback about successful load
            nameEditor.setText(presetFiles[selectedRow]); // Update name editor with loaded preset name
        }
        else
        {
            // Handle error: file not found (should not happen if list is correct)
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                   "Load Error",
                                                   "Preset file not found: " + fileToLoad.getFullPathName());
        }
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                               "Load Preset",
                                               "Please select a preset to load.");
    }
}

void PresetBrowserComponent::saveButtonClicked()
{
    juce::String presetName = nameEditor.getText().trim();
    if (presetName.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                               "Save Preset",
                                               "Please enter a name for the preset.");
        return;
    }

    juce::File fileToSave = presetDirectory.getChildFile(presetName + ".json");

    // Basic data to save - in a real app, this would come from the PresetManager or app state
    juce::DynamicObject::Ptr presetData (new juce::DynamicObject());
    presetData->setProperty("version", 1);
    presetData->setProperty("name", presetName);

    // Add category to the JSON if a category other than "All" is selected
    juce::String currentCategory = categoryComboBox.getText();
    if (currentCategory.isNotEmpty() && currentCategory != "All")
    {
        presetData->setProperty("category", currentCategory);
    }


    // Call PresetManager to save.
    // For this task, we assume presetManager.savePreset takes the file and the data.
    // The actual data collection would be more complex.
    presetManager.savePreset(fileToSave, juce::var(presetData.get()));

    // Refresh list
    scanPresetDirectory();
    filterPresetsByCategory(); // apply current filter

    // Try to re-select the saved preset
    int newRowIndex = presetFiles.indexOf(presetName);
    if (newRowIndex >= 0)
        presetListBox.selectRow(newRowIndex);

    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                           "Save Preset",
                                           "Preset saved: " + presetName);
}

void PresetBrowserComponent::categoryChanged()
{
    selectedCategory = categoryComboBox.getText();
    filterPresetsByCategory();
}
