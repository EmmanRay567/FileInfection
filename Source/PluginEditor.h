
#pragma once

#include <JuceHeader.h>
#include <memory>
#include "PluginProcessor.h"

//==============================================================================
// ===== CUSTOM KNOB LOOK (CONNECTED ARC + FULL AT MAX) =====
struct VirusKnobLookAndFeel : public juce::LookAndFeel_V4
{
    void drawRotarySlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override
    {
        auto radius = juce::jmin(width, height) * 0.5f - 12.0f;
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;

        constexpr float thickness = 6.0f;

        // ---- Outer dark ring ----
        g.setColour(juce::Colour::fromRGB(10, 25, 10));
        g.drawEllipse(centreX - radius,
            centreY - radius,
            radius * 2.0f,
            radius * 2.0f,
            4.0f);

        // ---- Green arc / full ring ----
        g.setColour(juce::Colours::limegreen);

        juce::PathStrokeType stroke(thickness);
        stroke.setEndStyle(juce::PathStrokeType::rounded);

        juce::Path path;

        // ===== FINAL FIX =====
        // When fully turned, draw a perfect closed circle
        if (sliderPosProportional >= 0.999f)
        {
            path.addEllipse(
                centreX - radius,
                centreY - radius,
                radius * 2.0f,
                radius * 2.0f
            );
        }
        else
        {
            auto angle = rotaryStartAngle +
                sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

            path.addCentredArc(
                centreX,
                centreY,
                radius,
                radius,
                0.0f,
                rotaryStartAngle,
                angle,
                true
            );
        }

        g.strokePath(path, stroke);
    }
};

//==============================================================================

class NewProjectAudioProcessorEditor :
    public juce::AudioProcessorEditor,
    public juce::Timer
{
public:
    NewProjectAudioProcessorEditor(NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    NewProjectAudioProcessor& audioProcessor;
    int glitchOffset = 0;

    // ===== BUTTON =====
    juce::TextButton startButton;

    // ===== SLIDERS =====
    juce::Slider Glitch_Slider;
    juce::Slider Corruption_Slider;
    juce::Slider Malware_Slider;
    juce::Slider Neuronic_Slider;

    // ===== LABELS =====
    juce::Label Glitch_Label;
    juce::Label Corruption_Label;
    juce::Label Malware_Label;
    juce::Label Neuronic_Label;

    // ===== PARAMETER ATTACHMENTS =====
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    std::unique_ptr<SliderAttachment> glitchAttachment;
    std::unique_ptr<SliderAttachment> corruptionAttachment;
    std::unique_ptr<SliderAttachment> malwareAttachment;
    std::unique_ptr<SliderAttachment> neuronicAttachment;

    // ===== LOOK AND FEEL INSTANCE =====
    VirusKnobLookAndFeel virusKnobLF;

    // ===== MAIN PANEL =====
    struct MainPanel : public juce::Component
    {
        void paint(juce::Graphics& g) override
        {
            juce::ColourGradient gradient(
                juce::Colours::limegreen, 0, 0,
                juce::Colour::fromRGB(5, 30, 5),
                0, (float)getHeight(),
                false);

            g.setGradientFill(gradient);
            g.fillAll();

            // Scanlines
            for (int y = 0; y < getHeight(); y += 3)
            {
                g.setColour(juce::Colour::fromRGBA(0, 0, 0, 25));
                g.drawLine(0, (float)y, (float)getWidth(), (float)y);
            }

            // Static noise
            juce::Random rng;
            for (int i = 0; i < 200; ++i)
            {
                g.setColour(juce::Colour::fromRGBA(
                    0,
                    rng.nextInt(150) + 100,
                    0,
                    rng.nextInt(80) + 20));

                g.fillRect(rng.nextInt(getWidth()),
                    rng.nextInt(getHeight()),
                    1, 1);
            }
        }
    };

    MainPanel mainPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessorEditor)
};
