/** Gfa.cpp *******************************************************************/

#ifndef	_gfa_
#define	_gfa_

#include "Gfm.cpp"
#include "Gfa.h"

void Gfa::init(int ptype, int rtime)
{
	frmNumber	= 0 ;
	frames		= 0 ;
	time		= 0 ;
	actualFrm	= 0 ;
	lastTick	= 0 ;
	playType	= ptype ;
	way			= 1 ;
	animated	= 1 ;
	rstTime		= rtime ;
	stopTime	= 0 ;
}

Gfa::Gfa(int ptype, int rtime)
{	init(ptype,rtime) ;
}

void Gfa::reset(void)
{
	animated=1 ;
	actualFrm=0 ;
	way=1 ;
}

void Gfa::addFrm(Gfm * frm, int frmTime)
{
	Gfm **tmp = (Gfm**)malloc((++frmNumber)*sizeof(Gfm*)) ;
	memcpy(tmp,frames,(frmNumber-1)*sizeof(Gfm*)) ;
	free(frames) ;	frames = tmp ;
	
	int * tmpTime = (int *)malloc(frmNumber*sizeof(int*)) ;
	memcpy(tmpTime,time,(frmNumber-1)*sizeof(int*)) ;
	free(time) ;	time = tmpTime ;

	frames[frmNumber-1]	= frm ;
	time[frmNumber-1]	= frmTime ;
}

void Gfa::set(Pixel * anim, int sx, int sy, int frmNb, int frmTime ,Pixel *TrColor)
{	int cpt=0 ;	int size=sx*sy ;
	while(cpt<frmNb)
	{	Gfm *frm = (Gfm*)malloc(sizeof(Gfm)) ; frm->Gfm() ;
		frm->set(anim+cpt*size,sx,sy,TrColor) ;
		addFrm(frm,frmTime) ;
	cpt++ ;	};
}

void Gfa::kill(void)
{	for(int cpt=0;cpt<frmNumber;cpt++) frames[cpt]->kill ;
}

void Gfa::draw(int px, int py)
{
	if(animated)
	{	if(GpTickCountGet() - lastTick > time[actualFrm])
		{	lastTick = GpTickCountGet() ;

			if(way)
			{		switch(playType)	// 0 123123 1 1232123 2 123333 3 123111
						{
							case 0 : if(++actualFrm >= frmNumber)	actualFrm=0 ;								break ;
							case 1 : if(++actualFrm >= frmNumber) { actualFrm=frmNumber-2 ;		way=0	;	}	break ;
							case 2 : if(++actualFrm >= frmNumber) {	actualFrm=frmNumber-2 ;	animated=0	; 	}	break ;
							case 3 : if(++actualFrm >= frmNumber) {	actualFrm=0 ;			animated=0	;	}	break ;
						};
			} else
			{		switch(playType)	// 0 321321 1 3212321 2 321333 3 321111
						{
							case 0 : if(--actualFrm < 0)	actualFrm=frmNumber-1 ;						break ;
							case 1 : if(--actualFrm < 0) {	actualFrm=1 ;	way=1 ;					}	break ;
							case 2 : if(--actualFrm < 0) {	actualFrm=1 ;			animated=0 ;	}	break ;
							case 3 : if(--actualFrm < 0) {	actualFrm=frmNumber-1 ;	animated=0 ;	}	break ;
						};
			}
			if(actualFrm < 0) actualFrm=0 ;
			if(!animated) stopTime = GpTickCountGet() ;
		}
	} else {	if(rstTime)	if(GpTickCountGet() - stopTime > rstTime) reset() ; }
				
	frames[actualFrm]->draw(px,py) ;
}

#endif