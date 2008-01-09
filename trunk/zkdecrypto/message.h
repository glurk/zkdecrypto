#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MAX_SYM		256
#define MAX_PAT 	1024
#define MAX_PAT_LEN	16

#define CLR_CIPHER	0x01
#define CLR_PLAIN	0x02
#define CLR_FREQ	0x04
#define CLR_ALL		0x07

#define BLANK char(0x97)

#define ROUNDTOINT(F) (F-int(F)>=.5? int(F)+1:int(F))
#define IS_ASCII(C) (C>0x1F && C<0x7F)
#define DECIMAL(N) (N-int(N))

/*Symbol*/
struct SYMBOL
{
	char cipher;
	char plain;
	int freq;
	char locked;
};

class Map
{
public:
	Map() {num_symbols=0; Clear(CLR_ALL); SetAllLock(0);}

	int Read(const char*);
	int Write(const char*);
	void Clear(int);
	void Init();

	int AddSymbol(SYMBOL,int);
	int GetSymbol(int,SYMBOL*);
	int FindByCipher(char);
	int GetNumSymbols() {return num_symbols;}

	void SortByFreq();
	void SetUnigraphs(double *ug) {memcpy(unigraphs,ug,26*sizeof(float));}
	float GetUnigraph(int letter) {return unigraphs[letter];}
	void SwapSymbols(int,int);
	void SymbolTable(char*);
	
	void ToKey(char*);
	void FromKey(char*);

	int GetLock(int index) {return symbols[index].locked;}
	void SetLock(int index, int lock) {symbols[index].locked=lock;}
	void ToggleLock(int index) {symbols[index].locked=!symbols[index].locked;}
	void SetAllLock(int lock) {for(int sym=0; sym<num_symbols; sym++) SetLock(sym,lock);}
	void GetLocked(char *locked) {for(int sym=0; sym<num_symbols; sym++) locked[sym]=symbols[sym].locked;}

	//set this map equal to another
	void operator = (Map src_map) 
	{
		num_symbols=src_map.num_symbols; 
		memcpy(symbols,src_map.symbols,num_symbols*sizeof(SYMBOL));
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
	int num_symbols;
	float unigraphs[26];
};

struct NGRAM
{
	char string[MAX_PAT_LEN];
	int length;
	int freq;
	int positions[128];
};

/*Message*/
class Message
{
public:
	Message() {msg_len=0; num_patterns=0; cipher=NULL; plain=NULL;}
	~Message() {if(cipher) delete[] cipher; if(plain) delete[] plain;}

	int Read(const char*);
	
	const char * GetCipher() {return cipher;}
	const char * GetPlain() {Decode(); return plain;}
	int GetLength() {return msg_len;}
	
	void GetExpFreq(int*);
	void GetActFreq(int*);
		
	int GetPattern(int,NGRAM*);
	int GetNumPatterns() {return num_patterns;}
		
	float GetStrength() {return float(cur_map.GetNumSymbols() / ((log(msg_len)*(num_patterns+1)/10.0)));}	
	
	void MergeSymbols(char,char);
	
	void operator = (Message &src_msg)
	{
		if(cipher) delete[] cipher;
		if(plain) delete[] plain;

		//text
		msg_len=src_msg.msg_len;
		cipher=new char[msg_len+1];
		plain=new char[msg_len+1];
		strcpy(cipher,src_msg.cipher);
		strcpy(plain,src_msg.plain);

		memcpy(exp_freq,src_msg.exp_freq,26*sizeof(int));
		memcpy(act_freq,src_msg.act_freq,26*sizeof(int));
		
		num_patterns=src_msg.num_patterns;
		memcpy(patterns,src_msg.patterns,num_patterns*sizeof(NGRAM));

		cur_map=src_msg.cur_map;
	}

	Map cur_map;

private:
	void Decode();
	void SetInfo();
	void FindPatterns();
	int PatternMatch(const char*,const char*,char*);
	int AddPattern(NGRAM*,int);
	
	char *cipher, *plain;
	int msg_len;
	int exp_freq[26], act_freq[26];
	NGRAM patterns[MAX_PAT];
	int num_patterns;
};

#endif
