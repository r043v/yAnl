#ifndef __MODPLAY_H__
#define __MODPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"

#define MAX_VOICES  32

typedef struct MOD_INSTR
  {
    char name[23];    /* 000..021 */
    u32 length;       /* 022..023 */
    u8 finetune;      /* 024 */
    u8 volume;        /* 025 */
    u32 loop_start;   /* 026..027 */
    u32 loop_end;     /* 028..029 */
    u32 loop_length;
    BOOL looped;
    s8 * data;
  } MOD_INSTR;

typedef struct MOD
  {
    BOOL loaded;
    char name[21];
    MOD_INSTR instrument[31];
    int num_patterns;
    u8 song_length;
    u8 ciaa;
    u8 song[128];
    char id[4];
    u8 * patterndata;
    int num_instr;
    int num_voices;   /* Number of voices in the MOD */
    int num_channels; /* Number of channels to actually mix (num_channels-num_voices = number of sfx channels) */
    u8 * mixingbuf;
    int mixingbuflen;
    int shiftval;     /* Number of bits to lshift every mixed 16bit word by */
    /* Player variables */
    BOOL channel_active[MAX_VOICES];
    int patterndelay;
    int speed;
    int bpm;
    int songpos;      /* In the song */
    int patternline;  /* In the pattern */
    int patternline_jumpto;     /* For the E6 effect */
    int patternline_jumpcount;  /* For the E6 effect */
    int speedcounter;
    int freq;
    int bits;
    int channels;     /* 1 = mono, 2 = stereo */
    u32 playpos[MAX_VOICES];   /* Playing position for each channel */
    u8 instnum[MAX_VOICES];    /* Current instrument */
    u16 chanfreq[MAX_VOICES];  /* Current frequency */
    u16 channote[MAX_VOICES];  /* Last note triggered */
    u8 volume[MAX_VOICES];     /* Current volume */
    u8 effect[MAX_VOICES];     /* Current effect */
    u8 effectop[MAX_VOICES];   /* Current effect operand */
    u8 last_effect[MAX_VOICES];
    /* Effects handling */
    u16 portamento_to[MAX_VOICES];
    u8 porta_speed[MAX_VOICES];
    u8 retrigger_counter[MAX_VOICES];
    u8 arp_counter;
    u8 sintabpos[MAX_VOICES];
    u8 vib_freq[MAX_VOICES];
    u8 vib_depth[MAX_VOICES];
    u16 vib_basefreq[MAX_VOICES];
    u8 trem_basevol[MAX_VOICES];
    u8 trem_freq[MAX_VOICES];
    u8 trem_depth[MAX_VOICES];
    BOOL glissando[MAX_VOICES];
    u8 trem_wave[MAX_VOICES];
    u8 vib_wave[MAX_VOICES];

    u8 nextinstr[MAX_VOICES];
    u16 nextnote[MAX_VOICES];
        
    unsigned int samplespertick;
    unsigned int samplescounter;
    
    u8 * modraw;

    u32 notebeats;
    void (*callback)(void*);
    
  } MOD;

int MOD_SetMOD ( MOD *, u8 * );
int MOD_Load ( MOD *, char * );
void MOD_Free ( MOD * );
void MOD_Start ( MOD * );
u32 MOD_Player ( MOD * );
int MOD_TriggerNote ( MOD *, int, u8, u16, u8 );
int MOD_AllocSFXChannels ( MOD *, int );

u16 getNote ( MOD *, int, int );
u8 getInstr ( MOD *, int, int );
u8 getEffect ( MOD *, int, int );
u8 getEffectOp ( MOD *, int, int );

#ifdef __cplusplus
  }
#endif

#endif
