/*
 ==============================================================================
 Dave Foster Swing City Music
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BassAutoTuneAudioProcessor::BassAutoTuneAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  AudioChannelSet::stereo(), true)
#endif
                  .withOutput ("Output", AudioChannelSet::stereo(), true)
#endif
                  )
#endif
{
    // Set default values
    autoTuneSpeed_ = 95;
    autoTuneThreshold_ = -13.0;
    
    currentOutputTuning_ = 0.0;
    
    inputBufferLength_ = 1;
    outputBufferLength_ = 1;
    
    samplesSinceLastBlockProcessed_ = 0;
    
    // YIN Settings
    
    yinThreshold_ = 0.6;
    yinConfidenceThreshold_ = 0.1;
    yinBufferSize_ = 2048;
    yinBufferHop_ = 1024;
    
    inputBufferWritePosition_ = 0;
    inputBufferReadPosition_ = 0;
    outputBufferReadPosition_ = 0;
    outputBufferWritePosition_ = yinBufferSize_;
    
    // Pitch settings
    newNoteIdentified_ = false;
    
    pitchAdjustPosition_ = 0;
    pitchAdjustStartValue_ = 0.0;
    ratio_ = 1.0;
    oldPeriodLength_ = 512;
    pitchShiftHopSize_ = 512; // This varies per frame depending on the wave length
    hopDiff_ = 0; // how far ahead the input pointer is off the output pointer
    
    fadeIn_ = false;
    fadeOut_ = false;
    fadePos_ = 0;
    
    pauseSamples_ = 0;
    crossFadeSamples_ = 512;
    
    tunedAttenuation_ = 1.0;
}

BassAutoTuneAudioProcessor::~BassAutoTuneAudioProcessor()
{
}

//==============================================================================
const String BassAutoTuneAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BassAutoTuneAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool BassAutoTuneAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool BassAutoTuneAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double BassAutoTuneAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BassAutoTuneAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int BassAutoTuneAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BassAutoTuneAudioProcessor::setCurrentProgram (int index)
{
}

const String BassAutoTuneAudioProcessor::getProgramName (int index)
{
    return {};
}

void BassAutoTuneAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================

// Getting and setting parameter values

int BassAutoTuneAudioProcessor::getNumParameters()
{
    return kNumParameters;
}

float BassAutoTuneAudioProcessor::getParameter(int index)
{
    switch (index)
    {
        case kAutoTuneSpeed:        return autoTuneSpeed_;
        case kAutoTuneThreshold:    return autoTuneThreshold_;
        default:                    return 0.0f;
    }
}

const String BassAutoTuneAudioProcessor::getParameterName(int index)
{
    switch (index)
    {
        case kAutoTuneSpeed:        return "AutoTune Speed";
        case kAutoTuneThreshold:    return "AutoTune Threshold";
        default:                break;
    }
    return String::empty;
}

void BassAutoTuneAudioProcessor::setParameter(int index, float newValue)
{
    switch (index)
    {
        case kAutoTuneSpeed:
        {
            autoTuneSpeed_ = newValue;
            autoTuneSamples_ = round((newValue / 1000) * getSampleRate());
            pauseSamples_ = autoTuneSamples_ - crossFadeSamples_;
        }
        case kAutoTuneThreshold:
        {
            autoTuneThreshold_ = newValue;
            break;
        }
        default:
            break;
    }
}

void BassAutoTuneAudioProcessor::updatecurrentOutputTuning_(float newValue)
{
    currentOutputTuning_ = newValue;
    sendChangeMessage();
}

void BassAutoTuneAudioProcessor::updatecurrentNoteName_(juce::String newValue)
{
    currentNoteName_ = newValue;
    sendChangeMessage();
}


//==============================================================================
void BassAutoTuneAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialise buffers
    
    inputBufferLength_ = BUFFER_SIZE;
    inputBuffer_.setSize(1, inputBufferLength_);
    inputBuffer_.clear();
    inputBufferReadPosition_ = 0;
    inputBufferWritePosition_ = 0;
    samplesSinceLastBlockProcessed_ = 0;
    
    outputBufferLength_ = BUFFER_SIZE;
    outputBuffer_.setSize(1, outputBufferLength_);
    outputBuffer_.clear();
    outputBufferReadPosition_ = 0;
    outputBufferWritePosition_ = yinBufferSize_ / 2;
    //outputBufferWritePosition_ = 0;
    
    yinBuffer_.setSize(1, yinBufferSize_);
    yinBuffer_.clear();
    
    // Set up helper objects
    pitchDetector_ = YINPitchDetector(getSampleRate(), yinBufferSize_, yinThreshold_);
    freqPitch_ = FreqPitch(0.0);
    
    // Check this value is using the correct fs!
    autoTuneSamples_ = (autoTuneSpeed_ / 1000) * getSampleRate();
    pauseSamples_ = autoTuneSamples_ - crossFadeSamples_;
    
    oldPeriodLength_ = 453;
    pitchShiftHopSize_ = 453;
    
}

void BassAutoTuneAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BassAutoTuneAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

// Helpers

void BassAutoTuneAudioProcessor::analysePitch(float *inBuffer, int pointer)
{
    // Fill the buffer, and get the maximum value
    float maxInputValue = 0.0;
    int x = pointer;
    float* pitchData = yinBuffer_.getWritePointer(jmin (0, yinBuffer_.getNumChannels() - 1));
    for (int n = 0; n < yinBufferSize_; n++)
    {
        pitchData[n] = inBuffer[x];
        if (fabs(inBuffer[x]) > maxInputValue)
            maxInputValue = fabs(inBuffer[x]);
        x++;
        if (x >= BUFFER_SIZE)
            x = 0;
    }
    
    // Convert to dB
    float maxInputValueDB = 10 * log10(maxInputValue);
    // If above the threshold, find the pitch
    if (maxInputValueDB > autoTuneThreshold_)
    {
        float yinPitch = pitchDetector_.calculatePitch(pitchData);
        float yinConfidence = pitchDetector_.confidenceYIN_;
        if (yinConfidence < yinConfidenceThreshold_)
        {
            freqPitch_.setNewFreq(yinPitch);
            
            // Update GUI
            updatecurrentNoteName_(freqPitch_.currentPitchName);
            updatecurrentOutputTuning_(freqPitch_.currentMIDIError);
            
            if (freqPitch_.currentPitchClassUpdated)
                newNoteIdentified_ = true;
            
            
            
        }
        else
        {
            // YIN Confidence too low (but above amplitude threshold)
            // Assume that it is the same as the previous
            newNoteIdentified_ = false;
        }
    }
    else
    {
        // Amplitude below threshold
        freqPitch_.setNewFreq(0.0);
        if (currentNoteName_ != "")
        {
            updatecurrentNoteName_("");
            updatecurrentOutputTuning_(0.0);
        }
        if (freqPitch_.currentPitchClassUpdated)
            newNoteIdentified_ = true;
    }
    
}

void BassAutoTuneAudioProcessor::calculateShift()
{
    
    if (freqPitch_.currentMIDIPitchClass > -1)
    {
        float actualPitch = freqPitch_.currentFreq;
        float targetPitch = freqPitch_.currentFreqInTune;
        oldPeriodLength_ = (int)round((float)getSampleRate() / actualPitch);
        if (newNoteIdentified_)
        {
            // Reset the output buffer
            outputBufferWritePosition_ = yinBufferSize_ / 2;
            outputBufferReadPosition_ = 0;
            outputBuffer_.clear();
            
            pitchAdjustStartValue_ = freqPitch_.currentPitchDev;
            pitchAdjustPosition_ = 0;
            
            // Trigger the start of the cross fade
            paused_ = true;
            fadeIn_ = false;
            fadePos_ = 0;
            
            newNoteIdentified_ = false;
        }
        
        float thisAdjust = 0.0f - freqPitch_.currentPitchDev;
        float thisTargetPitch = targetPitch + thisAdjust;
        ratio_ = thisTargetPitch / targetPitch;
        
        // Gliss up / down to the correct pitch over the designated time frame
        // If more time has passed than the threshold, then it is just the diff between actual and desired pitch
        // If the count has just been reset, then we leave the pitch as it is
        // This code left out as it produced audio glitches!
        /*
        if (pitchAdjustPosition_ == 0)
        {
            thisAdjust = 0.0;
            ratio_ = 1.0;
        }
        else
        {
            // If it is within the threshold time, then it is on the gradient between the first detected error and the desired pitch
            if (pitchAdjustPosition_ < autoTuneSamples_)
            {
                float positionRatio = 1.0f - (float(pitchAdjustPosition_) / float(autoTuneSamples_));
                float desiredError = pitchAdjustStartValue_ * positionRatio;
                thisAdjust += desiredError;
            }
            float thisTargetPitch = targetPitch + thisAdjust;
            ratio_ = thisTargetPitch / targetPitch;
        }
         */
    }
    else
    {
        // No pitch identied
        if (newNoteIdentified_)
        {
            // Trigger the start of the cross fade
            fadeOut_ = true;
            fadePos_ = 0;
            newNoteIdentified_ = false;
            ratio_ = 1.0;
        }
    }
}

