#pragma once  // Prevents this header from being included more than once

#include <JuceHeader.h>   // Brings in JUCE UI, graphics, and utilities
#include <memory>         // For std::unique_ptr
#include "PluginProcessor.h" // Lets the editor talk to the audio processor

//==============================================================================
// CUSTOM LOOK AND FEEL FOR ROTARY KNOBS
// This struct customizes how rotary sliders are drawn
struct VirusKnobLookAndFeel : public juce::LookAndFeel_V4
{
    // This function overrides JUCE’s default rotary slider drawing
    void drawRotarySlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPosProportional,   // Normalized value (0.0 → 1.0)
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override
    {
        // Calculate the radius so the knob scales with size
        // auto lets the compiler figure out what the data type is
        auto radius = juce::jmin(width, height) * 0.5f - 12.0f;

        // Calculate the center of the slider
        auto centreX = x + width * 0.5f;
        auto centreY = y + height * 0.5f;

        constexpr float thickness = 6.0f; // Thickness of the arc stroke

        // ---- Outer dark ring ----
        // Draws a subtle background ring for depth and contrast
        g.setColour(juce::Colour::fromRGB(10, 25, 10));
        g.drawEllipse(
            centreX - radius,
            centreY - radius,
            radius * 2.0f,
            radius * 2.0f,
            4.0f
        );

        // ---- Green value arc / full ring ----
        // This color represents the current knob value
        g.setColour(juce::Colours::limegreen);

        // Defines how the arc is drawn (thickness + rounded ends)
        juce::PathStrokeType stroke(thickness);
        stroke.setEndStyle(juce::PathStrokeType::rounded);

        // Path is used so we can build the shape before drawing it
        juce::Path path;

        // If the knob is at max value, draw a full circle
        // This avoids a small visual gap caused by floating-point precision
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
            // Convert normalized value into an angle
            auto angle = rotaryStartAngle +
                sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

            // Draw an arc from the start angle to the current value
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

        // Render the final path using the stroke style
        g.strokePath(path, stroke);
    }
};

//==============================================================================
// MAIN PLUGIN EDITOR CLASS (UI ONLY)
class NewProjectAudioProcessorEditor :
    public juce::AudioProcessorEditor, // Base class for plugin UIs
    public juce::Timer                 // Used for UI-only updates
{
public:
    NewProjectAudioProcessorEditor(NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    // Draws the editor’s visuals
    void paint(juce::Graphics&) override;

    // Positions UI components when the window is resized
    void resized() override;

    // Called periodically for visual effects (NOT audio)
    void timerCallback() override;

private:
    // Reference to the audio processor (editor does not own it)
    NewProjectAudioProcessor& audioProcessor;

    // Used for visual glitch effects only
    int glitchOffset = 0;

    // ===== BUTTON =====
    juce::TextButton startButton;

    // ===== SLIDERS (UI controls for parameters) =====
    juce::Slider Glitch_Slider;
    juce::Slider Corruption_Slider;
    juce::Slider Malware_Slider;
    juce::Slider Neuronic_Slider;

    // ===== LABELS (describe each slider) =====
    juce::Label Glitch_Label;
    juce::Label Corruption_Label;
    juce::Label Malware_Label;
    juce::Label Neuronic_Label;

    // ===== PARAMETER ATTACHMENTS =====
    // Attach sliders to audio parameters safely and automatically
    using SliderAttachment =
        juce::AudioProcessorValueTreeState::SliderAttachment;
//This creates pointers owns the object it points to
// in other words, no other pointers or variables can have the same object
// if the pointers are deleted or removed, the object is deleted.

    std::unique_ptr<SliderAttachment> glitchAttachment;
    std::unique_ptr<SliderAttachment> corruptionAttachment;
    std::unique_ptr<SliderAttachment> malwareAttachment;
    std::unique_ptr<SliderAttachment> neuronicAttachment;

    // Custom look-and-feel instance shared by knobs
    VirusKnobLookAndFeel virusKnobLF;

    // ===== BACKGROUND PANEL COMPONENT =====
// This section edits the UI, or the way the screen looks.
    struct MainPanel : public juce::Component
    {
        // Draws the plugin background
        void paint(juce::Graphics& g) override
        {
            // Vertical green gradient for depth
            juce::ColourGradient gradient(
                juce::Colours::limegreen, 0, 0,
                juce::Colour::fromRGB(5, 30, 5),
                0, (float)getHeight(),
                false);

            g.setGradientFill(gradient);
            g.fillAll();

            // Scanlines for subtle texture
            for (int y = 0; y < getHeight(); y += 3)
            {
                g.setColour(juce::Colour::fromRGBA(0, 0, 0, 25));
                g.drawLine(0, (float)y, (float)getWidth(), (float)y);
            }

            // Procedural static noise (lightweight, no textures)
            juce::Random rng;
            for (int i = 0; i < 200; ++i)
            {
                g.setColour(juce::Colour::fromRGBA(
                    0,
                    rng.nextInt(150) + 100,
                    0,
                    rng.nextInt(80) + 20));

                g.fillRect(
                    rng.nextInt(getWidth()),
                    rng.nextInt(getHeight()),
                    1, 1
                );
            }
        }
    };

    // Instance of the background panel
    MainPanel mainPanel;

    // Prevent copying and help detect memory leaks during development
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        NewProjectAudioProcessorEditor)
};
