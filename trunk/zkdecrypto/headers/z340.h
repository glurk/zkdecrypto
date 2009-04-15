#pragma once

//////////////////////////////////////////////////////////////////////// INCLUDES /////////////////////////////////////////////////////////////////

#include 		<stdio.h>
#include 		<string.h>
#include 		<stdlib.h>
#include 		<time.h>
#include		<math.h>
#include		"message.h"


//////////////////////////////////////////////////////////////////////// DEFINES //////////////////////////////////////////////////////////////////

#define		ASCII_SIZE 					256
#define		KEY_SIZE					512

#define UNI_SIZE	26			//26
#define BI_SIZE		676			//26*26
#define TRI_SIZE	17576		//26*26*26
#define TETRA_SIZE	456976		//26*26*26*26
#define PENTA_SIZE	11881376	//26*26*26*26*26

//parameters for solve function
struct SOLVEINFO
{
	//parameters
	long max_fail;
	int swaps;
	int revert;
	char *locked;
	char *exclude;

	float lang_ioc;
	int ioc_weight;
	int ent_weight;
	int chi_weight;
	
	//feedback
	char best_key[KEY_SIZE];
	char *best_trans;
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

/////////////////////////////////////////////////////////////////////// FUNCTIONS /////////////////////////////////////////////////////////////////

inline int		calcscore(const int,const char *,SOLVEINFO&);
inline void		shufflekey(char *,const int,const int,SOLVEINFO&);

void			printcipher(int,const char *,char *,int,char *);
void			printfrequency(int,int *,char *,int);
int				hillclimb(const char *,int,char *,SOLVEINFO&,int);
int				hillclimb2(Message&,SOLVEINFO&,int);

void 			GetUnigraphs(double*);
int 			ReadNGraphs(const char*,int);
int 			WordPlug(Message&,const char*,SOLVEINFO&);
