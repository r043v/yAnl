
class GfaInfo
{
  public :
	int		sx, sy		;	// aproximate anim size x and y (all frames will be centered on this value at draw)
	int		px, py		;	// anim pos x and y
	int		frmNumber 	;	// number of .. frames
	//int	   timingType	;	// 0 : time is an int, apply to all flip   1 : time is an array, 1 value is 1 flip value
	int		playType	;	// 0 123123 1 1232123 2 123333 3 123111
	int		rstTime		;	// if != 0 and if the anim is stopped, the anim will be reset after rstTime ms
} ;

class Gfa : public GfaInfo
{
  public :
	Gfm ** frames	;	// the frames
	int *  time		;	// time between frames flip
	//int *  move	;
	//int *  dec	;	// space between frm size and anim size.
	
	int way ;
	int actualFrm	;
	long lastTick	;
	long stopTime	;
	int animated	;

	void draw(int px, int py)			;
	void draw()							;
	void addFrm(Gfm * frm, int frmTime)	;

	void init(int ptype=0, int rtime=0) ;
	void reset(void);
	void set(Pixel * anim, int sx, int sy, int frmNb, int frmTime ,Pixel *TrColor=0);
	void kill(void) ;
	Gfa(int ptype=0, int rtime=0) ;
	//Gfa(void) ;
	//void moveTo() ;
} ;
