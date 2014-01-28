/** Gft.cpp *******************************************************************/

#ifndef	_gft_
#define	_gft_

// Gdl font part

#include "Gfm.cpp"
#include "Gft.h"	// font code and class

/*
	// load a font, rotate it to the right (to get normal pos when turn gp at 90°) and save it as raw in a file
	Pixel *i = loadPcx("gp:\\font8.pcx") ;
	Pixel *d = (Pixel*)malloc(8*8*91) ;
	
	for(int cpt=0;cpt<91;cpt++)
	{	rotate90r(i+cpt*64,d+cpt*64,8,8) ;	};
	
	{
		F_HANDLE f ;
		GpFileCreate("gp:\\yop.txt",ALWAYS_CREATE, &f);
		GpFileWrite(f,d,8*8*91) ;
		GpFileClose(f) ;
	}
	
	myfont.set(font8r,91,8,8,font8r) ;
	//clrScr() ;
	//myfont.draw("hello world !\ni'am rov :D",42,100,1) ;
	//flipScreen() ;
	//waitKey(kL) ;

*/





#endif