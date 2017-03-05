/**
	Драйвер audio кодека MAX98089

	STATUS
	MASTER CLOCK CONTROL
	DAI1 CLOCK CONTROL
	DAI1 CONFIGURATION
	DAI2 CLOCK CONTROL
	DAI2 CONFIGURATION
	SRC
	MIXERS
	LEVEL CONTROL
	MICROPHONE AGC
	SPEAKER SIGNAL PROCESSING
	CONFIGURATION
	POWER MANAGEMENT 
	DSP COEFFICIENTS DAI1
	DSP COEFFICIENTS DAI2
	REVISION ID
**/

#ifndef __MAX98089_H
#define __MAX98089_H

#include "lpc_types.h"

// registers STATUS 0x00-0x0f
#define	_ADDRESS_STATUS	0x00

#define M98088_DAI1L_TO_DACL            (1<<7)
#define M98088_DAI1R_TO_DACL            (1<<6)
#define M98088_DAI2L_TO_DACL            (1<<5)
#define M98088_DAI2R_TO_DACL            (1<<4)
#define M98088_DAI1L_TO_DACR            (1<<3)
#define M98088_DAI1R_TO_DACR            (1<<2)
#define M98088_DAI2L_TO_DACR            (1<<1)
#define M98088_DAI2R_TO_DACR            (1<<0)

#define M98088_HPLEN                    (1<<7)
#define M98088_HPREN                    (1<<6)
#define M98088_HPEN                     ((1<<7)|(1<<6))
#define M98088_SPLEN                    (1<<5)
#define M98088_SPREN                    (1<<4)
#define M98088_RECEN                    (1<<3)
#define M98088_DALEN                    (1<<1)
#define M98088_DAREN                    (1<<0)

#define M98088_S1NORMAL                 (1<<6)
#define M98088_S2NORMAL                 (2<<6)
#define M98088_SDATA                    (3<<0)

typedef struct 
{
  union REGSTATUS{
     struct {
         unsigned char RES0:1;
         unsigned char JDET:1;
         unsigned char RES24:3;
         unsigned char ULK:1;
         unsigned char SLD:1;
         unsigned char CLD:1;
     }bit_reg;
     unsigned char char_reg;
  } regSTATUS;
  union REGMicrophoneAGC_NG{
     struct {
         unsigned char AGC:5;
         unsigned char NG:3;
     }bit_reg;
     unsigned char char_reg;
  } regMicrophoneAGC_NG;
  union REGJackStatus{
     struct {
         unsigned char RES05:6;
         unsigned char JKSNS:2;
     }bit_reg;
     unsigned char char_reg;
  } regJackStatus;
  union REGBatteryVoltage{
     struct {
         unsigned char VBAT:5;
         unsigned char RES57:3;
     }bit_reg;
     unsigned char char_reg;
  } regBatteryVoltage;
  union REGInterruptEnable{
     struct {
         unsigned char RES0:1;
         unsigned char IJDET:1;
         unsigned char RES24:3;
         unsigned char IULK:1;
         unsigned char ISLD:1;
         unsigned char ICLD:1;
     }bit_reg;
     unsigned char char_reg;
  } regInterruptEnable;
} _MAX98_STATUS;

// register MASTER CLOCK CONTROL 0x10
#define	_ADDRESS_MASTER_CLOCK_CONTROL	0x10
typedef struct 
{
  union REGMasterClock{
     struct {
         unsigned char RES04:4;
         unsigned char PSCLK:2;
         unsigned char RES67:2;
     }bit_reg;
     unsigned char char_reg;
  } regMasterClock;
} _MAX98_MASTER_CLOCK_CONTROL;

// registers DAI1 CLOCK CONTROL  0x11-0x13
#define	_ADDRESS_DAI1_CLOCK_CONTROL	0x11
typedef union REGAnyClockControl{
	struct {
		unsigned char NI0 : 1;
		unsigned char NI17 : 7;
		unsigned char NI14_8 : 7;
		unsigned char PLL : 1;
	}bit_reg;
	unsigned short short_reg;
} regAnyClockControl;

typedef struct
{
  union {
     struct {
         unsigned char FREQ:4;
         unsigned char SR:4;
     }bit_reg;
     unsigned char char_reg;
  } regClockMode;
  regAnyClockControl AnyClockControl;
} _MAX98_DAI1_CLOCK_CONTROL;

