
// hey !! listen banana boat from Jogeir Liljedahl ! great mod

#ifndef	_GDL_
#define	_GDL_	"Gdl2.b"

#define bbp 8	// only 8 bit for now !!

#ifdef __cplusplus
extern "C" {
#endif
	#include "stdlib.h"
	#include "gpstdlib.h"
	#include "gpstdio.h"
	#include "gpos_def.h"
	#include "gpmm.h"
	#include "gpfont.h"
#ifdef __cplusplus
}
#endif

#define	uchar	unsigned char
#define	ushort	unsigned short
#define	uint		unsigned int
#define	ulong		unsigned long

#if (bbp == 16)
	#define 	Pixel	ushort
	#define	pixelSize	2
#else
	#define 	Pixel	uchar
	#define	pixelSize	1
#endif

#define	sprintf		(gp_str_func.sprintf)
#define	strlen		(gp_str_func.gpstrlen)
#define	strcmp		(gp_str_func.compare)
#define	strcpy		(gp_str_func.strcpy)
#define	getFreeRam	(gp_mem_func.availablemem)
#define	memcpy		(gp_str_func.memcpy)
#define	memset		(gp_str_func.memset)

void	(*error)(const char * txt) ;

void free (void * adr)
{	// can only free non null and 4 multiple address
	if(adr) if(!(((int)adr)&3)) { gp_mem_func.free(adr) ;	adr=0 ; }
}

#define	use_PCX
//#define	use_BMP
//#define	use_GFM
#define	usePcxFileOnLoad
//#define	useGfmFileOnLoad
//#define	useBmpFileOnLoad
#define	use_keyEvent

Pixel *	screen[2]	;
Pixel *	scrEnd[2]	;
int		nflip=1		;
char	gerr[2048]	;
uint	blankScr[320*240*pixelSize]	;

GPDRAWSURFACE	gpDraw[2] ;

void * malloc(unsigned size)
{
	void * mem = gm_malloc(size) ;
	if(size)
		if(!mem)
		{	sprintf(gerr,"error on alocating %i bytes",size) ;
			error(gerr) ;
		}
	return mem ;
}

#include "./io/key.cpp"
//#include "./common/dma.c"

//GpClockSpeedChange(16500000, 0x71142, 0); // 16Mhz

void _error(const char * txt)
{	char errBf[4096] ;
	sprintf(errBf,"%s\n\nram free %i\t- press R",txt,getFreeRam());
	memset(screen[!nflip],0xff,320*240*pixelSize);
	#if (bbp==16)
		GpTextOut16(NULL, &gpDraw[!nflip],5,10,(char *)errBf,0xB3);
	#else
		GpTextOut(NULL, &gpDraw[!nflip],5,10,(char *)errBf,0xB3);
	#endif
	waitKey(kR); killKeyFront() ;
}

void showfps(void)
{	static long time=0 ;
	static short fps=0,fps_count=0 ;
	char buffer[20] ;
	static long last_time ;
	long tick = GpTickCountGet() ;
	short frame_time = tick - last_time ;
	last_time = tick ;
	fps++ ; if(tick > (time + 1000)) { time=tick ; fps_count=fps; fps=0 ; }
	sprintf(buffer,"%i|%i",fps_count,frame_time) ;
	GpTextOut(NULL, &gpDraw[nflip],10,10,buffer,0xB3);
}

#define clrScr()	memcpy(screen[nflip],blankScr,320*240)

ushort	getGpColor(uint r, uint g, uint b)
{	r>>=3 ; g>>=3; b>>=3 ;
	return	(r<<11) | (g<<6) | (b<<1) | 1 ;
}

ushort	getGpColor_(uint r, uint g, uint b)
{	return	(r<<11) | (g<<6) | (b<<1) | 1 ;
}

ushort	fadeColorOut(ushort color)
{	uchar r,g,b ;
	r = (color>>11)&0x1F ;	if(r) r-- ;
	g = (color>>6)&0x1F ;	if(g) g-- ;
	b = (color>>1)&0x1F ;	if(b) b-- ;
	if(r>6) r=6 ; if(g>6) g=6 ; if(b>6) b=6 ;
	return	(r<<11) | (g<<6) | (b<<1) | 1 ;
}

ushort	fadeColorIn(ushort color)
{	uchar r,g,b ;
	r = (color>>11)&0x1F ;	if(r<11) r++ ;
	g = (color>>6)&0x1F ;	if(g<11) g++ ;
	b = (color>>1)&0x1F ;	if(b<11) b++ ;
	return	(r<<11) | (g<<6) | (b<<1) | 1 ;
}

