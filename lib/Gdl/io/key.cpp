/* *** Gdl key part ***************************** */

#ifndef	_key_
#define _key_

#define kLeft	1
#define kDown	2
#define kRight	4
#define kUp		8

#define kL		16
#define kB		32
#define kA		64

#define kR		128
#define kStart	256
#define kSelect	512

#define kStartAndSelect	kStart|kSelect
#define kLAndR			kL|kR

int actualKey = 0 ;
int oldKey	  = 0 ;

int	keyUp		(ushort key)	{ return	((!(oldKey&key))&&(actualKey&key));	}
int	keyDown		(ushort key)	{ return	((oldKey&key)&&(!(actualKey&key)));	}
int	keyCst		(ushort key)	{ return	((oldKey&key)&&(actualKey&key))	;	}
int	keyPush		(ushort key)	{ return	(actualKey&key)	;					}

#ifdef	use_keyEvent
	ushort		eventkey[4] = {0,0,0,0} ;
	void	(*onkey[24*4])(void) ;
	
	void	setKeyEvent(ushort key, int (*eventType)(ushort), void (*whatDo)(void))
	{	int type, num=key ;
		
		if(eventType == keyUp) type = 0 ;
		else	if(eventType == keyDown) type = 1 ;
				else	if(eventType == keyCst) type = 2 ;
						else	if(eventType == keyPush) type = 3 ;
								else	return ;
		if(key > kL)
		{	switch(key)
			{
				case kR				: num=17 ; break;
				case kA				: num=18 ; break;
				case kB 			: num=19 ; break;
				case kStart			: num=20 ; break;
				case kSelect		: num=21 ; break;
				case kStartAndSelect: num=22 ; break;
				case kLAndR			: num=23 ; break;
			};
		}

		eventkey[type] |= key ;
		onkey[24*type+num] = whatDo ;
	}
#endif

void	refreshKey(void)
{
	oldKey = actualKey ;
	GpKeyGetEx(&actualKey) ;
}

void	updateKey(void)	{
	refreshKey();
	#ifdef	use_keyEvent
	{	int type, key ;
		if(oldKey == actualKey)	type=2;								// cstKey
		else	if((actualKey)&&(oldKey != actualKey)) type=0;		// upFront
				else	if((oldKey)&&(oldKey != actualKey)) type=1; // downFront
						else	if(actualKey)	type=4 ;
								else	return ;	
		key = actualKey ;
		if(type == 1) key = oldKey ;
		if(!key) return ;

		if(((eventkey[type])&key)==key)
		{
			int num = key ;
			if(num > kL)
			{	switch(num)
				{
					case kR				: num=17 ; break;
					case kA				: num=18 ; break;
					case kB 			: num=19 ; break;
					case kStart			: num=20 ; break;
					case kSelect		: num=21 ; break;
					case kStartAndSelect: num=22 ; break;
					case kLAndR			: num=23 ; break;
				};
			}

			if(onkey[24*type+num])	onkey[24*type+num]() ;
		}
	}
	#endif
}

void	killKeyFront(void)	{ oldKey = actualKey ; }

void	resetKey	(void)	{ actualKey = oldKey = 0 ; }

void	waitKey		(ushort key)
{
	oldKey = actualKey = key ;	// kill key up front if there is one
	while(!keyUp(key)) refreshKey() ;	// wait for key
	resetKey() ;	// set key code as 0 (no key pressed)
}


#endif