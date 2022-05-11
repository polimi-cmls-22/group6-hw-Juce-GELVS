/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#define MAX_DELAY_TIME 2

//==============================================================================
/**
*/
class FlanGELVSAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    FlanGELVSAudioProcessor();
    ~FlanGELVSAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    /*===============================================================================*/
    /* Function: Linear Interpolation  */
    /* Calculate the interpolated sample at position 'inPhase' between two samples 'x' and 'x1 */

    float linear_interp(float sample_x, float sample_x1, float inPhase) {
        return (1 - inPhase) * sample_x + inPhase * sample_x1;
    }
    /*===============================================================================*/

private:
    float phaseLFO;
    float delayTime;
    juce::AudioParameterFloat* dryWetValue;
    juce::AudioParameterFloat* depthValue;
    juce::AudioParameterFloat* rateValue;
    juce::AudioParameterFloat* phaseOffsetValue;
    juce::AudioParameterFloat* feedbackValue;
    juce::AudioParameterFloat* ampValue;
    float dryWet;
    float feedbackLeft;
    float feedbackRight;
    float delayTimeInSamples;
    float delayReadHead;
    int circularBufferLength;
    int circularBufferWriteHead;
    std::unique_ptr<double> circularBufferLeft;
    std::unique_ptr<double> circularBufferRight;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlanGELVSAudioProcessor)
};
