#include <iostream>
#include <fstream>
#include <memory.h>
using namespace std;

struct SMinimalWaveFileHeader
{
	//the main chunk
	unsigned char m_szChunkID[4];
	unsigned int m_nChunkSize;
	unsigned char m_szFormat[4];

	//sub chunk 1 "fmt "
	unsigned char m_szSubChunk1ID[4];
	unsigned int m_nSubChunk1Size;
	unsigned short m_nAudioFormat;
	unsigned short m_nNumChannels;
	unsigned int m_nSampleRate;
	unsigned int m_nByteRate;
	unsigned short m_nBlockAlign;
	unsigned short m_nBitsPerSample;

	//sub chunk 2 "data"
	unsigned char m_szSubChunk2ID[4];
	unsigned int m_nSubChunk2Size;

	//then comes the data!
};

bool WriteWaveFile(const char *szFileName, void *pData, int nDataSize, short nNumChannels, int nSampleRate, int nBitsPerSample)
{
	//open the file if we can
	FILE *File = fopen(szFileName,"w+b");
	if(!File)
	{
		return false;
	}

	SMinimalWaveFileHeader waveHeader;

	//fill out the main chunk
	memcpy(waveHeader.m_szChunkID,"RIFF",4);
	waveHeader.m_nChunkSize = nDataSize + 36;
	memcpy(waveHeader.m_szFormat,"WAVE",4);

	//fill out sub chunk 1 "fmt "
	memcpy(waveHeader.m_szSubChunk1ID,"fmt ",4);
	waveHeader.m_nSubChunk1Size = 16;
	waveHeader.m_nAudioFormat = 1;
	waveHeader.m_nNumChannels = nNumChannels;
	waveHeader.m_nSampleRate = nSampleRate;
	waveHeader.m_nByteRate = nSampleRate * nNumChannels * nBitsPerSample / 8;
	waveHeader.m_nBlockAlign = nNumChannels * nBitsPerSample / 8;
	waveHeader.m_nBitsPerSample = nBitsPerSample;

	//fill out sub chunk 2 "data"
	memcpy(waveHeader.m_szSubChunk2ID,"data",4);
	waveHeader.m_nSubChunk2Size = nDataSize;

	//write the header
	fwrite(&waveHeader,sizeof(SMinimalWaveFileHeader),1,File);

	//write the wave data itself
	fwrite(pData,nDataSize,1,File);

	//close the file and return success
	fclose(File);
	return true;
}