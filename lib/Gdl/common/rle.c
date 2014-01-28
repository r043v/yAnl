#ifndef _rle_
#define _rle_

// natty dread powered *

void * RLE(void * bufSrc, uint *bufSize)
{
	char	*bufDst, *sBufPtr, *dBufPtr ;
 	char	value ;
 	uint	dBufASize, dBufSizeRemain ;
 	long	totalCount = *bufSize ;	// asign control var with src buf size.
 	uchar	repeat ;
 	void	*rleBf ;

  	dBufASize = dBufSizeRemain = ((*bufSize)*3)>>1 ;
 	if((dBufPtr = bufDst = (char *)gm_malloc(dBufASize)) == NULL) return NULL ;// { showMsg("unable to alocate dest. buffer in RLE.   bye !","error") ; return NULL ; } ;
 	sBufPtr = (char*)bufSrc ;

 	do {
		value = *sBufPtr ; sBufPtr++ ; (*bufSize)-- ; repeat=1 ; // get value from src buf

		while((*sBufPtr == value)&&(repeat < 63)&&(*bufSize>1))	// count sucessive same value number
		{   repeat++ ; sBufPtr++ ; (*bufSize)-- ;	} ;

		totalCount -= repeat ;	// decrease Control var with the data number from src buf write in the dest buffer.

		//if(!repeat)	showMsg("repeat found of 0 in RLE.","error") ;

   		if((repeat>1)||((value & 0xC0) == 0xC0))	// if sucessiv value or if value use the 2 first bits, RLE
   		{	*dBufPtr = (0xC0 | repeat) ;	dBufPtr++ ;	// write RLE flag and repeat number
   			*dBufPtr = value ;		dBufPtr++ ;	// write value to be repeteted
		          	dBufSizeRemain-=2 ;			// decrese free size of the dest. buffer
          		}
   		else	{ *dBufPtr = value ; dBufPtr++ ; dBufSizeRemain-- ; } // no RLE, just copy (if !repeat, allready copy the data.)

   		if(dBufSizeRemain < 10)	// to less free buffer, need realloc.
		{	return NULL ;	}
 	} while(*bufSize) ;	// stop when finish read source buffer

	*bufSize = (dBufPtr - bufDst) ;	// compute size used by RLE buffer

	rleBf = gm_malloc(*bufSize) ;
	memcpy(rleBf,bufDst,*bufSize);
	free(bufDst) ;	return rleBf ;
}

void * unRLE(void * bufSrc, uint *rleSize, uint *unRleSize)
{
	char *bufDst, *sBufPtr, *dBufPtr, *dBufEnd, value ;
	ushort repeat, show0error=0 ;
	uint * bufSize = rleSize ;
	int totalCount=1 ;
	uint size ;
	void * unrleBf ;

	if(unRleSize)	{ size = *unRleSize + *rleSize ; totalCount = *unRleSize ; }
	else			size = (*rleSize)*4 ;

	dBufPtr = bufDst = (char *)malloc(size) ;

 	if(!bufDst)	error("alloc error in unRLE.") ;
 	dBufEnd = bufDst + size ;
	sBufPtr = (char*)bufSrc ;

	do {	value = *sBufPtr ; sBufPtr++ ;
			if ((value & 0xC0) == 0xC0)  		// 0xC0 = 11000000 	->	rle
			{	repeat = value & 0x3F ;     	// 0x3F = 00111111	->	count = 2^6 -1 = 63 continuous pixels max
				value = *sBufPtr ; sBufPtr++ ;	// getvalue to repeat
				if(repeat)
				{
       	       		if(unRleSize) totalCount-=repeat ;
 	       		    // write the repeted pixels
  	       			while(repeat--) { *dBufPtr = value ; dBufPtr++ ; } ;
  	       		} else  if(!show0error) { error("read 0 as repeat in unRLE.") ;			 show0error++ ; }
  	      	} else { *dBufPtr = value ; dBufPtr++ ; if(unRleSize) totalCount-- ; }	// no rle, just copie value.

		  	if(dBufEnd - dBufPtr < 64)	// to less free buffer, need realloc.
			{
				free(bufDst) ;
				error("too less buffer to continu unRLE.");
				return NULL  ;
				/*uint dec, dec2 ; // up about 1ko
				dec = dBufPtr - bufDst ; dec2 = dBufEnd - bufDst ;
  				if(!(dBufEnd = dBufPtr = bufDst = (char *)realloc(bufDst,dec2 + 10240))) error("error on realloc in unRLE.") ;
 	       		dBufPtr+=dec ; dBufEnd += dec2+10240 ;*/
    		}

	} while((sBufPtr - (char*)bufSrc < *bufSize)&&(totalCount>0)) ;	// exit on finish reading all the src buffer, or finish read the number who want.

	size = dBufPtr - bufDst ;

	if(!size) error("size of 0 in unRLE.");

	unrleBf = gm_malloc(size) ;
	if(!unrleBf) error("alloc error");
	
	memcpy(unrleBf,bufDst,size);	
	free(bufDst) ;

	if(unRleSize)	*unRleSize = size ;
	else			*rleSize   = size ;

	return unrleBf ;	// return the dest. buffer
}

#endif