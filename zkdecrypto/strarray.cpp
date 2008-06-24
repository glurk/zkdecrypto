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
	if(string<0 || string>=num_strings) return 0;
	
	if(strings[string]) delete strings[string];
	
	memmove(&strings[string],&strings[string+1],(num_strings-string-1)*sizeof(char*));
	num_strings--;

	return num_strings;
}

int StringArray::GetString(int string, char *dest_string)
{
	if(string<0 || string>=num_strings) return 0;
	
	strcpy(dest_string,strings[string]);
	return (int)strlen(dest_string);
}

int StringArray::SortString(int string)
{
	int str_len=(int)strlen(strings[string]);
	char temp, swap;
	
	if(string<0 || string>=num_strings) return 0;
	
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

	return 1;
}

void StringArray::SortStrings(int order)
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


//count frequencies of chars in a string
int GetUniques(const char *string, char *unique_str, int *unique_freq)
{
	int length, index, ascii_index[256], unique=0;
	
	length=(int)strlen(string);
	
	memset(ascii_index,0,256*sizeof(int));
	
	for(index=0; index<length; index++)
		ascii_index[(unsigned char)string[index]]++;

	for(index=0; index<256; index++)
		if(ascii_index[index])
		{
			if(unique_str) unique_str[unique]=index;
			if(unique_freq) unique_freq[unique]=ascii_index[index];
			unique++;
		}
		
	if(unique_str) unique_str[unique]='\0';

	return unique;
}

//index of coincidence of a string
float IoC(const char *string)
{
	int freqs[256], length, unique;
	float ic=0;

	if(!string) return 0;

	length=(int)strlen(string);

	if(length<2) return 0;	

	unique=GetUniques(string,NULL,freqs);

	//calculate index of coincidence
	for(int sym_index=0; sym_index<unique; sym_index++)
		if(freqs[sym_index]>1) 
			ic+=(freqs[sym_index])*(freqs[sym_index]-1); 

	ic/=(length)*(length-1);

	return ic;
}

float Entropy(const char *string)
{
	int freqs[256], length, unique;
	float entropy=0, prob_mass, log2;

	if(!string) return 0;

	length=(int)strlen(string);
	
	if(length<0) return 0;

	unique=GetUniques(string,NULL,freqs);

	//for log base conversion
	log2=log((float)2);

	//calculate entropy
	for(int sym_index=0; sym_index<unique; sym_index++)
	{
		prob_mass=float(freqs[sym_index])/length;
		entropy+=prob_mass*(log(prob_mass)/log2);
	}

	return (-1*entropy);
}

float ChiSquare(const char *string)
{
	int freqs[256], length, unique;
	float chi2=0, prob_mass, cur_calc;

	if(!string) return 0;

	length=(int)strlen(string);
	
	if(length<0) return 0;

	unique=GetUniques(string,NULL,freqs);

	//calculate chi2
	for(int sym_index=0; sym_index<unique; sym_index++)
	{
		prob_mass=length*(1.0/unique);
		cur_calc=freqs[sym_index]-prob_mass;
		cur_calc*=cur_calc;
		cur_calc/=prob_mass;

		chi2+=cur_calc;
	}

	return chi2/length;
}

void Reverse(char *string)
{
	unsigned char temp;
	int i, j, size;
	size = (int)strlen(string);
	if(size <= 1) return;
	else
	{
		for(i = 0, j = size-1; i < size/2; i++, j--)
		{
			temp = string[j];
			string[j] = string[i];
			string[i] = temp;
		}
	}
}

void Transform(char *string, unsigned long *xfm, int num_xfm)
{
	unsigned short xfm_a, xfm_b;
	unsigned char temp;
	
	for(int cur_xfm=0; cur_xfm<num_xfm; cur_xfm++)
	{
		xfm_a=xfm[cur_xfm]>>16;
		xfm_b=xfm[cur_xfm]&0xFFFF;
		
		temp=string[xfm_a];
		string[xfm_a]=string[xfm_b];
		string[xfm_b]=temp;
	}
}

void SwapRows(unsigned long *xfm, int &num_xfm, int str_len, int row_len, int row_a, int row_b)
{
	int index1, index2;

	for(int cur_col=0; cur_col<row_len; cur_col++)
	{
		index1=(row_a*row_len)+cur_col;
		index2=(row_b*row_len)+cur_col;
		if(index1>=str_len || index2>=str_len) continue;
		xfm[num_xfm++]=index1<<16 | index2;
	}
}

void SwapCols(unsigned long *xfm, int &num_xfm, int str_len, int row_len, int col_a, int col_b)
{
	int index1, index2, num_rows=NUM_ROWS(str_len,row_len);

	for(int cur_row=0; cur_row<num_rows; cur_row++)
	{
		index1=(cur_row*row_len)+col_a;
		index2=(cur_row*row_len)+col_b;
		if(index1>=str_len || index2>=str_len) continue;
		xfm[num_xfm++]=index1<<16 | index2;
	}
}

void FlipHorz(unsigned long *xfm, int &num_xfm, int str_len, int row_len)
{
	for(int cur_col=0; cur_col<=(row_len>>1); cur_col++)
		SwapCols(xfm,num_xfm,str_len,row_len,cur_col,row_len-cur_col-1);
}

void FlipVert(unsigned long *xfm, int &num_xfm, int str_len, int row_len)
{
	int num_rows=NUM_ROWS(str_len,row_len);
	
	for(int cur_row=0; cur_row<=(num_rows>>1); cur_row++)
		SwapRows(xfm,num_xfm,str_len,row_len,cur_row,num_rows-cur_row-1);
}
