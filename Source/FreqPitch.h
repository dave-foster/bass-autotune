/*
 ==============================================================================
 
 ECS730P DAFX 2018
 David Foster 160896851
 ==============================================================================
 */

/***** FreqPitch.h *****/
/*
	Takes a frequency value, and calculates the corresponding MIDI pitch number,
	the note name and accidental (i.e. "C" and "true" for C#, "G" and "false" for G nat)
	as well as the tuning deviation from the in tune note
*/

#include <string>

class FreqPitch
{
	public:
		float currentFreq;
		std::string currentPitchName;
		// currentMIDIPitch is the actual MIDI note number
		int currentMIDIPitch;
		float currentMIDIError; // in cents = 100th semitone
		// currentMIDIPitchClass is the note letter number i.e. ignoring octave
		// So 0 = C, 1 = C#, 2 = D etc.
		// (-1 means that there is no current pitch)
		int currentMIDIPitchClass;
		float currentPitchDev;
		bool currentIsSharp;
		// currentFreqInTune is the frequency if the note was in tune
		float currentFreqInTune;
		float currentMIDIInTune;
		bool currentPitchClassUpdated;
		
		FreqPitch(float initialFreq = 0.0);
		bool setNewFreq(float newFreq);
		
	private:
		float midiToFreq(float midi);
		float freqToMIDI(float freq);
		int noteLetterFromMIDIPitch(int midiPitch);
};
