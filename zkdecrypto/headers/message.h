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
#define SOLVE_PERMUTE	7
#define SOLVE_COLTRANS	8
#define SOLVE_ADFGX		9
#define SOLVE_ADFGVX	10
#define SOLVE_CEMOPRTU	11
#define SOLVE_KRYPTOS	50

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
	Message() {patterns=NULL; cipher=plain=msg_temp=NULL; msg_len=num_patterns=good_pat=0; trans_type=1; min_pat_len=2; polybius5[0]=polybius6[0]=polybius8[0]=trifid_array[0]='\0'; InitArrays(); strcpy(coltrans_key,"1");}
	~Message() {if(cipher) delete[] cipher; if(plain) delete[] plain; if(msg_temp) delete[] msg_temp; if(patterns) ClearPatterns(patterns);}

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
	int CalcBestWidth(int);
	
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
	void DecodeDigraphic();
	void DecodePlayfair();
	void DecodeVigenere();
	void DecodeXfid(int);
	void DecodePermutation(int do_homo=true);
	void DecodeColumnar();
	void DecodeADFGX(int);
	void SetTableuAlphabet(char*);
	char *GetTableuAlphabet() {return vigenere_array[0];}
	
	void Decode()
	{
		switch(decode_type)
		{

			case SOLVE_HOMO:	DecodeHomo(); break;
			case SOLVE_DISUB:	DecodeDigraphic(); break;
			case SOLVE_PLAYFAIR: DecodePlayfair(); break;
			case SOLVE_VIG:		DecodeVigenere(); break;
			case SOLVE_RUNKEY:	DecodeVigenere(); break;
			case SOLVE_BIFID:	DecodeXfid(2); break;
			case SOLVE_TRIFID:	DecodeXfid(3); break;
			case SOLVE_PERMUTE: DecodePermutation(); break;
			case SOLVE_COLTRANS: DecodeColumnar(); break;
			case SOLVE_ADFGX:	DecodeADFGX(5); break;
			case SOLVE_ADFGVX:	DecodeADFGX(6); break;
			case SOLVE_CEMOPRTU:	DecodeADFGX(8); break;
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
			if(msg_temp) delete[] msg_temp;
			
			cipher=new char[src_msg.msg_len+1];
			plain=new char[src_msg.msg_len+1];
			msg_temp=new char[src_msg.msg_len+1];
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
		strcpy(coltrans_key,src_msg.coltrans_key);
		memcpy(key,src_msg.key,key_len);
		memcpy(polybius5,src_msg.polybius5,sizeof(polybius5));
		memcpy(polybius6,src_msg.polybius6,sizeof(polybius6));
		memcpy(polybius8,src_msg.polybius8,sizeof(polybius8));
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

	void InitArrays()
	{
		if(strlen(polybius5)!=25) strcpy(polybius5,"ABCDEFGHIKLMNOPQRSTUVWXYZ");
		if(strlen(polybius6)!=36) strcpy(polybius6,"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
		if(strlen(polybius8)!=64) strcpy(polybius8,"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .");
		if(strlen(trifid_array)!=27) strcpy(trifid_array,"ABCDEFGHIJKLMNOPQRSTUVWXYZ.");
	}
	
	char coltrans_key[512];
	char polybius5[26];
	char polybius6[37];
	char polybius8[65];
	char trifid_array[28];
	char vigenere_array[26][27];

	int FindPolybius5Index(char,int&,int&);
	int FindPolybius6Index(char,int&,int&);
	int FindTrifidIndex(char,int&,int&,int&);
	void SetTransType(int new_type) {trans_type=new_type;}
	int GetTransType() {return trans_type;}

	void RotateString(char*,int,int);

	void SetSplitKey(char *split_key, int poly_size)
	{
		char *key_split=strchr(split_key,'|'); //| separates the polybius key from transposition key

		if(!key_split) 
			switch(poly_size)
			{
				case 5: memcpy(polybius5,split_key,25); break;
				case 6: memcpy(polybius6,split_key,36); break;
				case 8: memcpy(polybius8,split_key,64); break;
			}

		else 
		{
			int length=int(key_split-split_key);
			if(*(key_split+1)=='\0') strcat(split_key,"1"); 
			strcpy(coltrans_key,key_split+1);  //get transposition key
			
			switch(poly_size)
			{
				case 5: memcpy(polybius5,split_key,MIN(length,25)); break;
				case 6: memcpy(polybius6,split_key,MIN(length,36)); break;
				case 8: memcpy(polybius8,split_key,MIN(length,64)); break;
			}
		}  
	}

private:
	int FindPattern(const char*,NGRAM*&,NGRAM*,NGRAM*);
	int FindPattern(const char*,NGRAM*&);
	int AddPattern(NGRAM&,int);
	long ForAllPatterns(NGRAM *,int,void (*do_func)(NGRAM*));
	void ClearPatterns(NGRAM*);
	
	char *cipher, *plain, *msg_temp;
	int msg_len, min_pat_len;
	int exp_freq[26];
	NGRAM *patterns;
	int num_patterns, good_pat;
	
	//for different decoding
	int decode_type;
	char key[4096];
	int key_len;
	int block_size;
	int trans_type; //0=permutation, 1=columnar for ADFGX types
};

void SwapStringColumns(char*,int,int,int);
void SwapStringRows(char*,int,int,int);
#endif
