#ifndef _STR_ARRAY_H_
#define _STR_ARRAY_H_

#include <string.h>

#define MAX_STRINGS 1024

class StringArray
{
	public:
		StringArray() {num_strings=0; Clear();}
		~StringArray() {Clear();}

		int AddString(const char*);
		int DeleteString(int);
		int GetString(int,char*);
		int SortString(int);
		int SortStrings(int);
		int RemoveDups();
		int Intersect(char*,float);
		int GetNumStrings() {return num_strings;}
		void Clear();
	
	private:
		char *strings[MAX_STRINGS];
		int num_strings;
};

#endif

