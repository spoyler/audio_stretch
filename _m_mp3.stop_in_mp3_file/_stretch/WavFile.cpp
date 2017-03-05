////////////////////////////////////////////////////////////////////////////////
///
/// Classes for easy reading & writing of WAV sound files.
///
/// For big-endian CPU, define _BIG_ENDIAN_ during compile-time to correctly
/// parse the WAV files with such processors.
///
/// Admittingly, more complete WAV reader routines may exist in public domain,
/// but the reason for 'yet another' one is that those generic WAV reader
/// libraries are exhaustingly large and cumbersome! Wanted to have something
/// simpler here, i.e. something that's not already larger than rest of the
/// SoundTouch/SoundStretch program...
///
/// Author        : Copyright (c) Olli Parviainen
/// Author e-mail : oparviai 'at' iki.fi
/// SoundTouch WWW: http://www.surina.net/soundtouch
///
////////////////////////////////////////////////////////////////////////////////
//
// Last changed  : $Date: 2006/02/05 16:44:06 $
// File revision : $Revision: 1.15 $
//
// $Id: WavFile.cpp,v 1.15 2006/02/05 16:44:06 Olli Exp $
//
////////////////////////////////////////////////////////////////////////////////
//
// License :
//
//  SoundTouch audio processing library
//  Copyright (c) Olli Parviainen
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdexcept>
#include <string>
#include <assert.h>
#include <limits.h>

#include "string.h"

#include "WavFile.h"


const static char riffStr[] = "RIFF";
const static char waveStr[] = "WAVE";
const static char fmtStr[]  = "fmt ";
const static char dataStr[] = "data";


//////////////////////////////////////////////////////////////////////////////
//
// Helper functions for swapping byte order to correctly read/write WAV files
// with big-endian CPU's: Define compile-time definition _BIG_ENDIAN_ to
// turn-on the conversion if it appears necessary.
//
// For example, Intel x86 is little-endian and doesn't require conversion,
// while PowerPC of Mac's and many other RISC cpu's are big-endian.

#ifdef BYTE_ORDER
    // In gcc compiler detect the byte order automatically
    #if BYTE_ORDER == BIG_ENDIAN
        // big-endian platform.
        #define _BIG_ENDIAN_
    #endif
#endif

#ifdef _BIG_ENDIAN_
    // big-endian CPU, swap bytes in 16 & 32 bit words

    // helper-function to swap byte-order of 32bit integer
    static inline void _swap32(unsigned int &dwData)
    {
        dwData = ((dwData >> 24) & 0x000000FF) |
                 ((dwData >> 8)  & 0x0000FF00) |
                 ((dwData << 8)  & 0x00FF0000) |
                 ((dwData << 24) & 0xFF000000);
    }

    // helper-function to swap byte-order of 16bit integer
    static inline void _swap16(unsigned short &wData)
    {
        wData = ((wData >> 8) & 0x00FF) |
                ((wData << 8) & 0xFF00);
    }

    // helper-function to swap byte-order of buffer of 16bit integers
    static inline void _swap16Buffer(unsigned short *pData, unsigned int dwNumWords)
    {
        unsigned long i;

        for (i = 0; i < dwNumWords; i ++)
        {
            _swap16(pData[i]);
        }
    }

#else   // BIG_ENDIAN
    // little-endian CPU, WAV file is ok as such

    // dummy helper-function
    static inline void _swap32(unsigned int &dwData)
    {
        // do nothing
    }

    // dummy helper-function
    static inline void _swap16(unsigned short &wData)
    {
        // do nothing
    }

    // dummy helper-function
    static inline void _swap16Buffer(unsigned short *pData, unsigned int dwNumBytes)
    {
        // do nothing
    }

#endif  // BIG_ENDIAN


//////////////////////////////////////////////////////////////////////////////
//
// Class WavInFile
//

WavInFile::WavInFile(const char *fileName)
{
    int hdrsOk;


    // Try to open the file for reading
    fptr = NULL;

	header.format.format_len = 16;
	header.format.fixed= 1;
	header.format.channel_number = 1;
	header.format.sample_rate = 22050;
	header.format.byte_rate = 44100;
	header.format.byte_per_sample = 2;
	header.format.bits_per_sample = 16;

    dataRead = 0;
}



WavInFile::~WavInFile()
{
    close();
}



void WavInFile::rewind()
{
}


int WavInFile::checkCharTags()
{
 
    return 0;
}


int WavInFile::read(char *buffer, int maxElems)
{

    return 0;
}


int WavInFile::read(short *buffer, int maxElems)
{

    return 0;
}



int WavInFile::read(float *buffer, int maxElems)
{

    return 0;
}


int WavInFile::eof() const
{
    // return true if all data has been read or file eof has reached
    return 0;
}


void WavInFile::close()
{
    fptr = NULL;
}



// test if character code is between a white space ' ' and little 'z'
static int isAlpha(char c)
{
    return (c >= ' ' && c <= 'z') ? 1 : 0;
}


// test if all characters are between a white space ' ' and little 'z'
static int isAlphaStr(char *str)
{
    int c;

    c = str[0];
    while (c)
    {
        if (isAlpha(c) == 0) return 0;
        str ++;
        c = str[0];
    }

    return 1;
}


int WavInFile::readRIFFBlock()
{

    return 0;
}




int WavInFile::readHeaderBlock()
{
    return 0;
}


int WavInFile::readWavHeaders()
{

    return 0;
}


uint WavInFile::getNumChannels() const
{
    return header.format.channel_number;
}


uint WavInFile::getNumBits() const
{
    return header.format.bits_per_sample;
}


uint WavInFile::getBytesPerSample() const
{
    return getNumChannels() * getNumBits() / 8;
}


uint WavInFile::getSampleRate() const
{
    return header.format.sample_rate;
}



uint WavInFile::getDataSizeInBytes() const
{
    return header.data.data_len;
}


uint WavInFile::getNumSamples() const
{
    return header.data.data_len / header.format.byte_per_sample;
}


uint WavInFile::getLengthMS() const
{
   uint numSamples;
   uint sampleRate;

   numSamples = getNumSamples();
   sampleRate = getSampleRate();

   assert(numSamples < UINT_MAX / 1000);
   return (1000 * numSamples / sampleRate);
}


//////////////////////////////////////////////////////////////////////////////
//
// Class WavOutFile
//

WavOutFile::WavOutFile(const char *fileName, int sampleRate, int bits, int channels)
{
   bytesWritten = 0;

   fptr =NULL;

    header.format.format_len = 16;
	header.format.fixed= 1;
	header.format.channel_number = 1;
	header.format.sample_rate = 22050;
	header.format.byte_rate = 44100;
	header.format.byte_per_sample = 2;
	header.format.bits_per_sample = 16;


}



WavOutFile::~WavOutFile()
{
    close();
}



void WavOutFile::fillInHeader(uint sampleRate, uint bits, uint channels)
{
}


void WavOutFile::finishHeader()
{

}



void WavOutFile::writeHeader()
{
}



void WavOutFile::close()
{
    fptr = NULL;
}


void WavOutFile::write(const char *buffer, int numElems)
{
}

void WavOutFile::write(const short *buffer, int numElems)
{
}


void WavOutFile::write(const float*buffer, int numElems)
{
}
