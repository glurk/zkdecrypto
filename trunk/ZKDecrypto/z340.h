//////////////////////////////////////////////////////////////////////// INCLUDES /////////////////////////////////////////////////////////////////

#include 		<stdio.h>
#include 		<string.h>
#include 		<stdlib.h>
#include 		<time.h>
#include		<math.h>


//////////////////////////////////////////////////////////////////////// DEFINES //////////////////////////////////////////////////////////////////
#define		WM_USER_THREAD_UPDATE_BESTSCORE (WM_USER + 0x101)
#define		WM_USER_THREAD_UPDATE_PLAINTEXT (WM_USER + 0x102)
#define		_DEB 0
#define 		VERSION 					"0.5"
#define 		ASCII_SIZE 					256
#define 		MAX_CIPH_LENGTH 				4096
#define		SCRAMBLESTARTKEY 				0				// 0 or 1 : Determines whether to pre-scramble the starting KEY
#define		SETSOLVED					for(x=0;x<cuniq;x++) { for(y=0;y<clength;y++) if(cipher[y]==uniqstr[x]) solved[y]=key[x]; }

/////////////////////////////////////////////////////////////////// GLOBAL VARIABLES //////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////// FUNCTIONS /////////////////////////////////////////////////////////////////

inline int		calcscore(const int,const char *);
inline int		calclsoc(const int,const char *);
inline void		shufflekey(char *);
int			readcipher(char *);
void			read_ngraphs(void);

void			printferror(char *);
void			printcipher(int,char *,char *);
void			printfrequency(int,int *,char *);
void			printvowels(int,char *);
int hillclimb(char*,char*,int,CFormView * cfv);