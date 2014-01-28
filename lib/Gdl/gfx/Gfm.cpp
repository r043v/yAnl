/** Gfm.cpp *******************************************************************/

/* fast 8 bit blit routine, assume draw 20000 32*32 transparent sprites in 666 tick, demoniak ? */

#ifndef	_gfm_
#define	_gfm_

#include "Gfm.h"
//#include "../common/dma.c"
#include "../common/rle.c"

char* mSaveGfm(Gfm *frm,int nb,int*s)
{	char* frmd[150] ;
	int frms[150] ; int tsize=0 ;
	for(int c=0;c<nb;c++)	{	frmd[c] = frm[c].msave(&(frms[c])) ; tsize+=frms[c]; }
	char *out = (char*)malloc(tsize+4+nb*4) ;
	int* ptr = (int*)out ;	*ptr++ = nb ;
	for(int c=0;c<nb;c++)	*ptr++ = frms[c] ;
	for(int c=0;c<nb;c++)	{	memcpy(ptr,frmd[c],frms[c]) ; ptr = (int*)(((char*)ptr) + frms[c]) ;	};
	if(s) *s = (int)((char*)ptr-out) ;
	return out ;
}

void fSaveGfm(Gfm *frm,int nb, const char*file)
{	int size=0 ; F_HANDLE f ;
	char *out = mSaveGfm(frm,nb,&size);
	if(SM_OK != GpFileCreate (file, ALWAYS_CREATE, &f))
		{ error("unable to save gfms."); return;}
	GpFileWrite(f,out,size);
	GpFileClose(f) ;
}

Gfm* mloadGfm(char*dta)
{	int nb = *(int*)dta ;	int * ptr = (int*)(dta+4) ;
	Gfm* frm = (Gfm*)malloc(sizeof(Gfm)*nb) ;
	char * p = dta + 4 + nb*4 ;
	for(int c=0;c<nb;c++)	{	frm[c].Gfm() ;
						frm[c].mload(p) ;
						p+=*ptr++ ;
					};
	return frm ;
}

inline Pixel* Gfm::uDraw(Pixel	*scr)	// blit unclipped on y
{
	int	colNb = colNum	;
	ushort	*sz  	=	colSize		;
	uint	*jump	=	scrJump		;
	Pixel	**dta	=	data		;
	//Pixel	*scr	=	(Pixel*)(screen[nflip] + px*240 + (239-py)) ;
	int yop, temp ;
	Pixel* scrend = scrEnd[nflip]	;
	
	int clp = !(scr + sx*240 < scrend) ;	// is clipped at right ?
	
	while(scr < screen[nflip] - *jump)	// left clipping
	{
		if(!(--colNb)) return 0 ;
		scr += *jump++ ;
		scr += *sz++ ;	dta++ ;
	};

	while(colNb--)
	{	scr += *jump++ ;	// jump into screen memory to go where blit colon
		temp = yop = *sz++ ; // get colon size
		if(clp) if(scr+yop >= scrend) { if(scr > scrend) return 0 ; colNb=0 ; yop = scrEnd[nflip] - scr ; } ; // right clipping
		if(yop > 5)	// size more than 5 pixel ?
		{	if(!(((int)scr)&1))	// pair adress
			{	if(!(((int)scr)&3))	// 4 multipl adress, 32 bits copie
				{	int * dataPtr	= (int *)(*dta) ;
					int m 			= yop&3 ;
					int * scrn		= (int*)scr ;	scr+=yop ;
					temp = yop>>4 ;	yop-=temp*16 ;
					while(temp--)	{	*scrn++ = *dataPtr++ ;	*scrn++ = *dataPtr++ ;
										*scrn++ = *dataPtr++ ;	*scrn++ = *dataPtr++ ;
									};
					temp = yop>>2 ;		while(temp--)	*scrn++ = *dataPtr++ ;
					if(m)	{	char * _data = (char*)dataPtr ;	scr-=m ;
								while(m--)		*scr++ = *_data++ ;
					}
				}	else	// 2 multipl adress, 16 bits copie
							{	short * dataPtr	= (short*)*dta ;
								int m = yop&1 ;
								short * scrn		= (short*)scr ;	scr+=(yop-m) ;
								temp = yop>>4 ;	yop -= temp*16 ;
								while(temp--)	{	*scrn++ = *dataPtr++ ;	*scrn++ = *dataPtr++ ;
													*scrn++ = *dataPtr++ ;	*scrn++ = *dataPtr++ ;
													*scrn++ = *dataPtr++ ;	*scrn++ = *dataPtr++ ;
													*scrn++ = *dataPtr++ ;	*scrn++ = *dataPtr++ ;
												};
								temp = yop>>1 ;		while(temp--)	*scrn++ = *dataPtr++ ;
								if(m)	*scr++ = *(char*)dataPtr ;
							}
			} else	// impair adress, copy pixel by pixel
					{	char * dataPtr = (char*)*dta ;
						temp = yop>>3 ;	yop -= temp*8 ;
						while(temp--)	{	*scr++ = *dataPtr++ ;	*scr++ = *dataPtr++ ;
											*scr++ = *dataPtr++ ;	*scr++ = *dataPtr++ ;
											*scr++ = *dataPtr++ ;	*scr++ = *dataPtr++ ;
											*scr++ = *dataPtr++ ;	*scr++ = *dataPtr++ ;
										}
						if(yop>3)		{	yop -= 4 ;
											*scr++ = *dataPtr++ ;	*scr++ = *dataPtr++ ;
											*scr++ = *dataPtr++ ;	*scr++ = *dataPtr++ ;
										}
						while(yop--)		*scr++ = *dataPtr++ ;
					}
		}	else	{	// size < 6 pixels, copie byte after byte
						char * dataPtr = (char*)*dta	;
						while(yop--)	*scr++ = *dataPtr++		;
					}
		dta++ ;		// jump to next colon
	};	return (Pixel*)scr + *jump + sy - 240;	// return screen adress of end blitting (-> next tile pos if a map),	still a bug here
}

