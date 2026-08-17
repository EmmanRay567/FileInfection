#pragma once
// Tells the compiler to include this file only once.
// This prevents class redefinitions, duplications, and extra errors.

#include <JuceHeader.h>
// Tells the compiler to include the JUCE Framework Library and its classes.

//==============================================================================
// Main audio processor class.
//
// override is written in the header to tell JUCE or make sure that
// the function exists, and the .cpp file is where I define what it actually does.
//
// Every plugin must have an AudioProcessor class.
// This is where all DSP and parameter state lives.
//
// The ": public juce::AudioProcessor" means this class inherits from
// JUCE's AudioProcessor class.
//
// From here JUCE knows to call functions such as processBlock()
// and prepareToPlay().

class FileInfectionAudioProcessor : public juce::AudioProcessor
{
public:

    //==========================================================================
    // Constructor & Destructor

    // Constructor sets up buses and parameters.
    FileInfectionAudioProcessor();

    // Destructor
    ~FileInfectionAudioProcessor() override;

    //==========================================================================
    // AUDIO SETUP

    // Called before audio playback starts.
    // Used to initialize DSP, allocate buffers, and store the sample rate.
    void prepareToPlay(
        double sampleRate,
        int samplesPerBlock
    ) override;

    // Called when playback stops or the plugin is unloaded.
    // Used to free resources if needed.
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations

    // Determines whether a given channel layout
    // such as mono or stereo is supported.
    bool isBusesLayoutSupported(
        const BusesLayout& layouts
    ) const override;

#endif

    //==========================================================================
    // MAIN AUDIO PROCESSING

    // Main audio processing callback.
    //
    // JUCE repeatedly calls this function while audio is playing.
    // The DAW gives the plugin a buffer containing audio samples.
    //
    // This function reads those samples, processes them,
    // and writes the modified samples back into the buffer.
    void processBlock(
        juce::AudioBuffer<float>&,
        juce::MidiBuffer&
    ) override;

    //==========================================================================
    // EDITOR / GUI

    // Creates and returns the plugin editor.
    juce::AudioProcessorEditor* createEditor() override;

    // Tells the host that this plugin has a GUI.
    bool hasEditor() const override;

    //==========================================================================
    // PLUGIN METADATA

    // Returns the name of the plugin.
    const juce::String getName() const override;

    //==========================================================================
    // MIDI SUPPORT

    // Checks whether the plugin accepts MIDI input.
    bool acceptsMidi() const override;

    // Checks whether the plugin produces MIDI output.
    bool producesMidi() const override;

    // Checks whether this is a MIDI-only effect.
    bool isMidiEffect() const override;

    //==========================================================================
    // AUDIO TAIL

    // Tells the DAW how long the plugin may keep producing sound
    // after the input stops.
    double getTailLengthSeconds() const override;

    //==========================================================================
    // PROGRAM / PRESET HANDLING

    // How many programs / presets exist.
    int getNumPrograms() override;

    // Returns the currently selected preset.
    int getCurrentProgram() override;

    // Changes the currently selected preset.
    void setCurrentProgram(
        int index
    ) override;

    // Returns the name of a preset.
    const juce::String getProgramName(
        int index
    ) override;

    // Allows a preset to be renamed.
    void changeProgramName(
        int index,
        const juce::String& newName
    ) override;

    //==========================================================================
    // STATE SAVING / LOADING

    // Saves plugin settings so the DAW can restore them later.
    void getStateInformation(
        juce::MemoryBlock& destData
    ) override;

    // Loads previously saved plugin settings.
    void setStateInformation(
        const void* data,
        int sizeInBytes
    ) override;

    //==========================================================================
    // PARAMETER ACCESS

    // Allows PluginEditor to access the parameter state.
    juce::AudioProcessorValueTreeState& getValueTreeState()
    {
        return parameters;
    }

private:

    //==========================================================================
    // DSP HELPER VARIABLES

    // Random number generator.
    // Used for randomized glitch-style effects.
    juce::Random random;

    // Current sample rate provided by the DAW.
    double currentSampleRate = 44100.0;

    //==========================================================================
    // GLITCH EFFECT
    // DECIMATOR / SAMPLE HOLD

    // Stores the last held sample for left and right channels.
    float sampleAccumulator[2] =
    {
        0.0f,
        0.0f
    };

    // Counts how many samples should be skipped
    // before updating the held sample.
    int skipCounter[2] =
    {
        0,
        0
    };

    //==========================================================================
    // MALWARE EFFECT
    // TREMOLO / AMPLITUDE MODULATION

    // Stores the current position of the tremolo sine wave.
    float tremoloPhase = 0.0f;

    //==========================================================================
    // NEURONIC EFFECT
    // CUSTOM DELAY

    // Circular buffer used to store delayed audio samples.
    juce::AudioSampleBuffer delayBuffer;

    // Current write position inside the delay buffer.
    int writePosition = 0;

    // Maximum delay length in samples.
    int maxDelayInSamples = 0;

    //==========================================================================
    // PARAMETER STATE

    // Manages plugin parameters, automation, and state saving.
    //
    // The editor connects its knobs to this object,
    // while the processor reads the parameter values during DSP.
    juce::AudioProcessorValueTreeState parameters;

    //==========================================================================
    // Prevent copying and add memory leak detection in debug builds.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        FileInfectionAudioProcessor
    )
};
