#define LZ_MAX_CODE	4095		/* Biggest code possible in 12 bits. */
#define LZ_BITS		12
#define NO_SUCH_CODE		4098    /* Impossible code, to signal empty. */
#define GIF_OK 0
#define GIF_ERROR 1

typedef struct tGifWork{
	int interlaced;
	int pixelBits;
	int colorResolutionBits;
	int initialCodeSize;
	unsigned char *cmR, *cmG, *cmB;	// color maps: R, G, B
	unsigned char *file;			// puntero al fichero volcado a memoria
	
						// para descomprimir:
	unsigned char *rasterData;		// puntero al siguiente byte de la ristra de datos 
	int shiftState;		// 
	int runningBits;	// The number of bits required to represent RunningCode.
	int shiftWord;		//
	int maxCode1;		//
	int runningCode;
	uchar *pixels;		// pixeles descomprimidos
	ushort *palette;	// 5:5:5:1 a partir de RGB
}tGifWork;

typedef struct tGif{
	int width;	
	int height;
	//short	*imag;	//puntero al array de pixels en 16bpp
	tGifWork *work;
	uint *pal ;
}tGif;

// se carga el fichero en memoria de esta manera:
int gifLoadFile(char *name, tGif *gif){
	F_HANDLE f;
	ulong size, count;
	//char *fpt;

	GpFileOpen (name, OPEN_R, &f);
	if(f == NULL)
		return GIF_ERROR;
	
	gif -> work = (tGifWork *)gm_malloc(sizeof(tGifWork));

	GpFileGetSize (name, &size);
	(gif -> work) -> file = (unsigned char *)gm_malloc((unsigned int)size);
	GpFileRead (f, (void *)(gif -> work) -> file, size, &count);
	if(count-size) error("load error") ;
	GpFileClose(f);
	
	return GIF_OK;	
}

inline int mgifLoadFile(uchar *img, tGif *gif){
	gif -> work = (tGifWork *)gm_malloc(sizeof(tGifWork));
	(gif -> work) -> file = img ;
	return GIF_OK;	
}

int strncmp(const char * s1, const char * s2, int nb)
{	int n=0 ; int cmpt=0 ;
	while(cmpt<nb) { n+=((s1[cmpt])!=(s2[cmpt])) ; cmpt++ ; } ;
	return n ;
}

