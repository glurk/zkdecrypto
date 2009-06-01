#ifndef _STR_ARRAY_H_
#define _STR_ARRAY_H_

#include <string.h>
#include <math.h>
#include <string>

#define MAX_STRINGS 2048
#define LOG2 .693147

#define NUM_ROWS(C,R) (C%R? (C/R)+1:(C/R))

class StringArray
{
	public:
		StringArray() {num_strings=0; Clear();}
		~StringArray() {Clear();}

		int AddString(const char*);
		int DeleteString(int);
		int GetString(int,char*);
		int SortString(int);
		void SortStrings(int);
		int RemoveDups();
		int Intersect(char*,float);
		int GetNumStrings() {return num_strings;}
		void Clear();
	
	private:
		char *strings[MAX_STRINGS];
		int num_strings;
};

int ChrIndex(const char*,char);
int RadixSort(char*);
int GetUniques(const char*, char*, int*);
float IoC(const char*,int);
float DIoC(const char*,int,int);
float Entropy(const char*,int);
float ChiSquare(const char*,int);
float avg_lsoc(const char*,int);
void Transform(char*,unsigned long*,int);
void Reverse(char*);
void FlipHorz(unsigned long*,int&,int,int);
void FlipVert(unsigned long*,int&,int,int);

#endif