// registers DAI1 CONFIGURATION 0x14-0x18
#define	_ADDRESS_DAI1_CONFIGURATION	0x14
typedef union REGFormat{
     struct {
         unsigned char WS:1;
         unsigned char FSW:1;
         unsigned char TDM:1;
         unsigned char RES3:1;
         unsigned char DLY:1;
         unsigned char BCI:1;
         unsigned char WCI:1;
         unsigned char MAS:1;
     }bit_reg;
     unsigned char char_reg;
  } regFormat;
typedef union REGIOConfiguration{
     struct {
         unsigned char SDIEN:1;
         unsigned char SDOEN:1;
         unsigned char HIZOFF:1;
         unsigned char DMONO:1;
         unsigned char LBEN:1;
         unsigned char LTEN:1;
         unsigned char SEL:2;
     }bit_reg;
     unsigned char char_reg;
  } regIOConfiguration;
typedef union REGTimeDivisionMultiplex{
     struct {
         unsigned char SLOTDLY:4;
         unsigned char SLOTR:2;
         unsigned char SLOTL:2;
     }bit_reg;
     unsigned char char_reg;
  } regTimeDivisionMultiplex;
typedef struct 
{
  regFormat Format;
  union {
     struct {
         unsigned char BSEL:3;
         unsigned char RES34:2;
         unsigned char DAC_ORS:1;
         unsigned char ADC_OSR:2;
     }bit_reg;
     unsigned char char_reg;
  } regClock;
  regIOConfiguration IOConfiguration;
  regTimeDivisionMultiplex TimeDivisionMultiplex;
  union {
     struct {
         unsigned char DVFLT:3;
         unsigned char DHF:1;
         unsigned char AVFLT:3;
         unsigned char MODE:1;
     }bit_reg;
     unsigned char char_reg;
  } regFilters;
} _MAX98_DAI1_CONFIGURATION;

// registers DAI2 CLOCK CONTROL  0x19-0x1b
#define	_ADDRESS_DAI2_CLOCK_CONTROL	0x19
typedef struct 
{
  union {
     struct {
         unsigned char RES03:4;
         unsigned char SR:4;
     }bit_reg;
     unsigned char char_reg;
  } regClockMode;
  regAnyClockControl AnyClockControl;
} _MAX98_DAI2_CLOCK_CONTROL;

// registers DAI2 CONFIGURATION  0x1c-0x20
#define	_ADDRESS_DAI2_CONFIGURATION	0x1c
typedef struct 
{
  regFormat Format;
  union {
     struct {
         unsigned char BSEL:3;
         unsigned char RES34:2;
         unsigned char DAC_ORS:1;
         unsigned char RES67:2;
     }bit_reg;
     unsigned char char_reg;
  } regClock;
  regIOConfiguration IOConfiguration;
  regTimeDivisionMultiplex TimeDivisionMultiplex;
  union {
     struct {
         unsigned char DCB:1;
         unsigned char RES12:2;	
         unsigned char DHF:1;
         unsigned char RES47:4;
     }bit_reg;
     unsigned char char_reg;
  } regFilters;
} _MAX98_DAI2_CONFIGURATION;

// register SRC 0x21
#define	_ADDRESS_SRC	0x21
typedef struct 
{
  union REGSampleRateConverter{
     struct {
         unsigned char SRC_ENR:1;
         unsigned char SRC_ENL:1;
         unsigned char SRMIX_ENR:1;
         unsigned char SRMIX_ENL:1;
         unsigned char SRMIX_MODE:1;
         unsigned char RES57:3;	 
     }bit_reg;
     unsigned char char_reg;
  } regSampleRateConverter;
} _MAX98_SRC;