void BassAutoTuneAudioProcessor::pitchShiftBuffer()
{
    pitchAdjustPosition_ += oldPeriodLength_;
    
    // For testing
    //ratio_ = 0.98;
    
    // Calculate new period length
    int newPeriodLength = (int)(floor((float)oldPeriodLength_ / ratio_));
    
    // Overlap-add two periods of the waveform into the output buffer (with Hann window)
    int writePos, readPos;
    float thisInputVal, thisHannVal, thisOldVal, thisNewVal, maxVal;
    maxVal = 0.0;
    for (int x = 0; x < (oldPeriodLength_ * 2); x++)
    {
        writePos = (x + outputBufferWritePosition_ + outputBufferLength_) % outputBufferLength_;
        readPos = (x + inputBufferReadPosition_ + inputBufferLength_) % inputBufferLength_;
        thisInputVal = inputBuffer_.getSample(0, readPos);
        thisHannVal = 0.5f * (1.0f - cosf(2.0 * M_PI * x / (float)((newPeriodLength * 2) - 1)));
        thisOldVal = outputBuffer_.getSample(0, writePos);
        thisNewVal = thisOldVal + (thisInputVal * thisHannVal);
        outputBuffer_.setSample(0, writePos, thisNewVal);
    }
    
    // Increment pointers by a period
    inputBufferReadPosition_ += oldPeriodLength_;
    outputBufferWritePosition_ += newPeriodLength;
    
    // Update difference between pointers
    hopDiff_ += (oldPeriodLength_ - newPeriodLength);
    
    // Miss a period out if output is too far ahead of input
    if ((1 - hopDiff_) >= newPeriodLength)
    {
        inputBufferReadPosition_ += oldPeriodLength_;
        hopDiff_ += oldPeriodLength_;
    }
    // Duplicate a period if output is too far behind input
    if (hopDiff_ >= newPeriodLength)
    {
        inputBufferReadPosition_ -= oldPeriodLength_;
        hopDiff_ -=oldPeriodLength_;
    }
    
    // Wrap around circular buffers
    if (inputBufferReadPosition_ >= inputBufferLength_)
        inputBufferReadPosition_ -= inputBufferLength_;
    if (outputBufferWritePosition_ >= outputBufferLength_)
        outputBufferWritePosition_ -= outputBufferLength_;
    
    // Update new hop size
    pitchShiftHopSize_ = oldPeriodLength_;
}

