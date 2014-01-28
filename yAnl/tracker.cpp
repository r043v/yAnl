
/* ************** Tr4K'r ! (c) 2k5 r043v, under GPL as other yAnl part **/

/*--------------*/  /* code to get octave/note or text from a mod frequence */

int getOctave(int freq)
{       if(freq < 57)  return 5;
   else if(freq < 113) return 4;
   else if(freq < 226) return 3;
   else if(freq < 453) return 2;
   else if(freq < 907) return 1;
   return 0;
}

const char *noteName[14] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B","???","   " } ;

// max freq for octave 0->5         C      C#    D     D#    E       F     F#    G    G#    A    A#  and < -> B
int maxFreqPerNote[6*11+1] = {	1616,	1525,1440,1357,1281,1209,1141,1077,1017, 961, 907,
                                                808,	762,	720,	 678,	 640,	 604,	 570, 538,	 508, 480,	 453,
                                                404,	381,	360,	 339,	 320,	 302,	 285,	 269,	 254,	 240,	 226,
                                		202,	190,	180,	 170,	 160,	 151,	 143,	 135,	 127,	 120,	 113,
		                                101,	95,	90,	 85,	 80,	 76,	 71,	 67,	 64,	 60,	 57,
                		                50,	47,	45,	 42,	 40,	 38,	 35,	 33,	 32,	 30,	 28,
						0
		                             };

int getNoteValue(int freq, int octave=0xff)
{ if(!freq) return 13 ;
  if(octave>5) octave = getOctave(freq);
  int * p = &(maxFreqPerNote[octave*11]);
  for(int c=0;c<12;c++)
    if(freq > p[c]) return c ;
  return 12 ; // 12, unknow
}

char noteText[6*11*4];
const char *getNoteText(int freq, int octave=0xff)
{ static int first=0 ; if(!first++)memset(noteText,0,6*11*4);
  if(!freq) return noteName[13];
  if(octave>5) octave = getOctave(freq);
  int v = getNoteValue(freq,octave) ;
  char *txt = &(noteText[octave*11*4 + v*4]);
  if(!*txt)
  { const char *s=noteName[v], *o="%s%i" ;
    if(!s[1]) o="%s-%i"; sprintf(txt,o,s,octave) ;
  } return txt ;
}

/*--------------*/

struct note
{	char	sample ;
	short	effect ;
	short	period ;
	const char	*text ;
};

class pattern
{	public :
	int*	dta ;
	note**	raw ;
	int loaded  ;

	pattern(int* dt) {dta=dt; loaded=0;}
	void	load(void) ;
	void	load(int nb) ;
	void	readRaw(int nraw) ;
};

class tracker
{	public :
	int	numPattern;
	pattern	*patterns	;
	uchar	pos[3]	;	// pattern, raw, chn
	uchar	cpos[3] 	;	// cursor pos -> pattern, raw, cursor song pos

	int		lock		;	// lock actual pattern
	int		mute		;	// set if channels are mute
	int		keepLock	;	// keep lock and mute when not draw

	int		alocated	;
	int 		loaded	;
	int		lastloaded 	;
	int		play		;
	MODPlay modplayer	;
	MOD		*mod		;
	note*	clipboard[64];
	int		clipSize	;
	uint		modSize	;
	Gft		*fnt		;
	int select, slpos		;
	ulong time			;

	void	applyLock(void);						// lock pattern and mute channels
	void	load(char * m, uint sze) ;				// init and alocate memory
	note*	getNote(int p, int raw, int chn) ;		// get a note from the tracker object
	void	setNote(int p, int raw, int chn, note *n) ;	// set a note, into the tracker and the mod in ram
	int		getmodnote(int ptn, int raw, int chn) ;	// get an undecrypted note from a mod file (in ram)
	note	convertNote(int nt) ;					// convert a mod file note into an note struct
	void	noteUp(int ptn, int raw, int chn) ;
	void	noteDown(int ptn, int raw, int chn) ;
	void  viewChannel(int chn) ; // view and edit a specific channel
	void	stop(void) ;
	void	show(void) ;								
	void	draw(void) ;						// draw
	void	bgLoad(void) ;						// background patterns parssing
	void	kill(void) ;							// free the memory used by the tracker
	void	save(void) ;
	~tracker() { kill() ; }
	tracker() {	loaded=0 ; patterns=0; alocated=0; }
};

