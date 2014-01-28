/***** yAnl ** yet ... another new launcher *|* v0.43 Marianne edition *|* flu- release * compile under ads * under GPL */

/* change log from 0.42 ..
	* now, enter a dir or load a file is always do under 133mhz is start was use
	* now reset screen and sound after eatch overclock -> sound is now fine on loading
	* if you enter a dir with start, it will parse it directly at 133mhz, without some slowest bg parsing
	* start an gp based channel editor for the tracker part
	           (now need to switch into a 16 bit screen buffer to made a cool graphic note fade same as pc release of tr4k'r)
	* add an archive who can contain anything and who's include into the firmware
	* clean fgb, remove ~40ko compressed at the final gxb, remove some ram alloc who was not use .. now can load big rom, add frequency choice in option
	* stop using chn frequency table, use instead the small code i develop for mdpl, my own mod parser -> size issue
	* icons are now precompute, same as font, for a fastest load
	* all yAnl data are now grouped into a single file in the rar, for do only one uncrunch and increase start speed
	* switch to b2fxec 0.6pre to compact the gxb, now use -f option and fxp0 for a fastest firmware load
	* now game boy games use executable icons, search game name at file parsing
	* some some and some rewrite, add and fix, increase stability and speed
	* start think about parsing archived file, an modified rar lib who return only file start can be done
	* add support for an ini file
	* directory are now placed in the first of the list
	* ~full flu- compatibility (Gtris still unpatchable for the bg fade, need to change level switch method)
*/

/* know bugs
	* a big memory leak in gif load	.. fixed
	* a memory leak in rar listing	.. fixed
	* it seem that one (or more ?) file name are not properly parsed | remove bg parsing, now is fine
	* refresh gpmm (and gp ?) will cause a memory leak
	* tetris will leak 1ko of ram at eatch launch
	* icons are never free ?!!!
	* need to reset time of fxe icon parsing after a dir listing
	* a fucking bug in gtris, crash in some of time after intro, due to the test when i want to switch to 16mhz ?
*/

#include	"./lib/Gdl/Gdl.h"				// Gdl2 beta, nothing to see with the pc release (and future gp release ?) of Gdl²
#include	"./lib/unrarlib/unrarlib.c"		// unrarlib
#include	"./lib/modPlay/gpmodplay.h"	// chn mod lib !
#include	"./lib/fxeLaunch.c"			// aquafish fxe launch code :)
#include	"./yAnl/data/yAnlData.h"		// yAnl data into a rar file
#include	"./lib/tga.c"				// tga load code, from bgif lib
#include	"./lib/libgif/gpgif.c"			// gpgif lib from a spanish guy, who are you ? great job :)
#include	"./lib/gpsys.c"				// to get smc id, from gamepark lib
#include	"./lib/unlha/unlha.cpp"		// port of unlha, from modplug tracker source
#include	"./lib/plg/libplg.h"			// slubman easy plugin lib :)
#include	"./yAnl/data/includedRAR.h"	// an included rar, who contain anything you want ...

extern "C" { void _fgbMain_(void *rom, ulong _size, char*romNme); }	// fgb from rlyeh !! rlyeh you pAwa a lot :D

#define dataRarSize		14439
#define includeRarSize	267090  

// radix2.mod, the default module, a metal song from radix
#define defaultModSize	34236
#define defaultModName	"RADIX2.MOD"

// option
#define iniSize 20
char yAnlIni[iniSize] = { 0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } ;
//#define useSaveFile
#define loadDefaultMod	(yAnlIni[1]-'0')
#define hideUnknowFiles	(yAnlIni[2]-'0')

int intetris=0 ;	 Gft bfont, vfont, nfont ;

void	sndInit(void);
void	overclock(void)		{GpClockSpeedChange(132000000, 0x3a011, 3);sndInit();} // up to 133Mhz
void	downclock(void)		{GpClockSpeedChange(33000000, 0x24003, 0);sndInit();} // down to 33Mhz
void	_downclock(void)	{GpClockSpeedChange(33000000, 0x24003, 0);rstScreen();sndInit();} // down to 33Mhz and reset screen
 ulong lastIcoTime = 0 ;