inline void Gfm::draw(int px, int py)	// clipped blit
{
	uint colNb = colNum ;
	ushort	*sz   = colSize;
	Pixel	**dta  = data;
	uint	*jump = scrJump ;
	int		decUp=0, decDown=0 ;
	register int yop ;
	register int temp ;
	Pixel* scrend = scrEnd[nflip]	;
	py=239-py ;	// invert y screen pos
	
	Pixel *scr = screen[nflip] + (px*240) + py ;

	if(px < -sx) return ;	// too more at left, return
		
	if(py-sy<0)	{	decDown = py-sy ;	if(decDown < 0) decDown=-decDown;
											if(decDown >= sy) return ;
				}	else	if(py > 239){	decUp = py-240 ;	if(decUp >= sy) return ;	}
							else {	uDraw(scr) ; return ; }

	int spriteTCount = *jump ;
	while(spriteTCount >= 240-sy) spriteTCount -= 240-sy ;

	int colEnd ;
	int blitSz, tmp ;

	int clp = !(scr + sx*240 + sy < scrEnd[nflip]) ;	// is clipped at right ?
	
	scr += *jump++ ;
	int jmp, _tmp ;

	while(scr+(*sz) < screen[nflip]) // left clipping
	{
		if(!(--colNb)) return ;
		_tmp = *jump ;	while(_tmp >= 240-sy) _tmp -= 240-sy ;
		spriteTCount += *sz + _tmp ;
		scr+=*sz++ + *jump++ ;	dta++ ;
	};

	Pixel *dtaPtr = *dta ;
//if(colNb > 16) { sprintf(err,"colNb > 16 !! %i",colNb) ; error(err) ; }
	while(colNb--)
	{	yop = *sz++ ;	if(clp) if(scr+yop >= scrend) { if(scr > scrend) return ; colNb=0 ; yop = scrEnd[nflip] - scr ; } ; // right clipping
		while(spriteTCount >= sy) spriteTCount -= sy ;
		colEnd = spriteTCount + yop ;
		_tmp = *jump ; if(_tmp > 1024){  _tmp = *jump = 0 ; }//sprintf(err,"b jump %i",_tmp) ; error(err) ; }
		//while(_tmp >= 240-sy) _tmp -= 240-sy ;

		if((spriteTCount >= decDown)&&(sy-colEnd >= decUp)) // if colon to blit is completely in draw area
		{	// see up for english comment ....
			if(yop > 5)	// si la taille a copier est superieure a 7 pixels
			{	if(!(((int)scr)&1))	// adresse paire
				{	if(!(((int)scr)&3))	// adresse multiple de 4, copie en 32 bit
					{	int * dataPtr	= (int *)dtaPtr ;
						int m 			= yop&3 ;
						int * scrn		= (int*)scr ;	scr+=yop ;
						temp = yop>>4 ;	yop-=temp*16 ;
						while(temp--)	{	*scrn++ = *dataPtr++ ;	*scrn++ = *dataPtr++ ;
											*scrn++ = *dataPtr++ ;	*scrn++ = *dataPtr++ ;
										};
						temp = yop>>2 ;		while(temp--)	*scrn++ = *dataPtr++ ;
						if(m)	{	char * _data = (char*)dataPtr ;	scr-=m ;
									while(m--)		*scr++ = *_data++ ;
						}
					}	else	// adresse paire et non multiple de 4, copie en 16 bit
								{	short * dataPtr	= (short*)dtaPtr ;
									int m = yop&1 ;
									short * scrn		= (short*)scr ;	scr+=(yop-m) ;
									temp = yop>>4 ;	yop -= temp*16 ;
									while(temp--)	{	*scrn++ = *dataPtr++ ;	*scrn++ = *dataPtr++ ;
														*scrn++ = *dataPtr++ ;	*scrn++ = *dataPtr++ ;
														*scrn++ = *dataPtr++ ;	*scrn++ = *dataPtr++ ;
														*scrn++ = *dataPtr++ ;	*scrn++ = *dataPtr++ ;
													};
									temp = yop>>1 ;		while(temp--)	*scrn++ = *dataPtr++ ;
									if(m)	*scr++ = *(char*)dataPtr ;
								}
				} else	// l'adresse est impaire, copie en 8 bit
						{	temp = yop>>3 ;	yop -= temp*8 ;
							while(temp--)	{	*scr++ = *dtaPtr++ ;	*scr++ = *dtaPtr++ ;
												*scr++ = *dtaPtr++ ;	*scr++ = *dtaPtr++ ;
												*scr++ = *dtaPtr++ ;	*scr++ = *dtaPtr++ ;
												*scr++ = *dtaPtr++ ;	*scr++ = *dtaPtr++ ;
											}
							if(yop>3)		{	yop -= 4 ;
												*scr++ = *dtaPtr++ ;	*scr++ = *dtaPtr++ ;
												*scr++ = *dtaPtr++ ;	*scr++ = *dtaPtr++ ;
											}
							while(yop--)		*scr++ = *dtaPtr++ ;
						}
			}	else	{	// taille inferieure a 8 pixels, copie en 8 bit
							//if(yop > 16) { sprintf(err,"> 16 !! %i",yop) ; error(err) ; }
							while(yop--)	*scr++ = *dtaPtr++		;
						}
			scr += *jump++ ;
		}	else	if((colEnd > decDown)&&(sy-spriteTCount > decUp))	// blit only a part of the colon
					{	// else, a part or all the colon is out of screen
						jmp = *jump++;

						if(decDown)
						{	blitSz = (colEnd - decDown) ;
							tmp = yop - blitSz ; // screen out at down
							scr+=tmp ; dtaPtr+=tmp ;
						}	else	{	if(colEnd > sy-decUp)	{	blitSz = yop-(colEnd-(sy-decUp)) ;	jmp+= yop-blitSz ;	}
										else	blitSz = yop ;
									}
								
						temp = blitSz>>3 ;	blitSz -= temp*8 ;
						while(temp--)	{	*scr++ = *dtaPtr++ ;	*scr++ = *dtaPtr++ ;
											*scr++ = *dtaPtr++ ;	*scr++ = *dtaPtr++ ;
											*scr++ = *dtaPtr++ ;	*scr++ = *dtaPtr++ ;
											*scr++ = *dtaPtr++ ;	*scr++ = *dtaPtr++ ;
										}
						if(blitSz>3)	{	blitSz -= 4 ;
											*scr++ = *dtaPtr++ ;	*scr++ = *dtaPtr++ ;
											*scr++ = *dtaPtr++ ;	*scr++ = *dtaPtr++ ;
										}
						while(blitSz--)	*scr++ = *dtaPtr++ ;

						//for(temp=0;temp<blitSz;temp++) *scr++ = *dtaPtr++ ;
						scr += jmp ;
					}	else	scr += yop + *jump++ ;	// else, colon not need to be blitted
		dtaPtr = *++dta ;
		spriteTCount = colEnd + _tmp ;
	};
}