#ifdef use_PCX
	#include "./io/pcx.cpp"
#endif

#ifdef use_BMP
	#include "./io/bmp.cpp"
#endif

#include "./gfx/Gfa.cpp"	// anim
#include "./gfx/Gmp.cpp"	// map
#include "./gfx/Gft.cpp"	// font

Gft GdlFont ;

void flipScreen(void)
{	//while((rDSTAT0 >> 20) & 0x03); // wait for dma.
	GpSurfaceFlip(&gpDraw[nflip]) ;
	nflip ^= 1 ;
}
// enable it if you got a flu-, a flu motherboard with a classic screen
#define sFLU 1

void dofadePass(int z=20)
{	uchar r,v,b; uint *pal = (uint*)0x14A00400 ;
#ifdef sFLU
	for (int i=0;i<=0xFF;i++)
	{	r= (pal[i]>>11) & 31 ;
		v= (pal[i]>> 6) & 31 ;
		b= (pal[i]>> 1) & 31 ;
		if (r>z) r-=z ; else r=0 ;
		if (v>z) v-=z ; else v=0 ;
		if (b>z) b-=z ; else b=0 ;
		pal[i] = (r<<11) | (v<<6) | (b<<1) | 0 ;
	};
#endif
}

void undofadePass(int z=20)
{	uchar r,v,b; uint *pal = (uint*)0x14A00400 ;
	int limit = 31-z ;
	for (int i=0;i<=0xFF;i++)
	{	r= (pal[i]>>11) & 31 ;
		v= (pal[i]>> 6) & 31 ;
		b= (pal[i]>> 1) & 31 ;
		if (r<limit) r+=z ; else r=31 ;
		if (v<limit) v+=z ; else v=31 ;
		if (b<limit) b+=z ; else b=31 ;
		pal[i] = (r<<11) | (v<<6) | (b<<1) | 0 ;
	};
}

void initScreen(void)
{	static int first=1 ;
	GpGraphicModeSet(bbp,NULL) ;
	dofadePass(20) ;
	if(!first) return ; else	first=0 ;
	GpLcdSurfaceGet(&gpDraw[0], 0);
	GpLcdSurfaceGet(&gpDraw[1], 1);
	GpSurfaceSet(&gpDraw[0]);
	screen[0] = (Pixel*)gpDraw[0].ptbuffer ;
	screen[1] = (Pixel*)gpDraw[1].ptbuffer ;
	scrEnd[0] = screen[0] + 320*240*pixelSize ;
	scrEnd[1] = screen[1] + 320*240*pixelSize ;
	memset(blankScr,0xFF,320*240*pixelSize) ;
	error = _error ;
}

void rstScreen(void)
{	//char cpy[320*240] ;
	//memcpy(cpy,screen[!nflip],320*240);
	GpGraphicModeSet(bbp,NULL) ;
	//GpSurfaceFlip(&gpDraw[!nflip]) ;
	dofadePass(20) ;
	//GpPcmInit(soundBuf.freq,PCM_16BIT); //22,8
	//memcpy(screen[!nflip],cpy,320*240);
}

#define _palSize	(256*sizeof(uint))

void setPal(uint *pal)	{ memcpy((uint*)0x14A00400,pal,_palSize); dofadePass() ; }
void setPal(uint *pal, int nb)	{ memcpy((uint*)0x14A00400,pal,(nb-1)*sizeof(uint)); memset(pal+nb*sizeof(uint),0xFF,(256-nb)*sizeof(uint)) ; dofadePass() ; }

uint * savePal(void)	{	undofadePass();	uint * pal = (uint*)malloc(_palSize); memcpy(pal,(uint*)0x14A00400,_palSize); dofadePass(); return pal ; }
void setAndFreePal(uint *pal)	{ setPal(pal) ; free(pal) ; }

