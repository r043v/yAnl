class GmpInfo
{
  public :
	int		sx, sy		;	// map size in tiles
	int		mdsx, mdsy	;	// shift value to divide with tile size
	int		stx, sty	;	// tile size in pixel
	int		ntx,nty		;	// tiles number draw on a screen
	int		x, y		;	// scroll x and y
	int		nbtiles		;	// tiles number into the tileset
} ;

class Gmp : public GmpInfo
{
  public :
  	int	*	animated	;	// 
	Gfm * tileset		;	// the tiles, in Gfm or Gfa, depend of the animated value
	short * Array		;	// the map array

	void draw()	;
	void set(short * array, Gfm * tlset, int frmNb, int msx, int msy) ;
	void set(short * array, Pixel * tlset, int frmNb, int msx, int msy, int tsx, int tsy) ;
} ;

