#pragma once

//////////////////////////////////////////////////////////////////////// INCLUDES /////////////////////////////////////////////////////////////////

#include 		<stdio.h>
#include 		<string.h>
#include 		<stdlib.h>
#include 		<time.h>
#include		<math.h>
#include		<map>
#include		<string>
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
	long max_tabu;
	int swaps;
	int max_tol;
	char *locked;
	char *exclude;
	char cribs[512][128];
	int num_cribs;
	char log_name[2048];
	std::map<std::string,int> *dictionary;
	std::map<std::string,int> *optima_tabu;
	int dict_words;

	float lang_ioc;
	float lang_dioc;
	float lang_ent;
	float lang_chi;
	int ioc_weight;
	int dioc_weight;
	int ent_weight;
	int chi_weight;
	
	//feedback
	char best_key[4096];
	int best_key4[14];
	int cur_tol;
	int cur_tabu;
	float last_time;
	int best_score;
	int best_block;

	//control
	int running;
	int num_words;
	int stray_letters;
	
	//callback functions
	void (*disp_all)(void);
	void (*disp_info)(void);
	unsigned long (*time_func)(void);
	int (*get_words)(const char*);
};

/////////////////////////////////////////////////////////////////////// FUNCTIONS /////////////////////////////////////////////////////////////////

int				calcscore(Message&,const int,const char *);
inline void		shufflekey(char *,const int,const int);

void			printcipher(int,const char *,char *,int,char *);
void			printfrequency(int,int *,char *,int);
int				hillclimb(Message&,const char *,int,char *,int);
int				hillclimb2(Message&,int,char*,int);
void			running_key(Message&,char*);
void			dictionary_vigenere(Message&);

void 			GetUnigraphs(double*);
int 			ReadNGraphs(const char*,int);
int 			WordPlug(Message&,const char*);

void			SetInfo(SOLVEINFO*);
