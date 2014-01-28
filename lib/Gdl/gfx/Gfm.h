/*	gfm file structure.

	header (struct GfmFile)
	'*'
	uint	RLE data size
	uint	RLE colSize size
	uint	RLE scrJump size
	'*'
	uchar	RLE data
	uchar	RLE colSize
	uchar	RLE scrJump
	'*'
	if palette include, 256*2 bytes here to define it.
*/

class GfmInfo
{
  public :
  	ushort	sx, sy			; // 2
	ushort	colNum, jmpNum	; // 2 4
	ushort	TrClr			; // 2 6
  	uint	pixelNum		; // 4 10
	char	bpp				;
	//GfmInfo(void)			;
} ;

struct GfmFile : public GfmInfo
{
	char	header[4]		;
	char	version[4]		;
	char	palInside		;
} ;

class Gfm : public GfmInfo
{
  public :
	Pixel	**	data		;
	uint*		scrJump	;
	ushort*	colSize	;
  	uchar	loaded	;
	ushort*	pal		;
  public :
  	char*	msave(int * s=0) ;
  	void		fsave(int * s, const char * file) ;
  	void		mload(char* dta) ;
 	int		set(Pixel * img, int sx, int sy, Pixel *TrColor=0, char rotated=1) ;
 	int		load(const char * fileName) ;
 	int		saveAs(const char * fileName) ;
 	void		draw(int px, int py) ;
 	void		drawI(int px, int py) { draw(px,240-py); } ;
 	Pixel*	uDraw(Pixel * scr) ;
 	void		showInfo(void) ;
 	void		showBuffer(uint limitSize=20) ;
 	void		kill(void) ;
 	Pixel *	getPixel(int px, int py) ;
 	Pixel *	getBuf(Pixel *TrColor = NULL) ;

  	~Gfm(void)	{ kill() ; } ;
  	Gfm(void) ;

	Gfm(Pixel * img, int sx, int sy, Pixel *TrColor, char rotated=0) { set(img,sx,sy,TrColor,rotated) ; }
	Gfm(Pixel * img, int sx, int sy) {	set(img,sx,sy) ; }
	Gfm(char * fileName) { load(fileName) ; } ;
} ;
