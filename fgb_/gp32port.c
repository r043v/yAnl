//bugs: fskip impares causan perdida de setcolors en emulacion

#include "defs.h"
#include "regs.h"

#include "lcd.h"

#include "fb.h"
#include "input.h"
#include "rc.h"
#include "pcm.h"
#include "mem.h"
#include "hw.h"
#include "rtc.h"
#include "sound.h"

#include "save.h"

char*romName, romSize ;

#define GAMMA 2.00
//#define GAMMA 0.01

extern void	(*error)(const char * txt) ;

#include "gp32_rl.h"

  int y_refresh_gp32;   //fixes displacement bug in rasterization when resetting video

 menu menugb,menugame,menusave,menuvideo,menudebug;

//-------------------  change default option here !!
 char mReset=0,mGamma=(200-(int)(GAMMA*100))/2;
 char mFSkip=1,mGBPal=0,mDebug=0;
 char mLoadState=0,mSaveState=0,mLoadMovie=0,mSaveMovie=0,mSnapShot=0;
 char mStretch=0,mVSync=0,mBuffering=0,mType;
 char frequency=0, lastfreq=0, menuabout=0 ;
 // ------------------------------------
 
int emu_running;
int framelimit_delay;

void state_save(int n);
void state_load(int n);

#define BLACK_COLOR 255

void init_video(void)
{       GpGraphicModeSet(GPC_DFLAG_8BPP, NULL);
        GpLcdEnable();

        gpDrawSurfaceIndex = 0;

        Picture=(unsigned char *)0x0C7B4000;
        GP32_Video=8;

        SetColor(253,0,115,130);
        SetColor(254,255,255,255);
        SetColor(BLACK_COLOR,0,0,0);
	memset(Picture,0xff,320*240*2); // set framebuffer to black
        //{int i; for(i=0;i<320*240;i++) Picture[i]=Picture[i+320*240]=BLACK_COLOR; }
}

struct fb fb;

byte vidram[320*240];

void vid_preinit(void) {}
void vid_init(void)
{
       fb.w = 166;
       fb.h = 144;
       fb.pelsize = 1;
       fb.pitch = 166;
       fb.ptr = (unsigned char *)&vidram[0]; //0x0C7B4000;
       fb.enabled = 1;
       fb.dirty = 0; ///1????
       fb.yuv = 0;

       fb.indexed = 1;
       fb.cc[0].l = 0; fb.cc[0].r = 0;
       fb.cc[1].l = 0; fb.cc[1].r = 0;
       fb.cc[2].l = 0; fb.cc[2].r = 0;
}
void vid_begin(void)
{
 y_refresh_gp32=0;
}

int tick=0;
int frames_rendered=0,frames_displayed=0; 
char fps_string[64];

