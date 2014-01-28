#ifndef __GPMODPLAY_H__
#define __GPMODPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "gpmm.h"
#include "modplay.h"

typedef struct MODSOUNDBBUF
  {
    PCM_SR freq;              /* Taken from gpmm.h */
    PCM_BIT format;           /* Taken from gpmm.h */
    unsigned int samples;     /* Buffer length (in samples) */
    void * userdata;          /* Userdata which gets passed to the callback function */
    void (*callback)(         /* Callback function (just like in SDL) */
          void * userdata,    /* GPSOUNDBUF.userdata */
          u8 * stream,        /* Pointer to the buffer which needs to be refilled */
          int len);           /* Length of the buffer in bytes */
    unsigned int pollfreq;    /* Frequency of the timer interrupt which polls the playing position
                               * recommended value: 2*(playingfreq in Hz/GPSOUNDBUF.samples) */
    unsigned int samplesize;  /* Size of one sample (8bit mono->1, 16bit stereo->4) - don't touch this */
  } MODSOUNDBUF;

typedef struct MODPlay
	{
		MOD mod;
		
		BOOL playing;
		int playfreq;
		BOOL bits;
		BOOL stereo;
		BOOL paused;
		int numSFXChannels;
		MODSOUNDBUF soundBuf;
	} MODPlay;

void MODPlay_Init ( MODPlay * );
int  MODPlay_AllocSFXChannels ( MODPlay*, int );
int  MODPlay_Load ( MODPlay*, char * fname );
void MODPlay_Unload ( MODPlay* );
int  MODPlay_Start ( MODPlay* );
int  MODPlay_Stop ( MODPlay* );
int  MODPlay_SetMOD ( MODPlay*, u8* );
int  MODPlay_Pause ( MODPlay*, BOOL );
void MODPlay_SetStereo ( MODPlay*, BOOL );
int  MODPlay_SetPlayFreq ( MODPlay*, int );
int  MODPlay_TriggerNote ( MODPlay*, int channel, u8 instnum, u16 freq, u8 vol );

#ifdef __cplusplus
	}
#endif

#endif