int gifGetInfo(tGif *gif){
	unsigned char *fpt;
	char flags;
	tGifWork *work;
	int colorMap;
	unsigned char *pt1, *pt2;
	int i, max;
	
	work = (*gif).work;
	fpt = work ->file;
	
	if(strncmp("GIF89a",(char*)fpt,6)) { error("not a gif") ; return GIF_ERROR ; }
	
	//saltar Signature (comprobar ?)
	fpt += 6;

	//desde el GlobalDescriptor
	fpt += 4;	// saltar widht y eigth
	
	flags = *fpt++;
	colorMap = (flags >> 7) & 1; // =1 si existe globalColorMap

	fpt +=2; //saltar backGroundColor y aspectRatio

	if(colorMap){	// si existe globalColorMap -> cargarlo
		//error("global color map") ;
		work ->pixelBits = flags & 7;
		work ->colorResolutionBits = (flags >> 4) & 7;

		//reservar espacio
		max = 1 << ((work->pixelBits) +1);
		work->cmR = (uchar *)gm_malloc(max);
		work->cmG = (uchar *)gm_malloc(max);
		work->cmB = (uchar *)gm_malloc(max);

		for(i=0;i<max;i++){
			*((work->cmR) + i) = *fpt++;
			*((work->cmG) + i) = *fpt++;
			*((work->cmB) + i) = *fpt++;
		};
	};

	// si existen ExtensionBlocks saltarlos
	while( *fpt == 0x21){
		int length; 
		fpt +=2; // apuntar al campo length
		length = *fpt;
		fpt += 1 + length;	// apuntar al primer dato y avanzar length posiciones
	}

	// avanzar hasta el LocalDescriptor, no se si habra un 0x00
	// por ahi en medio y por eso lo hago
	//char *fp = fpt ;
	while( *fpt++ != 0x2c); // al salir apunta al siguiente de 0x2c
	//desde el Local descriptor
	fpt +=4; //saltar posX, posY
	//error("yyoyop") ;
	//s_fpt = (short *)fpt;
	(*gif).width	= (*fpt) + 256*(*(fpt+1)); fpt+=2 ;
	(*gif).height	= (*fpt) + 256*(*(fpt+1)); fpt+=2 ;

	flags = *fpt++;
	work -> interlaced = (flags >> 6) &1;
	colorMap = (flags >> 7) &1;

	if(colorMap){ // si existe localColorMap -> cargarlo
		//error("local color map") ;
		work ->pixelBits = flags & 7;
		work ->colorResolutionBits = (flags >> 4) & 7;

		//reservar espacio
		max = 1 << ((work->pixelBits) +1);
		if((work->cmR) != NULL){	// si se habia reservado espacio -> liberar
			gm_free(work->cmR);
			gm_free(work->cmG);
			gm_free(work->cmB);
		}
		work->cmR = (uchar *)gm_malloc(max);
		work->cmG = (uchar *)gm_malloc(max);
		work->cmB = (uchar *)gm_malloc(max);

		for(i=0;i<max;i++){
			*((work->cmR) + i) = *fpt++;
			*((work->cmG) + i) = *fpt++;
			*((work->cmB) + i) = *fpt++;
		};
	};

	// desde el RasterDataBlock
	work ->initialCodeSize = *fpt++;

	// agrupar todos los RasterDataBlocks al comienzo del array work -> rasterData
	pt1 = fpt; //puntero al campo length del primer rasterDataBlock
	pt2 = work ->file; 

	while(*pt1 > 0){	// campo length > 0
		
		max = *pt1++;
		for(i=0;i<max;i++)
			*pt2++ = *pt1++;
	};		
	pt1++;	// tras esto *pt1 deberia ser 0x3b

	//reservar espacio para descomprimir la imagen, paleta y imagen a 16bpp
	work ->pixels = (uchar *)gm_malloc(gif->height * gif->width);
	work ->palette = (ushort *)gm_malloc(sizeof(unsigned short)* 256) ;//(1 << ((work->pixelBits) +1)));	// numero de colores
	//gif ->imag = (short *)gm_malloc(sizeof(unsigned short)*(gif->height * gif->width));

	//salida
	return GIF_OK;
}

static int gifGetCode(tGif *gif, int *code){
	
	tGifWork *work = gif ->work;
    unsigned char nextByte;
    static unsigned int codeMasks[] = {
	0x0000, 0x0001, 0x0003, 0x0007,
	0x000f, 0x001f, 0x003f, 0x007f,
	0x00ff, 0x01ff, 0x03ff, 0x07ff,
	0x0fff
    };

	while (work->shiftState < work->runningBits ) {
		/* Needs to get more bytes from input stream for next code: */
		nextByte = *(work -> rasterData)++;
		work->shiftWord |= ((int) nextByte) << work->shiftState;
		work->shiftState += 8;
	};

    *code = work->shiftWord & codeMasks[work->runningBits];

    work->shiftWord >>= work->runningBits;
    work->shiftState -= work->runningBits;

    /* If code cannt fit into RunningBits bits, must raise its size. Note */
    /* however that codes above 4095 are used for special signaling.      */
    if (++work->runningCode > work->maxCode1 &&	work->runningBits < LZ_BITS) {
		work->maxCode1 <<= 1;
		work->runningBits++;
    };
	return 0;
}

static int gifGetPrefixChar(unsigned int *Prefix, int Code, int ClearCode)
{
    int i = 0;

    while (Code > ClearCode && i++ <= LZ_MAX_CODE) Code = Prefix[Code];
    return Code;
}

