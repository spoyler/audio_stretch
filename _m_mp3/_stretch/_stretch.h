
#ifndef _STRETCH_H
#define _STRETCH_H

#include <stdio.h>

#define BUFF_SIZE           2048

#if defined (__cplusplus)
extern "C"
#endif
void _stretch_init(float newTempo);

#if defined (__cplusplus)
extern "C"
#endif
void _stretch_putSamples(const short *samples, unsigned long numSamples);

#if defined (__cplusplus)
extern "C"
#endif
unsigned long _stretch_getSamples(short *samples, unsigned int numSamples);

#if defined (__cplusplus)
extern "C"
#endif
void _stretch_en(unsigned char _p);

#if defined (__cplusplus)
extern "C"
#endif
unsigned char _stretch_Is_en(void);

#if defined (__cplusplus)
extern "C"
#endif
void _setTempo(float newTempo);

#endif