Gfm::Gfm()
{
 	colNum = sx = sy = 0 ;
	scrJump = NULL ;
	colSize = NULL ;
	data = NULL ;
	loaded=0 ; pal=NULL ;
}

void Gfm::kill(void)
{	if(data)
	{	while(colNum--) free(data[colNum]) ; 
		free(data) ;
	} ;
	
	if(colSize)		free(colSize) ;
	if(scrJump)	free(scrJump) ;
	
	loaded=0 ;
}

int Gfm::set(Pixel * img, int szx, int szy, Pixel *TrColor, char rotated)
{
	ushort	*clSzPtr ;
	uint	*jmpPtr ;
	Pixel	*imgPtr,  *imgEnd, **dtaPtr ;
	uint	imgSize, clNumber=0, jpNumber=0, totalSize=0, count=0, max, jump ;

	if(loaded) kill() ;
	if(!rotated) return 0 ;
	if(!img) { error("Gfm::set, your data pointer is null") ; return 0 ; }

 	sx = szx ;
 	sy = szy ;

 	if(TrColor)	TrClr = *TrColor ;
 	else		TrClr = *img ;

 	imgPtr  = img ;
 	imgSize = sx*sy*sizeof(Pixel) ;
  	imgEnd  = imgPtr + imgSize ;

	// fast scan
	do {
		max = sy - ((imgPtr-img)%sy) ;
		while((imgPtr < imgEnd)&&(*imgPtr == TrClr)) { count++ ; imgPtr++ ; }
		jpNumber++ ;
		count=0;		
		max = sy - ((imgPtr-img)%sy) ;
		while((imgPtr < imgEnd)&&(*imgPtr != TrClr)&&(max--)) { count++ ; imgPtr++ ; }
		if(count) { clNumber++ ; totalSize+=count ; count=0 ; }
	} while(imgPtr < imgEnd) ;

	jmpPtr  = scrJump	=	(uint*)		malloc(jpNumber*sizeof(uint))	;
	clSzPtr = colSize	=	(ushort*)	malloc(clNumber*sizeof(ushort))	;
	dtaPtr  = data		=	(Pixel**)	malloc(clNumber*sizeof(Pixel*))	;

	colNum = clNumber ;
	jmpNum = jpNumber ;
	pixelNum = totalSize ;

	count=0;
	imgPtr = img ;
	Pixel * tmpBuf = (Pixel*)malloc(10<<10) ; // 10k temp buffer
	Pixel * tmpPtr = tmpBuf ;
	int tmpClNum = 0 ;
	
	while(imgPtr < imgEnd)
	{
		max = sy - ((imgPtr-img)%sy) ; // transparent scan
		while((imgPtr < imgEnd)&&(*imgPtr == TrClr)) { count++ ; imgPtr++ ; }
		if(count>=max)	// colon change
		{
			jump = max+(240-sy) ;
			count -= max ;
			while(count >= sy) { jump += 240 ; count-=sy ; }
			jump += count ;
		} else jump = count;
		if(max==sy) jump += (240-sy) ;
		*jmpPtr++ = jump ;
		count=0;
		
		max = sy - ((imgPtr-img)%sy) ;	// untransparent scan
		while((imgPtr < imgEnd)&&(*imgPtr != TrClr)&&(max--)) { *tmpPtr++ = *imgPtr++ ; count++ ;}
		if(count)
		{	*clSzPtr++ = count ;
			dtaPtr[tmpClNum] = (Pixel*)malloc(count) ;
			memcpy(dtaPtr[tmpClNum],tmpBuf,count) ;
			tmpPtr = tmpBuf ;
			tmpClNum++ ;
			count=0 ;
		}
	};
	
	free(tmpBuf) ;	
	return loaded=1 ;
}

