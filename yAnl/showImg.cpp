inline Pixel get_color_buffer(int x, int y, int sy, Pixel *buffer)
{
	return *(buffer + x*sy + (sy-1-y)) ;
}

inline void set_color_buffer(int x, int y, int sy, Pixel *buffer, Pixel color)
{
	*(buffer + x*sy + (sy-1-y)) = color ;
}

void	showImg(uchar * img, ushort largeur, ushort hauteur)
{		Gft * fnt = &nfont ;
		uchar * rotate_data=img ;
		ulong	cptx, cpty, startx, starty ;
		int		cpt ;//draw_pal = 0, cpt ;
		int	decalagex, size_draw_x ;
		int	decalagey, size_draw_y ;
		//int start_drax_x,start_drax_y, color ;
		int		scrool_speed = 8 ;
		ulong	scrool_time=0, speed_time=0, zoom_time=0 ;
		int		zoom  = 1	;
		int		nzoom = 2	;
		char buffer[512] ;
		int draw_pal=0 ;

		startx = (320-largeur)>>1 ;
		starty = (240-hauteur)>>1 ;

		decalagex=0 ;
		decalagey=0 ;

		size_draw_x = largeur ;
		size_draw_y = hauteur ;

		if(hauteur>240) {  starty=0 ;  size_draw_y = 240 ; }
		if(largeur>320) {  startx=0 ;  size_draw_x = 320 ; }
		loading=0 ;
		if(rotate_data)
		do {
			refreshKey() ;

			if(startx||starty)	clrScr() ;

			if(!zoom) // zoom 1/2
			{			
				  for(cptx=decalagex*nzoom;cptx<decalagex*nzoom+size_draw_x;cptx+=nzoom)
					for(cpty=decalagey*nzoom;cpty<decalagey*nzoom+size_draw_y;cpty+=nzoom)
					{	//*(screen[nflip] + (startx+(cptx/nzoom)-decalagex)*240 + (239-(starty+(cpty/nzoom)-decalagey))) = get_color_buffer(cptx, cpty, hauteur, rotate_data) ;
					} ;
			}
			else
			{
				if(zoom==1)
				{	  for(cptx=decalagex;cptx<decalagex+size_draw_x;cptx++)
						for(cpty=decalagey;cpty<decalagey+size_draw_y;cpty++)
						{	*(screen[nflip] + (startx+cptx-decalagex)*240 + (239-(starty+cpty-decalagey))) = get_color_buffer(cptx, cpty, hauteur, rotate_data) ;
						} ;
				} else {
					  for(cptx=decalagex;cptx<decalagex+(size_draw_x/zoom);cptx++)
						for(cpty=decalagey;cpty<decalagey+(size_draw_y/zoom);cpty++)
						{	rectFill(startx+(cptx-decalagex)*zoom,starty+(cpty-decalagey)*zoom,zoom,zoom,get_color_buffer(cptx, cpty, hauteur, rotate_data)) ;
						} ;
					}
			}

			if(!keyPush(kSelect))
			{
				if(keyPush(kRight)) // right
				{
					if(zoom) {	for(cpt=0;cpt<scrool_speed;cpt++)	if(decalagex+(size_draw_x/zoom)<largeur)	decalagex++ ; }
					else	{	if(!startx) for(cpt=0;cpt<scrool_speed;cpt++)	if(decalagex < largeur/2-320)		decalagex++ ;
							//if((size_draw_x+decalagex)*nzoom>largeur*nzoom) decalagex -= ((decalagex+size_draw_x)*nzoom-(largeur*nzoom)) ;
						}					
					scrool_time = GpTickCountGet() ;
				}
	
				if(keyPush(kLeft)) // left
				{
					for(cpt=0;cpt<scrool_speed;cpt++)	if(decalagex)	decalagex-- ;
					
					scrool_time = GpTickCountGet() ;
				}
				
				if(keyPush(kDown)) // down
				{
					if(zoom) {	for(cpt=0;cpt<scrool_speed;cpt++)	if(decalagey+(size_draw_y/zoom)<hauteur)	decalagey++ ; }
					else	{	if(!starty) for(cpt=0;cpt<scrool_speed;cpt++)	if(decalagey < hauteur/2-240)		decalagey++ ;
							//if((startx+size_draw_y+decalagey)*nzoom>hauteur*nzoom) decalagey -= ((startx+decalagey+size_draw_y)*nzoom-(hauteur*nzoom)) ;
						}


					scrool_time = GpTickCountGet() ;
				}
				
				if(keyPush(kUp)) // up
				{
					for(cpt=0;cpt<scrool_speed;cpt++)	if(decalagey)	decalagey-- ;
					
					scrool_time = GpTickCountGet() ;
				}
			} else {
				if(keyUp(kDown)) // down, zoom -
				{
					if(zoom>1)
					{
						short old, _new, dif, cpt ;
						
						old = (size_draw_x/zoom) ;
						_new = (size_draw_x/(zoom-1)) ;
						dif = (_new-old)/2 ;
						
						for(cpt=0;cpt<dif;cpt++)
							if(decalagex)	decalagex-- ;
						
						old = (size_draw_y/zoom) ;
						_new = (size_draw_y/(zoom-1)) ;
						dif = (_new-old)/2 ;

						for(cpt=0;cpt<dif;cpt++)
							if(decalagey)	decalagey-- ;

						zoom-- ;
						
						startx = (320-largeur*zoom)>>1 ;
						starty = (240-hauteur*zoom)>>1 ;
				
						size_draw_y = hauteur*zoom ;
						size_draw_x = largeur*zoom ;
				
						if(hauteur*zoom>240) {  starty=0 ;  size_draw_y = 240 ; }
						if(largeur*zoom>320) {  startx=0 ;  size_draw_x = 320 ; }
						
						if(size_draw_x/zoom+decalagex>largeur) decalagex -= (decalagex+size_draw_x/zoom)-largeur ;
						if(decalagey+size_draw_y/zoom>hauteur) decalagey -= (decalagey+size_draw_y/zoom)-hauteur ;
					}		/*else if(zoom==1)
							{
								short cpt ;
		
								for(cpt=0;cpt<size_draw_x/2;cpt++)
									if(decalagex)	decalagex-- ;
		
								for(cpt=0;cpt<size_draw_y/2;cpt++)
									if(decalagey)	decalagey-- ;
		
								zoom=0 ;
		
								startx = (320-largeur/2)>>1 ;
								starty = (240-hauteur/2)>>1 ;
						
								size_draw_y = hauteur ;
								size_draw_x = largeur ;
						
								if(hauteur/2>240) {  starty=0 ;  size_draw_y = 240*2 ; }
								if(largeur/2>320) {  startx=0 ;  size_draw_x = 320*2 ; }

								if(size_draw_x*2+decalagex>largeur*2) decalagex -= (decalagex+size_draw_x*2)-largeur*2 ;
								if(decalagey+size_draw_y*2>hauteur*2) decalagey -= (decalagey+size_draw_y*2)-hauteur*2 ;
								
								if(decalagey > hauteur/2-240) decalagey = hauteur/2-240 ;
								if(decalagex > largeur/2-320) decalagex = largeur/2-320 ;	
							}*/

					zoom_time = GpTickCountGet() ;
				}
				
				if(keyUp(kUp)) // up, zoom +
				{					
					if(zoom < 20)
					{
						short old, _new, dif, cpt ;
						
						if(zoom)
						{
							old = (size_draw_x/zoom) ;
							_new = (size_draw_x/(zoom+1)) ;
							dif = (old-_new)/2 ;

							for(cpt=0;cpt<dif;cpt++)
								if(decalagex+(size_draw_x/zoom)<largeur)	decalagex++ ;

							old = (size_draw_y/zoom) ;
							_new = (size_draw_y/(zoom+1)) ;
							dif = (old-_new)/2 ;
	
							for(cpt=0;cpt<dif;cpt++)
								if(decalagey+(size_draw_y/zoom)<hauteur)	decalagey++ ;
						}
						else
						{
							size_draw_y/=2 ;
							size_draw_x/=2 ;
							
							old = (size_draw_x*2) ;
							_new = (size_draw_x/2) ;
							dif = (old-_new)/2 ;
							
							for(cpt=0;cpt<dif;cpt++)
								if(decalagex+(size_draw_x*2)<largeur)	decalagex++ ;
								
							old = (size_draw_y*2) ;
							_new = (size_draw_y/2) ;
							dif = (old-_new)/2 ;
	
							for(cpt=0;cpt<dif;cpt++)
								if(decalagey+(size_draw_y*2)<hauteur)	decalagey++ ;
						}

						zoom++ ;
						
						startx = (320-largeur*zoom)>>1 ;
						starty = (240-hauteur*zoom)>>1 ;
				
						size_draw_y = hauteur*zoom ;
						size_draw_x = largeur*zoom ;
				
						if(hauteur*zoom>240) {  starty=0 ;  size_draw_y = 240 ; }
						if(largeur*zoom>320) {  startx=0 ;  size_draw_x = 320 ; }
					}
					
					zoom_time = GpTickCountGet() ;
				}
			}

			if(keyUp(kL)) // L
			{	if(scrool_speed>1) scrool_speed-- ;
				speed_time = GpTickCountGet() ;
			}

			if(keyUp(kR)) // R
			{	if(scrool_speed<50) scrool_speed++ ;
				speed_time = GpTickCountGet() ;
			}

			if(speed_time+2000>GpTickCountGet())
			{	sprintf(buffer,"scrool speed : %i",scrool_speed) ;	
				fnt->drawIi(buffer,317,40) ;
			}
			
			if(scrool_time+2000>GpTickCountGet())
			{	sprintf(buffer,"%i-%i",decalagex,decalagey) ;
				fnt->drawIi(buffer,317,50) ;
			}

			if(zoom_time+2000>GpTickCountGet())
			{	if(zoom)	sprintf(buffer,"zoom %i/1",zoom) ;
				else		sprintf(buffer,"zoom 1/2") ;
				
				fnt->drawIi(buffer,317,60) ;
			}
			
			if(draw_pal)
			{	fnt->drawInt(largeur,5,5) ;
				fnt->drawInt(hauteur,35,5) ;
				fnt->drawInt(hauteur*largeur,80,5) ;
				//tr4k.font.drawInt(taille_fichier,140,5) ;
				//tr4k.font.draw(pcx_file+8,317,5) ;
	
				for(cpt=0;cpt<64;cpt++)
				{	rectFill(5*cpt, 15, 4, 4, cpt);
					rectFill(5*cpt, 20, 4, 4, cpt+64);
					rectFill(5*cpt, 225, 4, 4, cpt+128);
					rectFill(5*cpt, 230, 4, 4, cpt+192);
				} ;
			}

			flipScreen() ;
			
			if(keyUp(kA)) draw_pal^=1 ;
			
		} while(!keyUp(kB)) ;
}
