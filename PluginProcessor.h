#pragma once // tells the compiler include this file only once. this prevents class redefinitions and duplications and extra errors.

#include <JuceHeader.h> // tells the compiler to include the Juce Framework Library, and all classes... etc

//==============================================================================
//Note: class is called NewProject due to the title being called a new project, but we can say its a 404.exeAudioProcessor
//override is written in the header to tell JUCE or make sure that the function exists,
and the .cpp file is where I define what it actually does.
// Main audio processor class. Every plugin must have a Audio processor class.
// This is  where all DSP and parameter state lives.
// This class is just saying this is a class called audio Processor hence the :, However, it is using the JUCE framework.
// From here JUCE knows to call process block and prepare to play. It is just saying this class is a JUCE audio plugin processor.
class NewProjectAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    // Constructor & Destructor
    // Constructor sets up buses and parameters (implemented in .cpp)
   //
    NewProjectAudioProcessor(); // tells the compiler or c++ that this constructor exists and creates it, and without this, the code that creates
// the plugin wont know what to call.
    ~NewProjectAudioProcessor() override; //Destructor

    //============================================================================== 
    
    // Called before audio playback starts. Or in other words, audio is about to start
    // Used to initialize DSP, allocate buffers, and store the sample rate.
    void prepareToPlay(double sampleRate, int samplesPerBlock) override; // function

    // Called when playback stops or the plugin is unloaded.
    // Usually used to free resources if needed.
    void releaseResources() override; // override just means make sure this is the same function that the JUCE class or JUCE function wants.
// so this cleans up memory, buffers, and any other resources.

#ifndef JucePlugin_PreferredChannelConfigurations
    // Determines whether a given channel layout (mono/stereo/etc.) is supported.
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    //==============================================================================
    // Main audio processing callback.
    // This is where audio samples are read, processed, and written back.
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override; // Juce will call this function and provide me with values or sample of audio and MIDI.
//Without this, the program would be silent. This is called over and over when audio is being played, on the audio thread, or if the DAW needs more sound
//override makes sure JUCE calls the function and matches JUCEs process block. Essentially it is saying use my version of the function has JUCE already defines these functions

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

    // MIDI support flags. Const prevents data from being altered. Without this, audio can glitch.
    bool acceptsMidi() const override; // is asking if the plugin can accept midi functions
    bool producesMidi() const override; // is asking if the plugin can send out MIDI to something else
    bool isMidiEffect() const override; // Is asking if this plugin is only a MIDI tool or plugin effect

    // Tail length in seconds (used for delays/reverbs).
    double getTailLengthSeconds() const override; // this is saying after the sound stops, how long will the plugin keep making sounds

    //==============================================================================
    // Program / preset handling (legacy-style programs)
    int getNumPrograms() override; // Essentially asking how many presets does my plugin have
    int getCurrentProgram() override; // Analyzes which preset is active
    void setCurrentProgram(int index) override; //switches the preset to the current number or switch to preset index
    const juce::String getProgramName(int index) override; //asks or analyzes the preset's name. Essentially its figuring out what should it be called
    void changeProgramName(int index, const juce::String& newName) override; // signals if a user renames a preset.

    //==============================================================================
    // State saving/loading
    // These functions allow the host (DAW) to save and restore plugin settings.
    void getStateInformation(juce::MemoryBlock& destData) override; //write down/save the settings
    void setStateInformation(const void* data, int sizeInBytes) override; // read them back

    //==============================================================================
    // Helper function for the editor
    // Allows the GUI to access the AudioProcessorValueTreeState
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

private:
    //==============================================================================
    // ---------------- DSP HELPER VARIABLES ----------------

    // Random number generator (useful for glitch/randomized effects)
    juce::Random random; //Random audio effects to give that glitchy distored feel by skipping values.

    // Current sample rate provided by the host, or in this case the DAW.
    double currentSampleRate = 44100.0; // JUCE uses this in PreparetoPlay

    //==============================================================================
    // ---------------- GLITCH EFFECT (DECIMATOR / SAMPLE HOLD) ----------------

    // Stores the last held sample for each channel (L/R)
    float sampleAccumulator[2] = { 0.0f, 0.0f }; //stores left and right samples, as this prevents updating the sample, and allows for repeated outputs.

    // Counts how many samples should be skipped before updating the held sample
    int skipCounter[2] = { 0, 0 };
    //==============================================================================
    // ---------------- MALWARE EFFECT (TREMOLO / AM) ----------------

    // Phase accumulator for the tremolo LFO
    float tremoloPhase = 0.0f; //stores the current location of the sine wave

    //==============================================================================
    // ---------------- NEURONIC EFFECT (CUSTOM DELAY) ----------------

    // Circular buffer used to store delayed audio samples
    juce::AudioSampleBuffer delayBuffer; // stores a chunk of memory 

    // Current write position inside the delay buffer
    int writePosition = 0;

    // Maximum delay length in samples: prevents corruption and crashes
    int maxDelayInSamples = 0;

    //==============================================================================
    // ---------------- PARAMETER STATE ----------------

    // AudioProcessorValueTreeState
    // Manages all parameters, automation, and state saving, like knobs, sliders. Editor talks to this and processor reads from it. Keeps the UI connected,
    juce::AudioProcessorValueTreeState parameters;

    // Prevents copying and adds memory leak detection in debug builds.. This can lead to crashes, loud glitches, and memory corruption:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessor)
};