#include	"./yAnl/tracker.cpp"	// tr4cker part !
#include	"./yAnl/yAnl.cpp"	// code to manage file and list

memory_file *programData, *includeData ;
#include	"./yAnl/showImg.cpp"	// to blit image, code from my pcx viewer, Gpcx |but an old source, lose the last with the zoom 1/2 fixed :|
tracker *trk ;	uint * gpal ;	int first = 1 ;

void sndInit(void){if(trk)MODPlay_SetPlayFreq(&(trk->modplayer),44100);}
#include	"./games/Gtris/Gtris.cpp" // Gtris 1.2 .. stop to code Gtris as single program, last release will be only in yAnl !

int ctick ;	//char * startText = " m4rI4nN3 " ;

void GpMain(void * arg)
{	initScreen();	GpFatInit();	overclock();	refreshKey() ;

	if(keyPush(kL))
	{	F_HANDLE f ; char *p = (char*)"gp:\\yAnl.gxb" ; char * g ;
		if(!GpFileOpen(p,OPEN_R,&f)) // if file exist
		{	ulong s ;	GpFileGetSize(p,&s) ;
			g = (char*)malloc(s) ;
			if(GpFileRead(f,g,s,0)) free(g) ;
			else GpAppExecute(g-4,0) ;
		}
	}

	//startText[0] = startText[9] = 3 ; // #love#
	ulong data_size ; gpal = savePal() ;
	programData = loadMemoryFile(data,dataRarSize) ;
	includeData = loadMemoryFile(includedRAR,includeRarSize) ;
	char* yAnlData; urarlib_get(&yAnlData, &data_size, (char*)"yAnlData", programData,0) ;
	for(int c=0;c<82;c++) memcpy(screen[!nflip]+(119+c)*240+98,yAnlData+c*44,44) ;	
	//GpTextOut(NULL, &gpDraw[!nflip],124,160,startText,0) ; // damn :'|

	#ifdef useSaveFile
	// read save file
		{	F_HANDLE f ;	//p[10]=0 ; GpDirCreate(p,1) ; p[10]='y' ; p[18]=0 ;
			if(!GpFileOpen((char*)"gp:\\GPETC\\yAnl.ini",OPEN_R,&f)) // if file exist
			{	GpFileRead(f,yAnlIni,iniSize,0);		}
		}	
	#endif

	// get default mod
	char *mod=0, *data_ptr ; if(loadDefaultMod) urarlib_get(&mod, &data_size, (char*)defaultModName, includeData,0) ;

	// declare some var..
	int nb=0, last, movetime = 192 ; ulong lastMove = 0 ; ulong fileSize ;
	int yap, tm, tm0, isPlugin=0 ; char color, *ptr = l->fileName[yap] ; tracker tr4k ;	trk = &tr4k ;
	char smcId[42] ; ulong id=0 ;

	// launch music playback
	if(loadDefaultMod)	tr4k.load(mod,defaultModSize) ;		
	
	{	// create bg bitmap
		int tmpp ;
		short* ptr = (short*)blankScr ;
		int colorPath	= (222 << 8)|222 ;
		int colorBg		= (223 << 8)|223 ;
		int mixed = (colorPath&0xFF)|(colorBg&0xFF) ;
		for(int cpt=0;cpt<320;cpt++)
		{	tmpp=19 ;
			while(tmpp--)	*ptr++ = (short)(222*256)+223 ;//colorPath ;
			*ptr++ = mixed ;	tmpp=92 ;
			while(tmpp--)	*ptr++ = colorBg ;
			*ptr++ = mixed ;		*ptr++ = colorPath ;
			*ptr++ = colorPath ;	*ptr++ = colorPath ;	*ptr++ = colorPath ;
			*ptr++ = colorPath ;	*ptr++ = colorPath ;	*ptr++ = colorPath ;
		};
	}

	GdlFont.mload(yAnlData+13572)  ;
	   nfont.mload(yAnlData+3608)  ;
	   vfont.mload(yAnlData+43424) ;
	   bfont.mload(yAnlData+33156) ;
	icons = mloadGfm(yAnlData+23840); // load precomputed icons ..
	moon[0] = &(icons[11]) ; moon[1] = &(icons[12]) ; moon[2] = &(icons[13]) ;

	l=&racine ;	*(int*)path = *(int*)dpath = *(int*)"gp:\\" ;	path[4]=dpath[4]=0; // ini path

	if(!GpSMCDetected())
	{	isPlugin = PlgInit() ; // there is any plugin on the smc ?
		_GpSmcIDGet((uchar*)smcId);	id = *(ulong*)(smcId+4) ; // get smc id
		//l=&racine ;	*(int*)path = *(int*)dpath = *(int*)"gp:\\" ;	path[4]=dpath[4]=0; // ini path
		loadList(path) ;		// retreve default dir file list
		int n=2 ;	while(n--)	{	parsingList(); parsingList(); parsingList(); // parsing it a bit ..
							parsingList(); parsingList(); parsingList();
						};
		*(int*)(path+4) = *(int*)"GPMM" ; path[8]='\\' ; path[9]=0 ; loadList(path) ; // and do the same for gpmm
		n=2 ;		while(n--)	{	parsingList(); parsingList(); parsingList();
							parsingList(); parsingList(); parsingList();
						};
		 path[4]=0; loadList(path) ; // back to gp:
	}
	clearColor=222;
	downclock();	//GpTimerOptSet(0,2,10,parsingList);	GpTimerSet(0);	 // down to 33mhz and launch parsing thread
						// damn i remove bg parsing support !!
						// now some some and some bug fixed, it was made too many file open ?
	while(1)
	{	ctick = GpTickCountGet();
		updateKey() ;	if(actualKey)	lastKey = ctick ;
		if(keyPush(kL))
		{	if(keyUp(kB)) // need to able view it without smc .. hard work in perspective :|
			{	sprintf(path,"gp:\\includeRAR") ;
				freeList() ; loadList((char*)includeData,0,1) ; setDPath() ;
				loading=1 ; int n=22 ;	while(loading && --n)	{	parsingList(); parsingList(); parsingList(); parsingList();	};
			}
		}

		if(GpSMCDetected()) // there is no smc, play Gtris !
		{	GtrisMain() ;
			// an smc is insert, gtris has exit, see if it's the same card
			GpFatInit() ; clear=2 ;
			memset(smcId,0,42) ;
			_GpSmcIDGet((uchar*)smcId);
			ulong i = *(ulong*)(smcId+4) ;
			if(id != i)	// is the same smc ?
			{	id = i ;	nb = 0 ;
				freeList(1) ;
				l = &racine ;	freeList(1) ;
				l = &gpmm	;	freeList(1) ;
				l = &incRAR;	freeList(1) ;
				l = &maliste;	*(int*)path = *(int*)"gp:\\" ; path[4]=0;
				loadList(path,0,1); setDPath() ;
				int n=2 ;	while(n--)	{	parsingList(); parsingList(); parsingList();
									parsingList(); parsingList(); parsingList();
								};
				isPlugin = PlgInit() ;
			}	setPal(gpal) ;
		}

		if(actualKey)
		{	clear=2 ;	last=nb ;
			if(ctick-lastMove > movetime)
			{	if(!keyPush(kR))
				{	if(keyPush(kUp))	nb-=2 ;
					else	if(keyPush(kDown))	nb+=2 ;
				} else {	if(keyPush(kUp))	nb-=6 ;
							else	if(keyPush(kDown))	nb+=6 ;
						if(nb < 0) nb&=1 ;
						if(nb > l->entryNumber-1) nb=l->entryNumber-1-(1-(nb&1))+(1-((l->entryNumber-1)&1)) ;
				}
				if(keyPush(kLeft))	if(nb&1) nb-- ;
				if(keyPush(kRight))	if(!(nb&1)) nb++ ;
			} else {
				if(!keyPush(kR))
				{	if(keyUp(kUp))	nb-=2 ;
					else	if(keyUp(kDown))	nb+=2 ;
				} else {	if(keyUp(kUp))	nb-=6 ;
							else	if(keyUp(kDown))	nb+=6 ;
						if(nb < 0) nb&=1 ;
						if(nb > l->entryNumber-1) nb=l->entryNumber-1-(1-(nb&1))+(1-((l->entryNumber-1)&1)) ;
				}
				if(keyUp(kLeft))	if(nb&1) nb-- ;
				if(keyUp(kRight))	if(!(nb&1)) nb++ ;
			}
			if(keyUp(kSelect)) tr4k.draw();
			if(keyUp(kL))	{
				if(keyPush(kDown) || nb<2) nb=l->entryNumber-1-(1-(nb&1))+(1-((l->entryNumber-1)&1)) ;
				else nb&=1 ;
			}
		}

		if(last!=nb) // if cursor pos was change
		{	if(nb < 0) nb=abs(nb%2) ;
			else if(nb > l->entryNumber-1) nb-=2 ;
			clear=2 ; lastMove=ctick ;
		}

		if(!actualKey){	lastnokey=ctick ; if(ctick-lastIcoTime > 1042) loadIcon(l->order[nb]) ; }
		else	lastIcoTime=ctick ;

		if(nb > l->entryNumber-1) nb=l->entryNumber-1 ; 

		if(ctick - lastnokey > 1800)			movetime = 12	;
		else	if(ctick - lastnokey > 1000)		movetime = 42	;
			else	if(ctick - lastnokey > 500)	movetime = 102	;
				else					movetime = 192	;

		if(l->entryNumber > 20)
		{	yap=nb-10-(nb&1);
			if(yap<0) yap=0 ;
			else	if(yap+20 > l->entryNumber) yap = l->entryNumber - 20 + ((l->entryNumber)&1) ;
		} else yap=0 ;

		ptr = l->fileName[l->order[yap]] ;

		if(clear)
		{	clrScr() ;
			for(int cpt=0;cpt<20;cpt++)
			{	if(yap >= l->entryNumber) break ;
				if(l->ico[l->order[yap]])	color=16 ;
				else					color=15 ;
				if(yap!=nb)			strAdd(gerr,"-  ",ptr) ;
				else { color = 0x2A ;	strAdd(gerr,"-> ",ptr) ; }

				tm = 165*(cpt&1)+5 ; tm0 = 16+((cpt>>1)*16)+(cpt>>1) ;
				if(l->ico[l->order[yap]]==2)	((Gfa*)l->icon_s[l->order[yap]])->draw(tm,tm0) ;
				else if(l->ico[l->order[yap]])	((Gfm*)l->icon_s[l->order[yap]])->draw(tm,tm0) ;

				GpTextOut(NULL, &gpDraw[nflip],20+tm,tm0,gerr,color) ;
				yap++ ;	ptr = l->fileName[l->order[yap]] ;
			}
				GpTextOut(NULL, &gpDraw[nflip],45,200,l->entryName[l->order[nb]],0x2A) ;
				GpTextOut(NULL, &gpDraw[nflip],12,-3,dpath,1) ;
				
				sprintf(gerr,"%iko",getFreeRam()>>10);
				drawTextRight(242,218,gerr,1) ;
				sprintf(gerr,"%i/%i",nb+1,l->entryNumber) ;
				drawTextRight(316,-3,gerr,1) ;

				if(l->fileType[l->order[nb]]) {
					if(l->fileSize[l->order[nb]]) {
						if(l->fileSize[l->order[nb]] > 1024)	sprintf(gerr,"%i.%imo",l->fileSize[l->order[nb]]>>10,((l->fileSize[l->order[nb]])%1024)/100) ;
						else							sprintf(gerr,"%iko",l->fileSize[l->order[nb]]) ;
						drawTextRight(316,218,gerr,42) ;
					}
				} else drawTextRight(316,218,"???",42) ;

				if(l->ico[l->order[nb]]==2)		((Gfa*)l->icon[l->order[nb]])->draw(5,240-32-5) ;
				else if(l->ico[l->order[nb]])		((Gfm*)l->icon[l->order[nb]])->draw(5,240-32-5) ;
				flipScreen() ;	clear-- ;
		}

		if(keyUp(kStart)||keyUp(kA)) // launch or view selected file
		{	clear=2 ;	char ext[5], *nme ;	loading=1 ;
			// get file name
			if(!l->rarList)	nme = l->fileName[l->order[nb]] ;
			else			nme = l->entryName[l->order[nb]] ;

			strcpy(ext,nme + strlen(nme)-4) ;
			sprintf(gerr,"%s%s",l->path,nme) ;

					// pcx	(8 bit)
					if((!strcmp(ext,".PCX"))||(!strcmp(ext,".pcx")))
					{	int up = keyPush(kStart);
						uint * pal = savePal();	if(up) overclock();
						uint * pcxPal = getPal(gerr);
						Pixel * yop = loadPcx(gerr) ;
						ushort sx, sy ;	getPcxSize(gerr,&sx,&sy) ;
						if(up) downclock();
						setAndFreePal(pcxPal) ;
						showImg(yop,sx+1,sy+1) ;
						free(yop) ;		setAndFreePal(pal) ;
					}
			else		// gif
					if((!strcmp(ext,".GIF"))||(!strcmp(ext,".gif")))
					{	//int stram = getFreeRam();
						int up = keyPush(kStart); uint * pal = savePal() ;
						if(!l->rarList)
						{	if(up) overclock();
							tGif theGif ;
							if(!gifLoad(gerr,&theGif))
							{	if(up) downclock();	setAndFreePal(theGif.pal) ; 
								showImg((uchar*)(theGif.work->pixels),theGif.width,theGif.height) ;
								free((theGif.work)->pixels) ;	free(theGif.work) ;
								setAndFreePal(pal) ;
							} else if(up)  downclock();
						} else	{	ulong data_size ;	uchar *img ;
								if(up) overclock();
								if(urarlib_get(&img, &data_size, l->entryName[l->order[nb]],l->path,0))
								{	tGif theGif ;
									if(!mgifLoad(img,&theGif))
									{	if(up) downclock();
										setAndFreePal(theGif.pal) ;
										showImg((uchar*)(theGif.work->pixels),theGif.width,theGif.height) ;
										free((theGif.work)->pixels) ;	free(theGif.work) ;
										setAndFreePal(pal) ;
									} else if(up) downclock();
									free(img) ;	
								} else if(up) downclock();
							}
						if(pal) free(pal);
					/*	int diff = stram-getFreeRam();
						if(diff!=0)	{	if(diff<0) diff=-diff ;	sprintf(gerr,"ram leak about\n o  : %i\n ko : %i",diff,diff>>10);
									error(gerr);
								}*/
					}
			else		// fxe
					if((!strcmp(ext,".FXE"))||(!strcmp(ext,".fxe")))
					{	overclock();
						if(!l->rarList)	LaunchFXE(gerr) ;
						else	{	ulong data_size ;
									if(urarlib_get(&data_ptr, &data_size, l->entryName[l->order[nb]],l->path,0))
									{	if(!cancel())	memLaunchFXE((char *)data_ptr);
										else			free(data_ptr) ;
									}
								}
						downclock();
					}
			else		// gxb and axf
					if((!strcmp(ext,".GXB"))||(!strcmp(ext,".gxb"))||(!strcmp(ext,".AXF"))||(!strcmp(ext,".axf")))
					{	overclock();
						char * gxb = 0 ;
						if(!l->rarList)
						{	F_HANDLE file ;
							if(!GpFileOpen(gerr, OPEN_R, &file))
							{	if(!GpFileGetSize(gerr,&fileSize))
								{	if(fileSize > (512<<10) || keyPush(kStart)) overclock();
									gxb = (char*)malloc(fileSize) ;
									if(GpFileRead(file,gxb,fileSize,0)) free(gxb) ;
									
								}

								GpFileClose(file) ;	
							}
						} else	{	ulong data_size ;
								overclock(); urarlib_get(&gxb, &data_size, l->entryName[l->order[nb]],l->path,0); downclock();
							}
						if(gxb)	{	if(!cancel())	GpAppExecute(gxb-4,0) ;
									else			free(gxb) ;
								}
					}
			else		// bin (firmware)
					if((!strcmp(ext,".BIN"))||(!strcmp(ext,".bin")))
					{	overclock();
						char * bin = 0 ;
						if(!l->rarList)
						{	F_HANDLE file ;
							if(!GpFileOpen(gerr, OPEN_R, &file))
							{	if(!GpFileGetSize(gerr,&fileSize))
								{	bin = (char*)malloc(fileSize) ;
									if(GpFileRead(file,bin,fileSize,0)) free(bin) ;
								}
								GpFileClose(file) ;	
							}
						} else {	ulong data_size ;	urarlib_get(&bin, &data_size, l->entryName[l->order[nb]],l->path,0);	}
						if(bin)	{	if(!cancel())	GpAppExecute(bin+32768,0) ;
									else			free(bin) ;
								}
						downclock();
					}
			else		// lha (for mod only, will search the first mod and play it)
					if((!strcmp(ext,".LHA"))||(!strcmp(ext,".lha")))
					{	char * lha=0 ;	ulong data_size ;						
						int up = keyPush(kStart);	if(up) overclock();
						if(!l->rarList)
						{	F_HANDLE file ;
							if(GpFileOpen(gerr, OPEN_R, &file) != SM_OK) error("file open error.") ;
							else	{	if(GpFileGetSize(gerr,&data_size)) error("get size error") ;
										else {	lha = (char*)malloc(data_size) ;
												if(GpFileRead(file,lha,data_size,0)) { error("read error") ; free(mod) ; }
											 }
										GpFileClose(file) ;
									}
						} else 	urarlib_get(&lha, &data_size, l->entryName[l->order[nb]],l->path,0);

						CLhaArchive archive(lha,data_size);
						if(archive.IsArchive()) {
							if(archive.ExtractFile())
							{	mod = archive.GetOutputFile();
								tr4k.load(mod,archive.GetOutputFileLength()) ;
								killKeyFront() ;
							}
						}
						free(lha) ;	if(up) downclock();
					}
			else		// mod
					if((!strcmp(ext,".MOD"))||(!strcmp(ext,".mod")))	// mod pAwa :D
					{	int up = keyPush(kStart);	if(up) overclock();
						if(!l->rarList)
						{	F_HANDLE file ;
							if(GpFileOpen(gerr, OPEN_R, &file) != SM_OK) error("file open error.") ;
							else	{	if(!GpFileGetSize(gerr,&fileSize))
									{		if(!up) if(fileSize > (512<<10)) { up=1; overclock(); } // more than 512ko, auto overclock
											mod = (char*)malloc(fileSize) ;
											if(GpFileRead(file,mod,fileSize,0)) { error("read error") ; free(mod) ; }
									 }	GpFileClose(file) ;
								}
						} else	 urarlib_get(&mod, &fileSize, l->entryName[l->order[nb]],l->path,0);
						if(mod) tr4k.load(mod,fileSize) ;
						if(up) downclock();
					}
			else	// rar (made by winrar 2.x)
					if((!strcmp(ext,".RAR"))||(!strcmp(ext,".rar")))
					{	if(!l->rarList)	{	sprintf(path,"%s\\",gerr) ;
										freeList() ;	loadList(gerr,0,1) ;	setDPath() ;										
										overclock(); parsingList(); downclock();
									}
					}
			else	// tga (8 bit)
					if((!strcmp(ext,".TGA"))||(!strcmp(ext,".tga")))
					{	int up = keyPush(kStart);	if(up) overclock();	char * tga ;
						if(!l->rarList)
						{	F_HANDLE file ;
							if(GpFileOpen(gerr, OPEN_R, &file) != SM_OK) tga=0 ;
							else {	if(!GpFileGetSize(gerr,&fileSize))
									{	tga = (char*)malloc(fileSize) ;
										if(GpFileRead(file,tga,fileSize,0)) free(tga) ;
									}
									GpFileClose(file) ;
								 }
						} else { ulong data_size ; if(!urarlib_get(&tga, &data_size, l->entryName[l->order[nb]],l->path,0)) tga=0 ; }
						int x, y ;	if(mLoadTGA(tga,0,0,&x,&y,fileSize))
									{	uchar * out = (uchar*)malloc(x*y);	uint * pal  = (uint*)malloc(1024);
										if(!mLoadTGA(tga,out,pal,0,0,fileSize)){ if(up) downclock(); error("tga load error :("); }
										else	{	if(up) downclock();
												uint * pl = savePal() ;	setAndFreePal(pal) ;
												showImg(out,y,x) ;	setAndFreePal(pl) ;
											}	free(out) ;	free(pal) ;
									} else  if(up) downclock();
						free(tga) ;
					}
			else	// game boys games
					if((!strcmp(ext+1,".GB"))||(!strcmp(ext+1,".gb"))||(!strcmp(ext,".gbc"))||(!strcmp(ext,".GBC"))||(!strcmp(ext,".GB1"))||(!strcmp(ext,".gb1")))
					{	char * gbRom ;	ulong romSize=0 ;	overclock();
						if(!l->rarList)
						{	F_HANDLE file ;
							if(GpFileOpen(gerr, OPEN_R, &file) != SM_OK) gbRom=0 ;
							else {	if(!GpFileGetSize(gerr,&romSize))
									{	gbRom = (char*)malloc(romSize) ;
										if(GpFileRead(file,gbRom,romSize,0)) free(gbRom) ;
									}
									GpFileClose(file) ;
								 }
						} else	if(!urarlib_get(&gbRom, &romSize, l->entryName[l->order[nb]],l->path,0)) gbRom=0 ;

						if(gbRom)
						{	GpTimerKill(0); trk->stop() ;
							_fgbMain_(gbRom, romSize,gbRom+308) ;
							setPal(gpal);
							free(gbRom) ;
							GpTimerOptSet(0,2,10,parsingList);	GpTimerSet(0); trk->stop() ;
						}	memset(ext,0,5) ; while(keyPush(kA)||keyPush(kStart)) refreshKey() ;
						_downclock();
					}
			else		// maybe a directory
					{		if(!strcmp(l->fileName[l->order[nb]],"..")) // back to parrent '..'
							{	int len = strlen(path) ;
								char * tpath = path + len - 1 ;
								while(*--tpath != '\\') ;
								*(++tpath) = 0 ;
							} else	if(*l->fileName[l->order[nb]] != '.')
									sprintf(path,"%s\\",gerr) ; // not . or .. , add name to the path
								else	freeList(1) ; // refresh list '.'

						GPFILEATTR attr ;	GpFileAttr(path,&attr) ; // get file info						
						if( ( (((attr.attr)>>4)&1) && (!((attr.attr)&1)) )||(!strcmp(path,"gp:\\"))) // is a dir ?						
						{	freeList() ; loadList(path,0,1) ; first=1 ; // free list and load file list of the new path
							if(keyPush(kStart)) // if enter into the directory with start, parse it at 133mhz
							{	overclock(); parsingList(); downclock();	}
						} else	{	if(isPlugin) // not a dir, can maybe be read by a plugin
									{	ulong numPlg ;
										PLGINFO *PlgInfo = PlgSearchByExt(ext+1, &numPlg);
										if(numPlg) PlgLaunch(&PlgInfo[0], gerr, ext+1,0);
										free(PlgInfo);
									} 	// else, back to old and valid path
										int len = strlen(path) ;
										char * tpath = path + len - 1 ;
										while(*--tpath != '\\') ;
										*(++tpath) = 0 ;
								}
						setDPath() ; // update drawed path
					}
			loading=0 ;
		}
	} ;
}
