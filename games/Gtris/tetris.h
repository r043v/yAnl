#include "./pieces.h"
#include "stdlib.h"
extern int abs(int /*j*/);
extern int rand(void);
extern void srand(unsigned int /*seed*/);

ushort *beep[4] ;
Gfm	carre[4]	;
Gfm	text		;
Gfm balle		;
Pixel * bg ;
Pixel * img ;
int cpt,x,y ;
int yo ;
ulong time=0, lastDown=0, lastMove=0 ;
int count=0,go=0 ;
int array[16*8] ;
int posLeft=0,posRight=0,posDown=0 ;
int lineNumber=0 ;
int score=0 ;
int scoreDraw=0 ;

int lastLine=0;
int lastScore=0 ;

void playBeep(void)
{	/*static ulong lastBeep=0 ;
	return ;
	if(GpTickCountGet()>lastBeep+(100<<1))
	{
		GpPcmRemove(beep[0]) ;
		GpPcmPlay(beep[0],268,0);
		lastBeep = GpTickCountGet() ;
	}*/
}

void	drawPiece(int type, int px, int py, int turn)
{		int ppx,ppy ;
		int xmin=0, xmax=4, ymax=4, ymin=0 ;
		int start = 16*4*type + 16*turn ;
		
		if(px<0)
		{
			if(px < -4) return ;
			xmin = -px ;
		} else	if(px>4)
				{
					if(px>8) return ;
					xmax = 8-px ;
				}

		if(py>12)
		{
			if(py>16) return ;
			ymax = 4-(py-12) ;
		} else if(py<0){
			if(py < -4) return ;
			ymin = -py ;
		}

		for(x=xmin;x<xmax;x++)
			for(y=ymin;y<ymax;y++)
			{	if(piece[start+(x<<2)+y])
				{	ppx=px+x ; ppy=py+y ;
					carre[2].drawI((ppy<<4) + ppy + 32,178 - ((ppx<<4) + ppx)) ;
				}
			};
}

int * smile[4] ;
	
int smile0[64] = {	0,3,3,3,3,0,0,0,
					3,0,0,0,0,3,0,0,
					3,0,0,0,0,3,0,0,
					3,3,0,3,3,0,0,0,
					1,0,3,0,0,0,0,0,
					3,0,0,3,0,0,0,0, 
					3,0,0,3,0,0,0,0,
					0,3,3,0,0,0,0,0
				 };
				 
				 
int smile1[64] = {	0,0,0,0,0,0,0,0,
					0,0,0,3,0,0,0,0,
					0,0,3,0,0,3,0,0,
					0,3,0,0,0,0,0,0,
					0,3,0,0,0,0,0,0,
					0,0,3,0,0,3,0,0,
					0,0,0,3,0,0,0,0,
					0,0,0,0,0,0,0,0
				 } ;

int smile2[64] = {	0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0,
					0,0,3,0,0,3,0,0,
					0,0,0,0,0,0,0,0,
					0,3,3,3,3,3,3,0, 
					0,3,0,0,0,0,3,0,
					0,3,0,0,0,0,3,0,
					0,0,3,3,3,3,0,0
				 } ;


