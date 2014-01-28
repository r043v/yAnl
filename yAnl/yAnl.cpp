/*	yAnl.cpp, code to scan some file and directory	*/

char path[2048] ;	// the curent real path
char dpath[128] ;	// the path draw

char parsTmp[5120] ;
int loading = 0 ;
char *loadStr = (char*)"-\\|/_pAwa" ; // used to draw a rotated line who symbolise load

Gfm *icons ;
#define ico_mus	(&(icons[0]))
#define ico_mus_s	(&(icons[1]))
#define ico_dir		(&(icons[2]))
#define ico_dir_s	(&(icons[3]))
#define ico_rar		(&(icons[4]))
#define ico_rar_s	(&(icons[5]))
#define ico_img		(&(icons[6]))
#define ico_img_s	(&(icons[7]))
#define ico_exe	(&(icons[8]))
#define ico_exe_s	(&(icons[9]))
#define ico_lha		(&(icons[10]))
#define ico_lha_s	ico_mus_s

struct list
{	char * path		;	// dir path or rar path
	uint entryNumber	;	// entry number in the list
	char * fileNames ;
	char * entryNames ;
	char ** entryName	;	// text show in file selector
	char ** fileName	;	// file name, on the smc, or in archive, 32 char max !!
	void ** icon		;	// Gfm or Gfa object, for icons
	void ** icon_s		;	// same for small icons
	char *  fileType		;	// 0 not parsed, 1 exe, 2 music, 3 img, 4 archive, 5 dir, 6 unknow
	ushort *fileSize		;	// file size in ko
	ushort * order		;
	char   *ico			;	// 0 no icon, 1 classic icon, 2 animated icon
	uint lastParsed		;
	uint rarList		;	// 0 list a dir, 1 list an archive
} ;

list maliste	; // curent list
list racine 	; // gp:/
list gpmm	; // gp:/gpmm
list incRAR	; // inclused rar
list * l	;

Gfm * moon[3] ;

void setDPath(void)
{	int l = strlen(path) ;
	if(l < 24) { memcpy(dpath,path,l+1) ;	return ; }
	memcpy(dpath,path,10) ;
	dpath[11] = dpath[12] = '.' ;
	sprintf(dpath+13,"%s",path+l-11) ;
}

void drawTextRight(int x, int y, const char *text, char color)
{	int l = strlen(text) ;
	x -= 8*l ; if(x<0) x=0 ;
	GpTextOut(NULL, &gpDraw[nflip],x,y,(char*)text,color) ;
}

void allocList(int fileNumber)
{	char * mem	= (char*)malloc(fileNumber*118 + 2048); // 118bytes by file
	l->fileName	= (char**)mem					; // 4
	l->entryName	= (char**)(mem + fileNumber*4)		; // 4
	l->fileNames	= mem + fileNumber*8 ;			; // 16
	l->entryNames	= l->fileNames + fileNumber*16		; // 80
	l->icon		= (void**)(mem + fileNumber*104)	; // 4
	l->icon_s		= (void**)(mem + fileNumber*108)	; // 4
	l->ico			= (char*)(mem + fileNumber*112)	; // 1
	l->fileType 	= (char*)(mem + fileNumber*113)	; // 1
	l->fileSize		= (ushort*)(mem + fileNumber*114)	; // 2
	l->order		= (ushort*)(mem + fileNumber*116)	; // 2
	l->path		= mem + fileNumber*118 ;
	l->entryNumber = fileNumber ;
	memset(mem,0,fileNumber*118 + 42);
}

void freeList(char all=0)
{	if(!all) if((l==&gpmm)||(l==&racine)||(l==&incRAR)) return ;
	for(int cmpt=0;cmpt<l->entryNumber;cmpt++)
	{	if(l->icon[cmpt])
		{	if(l->icon[cmpt] != ico_mus)	if(l->icon[cmpt] != ico_rar)
			if(l->icon[cmpt] != ico_img)	if(l->icon[cmpt] != ico_dir)
			if(l->icon[cmpt] != ico_lha)	if(l->icon[cmpt] != ico_exe)
			{	((Gfa*)(l->icon[cmpt]))->kill(); ((Gfa*)(l->icon_s[cmpt]))->kill(); free(l->icon[cmpt]); }
			
		}
	} ;	l->entryNumber=0 ;	l->lastParsed = 0 ;	free(l->fileName);
}