void vid_end(void)
{
   static int strangebug=0;

   if((strangebug^=1))
   {
    //old

    {
    int ticktemp=GpTickCountGet()-tick;

    if(ticktemp>ONE_SECOND)
    {
     tick=GpTickCountGet();

     gm_sprintf(fps_string,"\n  ^%04d/%04d/60,%04d%%,%d fskip^",frames_displayed,frames_rendered,((frames_rendered*100)/60)+1,(int)mFSkip); //60 hz?

//auto frameskip:
//    mFSkip=60/(frames_rendered+1);
//    mFSkip=((ticktemp*50)/ONE_SECOND)/(frames_rendered+1); //50=60 teoricos-10 margen error
//    if(frames_rendered>60) if(mFSkip)     mFSkip--; //mFAuto
//    if(frames_rendered<60) if(mFSkip<255) mFSkip++; //mFAuto

     frames_displayed=0;
     frames_rendered=0;
    }
/*
    else
    if(ticktemp>ONE_SECOND/2)
    {
//      mFSkip=30/(frames_rendered+1);
//      mFSkip=((ticktemp*30)/ONE_SECOND)/(frames_rendered+1);

     if(frames_rendered>30) if(mFSkip)     mFSkip--; //mFAuto
     if(frames_rendered<30) if(mFSkip<255) mFSkip++; //mFAuto
    }
*/

    if(mDebug) textOut(fps_string,0,0,254,0);
    }

    fb.enabled=(((frames_rendered++)%(mFSkip+1))==0);
    if(fb.enabled) frames_displayed++;

    if(mBuffering) FlipScreen(0,0);

  if(mVSync){
    if(mVSync==2) { volatile long *lcdcon1 = (long *)0x14a00000; while(1!=(((*lcdcon1)>>18)&0x1ff)); }
	else
    {
    int delay;

    if(mFSkip) delay = (((ONE_SECOND/60)*3)/2) - (GpTickCountGet()-framelimit_delay);
    else delay = (((ONE_SECOND/60)*2)/1) - (GpTickCountGet()-framelimit_delay);

    //updates current frame time : (16743=16.74ms GB frame length)
    if(delay>0) while((GpTickCountGet()-framelimit_delay)<delay);

    framelimit_delay=GpTickCountGet();
    }
  }

  }

   //new auto skip
/*
    fb.enabled=(((frames_rendered++)%(mFSkip+1))==0);

    {
    int ticktemp=GpTickCountGet()-tick;

    if(ticktemp>ONE_SECOND)
    {
     int percent;

     fps=frames_rendered*(1+(int)mFSkip);

     if(frames_rendered>60) if(mFSkip)     mFSkip--;
     if(frames_rendered<60) if(mFSkip<255) mFSkip++;

     frames_rendered=0;
     percent=((fps*100)/60)+1; //avoid 0

     gm_sprintf(fps_string,"\n  ^%03d,%03d%%,%d fskip^",fps,percent,(int)mFSkip); //60 hz?

     tick=GpTickCountGet();
    }
    else
    if(ticktemp>ONE_SECOND/2)
    {
     if(frames_rendered>30) if(mFSkip)     mFSkip--;
     if(frames_rendered<30) if(mFSkip<255) mFSkip++;
    }
   
    if(mDebug) textOut(fps_string,0,0,254,0);
    }
*/
}
void vid_close(void) {}

void vid_settitle(void) {}
int monopal_backup[256];
void vid_setpal(int i, int r, int g, int b)
{
// char s[100]; gm_sprintf(s,"%d color",i);
// DrawMessageC(s); DelayA();
 if (i>=253)  return;
 if (hw.cgb) SetColor(i,r,g,b); 
  else {
       switch(mGBPal)
       {
        case 0: SetColor(i,g/2,g,0);break;   //green , the best for flu-
        case 1: SetColor(i,g*3/4,g,0);break; //lime
        case 2: SetColor(i,g,g,0);break;     //yellow
        case 3: SetColor(i,g,g,g);break;     //gray
        case 4: SetColor(i,0,g*3/4,g);break; //indigo
       }
       monopal_backup[i]=g;
       }
}

