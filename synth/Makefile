synth: synth.cpp wav.h Makefile synth.h
	x86_64-w64-mingw32-g++ -I../fftw-3.3.5-dll64 -I../rtaudio-5.1.0 -L../rtaudio-5.1.0/build -std=c++11 -o synth synth.cpp -lrtaudio -ltbb

synth.exe: synth.cpp wav.h Makefile synth.h
	cl synth.cpp