int flip=1 ;	// a var to know if we must flip the screen or not (when what is need to draw is on the screen, not draw and flip, parse pattern)
int edit=0 ;	// edit at 0, will follow song play, at 1, will follow the cursor.

extern u16 freqtab[256];

void	tracker::noteDown(int ptn, int raw, int chn)
{	note * n = &((patterns[ptn].raw[raw])[chn]) ;
	if(!n->period) return ;
	const char * tx = n->text ;
	int o = getOctave(n->period);
	if(o==5 && tx == &(noteText[5*11*4 + 1*4])) return ;
	do {	n->period = n->period / 1.0595 ;
		n->text = getNoteText(n->period) ;
	} while(tx == n->text);
	int t ;	char * y = (char*)&t ;
	y[0] =	((n->sample)&0xF0) + (((n->period)>>8)&0x0F) ;
	y[1] =	(n->period)&0xFF ;
	y[2] =	(((n->sample)&0x0F)<<4) + (((n->effect)>>8)&0x0F) ;
	y[3] =	(n->effect)&0xFF ;
	*(int*)((char*)(mod->patterndata) + 1024*ptn + 16*raw + chn*4 ) = t ;
}

void	tracker::noteUp(int ptn, int raw, int chn)
{	note * n = &((patterns[ptn].raw[raw])[chn]) ;
	if(!n->period) return ;
	const char * tx = n->text ;
	int o = getOctave(n->period);
	if(!o && tx == &(noteText[0])) return ;
	do {	n->period = n->period * 1.0595 ;
		n->text = getNoteText(n->period) ;
	} while(tx == n->text);
	int t ;	char * y = (char*)&t ;
	y[0] =	((n->sample)&0xF0) + (((n->period)>>8)&0x0F) ;
	y[1] =	(n->period)&0xFF ;
	y[2] =	(((n->sample)&0x0F)<<4) + (((n->effect)>>8)&0x0F) ;
	y[3] =	(n->effect)&0xFF ;
	*(int*)((char*)(mod->patterndata) + 1024*ptn + 16*raw + chn*4 ) = t ;
}

Pixel pink[320] ;

void  tracker::viewChannel(int chn)
{ pattern *p;// = &(patterns[ptn]) ;	if(p->loaded < 63) p->load(64) ; 
  note * n;// = &(p->raw[raw])[chn] ; 
  memset(pink,0,320);
  int raw = 0x2a2a, ptn ;
do{
   if(mod->patternline != raw) { // do we need to refresh draw ?
	ptn = mod->song[mod->songpos] ;
	raw = mod->patternline ;
  int y=312, x,o, nb=0, rawStart=raw-16 ; if(rawStart<0) rawStart=0 ;
  if(raw+16 > 63) rawStart = 32 ;
	p = &(patterns[ptn]) ;	if(p->loaded < 63) p->load(64) ;
  //memset(screen[nflip],0xff,240*320); // fucking memset !!!!!! why did you disturb sound ?!!! ... i finally found why the sound is poor (tick tick tick!!) in Gmineur ...
  memcpy(screen[nflip],blankScr,240*320);
  
/*  ligne(17,319,17,32,pink);
  ligne(215,319,215,32,pink);
  ligne(215,32,17,32,pink);
  ligne(20+11*16+8,319,20+11*16+8,32,pink);*/
  const char *chr = "*" ; /*Pixel *color ;*/ char txt[80] ;
  const char *argString ;
  if(raw<10)	argString = "ptn %i/%i raw 0%i chn %i/%i" ;
  	else		argString = "ptn %i/%i raw %i chn %i/%i" ;
sprintf(txt,argString,ptn+1,mod->num_patterns,raw,chn+1,mod->num_channels);
 fnt->draw(txt,10,22,1);
	  for(int c=rawStart;c<64;y-=9)
	   { n = &((p->raw[c++])[chn]) ;
	     o = getOctave(n->period);
	     x = getNoteValue(n->period,o);	
	     if(c-1==raw) { if(c<63)	ligne(19,y,19+11*16+7,y,pink);
	                    			ligne(19+11*16+7,y+7,19,y+7,pink);
fnt->draw(noteName[x],0,20+(x<<4),1);
        	            			//fnt->drawInt(c,10,220,"%i",1) ; // draw curent raw number        	            			
                	  }
	     if(c<64) { if(!((c-1)%16))      ligne(176,y+4,32,y+4,pink);
        	           else if(!((c-1)%8)) ligne(32,y+4,176,y+4,pink);
                	 } //else error("raw value error !");
	     if(x<13) { //if(c-1!=raw) { color=pink ; chr="." ; }
        	        //else { color=blue ; chr="*" ; }
                	   fnt->draw(chr,y,28+(x<<4)); // draw an * at note pos
		  	   fnt->drawIntI(o,y,28+12*16,"%i") ;// and draw note octave
        	      }
	       // fnt->draw("marianne.",142+8,142,1); /*   :)   */
        	       //if(n->sample) fnt->drawInt(n->sample,y,16,"%i",1) ;
	               //if(n->effect) fnt->drawInt(n->effect,y,220,"%3x",1) ;
	     if(++nb >= 32) break;
	   };
		   showfps();
		   flipScreen();
	} else {	refreshKey() ;
			//GpSurfaceFlip(&gpDraw[!nflip]) ;
		   //flipScreen();
		 }
		 
   } while(!keyDown(kB));
}

