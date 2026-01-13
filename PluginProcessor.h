#pragma once

#include <JuceHeader.h>

//==============================================================================
// Main audio processor class.
// This is  where all DSP and parameter state lives.
class NewProjectAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    // Constructor & Destructor
    // Constructor sets up buses and parameters (implemented in .cpp)
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    //==============================================================================
    // Called before audio playback starts.
    // Used to initialize DSP, allocate buffers, and store the sample rate.
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    // Called when playback stops or the plugin is unloaded.
    // Usually used to free resources if needed.
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    // Determines whether a given channel layout (mono/stereo/etc.) is supported.
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    //==============================================================================
    // Main audio processing callback.
    // This is where audio samples are read, processed, and written back.
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    // Editor (GUI) creation
    // Returns the plugin editor component.
    juce::AudioProcessorEditor* createEditor() override;

    // Tells the host whether this plugin has a GUI.
    bool hasEditor() const override;

    //==============================================================================
    // Plugin metadata
    // Returns the name of the plugin.
    const juce::String getName() const override;

    // MIDI support flags
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    // Tail length in seconds (used for delays/reverbs).
    double getTailLengthSeconds() const override;

    //==============================================================================
    // Program / preset handling (legacy-style programs)
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    // State saving/loading
    // These functions allow the host (DAW) to save and restore plugin settings.
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Helper function for the editor
    // Allows the GUI to access the AudioProcessorValueTreeState
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

private:
    //==============================================================================
    // ---------------- DSP HELPER VARIABLES ----------------

    // Random number generator (useful for glitch/randomized effects)
    juce::Random random;

    // Current sample rate provided by the host
    double currentSampleRate = 44100.0;

    //==============================================================================
    // ---------------- GLITCH EFFECT (DECIMATOR / SAMPLE HOLD) ----------------

    // Stores the last held sample for each channel (L/R)
    float sampleAccumulator[2] = { 0.0f, 0.0f };

    // Counts how many samples should be skipped before updating the held sample
    int skipCounter[2] = { 0, 0 };

    //==============================================================================
    // ---------------- MALWARE EFFECT (TREMOLO / AM) ----------------

    // Phase accumulator for the tremolo LFO
    float tremoloPhase = 0.0f;

    //==============================================================================
    // ---------------- NEURONIC EFFECT (CUSTOM DELAY) ----------------

    // Circular buffer used to store delayed audio samples
    juce::AudioSampleBuffer delayBuffer;

    // Current write position inside the delay buffer
    int writePosition = 0;

    // Maximum delay length in samples
    int maxDelayInSamples = 0;

    //==============================================================================
    // ---------------- PARAMETER STATE ----------------

    // AudioProcessorValueTreeState
    // Manages all parameters, automation, and state saving
    juce::AudioProcessorValueTreeState parameters;

    // Prevents copying and adds memory leak detection in debug builds
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessor)
};
