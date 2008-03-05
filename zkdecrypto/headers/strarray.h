#ifndef _STR_ARRAY_H_
#define _STR_ARRAY_H_

#include <string.h>

#define MAX_STRINGS 256

class StringArray
{
	public:
		StringArray() {num_strings=0; Clear();}
		~StringArray() {Clear();}

		int AddString(const char*);
		int GetString(int,char*);
		int Intersect(char*);
		int GetNumStrings() {return num_strings;}
		void Clear();
	
	private:
		char *strings[MAX_STRINGS];
		int num_strings;
};

#endif