void BassAutoTuneAudioProcessor::findAmpDifference()
{
    // Find the difference in amplitude between the input and the tuned (delayed) output
    
    float maxOrig = 0.0;
    float maxTuned = 0.0;
    int origPointer, tunedPointer;
    float origVal, tunedVal;
    float rmsOrig, rmsTuned;
    float countOrig = 0.0;
    float countTuned = 0.0;
    
    for (int i = 0; i < pitchShiftHopSize_; i++)
    {
        origPointer = (i + inputBufferWritePosition_ - pitchShiftHopSize_ + inputBufferLength_) % inputBufferLength_;
        origVal = abs(inputBuffer_.getSample(0, origPointer));
        tunedPointer = (i + outputBufferReadPosition_ + outputBufferLength_) % outputBufferLength_;
        tunedVal = abs(outputBuffer_.getSample(0, tunedPointer));
        if (origVal > maxOrig)
            maxOrig = origVal;
        if (tunedVal > maxTuned)
            maxTuned = tunedVal;
        countOrig += (origVal * origVal);
        countTuned += (tunedVal * tunedVal);
    }
    rmsOrig = sqrtf(countOrig / (float)pitchShiftHopSize_);
    rmsTuned = sqrtf(countTuned / (float)pitchShiftHopSize_);
    
    tunedAttenuation_ = maxOrig / maxTuned;
    //tunedAttenuation_ = rmsOrig / rmsTuned;
    if (tunedAttenuation_ > 1.0)
        tunedAttenuation_ = 1.0;
    if (maxTuned == 0.0)
        tunedAttenuation_ = 1.0;
    if (tunedAttenuation_ == 0.0)
        tunedAttenuation_ = 1.0;
    String message;
    //message << "SCM maxTuned = " << maxTuned << newLine;
    //message << "SCM maxOrig = " << maxOrig << newLine;
    message << "SCM tunedAtt = " << tunedAttenuation_ << newLine;
    //Logger::getCurrentLogger()->writeToLog (message);
    tunedAttenuation_ = 0.90;
}

// Main Processing

void BassAutoTuneAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();
    
    auto* leftChannelData = buffer.getWritePointer(0);
    auto* rightChannelData = buffer.getWritePointer(1);
    
    // inputBufferData is the circular buffer for collecting input samples for the block processing
    float* inputBufferData = inputBuffer_.getWritePointer(jmin (0, inputBuffer_.getNumChannels() - 1));
    
    for (int i = 0; i < numSamples; ++i)
    {
        const float in = (leftChannelData[i] + rightChannelData[i]) / 2.0f;
        inputBuffer_.setSample(0, inputBufferWritePosition_, in);
        
        // Store the current sample in the input buffer, incrementing the write pointer.
        inputBufferData[inputBufferWritePosition_] = in;
        if (++inputBufferWritePosition_ >= inputBufferLength_)
            inputBufferWritePosition_ = 0;
        
        
        //if (++samplesSinceLastBlockProcessed_ >= yinBufferHop_)
        if (++samplesSinceLastBlockProcessed_ >= pitchShiftHopSize_)
        {
            // Analysis buffer filled - get pitch
            samplesSinceLastBlockProcessed_ = 0;
            
            // Call helpers
            analysePitch(inputBufferData, ((inputBufferWritePosition_ - yinBufferSize_) % inputBufferLength_));
            calculateShift();
            pitchShiftBuffer();
            findAmpDifference();
        }
        
        // Calculate the output
        if (fadeIn_ || fadeOut_)
        {
            if (fadePos_ < autoTuneSamples_)
            {
                // Somewhere in the cross-fade - calculate the ratio
                
                float clean = in;
                float dirty = outputBuffer_.getSample(0, outputBufferReadPosition_) * tunedAttenuation_;
                float thisCleanVal;
                float thisDirtyVal;
                
                if (fadeIn_)
                {
                    // -1 < x < 1
                    float x = (float)(fadePos_ - pauseSamples_) / (float)crossFadeSamples_;
                    x = (2 * x) - 1;
                    thisCleanVal = sqrtf((1-x)/2) * clean;
                    thisDirtyVal = sqrtf((1+x)/2) * dirty;
                    buffer.setSample(0, i, (thisCleanVal + thisDirtyVal));
                }
                if (fadeOut_)
                {
                    // -1 < x < 1
                    float x = (float)fadePos_ / (float)autoTuneSamples_;
                    x = (2 * x) - 1;
                    thisCleanVal = sqrtf((1+x)/2) * clean;
                    thisDirtyVal = sqrtf((1-x)/2) * dirty;
                    buffer.setSample(0, i, (thisCleanVal + thisDirtyVal));
                }
                 
                fadePos_++;
            }
            else
            {
                // Cross-fade has ended - just output the end value (depending on in or out)
                
                if (fadeIn_)
                {
                    // All tuned output
                    buffer.setSample(0, i, (outputBuffer_.getSample(0, outputBufferReadPosition_)* tunedAttenuation_));
                    
                    
                }
                if (fadeOut_)
                {
                    buffer.setSample(0, i, in);
                }
                fadeIn_ = false;
                fadeOut_ = false;
                fadePos_ = 0;
            }
        }
        else
        {
            if (paused_)
            {
                // Initial pause (for onset) - pass through
                buffer.setSample(0, i, in);
                fadePos_++;
                if (fadePos_ > pauseSamples_)
                {
                    fadeIn_ = true;
                    paused_ = false;
                }
            }
            else
            {
                if (freqPitch_.currentMIDIPitchClass > -1)
                {
                    // It's the middle of a note, cross fade in has ended, play the tuned note
                    buffer.setSample(0, i, (outputBuffer_.getSample(0, outputBufferReadPosition_)* tunedAttenuation_));
                }
                else
                {
                    // No note detected, play the input audio
                    buffer.setSample(0, i, in);
                }
            }
        }
        
        // Original output passed through right channel
        buffer.setSample(1, i, in);
        
        
        // Clear the output sample in the buffer so it is ready for the next overlap-add
        outputBuffer_.setSample(0, outputBufferReadPosition_, 0);
        
        // Increment pointers and wrap
        if (++outputBufferReadPosition_ >= outputBufferLength_)
            outputBufferReadPosition_ = 0;
    }
    
    // Clear other channels
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
}

//==============================================================================
bool BassAutoTuneAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* BassAutoTuneAudioProcessor::createEditor()
{
    return new BassAutoTuneAudioProcessorEditor (*this);
}

//==============================================================================
void BassAutoTuneAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BassAutoTuneAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BassAutoTuneAudioProcessor();
}

