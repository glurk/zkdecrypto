#ifndef _NGRAPHS_H
#define _NGRAPHS_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "message.h"

#define UNI_SIZE	26
#define BI_SIZE		676
#define TRI_SIZE	17576
#define TETRA_SIZE	456976
#define PENTA_SIZE	11881376

#define USE_BI		0x01
#define USE_TRI		0x02
#define USE_TETRA	0x04
#define USE_PENTA	0x08


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


int ReadNGraphs(char*,int);
int CalcScore(const char*,int);
int Solve(Message&,SOLVEINFO&,int&);

#endif