char * Gfm::msave(int * s) // save the frame into ram
{	char * rtn = (char*)malloc(10+pixelNum+(colNum*5)+(jmpNum*4)) ;
	*(uint*)rtn = pixelNum ;	*(ushort*)(rtn+4) = sx ;	*(ushort*)(rtn+6) = sy ;
	*(ushort*)(rtn+8) =	colNum ;*(ushort*)(rtn+10)=	jmpNum ;
	memcpy(rtn+12,scrJump,jmpNum*4) ;	memcpy(rtn+12+(jmpNum*4),colSize,colNum*2) ;
	int pos = 12+(jmpNum*4)+(colNum*2) ;
	for(int c=0;c<colNum;c++){	if(((int)(rtn+pos))&3) pos += 4-(((int)(rtn+pos))&3) ;
								memcpy(rtn+pos,data[c],colSize[c]) ;	pos+=(colSize[c]) ;
								//if(colSize[c]&3){ sprintf(err,"%i -> +%i",colSize[c]&3,4-(colSize[c]%4)) ; error(err) ;
								 //pos += 4-(colSize[c]%4) ;	// align next to 4o if was not.
								 //}
							 };
	*(rtn+pos)	= '*' ; pos++ ;	if(((int)(rtn+pos))&3) pos += 4-(((int)(rtn+pos))&3) ;
	if(s) *s=pos ;
	return rtn ;
}

