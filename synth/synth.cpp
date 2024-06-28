#include <RtAudio.h>
#include <cmath>
#include <iostream>
#include <list>
#include <ctime>
#include "pedals.h"
#include "amps.h"


#define TWELFTH 1.0594630943592952646

#include <iostream>

using namespace std;

int record(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void* userData) {
	if (status) {
		printf("Audio error\n");
		return -1;
	}

	double* in = (double*)inputBuffer, * out = (double*)outputBuffer;
	Pedalboard* p = (Pedalboard*)userData;
	memcpy(out, in, sizeof(double) * nBufferFrames * 2);
	p->processEffects(out, nBufferFrames, 2);
	return 0;
}

void printMenu() {
	cout << "(p) to print current board, (a) to add, (e) to edit, (n) to toggle effects, (q) to quit\n";
}

void printEffectMenu() {
	cout << "D: Delay\nF: Flange\nDist: Distortion\nC: Compressor\nP: Phaser\nLPF: LPF\nVox: Vox\n";
}

void printEffects(Pedalboard& pedal) {
	int f = 1;
	for (list<Pedal*>::iterator i = pedal.effects.begin(); i != pedal.effects.end(); ++i) {
		if((*i)->enabled)
			printf("%d: %s ON\n", f++, (*i)->name.c_str());
		else
			printf("%d: %s OFF\n", f++, (*i)->name.c_str());
	}
}

void printEffectsList(Pedalboard& pedal) {
	int f = 1;
	cout << endl;
	for (list<Pedal*>::iterator i = pedal.effects.begin(); i != pedal.effects.end(); ++i) {
		printf("%d %s\n", f, (*i)->name.c_str());
		++f;
	}
	cout << endl;

	string s;
	cin >> s;
	list<Pedal*>::iterator i;
	for (i = pedal.effects.begin(), f = 1; i != pedal.effects.end() && f < stoi(s); ++i, ++f);
	Pedal* t = *i;
	t->printSettings();
	cin >> s;
	int selection = stoi(s);
	t->selectSetting(selection);
}

void handleEnable(Pedalboard& pedal) {
	int f = 1;
	for (list<Pedal*>::iterator i = pedal.effects.begin(); i != pedal.effects.end(); ++i) {
		if ((*i)->enabled)
			printf("%d: %s ON\n", f++, (*i)->name.c_str());
		else
			printf("%d: %s OFF\n", f++, (*i)->name.c_str());
	}

	string s;
	cin >> s;
	list<Pedal*>::iterator i;
	for (i = pedal.effects.begin(), f = 1; i != pedal.effects.end() && f < stoi(s); ++i, ++f);
	Pedal* t = *i;
	if (t->enabled)
		t->disable();
	else
		t->enable();
}



int main() {
	int count;
	RtAudio audio;
	RtAudio::StreamParameters inParam, outParam;
	RtAudio::DeviceInfo info;

	Pedalboard pedal(48000, 2, new Amplifier());
	Flange* f = new Flange();
	Delay* d = new Delay();
	copyLPF* l = new copyLPF();
	ArayaDistortion* dist = new ArayaDistortion();
	Compressor* c = new Compressor();
	FFT* fft = new FFT();
	

	//pedal.effects.push_back(d);
	//pedal.effects.push_back(l);
	//pedal.effects.push_back(c);
	//pedal.effects.push_back(f);
	//pedal.effects.push_back(dist);
	pedal.effects.push_back(fft);

	

	c->enable();

	f->setDelay(0, 100);
	f->setLFO(0.667);
	f->setBlend(0.8);
	f->enable();

	d->setBlend(0.2);
	d->setDelay(5400);
	d->enable();

	l->enable();
	dist->enable();

	Vox* v = new Vox();
	//Phaser* phaser = new Phaser();
	fft->plugins.push_back(v);
	//fft->plugins.push_back(phaser);
	fft->enable();

	for (int i = 0; i < audio.getDeviceCount(); ++i) {
		info = audio.getDeviceInfo(i);
		printf("Device %d: name: %s inChannels: %d outChannels: %d pref: %d\n", i, info.name.c_str(), info.inputChannels, info.outputChannels, info.preferredSampleRate);
	}

	int inS, outS;
	cout << "Enter input device: ";
	cin >> inS;
	cout << "Enter output device: ";
	cin >> outS;
	//inS = 7;
	//outS = 6;

	inParam.deviceId = inS;
	outParam.deviceId = outS;
	inParam.nChannels = audio.getDeviceInfo(inS).inputChannels;
	outParam.nChannels = audio.getDeviceInfo(outS).outputChannels;
	inParam.firstChannel = 0;
	outParam.firstChannel = 0;
	RtAudio::StreamOptions options;
	options.flags = RTAUDIO_MINIMIZE_LATENCY;
	unsigned int sampleRate = audio.getDeviceInfo(inS).preferredSampleRate, bufferFrames = SIZE_2;
	sampleRate = 48000;
	audio.openStream(&outParam, &inParam, RTAUDIO_FLOAT64, sampleRate, &bufferFrames, &record, &pedal, &options);
	audio.startStream();

	while (true) {
		printMenu();
		string s;
		cin >> s;

		if (s == "q") { break; }
		else if (s == "p") {
			printEffects(pedal);
		}
		else if (s == "a") {
			string t;
			printEffectMenu();
			cin >> t;
			if (t == "d" || t == "D") {
				pedal.effects.push_back(new Delay());
			}
			else if (t == "f" || t == "F") {
				pedal.effects.push_back(new Flange());
			}
			else if (t == "Dist")
			{
				pedal.effects.push_back(new ArayaDistortion());
			}
			else if (t == "C" || t == "c")
			{
				pedal.effects.push_back(new Compressor());
			}
			else if (t == "P" || t == "p")
			{
				fft->plugins.push_back(new Phaser());
			}
			else if (t == "Vox")
			{
				fft->plugins.push_back(new Vox());
			}
		}
		else if (s == "e") {
			printEffectsList(pedal);
		}
		else if (s == "n") {
			handleEnable(pedal);
		}
	}

	audio.stopStream();

	delete d;
	return 0;
}