uint * getPal(const char * fileName)
{
	F_HANDLE file;
	uint	 i,r,g,b ;
	uint	*pal, *palPtr ;
	ushort	colorRead=256 ;
	uchar	*buf, colorSize=3, vgaInverted=0 ;
	char	 ext[5] ;

	strcpy(ext,fileName+strlen(fileName)-4) ;

	if(SM_OK!=GpFileOpen(fileName, OPEN_R, &file)) { sprintf(gerr,"unable to open file\n%s",fileName); error(gerr); return 0 ; }
	pal = palPtr = (uint*)malloc(1024) ;
	memset(pal,0xFF,1024);

	if((!strcmp(ext,".BMP"))||(!strcmp(ext,".bmp"))) // case a bmp file
	{
		GpFileSeek(file,FROM_BEGIN,54,0);
		vgaInverted=1 ;
		colorSize=4 ;
	}
		else
		
			if((!strcmp(ext,".PCX"))||(!strcmp(ext,".pcx"))) // case a pcx file
			{
				uchar palType ;
				GpFileSeek(file, FROM_END, -769,0);
				GpFileRead(file,&palType,1,0); // palette type
				if(palType!=12) 	// 16 color palette
				{	colorRead = 16 ;
					GpFileSeek(file, FROM_BEGIN, 16, 0);
				}
			}	else	colorRead=0 ;

	ushort size = colorSize*colorRead ;
	buf = (uchar*)malloc(size) ;
	GpFileRead(file,buf,size,0);
	GpFileClose(file) ;
	
	for(i=0;i<colorRead;i++)
	{	
		r = (buf[colorSize*i])>>3	;
		g = (buf[colorSize*i+1])>>3	;
		b = (buf[colorSize*i+2])>>3	;
		
		if(!vgaInverted)	*palPtr++ = (r<<11)|(g<<6)|(b<<1)|0 ;
		else				*palPtr++ = (b<<11)|(g<<6)|(r<<1)|0 ;
	};

	free(buf) ;
	return pal ;
}

void rectFill(int x, int y, int sx, int sy, Pixel color)
{	if(x<0)
	{
		if(x+sx < 0) return ;
		sx+=x ; x=0 ;
	}
	if(y<0)
	{
		if(y+sy < 0) return ;
		sy+=y ; y=0 ;
	}
	if(x+sx > 319)
	{	if(x>319) return ;
		sx += x-319 ;
	}
	if(y+sy > 239)
	{
		if(y>239) return ;
		sy = 239-y ;
	}
	Pixel * s = screen[nflip] + x*240 + 239-y ;
	while(sx--){ s-=sy ; memset(s,color,sy) ; s+=240+sy ; } ;
}

void strAdd(char * dst, const char * s1, char *s2)
{ while((*dst++ = *s1++)!=0) ; dst-- ; while((*dst++ = *s2++)!=0) ; }

void rotate90r(uchar * s, uchar * d, int sx, int sy)
{	int cptx=sx ; uchar * dd = d ; d+=(sx-1) ;
	while(cptx--)
	{	for(int cpty=0;cpty<sy;cpty++)
		{	*d = *s++ ; d+=sx ;	};
		d = dd+(cptx-1) ;
	};
}

void pause(int ms)
{	ms*=2 ;
	uint ticks = GpTickCountGet();
	while(GpTickCountGet()-ticks<ms);
}

#define HEIGHT 240
#define WIDTH 320

/*
int abs(int nb)
{	if(nb<0)nb*=-1;
	return nb;
}*/

void ligne(int yi,int xi,int yf,int xf, Pixel *line) {
Pixel *p = screen[nflip] + xi*HEIGHT + yi ;
int dcx, xclip=0 ;
if(xf<0){ xclip=1 ;
          dcx=xi ;
} else if(xf>WIDTH-1){
          xclip=1 ;
          dcx = (WIDTH-1)-xi ;
       }
int dx,dy,i,xinc,yinc,cumul,x,y ;
x = xi ;
y = yi ;
dx = xf - xi ;
dy = yf - yi ;
xinc = ( dx > 0 ) ? 1 : -1 ;
yinc = ( dy > 0 ) ? 1 : -1 ;
dx = abs(dx) ;
dy = abs(dy) ;
*p = *line++ ;
if ( dx > dy ) {
  cumul = dx>>1 ;
  for ( i = 1 ; i <= dx ; i++ ) {
    p += xinc*HEIGHT ;  if(xclip) if(!dcx--) return ;
    cumul += dy ;
    if (cumul >= dx) {
      cumul -= dx ;
      p += yinc ;
    }
    *p = *line++ ;     } }
  else {
  cumul = dy>>1 ;
  for ( i = 1 ; i <= dy ; i++ ) {
    p += yinc ;
    cumul += dx ;
    if ( cumul >= dy ) {
         cumul -= dy ;
         p += xinc*HEIGHT ; if(xclip) if(!dcx--) return ;
       }
    *p = *line++ ;
     } }
}

#endif

