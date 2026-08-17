
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

// Constructor: builds and initializes the plugin’s UI editor.
FileInfectionAudioProcessorEditor::FileInfectionAudioProcessorEditor(FileInfectionAudioProcessor& p)
    : juce::AudioProcessorEditor(&p),   // Initialize the JUCE editor base class with a pointer to the processor
    audioProcessor(p)                 // Store a reference to the processor for accessing parameters/state
{
    setSize(900, 600);                  // Set the initial size of the plugin window
    startTimerHz(30);                   // Call timerCallback() 30 times per second for UI refresh

    // Configure the START button’s text and colors, then add it to the editor.
    startButton.setButtonText("START");
    startButton.setColour(
        juce::TextButton::buttonColourId,
        juce::Colours::darkgreen
    );

    startButton.setColour(
        juce::TextButton::textColourOffId,
        juce::Colours::limegreen
    );

    addAndMakeVisible(startButton);

    // Define what happens when START is clicked:
    // hide the button, show the main panel, and reveal the sliders.
    startButton.onClick = [this]()
        {
            startButton.setVisible(false);
            mainPanel.setVisible(true);

            Glitch_Slider.setVisible(true);
            Corruption_Slider.setVisible(true);
            Malware_Slider.setVisible(true);
            Neuronic_Slider.setVisible(true);
        };

    // Add the main panel to the editor,
    // but keep it hidden until START is pressed.
    addAndMakeVisible(mainPanel);
    mainPanel.setVisible(false);

    // Lambda used to apply the same setup to each slider.
    auto setupKnob = [&](juce::Slider& s)
        {
            // Set the slider to behave like a rotary knob.
            s.setSliderStyle(
                juce::Slider::RotaryHorizontalVerticalDrag
            );

            // Hide the slider's built-in textbox.
            s.setTextBoxStyle(
                juce::Slider::NoTextBox,
                false,
                0,
                0
            );

            // Set knob range and starting value.
            s.setRange(0.0, 1.0);
            s.setValue(0.5f);

            // Hide the knob until START is clicked.
            s.setVisible(false);

            // Add the slider to the main panel.
            mainPanel.addAndMakeVisible(s);
        };

    // Apply the knob setup to each slider.
    setupKnob(Glitch_Slider);
    setupKnob(Corruption_Slider);
    setupKnob(Malware_Slider);
    setupKnob(Neuronic_Slider);

    // Apply the custom LookAndFeel.
    Glitch_Slider.setLookAndFeel(&virusKnobLF);
    Corruption_Slider.setLookAndFeel(&virusKnobLF);
    Malware_Slider.setLookAndFeel(&virusKnobLF);
    Neuronic_Slider.setLookAndFeel(&virusKnobLF);

    // Lambda used to configure labels.
    auto setupLabel =
        [&](juce::Label& l,
            juce::Slider& s,
            const juce::String& t)
        {
            // Set the label text.
            l.setText(
                t,
                juce::dontSendNotification
            );

            // Set label color.
            l.setColour(
                juce::Label::textColourId,
                juce::Colours::limegreen
            );

            // Center the text.
            l.setJustificationType(
                juce::Justification::centred
            );

            // Attach the label to its slider.
            l.attachToComponent(
                &s,
                false
            );

            // Add the label to the main panel.
            mainPanel.addAndMakeVisible(l);
        };

    // Create labels for each knob.
    setupLabel(
        Glitch_Label,
        Glitch_Slider,
        "Glitch"
    );

    setupLabel(
        Corruption_Label,
        Corruption_Slider,
        "Corruption"
    );

    setupLabel(
        Malware_Label,
        Malware_Slider,
        "Malware"
    );

    setupLabel(
        Neuronic_Label,
        Neuronic_Slider,
        "Neuronic"
    );

    // Get the processor's AudioProcessorValueTreeState.
    auto& vts =
        audioProcessor.getValueTreeState();

    // Connect each slider to its processor parameter.
    glitchAttachment =
        std::make_unique<SliderAttachment>(
            vts,
            "glitch",
            Glitch_Slider
        );

    corruptionAttachment =
        std::make_unique<SliderAttachment>(
            vts,
            "corruption",
            Corruption_Slider
        );

    malwareAttachment =
        std::make_unique<SliderAttachment>(
            vts,
            "malware",
            Malware_Slider
        );

    neuronicAttachment =
        std::make_unique<SliderAttachment>(
            vts,
            "neuronic",
            Neuronic_Slider
        );
}

//==============================================================================

// Destructor: runs when the editor is destroyed.
FileInfectionAudioProcessorEditor::~FileInfectionAudioProcessorEditor()
{
}

//==============================================================================

// Timer callback: called at the rate set by startTimerHz(30).
void FileInfectionAudioProcessorEditor::timerCallback()
{
    repaint();
}

//==============================================================================

// paint(): draws the editor itself.
void FileInfectionAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Only draw the splash screen while the main panel is hidden.
    if (!mainPanel.isVisible())
    {
        // Fill the background with a dark color.
        g.fillAll(
            juce::Colour::fromRGB(
                10,
                15,
                20
            )
        );

        // Draw horizontal scanlines.
        for (int y = 0;
            y < getHeight();
            y += 4)
        {
            g.setColour(
                juce::Colour::fromRGBA(
                    0,
                    255,
                    0,
                    20
                )
            );

            g.drawLine(
                0,
                y,
                getWidth(),
                y
            );
        }

        // Draw centered plugin title.
        g.setColour(
            juce::Colours::limegreen
        );

        g.setFont(40.0f);

        g.drawFittedText(
            "FileInfection.exe",
            getLocalBounds(),
            juce::Justification::centred,
            1
        );
    }
}

//==============================================================================

// resized(): positions and sizes UI components whenever
// the editor size changes.
void FileInfectionAudioProcessorEditor::resized()
{
    // Position START near the center.
    startButton.setBounds(
        getWidth() / 2 - 60,
        getHeight() / 2 + 80,
        120,
        50
    );

    // Make the main panel fill the editor.
    mainPanel.setBounds(
        getLocalBounds()
    );

    // Calculate center coordinates.
    const int cx = getWidth() / 2;
    const int cy = getHeight() / 2;

    // Shared knob size.
    const int s = 130;

    // Glitch knob - top.
    Glitch_Slider.setBounds(
        cx - s / 2,
        cy - 200,
        s,
        s
    );

    // Corruption knob - left.
    Corruption_Slider.setBounds(
        cx - 220,
        cy - 10,
        s,
        s
    );

    // Malware knob - right.
    Malware_Slider.setBounds(
        cx + 90,
        cy - 10,
        s,
        s
    );

    // Neuronic knob - bottom.
    Neuronic_Slider.setBounds(
        cx - s / 2,
        cy + 160,
        s,
        s
    );
}
