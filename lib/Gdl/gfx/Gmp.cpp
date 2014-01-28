/** Gmp.cpp *******************************************************************/

#ifndef	_gmp_
#define	_gmp_
#include "./Gfa.cpp"	// anim
#include "Gmp.h"

int getShift(int size)
{
	int tsize = 2048;
	int shift = 11	;

	while(size < tsize)
	{
		tsize >>= 1	;
		shift --	;
	} ;
	
	return shift	;
}

void	Gmp::set(short * array, Pixel * tlset, int frmNb, int msx, int msy, int tsx, int tsy)
{
	//Gfm ** yyop = (Gfm**)malloc(sizeof(Gfm**)) ;
	Gfm * yop = (Gfm*)malloc(frmNb*sizeof(Gfm)) ;
	Pixel * img = tlset ;
	for(int cpt=0;cpt<frmNb;cpt++)	{	yop[cpt].Gfm() ;
										yop[cpt].set(img,tsx,tsy,tlset) ;
										img += tsx*tsy ;
									}
	set(array,yop,frmNb,msx,msy) ;
}

void	Gmp::set(short * array, Gfm * tlset, int frmNb, int msx, int msy)
{
	if(!tlset->loaded) error("tileset not loaded") ;
	stx = tlset->sx ;
	sty = tlset->sy ;
	tileset = tlset ;
	Array = array ;
	nbtiles = frmNb-1 ;
	x = y = 50 ;
	mdsx = getShift(stx) ;
	mdsy = getShift(sty) ;
	ntx = 320>>mdsx ;
	nty = 240>>mdsy ;
	sx = msx ; sy = msy ;
	//animated = (int*)malloc(frmNb*sizeof(int)) ;
	//memset(animated,0,frmNb*sizeof(int)) ;
	//sprintf(err,"sx %i sy %i stx %i sty %i ntx %i nty %i mdsx %i mdsy %i",sx,sy,stx,sty,ntx,nty,mdsx,mdsy) ; error(err) ;
}

void Gmp::draw(void)
{
	int debut_case_aff_x = x>>mdsx ;
	int debut_case_aff_y = y>>mdsy ;

	int decalage_case_x = (debut_case_aff_x+1)*stx - x ;
	int decalage_case_y = (debut_case_aff_y+1)*sty - y ;

	//ulong st, sp ;
	int cpt_x,cpt_y,id ;//,tmp ;

		for(cpt_x=0;cpt_x<ntx+1;cpt_x++)
			for(cpt_y=0;cpt_y<nty+1;cpt_y++)
			{
				id=Array[(cpt_y+debut_case_aff_y)*sx + cpt_x + debut_case_aff_x] ;
				if(id > 0 && id < nbtiles)
				{	//st = GpTickCountGet() ;
					tileset[id].draw((cpt_x-1)*stx + decalage_case_x,(cpt_y-1)*sty + decalage_case_y) ;
					//sp = GpTickCountGet() ;
					//int t = sp-st ;
					//if(t > 1) { sprintf(err,"ntx %i nty %i\nt %i\nid %i\nloaded %i\nclnum %i",ntx,nty,sp-st,id,tileset[id].loaded,tileset[id].colNum) ; error(err) ; }
				}
			} ;
}


/*
inline void drawMap(struct map * map)
{
	int mapStart = (map->x >> map->msize_tile_x) + (map->y >> map->msize_tile_y)*(map->sx) ;
	short * tvalue = (short*)(map->Array + mapStart) ;
	register int py = 0 ;
	register int value ;
	register int cpt=0 ;
	register short * next = tvalue+map->nbTile_x+1 ;
	register int dx = 15-(map->x%16) ;
	register int dy = 15-(map->y%16) ;

	while(py<257)
	{	value = *tvalue++ ;
		if(value) map->tiles[value].draw(((cpt%map->nbTile_x)<<map->msize_tile_x)+dx,py+dy) ;
		if(tvalue >= next){ py+=16 ; tvalue += (map->sx-map->nbTile_x) ; next = tvalue + (map->nbTile_x) ; }
		cpt++ ;
	};
}

*/


#endif