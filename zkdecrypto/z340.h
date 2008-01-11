#pragma once

//////////////////////////////////////////////////////////////////////// INCLUDES /////////////////////////////////////////////////////////////////

#include 		<stdio.h>
#include 		<string.h>
#include 		<stdlib.h>
#include 		<time.h>
#include		<math.h>
#include		"message.h"

//parameters for solve function
struct SOLVEINFO
{
	//parameters
	long max_fail;
	int swaps;
	int revert;
	
	//feedback
	char best_key[256];
	int cur_try;
	int cur_fail;
	float last_time;
	int best_score;

	//control
	int running;
	
	//callback functions
	void (*disp_all)(void);
	void (*disp_info)(void);
	unsigned long (*time_func)(void);
};

//////////////////////////////////////////////////////////////////////// DEFINES //////////////////////////////////////////////////////////////////

#define		_DEB						0
#define		ASCII_SIZE 					256
#define		MAX_CIPH_LENGTH 			4096
#define		SETSOLVED					for(int x=0;x<cuniq;x++) { for(int y=0;y<clength;y++) if(cipher[y]==uniqstr[x]) solved[y]=key[x]; }

#define UNI_SIZE	26					// THIS IS EASIER TO UNDERSTAND THAN 26, 676, 17576
#define BI_SIZE		26*26				// HARD-CODED MYSTERY NUMBERS SUCK!
#define TRI_SIZE	26*26*26
#define TETRA_SIZE	26*26*26*26
#define PENTA_SIZE	26*26*26*26*26

#define USE_BI		0x01
#define USE_TRI		0x02
#define USE_TETRA	0x04
#define USE_PENTA	0x08

/////////////////////////////////////////////////////////////////////// FUNCTIONS /////////////////////////////////////////////////////////////////

inline int		calcscore(const int,const char *,int&);
inline int		calclsoc(const int,const char *);
inline void		shufflekey(char *,const char *);
int				readcipher(char *);

void			printcipher(int,char *,char *);
void			printfrequency(int,int *,char *);
//void			updateGUI(char *,char *,int,wxFrame *);
int				hillclimb(const char *,int,char *,const char *,SOLVEINFO&,int&);

void 			GetUnigraphs(double*);
int 			ReadNGraphs(const char*,int);
int 			WordPlug(Message&, const char *, int);