int clear=2 ; // do we need to refresh screen ? if>0 yes

#define hSize (4+4+4+32+32+16+1024)

void loadIcon(int selected)
{	if(GpSMCDetected()) return ; if(l->rarList) return ; if(l->fileType[selected] != 1) return ;
	char pth[256] ;	sprintf(pth,"%s%s",l->path,l->fileName[selected]) ;
	char ext[5], *p = l->fileName[selected] ; while(*++p != '.'); ext[0]=*p++; ext[1]=*p++; ext[2]=*p++; ext[3]=*p; ext[4]=0;
	if(*(int*)ext != *(int*)".FXE" && *(int*)ext != *(int*)".fxe") return ;
	F_HANDLE file ;
	char * temp = (char*)malloc(hSize + 42*1024 + 42*512) ;
	(l->entryName)[selected] = l->entryNames + (selected*80) ;
	if(GpFileOpen(pth, OPEN_R, &file)) { sprintf((l->entryName)[selected],"open error while load icon."); return ; }
	GpFileRead(file,temp,hSize,0) ;
	if(*(int*)temp != 0x20657866) { free(temp) ; return ; }
	int size = *(int*)(temp + 4) ;
	Pixel * ico = (Pixel*)(temp + 4 + 4 + 4 + 32 + 32 + 16) ;
	ulong fileSize ; GpFileGetSize(pth,&fileSize) ;
	int isfxe=1 ;	char imgNb=1 ;
	if(fileSize > size + 1128)	// animated fxe
	{	GpFileSeek(file,FROM_BEGIN,size,0);
		int chk ;	GpFileRead(file,&chk,4,0) ;
		if(chk == 0x01332600)
		{	isfxe=2 ;
			GpFileRead(file,&imgNb,1,0) ;
			ico = (Pixel*)(temp + hSize) ; //(Pixel*)malloc(imgNb*1024) ;
			GpFileRead(file,ico,imgNb*1024,0) ;
		}
	}
	GpFileClose(file) ;	size = imgNb*1024 ;	l->fileType[selected] = 42 ;
	Pixel *tmps = ico + 42*1024 ;//(Pixel*)malloc(size>>2)	;
	Pixel * tmp = tmps ;
	for(int cmpt=0;cmpt<size;cmpt+=2) // reduct icon or anim size to 16²
	{	*tmp++ = *(ico + cmpt) ;
		if(!(cmpt%32)) if(cmpt) cmpt+=32 ;
	};	Pixel trClr = 0xef ;
	Gfa * plop = (Gfa*)malloc(sizeof(Gfa)*2)	;
	l->icon[selected] = plop++ ; l->icon_s[selected] = plop ;
	((Gfa*)(l->icon[selected]))->Gfa();	((Gfa*)(l->icon_s[selected]))->Gfa();
	((Gfa*)(l->icon[selected]))->set(ico,32,32,imgNb,342,&trClr);
	((Gfa*)(l->icon_s[selected]))->set(tmps,16,16,imgNb,342,&trClr);
	l->ico[selected] = 2 ;	free(temp) ; clear=2 ;
}

void orderList(void) // will separe directory and file
{	int c, dircnt=0, nb = l->entryNumber, filecnt=nb ;
	char str[1024] ; str[0]=0 ;	int pl = strlen(path) ; memcpy(str,path,pl) ;	GPFILEATTR attr ;
	for(c=0;c<nb;c++)
	{	if(!(l->rarList)) // if not archive
		{	if(l->fileType[c]==42 || l->fileType[c]==0) {
				memcpy(str+pl,(l->fileName)[c],12) ; (str+pl)[12]=0; GpFileAttr(str,&attr) ; // create path & retreve file info
				if( (*(str+pl) == '.') || ( (((attr.attr)>>4)&1) && (!((attr.attr)&1)) ) ) // if it's a dir
				{		(l->order)[dircnt++] = c ;
						if(*(str+pl) != '.')
						{	l->icon[c] = ico_dir ;	l->icon_s[c] = ico_dir_s ;
							l->ico[c]=1 ;	l->fileType[c]=5 ;
						}
				}	else	(l->order)[--filecnt] = c ;
			}	else		(l->order)[--filecnt] = c ;
		} else (l->order)[c] = c ;
	};
}