void ev_poll(void)
{
 int Key=GpKeyGet();

 hw.pad=0;

 if(Key&GPC_VK_UP)     hw.pad|=PAD_UP;
 else
 if(Key&GPC_VK_DOWN)   hw.pad|=PAD_DOWN;

 if(Key&GPC_VK_LEFT)   hw.pad|=PAD_LEFT;
  else
 if(Key&GPC_VK_RIGHT)  hw.pad|=PAD_RIGHT;

 if(Key&GPC_VK_FA)     hw.pad|=PAD_A;
 if(Key&GPC_VK_FB)     hw.pad|=PAD_B;
 if(Key&GPC_VK_SELECT) hw.pad|=PAD_SELECT;
 if(Key&GPC_VK_START)  hw.pad|=PAD_START;

 pad_refresh();

 if((Key&(GPC_VK_FL|GPC_VK_FR))==(GPC_VK_FL|GPC_VK_FR)) 
 {   unsigned char *buf, *bufp;

     char mPal=mGBPal;

     GpSoundBufStop();

     buf=bufp=gm_malloc(320*240);

     if(buf==NULL) while(1) { DrawMessageC("not mem enough"); FlipScreen(1,0); DrawMessageC("not mem enough"); FlipScreen(1,0); }

     { unsigned char *x=(unsigned char *)0x0c7b4000; int i; for(i=0;i<320*240;i++,x++) (*bufp++)=*(x+320*240)=*x; }

     if(mbc.batt && ram.loaded && mbc.ramsize) { DrawMessageC("Saving SRAM... Please wait"); FlipScreen(1,0); DrawMessageC("Saving SRAM... Please wait"); FlipScreen(1,0); sram_save(); }

     while(MenuDisplay(&menugb)!=GPC_VK_FB)
     {
      Gamma=(2.00f-(mGamma/50.0f))+0.01f;
      SetPalette(gfx_pal_backup);

      if(mPal!=mGBPal) { int i; for(i=0;i<256;i++) vid_setpal(i,monopal_backup[i],monopal_backup[i],monopal_backup[i]); mPal=mGBPal; } //change GB Mono palette layout
     }
/*  if(mSnapShot)
     {
     }
*/
     if(mSaveState&&(!mLoadState))
     {
      DrawMessageC("Saving State..."); FlipScreen(1,0);
      DrawMessageC("Saving State..."); FlipScreen(1,0);
      state_save((int)mSaveState-1);
      mSaveState=0;
     }

     if(mLoadState&&(!mSaveState))
     {
      DrawMessageC("Loading State..."); FlipScreen(1,0);
      DrawMessageC("Loading State..."); FlipScreen(1,0);
      state_load((int)mLoadState-1);
      mLoadState=0;
     }

     if(mLoadState&&mSaveState)
     {
      DrawMessageC("Copying State..."); FlipScreen(1,0);
      DrawMessageC("Copying State..."); FlipScreen(1,0);
      state_load((int)mLoadState-1);
      state_save((int)mSaveState-1);
      mLoadState=mSaveState=0;
     }

	if(frequency!=lastfreq)
	{	switch(frequency)
		{	case 0	:	GpClockSpeedChange(132000000, 0x3a011, 3);	break ;
			case 1	:	GpClockSpeedChange( 80000000, 0x48012, 2 );	break ;
			case 2	:	GpClockSpeedChange (67800000, 0x69032, 3);	break ;
			case 3	:	GpClockSpeedChange( 33000000, 0x24003, 0 );	break ;
		};	lastfreq=frequency ;	vid_init(); pcm_init();
	}

     framelimit_delay=0;

     init_video();
     SetPalette(gfx_pal_backup);

     if(mReset==1) { FUX0R_PAL(); vid_init(); emu_reset(); mReset=0; } //hay q reiniciarlo dos veces pa q las paletas mono se queden ok
     if(mReset==2) { mReset=0; emu_running=0; }
    
     gm_free(buf);

     GpSoundBufStart(&sndbuf);
 }

}

void GetInfo(byte *header)
{
}

rcvar_t vid_exports[] =
{
	RCV_END
};

rcvar_t joy_exports[] =
{
	RCV_END
};

struct pcm pcm;

rcvar_t pcm_exports[] =
{
	RCV_END
};

static void audio_callback(void *blah, byte *stream, int len)
{
       sound_mix();
       memcpy(stream, pcm.buf, len);
}

void pcm_init(void)
{
       pcm.hz = 22050;
       pcm.stereo = 0;
       pcm.len = 22050/60;
       pcm.buf = gm_malloc(pcm.len);
	pcm.pos = 0;
       gm_memset(pcm.buf, 0, pcm.len);

       sndbuf.freq=PCM_M22; 
       sndbuf.format=PCM_8BIT; 
       sndbuf.samples=22050/60;
       sndbuf.userdata=NULL;
       sndbuf.callback=audio_callback;
       sndbuf.pollfreq=100;
}

int pcm_submit()
{
	if (!pcm.buf) return 0;
	if (pcm.pos < pcm.len) return 1;

	pcm.pos = 0;
	return 1;
}

void pcm_close(void)
{
 GpSoundBufStop();
 gm_free(pcm.buf);
}

void *sys_timer() {return 0;}
int   sys_elapsed(void) {return 0;}
void  sys_sleep(int us) {}
void  sys_checkdir(char *path, int wr){}
void  sys_initpath(char *exe){}
void  sys_sanitize(char *s){}

char *strdup(char *s1)
{
 void *t=gm_zi_malloc(gm_lstrlen(s1)+2);
 gm_memcpy(t,s1,gm_lstrlen(s1)+1);
 return ((char *)t);
}

