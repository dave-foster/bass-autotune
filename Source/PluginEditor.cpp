/*
 ==============================================================================
 
 ECS730P DAFX 2018
 David Foster 160896851
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BassAutoTuneAudioProcessorEditor::BassAutoTuneAudioProcessorEditor (BassAutoTuneAudioProcessor& p)
: AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    // Set up the UI Elements
    
    // Speed slider
    addAndMakeVisible(&speedSlider_);
    speedSlider_.setSliderStyle(Slider::Rotary);
    speedSlider_.addListener(this);
    speedSlider_.setRange(0, 500, 1.0);
    speedSlider_.setTextValueSuffix(" ms");
    speedSlider_.setSkewFactorFromMidPoint(100);
    
    speedSlider_.setTextBoxStyle(Slider::TextBoxBelow, true, 50, 20);
    speedSlider_.setValue(processor.getParameter(BassAutoTuneAudioProcessor::kAutoTuneSpeed));
    speedLabel_.setText("Speed", dontSendNotification);
    speedLabel_.setJustificationType(Justification::centred);
    speedLabel_.attachToComponent(&speedSlider_, false);
    
    // Threshold slider
    
    addAndMakeVisible(&thresholdSlider_);
    thresholdSlider_.setSliderStyle(Slider::Rotary);
    thresholdSlider_.addListener(this);
    thresholdSlider_.setRange(-40.0, 0.0, 0.1);
    thresholdSlider_.setTextValueSuffix(" dB");
    
    thresholdSlider_.setTextBoxStyle(Slider::TextBoxBelow, true, 50, 20);
    thresholdSlider_.setValue(processor.getParameter(BassAutoTuneAudioProcessor::kAutoTuneThreshold));
    thresholdLabel_.setText("Threshold", dontSendNotification);
    thresholdLabel_.setJustificationType(Justification::centred);
    thresholdLabel_.attachToComponent(&thresholdSlider_, false);
    
    // Tuning slider
    
    addAndMakeVisible(&outOfTuneSlider_);
    outOfTuneSlider_.setSliderStyle(Slider::LinearHorizontal);
    outOfTuneSlider_.addListener(this);
    outOfTuneSlider_.setRange(-50, 50, 0.1);
    //outOfTuneSlider_.setTextValueSuffix(" dB");
    
    //outOfTuneSlider_.hideTextBox(false);
    //outOfTuneSlider_.setTextBoxStyle(Slider::NoTextBox, true, 10, 10);
outOfTuneSlider_.setTextBoxStyle(Slider::TextBoxAbove, true, 50, 20);
    outOfTuneSlider_.setTextValueSuffix("%");
    outOfTuneSlider_.setValue(0.0);
    
    // Note Name Label
    
    addAndMakeVisible(&currentNoteNameLabel_);
    currentNoteNameLabel_.setText("", dontSendNotification);
    currentNoteNameLabel_.setFont(90.0);
    currentNoteNameLabel_.setJustificationType(Justification::centred);
    
    // Start Timer
    startTimer(1000);
    
    // Listener
    processor.removeAllChangeListeners();
    processor.addChangeListener(this);
}

BassAutoTuneAudioProcessorEditor::~BassAutoTuneAudioProcessorEditor()
{
    processor.removeAllChangeListeners();
}

//==============================================================================
void BassAutoTuneAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    
    g.setColour (Colours::white);
    //g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
}

void BassAutoTuneAudioProcessorEditor::resized()
{
    speedSlider_.setBounds(0, 50, 150, 150);
    thresholdSlider_.setBounds(250, 50, 150, 150);
    
    outOfTuneSlider_.setBounds(50, 200, 300, 100);
    
    currentNoteNameLabel_.setBounds(150, 60, 100, 100);
}

void BassAutoTuneAudioProcessorEditor::timerCallback()
{
    speedSlider_.setValue(processor.autoTuneSpeed_, dontSendNotification);
    thresholdSlider_.setValue(processor.autoTuneThreshold_, dontSendNotification);
    
    //String message;
    //message << "SCM Timer Callback = " << newLine;
    //Logger::getCurrentLogger()->writeToLog (message);
    
}

void BassAutoTuneAudioProcessorEditor::changeListenerCallback(ChangeBroadcaster *source)
{
    // TODO: This still occasionally crashes when window opened and closed
    if (outOfTuneSlider_.isShowing())
    {
        outOfTuneSlider_.setValue(processor.currentOutputTuning_);
        
    }
    if (currentNoteNameLabel_.isShowing())
    {
        currentNoteNameLabel_.setText(processor.currentNoteName_, dontSendNotification);
    }
        
    
}

// This is our Slider::Listener callback, when the user drags a slider
void BassAutoTuneAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    if (slider == &speedSlider_)
    {
        processor.setParameterNotifyingHost(BassAutoTuneAudioProcessor::kAutoTuneSpeed, (float)speedSlider_.getValue());
    }
    else if (slider == &thresholdSlider_)
    {
        processor.setParameterNotifyingHost(BassAutoTuneAudioProcessor::kAutoTuneThreshold, (float)thresholdSlider_.getValue());
    }
    else if (slider == &outOfTuneSlider_)
    {
        outOfTuneSlider_.setValue(processor.currentOutputTuning_);
    }
}

