/*
 ==============================================================================
 Dave Foster Swing City Music
 ==============================================================================
 */

/***** YINPitchDetector.h *****/
/*
	Takes a buffer of samples, and performs the YIN algorithm for pitch detection
	Stores the pitch value as an ivar, as well as the confidence value
	The closer the confidence value is to zero, the better chance it is correct!
*/

class YINPitchDetector
{
	public:
	// Methods
		YINPitchDetector();
		YINPitchDetector(float sampleRate, int bufferSize, float threshold);
		float calculatePitch(float* inputBuffer);
	// Variables	
		float pitchYIN_;
		float confidenceYIN_;

	private:
	// Variables
		int bufferSize_;
		float sampleRate_;
		float threshold_;
		float* outputBuffer_;
};
