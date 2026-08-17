#include "PluginProcessor.h" // this is saying include the header file where classes and functions were declared
#include "PluginEditor.h"    // same for editor

//==============================================================================
// Helper function to define the parameters
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "glitch",
            "Glitch",
            0.0f,
            1.0f,
            0.5f
        )
    );

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "corruption",
            "Corruption",
            0.0f,
            1.0f,
            0.5f
        )
    );

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "malware",
            "Malware",
            0.0f,
            1.0f,
            0.5f
        )
    );

    params.push_back(
        std::make_unique<juce::AudioParameterFloat>(
            "neuronic",
            "Neuronic",
            0.0f,
            1.0f,
            0.5f
        )
    );

    return { params.begin(), params.end() };
}

//==============================================================================

FileInfectionAudioProcessor::FileInfectionAudioProcessor()

#ifndef JucePlugin_PreferredChannelConfigurations

    : AudioProcessor(
        BusesProperties()
        .withInput(
            "Input",
            juce::AudioChannelSet::stereo(),
            true
        )
        .withOutput(
            "Output",
            juce::AudioChannelSet::stereo(),
            true
        )
    ),

#endif

    parameters(
        *this,
        nullptr,
        "PARAMS",
        createParameterLayout()
    )
{
}

FileInfectionAudioProcessor::~FileInfectionAudioProcessor()
{
}

//==============================================================================

const juce::String FileInfectionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FileInfectionAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool FileInfectionAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool FileInfectionAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double FileInfectionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FileInfectionAudioProcessor::getNumPrograms()
{
    return 1;
}

int FileInfectionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FileInfectionAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String FileInfectionAudioProcessor::getProgramName(int index)
{
    return {};
}

void FileInfectionAudioProcessor::changeProgramName(
    int index,
    const juce::String& newName
)
{
}

//==============================================================================

void FileInfectionAudioProcessor::prepareToPlay(
    double sampleRate,
    int samplesPerBlock
)
{
    currentSampleRate = sampleRate;

    // --- SETUP CUSTOM DELAY BUFFER FOR 'NEURONIC' ---

    // Max delay time of 0.1 seconds (100ms)
    float maxDelayTimeSeconds = 0.1f;

    maxDelayInSamples =
        (int)(maxDelayTimeSeconds * currentSampleRate);

    // Allocate buffer for max delay time and number of channels
    delayBuffer.setSize(
        getTotalNumOutputChannels(),
        maxDelayInSamples
    );

    delayBuffer.clear();

    writePosition = 0;

    // Reset state variables
    sampleAccumulator[0] = 0.0f;
    sampleAccumulator[1] = 0.0f;

    skipCounter[0] = 0;
    skipCounter[1] = 0;

    tremoloPhase = 0.0f;
}

void FileInfectionAudioProcessor::releaseResources()
{
    // Free up the delay buffer memory
    delayBuffer.setSize(0, 0);
}

//==============================================================================

#ifndef JucePlugin_PreferredChannelConfigurations

bool FileInfectionAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts
) const
{
    if (
        layouts.getMainOutputChannelSet()
        != juce::AudioChannelSet::mono()

        &&

        layouts.getMainOutputChannelSet()
        != juce::AudioChannelSet::stereo()
        )
    {
        return false;
    }

#if !JucePlugin_IsMidiEffect

    if (
        layouts.getMainOutputChannelSet()
        != layouts.getMainInputChannelSet()
        )
    {
        return false;
    }

#endif

    return true;
}

#endif

//==============================================================================

void FileInfectionAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages
)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels =
        getTotalNumInputChannels();

    auto totalNumOutputChannels =
        getTotalNumOutputChannels();

    // Clear output channels that didn't contain input data
    for (
        auto i = totalNumInputChannels;
        i < totalNumOutputChannels;
        ++i
        )
    {
        buffer.clear(
            i,
            0,
            buffer.getNumSamples()
        );
    }

    // --- GET PARAMETER VALUES (0.0 to 1.0) ---

    auto glitchAmount =
        parameters
        .getRawParameterValue("glitch")
        ->load();

    auto corruptionAmount =
        parameters
        .getRawParameterValue("corruption")
        ->load();

    auto malwareAmount =
        parameters
        .getRawParameterValue("malware")
        ->load();

    auto neuronicAmount =
        parameters
        .getRawParameterValue("neuronic")
        ->load();

    // --- Prepare Delay Settings for Neuronic ---

    int delayInSamples = 0;

    if (neuronicAmount > 0.01f)
    {
        delayInSamples =
            (int)juce::jmap(
                neuronicAmount,
                0.0f,
                1.0f,
                1.0f,
                (float)maxDelayInSamples
            );

        delayInSamples =
            juce::jmax(
                1,
                delayInSamples
            );
    }

    auto* delayData =
        delayBuffer.getArrayOfWritePointers();

    // --- PER-SAMPLE PROCESSING LOOP ---

    for (
        int channel = 0;
        channel < totalNumInputChannels;
        ++channel
        )
    {
        auto* channelData =
            buffer.getWritePointer(channel);

        for (
            int sample = 0;
            sample < buffer.getNumSamples();
            ++sample
            )
        {
            float inputSample =
                channelData[sample];

            float processedSample =
                inputSample;

            // =============================================================
            // 1. GLITCH EFFECT
            // Sample-Rate Reduction / Decimator
            // =============================================================

            if (glitchAmount > 0.01f)
            {
                int maxSkipFactor =
                    (int)juce::jmap(
                        glitchAmount,
                        0.0f,
                        1.0f,
                        1.0f,
                        100.0f
                    );

                if (skipCounter[channel] <= 0)
                {
                    sampleAccumulator[channel] =
                        processedSample;

                    skipCounter[channel] =
                        random.nextInt(maxSkipFactor) + 1;
                }

                processedSample =
                    sampleAccumulator[channel];

                skipCounter[channel]--;
            }

            // =============================================================
            // 2. CORRUPTION
            // Bit Crusher
            // =============================================================

            if (corruptionAmount > 0.01f)
            {
                // Map knob from 16 bits down to 2 bits
                float bitDepth =
                    juce::jmap(
                        corruptionAmount,
                        0.0f,
                        1.0f,
                        16.0f,
                        2.0f
                    );

                float steps =
                    powf(
                        2.0f,
                        bitDepth
                    );

                // Quantize signal
                processedSample =
                    floorf(
                        processedSample * steps
                    ) / steps;
            }

            // =============================================================
            // 3. MALWARE
            // Randomized Tremolo / Amplitude Modulation
            // =============================================================

            if (malwareAmount > 0.01f)
            {
                // Base frequency: 1Hz to 15Hz
                float baseFreq =
                    juce::jmap(
                        malwareAmount,
                        0.0f,
                        1.0f,
                        1.0f,
                        15.0f
                    );

                // Add random variation
                float tremoloFreq =
                    baseFreq
                    + random.nextFloat()
                    * 10.0f
                    * malwareAmount;

                // Sine wave between 0.0 and 1.0
                float modulator =
                    (std::sin(tremoloPhase) * 0.5f)
                    + 0.5f;

                // Wet / Dry mix
                float dry =
                    processedSample;

                float wet =
                    processedSample * modulator;

                processedSample =
                    dry * (1.0f - malwareAmount)
                    +
                    wet * malwareAmount;

                // Advance phase
                tremoloPhase +=
                    2.0f
                    * juce::MathConstants<float>::pi
                    * tremoloFreq
                    / (float)currentSampleRate;

                if (
                    tremoloPhase >=
                    2.0f
                    * juce::MathConstants<float>::pi
                    )
                {
                    tremoloPhase -=
                        2.0f
                        * juce::MathConstants<float>::pi;
                }
            }

            // =============================================================
            // 4. NEURONIC
            // Randomized Delay / Echo
            // =============================================================

            if (delayInSamples > 0)
            {
                // Calculate read position
                int readPosition =
                    writePosition
                    + sample
                    - delayInSamples;

                // Wrap around circular buffer
                if (readPosition < 0)
                {
                    readPosition +=
                        maxDelayInSamples;
                }

                readPosition %=
                    maxDelayInSamples;

                // Read delayed sample
                float delayedSample =
                    delayData[channel][readPosition];

                // Mix delayed sample with processed sample
                processedSample =
                    processedSample
                    * (1.0f - neuronicAmount * 0.5f)

                    +

                    delayedSample
                    * (neuronicAmount * 0.5f);

                // Feedback
                float sampleToDelay =
                    processedSample
                    + delayedSample * 0.5f;

                int writeIndex =
                    (writePosition + sample)
                    % maxDelayInSamples;

                delayData[channel][writeIndex] =
                    sampleToDelay;
            }

            // =============================================================
            // FINAL OUTPUT
            // =============================================================

            channelData[sample] =
                processedSample;
        }
    }

    // Update write position after processing block
    if (totalNumInputChannels > 0)
    {
        writePosition +=
            buffer.getNumSamples();

        writePosition %=
            maxDelayInSamples;
    }
}

//==============================================================================

bool FileInfectionAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor*
FileInfectionAudioProcessor::createEditor()
{
    return new FileInfectionAudioProcessorEditor(
        *this
    );
}

//==============================================================================

void FileInfectionAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData
)
{
    auto state =
        parameters.copyState();

    std::unique_ptr<juce::XmlElement> xml(
        state.createXml()
    );

    copyXmlToBinary(
        *xml,
        destData
    );
}

void FileInfectionAudioProcessor::setStateInformation(
    const void* data,
    int sizeInBytes
)
{
    std::unique_ptr<juce::XmlElement> xml(
        getXmlFromBinary(
            data,
            sizeInBytes
        )
    );

    if (
        xml
        &&
        xml->hasTagName(
            parameters.state.getType()
        )
        )
    {
        parameters.replaceState(
            juce::ValueTree::fromXml(*xml)
        );
    }
}

//==============================================================================
// This creates new instances of the plugin.

juce::AudioProcessor*
JUCE_CALLTYPE createPluginFilter()
{
    return new FileInfectionAudioProcessor();
}
