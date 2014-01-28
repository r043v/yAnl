#define BLACK_COLOR 255

// features:
// auto save/load sram battery
// frameskip 1
// 133 mhz
// png

// to test:
// multiplayer
// core gfx (pre-rotated)

// to do:
// saves menu (st0 - st9) con png
// movie
// cheats
// multizip, do not preload max filesize per entry at fs

int version_emu=11003;

#include "..\lib\zlib\zlib.h"
#include "..\lib\zlib\zlib-gp32.h"

#include <stdlib.h>
#include <math.h>
#include "gp32.h"

#define DelayA(); while(!(GpKeyGet()&GPC_VK_FA)); while(GpKeyGet()&GPC_VK_FA);
#define DelayB(); while(!(GpKeyGet()&GPC_VK_FB)); while(GpKeyGet()&GPC_VK_FB);


#ifdef GP32_ADS
#define ONE_SECOND 2000 
#else
#define ONE_SECOND 1000
#endif

#ifdef GP32_ADS

int _x_GpKeyGet(void)
{
int value=0;

#define rKEY_A          0x4000
#define rKEY_B          0x2000
#define rKEY_L          0x1000
#define rKEY_R          0x8000
#define rKEY_UP         0x0800
#define rKEY_DOWN       0x0200
#define rKEY_LEFT       0x0100
#define rKEY_RIGHT      0x0400
#define rKEY_START      0x0040
#define rKEY_SELECT     0x0080
#define rPBDAT          (*(volatile unsigned *)0x1560000c)
#define rPEDAT          (*(volatile unsigned *)0x15600030)

unsigned long gpb = rPBDAT;   // 0x156
unsigned long gpe = rPEDAT;

//while ((gpb & 0x9000) != 0) { gpb = rPBDAT; gpe = rPEDAT;

if ((gpb & rKEY_LEFT) == 0) value|=GPC_VK_LEFT;
if ((gpb & rKEY_RIGHT) == 0) value|=GPC_VK_RIGHT;
if ((gpb & rKEY_UP) == 0) value|=GPC_VK_UP;
if ((gpb & rKEY_DOWN) == 0) value|=GPC_VK_DOWN;
if ((gpb & rKEY_A) == 0) value|=GPC_VK_FA;
if ((gpb & rKEY_B) == 0) value|=GPC_VK_FB;
if ((gpb & rKEY_L) == 0) value|=GPC_VK_FL;
if ((gpb & rKEY_R) == 0) value|=GPC_VK_FR;
if ((gpe & rKEY_SELECT) == 0) value|=GPC_VK_SELECT;
if ((gpe & rKEY_START) == 0) value|=GPC_VK_START;

return value;
}

#define GpKeyGet _x_GpKeyGet

#endif

char g_string[255];

#include "gp32\resource.h"
//#include "gp32\bg.h"
#include "gp32\gp32_video.h"
#include "gp32\gp32_window.h"
#include "gp32\gp32_crc.h"
#include "gp32\gp32_save.h"

//#include "gp32\gp32_irq.h"
#include "gp32\gp32_snd.h"

#include "gp32\gp32_good.h"
char szRomName[255];
#include "gp32\gp32_io2.h"

#include "gp32\gp32_menu.h"