// register MIXERS 0x22-0x2d
#define	_ADDRESS_MIXERS	0x22
typedef struct 
{
  union REGDACMixer{
     struct {
         unsigned char MIXDAR:4;
         unsigned char MIXDAL:4;
     }bit_reg;
     unsigned char char_reg;
  } regDACMixer;
  union REGLeftADCMixer{
     struct {
         unsigned char MIXADL:8;
     }bit_reg;
     unsigned char char_reg;
  } regLeftADCMixer;
  union REGRightADCMixer{
     struct {
         unsigned char MIXADR:8;
     }bit_reg;
     unsigned char char_reg;
  } regRightADCMixer;
  union REGLeftHeadphoneAmplifierMixer{
     struct {
         unsigned char MIXHPL:8;
     }bit_reg;
     unsigned char char_reg;
  } regLeftHeadphoneAmplifierMixer;
  union REGRightHeadphoneAmplifierMixer{
     struct {
         unsigned char MIXHPR:8;
     }bit_reg;
     unsigned char char_reg;
  } regRighttHeadphoneAmplifierMixer;
  union REGHeadphoneAmplifierMixerControl{
     struct {
         unsigned char MIXHPL_GAIN:2;
         unsigned char MIXHPR_GAIN:2;
         unsigned char MIXHPL_PATHSEL:1;
         unsigned char MIXHPR_PATHSEL:1;
         unsigned char RES67:2;
     }bit_reg;
     unsigned char char_reg;
  } regHeadphoneAmplifierMixerControl;
  union REGLeftReceiverAmplifierMixer{
     struct {
         unsigned char MIXRECL:8;
     }bit_reg;
     unsigned char char_reg;
  } regLeftReceiverAmplifierMixer;
  union REGRightReceiverAmplifierMixer{
     struct {
         unsigned char MIXRECR:8;
     }bit_reg;
     unsigned char char_reg;
  } regRightReceiverAmplifierMixer;
  union REGReceiverAmplifierMixerControl{
     struct {
         unsigned char MIXRECL_GAIN:2;
         unsigned char MIXRECR_GAIN:2;
         unsigned char RES46:3;
         unsigned char LINE_MODE:1;
     }bit_reg;
     unsigned char char_reg;
  } regReceiverAmplifierMixerControl;
  union REGLeftSpeakerAmplifierMixer{
     struct {
         unsigned char MIXSPL:8;
     }bit_reg;
     unsigned char char_reg;
  } regLeftSpeakerAmplifierMixer;
  union REGRightSpeakerAmplifierMixer{
     struct {
         unsigned char MIXSPR:8;
     }bit_reg;
     unsigned char char_reg;
  } regRightSpeakerAmplifierMixer;
  union REGSpeakerAmplifierMixerControl{
     struct {
         unsigned char MIXSPL_GAIN:2;
         unsigned char MIXSPR_GAIN:2;
         unsigned char RES47:4;
     }bit_reg;
     unsigned char char_reg;
  } regSpeakerAmplifierMixerControl;
} _MAX98_MIXERS;

// register LEVEL CONTROL 0x2e-0x3e
#define	_ADDRESS_LEVEL_CONTROL	0x2e
  typedef union REGADCLevel{
     struct {
         unsigned char AV:4;
         unsigned char AVG:2;
         unsigned char RES67:2;
     }bit_reg;
     unsigned char char_reg;
  } regADCLevel;
  typedef union REGMicrophoneInputLevel{
     struct {
         unsigned char PGAM:5;
         unsigned char PAEN:2;
         unsigned char RES7:1;
     }bit_reg;
     unsigned char char_reg;
  } regMicrophoneInputLevel;
  typedef union REGINInputLevel{
     struct {
         unsigned char PGAIN:3;
         unsigned char RES35:3;
         unsigned char INEXT:1;
         unsigned char RES7:1;
     }bit_reg;
     unsigned char char_reg;
  } regINInputLevel;
  typedef union REGHeadphoneAmplifierVolumeControl{
     struct {
         unsigned char HPVOL:5;
         unsigned char RES56:2;
         unsigned char HPM:1;
     }bit_reg;
     unsigned char char_reg;
  } regHeadphoneAmplifierVolumeControl;
  typedef union REGReceiverAmplifierVolumeControl{
     struct {
         unsigned char RECVOL:5;
         unsigned char RES56:2;
         unsigned char RECM:1;
     }bit_reg;
     unsigned char char_reg;
  } regReceiverAmplifierVolumeControl;
  typedef union REGSpeakerAmplifierVolumeControl{
     struct {
         unsigned char SPVOL:5;
         unsigned char RES56:2;
         unsigned char SPM:1;
     }bit_reg;
     unsigned char char_reg;
  } regSpeakerAmplifierVolumeControl;
