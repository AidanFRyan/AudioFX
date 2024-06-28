#pragma once
#include <list>
#include <vector>
#include <iostream>
#include "wdf.h"
#include "amps.h"
#include <string>
#include "oscillator.h"


using namespace std;

class Pedal {
public:
	Pedal(int sr = 48000, int ch = 2) {
		enabled = false;
		sampleRate = sr;
		channels = ch;
	}
	virtual void process(double* audio, int n, int c) = 0;
	void enable() {
		enabled = true;
	}
	void disable() {
		enabled = false;
	}

	virtual void printSettings() = 0;
	virtual void selectSetting(int s) = 0;
	bool enabled;
	int sampleRate, channels;
	string name;
};

class Compressor : public Pedal {
public:
	Compressor(int sr = 48000, int ch = 2) : Pedal(sr, ch) {
		name = "Kompressor";
		threshold = 0.5;
		ratio = 0.0125;
		width = 0.25;
	}
	virtual void process(double* a, int n, int c) {
		if (!enabled)
			return;
		for (int i = 0; i < n; ++i) {
			int o = i * c;
			double tt = 2.0 * abs(a[o] - threshold);
			if (tt >= -width && tt < width) {
				double ttt = (a[o] - threshold + width / 2.0);
				a[o] += ((1/(ratio-1))*ttt*ttt)/(2.0*width);
			}
			else if (tt >= width) {
				threshold + (a[o] - threshold) / ratio;
			}
		}
	}
	void printSettings() {
		printf("1: Threshold\n2: Ratio\n3: Width\n");
	}
	void selectSetting(int n) {
		string s;
		if (n == 1) {
			printf("Current value is %f\n", threshold);
			printf("Enter a value from 0 to 1\n");
			cin >> s;
			threshold = stod(s);
		}
		else if (n == 2) {
			printf("Current value is %f\n", ratio);
			printf("Enter a value from 0 to 1\n");
			cin >> s;
			ratio = stod(s);
		}
		else if (n == 3) {
			printf("Current value is %f\n", width);
			printf("Enter a value from 0 to 1\n");
			cin >> s;
			width = stod(s);
		}
	}
	double threshold, ratio, width;
};

class Delay : public Pedal {
public:
	Delay(int sr = 48000, int ch = 2) : Pedal(sr, ch){
		delay = 0;
		blend = 0.5;
		name = "Delay";
	}
	virtual void process(double* a, int n, int c) {
		if (!enabled)
			return;
		list<double> t;
		for(int i = 0; i < n; ++i) {
			int o = i * c;
			a[o] += blend * prev.front();
			if (!prev.empty())
				prev.pop_front();
			prev.push_back(a[o]);
		}
	}
	void setDelay(int samples) {
		delay = samples;
		
		for (int i = prev.size(); i < samples * channels; ++i) {
			prev.push_back(0);
		}
	}
	void setBlend(double b) {
		blend = b;
	}
	void printSettings() {
		printf("1: Set Delay\n2: Blend\n");
	}
	void selectSetting(int i) {
		string s;
		if (i == 1) {
			printf("Current value is %d\n", delay);
			printf("Enter an integer from 0 to infinity\n");
			cin >> s;
			setDelay(stoi(s));
		}
		if (i == 2) {
			printf("Current value is %f\n", blend);
			printf("Enter value from 0 to 1\n");
			cin >> s;
			setBlend(stod(s));
		}
	}

	list<double> prev;
	int delay;
	double blend;
};

#include "circularBuffer.h"

class Flange : public Pedal{
public:
	Flange(int sr = 48000, int ch = 2){
		maxSamples = 0;
		minSamples = 0;
		name = "Flange";
	}
	void process(double* a, int n, int c) {
		if (!enabled)
			return;
		
		for(int i = 0; i < n; ++i) {
			int o = i * c;
			prev.newSample(a[o]);
			double s = (maxSamples - minSamples) * ((lfo.advanceOscillatorSine(sampleRate) + 1.0) / 2) + minSamples;
			a[o] += blend * prev.getSample(s);
		}
	}
	void setDelay(int min, int max) {
		prev.resize(max);
		minSamples = min;
		maxSamples = max;
	}
	void setBlend(double b) {
		blend = b;
	}
	void setLFO(double freq) {
		lfo.frequency = freq;
	}
	void printSettings() {
		printf("1: LFO Frequency\n2: Min Delay\n3: Max Delay\n4: Blend\n");
	}
	void selectSetting(int i) {
		string s;
		if (i == 1) {
			printf("Current value is %f\n", lfo.frequency);
			printf("Enter value from 0 to infinity:\n");
			cin >> s;
			this->setLFO(stod(s));
		}
		if (i == 2) {
			printf("Current value is %d\n", minSamples);
			printf("Enter an integer from 0 to %d\n", maxSamples);
			cin >> s;
			setDelay(stoi(s), maxSamples);
		}
		if (i == 3) {
			printf("Current value is %d\n", maxSamples);
			printf("Enter an integer from %d to infinity\n", minSamples);
			cin >> s;
			setDelay(minSamples, stod(s));
		}
		if (i == 4) {
			printf("Current value is %f\n", blend);
			printf("Enter a value from 0 to 1\n");
			cin >> s;
			setBlend(stod(s));
		}
	}

