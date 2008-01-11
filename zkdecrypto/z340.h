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
#define		VERSION 					"0.7"
#define		ASCII_SIZE 					256
#define		MAX_CIPH_LENGTH 			4096
#define		SCRAMBLESTARTKEY 			0				// 0 or 1 : Determines whether to pre-scramble the starting KEY
#define		SETSOLVED					for(x=0;x<cuniq;x++) { for(y=0;y<clength;y++) if(cipher[y]==uniqstr[x]) solved[y]=key[x]; }

#define		ENGLISH						0
#define		GERMAN						1
#define		SPANISH						2

#define UNI_SIZE	26
#define BI_SIZE		676
#define TRI_SIZE	17576
#define TETRA_SIZE	456976
#define PENTA_SIZE	11881376

#define USE_BI		0x01
#define USE_TRI		0x02
#define USE_TETRA	0x04
#define USE_PENTA	0x08

/////////////////////////////////////////////////////////////////////// FUNCTIONS /////////////////////////////////////////////////////////////////

inline int		calcscore(const int,const char *,int&);
inline int		calclsoc(const int,const char *);
inline void		shufflekey(char *,const char *);
int				readcipher(char *);
int				read_ngraphs(char*,char*);

void			printferror(char *);
void			printcipher(int,char *,char *);
void			printfrequency(int,int *,char *);
//void			updateGUI(char *,char *,int,wxFrame *);
int				hillclimb(const char *,int,char *,const char *,SOLVEINFO&,int&);

void 			GetUnigraphs(double*);
int 			ReadNGraphs(const char*,int);
int 			WordPlug(Message&, const char *, int);
