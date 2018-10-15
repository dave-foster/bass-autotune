/*
 ==============================================================================
 Dave Foster Swing City Music
 ==============================================================================
 */

/***** FreqPitch.cpp *****/

#include "FreqPitch.h"
#include <math.h> 


FreqPitch::FreqPitch(float initialFreq)
{
	if (initialFreq < 0.0)
		initialFreq = 0.0;
	if (initialFreq > 4000.0)
		initialFreq = 0.0;
	setNewFreq(initialFreq);
}

bool FreqPitch::setNewFreq(float newFreq)
{
	// Setting with zero freq nulls all the settings
	if (newFreq < 0.0)
		newFreq = 0.0;
	if (newFreq > 4000.0)
		newFreq = 0.0;
	if (newFreq == 0.0)
	{
		if (currentFreq == 0.0)
			currentPitchClassUpdated = false;
		else
			currentPitchClassUpdated = true;
		currentFreq = 0.0;
		currentFreqInTune = 0.0;
		currentPitchDev = 0.0;
		currentMIDIError = 0.0;
		currentMIDIPitchClass = -1;
		currentMIDIPitch = -1;
		currentMIDIInTune = 0.0;
		currentPitchName = "";
		currentIsSharp = false;
		return false;
	}
	// Otherwise, calculate all of the parameters
	currentFreq = newFreq;
	float exactMIDIPitch = freqToMIDI(newFreq);
	float roundedMIDIPitch = roundf(exactMIDIPitch);
	currentFreqInTune = midiToFreq(roundedMIDIPitch);
	currentMIDIPitch = (int)roundf(exactMIDIPitch);
	currentMIDIError = (exactMIDIPitch - roundedMIDIPitch) * 100.0;
	currentPitchDev = currentFreq - currentFreqInTune;
	// Get the new pitch class, and check whether it has changed
	int newPitchClass = noteLetterFromMIDIPitch(currentMIDIPitch);
	if (currentMIDIPitchClass == newPitchClass)
		currentPitchClassUpdated = false;
	else
		currentPitchClassUpdated = true;
	currentMIDIPitchClass = newPitchClass;
	return true;
}

float FreqPitch::midiToFreq(float midi)
{
	// Assumes that we're being passed a reasonable value
	// Safe as it only gets called from setNewFreq where the check is already done
	return pow(2.0f, (midi - 69.0f)/12.0f) * 440.0f;
}

float FreqPitch::freqToMIDI(float freq)
{
	// Assumes that we're being passed a reasonable value
	// Safe as it only gets called from setNewFreq where the check is already done
	
	return 69.0f + 12.0f*log2f(freq/440.0f);
}

int FreqPitch::noteLetterFromMIDIPitch(int midiPitch)
{
	// Sets the ivars directly
	
	int mod = midiPitch % 12;
	switch(mod)
	{
		case 1:
            currentPitchName = "C#";
			currentIsSharp = true;
			break;
		case 2:
			currentPitchName = "D";
			currentIsSharp = false;
			break;
		case 3:
            currentPitchName = "D#";
			currentIsSharp = true;
			break;
		case 4:
			currentPitchName = "E";
			currentIsSharp = false;
			break;
		case 5:
			currentPitchName = "F";
			currentIsSharp = false;
			break;
		case 6:
            currentPitchName = "F#";
			currentIsSharp = true;
			break;
		case 7:
			currentPitchName = "G";
			currentIsSharp = false;
			break;
		case 8:
            currentPitchName = "G#";
			currentIsSharp = true;
			break;
		case 9:
			currentPitchName = "A";
			currentIsSharp = false;
			break;
		case 10:
			currentPitchName = "Bb";
			currentIsSharp = true;
			break;
		case 11:
			currentPitchName = "B";
			currentIsSharp = false;
			break;
		default: // mod = 0
			currentPitchName = "C";
			currentIsSharp = false;
			break;
	}
	return mod;
}
