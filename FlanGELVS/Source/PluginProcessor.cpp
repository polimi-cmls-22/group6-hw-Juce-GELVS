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
    /*  Initial setup of the knobs parameters   */

    addParameter(dryWetValue = new juce::AudioParameterFloat("drywet",
        "Dry/Wet",
        0.0,
        1.0,
        0.5));
    addParameter(depthValue = new juce::AudioParameterFloat("depth",
        "Depth",
        0.01f,
        1.0f,
        0.5f));
    addParameter(rateValue = new juce::AudioParameterFloat("rate",
        "Rate",
        0.1f,
        20.0f,
        0.5f));
    addParameter(feedbackValue = new juce::AudioParameterFloat("feedback",
        "Feedback",
        0.01f,
        0.99f,
        0.5f));
    addParameter(phaseOffsetValue = new juce::AudioParameterFloat("phaseoffset",
        "Phase Offset",
        0.0f,
        1.0f,
        0.0f));
    addParameter(ampValue = new juce::AudioParameterFloat("amplitude",
        "Amplitude",
        0.01,
        3.0,
        1.0));

    /*  Initial setup of the flanger delay parameters  */
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

    /*initialize phase*/
    phaseLFO = 0;
    /*calculate circular buffer length*/
    circularBufferLength = sampleRate * MAX_DELAY_TIME;
    /*initialize left buffer*/
    circularBufferLeft.reset(new double[circularBufferLength]);
    /*clear junk data in new buffer*/
    juce::zeromem(circularBufferLeft.get(), circularBufferLength * sizeof(double));
    /*initialize right buffer*/
    circularBufferRight.reset(new double[circularBufferLength]);
    /*clear junk data in new buffer*/
    juce::zeromem(circularBufferRight.get(), circularBufferLength * sizeof(double));
    /*initialize writeHear to 0*/
    circularBufferWriteHead = 0;
    /*initialize delayTime to 1*/
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

    /*obtain left and right audio data pointers*/
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    /*iterate all samples through the buffer*/
    for (int i = 0; i < buffer.getNumSamples(); i++) {

        /*generate LFO output*/
        float lfoOut = *ampValue * sin(2 * juce::MathConstants<float>::pi * phaseLFO);

        /*moves LFO phase forwad*/
        phaseLFO += *rateValue / getSampleRate();
        if (phaseLFO > 1) {
            phaseLFO -= 1;
        }

        /* control parameter for LFO depth*/
        lfoOut = lfoOut * *depthValue;

        /* map LFO output to desired delay times*/
        float lfoOutMapped = 0;

        /* define the parameters of the flanger */
        lfoOutMapped = juce::jmap(lfoOut, -1.0f, 1.0f, 0.001f, 0.005f);


        /* calculate delay times in samples*/
        delayTime = delayTime - 0.001 * (delayTime - lfoOutMapped);
        delayTimeInSamples = getSampleRate() * delayTime;

        /*write into circular buffer*/
        circularBufferLeft.get()[circularBufferWriteHead] = leftChannel[i] + feedbackLeft;
        circularBufferRight.get()[circularBufferWriteHead] = rightChannel[i] + feedbackRight;

        /* calulate read head position*/
        delayReadHead = circularBufferWriteHead - delayTimeInSamples;

        if (delayReadHead < 0) {
            delayReadHead = circularBufferLength + delayReadHead;
        }

        /* calculate interpolation points */
        int readHeadInt_x = (int)delayReadHead;
        int readHeadInt_x1 = readHeadInt_x + 1;
        float readHeadRemainderFloat = delayReadHead - readHeadInt_x;
        if (readHeadInt_x >= circularBufferLength) {
            readHeadInt_x -= circularBufferLength; //Wrapping around circular buffer if we are over the length
        }

        /* generate left and right output samples*/
        float delay_sample_left = linear_interp(circularBufferLeft.get()[readHeadInt_x], circularBufferLeft.get()[readHeadInt_x1], readHeadRemainderFloat);
        float delay_sample_right = linear_interp(circularBufferRight.get()[readHeadInt_x], circularBufferRight.get()[readHeadInt_x1], readHeadRemainderFloat);

        feedbackLeft = delay_sample_left * (*feedbackValue);
        feedbackRight = delay_sample_right * (*feedbackValue);

        /* update of the buffer */
        circularBufferWriteHead++;

        if (circularBufferWriteHead >= circularBufferLength) {
            circularBufferWriteHead = 0;
        }

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
