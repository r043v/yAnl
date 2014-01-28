
int RGB2INT(int R,int G,int B) {
  if(R>20) R-=20 ; else R=0 ;
  if(G>20) G-=20 ; else G=0 ;
  if(B>20) B-=20 ; else B=0 ;
 return ((R>>3)<<11)|((G>>3)<<6)|((B>>3)<<1) | 0 ;
}

typedef u16 Screen[GPC_LCD_WIDTH][GPC_LCD_HEIGHT];		// rotated!

GPDRAWSURFACE gpDrawSurfaces[2];
int gpDrawSurfaceIndex = 0;

int GP32_RGB(int r,int g,int b) {
	if(r>20) r-=20 ; else r=0 ;
	if(g>20) g-=20 ; else g=0 ;
	if(b>20) b-=20 ; else b=0 ;
	return (((r)<<11) | ((g)<<6) | ((b)<<1)) | 0 ;
}

void GP32_System_Reset()
{unsigned long (*jump)();
jump=0x0000000;jump();
}

unsigned char *Picture;    //[320*240];
unsigned char *PictureAlt; //[320*240];

int GP32_Video; long Surface[4], SurfaceIndex;

void Gp32Flip(void)
{
 volatile long *lcdcon1 = (long *)0x14a00000;
 volatile long *lcdaddr1 = (long *)0x14a00014;
 volatile long *lcdaddr2 = (long *)0x14a00018;

 while(1!=(((*lcdcon1)>>18)&0x1ff));
 while(0!=(((*lcdcon1)>>18)&0x1ff));

 if(!SurfaceIndex)
 {
        *lcdaddr1=Surface[0];
        *lcdaddr2=Surface[1];
 }
 else
 {
        *lcdaddr1=Surface[2];
        *lcdaddr2=Surface[3];
 }

 Picture=(unsigned char *)0x0c7b4000+320*240*(SurfaceIndex^=1);
}

void GP32_Video_Init(int mode)
{
  volatile long *lcdcon1 = (long *)0x14a00000;
  volatile long *lcdaddr1 = (long *)0x14a00014;
  volatile long *lcdaddr2 = (long *)0x14a00018;


	// high color mode
        if(mode==8)
        GpGraphicModeSet(GPC_DFLAG_8BPP, NULL);
        else
        GpGraphicModeSet(GPC_DFLAG_16BPP, NULL);

        GP32_Video=mode;

        GpLcdSurfaceGet(&gpDrawSurfaces[0], 0);  
        GpLcdSurfaceGet(&gpDrawSurfaces[1], 1);  
	
	// show other buffer
        GpSurfaceSet(&gpDrawSurfaces[0]);

        GpSurfaceFlip(&gpDrawSurfaces[0]);
        Surface[0]=*lcdaddr1;
        Surface[1]=*lcdaddr2;

        GpSurfaceFlip(&gpDrawSurfaces[1]);
        Surface[2]=*lcdaddr1;
        Surface[3]=*lcdaddr2;

        SurfaceIndex=0;
	// enable display
	GpLcdEnable();
}


#define GP32_Video_Copy() gm_memcpy((unsigned short *)(gpDrawSurface->ptbuffer), Picture, GPC_LCD_WIDTH*GPC_LCD_HEIGHT*2);  

#define GP32_Video_Flip() GpSurfaceFlip(&gpDrawSurfaces[gpDrawSurfaceIndex]); gpDrawSurfaceIndex ^= 1;                        


//#define printf(a) {char t[255];sprintf(t,a);GpTextOut(NULL,gpDrawSurface,0,0,t,0);}
int y_print=0;
#define gp32_printf(t) { GpTextOut(NULL,(GPDRAWSURFACE *)&gpDrawSurfaces[gpDrawSurfaceIndex],0,y_print,t,0); GP32_Video_Flip(); GpTextOut(NULL,(GPDRAWSURFACE *)&gpDrawSurfaces[gpDrawSurfaceIndex],0,y_print,t,0); GP32_Video_Flip(); y_print+=8; }




unsigned char gfx_pal_backup[768];
float Gamma=GAMMA; int Fade=0;

/** SetColor() ***********************************************/
/** Set color N (0..15) to (R,G,B).                         **/
/************************************ TO BE WRITTEN BY USER **/
void SetColor(unsigned char N,unsigned char R,unsigned char G,unsigned char B)
{
unsigned long *palette=(unsigned long *)0x14A00400; //32 bits?

if(!Fade) //fade not in progress, update
{ //if(R>20) R-=20 ; else R=0 ;
  //if(G>20) G-=20 ; else G=0 ;
  //if(B>20) B-=20 ; else B=0 ;
  
 gfx_pal_backup[N*3+0]=R;
 gfx_pal_backup[N*3+1]=G;
 gfx_pal_backup[N*3+2]=B;

 R=(unsigned char)(pow(((float)R)/255.0,Gamma)*255);
 G=(unsigned char)(pow(((float)G)/255.0,Gamma)*255);
 B=(unsigned char)(pow(((float)B)/255.0,Gamma)*255);

 palette[N]=RGB2INT(R,G,B);
}
}

void SetPalette(unsigned char *p)
{
 int i;
 for(i=0;i<256;i++) SetColor(i,p[i*3+0],p[i*3+1],p[i*3+2]);
}






