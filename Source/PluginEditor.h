/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class FlanGELVSAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FlanGELVSAudioProcessorEditor (FlanGELVSAudioProcessor&);
    ~FlanGELVSAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.

    FlanGELVSAudioProcessor& audioProcessor;

    /*  Creation of sliders and labels*/
    juce::Slider    dryWetSlider, depthSlider, rateSlider, feedbackSlider, ampSlider, phaseOffsetSlider;
    juce::Label     dryWetLabel, depthLabel, rateLabel, feedbackLabel, ampLabel, phaseOffsetLabel;
    juce::Image     backgroundImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlanGELVSAudioProcessorEditor)
};