void Gfm::fsave(int * s, const char * file)
{	F_HANDLE f ;	if(SM_OK != GpFileCreate (file, ALWAYS_CREATE, &f))
					{ error("unable to save gfm."); return;}
	char* i = msave(s);
	int size = 10+pixelNum+(colNum*5)+(jmpNum*4);
	GpFileWrite(f,i,size);
	GpFileClose(f) ;
}

void Gfm::mload(char *dta) // load the frame from ram
{	//kill() ;
	//if( ((int)dta) &3) error("not aligned.") ;
	
	pixelNum	= *(uint*)dta ;
	sx			= *(ushort*)(dta+4) ;
	sy			= *(ushort*)(dta+6) ;
	colNum		= *(ushort*)(dta+8) ;
	jmpNum		= *(ushort*)(dta+10);
	TrClr		= 0xef ;
	scrJump		= (uint*)(dta+12)  ;
	colSize		= (ushort*)(dta+12+(jmpNum*4)) ;
	int pos = 12+(jmpNum*4)+(colNum*2) ;
	data		= (Pixel**)malloc(colNum*sizeof(Pixel*)) ;
	for(int c=0;c<colNum;c++)	{	if(((int)(dta+pos))&3) pos += 4-(((int)(dta+pos))&3) ;
									data[c] = (Pixel*)(dta+pos) ;	pos+=(colSize[c]) ;
									//if(colSize[c]&3) pos += 4-(colSize[c]%4) ;
								};
	//if(*(dta+pos) != '*') error("mload error.") ;
}


// non converti a la nouvelle classe

