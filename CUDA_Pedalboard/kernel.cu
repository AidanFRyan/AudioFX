
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "RtAudio.h"
#include <cmath>
#include <iostream>
#include <list>

#define TWELFTH 1.0594630943592952646

#include <iostream>

using namespace std;

__global__ void cudaDelay(double* a, double* prev, unsigned int numFrames, int numChannels, unsigned int d, double b) {
	unsigned int offset = gridDim.x * blockDim.x;
	for (unsigned int i = threadIdx.x + blockIdx.x * blockDim.x; i < numFrames*numChannels; i += offset) {
		//unsigned int t = i * numChannels;
		//printf("%d %f\n", i, a[i]);
		if (prev != 0 && i < d*numChannels) {
			a[i] += b*prev[numFrames*numChannels - i];
		}
		else if(prev != 0) {
			a[i] += b * a[i - d*numChannels];
		}
		//printf("%d\t%f\n", a[t + j]);
	}
}

__global__ void monoOneToStereo(double* in, double* a, unsigned int numFrames, int numChannels) {
	unsigned int offset = gridDim.x * blockDim.x;
	for (unsigned int i = threadIdx.x + blockIdx.x * blockDim.x; i < numFrames; i += offset) {
		unsigned int t = i * numChannels;
		for (int j = 0; j < numChannels; ++j) {
			a[t + j] = in[t];
			//printf("%f\n", a[t + j]);
		}
	}
}

class Pedal {
public:
	virtual void process(double* audio, int n, int c) = 0;
	void enable() {
		enabled = true;
	}
	void disable() {
		enabled = false;
	}
	bool enabled;
	list<double*> prev;
};

class Delay : public Pedal {
public:
	Delay() {
		//prev = 0;
		delay = 0;
		enabled = true;
	}
	virtual void process(double* a, int n, int c) {
		if (!enabled)
			return;
		cudaDelay << <1, 512 >> > (a, prev.back(), n, c, delay, blend);
		cudaDeviceSynchronize();
		if (prev.size() == 0) {
			double* t;
			cudaMalloc((void**)& t, sizeof(double) * n * c);
			prev.push_back(t);
		}
		cudaMemcpy(prev.back(), a, sizeof(double) * n * c, cudaMemcpyDeviceToDevice);

	}
	void setDelay(int samples) {
		delay = samples;
	}
	void setBlend(double b) {
		blend = b;
	}
	int delay;
	double blend;
};

class Pedalboard {
public:
	Pedalboard() {
		audio = 0;
		dev = 0;
	}

	void allOff() {
	}

	void allOn() {
	}

	void processEffects(double* dev, int numFrames, int numChannels) {
		for(list<Pedal*>::iterator i = effects.begin(); i != effects.end(); i++){
			(*i)->process(dev, numFrames, numChannels);
		}		
	}

	double *audio, *dev;
	list<Pedal*> effects;
};

int record(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void* userData) {
	if (status) {
		printf("Audio error\n");
		return -1;
	}

	double *in = (double*)inputBuffer, *out = (double*)outputBuffer, *d_in, *d_out;
	Pedalboard* p = (Pedalboard*)userData;
	

	cudaMalloc((void**)& d_in, sizeof(double) * nBufferFrames * 2);
	cudaMalloc((void**)& d_out, sizeof(double) * nBufferFrames * 2);

	cudaMemcpy(d_in, in, sizeof(double) * nBufferFrames * 2, cudaMemcpyHostToDevice);
	monoOneToStereo << <1, 512 >> > (d_in, d_out, nBufferFrames, 2);
	cudaDeviceSynchronize();

	cudaFree(d_in);
	p->processEffects(d_out, nBufferFrames, 2);

	cudaMemcpy(out, d_out, sizeof(double) * nBufferFrames * 2, cudaMemcpyDeviceToHost);
	cudaFree(d_out);

	return 0;
}

int main() {
	int count;
	cudaGetDeviceCount(&count);
	printf("CUDA Devices: %d\n", count);
	RtAudio audio;
	RtAudio::StreamParameters inParam, outParam;
	RtAudio::DeviceInfo info;

	Pedalboard pedal;
	Delay* d = new Delay();
	pedal.effects.push_back(d);
	d->setDelay(120);
	d->setBlend(1);
	d->disable();

	for (int i = 0; i < audio.getDeviceCount(); ++i) {
		info = audio.getDeviceInfo(i);
		printf("Device %d: name: %s inChannels: %d outChannels: %d pref: %d\n", i, info.name.c_str(), info.inputChannels, info.outputChannels, info.preferredSampleRate);
	}

	int inS, outS;
	//cout << "Enter input device: ";
	//cin >> inS;
	//cout << "Enter output device: ";
	//cin >> outS;
	inS = 8;
	outS = 6;

	inParam.deviceId = inS;
	outParam.deviceId = outS;
	inParam.nChannels = audio.getDeviceInfo(inS).inputChannels;
	outParam.nChannels = audio.getDeviceInfo(outS).outputChannels;
	inParam.firstChannel = 0;
	outParam.firstChannel = 0;
	RtAudio::StreamOptions options;
	options.flags = RTAUDIO_MINIMIZE_LATENCY;
	unsigned int sampleRate = audio.getDeviceInfo(inS).preferredSampleRate, bufferFrames = 8192;
	audio.openStream(&outParam, &inParam, RTAUDIO_FLOAT64, sampleRate, &bufferFrames, &record, &pedal, &options);
	audio.startStream();

	cout << "\nRecording ... press enter to quit\n";

	string s;
	cin >> s;

	audio.stopStream();

	delete d;
	return 0;
}