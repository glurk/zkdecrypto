#ifndef _MAP_H_
#define _MAP_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "unicode.h"
#include "macros.h"

#define MAX_SYM		256

#define CLR_CIPHER	0x01
#define CLR_PLAIN	0x02
#define CLR_FREQ	0x04
#define CLR_EXCLUDE	0x08
#define CLR_ALL		0x0F

#define BLANK char(0x97)

#define MAX_GRA_ROW 35

#pragma warning( disable : 4996)  //STOP MSVS2005 WARNINGS

struct SYMBOL
{
	char cipher; //char cipher2;
	char plain; //char plain2;
	int freq;
	char exclude[27];
};

class Map
{
public:
	Map() {num_symbols=0; Clear(CLR_ALL); memset(locked,0,MAX_SYM);}

	int Read(const char*);
	int Write(const char*);
	void Clear(int);
	void Init(int*);
	void AsCipher();

	int AddSymbol(SYMBOL&,int);
	int GetSymbol(int,SYMBOL*);
	int FindByCipher(char);
	int GetNumSymbols() {return num_symbols;}

	void SortByFreq();
	void SetUnigraphs(double *ug) {memcpy(unigraphs,ug,26*sizeof(float));}
	float GetUnigraph(int letter) {return unigraphs[letter];}
	void SwapSymbols(int,int);
	void SymbolTable(char*);
	long SymbolGraph(wchar*);
	long GetMergeLog(wchar*);
	long GetExclusions(wchar*,int);

	void MergeSymbols(char,char);
	
	void ToKey(char*,char*);
	void FromKey(char*);

	int GetLock(int index) {return locked[index];}
	void SetLock(int index, int lock) {locked[index]=lock;}
	void ToggleLock(int index) {locked[index]=!locked[index];}
	void SetAllLock(int lock) {memset(locked,lock,num_symbols);}
	const char* GetLocked() {return locked;}	

	//set this map equal to another
	void operator = (Map src_map) 
	{
		num_symbols=src_map.num_symbols; 
		memcpy(symbols,src_map.symbols,num_symbols*sizeof(SYMBOL));
		memcpy(locked,src_map.locked,num_symbols);
		strcpy(merge_log,src_map.merge_log);
	}

	//update symbols in this map to symbols in another
	void operator += (Map src_map)
	{
		for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		{
			//see if current symbol is in the copy map
			int symbol=src_map.FindByCipher(symbols[cur_symbol].cipher);

			//if it is, set it the copy map's symbol
			if(symbol!=-1) 
			{
				AddSymbol(src_map.symbols[symbol],0);
				locked[symbol]=src_map.locked[symbol];
			}
		}
	}
	
private:	
	SYMBOL symbols[MAX_SYM];
	//SYMBOL digraphs[MAX_SYM*MAX_SYM];
	char locked[MAX_SYM], merge_log[512];
	int num_symbols;
	float unigraphs[26];
};

#endif
