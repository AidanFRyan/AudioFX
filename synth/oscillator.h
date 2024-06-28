#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

class Oscillator {
public:
	Oscillator() {
		phase = 0;
		frequency = 0;
	}
	double advanceOscillatorSine(int sampleRate) {
		phase += 2 * M_PI * frequency / sampleRate;

		while (phase >= 2 * M_PI) {
			phase -= 2 * M_PI;
		}

		while (phase <= 0) {
			phase += 2 * M_PI;
		}

		return sin(phase);
	}

	double frequency, phase;
};