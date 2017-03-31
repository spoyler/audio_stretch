#include "_stretch.h"

#include "RunParameters.h"
#include "WavFile.h"
#include "SoundTouch.h"

using namespace soundtouch;

static RunParameters *params;
static SoundTouch *pSoundTouch;

static unsigned char _en; //надобность корректора

static RunParameters _RunParameters(0);

static void setup(SoundTouch *pSoundTouch, const RunParameters *params)
{
    int sample_rate = 22050;
    int channels = 1;
	float new_tempo = 2;

    pSoundTouch->setChannels(channels);
	pSoundTouch->setTempo(new_tempo);
	pSoundTouch->setSampleRate(sample_rate);

    pSoundTouch->setSetting(SETTING_USE_QUICKSEEK, params->quick);
}

void _stretch_init(float newTempo)
{
	pSoundTouch = new SoundTouch();
	params = &_RunParameters;

	setup(pSoundTouch, params);
  
    _en = 0x01; 
  
	//delete   pSoundTouch;
}

void _stretch_putSamples(const short *samples, unsigned long numSamples)
{
   pSoundTouch->putSamples(samples, numSamples);
}

unsigned long _stretch_getSamples(short *samples, unsigned int numSamples)
{
    return pSoundTouch->receiveSamples(samples, numSamples);
}

void _stretch_en(unsigned char _p)
{
	_en = _p;
}

unsigned char _stretch_Is_en(void)
{
	return _en;
}