int clearBuffer(void)
{
	static int cnt=0 ;
	static int cnt2=16*8-1 ;
	static int _time = GpTickCountGet() ;
	static int clearType = 1 ;
	static int old = 0 ;
	static int * sml ;
	
	if(GpTickCountGet() > _time + 12000)
	{
		_time = GpTickCountGet() ;
		
		do { clearType=rand()%4 ; } while(clearType == old) ;
		old = clearType ;
		
		switch(clearType)
		{
			case 0 : cnt=0;cnt2=16*8-1;	break ;
			case 1 : cnt=0;cnt2=16*8-1;	break ;
			case 2 : cnt=0;cnt2=0;		break ;
			case 3 : cnt=0 ; sml=smile[rand()%3];	break ;
		} ;
	}

	playBeep() ;
	
	switch(clearType)
	{
		case 0 :
		 array[cnt] = 0 ;
		 array[cnt2] = 0 ;
		 cnt++ ; cnt2-- ;
		 if(cnt==(16*8)>>1) return 1 ;
		 array[cnt] = 3 ;
		 array[cnt2] = 3 ;
		break ;
		
		case 1 :
		 array[((cnt%8)<<4)+(cnt>>3)] = 0 ;
		 array[((cnt2%8)<<4)+(cnt2>>3)] = 0 ;
		 cnt++ ; if(cnt2) cnt2-- ;
		 if(cnt>(16*8)>>1) return 1 ;
		 array[((cnt%8)<<4)+(cnt>>3)] = 3 ;
		 array[((cnt2%8)<<4)+(cnt2>>3)] = 3 ;
		break ;
		
		case 2 :
			if(!cnt2)
			{
				int tmp=cnt ;
				while((tmp--)&&(cnt-tmp-1 < 8))
				 if(tmp<16) array[((cnt-tmp-1)<<4) + tmp] = 3 ;
				cnt++  ;	if(cnt==16+8) { cnt2=1 ; cnt=15; }
			} else return 1 ;
		break ;
		
		case 3 : // smile
			if(cnt<16)
			{
				for(int cmpt=0;cmpt<8;cmpt++)
				{
					for(int cmpt2=1;cmpt2<16;cmpt2++)
						array[cmpt*16 + (cmpt2-1)] = array[cmpt*16 + cmpt2] ;
					if(cnt < 8) array[cmpt*16 + 15] = sml[cmpt*8+cnt] ;
					else		array[cmpt*16 + 15] = 0 ;
				};
				cnt++ ;
			} else return 1 ;
		break ;
	} ;

	return 0 ;
}

	int	title[16*8] =	{	0,0,3,3,3,3,0,0,
							0,3,0,0,0,0,3,0,
							0,3,0,3,2,0,3,0,
							0,0,0,0,3,3,1,0,
							0,2,0,0,0,0,0,0,
							0,3,2,2,2,2,1,0,
							0,2,0,0,0,0,0,0,
							0,0,3,3,3,3,3,0,
							0,3,0,0,2,0,0,0,
							0,3,3,3,0,3,3,0,
							0,0,0,0,0,0,0,0,
							2,0,1,3,3,2,1,0,
							0,0,0,0,0,0,0,0,
							0,0,3,2,0,0,2,0,
							0,3,0,0,2,0,0,3,
							0,0,2,0,0,2,3,0	} ;

void pushLineright(int * line)
{
	int cpt,cmpt ;
	
	for(cpt=0;cpt<8;cpt++)
	{
		for(cmpt=1;cmpt<16;cmpt++)
			array[cpt*16 + (cmpt-1)] = array[cpt*16 + cmpt] ;
		array[cpt*16 + 15] = *line ;
		line++;
	} ;
}

void pushLineleft(int * line)
{
	int cpt,cmpt ;
	
	for(cpt=0;cpt<8;cpt++)
	{
		for(cmpt=0;cmpt<15;cmpt++)
			array[cpt*16 + (cmpt+1)] = array[cpt*16 + cmpt] ;
		array[cpt*16] = *line ;
		line++;
	} ;
}

void lineUp(int line)
{
	int cpt ;
	
	for(cpt=0;cpt<8;cpt++)
	{
			array[cpt*16 + line]++  ;
			array[cpt*16 + line]%=4 ;
	} ;
}
//int cl = 31 ;
void levelSet(void)
{	static int last = lineNumber ;
	static int first=5;
	if(first) { for(;first;first--) ((uint*)0x14A00400)[4] = fadeColorOut(((uint*)0x14A00400)[4]) ; }
	if(lineNumber - last > 9)
	{	//if(!lineNumber) cl=31 ;
		last = lineNumber ;
		//uchar *color = (uchar *)(bg + 240*306) ;
		//sprintf(err,"color %i -> %i",((uint*)0x14A00400)[*color],*color) ; error(err) ;
		((uint*)0x14A00400)[4] = fadeColorOut(((uint*)0x14A00400)[4]) ;
		if(downSpeed) downSpeed -= 42 ;
		if(downSpeed<10) { /*error("you are a master :)\n\ndownSpeed=0 ;\n") ; downSpeed=0 ;*/ downSpeed=10 ; }
		//cl-=2 ;
		//if(cl<3) cl=31 ;
		((uint*)0x14A00400)[42] = getGpColor_(31,31,31) ;//moon
	} else if(last > lineNumber) last=0 ;
}

