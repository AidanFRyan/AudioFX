#define _USE_MATH_DEFINES

#include "RtAudio.h"
#include "wav.h"
#include <cmath>
using namespace std;

class AudioObject{
public:
	AudioObject() {
		if (audioOut.getDeviceCount() < 1)
			printf("No audio output found\n");
		if (audioIn.getDeviceCount() < 1) {
			printf("No audio input found\n");
		}
		name = 0;
		a = 0;
		p = false;
		
	}

	void enableStream(){
		RtAudio::StreamParameters p;
		RtAudio::StreamOptions o;
		o.flags = RTAUDIO_MINIMIZE_LATENCY;
		p.deviceId = audioOut.getDefaultOutputDevice();
		p.nChannels = this->channels;
		p.firstChannel = 0;
		unsigned int buff = samples;
		dat[0] = this;
		try {
			audioOut.openStream(&p, 0, RTAUDIO_FLOAT64, rate, &buff, (RtAudioCallback)&copyBuffOut, &dat, &o);
			printf("Device %d enabled\n", p.deviceId);
		}
		catch (RtAudioError &error) {
			error.printMessage();
		}

    }

	void start() {
		audioOut.startStream();
	}

	void close() {
		audioOut.closeStream();
	}

    void writeToWAV(){
        for(int i = 0; i < size; ++i){
        }
        WriteWaveFile(name, a, size*sizeof(double), channels, rate, sizeof(double)*8);
    }

	static int copyBuffOut(void* outBuff, void* inBuff, unsigned int buffSize, double streamTime, RtAudioStreamStatus stat, void** data) {
		if (stat) {
			printf("audio underflow\n");
			return -1;
		}
		//printf("called\n");
		AudioObject* obj = (AudioObject*)(data[0]);
		double* out = (double*) outBuff;
		//unsigned int t = obj->size - buffSize;
		
		for (int i = 0; i < obj->size; ++i) {
			//out[i] = obj->a[i];
			out[i] = 1;
		}
		//if (obj->p)
			//obj->generate();

		return 0;
	}

	void play() {
		p = true;
		generate();
	}

	void stop() {
		p = false;
		for (int i = 0; i < size; ++i) {
			a[i] = 0;
		}
	}

	void print() {
		for (int i = 0; i < samples; ++i){
			printf("%d: ", i);
			for (int j = 0; j < channels; ++j)
				printf("%d[%f] ", j, a[i * channels + j]);
			printf("\n");

		}
	}

	void setFrequency(double f) {
		freq = f;
	}

	virtual void generate() = 0;

    RtAudio audioOut, audioIn;
    char* name;
    unsigned int rate, channels, samples, size;
	double* a, phase, freq;
	void* dat[2];
	bool p;
};

class Synth : public AudioObject{
public:
    Synth(){
        size = 0;
        name = 0;
        rate = 0;
        channels = 0;
        //duration = 0;
        samples = 0;
    }

    Synth(char* n, int r, int c, int s, double f) : Synth(){
        name = n;
        rate = r;
        channels = c;
        samples = s;
        size = samples*channels;
        a = new double[size];
		freq = f;
    }
    
    void setChannelValueAtSample(int sample, int channel, double value){
        a[sample*channels+channel] = value;
		//a[channel * samples + sample] = value;//noninterleaved
    }

    void fillChannelSinFrequency(int channel, double frequency){
        phase = 0;
        for(int i = 0; i < samples; ++i){
            setChannelValueAtSample(i, channel, advanceOscillatorSine(frequency));
        }
		freq = frequency;
    }

    double advanceOscillatorSine(double frequency){
        phase += 2 * M_PI * frequency/rate;

        while(phase >= 2 *M_PI){
            phase -= 2*M_PI;
        }

        while(phase <= 0){
            phase += 2*M_PI;
        }

        return sin(phase);
    }

    ~Synth(){
        if(a != nullptr)
            delete[] a;
    }

    void bandLimitedSaw(int channel, double frequency){
        for(int i = 0; i < samples; ++i){
            a[i*channels + channel] = 0;
        }
        int l = 1; 
        for(double j = frequency; j < rate/2; j += frequency, ++l){
            for(int i = 0; i < samples; ++i){
                a[i*channels + channel] += (1/l) * advanceOscillatorSine(j);
            }
        }
        for(int i = 0; i < samples; ++i){
            a[i*channels + channel] *= 2/M_PI;
        }
		freq = frequency;
    } 
	void generate() {
		//fillChannelSinFrequency(0, freq);
		//fillChannelSinFrequency(1, freq);
		for (int i = 0; i < channels; ++i) {
			fillChannelSinFrequency(i, freq);
		}
	}
};


class Flange : public AudioObject{
    double *input, LFOfrequency, depth, phase;

public:
    Flange(){
        input = nullptr;
        a = nullptr;
        size = 0;
        samples = 0;
    }

    Flange(const Synth& s, double LFO, double d) : Flange() {
        rate = s.rate;
        name = s.name;
        channels = s.channels;
        //duration = s.duration; 
        samples = s.samples;
        size = s.size;

        input = s.a;
        a = new double[size];
        LFOfrequency = LFO;
        depth = d;
        phase = 0;
    }

    double advanceLFO(){
        phase += 2 * M_PI * LFOfrequency/rate;

        while(phase >= 2 *M_PI){
            phase -= 2*M_PI;
        }

        while(phase <= 0){
            phase += 2*M_PI;
        }

        return (sin(phase)+1.0)/2.0;
    }

    double interpFromLFOPhase(int cur, int channel){
        if(cur <= (int)(depth*rate))
            return 0;
        double t = advanceLFO()*depth*rate, rem = t - (int)t;
        int it = int(t);
        return input[(cur-(it)-1)*channels + channel]*(rem) + input[(cur-(it))*channels + channel]*(1.0-rem);
    }
    
    void solveFlange(){
        for(int i = 0; i < samples; ++i){
            for(int j = 0; j < channels; ++j){
                    a[i*channels + j] = input[i*channels + j] + interpFromLFOPhase(i, j);
            }
        }
    }
    
    ~Flange(){
        if(a != nullptr){
            delete[] a;
            a = nullptr;
        }
    }
};