int strcasecmp(char *s1, char *s2)
{
  while ((*s1 != '\0')
  && (tolower(*(unsigned char *)s1) == tolower(*(unsigned char *)s2)))
  {
      s1++;
      s2++;
  }

  return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
}

#include "stdio.h"

extern char *romfile;
byte *RomImage; 

byte *loadfile(FILE *fp, int *len)
{/*
 byte *d ;

 MyGameSize = *len;

 if(MyGameSize>=gm_availablesize()) return (d=((byte *)fp)); //no ram enough, try another solution

 d=(byte *)gm_malloc(*len);

 if(d==NULL)
 { char err[1024];
  init_video();
  sprintf(err,"NO MEMORY ENOUGH!\nrequest %i\nfree %i",*len,gm_availablesize());
  error(err) ;
  //DrawMessageC("NO MEMORY ENOUGH!    ");
  while(1);
 }

 gm_memcpy(d,(byte *)fp,*len);
 return d;*/
 return (byte *)fp;
}

char *saveDir = (char*)"gp:\\GPETC\\GBSAVE\\" ;

extern char * gbgetLegalName(char *strout);

int sram_load()
{      F_HANDLE f; unsigned long temp;
       char s[100];

       if (!mbc.batt) return -1;

	/* Consider sram loaded at this point, even if file doesn't exist */
	ram.loaded = 1;
       gm_sprintf(s,"%s%8s.srm",saveDir,romName);
       if(GpFileOpen(s,OPEN_R,&f)!=SM_OK) return -1;

       if(GpFileRead(f, (void *)&ram.sbank[0], 8192 * mbc.ramsize, &temp)!=SM_OK) return -1;
       GpFileClose(f);
	
	return 0;
}

int sram_save()
{      F_HANDLE f;
       char s[100];

	/* If we crash before we ever loaded sram, DO NOT SAVE! */
       if (!mbc.batt || !ram.loaded || !mbc.ramsize)  return -1;

       gm_sprintf(s,"%s%8s.srm",saveDir,romName);

       if(GpFileCreate(s,ALWAYS_CREATE,&f)!=SM_OK)
       { saveDir[10]=0 ; GpDirCreate(saveDir,1) ; saveDir[10]='G' ;
         saveDir[16]=0 ; GpDirCreate(saveDir,1) ; saveDir[16]='\\' ;
       	 if(GpFileCreate(s,ALWAYS_CREATE,&f)!=SM_OK) return -1 ;
       }

       GpFileClose(f);

       if(GpFileOpen(s,OPEN_W,&f)!=SM_OK) return -1;

       if(GpFileWrite(f, (void *)&ram.sbank[0], 8192 * mbc.ramsize)!=SM_OK) return -1;
       GpFileClose(f);
	
       return 0;
}

