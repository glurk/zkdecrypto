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
#pragma warning( disable : 4267)

#define SOLVE_HOMO		0
#define SOLVE_DISUB		40
#define SOLVE_PLAYFAIR	1
#define SOLVE_VIG		2
#define SOLVE_DICTVIG	3
#define SOLVE_RUNKEY	4
#define SOLVE_BIFID		5
#define SOLVE_TRIFID	6
#define SOLVE_PERMUTE	7
#define SOLVE_COLTRANS	8
#define SOLVE_DOUBLE	9
#define SOLVE_TRIPPLE	10
#define SOLVE_ADFGX		11
#define SOLVE_ADFGVX	12
#define SOLVE_CEMOPRTU	13
#define SOLVE_SUBPERM	14
#define SOLVE_SUBCOL	15
#define SOLVE_COLVIG	16

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
	Message() 
	{
		patterns=NULL; cipher=plain=msg_temp=NULL; 
		msg_len=num_patterns=good_pat=0; 
		trans_type=0; 
		min_pat_len=2; 
		memset(coltrans_key,0,sizeof(coltrans_key));
		polybius5[0]=polybius6[0]=polybius8[0]=trifid_array[0]='\0'; 
		vig_key_len=0;
		coltrans_key[0][0]=coltrans_key[1][0]=coltrans_key[2][0]='\0';
		InitKeys();  
		
		memset(POLYBIUS_INDEXS,-1,256); //adfgvx decoding
		POLYBIUS_INDEXS['C']=POLYBIUS_INDEXS['A']=0;
		POLYBIUS_INDEXS['E']=POLYBIUS_INDEXS['D']=1;
		POLYBIUS_INDEXS['M']=POLYBIUS_INDEXS['F']=2;
		POLYBIUS_INDEXS['O']=POLYBIUS_INDEXS['G']=3;
		POLYBIUS_INDEXS['P']=POLYBIUS_INDEXS['V']=4;
		POLYBIUS_INDEXS['R']=5;
		POLYBIUS_INDEXS['T']=6;
		POLYBIUS_INDEXS['U']=7;
	}
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

	void SetInfo(int set_maps=false);
	void FindPatterns(int);

	void SetKeyLength(int new_key_len) {vig_key_len=new_key_len; key[vig_key_len]='\0';}
	int GetKeyLength() {return vig_key_len;}
	void SetBlockSize(int new_block_size) {block_size=new_block_size;}
	int GetBlockSize() {return block_size;} 

	//decoding
	void DecodeHomo();
	void DecodeDigraphic();
	void DecodePlayfair();
	void DecodeVigenere(char*);
	void DecodeXfid(int);
	void DecodePermutation(char*);
	void ColumnarStage(char*);
	void DecodeColumnar(int);
	void DecodeADFGX(int,char*);
	void SetTableuAlphabet(char*);
	char *GetTableuAlphabet() {return vigenere_array[0];}
	
	void Decode()
	{
		switch(decode_type)
		{

			case SOLVE_HOMO:	DecodeHomo(); break;
			case SOLVE_DISUB:	DecodeDigraphic(); break;
			case SOLVE_PLAYFAIR:DecodePlayfair(); break;
			case SOLVE_VIG:		DecodeVigenere(cipher); break;
			case SOLVE_DICTVIG:	DecodeVigenere(cipher); break;
			case SOLVE_RUNKEY:	DecodeVigenere(cipher); break;
			case SOLVE_BIFID:	DecodeXfid(2); break;
			case SOLVE_TRIFID:	DecodeXfid(3); break;
			case SOLVE_PERMUTE: DecodeHomo(); DecodePermutation(coltrans_key[0]); break;
			case SOLVE_COLTRANS:DecodeHomo(); DecodeColumnar(1); break;
			case SOLVE_DOUBLE:  DecodeHomo(); DecodeColumnar(2); break;
			case SOLVE_TRIPPLE:	DecodeHomo(); DecodeColumnar(3); break;
			case SOLVE_ADFGX:	DecodeHomo(); DecodeColumnar(1); DecodeADFGX(5,polybius5); break;
			case SOLVE_ADFGVX:	DecodeHomo(); DecodeColumnar(1); DecodeADFGX(6,polybius6); break;
			case SOLVE_CEMOPRTU:DecodeHomo(); DecodeColumnar(1); DecodeADFGX(8,polybius8); break;
			case SOLVE_SUBPERM: DecodeHomo(); DecodePermutation(coltrans_key[0]); break; 
			case SOLVE_SUBCOL:	DecodeHomo(); DecodeColumnar(2); break;
			case SOLVE_COLVIG:	DecodeHomo(); DecodeColumnar(2); DecodeVigenere(plain); break;
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
			msg_temp=new char[(src_msg.msg_len<<1)+1];
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
		vig_key_len=src_msg.vig_key_len;
		block_size=src_msg.block_size;
		trans_type=src_msg.trans_type;
		strcpy(coltrans_key[0],src_msg.coltrans_key[0]);
		strcpy(coltrans_key[1],src_msg.coltrans_key[1]);
		strcpy(coltrans_key[2],src_msg.coltrans_key[2]);
		memcpy(key,src_msg.key,vig_key_len);
		strcpy(polybius5,src_msg.polybius5);
		strcpy(polybius6,src_msg.polybius6);
		strcpy(polybius8,src_msg.polybius8);
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

	void InitKeys()
	{
		if(!vig_key_len || !strlen(key)) {strcpy(key,"ABCDE"); vig_key_len=5;}		
		if(strlen(polybius5)!=25) strcpy(polybius5,"ABCDEFGHIKLMNOPQRSTUVWXYZ");
		if(strlen(polybius6)!=36) strcpy(polybius6,"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
		if(strlen(polybius8)!=64) strcpy(polybius8,"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .");
		if(strlen(trifid_array)!=27) strcpy(trifid_array,"ABCDEFGHIJKLMNOPQRSTUVWXYZ.");

		if(!strlen(coltrans_key[0])) coltrans_key[0][0]='1';
		if(!strlen(coltrans_key[1])) coltrans_key[1][0]='1';
		if(!strlen(coltrans_key[2])) coltrans_key[2][0]='1';
	}
	
	char key[4096];
	char coltrans_key[10][512];
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

	void SetPolybius(int poly_size, const char *new_key, int new_key_len) 
	{
		char *polybius;

		switch(poly_size)
		{
			case 5: polybius=polybius5; break;
			case 6: polybius=polybius6; break;
			case 8: polybius=polybius8; break;
			default: return;
		}

		int length=MIN(new_key_len,poly_size*poly_size);

		memcpy(polybius,new_key,length); 
		
		polybius[length]=0; 
	}

	inline void SetTransKey(int key_num, const char *new_key, int new_key_len) {memcpy(coltrans_key[key_num],new_key,new_key_len); coltrans_key[key_num][new_key_len]='\0';}
	void SetKey(const char *split_key)
	{	
		int cur_key,key_start;

		for(cur_key=0,key_start=0; cur_key<10; cur_key++, key_start++)
		{
			int key_length=ChrIndex(split_key+key_start,'|');
			if(key_length==-1) key_length=strlen(split_key+key_start); //last key
			const char *key_ptr=split_key+key_start;

			switch(decode_type)
			{
				/*case SOLVE_HOMO:	cur_map.FromKey(key_ptr); break;
				case SOLVE_DISUB:	digraph_map.FromKey(key_ptr); break;*/
				case SOLVE_PLAYFAIR:SetPolybius(5,key_ptr,key_length); break;
				case SOLVE_VIG:		
				case SOLVE_RUNKEY:	memcpy(key,key_ptr,vig_key_len); strupr(key); break;
				case SOLVE_DICTVIG: SetKeyLength(strlen(key_ptr)); memcpy(key,key_ptr,vig_key_len); strupr(key); break;
				case SOLVE_BIFID:	SetPolybius(5,key_ptr,key_length); break;
				case SOLVE_TRIFID:	memcpy(trifid_array,key_ptr,MIN(key_length,27)); trifid_array[MIN(key_length,27)]=0; break;
				case SOLVE_PERMUTE:	
				case SOLVE_COLTRANS: 	
				case SOLVE_DOUBLE:	
				case SOLVE_TRIPPLE:	SetTransKey(cur_key,key_ptr,key_length); break;
				case SOLVE_ADFGX:	if(cur_key==0) SetPolybius(5,key_ptr,key_length); else SetTransKey(cur_key-1,key_ptr,key_length); break;
				case SOLVE_ADFGVX:	if(cur_key==0) SetPolybius(6,key_ptr,key_length); else SetTransKey(cur_key-1,key_ptr,key_length); break;
				case SOLVE_CEMOPRTU:if(cur_key==0) SetPolybius(8,key_ptr,key_length); else SetTransKey(cur_key-1,key_ptr,key_length); break;
				case SOLVE_SUBPERM:
				case SOLVE_SUBCOL:
					if(cur_key==0) cur_map.FromKey(key_ptr); 
					else SetTransKey(cur_key-1,key_ptr,key_length);
					break;
				case SOLVE_COLVIG:
					if(cur_key==0) {memcpy(key,key_ptr,vig_key_len); strupr(key);}
					else SetTransKey(cur_key-1,key_ptr,key_length);
					break;
			}

			key_start+=key_length;

			if(!split_key[key_start]) break;
		}
	}

	void GetKey(char *string, char *extra)
	{
		switch(decode_type) //set key text & max length
		{
			case SOLVE_HOMO: cur_map.ToKey(string,extra); break;
			case SOLVE_DISUB: digraph_map.ToKey(string,extra); break;
			case SOLVE_VIG: strcpy(string,key); strcat(string,extra); break;
			case SOLVE_DICTVIG: strcpy(string,key); strcat(string,extra); break;
			case SOLVE_RUNKEY: strcpy(string,key); strcat(string,extra); break;
			case SOLVE_PLAYFAIR: 
			case SOLVE_BIFID: strcpy(string,polybius5); break;
			case SOLVE_TRIFID: strcpy(string,trifid_array); break;
			case SOLVE_PERMUTE:
			case SOLVE_COLTRANS: strcpy(string,coltrans_key[0]); break;
			case SOLVE_DOUBLE: sprintf(string,"%s|%s",coltrans_key[0],coltrans_key[1]); break;
			case SOLVE_TRIPPLE: sprintf(string,"%s|%s|%s",coltrans_key[0],coltrans_key[1],coltrans_key[2]); break;
			case SOLVE_ADFGX: sprintf(string,"%s|%s",polybius5,coltrans_key[0]); break;
			case SOLVE_ADFGVX: sprintf(string,"%s|%s",polybius6,coltrans_key[0]);  break;
			case SOLVE_CEMOPRTU: sprintf(string,"%s|%s",polybius8,coltrans_key[0]);  break;
			case SOLVE_SUBPERM: cur_map.ToKey(string,extra); strcat(string,"|"); strcat(string,coltrans_key[0]); break;
			case SOLVE_SUBCOL: cur_map.ToKey(string,extra); strcat(string,"|"); strcat(string,coltrans_key[0]); strcat(string,"|"); strcat(string,coltrans_key[1]); break;
			case SOLVE_COLVIG: sprintf(string,"%s%s|%s|%s",key,extra,coltrans_key[0],coltrans_key[1]); break;
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
	int vig_key_len;
	int block_size;
	int trans_type; //reading direction of columnar transposition
	char POLYBIUS_INDEXS[256];
};

void SwapStringColumns(char*,int,int,int);
void SwapStringRows(char*,int,int,int);
#endif
