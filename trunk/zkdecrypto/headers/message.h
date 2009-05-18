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

#pragma warning( disable : 4996)  //STOP MSVS2005 WARNINGS

#define SOLVE_HOMO		0
#define SOLVE_DISUB		1
#define SOLVE_PLAYFAIR	2
#define SOLVE_VIG		3
#define SOLVE_RUNKEY	4
#define SOLVE_BIFID		5
#define SOLVE_TRIFID	6
#define SOLVE_COLTRANS	7
#define SOLVE_ANAGRAM	8
#define SOLVE_KRYPTOS	9

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
	Message() {msg_len=0; patterns=NULL; num_patterns=0; good_pat=0; min_pat_len=2; cipher=NULL; plain=NULL; bifid_array[0]=trifid_array[0]='\0'; InitArrays(); }
	~Message() {if(cipher) delete[] cipher; if(plain) delete[] plain; if(patterns) ClearPatterns(patterns);}

	int Read(const char*);
	int ReadNumeric(const char*);
	int Write(const char*);
	void SetCipher(const char*);
	void SetCipherTrans(char *cipher_trans) {strcpy(cipher,cipher_trans);}
	void SetPlain(char * new_plain) {strcpy(plain,new_plain);}

	void Insert(int,const char*);
	
	char * GetCipher() {return cipher;}
	char * GetPlain() {Decode(); return plain;}

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
	int Rotate(int,int);
	void SwapColumns(int,int,int);
	void SwapRows(int,int,int);
	void DecodeElgar();

	long LetterGraph(wchar*);
	long PolyKeySize(wchar*,int,float);
	long RowColIoC(wchar*,int);

	void SetInfo();
	void FindPatterns(int);

	void SetKey(char *new_key) {memcpy(key,new_key,key_len); key[key_len]='\0';}
	char *GetKey() {return key;}
	void SetKeyLength(int new_key_len) {key_len=new_key_len;}
	int GetKeyLength() {return key_len;}
	void SetBlockSize(int new_block_size) {block_size=new_block_size;}
	int GetBlockSize() {return block_size;} 

	//decoding
	void DecodeHomo();
	void DecodeVigenere();
	void DecodeXfid(int);
	void DecodeDigraphic();
	void DecodePlayfair();
	void SetTableuAlphabet(char*);
	char *GetTableuAlphabet() {return vigenere_array[0];}
	
	void Decode()
	{
		switch(decode_type)
		{

			case SOLVE_HOMO:	DecodeHomo(); break;
			case SOLVE_VIG:		DecodeVigenere(); break;
			case SOLVE_BIFID:	DecodeXfid(2); break;
			case SOLVE_TRIFID:	DecodeXfid(3); break;
			case SOLVE_ANAGRAM: DecodeHomo(); break;
			case SOLVE_COLTRANS: DecodeHomo(); break;
			case SOLVE_RUNKEY:	DecodeVigenere(); break;
			case SOLVE_DISUB:	DecodeDigraphic(); break;
			case SOLVE_PLAYFAIR: DecodePlayfair(); break;
		}
	}

	void SetDecodeType(int new_type) {decode_type=new_type;}
	int GetDecodeType() {return decode_type;}
	
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
		digraph_map=src_msg.digraph_map;
		memcpy(exp_freq,src_msg.exp_freq,26*sizeof(int));

		//decoding data
		decode_type=src_msg.decode_type;
		key_len=src_msg.key_len;
		block_size=src_msg.block_size;
		memcpy(key,src_msg.key,key_len);
		memcpy(bifid_array,src_msg.bifid_array,sizeof(bifid_array));
		memcpy(trifid_array,src_msg.trifid_array,sizeof(trifid_array));
		memcpy(vigenere_array,src_msg.vigenere_array,sizeof(vigenere_array));
	}
	
	void operator = (Message &src_msg)
	{
		*this+=src_msg;		
		FindPatterns(true);
	}

	//decoding
	Map cur_map;
	DiMap digraph_map;
	
	char bifid_array[26];
	char trifid_array[28];
	char vigenere_array[26][27];

	void InitArrays();
	int FindBifidIndex(char,int&,int&);
	int FindTrifidIndex(char,int&,int&,int&);

private:
	int FindPattern(const char*,NGRAM*&,NGRAM*,NGRAM*);
	int FindPattern(const char*,NGRAM*&);
	int AddPattern(NGRAM&,int);
	long ForAllPatterns(NGRAM *,int,void (*do_func)(NGRAM*));
	void ClearPatterns(NGRAM*);
	
	char *cipher, *plain;
	int msg_len, min_pat_len;
	int exp_freq[26];
	NGRAM *patterns;
	int num_patterns, good_pat;
	
	//for different decoding
	int decode_type;
	char key[4096];
	int key_len;
	int block_size;
};

void SwapStringColumns(char*,int,int,int);
void SwapStringRows(char*,int,int,int);
#endif
