#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Helper function to define the parameters
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("glitch", "Glitch", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("corruption", "Corruption", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("malware", "Malware", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("neuronic", "Neuronic", 0.0f, 1.0f, 0.5f));

    return { params.begin(), params.end() };
}

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
#endif
    parameters(*this, nullptr, "PARAMS", createParameterLayout())
{
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName(int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // --- SETUP CUSTOM DELAY BUFFER FOR 'NEURONIC' ---
    // Max delay time of 0.1 seconds (100ms)
    float maxDelayTimeSeconds = 0.1f;
    maxDelayInSamples = (int)(maxDelayTimeSeconds * currentSampleRate);

    // Allocate the buffer for the max delay time and number of channels
    delayBuffer.setSize(getTotalNumOutputChannels(), maxDelayInSamples);
    delayBuffer.clear();
    writePosition = 0;

    // Reset state variables
    sampleAccumulator[0] = sampleAccumulator[1] = 0.0f;
    skipCounter[0] = skipCounter[1] = 0;
    tremoloPhase = 0.0f;
}

void NewProjectAudioProcessor::releaseResources()
{
    // Free up the delay buffer memory
    delayBuffer.setSize(0, 0);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsMidiEffect
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
}
#endif

void NewProjectAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that didn't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // --- GET PARAMETER VALUES (0.0 to 1.0) ---
    auto glitchAmount = parameters.getRawParameterValue("glitch")->load();
    auto corruptionAmount = parameters.getRawParameterValue("corruption")->load();
    auto malwareAmount = parameters.getRawParameterValue("malware")->load();
    auto neuronicAmount = parameters.getRawParameterValue("neuronic")->load();

    // --- Prepare Delay Settings for Neuronic (if active) ---
    int delayInSamples = 0;
    if (neuronicAmount > 0.01f)
    {
        // Calculate delay from 1 sample up to maxDelayInSamples (0.1s)
        delayInSamples = (int)juce::jmap(neuronicAmount, 0.0f, 1.0f, 1.0f, (float)maxDelayInSamples);
        delayInSamples = juce::jmax(1, delayInSamples);
    }
    auto* delayData = delayBuffer.getArrayOfWritePointers();


    // --- PER-SAMPLE PROCESSING LOOP ---
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float inputSample = channelData[sample];
            float processedSample = inputSample;

            // =============================================================
            // 1. GLITCH EFFECT (Sample-Rate Reduction / Decimator)
            // =============================================================
            if (glitchAmount > 0.01f)
            {
                int maxSkipFactor = (int)juce::jmap(glitchAmount, 0.0f, 1.0f, 1.0f, 100.0f);

                if (skipCounter[channel] <= 0)
                {
                    sampleAccumulator[channel] = processedSample;
                    skipCounter[channel] = random.nextInt(maxSkipFactor) + 1;
                }

                processedSample = sampleAccumulator[channel];
                skipCounter[channel]--;
            }

            // =============================================================
            // 2. CORRUPTION (Bit Crusher)
            // =============================================================
            if (corruptionAmount > 0.01f)
            {
                // Map knob (0.0 to 1.0) to effective bit depth (16 bits down to 2 bits)
                float bitDepth = juce::jmap(corruptionAmount, 0.0f, 1.0f, 16.0f, 2.0f);
                float steps = powf(2.0f, bitDepth);

                // Quantize the signal: floorf(sample * steps) / steps
                processedSample = floorf(processedSample * steps) / steps;
            }

            // =============================================================
            // 3. MALWARE (Randomized Tremolo/Amplitude Modulation)
            // =============================================================
            if (malwareAmount > 0.01f)
            {
                // Base Frequency: 1Hz to 15Hz
                float baseFreq = juce::jmap(malwareAmount, 0.0f, 1.0f, 1.0f, 15.0f);

                // Add random variation
                float tremoloFreq = baseFreq + random.nextFloat() * 10.0f * malwareAmount;

                // Modulator (Sine wave between 0.0 and 1.0)
                float modulator = (std::sin(tremoloPhase) * 0.5f) + 0.5f;

                // Wet/Dry Mix
                float dry = processedSample;
                float wet = processedSample * modulator;

                processedSample = dry * (1.0f - malwareAmount) + wet * malwareAmount;

                // Advance phase
                tremoloPhase += 2.0f * juce::MathConstants<float>::pi * tremoloFreq / (float)currentSampleRate;
                if (tremoloPhase >= 2.0f * juce::MathConstants<float>::pi)
                    tremoloPhase -= 2.0f * juce::MathConstants<float>::pi;
            }

            // =============================================================
            // 4. NEURONIC (Randomized Delay/Echo)
            // =============================================================
            if (delayInSamples > 0)
            {
                // Calculate the read position (circular buffer logic)
                int readPosition = writePosition + sample - delayInSamples;

                // Wrap around the buffer
                if (readPosition < 0)
                    readPosition += maxDelayInSamples;
                readPosition %= maxDelayInSamples;

                // 1. Read the delayed sample
                float delayedSample = delayData[channel][readPosition];

                // 2. Mix the delayed sample with the processed sample (0.5 mix level max)
                processedSample = processedSample * (1.0f - neuronicAmount * 0.5f) +
                    delayedSample * (neuronicAmount * 0.5f);

                // 3. Write the sample (with feedback: 0.5f) to the delay buffer
                float sampleToDelay = processedSample + delayedSample * 0.5f;
                int writeIndex = (writePosition + sample) % maxDelayInSamples;
                delayData[channel][writeIndex] = sampleToDelay;
            }

            // =============================================================
            // FINAL OUTPUT
            // =============================================================
            channelData[sample] = processedSample;
        }
    }

    // --- Update write position AFTER the loop (for Neuronic) ---
    if (totalNumInputChannels > 0)
    {
        writePosition += buffer.getNumSamples();
        writePosition %= maxDelayInSamples; // Wrap around (circular buffer)
    }
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor(*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void NewProjectAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