void drawArray(void)
{		Gft *fnt = &bfont ;// &GdlFont ;
		int temp2 ;
		memset(screen[nflip]+240*308,*(bg+239*308),240*12) ;
		memcpy(screen[nflip],bg,240*308) ;

		if(keyPush(kL)) if(!keyPush(kR)) if(keyPush(kB)) if(keyPush(kA)) lineNumber+=2 ;

		static int ld = 0 ;

		if(ld<lineNumber) ld++ ;
		else if(ld>lineNumber) ld-- ;

		if(lineNumber<50)	moon[0]->draw(270-(ld>>2),182-ld) ;
		else	if(lineNumber<100)	moon[1]->draw(270-(ld>>2),182-ld) ;
				else	if(lineNumber<150)	moon[2]->draw(270-(ld>>2),182-ld) ;
						else	{	static int c=2 ;	static int y=182-ld ;
									if(lineNumber>=150 && lineNumber<200) y=32 ;
									static ulong t=0 ;	if(y<182) y++ ;
									moon[c]->draw(280-(ld>>2),y) ;
									if(GpTickCountGet()-t>2042) {
										c++ ;	if(c==3) c=0 ;	// Oo
										t=GpTickCountGet() ;
									}
								}

		levelSet() ;
		if(scoreDraw < score)
		{	if(score - scoreDraw < 100)	scoreDraw+=5 ;
			else						scoreDraw+=25 ;
		}

		if(scoreDraw > score)	scoreDraw=score ;
		
		if(tetriswasinit)	{	fnt->drawInt(scoreDraw,242,10,"%i",1) ;
								fnt->drawInt(lineNumber,224,10,"%i",1) ;
								if(lastScore){	fnt->drawInt(lastScore,12,10,"last %i",1) ;
												fnt->drawInt(lastLine,2,10,"with %i lines",1) ;
								}
								note * n = trk->patterns[trk->mod->song[trk->mod->songpos]].raw[trk->mod->patternline] ;
								for(int m=0;m<4;m++)
								{	rectFill(20+58*m, 6, 32, (trk->mod->volume[m])>>2, 0); // volume bar
									if(n[m].sample)	fnt->drawIntI(n[m].sample,22+58*m,226,"%x") ;
									if(n[m].period)	fnt->drawI(n[m].text,22+58*m+32,226) ;
								};
							}
		
		for(y=0;y<16;y++)
		{	temp2 = (y<<4) + y + 32 ;
			carre[array[y]].drawI(temp2,178) ;
			carre[array[y + 16]].drawI(temp2,178 - 17) ;
			carre[array[y + 32]].drawI(temp2,178 - 34) ;
			carre[array[y + 48]].drawI(temp2,178 - 51) ;
			carre[array[y + 64]].drawI(temp2,178 - 68) ;
			carre[array[y + 80]].drawI(temp2,178 - 85) ;
			carre[array[y + 96]].drawI(temp2,178 - 102) ;
			carre[array[y + 112]].drawI(temp2,178 - 119) ;
		}
}

int isdie=0 ;

void	putPiece(int type, int px, int py, int turn)
{
		int ppx,ppy ;
		int xmin=0, xmax=4, ymax=4, ymin=0 ;
		
		playBeep() ;
		
		if(px<0)
		{
			if(px < -4) return ;
			xmin = -px ;
		} else	if(px>4)
				{
					if(px>8) return ;
					xmax = 8-px ;
				}

		if(py>12)
		{
			if(py>15) return ;
			//ymax = 4-(py-12) ;
		} else if(py<0){
			if(py < -4) return ;
			ymin = -py ;
		}
		

		for(x=xmin;x<xmax;x++)
			for(y=ymin;y<ymax;y++)
			{	
				if(piece[(16*4*type)+(16*turn)+(4*x)+y])
				{
					ppx=px+x ; ppy=py+y ;
					if(ppy < 16)	array[ppx*16 + ppy]=3 ;
					else		{ isdie=1 ; lastLine = lineNumber ; lastScore = score ; }
				}
			};
}

void intro(void)
{
	ulong startTime = GpTickCountGet() ;
	ulong _time = startTime ;
	
	while(1)
	{	if(count < 16)
		{	if(GpTickCountGet() > startTime+(942<<1))
			{
				if((_time+100<GpTickCountGet())&&(count<16))
				{	pushLineright(&(title[count*8])) ;
					count++ ;
					_time = GpTickCountGet() ;
				}
			} else		{	static int yop=0 ;
						lineUp(yop++) ;
						yop%=16 ;
					}
		} else {
			do{ refreshKey() ; } while(!keyPush(kStart) && (GpSMCDetected()));
			while(!clearBuffer()){ drawArray() ; flipScreen() ; } ;
			return ;
		}
		drawArray() ;
		flipScreen();
		updateKey() ;
	};
}