/*Pixel *	Gfm::getBuf(Pixel *TrColr)
{
	Pixel	*resBuf, TrColor, **dataPtr = data, *bufStart ;
	ushort	*sizePtr = colSize ;
	uint	*jumpPtr = scrJump ;
	uint	buf, cpt, size=0, jump, bigJump = 240-sy ;

	if(!loaded) return 0 ;

	if(!TrColr)	TrColor = TrClr ;
	else		TrColor = *TrColr ;

	bufStart = resBuf = (Pixel*)malloc(sx*sy*pixelSize) ;

	if(!bufStart)
	{ error("alloc error in Gfm::getBuf.") ; return NULL ; }

	for(cpt=0;cpt<colNum;cpt++) {
		jump = *jumpPtr ; jumpPtr++;
		while(jump >= bigJump)
		{
			*jumpPtr -= bigJump ;
		};
		buf = jump ;
		while(buf) { *(resBuf) = TrColor ; resBuf++ ; buf-- ; size++ ; } ;
		buf = *sizePtr ; sizePtr++ ;
//		while(buf) { *(resBuf) = *dataPtr ; resBuf++ ; dataPtr++ ; buf-- ; size++ ; } ;
	};

   while(size < sx*sy) { *(resBuf) = TrColor ; resBuf++ ; size++ ; } ;

   return bufStart ;
}

int	Gfm::saveAs(const char * fileName)
{
	F_HANDLE file ;
	struct	GfmFile GfmFile ;
	Pixel	*dtaPtr ;
	ushort	*szPtr ;
	uint	*jmpPtr ;
	uint	sz, jmp, dta ;
	char	ext[4] ;

	if(!loaded) return 0 ;

	strcpy(ext,fileName+strlen(fileName)-3) ;

	if(!strcmp("gfm",ext)) // case a gfm file
	{
		strcpy(GfmFile.header,"Gfm\0") ;
		strcpy(GfmFile.version,"2.3\0") ;
		GfmFile.TrClr	 = TrClr ;
	  	GfmFile.sx		 = sx	;
	  	GfmFile.sy		 = sy	;
	  	GfmFile.pixelNum = pixelNum	;
	  	GfmFile.colNum	 = colNum	;
		GfmFile.bpp 	 = bbp	;
		GfmFile.jmpNum	 = jmpNum	;
		
		GpFileCreate(fileName, ALWAYS_CREATE, &file);
		
		// open file in write
		GpFileOpen(fileName, OPEN_W, &file);

		// write header
		GpFileWrite(file,&GfmFile,sizeof(struct GfmFile)) ;
		GpFileWrite(file,"*",1) ;

		// get data, scrJump and colSize buffer size, compress them in RLE.
		dta = pixelNum ;
		sz =  colNum * sizeof(ushort) ;
		jmp = jmpNum * sizeof(uint) ;

		dtaPtr	= (Pixel*)	RLE((void*)data,&dta) ;
		szPtr	= (ushort*)	RLE((void*)colSize,&sz) ;
		jmpPtr	= (uint*)	RLE((void*)scrJump,&jmp) ;
		
		// write RLE buffers's size
		GpFileWrite(file,&dta,sizeof(uint)) ;
		GpFileWrite(file,&sz, sizeof(uint)) ;
		GpFileWrite(file,&jmp,sizeof(uint)) ;
		GpFileWrite(file,"*",1) ;

		// write RLE buffers's data
		GpFileWrite(file,dtaPtr,dta) ;
		GpFileWrite(file,szPtr,sz) ;
		GpFileWrite(file,jmpPtr,jmp) ;
	 	GpFileWrite(file,"*",1) ;

	 	// close file
		GpFileClose(file) ;
		return 1 ;
	}
	return 0 ;
}

void	Gfm::showInfo(void)
{
	char string[1024] ;
	char dtaStr[128]={0};
	char jmpStr[128]={0};
	char szeStr[128]={0};
	for(int cpt=0;cpt<10;cpt++)
	{
		//sprintf(dtaStr,"%s %x",dtaStr,(Pixel)data[cpt]) ;
		sprintf(jmpStr,"%s %i",jmpStr,(ushort)scrJump[cpt]) ;
		sprintf(szeStr,"%s %i",szeStr,(ushort)colSize[cpt]) ;
	};
	
	sprintf(string,"info --\nsx %i | sy %i\n%i colon | %i jump\ntransp. color %i\npixel number %i - bpp %i\n\ndata %s\njump %s\nsize %s",sx,sy,colNum,jmpNum,TrClr,pixelNum,bpp,dtaStr,jmpStr,szeStr);
	error(string) ;
}

*/
int	Gfm::load(const char * fileName)
{
	Pixel	* img ;
	char	ext[4] ;
	strcpy(ext,fileName+strlen(fileName)-3) ;

	kill() ;	// free object old content
	img=NULL ;

	// get file ext.
	//while(*ext != '\0') ext++ ;
	//while((*ext != '.')&&(ext != fileName)) ext--  ; ext++ ;

	#ifdef use_PCX
		#ifdef usePcxFileOnLoad
			if(!strcmp("pcx",ext)) // case a pcx file
			{
				img = loadPcx(fileName)		;
				if(!img) { error("load pcx error") ; return 0 ; }
		  		loaded = set(img,getPcxSize(fileName,'x'),getPcxSize(fileName,'y')) ;
		  		free(img) ;	return loaded ;
		    }
		#endif
	#endif

	#ifdef use_BMP
		#ifdef useBmpFileOnLoad
			if(!strcmp("bmp",ext)) // case a bmp file
			{
				img = loadBmp(fileName)		;
				if(!img) { error("load bmp error") ; return 0 ; }
		  		loaded = set(img,getBmpSize(fileName,'x'),getBmpSize(fileName,'y')) ;
		  		free(img) ;	return loaded ;
		    }
		#endif
	#endif

	#ifdef useGfmFileOnLoad	// pas encore porté
		if(!strcmp("gfm",ext))	// case a Gfm file
		{
			F_HANDLE file ;
			GfmFile * GfmFile ;
			char *buffer, *bf ;
			uint dta,sz,jmp,tmp ;
			ulong readCount, fileSize ;

			// open file in read
			if(SM_OK != GpFileOpen(fileName, OPEN_R, &file)) { sprintf(err,"unable to open file\n%s",fileName); error(err); return 0 ; }
			// get file size
			if(SM_OK != GpFileGetSize(fileName,&fileSize)) { error("unable to get size."); return 0 ; }
			if(fileSize < 42)	{ error("load : file is too small."); return 0 ; }
			// alloc buffer, assign bf ptr with the buffer adress
			bf = buffer = (char*)malloc(fileSize) ;
			if(!bf)
			{
				sprintf(err,"load : error on alocate %i bytes.",(int)fileSize) ;
				error(err) ;	return 0 ;
			}
			// load file into buffer
			GpFileRead(file,buffer,fileSize,&readCount);
			if(readCount != fileSize) { error("load : read error."); return 0 ; }
			GpFileClose(file) ;
			// affect GfmFile header
			GfmFile = (struct GfmFile*)buffer ; buffer += sizeof(struct GfmFile) ;
			// test control char : '*'
			if(*buffer != '*') { error("load : control char error."); return 0 ; }
			buffer++ ;
			// read the 3 compressed buffer size
			memcpy(&dta,buffer,4);
			memcpy(&sz, buffer+4,4);
			memcpy(&jmp,buffer+8,4);
			buffer+=12;
			// test control char
			if(*buffer != '*') { error("load : control char error."); return 0 ; }
			buffer++ ;
			// read rle, uncompress and affect.
			tmp = dta ;
			data	=	(Pixel*)unRLE((char*)buffer,&dta,NULL)	;	buffer += tmp ; tmp = sz ;
			colSize	=	(ushort*)	unRLE((char*)buffer,&sz,NULL)	;	buffer += tmp  ; tmp = jmp ;
			scrJump	=	(uint*)	unRLE((char*)buffer,&jmp,NULL)	;	buffer += tmp ;
			// test control char
			if(*buffer != '*') { error("load : control char error."); return 0 ; }
			// assign object properties.
			pixelNum = GfmFile->pixelNum ;	TrClr	= GfmFile->TrClr 	;
	  		sx 		 = GfmFile->sx		 ;	sy		= GfmFile->sy		;
	  	  	colNum	 = GfmFile->colNum	 ;	loaded	= 1					;
	  	  	jmpNum	 = GfmFile->jmpNum	 ;	bpp		= GfmFile->bpp		;
	  	  	// free buffer and return ok
	  	  	free(bf) ;	return 1 ;
	    }
	#endif
	
    // an unsupported file format return an error.
    sprintf(gerr,"* unknow file ext. (%s)",ext) ;	error(gerr) ;
    return 0 ;
}

#endif