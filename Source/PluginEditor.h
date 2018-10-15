/*
 ==============================================================================
 
 ECS730P DAFX 2018
 David Foster 160896851
 ==============================================================================
 */

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
 */
class BassAutoTuneAudioProcessorEditor  : public AudioProcessorEditor, public Slider::Listener, public Timer, public ChangeListener
{
public:
    BassAutoTuneAudioProcessorEditor (BassAutoTuneAudioProcessor&);
    ~BassAutoTuneAudioProcessorEditor();
    
    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    
    //==============================================================================
    
    // UI functions
    void sliderValueChanged (Slider*) override;
    void timerCallback() override;
    void changeListenerCallback(ChangeBroadcaster *source) override;
    
    
private:
    
    // UI Elements
    Label currentNoteNameLabel_, speedLabel_, thresholdLabel_;
    
    Slider speedSlider_, thresholdSlider_, outOfTuneSlider_;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BassAutoTuneAudioProcessor& processor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassAutoTuneAudioProcessorEditor)
};

