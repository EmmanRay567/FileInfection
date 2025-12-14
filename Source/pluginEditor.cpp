
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor(NewProjectAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(900, 600);
    startTimerHz(30);

    startButton.setButtonText("START");
    startButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);
    startButton.setColour(juce::TextButton::textColourOffId, juce::Colours::limegreen);
    addAndMakeVisible(startButton);

    startButton.onClick = [this]()
        {
            startButton.setVisible(false);
            mainPanel.setVisible(true);

            Glitch_Slider.setVisible(true);
            Corruption_Slider.setVisible(true);
            Malware_Slider.setVisible(true);
            Neuronic_Slider.setVisible(true);
        };

    addAndMakeVisible(mainPanel);
    mainPanel.setVisible(false);

    auto setupKnob = [&](juce::Slider& s)
        {
            s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);

            // ✅ REMOVES BOXES + NUMBERS
            s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

            s.setRange(0.0, 1.0);
            s.setValue(0.5f);
            s.setVisible(false);
            mainPanel.addAndMakeVisible(s);
        };

    setupKnob(Glitch_Slider);
    setupKnob(Corruption_Slider);
    setupKnob(Malware_Slider);
    setupKnob(Neuronic_Slider);

    // ===== ADDON: APPLY CUSTOM KNOB LOOK =====
    Glitch_Slider.setLookAndFeel(&virusKnobLF);
    Corruption_Slider.setLookAndFeel(&virusKnobLF);
    Malware_Slider.setLookAndFeel(&virusKnobLF);
    Neuronic_Slider.setLookAndFeel(&virusKnobLF);

    auto setupLabel = [&](juce::Label& l, juce::Slider& s, const juce::String& t)
        {
            l.setText(t, juce::dontSendNotification);
            l.setColour(juce::Label::textColourId, juce::Colours::limegreen);
            l.setJustificationType(juce::Justification::centred);
            l.attachToComponent(&s, false);
            mainPanel.addAndMakeVisible(l);
        };

    setupLabel(Glitch_Label, Glitch_Slider, "Glitch");
    setupLabel(Corruption_Label, Corruption_Slider, "Corruption");
    setupLabel(Malware_Label, Malware_Slider, "Malware");
    setupLabel(Neuronic_Label, Neuronic_Slider, "Neuronic");

    auto& vts = audioProcessor.getValueTreeState();
    glitchAttachment = std::make_unique<SliderAttachment>(vts, "glitch", Glitch_Slider);
    corruptionAttachment = std::make_unique<SliderAttachment>(vts, "corruption", Corruption_Slider);
    malwareAttachment = std::make_unique<SliderAttachment>(vts, "malware", Malware_Slider);
    neuronicAttachment = std::make_unique<SliderAttachment>(vts, "neuronic", Neuronic_Slider);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor() {}

void NewProjectAudioProcessorEditor::timerCallback()
{
    repaint();
}

void NewProjectAudioProcessorEditor::paint(juce::Graphics& g)
{
    if (!mainPanel.isVisible())
    {
        g.fillAll(juce::Colour::fromRGB(10, 15, 20));

        for (int y = 0; y < getHeight(); y += 4)
        {
            g.setColour(juce::Colour::fromRGBA(0, 255, 0, 20));
            g.drawLine(0, y, getWidth(), y);
        }

        g.setColour(juce::Colours::limegreen);
        g.setFont(40.0f);
        g.drawFittedText("VirusInfection.exe",
            getLocalBounds(),
            juce::Justification::centred,
            1);
    }
}

void NewProjectAudioProcessorEditor::resized()
{
    startButton.setBounds(getWidth() / 2 - 60, getHeight() / 2 + 80, 120, 50);
    mainPanel.setBounds(getLocalBounds());

    const int cx = getWidth() / 2;
    const int cy = getHeight() / 2;
    const int s = 130;

    Glitch_Slider.setBounds(cx - s / 2, cy - 200, s, s);
    Corruption_Slider.setBounds(cx - 220, cy - 10, s, s);
    Malware_Slider.setBounds(cx + 90, cy - 10, s, s);
    Neuronic_Slider.setBounds(cx - s / 2, cy + 160, s, s);
}