int gifDecompress(tGif *gif){
    
	int i = 0, j, CrntCode, EOFCode, ClearCode, CrntPrefix, LastCode, StackPtr, BitsPerPixel;
    unsigned char *Stack, *Suffix;
    unsigned int *Prefix;
    tGifWork *work = gif ->work ;
	unsigned char *Line;

    BitsPerPixel = work->initialCodeSize;
	Line = work->pixels;

    ClearCode = (1 << BitsPerPixel);
    EOFCode = ClearCode + 1;
    work->runningCode = EOFCode + 1;
    work->runningBits = BitsPerPixel + 1;	 /* Number of bits per code. */
    work->maxCode1 = 1 << work->runningBits;     /* Max. code + 1. */
    StackPtr = 0;		    /* No pixels on the pixel stack. */
    LastCode = NO_SUCH_CODE;
    work->shiftState = 0;	/* No information in ShiftWord. */
    work->shiftWord = 0;
	work->rasterData = work->file; // inicializar puntero a datos
	
	Prefix = (uint*)gm_malloc(sizeof(unsigned int)*(LZ_MAX_CODE+1));
    Suffix = (uchar*)gm_malloc(sizeof(unsigned char)*(LZ_MAX_CODE+1));
    Stack = (uchar*)gm_malloc(sizeof(unsigned char)*(LZ_MAX_CODE));

    for (j = 0; j <= LZ_MAX_CODE; j++) Prefix[j] = NO_SUCH_CODE;

    while (CrntCode != EOFCode) { // decomprimir hasta alcanzar fin del gif
		
		gifGetCode(gif, &CrntCode);
    
		if (CrntCode == EOFCode) {
			// no hacer nada
		}
		else if (CrntCode == ClearCode) {
			/* We need to start over again: */
			for (j = 0; j <= LZ_MAX_CODE; j++) Prefix[j] = NO_SUCH_CODE;
			work->runningCode = (1 << BitsPerPixel) + 2;
			work->runningBits = BitsPerPixel + 1;
			work->maxCode1 = 1 << work->runningBits;
			LastCode = NO_SUCH_CODE;
		}
		else {
			/* Its regular code - if in pixel range simply add it to output  */
			/* stream, otherwise trace to codes linked list until the prefix */
			/* is in pixel range:					     */
			if (CrntCode < ClearCode) {
			/* This is simple - its pixel scalar, so add it to output:   */
			Line[i++] = CrntCode;
			}
			else {
			/* Its a code to needed to be traced: trace the linked list  */
			/* until the prefix is a pixel, while pushing the suffix     */
			/* pixels on our stack. If we done, pop the stack in reverse */
			/* (thats what stack is good for!) order to output.	     */
			if (Prefix[CrntCode] == NO_SUCH_CODE) {
				/* Only allowed if CrntCode is exactly the running code: */
				/* In that case CrntCode = XXXCode, CrntCode or the	     */
				/* prefix code is last code and the suffix char is	     */
				/* exactly the prefix of last code!			     */
				if (CrntCode == work->runningCode - 2) {
					CrntPrefix = LastCode;
					Suffix[work->runningCode - 2] =
					Stack[StackPtr++] = gifGetPrefixChar(Prefix,
								LastCode, ClearCode);
				}
				else {
					// liberar memoria
					gm_free(Prefix);
					gm_free(Suffix);
					gm_free(Stack);
					gm_free(work->file);
					return GIF_ERROR;
				}
			}
			else
				CrntPrefix = CrntCode;

				/* Now (if image is O.K.) we should not get an NO_SUCH_CODE  */
				/* During the trace. As we might loop forever, in case of    */
				/* defective image, we count the number of loops we trace    */
				/* and stop if we got LZ_MAX_CODE. obviously we can not      */
				/* loop more than that.					     */
				j = 0;
				while (j++ <= LZ_MAX_CODE && CrntPrefix > ClearCode && CrntPrefix <= LZ_MAX_CODE) {
					Stack[StackPtr++] = Suffix[CrntPrefix];
					CrntPrefix = Prefix[CrntPrefix];
				}
				if (j >= LZ_MAX_CODE || CrntPrefix > LZ_MAX_CODE) {
						// liberar memoria
					gm_free(Prefix);
					gm_free(Suffix);
					gm_free(Stack);
					gm_free(work->file);
					return GIF_ERROR;
				}
				/* Push the last character on stack: */
				Stack[StackPtr++] = CrntPrefix;

				/* Now lets pop all the stack into output: */
				while (StackPtr != 0)
				    Line[i++] = Stack[--StackPtr];
				}
				if (LastCode != NO_SUCH_CODE) {
					Prefix[work->runningCode - 2] = LastCode;
			
					if (CrntCode == work->runningCode - 2) {
						/* Only allowed if CrntCode is exactly the running code: */
						/* In that case CrntCode = XXXCode, CrntCode or the	     */
						/* prefix code is last code and the suffix char is	     */
						/* exactly the prefix of last code!			     */
						Suffix[work->runningCode - 2] =
						gifGetPrefixChar(Prefix, LastCode, ClearCode);
					}
					else {
						Suffix[work->runningCode - 2] =
						gifGetPrefixChar(Prefix, CrntCode, ClearCode);
					}
				}
			LastCode = CrntCode;
		}
    };

	// liberar memoria
	gm_free(Prefix);
	gm_free(Suffix);
	gm_free(Stack);
	gm_free(work->file);

	if(i == gif->height * gif->width){
		return GIF_OK;
	}
	else{
		return GIF_ERROR;
	};
}