inline void pattern::load(void)
{	if(loaded<64)	readRaw(loaded++) ;
}

inline void pattern::load(int nb)
{	int s=loaded+nb ; if(s > 64) s=64 ;
	while(loaded<s)	readRaw(loaded++) ;
}

inline void	pattern::readRaw(int nraw)
{	note*	n = raw[nraw] ;
	int*	p = (int*)((char*)dta + 16*nraw) ;	char *pt ;
	for(int cpt=0;cpt<4;cpt++)
	{	if(*p){
			pt = (char*)p ; p++ ;
			n[cpt].sample	=	((pt[2])>>4) + (pt[0] & 0xF0)	;
			n[cpt].period	=	((pt[0] & 0x0F)*256) + pt[1]	;
			n[cpt].effect	=	((pt[2] & 0x0F)*256) + pt[3]	;
			n[cpt].text		=	getNoteText(n[cpt].period) ;
		} else {	p++ ;	memset(&(n[cpt]),0,sizeof(note)) ;	}
	};
}

void tracker::load(char * m, uint sze)
{	kill() ;	if(!m) { error("nothing to load.") ; return ; }
	modSize = sze ;
	lock = 255 ; mute = keepLock = 0 ;
	MODPlay_Init(&modplayer) ;
	MODPlay_SetMOD(&modplayer,(uchar*)m) ;
        MODPlay_SetStereo (&modplayer,0);	// my right speaker is dead .. as the screen and the pad :|
        MODPlay_SetPlayFreq ( &modplayer, 44100 );
        //modplayer.mod.musicvolume = 64 ;
	MODPlay_Start(&modplayer) ;
	mod=&(modplayer.mod) ;	fnt = &GdlFont ;
	cpos[0] = pos[0] = mod->song[0] ;	pos[1] = pos[2] = 0 ;
	numPattern = mod->num_patterns ;	loaded = edit = cpos[1] = 0 ;
	patterns = (pattern *)malloc(numPattern*sizeof(pattern)) ;
	int c ;	note * n = (note*)malloc(4*sizeof(note)*64*numPattern) ;
	for(int cpt=0;cpt<numPattern;cpt++)
	{	patterns[cpt].pattern((int*)(mod->patterndata + 1024*cpt)) ;
		patterns[cpt].raw = (note**)malloc(64*sizeof(note*)) ;
		for(c=0;c<64;c++)	patterns[cpt].raw[c] = &(n[4*c + 256*cpt]) ;
	};			alocated=1 ;
	patterns[mod->song[0]].load(64) ;
	lastloaded = 0 ; play = 1 ;	select=time=0 ;
	sprintf(gerr+582,"%i/%i",pos[0],numPattern-1) ;
	sprintf(gerr+542,"%i/%i",mod->songpos,mod->song_length-1) ;
}