	CircularBuffer<double> prev;
	Oscillator lfo;
	int minSamples, maxSamples;
	double blend;
};

class Pedalboard {
public:
	Pedalboard(int sr = 48000, int ch = 2, Amplifier* a = 0) {
		if (a == 0) {
			amp = new Amplifier(sr, ch);
		}
		else {
			amp = a;
		}
		audio = 0;
		dev = 0;
		sampleRate = sr;
		channels = ch;
	}

	void allOff() {
	}

	void allOn() {
	}

	void monoify(double* dev, int n, int numChannels = 2, int targetChannel = 0) {
		for (int i = 0; i < n; ++i) {
			dev[i] = dev[i * numChannels + targetChannel];
		}
	}

	void processEffects(double* dev, int numFrames, int numChannels) {
		for (list<Pedal*>::iterator i = effects.begin(); i != effects.end(); i++) {
			(*i)->process(dev, numFrames, numChannels);
		}
		amp->assignAudio(dev);
		amp->process(numFrames);
		amp->monoToStereo(numFrames);
	}

	double* audio, * dev;
	list<Pedal*> effects;
	Amplifier* amp;
	int sampleRate, channels;
};

class copyLPF : public Pedal {
public:
	copyLPF(int sr = 48000, int ch = 2) : Pedal(sr, ch) {
		name = "copyLPF";
		createWDF();
		setSR(sampleRate);
	}
	WdfSeriesAdaptor sAL1;
	WdfParallelAdaptor pAC1;
	WdfSeriesTerminatedAdaptor sTAL2;
	void createWDF() {
		sAL1.setComponent(wdfComponent::L, 9.549e-3);
		pAC1.setComponent(wdfComponent::C, 0.05305e-6);
		sTAL2.setComponent(wdfComponent::L, 9.549e-3);
		WdfAdaptorBase::connectAdaptors(&sAL1, &pAC1);
		WdfAdaptorBase::connectAdaptors(&pAC1, &sTAL2);
		sAL1.setSourceResistance(600.0);
		sTAL2.setSourceResistance(600.0);
	}
	bool setSR(double sr) {
		sAL1.reset(sr);
		pAC1.reset(sr);
		sTAL2.reset(sr);
		sAL1.initializeAdaptorChain();
		sampleRate = sr;
		return true;
	}
	void process(double* audio, int n, int c = 2) {
		if (!enabled)
			return;
		for (int i = 0; i < n; ++i) {
			int o = i * c;
			sAL1.setInput1(audio[o]);
			audio[o] = sTAL2.getOutput2();
		}
	}
	void selectSetting(int n) {
		printf("1: Band\n");
	}

	void printSettings() {

	}
};

class ArayaDistortion : public Pedal {
public:
	ArayaDistortion(int sr = 48000, int ch = 2) : Pedal(sr, ch) {
		name = "Soft Clip";
	}
	void process(double* audio, int n, int c = 2) {
		if (!enabled) {
			return;
		}
		for(int i = 0; i < n; ++i) {
			int o = i * c;
			if (audio[o] > 1.0) {
				audio[o] = 0.666666666;
			}
			else if (audio[o] < -1.0) {
				audio[o] = -0.666666666;
			}
			else {
				audio[o] = audio[o] - audio[o] * audio[o] * audio[o] / 3;
			}
		}
	}
	void selectSetting(int n) {
		string s;
		if (n == 1) {
			printf("Current value is %f\nEnter a value from 0 to 1\n", offset);
			cin >> s;
			offset = stod(s);
		}
	}

	void printSettings() {
		printf("1: Offset\n");
	}
	double offset = 0.8;
};

class Stomper : public Pedal {

};

class FFTPlugin {
public:
	virtual void process(fftw_complex* d) = 0;
	int sampleRate;
	int channels;
};

class Vox : public FFTPlugin{
public:
	Vox(int sr = 48000, int ch = 2) {
		sampleRate = sr;
		channels = ch;
		for (int i = 0; i < 32; ++i) {
			adjusted[i] = spl[i] / ampMax;
		}
		precomp = 0;
		n = SIZE;
		precompute();
	}
	double interp(double f) {
		int i = 1;
		for (i = 1; i < 30 && freqs[i] < f; ++i);
		--i;
		f -= freqs[i];
		double t = f / (freqs[i + 1] - freqs[i]);
		return adjusted[i + 1] * t + (1 - t) * adjusted[i];
	}
	void precompute() {
		if (precomp != 0) {
			delete[] precomp;
		}
		precomp = new double[n];
		for (int i = 0; i < n; ++i) {
			precomp[i] = interp(i * sampleRate / n);
		}
	}
	void setSamples(int nn) {
		n = nn;
		precompute();
	}
	void process(fftw_complex* d) {
		for (int i = 0; i < n; ++i){
			d[i][0] *= precomp[i];
		}
	}
	double freqs[30] = { 20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 600, 700, 800, 900, 950, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 15000, 20000 },
			 spl[30] = { 70, 80, 80, 83, 85, 86, 90, 92, 92, 97, 97.5, 100, 100, 100, 102, 102, 102, 100, 95, 105, 106, 106, 100, 95, 86, 84, 79, 82, 60, 75 }, ampMax = 106, adjusted[30],
		*precomp;
	int n;
};

