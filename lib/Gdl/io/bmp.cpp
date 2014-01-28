
// !!! bmp_sx%2 must be 0, or use 0 as transparent color
Pixel * loadBmp(const char * fileName)
{
	F_HANDLE file;
	char	buffer[10] ;
	Pixel * bmpDta, *data;
	ulong	fileSize ;
	uint	dtaSize, dtaOffset ;
	uint	sx, sy, fileComp ;
	ushort	imgbpp ;
	uint	x,y ;

	error("entering load bmp") ;

	if(SM_OK!=GpFileGetSize(fileName,&fileSize)) { error("unable to get size."); return 0 ; }
	if(fileSize < 42)	// 54 is header size but 42 pawa
	{	error("bmp error, file size is too small.") ; return 0; }

	if(SM_OK!=GpFileOpen(fileName, OPEN_R, &file)) { sprintf(err,"unable to open file\n%s",fileName); error(err); return 0 ; }

	GpFileRead(file,buffer,2,&p_read_count);
	buffer[2] = '\0' ;
	if(strcmp(buffer,"BM"))
	{
		sprintf(err,"it's not a valid bmp file !\nfileChk : %s",buffer) ;
		error(err) ; return NULL ;
	}
	
	GpFileRead(file,buffer,8,&p_read_count); 	// skip junk
	GpFileRead(file,&dtaOffset,4,&p_read_count);// offset to data
	GpFileRead(file,buffer,4,&p_read_count); 	// skip junk
	GpFileRead(file,&sx,4,&p_read_count); 		// read sx
	GpFileRead(file,&sy,4,&p_read_count); 		// read sy
	GpFileRead(file,buffer,2,&p_read_count); 	// skip junk

	GpFileRead(file,&imgbpp,2,&p_read_count); 	// read img bpp
	GpFileRead(file,&fileComp,4,&p_read_count); // read img compression 0=no
	GpFileRead(file,&dtaSize,4,&p_read_count);	// read data size

	if(!fileComp)	dtaSize = sx*sy*(imgbpp>>3) ;
	if(fileComp)	error("cannot load compressed file.") ;
	
	if(sx%4) { error("unaligned bmp.") ; sx+=(sx%4) ; }

	// alloc buffer to get bmp data, and rotate data
	bmpDta	= (Pixel *)malloc(dtaSize) ;
	data	= (Pixel *)malloc(dtaSize) ;
	if(!bmpDta || !data) { error("aloc error in bmpLoad."); free(bmpDta); free(data); return NULL ; }

	// go on data
	GpFileSeek(file,FROM_BEGIN,dtaOffset,0);
	GpFileRead(file,bmpDta,dtaSize,&p_read_count);
	GpFileClose(file) ;

	if(dtaSize != p_read_count)
	{
		sprintf(err,"read error in loadBmp.\nsize need %i\nsize read %i",dtaSize,p_read_count) ;
		error(err) ;
		free(bmpDta); free(data) ;
		return 0 ;
	}

	for (y=0;y<sy;y++)
		for (x=0;x<sx;x++)
               *(data+x*sy+y)=bmpDta[y*sx+x];

	free(bmpDta) ;
	return data ;
}

int getBmpSize(const char * fileName, char f)
{
	F_HANDLE file ;	uint size ; ulong p_read_count ; char jump=22 ;
	if(SM_OK != GpFileOpen(fileName, OPEN_R, &file)) return 0 ;
	if(f=='x') jump=18;
	GpFileSeek(file, FROM_BEGIN, jump, 0);
	GpFileRead(file,&size,4,&p_read_count);
	GpFileClose(file) ;
	return size ;
}

int getBmpSize(const char *fileName, uint *sx, uint *sy)
{
	F_HANDLE file ;
	if(SM_OK != GpFileOpen(fileName, OPEN_R, &file)) return 0 ;
	GpFileSeek(file, FROM_BEGIN, 18, 0);
	GpFileRead(file,sx,4,&p_read_count);
	GpFileRead(file,sy,4,&p_read_count);
	if((*sx)%4) { error("unaligned bmp.") ; *sx += ((*sx)%4) ; }

	
	GpFileClose(file) ;
	return (*sx) * (*sy) ;
}

int getBmpBbp(const char * fileName)
{	F_HANDLE file ; ushort f ;
	if(SM_OK != GpFileOpen(fileName, OPEN_R, &file)) return 0 ;
	GpFileSeek(file, FROM_BEGIN, 28, 0);
	GpFileRead(file,&f,2,&p_read_count);
	GpFileClose(file) ;
	return f ;
}
/*
uint * getBmpPal(const char * fileName)
{
	F_HANDLE file;
	uint i,r,g,b ;
	uint * pal, *palPtr ;
	uchar bmpPal[1024] ;

	if(SM_OK!=GpFileOpen(fileName, OPEN_R, &file)) { sprintf(err,"unable to open file\n%s",fileName); error(err); return 0 ; }
	GpFileSeek(file,FROM_BEGIN,0x36,(long*)&p_read_count);
	GpFileRead(file,bmpPal,1024,&p_read_count);
	GpFileClose(file) ;

	pal = palPtr = (uint*)malloc(1024) ;
	memset(pal,0xFF,1024);

	for(i=0;i<256;i++)
	{	
		b =   (bmpPal[4*i])>>3	;
		g = (bmpPal[4*i+1])>>3	;
		r = (bmpPal[4*i+2])>>3	;
		
		*palPtr++ = (r<<11)|(g<<6)|(b<<1)|0 ;
	};

	return pal ;
}
*/