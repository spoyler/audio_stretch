#include "_stretch.h"

#include "RunParameters.h"
#include "WavFile.h"
#include "SoundTouch.h"


static WavInFile *inFile;
static WavOutFile *outFile;
static RunParameters *params;
static SoundTouch *pSoundTouch;
static int nChannels;
static int buffSizeSamples;
static int num;
static unsigned char _en; //надобность корректора

static SoundTouch  _SoundTouch;
static RunParameters _RunParameters(0,NULL);


static WavInFile _WavInFile(NULL);
static WavOutFile _WavOutFile(NULL,0,0,0);
static void openFiles(WavInFile **inFile, WavOutFile **outFile, const RunParameters *params)
{
    int bits, samplerate, channels;
    *inFile = &_WavInFile;
    *outFile = &_WavOutFile;
}

static void setup(SoundTouch *pSoundTouch, const WavInFile *inFile, const RunParameters *params)
{
    int sampleRate;
    int channels;

    channels = inFile->getNumChannels();
    pSoundTouch->setChannels(channels);


    pSoundTouch->setSetting(SETTING_USE_QUICKSEEK, params->quick);
}

void _setTempo(float newTempo)
{
  pSoundTouch->setTempo(newTempo);
}

static float _bnewTempo;
void _stretch_init(float newTempo)
{

  pSoundTouch = &_SoundTouch;
  params = &_RunParameters;
  openFiles(&inFile, &outFile, params);
  setup(pSoundTouch, inFile, params);
  nChannels = inFile->getNumChannels();
  buffSizeSamples = BUFF_SIZE / nChannels;
  _en = 0x01; //не нужен корректор
  _setTempo(newTempo);
  _bnewTempo = newTempo;
  if(newTempo<1.4)
    _en = 0x0;
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
  if(_bnewTempo<1.4)
    return;
  
  _en = _p;
}

unsigned char _stretch_Is_en(void)
{
  return _en;
}