typedef struct 
{
  union REGSidetone{
     struct {
         unsigned char DVST:5;
         unsigned char RES5:1;
         unsigned char DSTS:2;
     }bit_reg;
     unsigned char char_reg;
  } regSidetone;
  union REGDAI1PlaybackLevel{
     struct {
         unsigned char DVEQ:4;
         unsigned char EQCLP:1;
         unsigned char RES1215:3;
         unsigned char DV:4;
         unsigned char DVG:2;
         unsigned char RES6:1;
         unsigned char DVM:1;
     }bit_reg;
     unsigned short short_reg;
  } regDAI1PlaybackLevel;
  union REGDAI2PlaybackLevel{
     struct {
         unsigned char DVEQ:4;
         unsigned char EQCLP:1;
         unsigned char RES1215:3;
         unsigned char DV:4;
         unsigned char RES46:3;
         unsigned char DVM:1;
     }bit_reg;
     unsigned short short_reg;
  } regDAI2PlaybackLevel;
  regADCLevel LeftADCLevel;
  regADCLevel RightADCLevel;
  regMicrophoneInputLevel Microphone1InputLevel;
  regMicrophoneInputLevel Microphone2InputLevel;
  regINInputLevel INAInputLevel;
  regINInputLevel INBInputLevel;
  regHeadphoneAmplifierVolumeControl LeftHeadphoneAmplifierVolumeControl;
  regHeadphoneAmplifierVolumeControl RightHeadphoneAmplifierVolumeControl;
  regReceiverAmplifierVolumeControl LeftReceiverAmplifierVolumeControl;
  regReceiverAmplifierVolumeControl RightReceiverAmplifierVolumeControl;
  regSpeakerAmplifierVolumeControl LeftSpeakerAmplifierVolumeControl;
  regSpeakerAmplifierVolumeControl RightSpeakerAmplifierVolumeControl;
} _MAX98_LEVEL_CONTROL;

// register MICROPHONE AGC 0x3f-0x40
#define	_ADDRESS_MICROPHONE_AGC	0x3f
typedef struct 
{
  union REGConfiguration{
     struct {
         unsigned char AGCHLD:2;
         unsigned char AGCATK:2;
         unsigned char AGCRLS:3;
         unsigned char AGCSRC:1;
     }bit_reg;
     unsigned char char_reg;
  } regConfiguration;
  union REGThreshold{
     struct {
         unsigned char AGCTH:4;
         unsigned char ANTH:4;
     }bit_reg;
     unsigned char char_reg;
  } regThreshold;
} _MAX98_MICROPHONE_AGC;

// register SPEAKER SIGNAL PROCESSING 	0x41-0x46
#define	_ADDRESS_SPEAKER_SIGNAL_PROCESSING	0x41
typedef struct 
{
  union REGExcursionLimiterFilter{
     struct {
         unsigned char DHPLCF:2;
         unsigned char RES23:2;
         unsigned char DHPUCF:3;
         unsigned char RES7:1;
     }bit_reg;
     unsigned char char_reg;
  } regExcursionLimiterFilter;
  union REGExcursionLimiterThreshold{
     struct {
         unsigned char DHPTH:3;
         unsigned char RES37:5;
     }bit_reg;
     unsigned char char_reg;
  } regExcursionLimiterThreshold;
  union REGALC{
     struct {
         unsigned char ALCTH:3;
         unsigned char ALCMB:1;
         unsigned char ALCRLS:3;
         unsigned char ALCEN:1;
     }bit_reg;
     unsigned char char_reg;
  } regALC;
  union REGPowerLimiter{
     struct {
         unsigned char PWRT1:4;
         unsigned char PWRT2:4;
         unsigned char PWRK:3;
         unsigned char RES3:1;
         unsigned char PWRTH:4;
     }bit_reg;
     unsigned short short_reg;
  } regPowerLimiter;
  union REGDistortionLimiter{
     struct {
         unsigned char THDT1:1;
         unsigned char RES13:3;
         unsigned char THDCLP:4;
     }bit_reg;
     unsigned char char_reg;
  } regDistortionLimiter;
} _MAX98_SPEAKER_SIGNAL_PROCESSING;

