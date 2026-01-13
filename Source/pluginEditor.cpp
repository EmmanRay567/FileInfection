#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

// Constructor: builds and initializes the plugin’s UI editor.
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor(NewProjectAudioProcessor& p)
    : AudioProcessorEditor(&p),   // Initialize the JUCE editor base class with a pointer to the processor
      audioProcessor(p)           // Store a reference to the processor for accessing parameters/state
{
    setSize(900, 600);            // Set the initial size of the plugin window
    startTimerHz(30);             // Call timerCallback() 30 times per second for UI refresh

    // Configure the START button’s text and colors, then add it to the editor.
    startButton.setButtonText("START");
    startButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);
    startButton.setColour(juce::TextButton::textColourOffId, juce::Colours::limegreen);
    addAndMakeVisible(startButton);

    // Define what happens when START is clicked: hide the button, show the main panel,
    // and reveal the sliders.
    // “[This]() defines what code runs when the button is clicked, and [this] lets that code access the editor’s variables.
    // or another way to say is that When the button is clicked, change do a specific action.
    startButton.onClick = [this]() 
    {
        startButton.setVisible(false);
        mainPanel.setVisible(true);

        Glitch_Slider.setVisible(true);
        Corruption_Slider.setVisible(true);
        Malware_Slider.setVisible(true);
        Neuronic_Slider.setVisible(true);
    };

    // Add the main panel to the editor, but keep it hidden until START is pressed.
    addAndMakeVisible(mainPanel);
    mainPanel.setVisible(false);

    // Lambda used to apply the same setup to each slider (knob).
    auto setupKnob = [&](juce::Slider& s)
    {
        // Set the slider to behave like a rotary knob controlled by dragging.
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);

        // Hide the slider’s built-in textbox so the control shows no numeric entry/display.
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

        // Set the knob’s range and initial value.
        s.setRange(0.0, 1.0);
        s.setValue(0.5f);

        // Keep the knob hidden at first; it becomes visible after START is clicked.
        s.setVisible(false);

        // Add the slider to the main panel so it appears on top of that background.
        mainPanel.addAndMakeVisible(s);
    };

    // Apply the same knob configuration to each slider.
    setupKnob(Glitch_Slider);
    setupKnob(Corruption_Slider);
    setupKnob(Malware_Slider);
    setupKnob(Neuronic_Slider);

    // Apply the custom LookAndFeel so these sliders are drawn with your custom knob style.
    Glitch_Slider.setLookAndFeel(&virusKnobLF);
    Corruption_Slider.setLookAndFeel(&virusKnobLF);
    Malware_Slider.setLookAndFeel(&virusKnobLF);
    Neuronic_Slider.setLookAndFeel(&virusKnobLF);

    // Lambda used to apply consistent label settings and attach each label to its slider.
    auto setupLabel = [&](juce::Label& l, juce::Slider& s, const juce::String& t)
    {
        // Set the label text without triggering notifications.
        l.setText(t, juce::dontSendNotification);

        // Style and align the label text.
        l.setColour(juce::Label::textColourId, juce::Colours::limegreen);
        l.setJustificationType(juce::Justification::centred);

        // Attach the label to the slider so it stays positioned relative to that slider.
        l.attachToComponent(&s, false);

        // Add the label to the main panel so it shows with the rest of the UI.
        mainPanel.addAndMakeVisible(l);
    };

    // Create and attach labels for each knob.
    setupLabel(Glitch_Label, Glitch_Slider, "Glitch");
    setupLabel(Corruption_Label, Corruption_Slider, "Corruption");
    setupLabel(Malware_Label, Malware_Slider, "Malware");
    setupLabel(Neuronic_Label, Neuronic_Slider, "Neuronic");

    // Connect each slider to an AudioProcessorValueTreeState parameter using attachments.
    // This keeps the slider values synchronized with the processor parameters.
    auto& vts = audioProcessor.getValueTreeState();
    glitchAttachment     = std::make_unique<SliderAttachment>(vts, "glitch",     Glitch_Slider);
    corruptionAttachment = std::make_unique<SliderAttachment>(vts, "corruption", Corruption_Slider);
    malwareAttachment    = std::make_unique<SliderAttachment>(vts, "malware",    Malware_Slider);
    neuronicAttachment   = std::make_unique<SliderAttachment>(vts, "neuronic",   Neuronic_Slider);
}

// Destructor: runs when the editor is destroyed.
NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor() {}

// Timer callback: called at the rate set by startTimerHz(30).
// repaint() requests that the editor be redrawn.
void NewProjectAudioProcessorEditor::timerCallback()
{
    repaint();
}

// paint(): draws the editor itself.
// This draws a title/splash screen only while the main panel is hidden.
void NewProjectAudioProcessorEditor::paint(juce::Graphics& g)
{
    // If the main panel is not visible, draw the initial screen.
    if (!mainPanel.isVisible())
    {
        // Fill background with a dark color.
        g.fillAll(juce::Colour::fromRGB(10, 15, 20));

        // Draw horizontal scanlines across the editor.
        for (int y = 0; y < getHeight(); y += 4)
        {
            g.setColour(juce::Colour::fromRGBA(0, 255, 0, 20));
            g.drawLine(0, y, getWidth(), y);
        }

        // Draw centered title text.
        g.setColour(juce::Colours::limegreen);
        g.setFont(40.0f);
        g.drawFittedText("VirusInfection.exe",
                         getLocalBounds(),
                         juce::Justification::centred,
                         1);
    }
}

// resized(): positions and sizes UI components whenever the editor size changes.
void NewProjectAudioProcessorEditor::resized()
{
    // Position the START button near the center of the window.
    startButton.setBounds(getWidth() / 2 - 60, getHeight() / 2 + 80, 120, 50);

    // Make the main panel fill the entire editor.
    mainPanel.setBounds(getLocalBounds());

    // Compute center coordinates and a shared knob size.
    const int cx = getWidth() / 2;
    const int cy = getHeight() / 2;
    const int s  = 130;

    // Arrange the knobs around the center.
    Glitch_Slider.setBounds(cx - s / 2, cy - 200, s, s);
    Corruption_Slider.setBounds(cx - 220, cy - 10,  s, s);
    Malware_Slider.setBounds(cx + 90,  cy - 10,  s, s);
    Neuronic_Slider.setBounds(cx - s / 2, cy + 160, s, s);
}
