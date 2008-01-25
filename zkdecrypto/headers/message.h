#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "unicode.h"

#define MAX_SYM		256
#define MAX_PAT_LEN	15
#define MAX_GRA_ROW 35

#define CLR_CIPHER	0x01
#define CLR_PLAIN	0x02
#define CLR_FREQ	0x04
#define CLR_ALL		0x07

#define BLANK char(0x97)

#define ROUNDTOINT(F) (DECIMAL(F)>=.5? int(F)+1:int(F))
#define ROUNDUP(F) (DECIMAL(F)>0? int(F)+1:int(F))
#define IS_ASCII(C) (C>0x1F && C<0x7F)
#define DECIMAL(N) (N-int(N))
#define ABS(X) (X<0? (-1*X):X)
#define CLOSER(A,B,C) (ABS(A-C)<ABS(B-C)) //TRUE if A is closer to C than B is

#pragma warning( disable : 4996)  //STOP STUPID MSVS2005 "strcpy" WARNINGS

/*Symbol*/
struct SYMBOL
{
	char cipher;
	char plain;
	int freq;
};

class Map
{
public:
	Map() {num_symbols=0; Clear(CLR_ALL); SetAllLock(0);}

	int Read(const char*);
	int Write(const char*);
	void Clear(int);
	void Init(int);

	int AddSymbol(SYMBOL&,int);
	int GetSymbol(int,SYMBOL*);
	int FindByCipher(char);
	int GetNumSymbols() {return num_symbols;}

	void SortByFreq();
	void SetUnigraphs(double *ug) {memcpy(unigraphs,ug,26*sizeof(float));}
	float GetUnigraph(int letter) {return unigraphs[letter];}
	void SwapSymbols(int,int);
	void SymbolTable(char*);
	long SymbolGraph(wchar *dest);

	void MergeSymbols(char,char);
	
	void ToKey(char*,char*);
	void FromKey(char*);

	int GetLock(int index) {return locked[index];}
	void SetLock(int index, int lock) {locked[index]=lock;}
	void ToggleLock(int index) {locked[index]=!locked[index];}
	void SetAllLock(int lock) {memset(locked,lock,MAX_SYM);}
	const char* GetLocked() {return locked;}

	//set this map equal to another
	void operator = (Map src_map) 
	{
		num_symbols=src_map.num_symbols; 
		memcpy(symbols,src_map.symbols,num_symbols*sizeof(SYMBOL));
		memcpy(locked,src_map.locked,num_symbols);
	}

	//update symbols in this map to symbols in another
	void operator += (Map src_map)
	{
		for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		{
			//see if current symbol is in the copy map
			int symbol=src_map.FindByCipher(symbols[cur_symbol].cipher);

			//if it is, set it the copy map's symbol
			if(symbol!=-1) AddSymbol(src_map.symbols[symbol],0);
		}
	}
	
private:	
	SYMBOL symbols[MAX_SYM];
	char locked[MAX_SYM];
	int num_symbols;
	float unigraphs[26];
};

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
	Message() {msg_len=0; patterns=NULL; num_patterns=0; good_pat=0; cipher=NULL; plain=NULL;}
	~Message() {if(cipher) delete[] cipher; if(plain) delete[] plain; if(patterns) ClearPatterns(patterns);}

	int Read(const char*);
	void SetCipher(const char*);
	
	const char * GetCipher() {return cipher;}
	const char * GetPlain() {Decode(); return plain;}
	int GetLength() {return msg_len;}
	int GetRow(int,int,char*);
	int GetColumn(int,int,char*);
	
	void GetExpFreq(int*);
	void GetActFreq(int*);
		
	int GetPattern(NGRAM*);
	int GetNumPatterns() {return num_patterns;}
	long PrintPatterns(void (*print_func)(NGRAM*));
		
	float Multiplicity() {return float(cur_map.GetNumSymbols())/msg_len;}

	void MergeSymbols(char,char,int);
	int Simplify(char&,char&);
	
	long LetterGraph(wchar*);
	
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
	long Message::WritePatterns(NGRAM*,int);
	
	char *cipher, *plain;
	int msg_len;
	int exp_freq[26];
	NGRAM *patterns;
	int num_patterns, good_pat;
	FILE *ngram_file;
};

void to_unicode(char*);

#endif