void setMinMax(int type, int turn)
{
	int start = (16*4*type)+(16*turn) ;

		for(y=0;y<4;y++)
			for(x=0;x<4;x++)
			{	if(piece[start+(4*x)+y])
				{
					posDown=y ;
					x=y=4 ;
				}
			};
			
		for(x=0;x<4;x++)
			for(y=0;y<4;y++)
			{	if(piece[start+(4*x)+y])
				{
					posLeft=x ;
					x=y=4 ;
				}
			};
			
		for(x=0;x<4;x++)
			for(y=0;y<4;y++)
			{	if(piece[start+(4*(3-x))+y])
				{
					posRight=x ;
					x=y=4 ;
				}
			};			
}

int testPieceArray(int type, int px, int py, int turn)
{
	int start = (16*4*type)+(16*turn) ;
	int tmp, cptx, cpty ;
	
	if(type == 7 ) // single one fantom block.
	{	int cmpt=0 ;
		while(array[(px+posLeft)*16 + cmpt]) cmpt++ ;
		return (py+posDown+1 <= cmpt) ;
	}
	
	for(cpty=posDown;cpty<4;cpty++)
		for(cptx=px+posLeft;cptx<px+4-posRight;cptx++)
		{
			tmp=py+cpty ;
			if(tmp < 16)
				if(array[16*cptx+tmp]) // box colide
				{
						if(piece[start+(4*(cptx-px))+cpty])
							return 1 ;
				}
		};
	return 0 ;
}


int canRotate(int type, int px, int py, int turn)
{
	int start = (16*4*type)+(16*((turn+1)%4)) ;

	for(x=0;x<4;x++)
		for(y=0;y<4;y++)
		{
			if((x+px>=0)&&(x+px<8))
				if((y+py>=0)&&(y+py<16))
					if(piece[start+4*x+y])
						if(array[16*(x+px)+py+y]) return 0 ;
		};
	return 1 ;
}

int getFirstEmptyInColon(int colon) // first empty on down
{
	int start = colon * 16 ;
	int cmpt = 0 ;
	while(array[start+cmpt]) cmpt++ ;
	return cmpt ;
}

int getFirstEmptyUpInColon(int colon) // first empty on up
{
	int start = colon * 16 ;
	int cmpt = 15 ;
	while((cmpt)&&(!array[start+cmpt])) cmpt-- ;
	if((!cmpt)&&(!array[start])) return 0 ;
	return cmpt+1 ;
}

int canLeft(int type, int px, int py, int turn)
{
	int start = (16*4*type)+(16*turn) ;

	for(x=0;x<4;x++)
		for(y=0;y<4;y++)
		{
				if(piece[start+4*x+y])
				{
					if((px+x>0)&&(px+x<8))
						if((py+y>=0)&&(py+y<16))
							if(array[16*(x+px-1)+py+y]) return 0 ;
				}
		};
	return 1 ;
}

int canRight(int type, int px, int py, int turn)
{
	int start = (16*4*type)+(16*turn) ;

	for(x=0;x<4;x++)
		for(y=0;y<4;y++)
		{
				if(piece[start+4*x+y])
				{
					if((px+x>=0)&&(px+x<7))
						if((py+y>=0)&&(py+y<16))
							if(array[16*(x+px+1)+py+y]) return 0 ;
				}
		};
	return 1 ;
}

int pnb=0 ;
int rotate=0 ;
int posx=0, posy=14 ;

int testLines(int line)
{	int tmp=0, cmpt2 ;

	for(cmpt2=0;cmpt2<8;cmpt2++)
	{	if(array[16*cmpt2+line]) tmp++ ;
	};

	if(tmp==8)		return 0 ;
	else	if(tmp) return tmp ;
			else	return 1 ;
}

struct lineDestruc
{
	ulong time ;
	int boomMethod ;
	int enable ;
} ;

struct lineDestruc lineBoom[16] ;

void addLineDestruct(int line, int method=rand()%3)
{	static ulong lastAdd=0 ; static int nb=0 ;
	if(lineBoom[line].enable) return ;
	if(lastAdd + 300 < GpTickCountGet()) nb=0 ;
	
	lastAdd = GpTickCountGet() ;
	switch(nb++)
	{	case 0 : score+= 75 ; break ;
		case 1 : score+=125 ; break ;
		case 2 : score+=200 ; break ; 
		case 3 : score+=400 ; break ;
		case 4 : lastScore=score ; score=0 ; lastLine = lineNumber ;	break ;
	};
	
	lineNumber++ ;

	lineBoom[line].enable = 1 ;
	lineBoom[line].time = GpTickCountGet() ;
	lineBoom[line].boomMethod = method ;
}

