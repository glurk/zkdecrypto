#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MAX_SYM		256
#define MAX_PAT 	1024
#define MAX_PAT_LEN	15

#define CLR_CIPHER	0x01
#define CLR_PLAIN	0x02
#define CLR_FREQ	0x04
#define CLR_ALL		0x07

#define ROUNDTOINT(F) (F-int(F)>=.5? int(F)+1:int(F))
//#define ROUNDUP(F) (int(F)+1)
#define IS_ASCII(C) ((C>0x1F && C<0x7F)? true:false)

const double unigraphs[]=
{
8.167, 1.492, 2.782, 4.253, 12.702, 2.228, 2.015, 6.094, 6.966, 0.153, 0.772, 4.025, 2.406,
6.749, 7.507, 1.929, 0.095, 5.987, 6.327, 9.056, 2.758, 0.978, 2.360, 0.150, 1.974, 0.0749
};

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
	Map() {Clear(CLR_ALL); SetAllLock(0);}

	void Clear(int);
	void Init();
	int Read(const char*);
	int Write(const char*);

	int AddSymbol(SYMBOL,int);
	int GetSymbol(int,SYMBOL*);
	int GetNumSymbols() {return num_symbols;}

	void SwapSymbols(int,int);
	void SortByFreq();
	void SymbolTable(char*);

	int GetLock(int index) {return locked[index];}
	void SetLock(int index, int lock) {locked[index]=lock;}
	void ToggleLock(int index) {locked[index]=!locked[index];}
	void SetAllLock(int lock) {memset(locked,lock,MAX_SYM);}
	const char *GetLocked() {return locked;}

	void operator = (Map src_map) 
	{
		num_symbols=src_map.num_symbols; 
		memcpy(symbols,src_map.symbols,num_symbols*sizeof(SYMBOL));
	}
	
private:
	SYMBOL symbols[MAX_SYM];
	char locked[MAX_SYM];
	int num_symbols;
	int swap1, swap2;
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
	Message() {msg_len=0; cipher=NULL; plain=NULL;}
	~Message() {if(cipher) delete[] cipher; if(plain) delete[] plain;}

	int Read(const char*);
	const char * GetCipher() {return cipher;}
	void Decode();
	const char * GetPlain() {Decode(); return plain;}
	int GetLength() {return msg_len;}
	void GetExpFreq(int*);
	void GetActFreq(int*);
	void FindPatterns();
	int AddPattern(NGRAM*,int);
	int GetNumPatterns() {return num_patterns;}
	int GetPattern(int,NGRAM*);
	float GetStrength() {return float(cur_map.GetNumSymbols() / ((log(msg_len)*(num_patterns+1)/10.0)));}
	void CipherInfo();

	Map cur_map;

private:
	char *cipher, *plain;
	int msg_len;
	int exp_freq[26], act_freq[26];
	NGRAM patterns[MAX_PAT];
	int num_patterns;
};

#endif