// register CONFIGURATION 	0x47-0x4b
#define	_ADDRESS_CONFIGURATION	0x47
typedef struct 
{
  union REGAudioInput{
     struct {
         unsigned char RES05:6;
         unsigned char INBDIFF:1;
         unsigned char INADIFF:1;
     }bit_reg;
     unsigned char char_reg;
  } regAudioInput;
  union REGMicrophone{
     struct {
         unsigned char EXTMIC:2;
         unsigned char RES23:2;
         unsigned char DIGMICR:1;
         unsigned char DIGMICL:1;
         unsigned char MICCLK:2;
     }bit_reg;
     unsigned char char_reg;
  } regMicrophone;
  union REGLevelControl{
     struct {
         unsigned char EQ1EN:1;
         unsigned char EQ2EN:1;
         unsigned char RES25:3;
         unsigned char ZDEN:1;
         unsigned char VSEN:1;
         unsigned char VS2EN:1;
     }bit_reg;
     unsigned char char_reg;
  } regLevelControl;
  union REGBypassSwitches{
     struct {
         unsigned char SPKBYP:1;
         unsigned char RECBYP:1;
         unsigned char RES23:2;
         unsigned char MIC2BYP:1;
         unsigned char RES56:2;
         unsigned char INABYP:1;
     }bit_reg;
     unsigned char char_reg;
  } regBypassSwitches;
  union REGJackDetection{
     struct {
         unsigned char JDEB:2;
         unsigned char RES26:5;
         unsigned char JDETEN:1;
     }bit_reg;
     unsigned char char_reg;
  } regJackDetection;
} _MAX98_CONFIGURATION;

// register POWER MANAGEMENT 	0x4c-0x51
#define	_ADDRESS_POWER_MANAGEMENT	0x4c
typedef struct 
{
  union REGInputEnable{
     struct {
         unsigned char ADREN:1;
         unsigned char ADLEN:1;
         unsigned char RES2:1;
         unsigned char MBEN:1;
         unsigned char RES45:2;
         unsigned char INBEN:1;
         unsigned char INAEN:1;
     }bit_reg;
     unsigned char char_reg;
  } regInputEnable;
  union REGOutputEnable{
     struct {
         unsigned char DAREN:1;
         unsigned char DALEN:1;
         unsigned char RECREN:1;
         unsigned char RECLEN:1;
         unsigned char SPREN:1;
         unsigned char SPLEN:1;
         unsigned char HPREN:1;
         unsigned char HPLEN:1;
     }bit_reg;
     unsigned char char_reg;
  } regOutputEnable;
  union REGTopLevelBiasControl{
     struct {
         unsigned char JDWK:1;
         unsigned char RES13:3;
         unsigned char BIASEN:1;
         unsigned char VCMEN:1;
         unsigned char SPREGEN:1;
         unsigned char BGEN:1;
     }bit_reg;
     unsigned char char_reg;
  } regTopLevelBiasControl;
  union REGDACLowPowerMode1{
     struct {
         unsigned char DAI1_DAC_LP:4;
         unsigned char DAI2_DAC_LP:4;
     }bit_reg;
     unsigned char char_reg;
  } regDACLowPowerMode1;
  union REGDACLowPowerMode2{
     struct {
         unsigned char CGM1_EN:1;
         unsigned char CGM2_EN:1;
         unsigned char DAC1_IP_DITH_EN:1;
         unsigned char DAC2_IP_DITH_EN:1;
         unsigned char RES47:4;
     }bit_reg;
     unsigned char char_reg;
  } regDACLowPowerMode2;
  union REGSystemShutdown{
     struct {
         unsigned char PWRSV:1;
         unsigned char PWRSV8K:1;
         unsigned char HPPLYBACK:1;
         unsigned char PERFMODE:1;
         unsigned char RES45:2;
         unsigned char VBATEN:1;
         unsigned char SHDN:1;
     }bit_reg;
     unsigned char char_reg;
  } regSystemShutdown;
} _MAX98_POWER_MANAGEMENT;

// register DSP COEFFICIENTS 	1 0x52-0x83,0xb6-0xbf  /  2 0x84-0xb5,0xc0-0xc9
#define	_ADDRESS_DSP_COEFFICIENTS1	0x52
#define	_ADDRESS_DSP_COEFFICIENTS2	0x84
typedef struct 
{
   struct {
      unsigned short K_1:16;
      unsigned short K1_1:16;
      unsigned short K2_1:16;
      unsigned short C1_1:16;
      unsigned short C2_1:16;
   }bandEQ1;
   struct {
      unsigned short K_2:16;
      unsigned short K1_2:16;
      unsigned short K2_2:16;
      unsigned short C1_2:16;
      unsigned short C2_2:16;
   }bandEQ2;
   struct {
      unsigned short K_3:16;
      unsigned short K1_3:16;
      unsigned short K2_3:16;
      unsigned short C1_3:16;
      unsigned short C2_3:16;
   }bandEQ3;
   struct {
      unsigned short K_4:16;
      unsigned short K1_4:16;
      unsigned short K2_4:16;
      unsigned short C1_4:16;
      unsigned short C2_4:16;
   }bandEQ4;
   struct {
      unsigned short K_5:16;
      unsigned short K1_5:16;
      unsigned short K2_5:16;
      unsigned short C1_5:16;
      unsigned short C2_5:16;
   }bandEQ5;
   struct {
      unsigned short A1:16;
      unsigned short A2:16;
      unsigned short B0:16;
      unsigned short B1:16;
      unsigned short B2:16;
   }bandExcursionLimiterBiquad;
} _MAX98_DSP_COEFFICIENTS;


