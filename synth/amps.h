#pragma once
#include "common.h"
#include "wdf.h"
#include <fftw3.h>
#include "circularBuffer.h"
#include "fft.h"
#include "oscillator.h"
using namespace std;

class Amplifier {
public:
	Amplifier(int sr = 48000, int c = 2) {
		numChannels = 2;
		sampleRate = sr;
		audio = 0;
	}
	virtual void process(int numFrames) {
	}
	void monoToStereo(int numFrames) {
		for (int i = numFrames - 1; i >= 0; --i) {
			int t = i * numChannels;
			for (int j = 1; j < numChannels; ++j) {
				audio[t + j] = audio[t];
			}
		}
	}
	void assignAudio(double* a) {
		audio = a;
	}
	double* audio;
	int numChannels, sampleRate;
};

class Tube : public Amplifier {
public:
	Tube(int s = 48000, int c = 2) : Amplifier(s, c) {

	}
	void process(int numFrames) {

	}
};