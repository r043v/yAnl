
class Gft
{	public :
	Gfm *frm ;
	int ldd ;
	void set(Pixel *img, int nb, int sx, int sy, Pixel * trClr)	;
	void draw(const char *str,int px, int py, int way=0, int limit=42)	;
	void drawInt(int n,int px, int py, const char * method="%i", int way=0);//, void * fct() = &draw)	;
	void drawIntI(int n,int px, int py, const char * method="%i");
	void drawI(const char *str,int px, int py, int limit=42)	;
	void drawIi(const char *str,int px, int py, int limit=42)	;
	char * msave(int * s) ;
	void  mload(char * dta) ;
	Gft() { ldd=0 ; }
	//~Gft() { error("unload font.") ; }
};

void Gft::set(Pixel *img, int nb, int sx, int sy, Pixel * trClr)
{	int cpt=0 ;	int size=sx*sy ;
	frm = (Gfm*)malloc(sizeof(Gfm)*(nb+32)) ;
	while(cpt<nb)
	{	frm[cpt+32].Gfm() ;	frm[cpt+32].set(img+cpt*size,sx,sy,trClr) ;
		cpt++ ;
	};

	ldd=nb ;
}

char * Gft::msave(int * s)
{	if(!ldd) return 0 ;
	char* frmd[150] ;
	int frms[150] ;
	int tsize=0 ;
	for(int c=0;c<ldd;c++){	frmd[c] = frm[32+c].msave(&(frms[c])) ; tsize+=frms[c]; }
	
	char *out = (char*)malloc(tsize+4+ldd*4) ;
	int* ptr = (int*)out ;	*ptr++ = ldd ;
	for(int c=0;c<ldd;c++)	*ptr++ = frms[c] ;
	for(int c=0;c<ldd;c++)	{	memcpy(ptr,frmd[c],frms[c]) ; ptr = (int*)(((char*)ptr) + frms[c]) ;	};
	if(s) *s = (int)((char*)ptr-out) ;
	return out ;
}

void   Gft::mload(char * dta)
{	int nb = *(int*)dta ;	int * ptr = (int*)(dta+4) ;
	frm = (Gfm*)malloc(sizeof(Gfm)*(nb+32)) ;
	char * p = dta + 4 + nb*4 ;
	for(int c=0;c<nb;c++)	{	frm[c+32].Gfm() ;
								frm[c+32].mload(p) ;
								p+=*ptr++ ;
							};
	ldd = nb ;
}

void Gft::draw(const char *str,int px, int py, int way, int limit)
{	if(!ldd) return ;
	int tmp = *str++ ;
	int line= 0 ;
	int cx = 0 ;	int h = frm[32].sy ; int l = frm[32].sx ;
	while(tmp && str < str+limit)
	{	if(tmp == '\n') { line++ ; cx=0 ; }
		else	{	if(!way)	frm[tmp].draw(px + cx*l,py + line*h) ;
					else		frm[tmp].draw(px + line*l,py + cx*h) ;
				}
		tmp = *str++ ; cx++ ;
	};
}

void Gft::drawInt(int n,int px, int py, const char * method, int way)//, void * fct())
{	if(!ldd) return ;
	sprintf(gerr,method,n)		;
	draw(gerr,px,py,way,10)	;
}

void Gft::drawIntI(int n,int px, int py, const char * method)
{	if(!ldd) return ;
	sprintf(gerr,method,n);
	drawI(gerr,px,py,10)	;
}

void Gft::drawI(const char *str,int px, int py, int limit)
{	if(!ldd) return ;
	int tmp = *str++ ;
	int cx = 0 ;	int l = frm[32].sx ;	px-=l++ ;
	while(tmp && str < str+limit)
	{	frm[tmp].draw(px + (cx*l),py) ;
		tmp = *str++ ; cx-- ;
	};
}

void Gft::drawIi(const char *str,int px, int py, int limit)
{	if(!ldd) return ;
	int tmp = *str++ ;
	int cx = 0 ;	int l = frm[32].sx ; px-=l*(strlen(str)+1) ;
	while(tmp && str < str+limit)
	{	frm[tmp].draw(px + (cx*l),py) ;
		tmp = *str++ ; cx++ ;
	};
}








































