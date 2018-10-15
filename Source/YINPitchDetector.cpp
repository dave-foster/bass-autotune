/*
 ==============================================================================
 Dave Foster Swing City Music
 ==============================================================================
 */

/***** YINPitchDetector.cpp *****/

#include "../JuceLibraryCode/JuceHeader.h"
#include "YINPitchDetector.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

YINPitchDetector::YINPitchDetector()
{
	
}

YINPitchDetector::YINPitchDetector(float sampleRate, int bufferSize, float threshold)
{
	// Initialise the object and buffer etc
	sampleRate_ = sampleRate;
	bufferSize_ = bufferSize;
	threshold_ = threshold;
	
	pitchYIN_ = 0.0;
	confidenceYIN_ = 1.0;
	
	int halfBufferSize = bufferSize_ / 2;
	outputBuffer_ = (float *) malloc(sizeof(float)* halfBufferSize);
	memset(outputBuffer_, 0, halfBufferSize *  sizeof(float));
}

float YINPitchDetector::calculatePitch(float* inputBuffer)
{
    
    
	// Initialise empty values
	pitchYIN_ = 0.0;
	confidenceYIN_ = 1.0;
	int halfBufferSize = bufferSize_ / 2;
	memset(outputBuffer_, 0, halfBufferSize *  sizeof(float));
	
	// Start the algorithm - first, calculate the square difference
	float delta;
	
	float temp;
	float runningTotal = 0;
	for (int x = 0; x < halfBufferSize; x++)
	{
		delta = 0;
		temp = 0;
		for (int y = 0; y < halfBufferSize; y++)
		{
			delta = inputBuffer[y] - inputBuffer[x + y];
			temp += delta * delta;
		}
		runningTotal += temp;
		// Values are calculated by weighting using running sum and index values
		if (x == 0)
		{
			outputBuffer_[0] = 1;
		}
		else
		{
			outputBuffer_[x] = temp * (x / runningTotal);
		}
	}
	
	// Estimate tau by finding the first min below the threshold
	int tau = -1;
	for (int x = 1; x < (halfBufferSize - 1); x++)
	{
        if (outputBuffer_[x] < threshold_)
		{
			if ((outputBuffer_[x] < outputBuffer_[x-1]) &&  (outputBuffer_[x] < outputBuffer_[x+1]))
			{
				// Found it!
				tau = x;
				// The confidence value is the value of this bin
				confidenceYIN_ = outputBuffer_[x];
				// break
				x = halfBufferSize;
			}
		}
	}
	
	// If there are no values that are below the threshold, fail gracefully
	if (tau < 1)
	{
        return 0.0;
	}
	
	// Use parabolic interpolation to get exact pitch value
	// http://ccrma.stanford.edu/~jos/sasp/Quadratic_Peak_Interpolation.html
	float s0 = outputBuffer_[tau - 1];
	float s1 = outputBuffer_[tau];
	float s2 = outputBuffer_[tau + 1];
	float interpolatedValue = tau + ((s2 - s0) / (2 * (2 * s1 - s2 - s0)));
	//float interpolatedLocation = 0.5 * (s0 - s1)/(s0 - 2*s1 + s2);
	//float interpolatedValue = s1 - 0.25*(s0 - s2)*interpolatedLocation;
	pitchYIN_ = sampleRate_ / interpolatedValue;
	
	return pitchYIN_;
}