void gifGetPalette(tGif *gif){
	tGifWork *work = gif->work;
	int numColors = (1 << ((work ->pixelBits) +1));
	int i;
	unsigned char R,G,B;
	int shift = (work->colorResolutionBits + 1) - 5;
	if(shift<0) shift = 0;

	//sprintf(err,"%i colors\nshift %i",numColors, shift) ; error(err) ;

	gif->pal = (uint*)malloc(256*sizeof(uint)) ;
	for(i=0; i< numColors; i++){
		R = ((work->cmR)[i]) >> shift;
		G = ((work->cmG)[i]) >> shift;
		B = ((work->cmB)[i]) >> shift;
		(gif->pal)[i] = (R << 11) | (G << 6) | (B << 1) | 0;
	};

	//for(i=0; i< numColors; i++)	(gif->pal)[i] = tpal[i] ;
	//for(i=numColors; i< 256; i++)	p[i] = 0xFFFFFFFF ;
	free(work->palette) ;
	gm_free(work->cmR);
	gm_free(work->cmG);
	gm_free(work->cmB);
}

void gifApplyPalette(tGif *gif){	// fix to got a 8 bit output
	int i,j;
	int w = gif->width , h = gif->height ;
	int maxPixel = h*w ;
	tGifWork *work = gif->work;

	unsigned char *rimag = (uchar *)gm_malloc(maxPixel);	
	memcpy(rimag,work->pixels,maxPixel) ;

	if(work->interlaced){ //error("interlaced") ;
		unsigned char *rimag2 = (uchar *)gm_malloc(maxPixel);	
		unsigned char *pt1, *pt2;

		for(i=0;i<(h+7)/8;i++){
			pt2 = rimag2 + i*w*8; 
			pt1 = rimag + i*w;
			for(j=0;j<w;j++)
				pt2[j] = pt1[j];
		};
		for(i=0;i<(h+3)/8;i++){
			pt2 = rimag2 + 4*w + i*w*8; 
			pt1 = rimag + (i+(h+7)/8)*w;
			for(j=0;j<w;j++)
				pt2[j] = pt1[j];
		};		
		for(i=0;i<(h+1)/4;i++){
			pt2 = rimag2 + 2*w + i*w*4; 
			pt1 = rimag + (i+(h+7)/8+(h+3)/8)*w;
			for(j=0;j<w;j++)
				pt2[j] = pt1[j];
		};		
		for(i=0;i<h/2;i++){
			pt2 = rimag2 + w + i*w*2; 
			pt1 = rimag + (i+(h+7)/8+(h+3)/8+(h+1)/4)*w;
			for(j=0;j<w;j++)
				pt2[j] = pt1[j];
		};

		pt2 = rimag2; pt1 = rimag;
		for(i=0;i<h*w;i++)
			pt1[i] = pt2[i];

		gm_free(rimag2);
	};

	//rotar la imagen 90 grados como las agujas del reloj
	for(i=0;i<w;i++)
		for(j=0;j<h;j++)
			(work->pixels)[i*h+j] = rimag[(h-j-1)*w+i];
	gm_free(rimag);

	//liberar memoria
	//gm_free(work->palette); // don't free pal for 8 bit
	//gm_free(work->pixels);
}

int gifLoad(char *name, tGif *gif)
{	if(gifLoadFile(name, gif)) return GIF_ERROR ;
	if(gifGetInfo(gif)) return GIF_ERROR ;
	gifGetPalette(gif);
	if(gifDecompress(gif)) return GIF_ERROR ;
	gifApplyPalette(gif);
	return GIF_OK;
}

int mgifLoad(uchar *img, tGif *gif)
{	if(mgifLoadFile(img, gif)) return GIF_ERROR ;
	if(gifGetInfo(gif)) return GIF_ERROR ;
	gifGetPalette(gif);
	if(gifDecompress(gif)) return GIF_ERROR ;
	gifApplyPalette(gif);
	return GIF_OK;
}