void state_save(int n)
{       F_HANDLE f;
       char s[255];
	long tt;	if (n < 0) n = 0;

       gm_sprintf(s,"%s%8s.st%d",saveDir,romName,n);

       if(GpFileCreate(s,ALWAYS_CREATE,&f)!=SM_OK) // create dir if not exist
       { saveDir[10]=0 ; GpDirCreate(saveDir,1) ; saveDir[10]='G' ;
         saveDir[16]=0 ; GpDirCreate(saveDir,1) ; saveDir[16]='\\' ;
       	 if(GpFileCreate(s,ALWAYS_CREATE,&f)!=SM_OK)	return;
       }
       GpFileClose(f);

       if(GpFileOpen(s,OPEN_W,&f)!=SM_OK) return;

	//savestate(f)
	{
	int i; 
	byte buf[4096];
	un32 (*header)[2] = (un32 (*)[2])buf;
	un32 d = 0;
	int irl = hw.cgb ? 8 : 2;
	int vrl = hw.cgb ? 4 : 2;
	int srl = mbc.ramsize << 1;

	ver = 0x105;
	iramblock = 1;
	vramblock = 1+irl;
	sramblock = 1+irl+vrl;
	wavofs = 4096 - 784;
	hiofs = 4096 - 768;
	palofs = 4096 - 512;
	oamofs = 4096 - 256;
	memset(buf, 0, sizeof buf);

	for (i = 0; svars[i].len > 0; i++)
	{
		header[i][0] = *(un32 *)svars[i].key;
		switch (svars[i].len)
		{
		case 1:
			d = *(byte *)svars[i].ptr;
			break;
		case 2:
			d = *(un16 *)svars[i].ptr;
			break;
		case 4:
			d = *(un32 *)svars[i].ptr;
			break;
		}
		header[i][1] = LIL(d);
	}
	header[i][0] = header[i][1] = 0;
	memcpy(buf+hiofs, ram.hi, sizeof ram.hi);
	memcpy(buf+palofs, lcd.pal, sizeof lcd.pal);
	memcpy(buf+oamofs, lcd.oam.mem, sizeof lcd.oam);
	memcpy(buf+wavofs, snd.wave, sizeof snd.wave);
	GpFileSeek(f,FROM_BEGIN,0,&tt);
	GpFileWrite(f, (void *)&buf[0], 4096 * 1);
	GpFileSeek(f,FROM_BEGIN,iramblock<<12,&tt);
	GpFileWrite(f, (void *)&ram.ibank[0], 4096 * irl);
	GpFileSeek(f,FROM_BEGIN,vramblock<<12,&tt);
	GpFileWrite(f, (void *)&lcd.vbank[0], 4096 * vrl);
	GpFileSeek(f,FROM_BEGIN,sramblock<<12,&tt);
	GpFileWrite(f, (void *)&ram.sbank[0], 4096 * srl);
	}
       GpFileClose(f);
}

void state_load(int n)
{	F_HANDLE f;
       char s[255];
	long tt;

	if (n < 0) n = 0;

       gm_sprintf(s,"%s%8s.st%d",saveDir,romName,n);
       if(GpFileOpen(s,OPEN_R,&f)!=SM_OK) return;

//       loadstate(f);
       {
	int i, j;
	byte buf[4096];
	un32 (*header)[2] = (un32 (*)[2])buf;
	un32 d;
	int irl = hw.cgb ? 8 : 2;
	int vrl = hw.cgb ? 4 : 2;
	int srl = mbc.ramsize << 1;

	ver = hramofs = hiofs = palofs = oamofs = wavofs = 0;
       GpFileSeek(f,FROM_BEGIN,0,&tt);
       GpFileRead(f, (void *)&buf[0], 4096 * 1, (unsigned long *)&tt);

	for (j = 0; header[j][0]; j++)
	{
		for (i = 0; svars[i].ptr; i++)
		{
			if (header[j][0] != *(un32 *)svars[i].key)
				continue;
			d = LIL(header[j][1]);
			switch (svars[i].len)
			{
			case 1:
				*(byte *)svars[i].ptr = d;
				break;
			case 2:
				*(un16 *)svars[i].ptr = d;
				break;
			case 4:
				*(un32 *)svars[i].ptr = d;
				break;
			}
			break;
		}
	}

	/* obsolete as of version 0x104 */
	if (hramofs) memcpy(ram.hi+128, buf+hramofs, 127);
	
	if (hiofs) memcpy(ram.hi, buf+hiofs, sizeof ram.hi);
	if (palofs) memcpy(lcd.pal, buf+palofs, sizeof lcd.pal);
	if (oamofs) memcpy(lcd.oam.mem, buf+oamofs, sizeof lcd.oam);

	if (wavofs) memcpy(snd.wave, buf+wavofs, sizeof snd.wave);
	else memcpy(snd.wave, ram.hi+0x30, 16); /* patch data from older files */
       GpFileSeek(f,FROM_BEGIN,iramblock<<12,&tt);
       GpFileRead(f, (void *)&ram.ibank[0], 4096 * irl, (unsigned long *)&tt);
       GpFileSeek(f,FROM_BEGIN,vramblock<<12,&tt);
       GpFileRead(f, (void *)&lcd.vbank[0], 4096 * vrl, (unsigned long *)&tt);
       GpFileSeek(f,FROM_BEGIN,sramblock<<12,&tt);
       GpFileRead(f, (void *)&ram.sbank[0], 4096 * srl, (unsigned long *)&tt);
       }

       GpFileClose(f);

       vram_dirty();
       pal_dirty();
       sound_dirty();
       mem_updatemap();
}