void loadList(const char* dir, uint start=0, int _overclock=0)
{	char ext[5] ;	loading=0 ;	const char *z = dir ;
	
	if(*dir != 'g')
	{	l = &incRAR ; if(l->lastParsed != 0) return ;
		dir = (char*)"z.rar";
	} else {	if(!(dir[4])){ l = &racine ; if(l->lastParsed != 0) return ; } // smc root
			else	{	if( ( (*(int*)(dir+4) == *(int*)"GPMM")||(*(int*)(dir+4) == *(int*)"gpmm") ) && ((!(dir[9]))||(!(dir[10]))) )
						{	l = &gpmm ; if(l->lastParsed != 0) return ; } // gpmm
					else		l =  &maliste ;
				}
	}
	_overclock = keyPush(kStart)||_overclock;	if(_overclock) overclock();
	strcpy(ext,dir + strlen(dir)-4) ;
	if( (*(int*)ext != *(int*)".RAR") && (*(int*)ext != *(int*)".rar") )
	{	GPDIRENTRY file_list[256] ;	ulong	fileNumber=0 ;
		GpDirEnumList(dir,start,start+256,(GPDIRENTRY*)&file_list,&fileNumber) ;
		allocList(fileNumber);		l->lastParsed = 0 ;

		char * f ;
		for(int cpt=0;cpt<fileNumber;cpt++)
		{	f = (l->fileName)[cpt] = l->fileNames + (cpt<<4) ;
			memcpy(f,file_list[cpt].name,12); f[12]=0; (l->entryName)[cpt] = f ;
		} ;

		strcpy(l->path,path) ;
		l->rarList = 0 ;
	} else {		l->rarList = 1 ;
				ArchiveList_struct *List = NULL, *lst ;
				int fnb, fileNumber ;
				
				if(*dir == 'z') dir = z ;
				fileNumber = urarlib_list((char*)dir,(ArchiveList_struct*)&List);				
				lst = List ; fileNumber++ ;	fnb = fileNumber ;

		allocList(fileNumber) ;	l->lastParsed = 0 ;

		{	// add '..' to back from archive list
			char * f = *(l->fileName) = *(l->entryName) = l->fileNames ; *(int*)f = *(int*)"..\0" ;
		}
				char *f, *e, *p ;
				for(int cpt=1;cpt<fileNumber;cpt++)
				{	f = (l->fileName)[cpt] = l->fileNames + (cpt<<4) ;
					e = (l->entryName)[cpt] = l->entryNames + (cpt*80) ;
					p = List->item.Name ;
					while(*p && p<(List->item.Name)+12) *e++ = *f++ = *p++ ; *f=0 ; while(*p) *e++ = *p++ ; *e=0 ;					
					(l->fileSize)[cpt] = (List->item.PackSize)>>10 ; // show compressed size
					List = List->next ;
				} ;
				strcpy(l->path,dir) ;	urarlib_freelist(lst);
		}	orderList() ;	loading=1 ;	lastIcoTime=GpTickCountGet();	if(_overclock) downclock();
}

ulong lastnokey = 0 ;
ulong lastKey = 0 ;
int clearColor=0xff ;