void tracker::applyLock(void)
{	if(lock!=255) mod->songpos = lock ;
	if(mute)
	{	if(mute&0xFF)			mod->channel_active[0]=0 ;
		if(mute&0xFF00)		mod->channel_active[1]=0 ;
		if(mute&0xFF0000)		mod->channel_active[2]=0 ;
		if(mute&0xFF000000)	mod->channel_active[3]=0 ;
	}
}

char * getLegalName(char*str)
{	char *p = str ;
	char *out = (char*)malloc(13) ;	char *o=out ;
	*(int*)(out+8) = 0x646F6D2E ;	out[12] = 0 ; // write ".mod\0"
	while(o<out+8)
	{	if(p < str+19)
		{	if((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9')) *o++ = *p ;
			p++ ;
		} else *o++ = '_' ;
	};	return out ;
}

void tracker::save(void)
{	if(GpSMCDetected()) return ;
	overclock();
	char * pth = getLegalName((char*)(mod->modraw)) ;
	char pt[128] ;	char *p = (char*)"gp:\\GPETC\\MYMOD\\" ;
	p[10]=0 ; GpDirCreate(p,1) ; p[10]='M' ; p[15]=0 ; GpDirCreate(p,1) ; p[15]='\\' ;
	sprintf(pt,"%s%s",p,pth) ;
	F_HANDLE f ;
	GpFileCreate(pt,ALWAYS_CREATE, &f);
	GpFileWrite(f,mod->modraw,modSize) ;	GpFileClose(f) ;
	downclock();
	sprintf(pt,"mod saved into\n%s%s\n%i bytes",p,pth,modSize) ; error(pt) ;
}

void tracker::stop(void)
{	static char * modSave=0 ;	static uint s = 0 ;
	if(s)
	{	load(modSave,s) ;	s=0 ;
	} else {	modSave = (char*)malloc(modSize) ; s=modSize ;
				memcpy(modSave,mod->modraw,modSize) ;
				kill() ;
	}
}

void tracker::kill(void)
{	if(alocated)	{	MODPlay_Stop(&modplayer) ;
					free(modplayer.mod.modraw) ;
					MODPlay_Unload(&modplayer) ;
					free(patterns[0].raw[0]) ;
					for(int cpt=0;cpt<numPattern;)	free(patterns[cpt++].raw) ;
					free(patterns) ;	loaded=alocated=0 ;
				}
}

inline void tracker::bgLoad()
{	if(loaded) return ;
	if(patterns[mod->song[mod->songpos]].loaded < 63) patterns[mod->song[mod->songpos]].load() ;
	else	{	while(patterns[lastloaded].loaded>63)	{	if(lastloaded<numPattern-1)	lastloaded++ ;
															else	{	loaded=1 ;	return ;	}
														};
				patterns[lastloaded].load() ;
			}
}

void tracker::show(void)
{	static int last = 142 ;
	static int lastRaw = 0x2a ;
	static Gft *sfnt = &bfont ;
	static Gft *vfnt = &vfont ;
	if(pos[1] != lastRaw || ((!play) && actualKey))
	{	if(edit)
		{	if(last != cpos[2])
			{	last = cpos[2] ;
				sprintf(gerr+582,"%i/%i",cpos[0],numPattern-1) ;
				sprintf(gerr+542,"%i/%i",cpos[2],mod->song_length-1) ;
			}
		} else {	if(last != pos[0])
					{	last = pos[0] ;
						sprintf(gerr+582,"%i/%i",pos[0],numPattern-1) ;
						sprintf(gerr+542,"%i/%i",mod->songpos,mod->song_length-1) ;
					}		
		}
		
		lastRaw = pos[1] ;	flip=1 ;
		pattern *p = &(patterns[cpos[0]]), *q ;
		if(p->loaded < 63) p->load(64) ;
		int s = cpos[1] - 16 ;
		if(s+32 > 63) s = 31 ;
		else	if(s<0) s=0 ;
		note * n ;	clrScr() ;	int t=32 ;
		if(pos[0] == cpos[0])	// if play same pattern as cursor here, mark the played raw
		{	q=p ;	if(pos[1] >= s && pos[1] <= s+32)	rectFill(9*(32-(pos[1]-s))+17,14,8,193,33) ;
		}	else	q = &(patterns[pos[0]]) ;

		if(select)
		{	int px=9*(32-(slpos-s))+16 ;
			int py=14+pos[2]*50 ;
			int sx=(slpos-cpos[1])*9 ;

			if(sx>0){	if(px < 15){ px=15 ; sx-=15 ; }
							rectFill(px,py,sx+9,50,42) ;
			} else rectFill(px+sx,py,-sx+9,50,42) ;
		}

		rectFill(9*(32-(cpos[1]-s))+16,14+pos[2]*50,10,44,22) ; // show cursor
		do{	//if(s+(32-t) > 63) {error("raw is highter than 63") ; break ; }
			n = p->raw[s+(32-t)] ;
		
			if(!((s+(32-t))&7))	// show line eatch 8 and 16 raw
			{	if(!((s+(32-t))&15))
				{	int *sr = (int*)(screen[nflip] + 240*(26 + 9*t) + 48) ; int num = 39 ;
					while(num--) *sr++ = 33 ; //*(short*)sr = 0 ;
				}	else	{	int *sr = (int*)(screen[nflip] + 240*(26 + 9*t) + 72) ; int num = 27 ;
								while(num--) *sr++ = 22 ; //*(short*)sr = 0 ;			
							}
			}
			for(int m=0;m<4;m++)	
			{	if(n[m].sample)
				{		//fnt->drawInt(n[m].sample,9*t + 16,15+28+m*50,"%x",1) ;
					vfnt->drawInt(n[m].sample,9*t + 16,15+28+m*50,"%X",1) ;
				}else	if(n[m].effect)
						{	switch((n[m].effect)&0xF00)
							{	case 0xA00 :	{	int d=0 ;
													if((n[m].effect)&0x0F0)	{ 	sfnt->draw("+",9*t + 16,15+20+m*50,1) ; d=4 ; }
													else						sfnt->draw("-",9*t + 16,15+20+m*50,1) ;
													sfnt->drawInt(((n[m].effect)&0x0FF)>>d,9*t + 16,15+28+m*50,"%x",1) ;	break ;
												}
								case 0xC00 :	{	sfnt->drawInt((n[m].effect)&0x0FF,9*t + 16,15+28+m*50,"%i",1) ;	break ;	}
							};
						}
				if(n[m].period)	fnt->draw(n[m].text,9*t + 16,15+m*50,1) ;
				else	{	if(!n[m].effect)	fnt->draw("--",9*t + 16,20+m*50,1) ;
												fnt->draw("-",9*t + 16,20+m*50,1) ;
				}
			};
		} while(t--) ;

		fnt->drawI(gerr+582,315,2) ;	fnt->drawI(gerr+542,260,2) ;
		fnt->drawIntI(cpos[1],172,2,"%2i") ;
		
		if(edit)	fnt->drawIntI(pos[0],132,2,"%2i") ;

		if(mute) // draw a rect under all enabled channel
		{	if(mute&0xFF)			rectFill(1,14,12,48,219) ;
			else					rectFill(2,14,12,48,222) ;			
			if(mute&0xFF00)		rectFill(1,62,12,50,219) ;
			else					rectFill(2,62,12,50,222) ;			
			if(mute&0xFF0000)		rectFill(1,112,12,50,219) ;
			else					rectFill(2,112,12,50,222) ;			
			if(mute&0xFF000000)	rectFill(1,162,12,50,219) ;
			else					rectFill(2,162,12,50,222) ;
				rectFill(2,213,12,27,222) ;
		}	else	rectFill(2,14,12,226,222) ; // or draw all in one time

		rectFill(14,13+50*pos[2],1,49,33) ;

		n = q->raw[pos[1]] ;
		for(int m=0;m<4;m++)
		{	if(n[m].sample)	fnt->drawInt(n[m].sample,3,15+28+m*50,"%x",1) ;
			if(n[m].period)	fnt->draw(n[m].text,3 ,15+m*50,1) ;
			//else			fnt->draw("--",9*t + 16,20+m*50,1) ;
			//int v=((mod->volume[m])>>1)&26 ;
			//rectFill(20+58*m, 212, 42, v, 219); // volume bar at right
			// volume bar on eatch channel
			rectFill(15+mod->volume[m], 58 + m*50, 1, 3, 33);
			rectFill(15, 60 + m*50, mod->volume[m], 2, 33);
		};

		fnt->drawInt(mod->patternline,3,216,"%i",1) ;
		if(!loaded)	fnt->drawIntI(lastloaded,100,2,"%i") ;
	} else	{	flip=0 ;	bgLoad() ;	}
}

note*	tracker::getNote(int ptn, int raw, int chn)
{	if(patterns[ptn].loaded>raw)	return &((patterns[ptn].raw[raw])[chn]) ;
	else	return 0 ;
}

void	tracker::setNote(int ptn, int raw, int chn, note* n)
{	// fix the pattern in the tracker object
	note * nt = &((patterns[ptn].raw[raw])[chn]) ;
	nt->sample = n->sample ;
	nt->period = n->period ;
	nt->effect = n->effect ;
	nt->text   = n->text   ;
	// and in the mod
	int t ;	char * y = (char*)&t ;
	y[0] =	((n->sample)&0xF0) + (((n->period)>>8)&0x0F) ;
	y[1] =	(n->period)&0xFF ;
	y[2] =	(((n->sample)&0x0F)<<4) + (((n->effect)>>8)&0x0F) ;
	y[3] =	(n->effect)&0xFF ;
	*(int*)((char*)(mod->patterndata) + 1024*ptn + 16*raw + chn*4 ) = t ;
}

int	tracker::getmodnote(int ptn, int raw, int chn) { return *(int*)((char*)(mod->patterndata) + 1024*ptn + 16*raw + chn*4 ) ; }

note	tracker::convertNote(int nt)
{	note n ; char *pt = (char*)(&nt) ;
	n.sample = ((pt[2])>>4) + (pt[0] & 0xF0);
	n.period = ((pt[0] & 0x0F)*256) + pt[1]	;
	n.effect = ((pt[2] & 0x0F)*256) + pt[3]	;
	n.text	 = getNoteText(n.period) ;
	return n ;
}

void bgFlip(void)	// switch bg bitmap to file selector or mod player
{	static int lastFlip = 0 ;
	static Pixel * s = (Pixel*)blankScr ;
	//Pixel color = 0xdf*lastFlip ; int t ;
	lastFlip ^= 1 ;		int t=56 ;

	short* ptr ;
	for(int cpt=0;cpt<320;cpt++)
	{	ptr = (short*)blankScr ; ptr += cpt*120 ;
		if(!lastFlip)
		{	int tmpp=19 ;
			while(tmpp--)	*ptr++ = (short)(222*256)+223 ;
			*ptr++ = 222|223 ;
		} else {	memset(ptr,223,64) ;	}
	};

	{	int *sr = (int*)(s + 240*14) ;	int clr = 0xDFDF0000 * lastFlip ;
		if(!lastFlip) { t-=10 ; sr+=10 ; clr = 0xDFDFDFDF ; }
		while(t--) *sr++ = clr ; *(short*)sr = clr ;
		if(!lastFlip) *(((char*)sr)+1)=0 ;
	}

	/*if(!lastFlip)
	{	Pixel *sr ;
		for(int tmp=0;tmp<4;tmp++)
		{	sr = (Pixel*)(s + 240*15 + 28 + tmp*50) ;	t = 300 ;
			while(t--)	{	*sr = color ; sr+=240 ; }
		};
	}*/
}

void tracker::draw(void)
{	bgFlip();	note* cpy=getNote(0,0,0) ; ulong lastkey = 0 ; flip=0 ; int exit=0 ;
	do {	applyLock() ; show() ;
			if(lock!=255)	fnt->drawI("L",198,2) ;

			if(play)
				{	pos[0] = mod->song[mod->songpos] ;
					pos[1] = mod->patternline ;
				}	else	mod->patternline = pos[1] ;

		applyLock() ;
		refreshKey() ;

		if(actualKey || keyDown(kR))
		{	if(!edit)
			{	if(keyUp(kSelect)) { bgFlip() ; return ; }
				if(keyPush(kR)||keyDown(kR))	{	static int slct=0 ;
													if(keyUp(kR))	slct=0 ;
													else	if(keyDown(kR))	if(!slct) edit^=1 ;
													if(!slct)	if(keyPush(kRight)||keyPush(kLeft)||keyPush(kUp)||keyPush(kDown))	slct=1 ;
												}
				if(!keyPush(kStart))
				{	if(GpTickCountGet() - lastkey > 150)
					{	if(keyPush(kRight))	{	if(cpos[1])		cpos[1]-- ;	lastkey = GpTickCountGet() ;}
						if(keyPush(kLeft))	{	if(cpos[1]<63)	cpos[1]++ ;	lastkey = GpTickCountGet() ;}
					}	else	{	if(keyUp(kRight)) { if(cpos[1])		cpos[1]-- ;	lastkey = GpTickCountGet() ; }
									if(keyUp(kLeft))  { if(cpos[1]<63)	cpos[1]++ ;	lastkey = GpTickCountGet() ; }
								}
						if(keyUp(kUp))	{	if(pos[2])		pos[2]-- ;	}
						else		if(keyUp(kDown))	{	if(pos[2]<3)	pos[2]++ ;	}
				} else	{			if(keyUp(kRight))	{	mod->patternline = pos[1] = 0 ;	pos[0] = cpos[0] ; mod->songpos = cpos[2] ;	} // start+up will raw pos at 0
									if(keyUp(kLeft))	{	if(lock!=255) lock=255 ;
															else lock = mod->songpos ; } // with down lock the pattern
									if(keyUp(kUp))	{	if(mod->songpos) pos[0] = mod->song[--(mod->songpos)] ;
														if(lock!=255) lock=mod->songpos ;
														mod->patternline = pos[1] = 0 ; // with left, go to last pattern
													}
									if(keyUp(kDown)){	if(mod->songpos < mod->song_length) pos[0] = mod->song[++(mod->songpos)] ;
														if(lock!=255) lock=mod->songpos ;
														mod->patternline = pos[1] = 0 ; // and right to next pattern
													}
						}
			} else {	if(keyUp(kSelect)) exit=1 ;
						
						if(keyPush(kSelect))
						{	if(!keyPush(kStart)) {	if(keyUp(kUp))	{ noteUp(cpos[0],cpos[1],pos[2]) ; exit=2 ; }
							else	if(keyUp(kDown)) {	noteDown(cpos[0],cpos[1],pos[2]) ; exit=2 ; }
								else if(keyUp(kRight)) { viewChannel(pos[2]); exit=2; }
							} else save() ;	
						}
						
						if(keyPush(kR)||keyDown(kR))	{	if(!select) if(keyPush(kRight)||keyPush(kLeft))	select=1 ;
															if(keyUp(kR)) {	select = 0 ;	slpos=cpos[1] ; time=GpTickCountGet() ; }
															else	if(keyDown(kR)||(!select && (GpTickCountGet()-time > 1300)))
																	{	if(!select)
																		{	if(GpTickCountGet()-time < 1242) edit^=1 ;
																			else	{	int raw=0 ;	 // paste
																						if(clipSize){
																								for( int r=cpos[1];(r<=cpos[1]+clipSize) && (r<64);r++)
																									setNote(cpos[0], r, pos[2], clipboard[raw++]) ;
																							}
																					}
																		}	else {	clipSize = cpos[1]-slpos ;	// copy
																					int raw=0 ;	memset(clipboard,0,64*4) ;
																					if(clipSize>0){
																						for( int r=slpos;r<=cpos[1];r++)	clipboard[raw++] = getNote(cpos[0], r, pos[2]) ;
																							select=0 ;
																					} else  if(clipSize<0)
																							{	clipSize = - clipSize ;
																								for( int r=cpos[1];r<=slpos;r++)	clipboard[raw++] = getNote(cpos[0], r, pos[2]) ;
																								select=0 ;
																							} else select=0 ;
																					//sprintf(err,"copy %i notes\nfrom channel %i, raw %i to %i",clipSize,pos[2],slpos[0],cpos[1]) ; error(err) ;
																				 }
																	}

															if(!keyPush(kR))
															{	if(keyUp(kUp))	{	if(pos[2])		pos[2]-- ;	}
																else	if(keyUp(kDown))	{	if(pos[2]<3)	pos[2]++ ;	}
															}
																														
															if(GpTickCountGet() - lastkey > 150)
															{	if(keyPush(kRight))	{	if(cpos[1])		cpos[1]-- ;	lastkey = GpTickCountGet() ;}
																if(keyPush(kLeft))	{	if(cpos[1]<63)	cpos[1]++ ;	lastkey = GpTickCountGet() ;}
															}	else	{	if(keyUp(kRight)) { if(cpos[1])		cpos[1]-- ;	lastkey = GpTickCountGet() ; }
																			if(keyUp(kLeft))  { if(cpos[1]<63)	cpos[1]++ ;	lastkey = GpTickCountGet() ; }
																		}
														}
									else
						{
							if(!keyPush(kStart))	{		if(!keyPush(kSelect))
															{	if(keyUp(kUp))	{	if(pos[2])		pos[2]-- ;	}
																else	if(keyUp(kDown))	{	if(pos[2]<3)	pos[2]++ ;	}
															}
															
															if(GpTickCountGet() - lastkey > 150)
															{	if(keyPush(kRight))	{	if(cpos[1])		cpos[1]-- ;	lastkey = GpTickCountGet() ;}
																if(keyPush(kLeft))	{	if(cpos[1]<63)	cpos[1]++ ;	lastkey = GpTickCountGet() ;}
															}	else	{	if(keyUp(kRight)) { if(cpos[1])		cpos[1]-- ;	lastkey = GpTickCountGet() ; }
																			if(keyUp(kLeft))  { if(cpos[1]<63)	cpos[1]++ ;	lastkey = GpTickCountGet() ; }
																		}
															
								}	else				{	if(keyUp(kRight)) {	mod->patternline = pos[1] = 0 ;	pos[0] = cpos[0] ; mod->songpos = cpos[2] ; }	// set played pattern as cursor pattern
															else	if(keyUp(kLeft))	{	if(lock!=255) lock=255 ;
																							else lock = mod->songpos ; } // with down lock the pattern
																							
															if(keyUp(kUp))				{	if(cpos[2])						cpos[0] = mod->song[--cpos[2]] ;	}
															else	if(keyUp(kDown))		if(cpos[2] < mod->song_length)	cpos[0] = mod->song[++cpos[2]] ;
								}
						}
			}

			if(keyUp(kB))	cpy = getNote(cpos[0],cpos[1],pos[2]) ;
			else	if(keyUp(kA))	{	setNote(cpos[0],cpos[1],pos[2],cpy) ;
										//for(int c=0;c<4;c++)	if(c!=pos[2]) mod->channel_active[c]=0 ;
										if(!play)	{	mod->songpos = cpos[2] ;	mod->patternline = cpos[1] ;
														for(int c=0;c<4;c++)	if(c!=pos[2]) mod->channel_active[c]=0 ;
														MODPlay_Pause(&modplayer,0) ;
														while(mod->patternline == cpos[1]) { nflip^=1 ; flipScreen() ; } // why does i need to flip the screen ???!
														MODPlay_Pause(&modplayer,1) ;	mod->patternline = cpos[1] ;
													} else MODPlay_TriggerNote(&modplayer, 0, cpy->sample, cpy->period, 64);
									}
			if(keyUp(kL))
			{	if(keyPush(kStart))	{	MODPlay_Pause(&modplayer,play) ; play^=1; if(play) mod->patternline = cpos[1] ; }
				else				{	((char*)(&mute))[pos[2]] ^= 1 ;	}
			}
		}
			applyLock() ;
				if(flip)
				{	if(cpy)
					{	fnt->drawInt(cpy->sample,270,212,"%x",1) ;
						fnt->drawInt(cpy->effect,250,212,"%x",1) ;
						fnt->draw(cpy->text,260,212,1) ;
					}
					flipScreen() ;
				}

			if(!edit) {	cpos[0] = pos[0] ; cpos[1] = pos[1] ; cpos[2] = mod->songpos ; } // if not edit, follow the song play, else, follow cursor
	} while( (!keyDown(kSelect) || exit!=1) && (GpSMCDetected() || !intetris) && (!GpSMCDetected() || intetris)) ;
	bgFlip() ;
}