void deleteline(int line)
{
	int cmpt2 ;

	for(x=0;x<8;x++)
	{
		array[(16*x)+line] = 0 ;
	};

	for(x=0;x<8;x++)
	{
		for(cmpt2=line;cmpt2<15;cmpt2++)
			array[x*16 + cmpt2] = array[x*16 + (cmpt2+1)] ;
		array[x*16 + 15] = 0 ;
	};
	lineBoom[line].enable = 0 ;
}

void updateLineBoom(void)
{
	for(int cpt=15;cpt>=0;cpt--)
	{
		if(lineBoom[cpt].enable)
		{
			if(lineBoom[cpt].time + (300<<1) - (cpt<<4) < GpTickCountGet())
			{	int tmp=0, cmpt2, yop, yap ;

				switch(lineBoom[cpt].boomMethod)
				{
					case 0 :	// left
							for(cmpt2=0;cmpt2<8;cmpt2++)
							{
								if(array[16*cmpt2+cpt])
								{	tmp++ ;
									array[16*cmpt2+cpt]-- ; cmpt2=8 ;
								}
							};
							if(!tmp) { deleteline(cpt) ; lineBoom[cpt].enable = 0 ; }
							else playBeep() ;
					break ;

					case 1 :	// middle
							for(cmpt2=7;cmpt2>=0;cmpt2--)
							{	yop=0 ; yap=0 ;
								if(!yop)
									if(array[16*cmpt2+cpt])
									{	tmp++ ;
										array[16*cmpt2+cpt]-- ; yop=1 ;
									}

								if(!yap)
								if(array[16*(7-cmpt2)+cpt])
								{	tmp++ ;
									array[16*(7-cmpt2)+cpt]-- ; yap=1 ;
								}
								if(yop&&yap) cmpt2=0 ;
							};
							if(!tmp) { deleteline(cpt) ; lineBoom[cpt].enable = 0 ; }
							else playBeep() ;
					break ;					
					
					case 2 :	// right
							for(cmpt2=7;cmpt2>=0;cmpt2--)
							{
								if(array[16*cmpt2+cpt])
								{	tmp++ ;
									array[16*cmpt2+cpt]-- ; cmpt2=0 ;
								}
							};
							if(!tmp) { deleteline(cpt) ; lineBoom[cpt].enable = 0 ; }
							else playBeep() ;
					break ;
				};
			}
		}
	};
}


void searchAndDestructLine(int pxStart=3, int pxEnd=4)
{
	int lineBoomMethod = 0 ; // left
	if(pxStart>2)
	{
		if(pxEnd<5)	lineBoomMethod=1 ; // middle
		else		lineBoomMethod=2 ; // right
	}

	for(int cmpt=15;cmpt>=0;cmpt--)
		if(!testLines(cmpt)) addLineDestruct(cmpt,lineBoomMethod) ;
}

int die(void)
{	if(!lineNumber)
	{	static ulong lst = 0 ;
		if(GpTickCountGet() - lst > 242)
		{	lst = GpTickCountGet() ;
			//ushort *color = (ushort *)(bg + 240*306) ;
			((uint*)0x14A00400)[4] = fadeColorIn(((uint*)0x14A00400)[4]) ;
			//if(cl<31) { cl++ ; ((uint*)0x14A00400)[42] = getGpColor_(cl,31,31) ; }
		}	downSpeed = 1242 ;
	}		return isdie ;
}

void switchRotate(void)
{	setMinMax(pnb,rotate);
	lastDown += 42 ;
	
	//if(int cheat=0)
	if(0)
		if(pnb==7)
		{
			int line = getFirstEmptyUpInColon(posx+1) ;
			if(line >= posy+2) { lastDown-=1000 ; return ; }
			array[(posx+1)*16+line] = 3 ;
			//searchAndDestructLine() ;
			return ;
		}
	
	if(!canRotate(pnb,posx,posy,rotate)) return ;

	rotate++ ;
	rotate%=4 ;
	setMinMax(pnb,rotate) ;
	while(posx < -posLeft ) posx++ ;
	while(posx > 4+posRight) posx-- ;
}

void switchpnb(void)
{	pnb++ ;
	pnb%=8 ;
	setMinMax(pnb,rotate) ;
}

int downTime ;