void parsingList(void)
{	if(GpSMCDetected() || (l->lastParsed >= (l->entryNumber)-1) ) return ;
	char ext[5] ;	ulong fileSize ;	F_HANDLE file ;	int pos=0 ;

   for(int cpt=l->lastParsed;cpt<l->entryNumber;cpt++)
   {	if(!pos){ nflip^=1 ; rectFill(222,0,10,12,clearColor) ; nflip^=1 ; }
	GpTextNOut(NULL, &gpDraw[!nflip],222,-2,loadStr+pos,0,1,1) ;	if(++pos > 3) pos=0 ;
   	
   	if(l->fileType[cpt]==0) // if not parsed
	   {	if(!l->rarList)
		{	strcpy(ext,l->fileName[cpt] + strlen(l->fileName[cpt])-4) ;
			sprintf(parsTmp,"%s%s",l->path,l->fileName[cpt]) ;
			GpFileGetSize(parsTmp,&fileSize) ;	l->fileSize[cpt] = fileSize>>10 ;
		}	else	strcpy(ext,l->entryName[cpt] + strlen(l->entryName[cpt])-4) ;
		
		if(ext[1]=='.' && ((ext[2]=='g')||(ext[2]=='G'))) *(int*)ext = 0x0042472E; // patch extention for game boys games in .gb
		if(ext[0] != '.') // not a file with a 3 letter extention, or no extention
		{	l->fileType[cpt]=6 ; l->lastParsed = cpt ;
			if(!(l->rarList))
			{	GPFILEATTR attr ;	GpFileAttr(parsTmp,&attr) ;
				if( (((attr.attr)>>4)&1) && (*(l->entryName[cpt]) != '.') || (!strcmp(parsTmp,"gp:\\")) )	// if directory and not . or ..
				{	l->icon[cpt] = ico_dir ;
					l->icon_s[cpt] = ico_dir_s ;
					l->fileType[cpt]=5 ;
					l->ico[cpt]=1 ;
				}
			}	continue ; // go to next file
		}
		
		switch(*(int*)ext)
		{ 	case 0x4558462E : // fxe
			case 0x6578662E :
				if(l->rarList)
				{	l->icon[cpt] = ico_exe ;
					l->icon_s[cpt] = ico_exe_s ;
					l->fileType[cpt]=1 ;
					l->ico[cpt]=1 ;	l->lastParsed = cpt ;
				} else {	(l->entryName)[cpt] = l->entryNames + (cpt*80) ;
						if(GpFileOpen(parsTmp, OPEN_R, &file)) { sprintf((l->entryName)[cpt],"file open error"); break ; }
						GpFileRead(file,parsTmp+1024,4 + 4 + 4 + 32 + 32,0);
						GpFileClose(file) ;
					if(*(int*)(parsTmp+1024)==0x20657866) // it's an fxe
					{	*(char*)(parsTmp + 1024 + 4 + 4 + 4 + 31) = 0 ;
						if(*(char*)(parsTmp + 1024 + 4 + 4 + 4 + 32))	sprintf(l->entryName[cpt],"%s\nby %s",parsTmp + 1024 + 4 + 4 + 4,parsTmp + 1024 + 4 + 4 + 4 + 32) ;
						else									sprintf(l->entryName[cpt],"%s",parsTmp + 1024 + 4 + 4 + 4) ;
						l->icon[cpt] = ico_exe ;
						l->icon_s[cpt] = ico_exe_s ;
						l->fileType[cpt]=1 ;
						l->ico[cpt]=1 ;	l->lastParsed = cpt ;
					} else {	memcpy(l->entryName[cpt],(char*)"corupted fxe :(\0",16) ;
							l->fileType[cpt]=255 ; l->lastParsed = cpt ; // 255, corupt
					}
				}
			break ;
			case 0x444F4D2E : // mod
			case 0x646F6D2E :
				l->icon[cpt] = ico_mus ;
				l->icon_s[cpt] = ico_mus_s ;
				l->ico[cpt]=1 ;
				if(!l->rarList){
					(l->entryName)[cpt] = l->entryNames + (cpt*80) ;
					if(GpFileOpen(parsTmp, OPEN_R, &file)) { sprintf((l->entryName)[cpt],"file open error"); break ; }
					GpFileRead(file,l->entryName[cpt],20,0);
					GpFileClose(file) ;
					*(l->entryName[cpt] + 20) = 0 ;
					l->lastParsed = cpt ; l->fileType[cpt]=2 ; // mark as music
				} else { l->lastParsed = cpt ; l->fileType[cpt]=2 ; } // mark as music
			break ;
			case	0x4649472E : // gif
			case	0x6669672E :
			case	0x5843502E : // pcx
			case	0x7863702E :
			case	0x5447412E : // tga
			case	0x7467612E :
				l->icon[cpt] = ico_img ;
				l->icon_s[cpt] = ico_img_s ; l->ico[cpt]=1 ;
				l->fileType[cpt]=3 ; l->lastParsed = cpt ;
			break ;
			case 0x5241522E : // rar
			case 0x7261722E :
				l->icon[cpt] = ico_rar ;
				l->icon_s[cpt] = ico_rar_s ; l->ico[cpt]=1 ;
				l->fileType[cpt]=4 ; l->lastParsed = cpt ;				
			break ;
			case 0x41484C2E : // lha
			case 0x61686C2E :
				l->icon[cpt] = ico_lha ;
				l->icon_s[cpt] = ico_lha_s ; l->ico[cpt]=1 ;
				l->fileType[cpt]=4 ; l->lastParsed = cpt ;
			break ;
			case 0x4258472E : // gxb
			case 0x6278672E :
			case 0x4558472E : // gxe
			case 0x6578672E :
			case 0x4E49422E : // bin
			case 0x6E69622E :
				l->icon[cpt] = ico_exe ; l->icon_s[cpt] = ico_exe_s ; l->ico[cpt]=1 ;
				l->fileType[cpt]=1 ; l->lastParsed = cpt ;
			break ;
			case 0x4342472E : // gbc
			case 0x6362672E :
			case 0x3142472E : // gb1
			case 0x3162672E :
			case 0x0042472E : // gb
			case 0x0062672E :			
				l->icon[cpt] = ico_exe ; l->icon_s[cpt] = ico_exe_s ; l->ico[cpt]=1 ;
				if(!l->rarList){
					(l->entryName)[cpt] = l->entryNames + (cpt*80) ;
					if(GpFileOpen(parsTmp, OPEN_R, &file)) { sprintf((l->entryName)[cpt],"file open error"); break ; }
					GpFileSeek(file,FROM_BEGIN,308,0);
					GpFileRead(file,l->entryName[cpt],11,0);
					GpFileClose(file) ; *(l->entryName[cpt] + 11) = 0 ;
					l->fileType[cpt]=1 ; l->lastParsed = cpt ;
				} else { l->fileType[cpt]=1 ; l->lastParsed = cpt ; }
			break ;
			default : // unknow or dir
				/*if(!(l->rarList))
				{	GPFILEATTR attr ;	GpFileAttr(parsTmp,&attr) ;
					if(( (((attr.attr)>>4)&1) || (!strcmp(parsTmp,"gp:\\")) )&&(*(l->entryName[cpt]) != '.'))	// if directory and not . or ..
					{	l->icon[cpt] = ico_dir ;
						l->icon_s[cpt] = ico_dir_s ; l->ico[cpt]=1 ;
						l->fileType[cpt]=5 ; l->lastParsed = cpt ;						
					}	else	{ l->fileType[cpt]=6 ; l->lastParsed = cpt ; if(++nb >= 8) { clear=2 ; return ; }}	// mark as unknow
				}	else	{ l->fileType[cpt]=6 ; l->lastParsed = cpt ; if(++nb >= 8) { clear=2 ; return ; } }	// mark as unknow
				*/l->lastParsed = cpt ; l->fileType[cpt]=42 ;
			break ;
		}; // switch extention end
	   }// else if(l->lastParsed == l->entryNumber-2) loading=0 ; // if not parsed end
	}; // for end
	loading=0; clear=2; // stop loading if go out of the for routine ( -> all is parsed)
} // parsing routine end

void fadeOut(int z=0)
{	int r,v,b,i ;	unsigned int *color ;
	memcpy(screen[nflip],screen[!nflip],320*240) ;
	do {	for (i=0;i<256;i++)
		{	color = &(((uint*)0x14A00400)[i]) ;
			r =	((*color) >> 11)& 0x1f; if (r) r--;
			v =	((*color) >>  6)& 0x1f; if (v) v--;
			b =	((*color) >>  1)& 0x1f; if (b) b--;
			*color = (r<<11) | (v<<6) | (b<<1) | 1 ; // 1, for flu-
		}	if(!z) parsingList() ;	if(!(r||v||b)) break;
	} while(1) ;
}

