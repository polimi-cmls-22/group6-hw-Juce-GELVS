/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FlanGELVSAudioProcessor::FlanGELVSAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    /*Initial setup of the knobs parameters*/

    addParameter(dryWetValue = new juce::AudioParameterFloat("drywet",
        "Dry/Wet",
        0.0,
        1.0,
        0.3));
    addParameter(depthValue = new juce::AudioParameterFloat("depth",
        "Depth",
        0.0,
        1.0,
        0.3));
    addParameter(rateValue = new juce::AudioParameterFloat("rate",
        "Rate",
        0.1f,
        20.0f,
        0.5f));
    addParameter(feedbackValue = new juce::AudioParameterFloat("feedback",
        "Feedback",
        0.01,
        0.99,
        0.2));
    addParameter(phaseOffsetValue = new juce::AudioParameterFloat("phaseoffset",
        "Phase Offset",
        0.0f,
        5.0f,
        0.0f));
    addParameter(ampValue = new juce::AudioParameterFloat("amplitude",
        "Amplitude",
        0.01,
        3.0,
        1.0));

    /*Initial setup of the flanger delay parameters*/
    phaseLFO = 0;
    delayTime = 0;
    feedbackLeft = 0;
    feedbackRight = 0;
    delayTimeInSamples = 0;
    delayReadHead = 0;
    circularBufferLength = 0;
    circularBufferWriteHead = 0;
    circularBufferLeft = nullptr;
    circularBufferRight = nullptr;
}

FlanGELVSAudioProcessor::~FlanGELVSAudioProcessor()
{
}

//==============================================================================
const juce::String FlanGELVSAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FlanGELVSAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FlanGELVSAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FlanGELVSAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FlanGELVSAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FlanGELVSAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FlanGELVSAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FlanGELVSAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FlanGELVSAudioProcessor::getProgramName (int index)
{
    return {};
}

void FlanGELVSAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FlanGELVSAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    /*initialize data for current sample rate, reset Phase and writeHeads*/

    /*Initialize phase*/
    phaseLFO = 0;
    /*Set the circular buffer length*/
    circularBufferLength = sampleRate * MAX_DELAY_TIME;
    /*Initialize left buffer*/
    circularBufferLeft.reset(new double[circularBufferLength]);
    /*Clear junk data in new buffer*/
    juce::zeromem(circularBufferLeft.get(), circularBufferLength * sizeof(double));
    /*Initialize right buffer*/
    circularBufferRight.reset(new double[circularBufferLength]);
    /*Clear junk data in new buffer*/
    juce::zeromem(circularBufferRight.get(), circularBufferLength * sizeof(double));
    /*Initialize the buffer write head to 0*/
    circularBufferWriteHead = 0;
    /*Initialize delayTime to 1*/
    delayTime = 1;
}

void FlanGELVSAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FlanGELVSAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void FlanGELVSAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    /*Obtain left and right audio data pointers*/
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    /*Iterate all samples through the buffer*/
    for (int i = 0; i < buffer.getNumSamples(); i++) {

        /*Generate LFO output*/
        float lfoOut = *ampValue * sin(2 * juce::MathConstants<float>::pi * phaseLFO + *phaseOffsetValue);

        /*Moves LFO phase forwad*/
        /*Set the LFO frequency according to the Rate parameter*/
        phaseLFO += *rateValue / getSampleRate();
        /*In case it's out of range*/
        if (phaseLFO > 1) {
            phaseLFO -= 1;
        }

        /*Control parameter for LFO depth : apply the Depth parameter to the LFO waveform*/
        lfoOut = lfoOut * *depthValue;

        /*Declare the mapped LFOs : map LFO output to desired delay times*/
        float lfoOutMapped = 0;

        /*Define the parameters of the flanger*/
        /*Map LFO to oscillate between 1ms and 5ms*/
        lfoOutMapped = juce::jmap(lfoOut, -1.0f, 1.0f, 0.001f, 0.005f);
        


        /*Calculate the delay time in samples accoring to the LFO*/
        delayTime = delayTime - 0.001 * (delayTime - lfoOutMapped);
        delayTimeInSamples = getSampleRate() * delayTime;

        /*Write into circular buffer : populate the circular buffer with the current write sample plus feedback*/
        circularBufferLeft.get()[circularBufferWriteHead] = leftChannel[i] + feedbackLeft;
        circularBufferRight.get()[circularBufferWriteHead] = rightChannel[i] + feedbackRight;

        /*Calulate read head position*/
        /*Set up the delay read head*/
        delayReadHead = circularBufferWriteHead - delayTimeInSamples;
        /*In case it's not within the buffer range*/
        if (delayReadHead < 0) {
            delayReadHead = circularBufferLength + delayReadHead;
        }

        /*Calculate interpolation points*/
        /*Set up variables for linear interpolation*/
        int readHeadInt_x = (int)delayReadHead;
        int readHeadInt_x1 = readHeadInt_x + 1;
        float readHeadRemainderFloat = delayReadHead - readHeadInt_x;
        /*Wrapping around circular buffer if we are over the length*/
        if (readHeadInt_x >= circularBufferLength) {
            readHeadInt_x -= circularBufferLength;
        }

        /*Perform linear interpolation for delay sample on both left and right output samples*/
        float delay_sample_left = linear_interp(circularBufferLeft.get()[readHeadInt_x], circularBufferLeft.get()[readHeadInt_x1], readHeadRemainderFloat);
        float delay_sample_right = linear_interp(circularBufferRight.get()[readHeadInt_x], circularBufferRight.get()[readHeadInt_x1], readHeadRemainderFloat);

        /*Calculate the feedback according to the Feedback parameter*/
        feedbackLeft = delay_sample_left * (*feedbackValue);
        feedbackRight = delay_sample_right * (*feedbackValue);

        /*Update of the buffer */
        /*Increment the write head circular buffer iterator*/
        circularBufferWriteHead++;
        /*In case it's out of range*/
        if (circularBufferWriteHead >= circularBufferLength) {
            circularBufferWriteHead = 0;
        }

        /*Sum the dryand delayed signalsaccording to the DryWet parameter*/
        buffer.setSample(0, i, buffer.getSample(0, i) * (1 - (*dryWetValue)) + delay_sample_left * (*dryWetValue));
        buffer.setSample(1, i, buffer.getSample(1, i) * (1 - (*dryWetValue)) + delay_sample_right * (*dryWetValue));

    }
}

//==============================================================================
bool FlanGELVSAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FlanGELVSAudioProcessor::createEditor()
{
    return new FlanGELVSAudioProcessorEditor (*this);
}

//==============================================================================
void FlanGELVSAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FlanGELVSAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FlanGELVSAudioProcessor();
}
