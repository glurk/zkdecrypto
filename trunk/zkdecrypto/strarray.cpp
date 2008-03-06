#pragma warning( disable : 4996)
#pragma warning( disable : 4244)	// STOP MSVS2005 WARNINGS

#include "headers/strarray.h"

int StringArray::AddString(const char *new_string)
{
	strings[num_strings]=new char[strlen(new_string)+1];
	strcpy(strings[num_strings],new_string);
	return ++num_strings;
}

int StringArray::DeleteString(int string)
{
	if(string<0 || string>num_strings) return 0;
	
	if(strings[string]) delete strings[string];
	
	memmove(&strings[string],&strings[string+1],(num_strings-string-1)*sizeof(char*));
	num_strings--;
}

int StringArray::GetString(int string, char *dest_string)
{
	if(string<0 || string>num_strings) return 0;
	
	strcpy(dest_string,strings[string]);
	return (int)strlen(dest_string);
}

int StringArray::SortString(int string)
{
	int str_len=strlen(strings[string]);
	char temp, swap;
	
	if(string<0 || string>num_strings) return 0;
	
	do
	{
		swap=false;
		
		for(int index_a=0; index_a<str_len-1; index_a++)
			for(int index_b=index_a+1; index_b<str_len; index_b++)
				if(strings[string][index_b]<strings[string][index_a])
				{
					temp=strings[string][index_b];
					strings[string][index_b]=strings[string][index_a];
					strings[string][index_a]=temp;
					swap=true;
				}
	} while(swap);
}

int StringArray::SortStrings(int order)
{
	char *temp, swap;
	int cmp;
	
	do
	{
		swap=false;
		
		for(int index_a=0; index_a<num_strings-1; index_a++)
			for(int index_b=index_a+1; index_b<num_strings; index_b++)
			{
				cmp=strcmp(strings[index_b],strings[index_a]);
				
				if((cmp<0 && !order) || (cmp>0 && order))
				{
					temp=strings[index_b];
					strings[index_b]=strings[index_a];
					strings[index_a]=temp;
					swap=true;
				}
			}
	} while(swap);
}

int StringArray::RemoveDups()
{
	for(int index_a=0; index_a<num_strings-1; index_a++)
		for(int index_b=index_a+1; index_b<num_strings; index_b++)
			if(!strcmp(strings[index_b],strings[index_a]))
				DeleteString(index_b--);
				
	return num_strings;
}

int StringArray::Intersect(char *sect_string, float match)
{
	char find_char;
	int str_len, num_sect;
	float found;

	sect_string[0]='\0';
	num_sect=0;

	//for all strings, find chars that are in all other strings
	for(int string_a=0; string_a<num_strings; string_a++)
	{
		//for each character in this string
		str_len=(int)strlen(strings[string_a]);

		for(int cur_char=0; cur_char<str_len; cur_char++)
		{
			//check all other strings for this char
			find_char=strings[string_a][cur_char];
			found=0;

			for(int string_b=0; string_b<num_strings; string_b++)
				if(strchr(strings[string_b],find_char)) found++;

			//if it is in all strings, add to intersect string
			found/=num_strings;
			if(found>=match)
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