#include <random>

class Phaser : public FFTPlugin {
public:
	Phaser(int sr = 48000, int ch = 2) {
		dist = uniform_real_distribution<double>(0.0, 1.0);
		sampleRate = sr;
		channels = ch;
		double base = 0, width = 0.8;
		for (int i = 0; i < 2; ++i) {
			Oscillator l;
			l.frequency = base + dist(gen)*width;
			lfos.push_back(l);
		}
	}
	void process(fftw_complex* d) {
		for (list<Oscillator>::iterator i = lfos.begin(); i != lfos.end(); ++i) {
			Oscillator& lfo = *i;
			double lf = lfo.advanceOscillatorSine(sampleRate);// , p = lfo.phase;
			double o = lf * 2 * M_PI - M_PI;
			for (int i = 0; i < SIZE; ++i) {
				d[i][1] += o;
			}
		}
	}
	list<Oscillator> lfos;
	default_random_engine gen;
	uniform_real_distribution<double> dist;
};

class FFT : public Pedal {
public:
	FFT(int sr = 48000, int c = 2) : Pedal(sr, c) {
		name = "FFT";
		lastHalf.reserve(SIZE);
		for (int i = 0; i < SIZE; ++i) {
			lastHalf.push_back(0);
		}
		
		blend = 0.5;
		ou = new fftw_complex[SIZE];
		p = fftw_plan_dft_1d(SIZE, ou, ou, FFTW_FORWARD, FFTW_ESTIMATE);
		q = fftw_plan_dft_1d(SIZE, ou, ou, FFTW_BACKWARD, FFTW_ESTIMATE);
	}
	void copyVec(int n) {
		out.clear();
		out.resize(n);
	}
	void convertToMagnitudeAndPhase(fftw_complex* t, int n) {
		for (int i = 0; i < n; ++i) {
			fftw_complex X;
			X[0] = t[i][0];
			X[1] = t[i][1];
			t[i][0] = sqrt(X[0] * X[0] + X[1] * X[1]);
			t[i][1] = atan(X[1] / X[0]) + (X[0] >= 0 ? 0 : M_PI);
		}
	}
	void convertToRealAndImaginary(fftw_complex* t, int n) {
		for (int i = 0; i < n; ++i) {
			fftw_complex X;
			X[0] = t[i][0];
			X[1] = t[i][1];
			t[i][0] = X[0]*cos(X[1]);
			t[i][1] = X[0]*sin(X[1]);
		}
	}
	void forward() {
		fftw_execute(p);
		convertToMagnitudeAndPhase(ou, SIZE);
	}
	void backward() {
		convertToRealAndImaginary(ou, SIZE);
		
		fftw_execute(q);
	}
	void processFFT() {
		for (list<FFTPlugin*>::iterator i = plugins.begin(); i != plugins.end(); ++i) {
			(*i)->process(ou);
		}
	}
	virtual void process(double* audio, int n, int c) {
		if (!enabled)
			return;
		out.resize(n);

		for (int i = -(SIZE); i <= n - SIZE; ++i) {
			double tttt = 2.0 * M_PI / SIZE;
			for (int j = 0; j < SIZE; ++j) {
				if (i + j < 0) {
					ou[j][0] = lastHalf[i + (SIZE) + j] * (0.54 - 0.46 * cos(tttt));
				}
				else {
					ou[j][0] = audio[(i + j)*c] * (0.54 - 0.46 * cos(tttt));
				}
				ou[j][1] = 0;
				tttt += tttt;
			}

			forward();
			processFFT();
			backward();
			tttt = 2.0 * M_PI / SIZE;
			for (int j = 0; j < SIZE; ++j) {
				if (i + j >= 0) {
					out[i + j] += (ou[j][0] / SIZE)/(0.54 - 0.46 * cos(tttt));
					if (j == 0) {
						out[i] /= SIZE;
					}

					
				}
				tttt += tttt;
			}
		}
		for (int i = 1; i < SIZE; ++i) {
			out[n - i] /= i;
		}
		for (int i = 0; i < n; ++i) {
			if (i >= n - (SIZE)) {
				lastHalf[i - (n - (SIZE))] = audio[i*c];
			}
			//printf("%d %f %f\n", i, audio[i*c], out[i]);
			audio[i*c] = out[i]*blend + (1.0-blend)*audio[i*c];
			out[i] = 0;
		}
	}
	~FFT() {
		fftw_destroy_plan(q);
		fftw_destroy_plan(p);
	}
	void printSettings() {

	}
	void selectSetting(int n) {

	}
	vector<double> out, lastHalf;
	fftw_plan p, q;
	fftw_complex *ou;
	//Oscillator lfo;
	list<FFTPlugin*> plugins;
	double blend;
};
