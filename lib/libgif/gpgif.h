typedef struct tGifWork{
	int interlaced;
	int pixelBits;
	int colorResolutionBits;
	int initialCodeSize;
	unsigned char *cmR, *cmG, *cmB;	// color maps: R, G, B
	unsigned char *file;				// puntero al fichero volcado a memoria
	
						// para descomprimir:
	unsigned char *rasterData;		// puntero al siguiente byte de la ristra de datos 
	int shiftState;		// 
	int runningBits;	// The number of bits required to represent RunningCode.
	int shiftWord;		//
	int maxCode1;		//
	int runningCode;
	unsigned char *pixels;		// pixeles descomprimidos
	unsigned short *palette;	// 5:5:5:1 a partir de RGB

}tGifWork;

typedef struct tGif{
	int width;	
	int height;
	short *imag;	//puntero al array de pixels en 16bpp
	tGifWork *work;
}tGif;

extern int gifLoad(char *name, tGif *gif);
