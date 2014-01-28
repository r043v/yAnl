int cancel(void)
{	refreshKey() ;
	if(keyPush(kSelect)) return 1 ;
	return 0 ;
}

void LaunchFXE(const char* lpszPath)
{
        ERR_CODE result;
        F_HANDLE fHandle;

        unsigned long dwDummy;
        unsigned long dwHeader, dwFileLength, dwInfoLength;

        int    i, j, k;
        int    n_read, n_axf_ro, n_axf_rw;
        unsigned char *p_axf,  *p_axf_ro, *p_axf_rw;
        unsigned char *p_work, *p_temp;

        result = GpFileOpen(lpszPath, OPEN_R, &fHandle);
        if (result != SM_OK)
                return;

        GpFileRead(fHandle, &dwHeader,     sizeof(unsigned long), &dwDummy);
        GpFileRead(fHandle, &dwFileLength, sizeof(unsigned long), &dwDummy);
        GpFileRead(fHandle, &dwInfoLength, sizeof(unsigned long), &dwDummy);

        GpFileSeek(fHandle, FROM_BEGIN, dwInfoLength + 8, (long *) &dwDummy);

        GpFileRead(fHandle, &n_axf_ro, sizeof(int), &dwDummy);
        GpFileRead(fHandle, &n_axf_rw, sizeof(int), &dwDummy);

        n_read = n_axf_ro + n_axf_rw;
        p_axf = (uchar*)malloc(n_read + 4);
        if (!p_axf)
        {       GpFileClose(fHandle);
                return;
        }

        p_axf_ro = p_axf + 4;
        p_axf_rw = p_axf_ro + n_axf_ro;
        *(int*) p_axf = n_read;

        GpFileRead(fHandle, p_axf_rw, n_axf_rw, &dwDummy);

        j = n_axf_rw;
        p_work = p_axf_rw;
        for (i = 0; i < j; i ++)
        {
                *p_work ^= 0xff;
                p_work ++;
        }

        GpFileRead(fHandle, p_axf_ro, n_axf_ro, &dwDummy);
        GpFileClose(fHandle);

        j = n_axf_ro;
        k = n_axf_rw;
        p_work = p_axf_ro;
        p_temp = p_axf_rw;
        for (i = 0; i < j; i ++)
        {
                *p_work ^= *p_temp;
                p_work ++;
                p_temp ++;
                if (p_temp >= (p_axf_rw + k))
                        p_temp = p_axf_rw;
        }

        p_work = (unsigned char*) lpszPath;
        while (*p_work)
                p_work ++;
        while (*p_work != '.')
                p_work --;
        *p_work = 0;

        if(!cancel()){
	        GpClockSpeedChange (67800000, 0x69032, 3);
    	    GpAppExecute((char*) p_axf, 0);
    	} else free(p_axf) ;
}

//////////////////////////////////////////////////////////////////////////////////////
// void GpAppReset()
// reset the current application
//////////////////////////////////////////////////////////////////////////////////////

void GpAppReset(void)
{ GpAppExecute((char*)(0xC000000-4), NULL); }

void memLaunchFXE(char* fxe)
{	unsigned long dwHeader, dwFileLength, dwInfoLength;
	int    i, j, k;
	int    n_read, n_axf_ro, n_axf_rw;
	unsigned char *p_axf,  *p_axf_ro, *p_axf_rw;
	unsigned char *p_work, *p_temp;

	char * f = fxe ;

	dwHeader = *(unsigned long*)f ; f+= sizeof(unsigned long) ;
	dwFileLength = *(unsigned long*)f ; f+= sizeof(unsigned long) ;
	dwInfoLength = *(unsigned long*)f ; f+= sizeof(unsigned long) ;

	f = fxe + dwInfoLength + 8 ;

	n_axf_ro = *(int*)f ; f+=sizeof(int) ;
	n_axf_rw = *(int*)f ; f+=sizeof(int) ;

	n_read = n_axf_ro + n_axf_rw;
	p_axf = (unsigned char*) malloc(n_read + 4);

	p_axf_ro = p_axf + 4;
	p_axf_rw = p_axf_ro + n_axf_ro;
	*(int*) p_axf = n_read;

	memcpy(p_axf_rw,f,n_axf_rw); f+=n_axf_rw ;

	j = n_axf_rw;
	p_work = p_axf_rw;
	for (i = 0; i < j; i ++)
	{
	        *p_work ^= 0xff;
	        p_work ++;
	}

	memcpy(p_axf_ro,f,n_axf_ro);

	j = n_axf_ro;
	k = n_axf_rw;
	p_work = p_axf_ro;
	p_temp = p_axf_rw;
	for (i = 0; i < j; i ++)
	{
	        *p_work ^= *p_temp;
	        p_work ++;
	        p_temp ++;
	        if (p_temp >= (p_axf_rw + k))
	                p_temp = p_axf_rw;
	}

	if(!cancel()){
	    GpClockSpeedChange (67800000, 0x69032, 3);
	    GpAppExecute((char*) p_axf, 0);
	} else { free(fxe) ; free(p_axf) ; }
}

