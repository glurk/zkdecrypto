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
	if(string<0 || string>=num_strings) return 0;
	return RadixSort(strings[string]);
}

void StringArray::SortStrings(int order)
{
	char swap;
	do
	{
		swap=false;
		
		for(int index_a=0; index_a<num_strings-1; index_a++)
			for(int index_b=index_a+1; index_b<num_strings; index_b++)
			{
				int cmp=strcmp(strings[index_b],strings[index_a]);
				if((cmp<0 && !order) || (cmp>0 && order)) {std::swap(strings[index_a],strings[index_b]); swap=true;}
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

int ChrIndex(const char *string, char chr)
{
	char *chr_ptr=strchr((char *)string,chr);
	if(!chr_ptr) return -1;
	return int(chr_ptr-string);
}

int RadixSort(char *string)
{
	char array1[4096], array0[4096]; //arrays for the radix sort	
	int num1, num0, mask=1;
	int length=(int)strlen(string);

	for(int bit=0; bit<8; bit++) //for each bit of the string type
	{
		num1=num0=0; //set num of 0's & 1's back to 0
		for(int index=0; index<length; index++) //for each character in the number array, copy the character to the correct array according to this bit
		{
			if(string[index] & mask) {array1[num1]=string[index]; num1++;} //bit is 1
			else {array0[num0]=string[index]; num0++;} //bit is 0
		}
		//Copy the numbers into the number array from the 1's and 0's arrays, 1's first for decreasing; 0's array first for increasing	
		memcpy(string+num0, array1, num1); 
		memcpy(string, array0, num0); 
		mask<<=1; //bitshift mask left for next bit for next bit
	}

	return 1;
}

void Reverse(char *string)
{
	int i, j, size=(int)strlen(string);

	if(size<=1) return;
	for(i=0, j=size-1; i<size/2; i++, j--)
		std::swap(string[i],string[j]);
}

void Transform(char *string, unsigned long *xfm, int num_xfm)
{
	for(int cur_xfm=0; cur_xfm<num_xfm; cur_xfm++)
		std::swap(string[xfm[cur_xfm]>>16],string[xfm[cur_xfm]&0xFFFF]);
}

void SwapRows(unsigned long *xfm, int &num_xfm, int str_len, int row_len, int row_a, int row_b)
{
	for(int cur_col=0; cur_col<row_len; cur_col++)
	{
		int index1=(row_a*row_len)+cur_col;
		int index2=(row_b*row_len)+cur_col;
		if(index1>=str_len || index2>=str_len) continue;
		xfm[num_xfm++]=index1<<16 | index2;
	}
}

void SwapCols(unsigned long *xfm, int &num_xfm, int str_len, int row_len, int col_a, int col_b)
{
	int num_rows=NUM_ROWS(str_len,row_len);

	for(int cur_row=0; cur_row<num_rows; cur_row++)
	{
		int index1=(cur_row*row_len)+col_a;
		int index2=(cur_row*row_len)+col_b;
		if(index1>=str_len || index2>=str_len) continue;
		xfm[num_xfm++]=index1<<16 | index2;
	}
}

void FlipHorz(unsigned long *xfm, int &num_xfm, int str_len, int row_len)
{
	for(int cur_col=0; cur_col<(row_len>>1); cur_col++)
		SwapCols(xfm,num_xfm,str_len,row_len,cur_col,row_len-cur_col-1);
}

void FlipVert(unsigned long *xfm, int &num_xfm, int str_len, int row_len)
{
	int num_rows=NUM_ROWS(str_len,row_len);
	
	for(int cur_row=0; cur_row<(num_rows>>1); cur_row++)
		SwapRows(xfm,num_xfm,str_len,row_len,cur_row,num_rows-cur_row-1);
}

//count frequencies of chars in a string
int GetUniques(const char *string, char *unique_str, int *unique_freq)
{
	int length, index, ascii_index[256], unique=0;
	
	length=(int)strlen(string);
	
	memset(ascii_index,0,256*sizeof(int));
	
	for(index=0; index<length; index++)
		ascii_index[(unsigned char)string[index]]++;

	for(index=32; index<256; index++)
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
float IoC(const char *string, int length)
{
	int index, freqs[256];
	float ic=0;
	
	memset(freqs,0,1024);
	
	for(index=0; index<length; index++) //frequency table
		freqs[(unsigned char)string[index]]++;

	for(index=32; index<256; index++) //calculate ioc
		if(freqs[index]>1) ic+=(freqs[index])*(freqs[index]-1); 

	ic/=(length)*(length-1);

	return ic;
}

float DIoC(const char* string, int length, int step)
{
	int index, freqs[65536];
	float ic=0;
	
	memset(freqs,0,65536*4);
	
	for(index=0; index<length-1; index+=step) //frequency table
		freqs[(int((unsigned char)string[index])<<8)+(unsigned char)string[index+1]]++;

	for(index=0; index<65536; index++) //calculate dioc
		if(freqs[index]>1) ic+=(freqs[index])*(freqs[index]-1); 
	
	if(step==1) length--;
	if(step==2) length>>=1;

	ic/=(length)*(length-1);

	return ic;
}

float Entropy(const char *string, int length)
{
	int freqs[256], index;
	float entropy=0, prob_mass;
	
	memset(freqs,0,1024);
	
	for(index=0; index<length; index++) //frequency table
		freqs[(unsigned char)string[index]]++;

	for(index=32; index<256; index++) //calculate entropy
	{
		if(!freqs[index]) continue;
		prob_mass=float(freqs[index])/length;
		entropy+=prob_mass*(log(prob_mass)/LOG2);
	}

	if(entropy==0.0) return entropy;
	return (-1*entropy);
}

float ChiSquare(const char *string, int length)
{
	int freqs[256], index, unique=0;
	float chi2=0, prob_mass, cur_calc;

	memset(freqs,0,1024);
	
	for(index=0; index<length; index++) //frequency table, and uniques
	{
		if(!freqs[(unsigned char)string[index]]) unique++;
		freqs[(unsigned char)string[index]]++;
	}

	prob_mass=float(length)/unique;

	for(index=32; index<256; index++) //calculate chi2
	{
		if(!freqs[index]) continue;
		cur_calc=freqs[index]-prob_mass;
		cur_calc*=cur_calc;
		cur_calc/=prob_mass;
		chi2+=cur_calc;
	}

	return chi2/length;
}
/*
float avg_lsoc(const char *string, int length)
{
	int total_clusters=0, cur_length=0, total_length=0;
	
	for(int index=0; index<length; index++)
	{
		//end of cluster
		if(string[index]=='A' || string[index]=='E'  || string[index]=='I' || string[index]=='O'  || string[index]=='U')
		{
			if(index) total_clusters++;
			total_length+=cur_length;
			cur_length=0;
		}

		else cur_length++;
	}

	if(!total_clusters) return 0.0;

	return float(total_length)/total_clusters;
}*/

