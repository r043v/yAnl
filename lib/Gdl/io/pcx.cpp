#include "rle.c"
ulong p_read_count ;

typedef struct
{
   uchar	Id;
   uchar	Version;
   uchar	Format;
   uchar   	BitsPointPlane;
   ushort   Xmin;
   ushort   Ymin;
   ushort   Xmax;
   ushort   Ymax;
   ushort   Hdpi;
   ushort   Vdpi;
   uchar   	EgaPalette[48];
   uchar   	Reserved;
   uchar   	NumberOfPlanes;
   ushort   ucharsLinePlane;
   ushort   PaletteInfo;
   ushort   HScreenSize;
   ushort   VScreenSize;
   uchar   	Filler[54];
} pcxFileHeader ;

uchar get_color_buffer8(uint x, uint y, uint size_y, uchar *buffer)
{
	return *(buffer + x*size_y + (size_y-1-y)) ;
}

void set_color_buffer8(uint x, uint y, uint size_y, uchar *buffer, uchar color)
{
	*(buffer + x*size_y + (size_y-1-y)) = color ;
}

ushort get_color_buffer16(uint x, uint y, uint size_y, ushort *buffer)
{
	return *(buffer + x*size_y + (size_y-1-y)) ;
}

void set_color_buffer16(uint x, uint y, uint size_y, ushort *buffer, ushort color)
{
	*(buffer + x*size_y + (size_y-1-y)) = color ;
}

void rotate_90(void * bSrc, void * bDst, int sx, int sy, int bppMode)
{
	int cptx,cpty ;

	if(bppMode==8)
	{	for(cpty=0;cpty<sx;cpty++)
			for(cptx=0;cptx<sy;cptx++)
				set_color_buffer8(cpty, cptx, sy, (uchar*)bDst, get_color_buffer8(cptx, (sx-1)-cpty, sx, (uchar*)bSrc)) ;
	} else {
		for(cpty=0;cpty<sx;cpty++)
			for(cptx=0;cptx<sy;cptx++)
				set_color_buffer16(cpty, cptx, sy, (ushort*)bDst, get_color_buffer16(cptx, (sx-1)-cpty, sx, (ushort*)bSrc)) ;
	}
}
#ifdef __cplusplus
int getPcxSize(const char * fileName, char f)
{
	F_HANDLE file ;	ushort size ; ulong p_read_count ; char jump=10 ;
	if(SM_OK != GpFileOpen(fileName, OPEN_R, &file)) return 0 ;
	if(f=='x') jump=8;
	GpFileSeek(file, FROM_BEGIN, jump, 0);
	GpFileRead(file,&size,2,&p_read_count);
	GpFileClose(file) ;
	return size+1 ;
}
#endif

int getPcxSize(const char *fileName, ushort *sx, ushort *sy)
{
	F_HANDLE file ;
	if(SM_OK != GpFileOpen(fileName, OPEN_R, &file)) return 0 ;
	GpFileSeek(file, FROM_BEGIN, 8, 0);
	GpFileRead(file,sx,2,&p_read_count);
	GpFileRead(file,sy,2,&p_read_count);
	GpFileClose(file) ;
	return (*sx) * (*sy) ;
}

int getPcxBbp(const char * fileName)
{	F_HANDLE file ; char f ;
	if(SM_OK != GpFileOpen(fileName, OPEN_R, &file)) return 0 ;
	GpFileSeek(file, FROM_BEGIN, 0x41, 0);
	GpFileRead(file,&f,1,&p_read_count);
	GpFileClose(file) ;
	if(f == 0x01)	return 8 ;	// 256 color file
	else			return 16 ; // 65k color file
}

Pixel * loadPcx(const char * fileName)
{
	F_HANDLE file;
 	uint	unRleSize, fileChk=0, errRes ;
 	void	*rleBuf, *unRleBuf, *rotateData ;
	char	err[1024] ;
	ulong	fileSize ;
	pcxFileHeader pcxHeader ;
	ushort	sx, sy, filePixelSize=1 ;

 	// open pcx file in read
	if(SM_OK != GpFileOpen(fileName, OPEN_R, &file)) { sprintf(err,"unable to open file\n%s",fileName); error(err); return 0 ; }
	// get file size
	if(SM_OK != GpFileGetSize(fileName,&fileSize)) { error("unable to get size."); return 0 ; }
	if(fileSize > 126)	// 128 is header size but 126 = 42*3 :)
	{
		// read header
		GpFileRead(file,&pcxHeader,sizeof(pcxFileHeader),&p_read_count);

		if(pcxHeader.Id == 0x0A)
			if(pcxHeader.Format == 0x01)
				fileChk = 1 ;
	}

	if(!fileChk) { error("it's not a valid pcx file !") ; return NULL ; }

	sy = pcxHeader.Xmax + 1 ;	sx = pcxHeader.Ymax + 1 ;

	rleBuf = malloc(fileSize-128) ;	 // alocate memory for pcx's rle data
	GpFileRead(file,rleBuf,fileSize-128,&p_read_count); // load file data into rleBuf
	GpFileClose(file) ;

	if(pcxHeader.NumberOfPlanes != 1)	filePixelSize++ ;
	unRleSize = sx*sy*filePixelSize ;
	errRes = fileSize-128 ;

	unRleBuf = (uchar*)unRLE((char*)rleBuf,&errRes,&unRleSize) ;	// uncompress rle data into unRleBuf
	free(rleBuf) ;
	
	if(!unRleBuf) error("unRLE error.");

	// test size of unRleBuffer with the theorical size : size x * size y
	if(unRleSize != (sx*sy*filePixelSize)) {	// if not equal !
		sprintf(err,"size found when unRLE not equal with normal size.\nactual size %i\nnormal size %i (%ix%ix  %i)",errRes,sx*sy,sx,sy,filePixelSize) ;
		error(err);
	}

	rotateData = malloc(sx*sy*filePixelSize) ;
	rotate_90(unRleBuf,rotateData,sy,sx,filePixelSize<<3) ;
	//rotate_90(rotateData,unRleBuf,sx,sy,filePixelSize<<3) ;
	//rotate_90(unRleBuf,rotateData,sy,sx,filePixelSize<<3) ;
	free(unRleBuf) ;
	return (Pixel*)rotateData ;
	//return (Pixel*)unRleBuf ;
}

