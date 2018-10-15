/*
 ==============================================================================
 
 ECS730P DAFX 2018
 David Foster 160896851
 ==============================================================================
 */

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "YINPitchDetector.h"
#include "FreqPitch.h"

#define BUFFER_SIZE 16384

//==============================================================================
/**
 */
class BassAutoTuneAudioProcessor  : public AudioProcessor, public ChangeBroadcaster
{
public:
    //==============================================================================
    BassAutoTuneAudioProcessor();
    ~BassAutoTuneAudioProcessor();
    
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif
    
    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    
    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    
    //==============================================================================
    const String getName() const override;
    
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    
    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;
    
    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    //==============================================================================
    int getNumParameters() override;
    
    float getParameter(int index) override;
    void setParameter(int index, float newValue) override;
    
    const String getParameterName(int index) override;
    
    enum Parameters
    {
        kAutoTuneSpeed = 0,
        kAutoTuneThreshold,
        kNumParameters
    };
    
    // Adjustable parameters:
    float autoTuneSpeed_; // The speed at which AutoTune is applied
    float autoTuneThreshold_; // The threshold over which AutoTune is applied
    
    // UI Label
    String currentNoteName_;
    
    // AutoTune Variable
    float currentOutputTuning_; // The current tuning (-0.5 to 0.5) output
    
    
    
    
    
    //==============================================================================
    
    // FFT
    enum
    {
        fftOrder = 20,
        fftSize = 1 << fftOrder
    };

    
private:
    // YIN Pitch Detector
    YINPitchDetector pitchDetector_;
    AudioSampleBuffer yinBuffer_;
    float yinThreshold_;
    float yinConfidenceThreshold_;
    int yinBufferSize_;
    int yinBufferHop_;
    
    // Pitch / Frequency Helper
    FreqPitch freqPitch_;
    
    // Circular Buffers
    AudioSampleBuffer inputBuffer_;
    int inputBufferLength_;
    int inputBufferWritePosition_;
    int inputBufferReadPosition_;
    
    AudioSampleBuffer outputBuffer_;
    int outputBufferLength_;
    int outputBufferReadPosition_, outputBufferWritePosition_;
    
    int samplesSinceLastBlockProcessed_;
    
    // Pitch shifting adjustment variables
    int autoTuneSamples_; //
    int crossFadeSamples_;
    int pauseSamples_;
    int pitchAdjustPosition_;
    float pitchAdjustStartValue_;
    int pitchShiftHopSize_;
    int hopDiff_;
    
    float ratio_; // The derived ratio for pitch shifting
    int oldPeriodLength_;
    
    // Variables to control cross-fading pitch shifted buffer into real time audio
    bool fadeIn_, fadeOut_, paused_, newNoteIdentified_;
    int fadePos_;
    
    // Value for adjust pitch shifted output after cross-fade
    float tunedAttenuation_;
    
    //
    
    
    // Helper methods
    
    void updatecurrentOutputTuning_(float newValue);
    void updatecurrentNoteName_(String newValue);
    
    void analysePitch(float *inBuffer, int pointer);
    void calculateShift();
    void pitchShiftBuffer();
    void findAmpDifference();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassAutoTuneAudioProcessor)
};

