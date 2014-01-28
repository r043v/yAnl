inline ushort m_getw(char * p) { return *p + *(p+1)*256 ; }

int mLoadTGA(char* file, uchar *outpix, uint * pal, int* w, int* h, int size)
{	char *fptr = file ;
	if(*(fptr + size - 7) != 'X') { error(fptr+size-18);	return 0 ; }	fptr += 18 ;
	int width	= m_getw(file+12) ;	if(w)	*w=width	;
	int height	= m_getw(file+14) ;	if(h)	*h=height	;
	if(!outpix && !pal)	return 1 ;

	if(pal)
	{	if (*(file+1) != 1)	return 0 ;
		memset(pal, 0, 1024);	fptr += *file ;
		int i = m_getw(file+5) ;	if(i>256) i=256 ; // cmlen
		register int r,g,b, color ;	int j=0 ;
		if(i)	switch (*(file+7))	// cmdepth
        		{	case 15:
					case 16:
		                do{	color = *fptr++ ; color += *fptr++ * 256	;
							r = (color & 0x7C00) >> (10-3);
							g = (color & 0x03E0) >> (5-3);
							b = (color & 0x001F) << (3-0);
							pal[j] = getGpColor(r,g,b) ;
						} while(++j<i) ;
		                break;
		            case 24:
   		                do{	b = *fptr++;
							g = *fptr++;
							r = *fptr++;
							pal[j] = getGpColor(r,g,b) ;
						} while(++j<i) ;
		                break;
		            case 32:
   		                do{	b = *fptr++;
							g = *fptr++;
							r = *fptr++;	fptr++ ;
							pal[j] = getGpColor(r,g,b) ;
						} while(++j<i) ;
		                break;
				};
    }

    if(outpix)
	{   if(*(file+2) != 1 && *(file+2) != 9) return 0 ; // imgtype
        if(*(file+16) != 8) return 0 ; // bpp

        fptr = file + 18 + *file ; // idsize
        if(*(file+1)) fptr += (*(file+7)+1)/8*m_getw(file+5); // cmapt cmdepth cmlen

        if(*(file+2) == 1) {     // Raw
        	int i = height ;
            while(i)	memcpy(outpix + (height - (i--) - 1)*width, fptr+=width, width) ;
        } else {                    // RLE
            long size;
            uchar *outp;

            for(int i = 0; i < height; i++) {
                size = width;
                outp = outpix + (height - i - 1)*width;
                while (size > 0) {
                    byte p;

                    p = *fptr++ ;
                    if (p & 0x80) {
                        byte v;
                        v = *fptr++ ;
                        p = p & 0x7F;
                        p += 1;
                        memset(outp, v, p);
                    } else {
                        p += 1;
                        memcpy(outp,fptr,p) ;	fptr+=p ;
                    }
                    outp += p;
                    size -= p;
                }
            }
        }
    }

    return 1 ;
}