// register REVISION ID 	0xff
#define	_ADDRESS_REVISION_ID	0xff
typedef struct 
{
  union REGRevID{
     struct {
         unsigned char REV:8;
     }bit_reg;
     unsigned char char_reg;
  } regRevID;
} _MAX98_REVISION_ID;

int Write_STATUS(INT_32 devid,_MAX98_STATUS* data);
int Read_STATUS(INT_32 devid,_MAX98_STATUS* data);
int Write_MASTER_CLOCK_CONTROL(INT_32 devid,_MAX98_MASTER_CLOCK_CONTROL* data);
int Read_MASTER_CLOCK_CONTROL(INT_32 devid,_MAX98_MASTER_CLOCK_CONTROL* data);
int Write_DAI1_CLOCK_CONTROL(INT_32 devid,_MAX98_DAI1_CLOCK_CONTROL* data);
int Read_DAI1_CLOCK_CONTROL(INT_32 devid,_MAX98_DAI1_CLOCK_CONTROL* data);
int Write_DAI1_CONFIGURATION(INT_32 devid,_MAX98_DAI1_CONFIGURATION* data);
int Read_DAI1_CONFIGURATION(INT_32 devid,_MAX98_DAI1_CONFIGURATION* data);
int Write_DAI2_CLOCK_CONTROL(INT_32 devid,_MAX98_DAI2_CLOCK_CONTROL* data);
int Read_DAI2_CLOCK_CONTROL(INT_32 devid,_MAX98_DAI2_CLOCK_CONTROL* data);
int Write_DAI2_CONFIGURATION(INT_32 devid,_MAX98_DAI2_CONFIGURATION* data);
int Read_DAI2_CONFIGURATION(INT_32 devid,_MAX98_DAI2_CONFIGURATION* data);
int Write_SRC(INT_32 devid,_MAX98_SRC* data);
int Read_SRC(INT_32 devid,_MAX98_SRC* data);
int Write_MIXERS(INT_32 devid,_MAX98_MIXERS* data);
int Read_MIXERS(INT_32 devid,_MAX98_MIXERS* data);
int Write_LEVEL_CONTROL(INT_32 devid,_MAX98_LEVEL_CONTROL* data);
int Read_LEVEL_CONTROL(INT_32 devid,_MAX98_LEVEL_CONTROL* data);
int Write_MICROPHONE_AGC(INT_32 devid,_MAX98_MICROPHONE_AGC* data);
int Read_MICROPHONE_AGC(INT_32 devid,_MAX98_MICROPHONE_AGC* data);
int Write_SPEAKER_SIGNAL_PROCESSING(INT_32 devid,_MAX98_SPEAKER_SIGNAL_PROCESSING* data);
int Read_SPEAKER_SIGNAL_PROCESSING(INT_32 devid,_MAX98_SPEAKER_SIGNAL_PROCESSING* data);
int Write_CONFIGURATION(INT_32 devid,_MAX98_CONFIGURATION* data);
int Read_CONFIGURATION(INT_32 devid,_MAX98_CONFIGURATION* data);
int Write_POWER_MANAGEMENT(INT_32 devid,_MAX98_POWER_MANAGEMENT* data);
int Read_POWER_MANAGEMENT(INT_32 devid,_MAX98_POWER_MANAGEMENT* data);
int Write_DSP_COEFFICIENTS1(INT_32 devid,_MAX98_DSP_COEFFICIENTS* data);
int Read_DSP_COEFFICIENTS1(INT_32 devid,_MAX98_DSP_COEFFICIENTS* data);
int Write_DSP_COEFFICIENTS2(INT_32 devid,_MAX98_DSP_COEFFICIENTS* data);
int Read_DSP_COEFFICIENTS2(INT_32 devid,_MAX98_DSP_COEFFICIENTS* data);
int Read_DSP_REVISION_ID(INT_32 devid,_MAX98_REVISION_ID* data);

#endif
