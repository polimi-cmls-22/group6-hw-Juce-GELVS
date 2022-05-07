/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FlanGELVSAudioProcessorEditor::FlanGELVSAudioProcessorEditor (FlanGELVSAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // BACKGROUND 
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
    backgroundImage = backgroundImage.rescaled(500, 400);
    //

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setSize(500,400);

    auto& params = processor.getParameters();

    /* setting the colour of the all the sliders */
    getLookAndFeel().setColour(juce::Slider::thumbColourId, juce::Colours::darkblue);                               //Pointer colour
    getLookAndFeel().setColour(juce::Slider::rotarySliderFillColourId, juce::Colour::fromRGBA(233,231,119,255));    //SliderFill colour
    getLookAndFeel().setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::lightcyan);                //SliderOutline colour

    /* setting the custom and parameters of the each slider */

    /*=====================================================================================================*/
    juce::AudioParameterFloat* dryWetParameter = (juce::AudioParameterFloat*)params.getUnchecked(0);

    dryWetSlider.setBounds(0, 100, 120, 120);
    dryWetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalDrag);
    dryWetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 25);
    dryWetSlider.setColour(dryWetSlider.textBoxOutlineColourId, juce::Colour());
    dryWetSlider.setRange(dryWetParameter->range.start, dryWetParameter->range.end, 0.01);
    dryWetSlider.setValue(*dryWetParameter);
    addAndMakeVisible(dryWetSlider);

    dryWetLabel.setText("Dry/Wet", juce::dontSendNotification);
    dryWetLabel.attachToComponent(&dryWetSlider, false);
    dryWetLabel.setJustificationType(juce::Justification::centredTop);
    addAndMakeVisible(dryWetLabel);

    dryWetSlider.onValueChange = [this, dryWetParameter] { *dryWetParameter = dryWetSlider.getValue();};
    dryWetSlider.onDragStart = [dryWetParameter] { dryWetParameter->beginChangeGesture(); };
    dryWetSlider.onDragEnd = [dryWetParameter] { dryWetParameter->endChangeGesture(); };


    /*=====================================================================================================*/
    juce::AudioParameterFloat* depthParameter = (juce::AudioParameterFloat*)params.getUnchecked(1);

    depthSlider.setBounds(200, 100, 120, 120);
    depthSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalDrag);
    depthSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 25);
    depthSlider.setColour(depthSlider.textBoxOutlineColourId, juce::Colour());
    depthSlider.setRange(depthParameter->range.start, depthParameter->range.end, 0.01);
    depthSlider.setValue(*depthParameter);
    addAndMakeVisible(depthSlider);

    depthLabel.setText("Depth", juce::dontSendNotification);
    depthLabel.attachToComponent(&depthSlider, false);
    depthLabel.setJustificationType(juce::Justification::centredTop);
    addAndMakeVisible(depthLabel);

    depthSlider.onValueChange = [this, depthParameter] { *depthParameter = depthSlider.getValue();};
    depthSlider.onDragStart = [depthParameter] { depthParameter->beginChangeGesture(); };
    depthSlider.onDragEnd = [depthParameter] { depthParameter->endChangeGesture(); };

    /*=====================================================================================================*/
    juce::AudioParameterFloat* rateParameter = (juce::AudioParameterFloat*)params.getUnchecked(2);

    rateSlider.setBounds(380, 100, 120, 120);
    rateSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalDrag);
    rateSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 25);
    rateSlider.setColour(rateSlider.textBoxOutlineColourId, juce::Colour());
    rateSlider.setRange(rateParameter->range.start, rateParameter->range.end, 0.01);
    rateSlider.setValue(*rateParameter);
    addAndMakeVisible(rateSlider);

    rateLabel.setText("Rate", juce::dontSendNotification);
    rateLabel.attachToComponent(&rateSlider, false);
    rateLabel.setJustificationType(juce::Justification::centredTop);
    addAndMakeVisible(rateLabel);

    rateSlider.onValueChange = [this, rateParameter] { *rateParameter = rateSlider.getValue();};
    rateSlider.onDragStart = [rateParameter] { rateParameter->beginChangeGesture(); };
    rateSlider.onDragEnd = [rateParameter] { rateParameter->endChangeGesture(); };

    /*=====================================================================================================*/
    juce::AudioParameterFloat* feedbackParameter = (juce::AudioParameterFloat*)params.getUnchecked(3);

    feedbackSlider.setBounds(0, 250, 120, 120);
    feedbackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalDrag);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 25);
    feedbackSlider.setColour(feedbackSlider.textBoxOutlineColourId, juce::Colour());
    feedbackSlider.setRange(feedbackParameter->range.start, feedbackParameter->range.end, 0.01);
    feedbackSlider.setValue(*feedbackParameter);
    addAndMakeVisible(feedbackSlider);

    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.attachToComponent(&feedbackSlider, false);
    feedbackLabel.setJustificationType(juce::Justification::centredTop);
    addAndMakeVisible(feedbackLabel);

    feedbackSlider.onValueChange = [this, feedbackParameter] { *feedbackParameter = feedbackSlider.getValue(); };
    feedbackSlider.onDragStart = [feedbackParameter] { feedbackParameter->beginChangeGesture(); };
    feedbackSlider.onDragEnd = [feedbackParameter] { feedbackParameter->endChangeGesture(); };

    /*=====================================================================================================*/
    juce::AudioParameterFloat* phaseParameter = (juce::AudioParameterFloat*)params.getUnchecked(4);

    phaseOffsetSlider.setBounds(250, 250, 120, 120);
    phaseOffsetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalDrag);
    phaseOffsetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 25);
    phaseOffsetSlider.setColour(phaseOffsetSlider.textBoxOutlineColourId, juce::Colour());
    phaseOffsetSlider.setRange(phaseParameter->range.start, phaseParameter->range.end, 0.01);
    phaseOffsetSlider.setValue(*phaseParameter);
    //addAndMakeVisible(phaseOffsetSlider);

    phaseOffsetLabel.setText("PhaseOffset", juce::dontSendNotification);
    phaseOffsetLabel.attachToComponent(&phaseOffsetSlider, false);
    phaseOffsetLabel.setJustificationType(juce::Justification::centredTop);
    //addAndMakeVisible(phaseOffsetLabel);

    phaseOffsetSlider.onValueChange = [this, phaseParameter] { *phaseParameter = phaseOffsetSlider.getValue();};
    phaseOffsetSlider.onDragStart = [phaseParameter] { phaseParameter->beginChangeGesture(); };
    phaseOffsetSlider.onDragEnd = [phaseParameter] { phaseParameter->endChangeGesture(); };

    /*=====================================================================================================*/
    juce::AudioParameterFloat* delayParameter = (juce::AudioParameterFloat*)params.getUnchecked(5);

    ampSlider.setBounds(380, 250, 120, 120);
    ampSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalDrag);
    ampSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 50, 25);
    ampSlider.setColour(ampSlider.textBoxOutlineColourId, juce::Colour());
    ampSlider.setRange(delayParameter->range.start, delayParameter->range.end, 0.01);
    ampSlider.setValue(*delayParameter);
    addAndMakeVisible(ampSlider);

    ampLabel.setText("Amplitude", juce::dontSendNotification);
    ampLabel.attachToComponent(&ampSlider, false);
    ampLabel.setJustificationType(juce::Justification::centredTop);
    addAndMakeVisible(ampLabel);

    ampSlider.onValueChange = [this, delayParameter] { *delayParameter = ampSlider.getValue();};
    ampSlider.onDragStart = [delayParameter] { delayParameter->beginChangeGesture(); };
    ampSlider.onDragEnd = [delayParameter] { delayParameter->endChangeGesture(); };

    /*=====================================================================================================*/

}

FlanGELVSAudioProcessorEditor::~FlanGELVSAudioProcessorEditor()
{
}

//==============================================================================
void FlanGELVSAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour) 
    g.fillAll(juce::Colours::darkorange);

    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawImageAt(backgroundImage, 0, 0);

    g.setFont(juce::Font("Arial", 25.0f, juce::Font::bold));
    //g.drawFittedText("--FLANGELVS (Flanger effect)--", getLocalBounds(), juce::Justification::centredTop, 1);
}

void FlanGELVSAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
