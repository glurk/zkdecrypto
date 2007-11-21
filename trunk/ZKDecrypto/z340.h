#pragma once
//////////////////////////////////////////////////////////////////////// INCLUDES /////////////////////////////////////////////////////////////////

#include 		<stdio.h>
#include 		<string.h>
#include 		<stdlib.h>
#include 		<time.h>
#include		<math.h>

#define 		VERSION 					"0.5"
#define 		ASCII_SIZE 					256
#define 		MAX_CIPH_LENGTH 				4096
//#define 		DEBUG 					0				// 0 or 1 : Sets DEBUG mode
#define		SCRAMBLESTARTKEY 				0				// 0 or 1 : Determines whether to pre-scramble the starting KEY
#define		SETSOLVED	for(x=0;x<cuniq;x++) { for(y=0;y<clength;y++) if(cipher[y]==uniqstr[x]) solved[y]=key[x]; }
// Period parameters 
#define N 624
#define M 397
	
#define MATRIX_A 0x9908b0dfUL  // constant vector a 
#define UMASK 0x80000000UL // most significant w-r bits 
#define LMASK 0x7fffffffUL // least significant r bits 
#define MIXBITS(u,v) ( ((u) & UMASK) | ((v) & LMASK) )
#define TWIST(u,v) ((MIXBITS(u,v) >> 1) ^ ((v)&1UL ? MATRIX_A : 0UL))

static unsigned long state[N];  //the array for the state vector  
static int left = 1;
static int initf = 0;
static unsigned long *next;
//////////////////////////////////////////////////////////////////////// DEFINES //////////////////////////////////////////////////////////////////
//using namespace std;

class z340
{
public:

	z340();
	~z340();
/////////////////////////////////////////////////////////////////// GLOBAL VARIABLES //////////////////////////////////////////////////////////////
int hillclimb();
		//LONGEST STRING OF CONSONANTS DATA

short int		bigraphs[26*26];
short int 		trigraphs[26*26*26];
short int 		tetragraphs[26*26*26*26];
short int		pentagraphs[26*26*26*26*26];

//int freqs[26];
//int lsocdata[26];

char			cipher[MAX_CIPH_LENGTH];
int			keylength;

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

////////////////////////////////////Random Number Generator///////////////////////////////////////////////
inline unsigned long genrand_int32(void);
inline static void next_state(void);
void static init_genrand(unsigned long s);
private:

};




