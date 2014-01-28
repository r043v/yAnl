#ifndef __MODPLAYPP_H__
#define __MODPLAYPP_H__

#include "gpmodplay.h"

class CMODPlay
  {
    private:
      MODPlay * modplay;
      u8 * getNoteData(int pattern, int row, int chan);
      int numSFXChannels;
    public:
      CMODPlay(void);
      ~CMODPlay(void);
      int triggerNote ( int, u8, u16, u8 );
      int allocSFXChannels ( int );
      int getNumSFXChannels (void);
      int load(char * fname);
      void unload(void);
      int start(void);
      int stop(void);
      int setMOD(u8 * mem); /* Plays the MOD from the memory */
      int pause(BOOL p);
      BOOL isPaused(void);
      BOOL isPlaying(void);
      void setStereo ( BOOL );
      BOOL isStereo(void);
      int setPlayFreq ( int );
      int getPlayFreq(void);
      MOD * getMODStruct(void);
      MODPlay * getMODPlayStruct(void);
      
      int getSpeed(void);
      int getBPM(void);
      int getPatternLine(void);
      int getPatternNum(void);
      int getSongPos(void);
      
      int getNoteFreq ( int pattern, int row, int chan );
      int getEffect ( int pattern, int row, int chan );
      int getEffectOp ( int pattern, int row, int chan );
      int getInstrument ( int pattern, int row, int chan );
      
      int getNoteFreq ( int chan );
      int getEffect ( int chan );
      int getEffectOp ( int chan );
      int getInstrument ( int chan );
  };

#endif
