#ifndef	_dma_
#define	_dma_

/* DMA */
#define rDISRC0		(*(volatile unsigned *)0x14600000)
#define rDIDST0		(*(volatile unsigned *)0x14600004)
#define rDCON0		(*(volatile unsigned *)0x14600008)
#define rDSTAT0		(*(volatile unsigned *)0x1460000c)
#define rDCSRC0		(*(volatile unsigned *)0x14600010)
#define rDCDST0		(*(volatile unsigned *)0x14600014)
#define rDMASKTRIG0	(*(volatile unsigned *)0x14600018)

inline void dma0cpy(Pixel * lpDest, Pixel * lpSource, ulong dwLength, uint cpySz = 0)
{
	rDISRC0 = (long) lpSource;
	rDIDST0 = (long) lpDest;
	rDCON0 = 		(0 << 30) | 		// demand mode
				(1 << 29) |			// DREQ & DACK are synched to HCLK (AHB)
				(0 << 28) |			// disable DMA0 int
				(0 << 27) |			// unit transfer
				(1 << 26) |			// whole servmode
				(0x00 << 24) |		// HWSRCSEL ?? not used
				(0 << 23) |			// sw request mode
				(1 << 22) |			// auto reload off
				(cpySz << 20) |			// data size == char // short = 1 (0x02)
				(dwLength);			// count..
	// start it
	rDMASKTRIG0 = 0x03;
}

#endif