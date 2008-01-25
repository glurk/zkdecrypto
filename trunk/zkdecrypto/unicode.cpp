#include "headers/unicode.h"

//convert an ascii string into unicode
int ustrcpy(wchar *unicode, char *ascii)
{
	int length, index;
	
	length=(int)strlen(ascii);

	for(index=0; index<length; index++)
		unicode[index]=ascii[index];

	unicode[index]=0;
	
	return length;
}

int ustrlen(wchar *string)
{
	int index=0;

	while(string[index]) index++;

	return index;
}

int ustrcat(wchar *dest, wchar *src)
{
	int dest_len, src_len;
	
	dest_len=ustrlen(dest);
	src_len=ustrlen(src);

	for(int src_index=0; src_index<=src_len; src_index++)
		dest[dest_len+src_index]=src[src_index];

	return dest_len+src_len;
}

int ustrcat(wchar *dest, wchar src)
{
	int dest_len=ustrlen(dest);

	dest[dest_len]=src;
	dest[dest_len+1]=0;

	return dest_len+1;
}

int ustrcat(wchar *dest, char *src)
{
	int dest_len, src_len;
	wchar *uni;

	dest_len=ustrlen(dest);
	src_len=(int)strlen(src);

	uni=new wchar[src_len+1];
	ustrcpy(uni,src);
	ustrcat(dest,uni);
	delete[] uni;
	return dest_len+src_len;
}
