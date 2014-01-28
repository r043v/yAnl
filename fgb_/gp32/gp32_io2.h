
//diferencias respecto a fNES, la cabecera de archivo, el (GG)/(SMS), y el sort

unsigned long MyGameSize; unsigned char *MyGame; unsigned long MyGameNo, MyGameDatNo;
/*
#define MAXFS 11
#define MAX_COUNT_FILE  1                                      //maximum number of file entries in one folder
#define MAX_PATH_LEN	1					//maximum length of full path name
ERR_CODE        err;
char		g_path_curr[MAX_PATH_LEN + 1];	//current path (in full path format)
GPDIRENTRY	g_list_file[MAX_COUNT_FILE];	//list of files in current path
unsigned long g_cnt_file = 0;                                 //number of files in current path
*/
F_HANDLE g__file;

//void LoadPacked(int, int, int){}

//ok
enum
{
UNK,
UNP,
GZ,
ZIP,
RAM
};

typedef struct {
                char type;
                unsigned short entry; 
                char file[8+3+1+1];
                unsigned long offset;
               } ini_game; 

struct {
        int version_ini;
        int num_games_in_ini;
   ini_game game[1]; //1 roms max
        } ini;
/*
void fs_add(char,unsigned short,char*,unsigned long){}
int fs_loadgame(char *, char *, unsigned long *, int , int) { return 0 ;}
void fs_scanfile(char *, char *){}
void fs_write_ini(char *, char *){}
int fs_read_ini(char *, char *)
{ return 0;
}
void fs_scandir(char *, char *){}
void fs(char *, char *, char *, unsigned char *){}
*/