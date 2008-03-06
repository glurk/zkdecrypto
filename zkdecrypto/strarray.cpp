#pragma warning( disable : 4244)	// STOP MSVS2005 WARNINGS

#include "headers/strarray.h"

int StringArray::AddString(const char *new_string)
{
	strings[num_strings]=new char[strlen(new_string)+1];
	strcpy(strings[num_strings],new_string);
	return ++num_strings;
}

int StringArray::GetString(int string, char *dest_string)
{
	strcpy(dest_string,strings[string]);
	return strlen(dest_string);
}

int StringArray::Intersect(char *sect_string)
{
	char find_char;
	int str_len, num_sect;
	int not_found;

	sect_string[0]='\0';
	num_sect=0;

	//for all strings, find chars that are in all other strings
	for(int string_a=0; string_a<num_strings; string_a++)
	{
		//for each character in this string
		str_len=strlen(strings[string_a]);

		for(int cur_char=0; cur_char<str_len; cur_char++)
		{
			//check all other strings for this char
			find_char=strings[string_a][cur_char];
			not_found=false;

			for(int string_b=0; string_b<num_strings; string_b++)
				if(!strchr(strings[string_b],find_char)) not_found=true;

			//if it is in all strings, add to intersect string
			if(!not_found)
				if(!strchr(sect_string,find_char))
				{
					sect_string[num_sect]=find_char;
					sect_string[++num_sect]='\0';
				}
		}
	}

	return num_sect;	
}

void StringArray::Clear()
{
	for(int cur_string=0; cur_string<num_strings; cur_string++)
		if(strings[cur_string]) delete[] strings[cur_string];
	
	memset(strings,0,MAX_STRINGS*sizeof(char*));
	num_strings=0;
}