void *gm_realloc(void *x, int s)
{ void *nw=gm_zi_malloc(s);
 if(nw==NULL) return NULL;
 gm_memcpy(nw,x,s);
 gm_free(x);
 return nw;
}

char * gbgetLegalName(char *strout)
{	char n[17] ; char *p = n ;
	char *out = strout ;	char *o=out ;
	//error(strout);
	memcpy(n,strout,16);
	while(o<out+8)
	{	if(p < n+16)
		{	if((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9')) *o++ = *p ;
			p++ ;
		} else *o++ = '_' ;
	}; *o=0 ;// error(out);
	return out ;
}

void _fgbMain_(void *_rom, ulong _size, char*romNme)
{int firsttime=1; char romNmeeeee[42] ; memcpy(romNmeeeee,romNme,16) ;
 romName = romNmeeeee ; romSize=_size ;
 gbgetLegalName(romNmeeeee);
 //error(romNmeeeee);	error(romName);
  Gamma=GAMMA;
  RomImage=_rom;
 GpClockSpeedChange(132000000, 0x3a011, 3); //spiv
vid_init();pcm_init();init_video();loader_init((char*)_rom);
emu_running=1;emu_reset();

  MenuInit(&menugb);
  MenuAdd(&menugb,OPTIONS,&mReset,BACK,   "Game\tContinue\tReset\tBack to yAnl\n");
  MenuAdd(&menugb,SUBMENU ,(char *)&menuvideo,CONTINUE    ,"Video/options");
  MenuAdd(&menugb,SUBMENU ,(char *)&menusave,CONTINUE     ,"Load/Save");
  MenuAdd(&menugb,SUBMENU ,(char *)&menudebug,CONTINUE    ,"Debug...");
  MenuInit(&menusave);
  MenuAdd(&menusave,OPTIONS,&mSaveState,BACK,"Save State\tno\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9");
  MenuAdd(&menusave,OPTIONS,&mLoadState,BACK,"Load State\tno\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9");
  MenuAdd(&menusave,SUBMENUEXIT,(char *)&menugb,BACK,"Back\tBack\n");
  MenuInit(&menuvideo);
  MenuAdd(&menuvideo,OPTIONS ,&mStretch,BACK ,"GB Stretching\t1:1\t2:1.66\t2:1\t1:1.66\t1.5:1.5\t2:1.5");
  MenuAdd(&menuvideo,OPTIONS ,&mFSkip,BACK     ,"FrameSkip\t0\t1\t2\t3\t4");
  MenuAdd(&menuvideo,OPTIONS ,&frequency,BACK ,"frequency (mhz)\t133\t80\t66\t33");
  MenuAdd(&menuvideo,OPTIONS ,&mVSync,BACK,"Synchronization\toff\tframe limit\taccurate");
  MenuAdd(&menuvideo,OPTIONS ,&mBuffering,BACK,"Double buffering\tdisabled\tenabled");
  if(!hw.cgb)  MenuAdd(&menuvideo,OPTIONS ,&mGBPal,BACK     ,"GB Mono Palette\tGreen\tLime\tYellow\tGray\tIndigo\n");
  MenuAdd(&menuvideo,PROGRESS,&mGamma,BACK,"Gamma Correction\tMIN\tMAX\t1\t100\n");
  MenuAdd(&menuvideo,SUBMENUEXIT,(char *)&menugb,BACK,"Back\tBack");
  MenuInit(&menudebug);
   mType=1^hw.cgb;
  
  MenuAdd(&menudebug,OPTIONS ,&mType , BACK, "Force system (*) to\tGB\tGBC\n");
  MenuAdd(&menudebug,OPTIONS ,&mDebug, BACK, "Debug mode\tOff\tOn");
  MenuAdd(&menudebug,SUBMENUEXIT,(char *)&menugb,BACK,"Back\tBack\n");

       frames_rendered=frames_displayed=0; fps_string[0]='\0';
       framelimit_delay=0;
       GpSoundBufStart(&sndbuf);
	emu_run();
       pcm_close();
       loader_unload();
       FUX0R_PAL();
}
