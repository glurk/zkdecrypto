#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "map.h"
#include "unicode.h"
#include "strarray.h"
#include "macros.h"

#define MAX_SYM		256
#define MAX_PAT_LEN	15

#pragma warning( disable : 4996)  //STOP STUPID MSVS2005 "strcpy" WARNINGS

struct NGRAM
{
	char string[MAX_PAT_LEN+1];
	int length;
	int freq;
	int *positions;
	int pos_size;

	NGRAM *left;
	NGRAM *right;
};

/*Message*/
class Message
{
public:
	Message() {msg_len=0; patterns=NULL; num_patterns=0; good_pat=0; min_pat_len=2; cipher=NULL; plain=NULL;}
	~Message() {if(cipher) delete[] cipher; if(plain) delete[] plain; if(patterns) ClearPatterns(patterns);}

	int Read(const char*);
	int Write(const char*);
	void SetCipher(const char*);
	void Insert(int,const char*);
	
	const char * GetCipher() {return cipher;}
	const char * GetPlain() {Decode(); return plain;}
	int GetLength() {return msg_len;}
	int GetRow(int,int,char*);
	int GetColumn(int,int,char*);
	
	void SetExpFreq();
	void GetExpFreq(int*);
	void GetActFreq(int*);
		
	int GetPattern(NGRAM*);
	int GetNumPatterns() {return num_patterns;}
	long PrintPatterns(void (*print_func)(NGRAM*));
		
	float Multiplicity() {return float(cur_map.GetNumSymbols())/msg_len;}

	void MergeSymbols(char,char,int);
	int Simplify(char*);
	long SeqHomo(wchar*,char*,float,int);
	void Flip(int,int);

	long LetterGraph(wchar*);
	long PolyKeySize(wchar*,int,float);
	long RowColIoC(wchar*,int);
	
	void PatternsToFile(const char*,int);
	
	void operator += (Message &src_msg)
	{
		//src message is longer, must reallocate
		if(src_msg.msg_len>msg_len)
		{
			if(cipher) delete[] cipher;
			if(plain) delete[] plain;
			
			cipher=new char[src_msg.msg_len+1];
			plain=new char[src_msg.msg_len+1];
		}

		//text
		msg_len=src_msg.msg_len;
		strcpy(cipher,src_msg.cipher);
		strcpy(plain,src_msg.plain);
		
		cur_map=src_msg.cur_map;
		memcpy(exp_freq,src_msg.exp_freq,26*sizeof(int));
	}
	
	void operator = (Message &src_msg)
	{
		*this+=src_msg;		
		FindPatterns(true);
	}

	Map cur_map;

private:
	void Decode();
	void SetInfo();
	void FindPatterns(int);
	int FindPattern(const char*,NGRAM*&,NGRAM*,NGRAM*);
	int FindPattern(const char*,NGRAM*&);
	int AddPattern(NGRAM&,int);
	long ForAllPatterns(NGRAM *,int,void (*do_func)(NGRAM*));
	void ClearPatterns(NGRAM*);
	long WritePatterns(NGRAM*,int);
	
	char *cipher, *plain;
	int msg_len, min_pat_len;
	int exp_freq[26];
	NGRAM *patterns;
	int num_patterns, good_pat;
	FILE *ngram_file;
};

#endif
