#pragma warning( disable : 4244)	// STOP MSVS2005 WARNINGS

#include "message.h"

/*Map*/

int Map::Read(const char *filename)
{
	FILE *mapfile;
	char cur;
	int in_chars, cur_symbol;
	SYMBOL entries[MAX_SYM];

	if(!(mapfile=fopen(filename,"r"))) return 0;

	in_chars=0;
	cur_symbol=0;

	memset(entries,0,MAX_SYM*sizeof(SYMBOL));

	while(fscanf(mapfile,"%c",&cur)!=EOF)
	{
		//start letters
		if(cur==0x0D || cur==0x0A) 
		{
			if(in_chars) break;
			in_chars=1;
			cur_symbol=0;
		}

		if(!IS_ASCII(cur)) continue;
		if(cur==' ') cur=0x00;
		if(!in_chars) entries[cur_symbol].cipher=cur;
		else entries[cur_symbol].plain=cur;

		AddSymbol(entries[cur_symbol],0);
		cur_symbol++;
	}

	fclose(mapfile);

	return 1;
}

int Map::Write(const char *filename)
{
	FILE *mapfile;
	int cur_symbol;

	mapfile=fopen(filename,"w");

	if(!mapfile) return 0;

	for(cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		putc(symbols[cur_symbol].cipher,mapfile);

	putc('\n',mapfile);

	for(cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
	{
		if(!symbols[cur_symbol].plain) putc(' ',mapfile);
		else putc(symbols[cur_symbol].plain,mapfile);
	}

	fclose(mapfile);

	return 1;
}

//clear map data, you can | together mode values, or use CLR_ALL
void Map::Clear(int mode)
{
	if(mode==CLR_ALL) 
	{
		num_symbols=0;
		memset(symbols,0,MAX_SYM*sizeof(SYMBOL));
		return;
	}
	
	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++) 
	{
		if(symbols[cur_symbol].locked) continue;
		
		if(mode & CLR_CIPHER) symbols[cur_symbol].cipher=0;
		if(mode & CLR_PLAIN) symbols[cur_symbol].plain=0;
		if(mode & CLR_FREQ) symbols[cur_symbol].freq=0;
	}
}

void Map::Init(int first)
{
	int letter, set_symbols=0, max_letter;
	double n[26];
	
	if(first<0) return;
	if(first>num_symbols) first=num_symbols;

	//calculate real number of occurances for each letter
	for(letter=0; letter<26; letter++)
	{
		n[letter]=(unigraphs[letter]/100)*first;
		set_symbols+=int(n[letter]);
	}

	while(set_symbols<first)
	{
		//find the letter with the highest decimal
		max_letter=0;

		for(letter=0; letter<26; letter++)
			if(DECIMAL(n[letter])>DECIMAL(n[max_letter])) 
				max_letter=letter;
	
		//set that letter to the next whole number
		n[max_letter]=int(n[max_letter])+1;
		set_symbols++;
	}

	//set symbols
	int cur_symbol=0;

	for(letter=0; letter<26; letter++)
		while(n[letter]>=1)
		{		
			symbols[cur_symbol++].plain=letter+'A';
			n[letter]--;
		}
	
	SetAllLock(0);
	
	//clear and lock the rest
	while(cur_symbol<num_symbols)
	{
		symbols[cur_symbol].plain=0;
		symbols[cur_symbol].locked=true;
		cur_symbol++;
	}
}

//add/update a symbol; if inc_freq is true, 
//when an existing symbol is updated its frequency is incremented
int Map::AddSymbol(SYMBOL symbol, int inc_freq)
{
	int index;

	if(num_symbols>=MAX_SYM) return 0;

	//search for symbol, and update if it exists
	index=FindByCipher(symbol.cipher);
	
	if(index>-1)
	{
		if(symbols[index].locked) return num_symbols;
		//if(IS_ASCII(symbol.plain)) 
		symbols[index].plain=symbol.plain;
		symbols[index].locked=symbol.locked;
		if(inc_freq) symbols[index].freq++;
		return num_symbols;
	}

	symbols[num_symbols].cipher=symbol.cipher;
	symbols[num_symbols].plain=symbol.plain;
	symbols[num_symbols].freq=1;
	num_symbols++;

	return 1;
}

//return index of symbol with specified cipher character
int Map::FindByCipher(char cipher)
{
	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		if(symbols[cur_symbol].cipher==cipher)
			return cur_symbol;

	return -1;
}

//get symbol at index
int Map::GetSymbol(int index, SYMBOL *symbol)
{
	if(index<0 || index>num_symbols) return 0;
	memcpy(symbol,&symbols[index],sizeof(SYMBOL));
	return 1;
}

//swap the plain text letters for two symbols
void Map::SwapSymbols(int swap1, int swap2)
{
	if(symbols[swap1].locked || symbols[swap2].locked) return;
	
	char temp=symbols[swap1].plain;
	symbols[swap1].plain=symbols[swap2].plain;
	symbols[swap2].plain=temp;
}

void Map::MergeSymbols(char symbol1, char symbol2)
{
	int index1, index2;

	index1=FindByCipher(symbol1);
	index2=FindByCipher(symbol2);

	num_symbols--;
	symbols[index1].freq+=symbols[index2].freq;
	memmove(&symbols[index2],&symbols[index2+1],(num_symbols-index2)*sizeof(SYMBOL));

	SortByFreq();
}

//sort the symbols in the same order that hillclimber expects
void Map::SortByFreq()
{
	SYMBOL temp;
	int freq1, freq2;
	char cipher1, cipher2;
	int next, swap;

	do //buble sort
	{
		next=false;

		for(int cur_symbol=0; cur_symbol<num_symbols-1; cur_symbol++)
		{
			cipher1=symbols[cur_symbol].cipher;
			cipher2=symbols[cur_symbol+1].cipher;
			freq1=symbols[cur_symbol].freq;
			freq2=symbols[cur_symbol+1].freq;
			
			if(freq1<freq2) swap=true;
			else if(freq1==freq2 && cipher1<cipher2) swap=true;
			else swap=false;
			
			if(swap)
			{
				memcpy(&temp,&symbols[cur_symbol],sizeof(SYMBOL));
				memcpy(&symbols[cur_symbol],&symbols[cur_symbol+1],sizeof(SYMBOL));
				memcpy(&symbols[cur_symbol+1],&temp,sizeof(SYMBOL));
				next=true;
			}
		}
	} 
	while(next);
}

//get string for the symbols table
void Map::SymbolTable(char *dest)
{
	SYMBOL symbol;
	int cur_char=0;

	for(int letter=0; letter<26; letter++)
	{
		dest[cur_char++]=letter+'A'; dest[cur_char++]=' ';
		
		for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		{
			GetSymbol(cur_symbol,&symbol);
			if(symbol.plain==letter+'A') 
				dest[cur_char++]=symbol.cipher;
		}

		dest[cur_char++]='\r'; dest[cur_char++]='\n';
	}

	dest[cur_char++]='\0';
}

//hillclimb key <-> Map class conversion
void Map::ToKey(char *key)
{
	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
	{
		if(symbols[cur_symbol].plain) key[cur_symbol]=symbols[cur_symbol].plain;
		else key[cur_symbol]=BLANK;
	}	
	
	key[num_symbols]='\0';
}

void Map::FromKey(char *key)
{
	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		symbols[cur_symbol].plain=key[cur_symbol];
}

/*Message*/
int Message::Read(const char *filename)
{
	FILE *msgfile;
	int size;
	char cur;

	if(!(msgfile=fopen(filename,"r"))) return 0;

	//get file size
	fseek(msgfile,0,SEEK_END);
	size=ftell(msgfile)+1;
	fseek(msgfile,0,SEEK_SET);

	//allocate text arrays
	if(cipher) delete[] cipher;
	if(plain) delete[] plain;
	cipher=new char[size];
	plain=new char[size];

	if(!cipher || !plain) return 0;

	//read from file
	msg_len=0;

	while(fscanf(msgfile,"%c",&cur)!=EOF)
	{
		if(!IS_ASCII(cur) || cur==' ') continue;
		cipher[msg_len++]=cur;
	}

	cipher[msg_len]='\0';
	fclose(msgfile);
	
	SetInfo();

	return msg_len;
}

void Message::SetCipher(const char *new_cipher)
{
	msg_len=strlen(new_cipher);
	strcpy(cipher,new_cipher);
	SetInfo();
}

//set all info for new cipher data
void Message::SetInfo()
{
	SYMBOL symbol;
	
	//create map
	memset(&symbol,0,sizeof(SYMBOL));
	cur_map.Clear(CLR_ALL);
	symbol.plain='\0';
	
	for(int chr=0; chr<msg_len; chr++)
	{
		symbol.cipher=cipher[chr];
		cur_map.AddSymbol(symbol,1);
	}
	
	//sort map
	cur_map.SortByFreq();

	//set expected frequencies
	for(int letter=0; letter<26; letter++)
		exp_freq[letter]=ROUNDTOINT((cur_map.GetUnigraph(letter)/100)*msg_len);
	
	//patterns
	FindPatterns();
}

//put the specifed row in dest, according to the specifiec line length 
int Message::GetRow(int row, int line_len, char *dest)
{
	int row_len, row_start, cipher_end;
	
	//find start
	row_start=row*line_len;
	row_len=line_len;
	
	//would start beyond the end of buffer
	if(row_start>=msg_len) return 0;
	
	//not enough left for a complete line
	if(row_start+row_len>msg_len) row_len=msg_len-row_start;
	
	memcpy(dest,cipher+row_start,row_len);
	dest[row_len]='\0';
	
	return row_len;
}

//put the specifed column in dest, according to the specifiec line length 
int Message::GetColumn(int col, int line_len, char *dest)
{
	int row, msg_index;
	
	//columng greater than line length
	if(col>=line_len) return 0;
	
	//get character at the same index of every row possible
	msg_index=col;
	
	for(row=0; msg_index<msg_len; row++)
	{
		dest[row]=cipher[msg_index];
		msg_index+=line_len;
	}
	
	dest[row]='\0';
	
	return row;
}

//decode cipher into plain
void Message::Decode()
{
	int cur_symbol, num_symbols;
	char decoder[256];
	SYMBOL symbol;

	num_symbols=cur_map.GetNumSymbols();

	//setup decoding table
	for(cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
	{
		cur_map.GetSymbol(cur_symbol,&symbol);
		decoder[symbol.cipher]=symbol.plain;
	}

	//decode string
	for(cur_symbol=0; cur_symbol<msg_len; cur_symbol++)
	{
		plain[cur_symbol]=decoder[cipher[cur_symbol]];
		if(!plain[cur_symbol]) plain[cur_symbol]=BLANK;
		//do transposition here
	}

	plain[msg_len]='\0';
}

void Message::GetExpFreq(int *freq)
{
	memcpy(freq,exp_freq,26*sizeof(int));
}

void Message::GetActFreq(int *freq)
{
	memset(freq,0,26*sizeof(int));
	
	for(int chr=0; chr<msg_len; chr++)
		freq[plain[chr]-'A']++;
}

//replace all instances of symbol2 with symbol1, and update map
void Message::MergeSymbols(char symbol1, char symbol2)
{
	Map old_map;

	if(symbol1==symbol2) return;

	for(int index=0; index<msg_len; index++)
		if(cipher[index]==symbol2) cipher[index]=symbol1;

	//merge symbols in map and find new patterns
	cur_map.MergeSymbols(symbol1,symbol2);
	FindPatterns();
}

int Message::Simplify(char &simp1, char &simp2)
{
	int max_patterns, old_patterns;
	char max1, max2;
	Message test_msg;
	SYMBOL symbol1, symbol2;
	
	old_patterns=max_patterns=num_patterns;
	
	test_msg=*this;

	//find the best of all possible sustitutions 
	for(int cur_sym1=0; cur_sym1<(cur_map.GetNumSymbols())-1; cur_sym1++)
	{
		cur_map.GetSymbol(cur_sym1,&symbol1);

		for(int cur_sym2=cur_sym1+1; cur_sym2<cur_map.GetNumSymbols(); cur_sym2++)
		{
			cur_map.GetSymbol(cur_sym2,&symbol2);

			test_msg.MergeSymbols(symbol1.cipher,symbol2.cipher);
			
			//good substitution
			if(test_msg.num_patterns>max_patterns)
			{
				max_patterns=test_msg.num_patterns;
				max1=symbol1.cipher;
				max2=symbol2.cipher;
			}

			test_msg=*this;
		}
	}

	simp1=max1;
	simp2=max2;
	MergeSymbols(max1,max2);

	return (max_patterns-old_patterns);
}

int Message::AddPattern(NGRAM *pattern, int inc_freq)
{
	//find pattern
	for(int cur_pat=0; cur_pat<num_patterns; cur_pat++)
		if(!strcmp(pattern->string,patterns[cur_pat].string))
		{
			if(inc_freq) 
			{
				patterns[cur_pat].positions[patterns[cur_pat].freq]=pattern->positions[1];
				patterns[cur_pat].freq++;
			}
			return num_patterns;
		}
	
	if(num_patterns==MAX_PAT) return num_patterns;
		
	memcpy(&patterns[num_patterns],pattern,sizeof(NGRAM));
	num_patterns++;	
	
	return num_patterns;	
}

//check if pat1 & pat2 are more than 50% similar, put pattern string into pat3
inline int pat_match(const char *pat1, const char *pat2, char *pat_temp, int len)
{
	int diff=0, run=0;
	int end=len-1;

	//first & last characters must be the same
	if(pat1[0]!=pat2[0]) return 0;
	if(pat1[end]!=pat2[end]) return 0;

	//set characters in the template string
	pat_temp[0]=pat1[0];
	pat_temp[end]=pat1[end];
	
	//check other characters
	for(int chr=1; chr<end; chr++)
	{
		//different
		if(pat1[chr]!=pat2[chr]) {pat_temp[chr]=BLANK; diff++; run++;}
		
		//same
		else {pat_temp[chr]=pat1[chr]; run=0;}
		
		//fail if 3 characters in a row are different
		if(run>3) return 0;
	}
	
	pat_temp[len]='\0';
	
	if(!diff) return 0; //exactly the same
	if((len-diff)<=len>>1) return 0; //at least half of the characters are different
	
	return 1;
}

void Message::FindPatterns()
{
	char *cur_pos;
	int length, next;
	NGRAM pattern;

	num_patterns=0;
	next=true;

	//exact patterns
	for(length=2; next && length<MAX_PAT_LEN; length++)
	{
		next=false;
		
		for(int index=0; index<msg_len-length-1; index++)
		{
			//init pattern
			memcpy(pattern.string,cipher+index,length);
			pattern.string[length]='\0';
			pattern.length=length;
			pattern.freq=0;
			
			//count pattern
			cur_pos=cipher+index;
			while(cur_pos=strstr(cur_pos,pattern.string)) 
			{
				pattern.positions[pattern.freq]=cur_pos-cipher;
				pattern.freq++;
				//cur_pos++;
				cur_pos+=length;
			}
		
			//add pattern
			if(pattern.freq>2 || (pattern.length>2 && pattern.freq>1))
			{
				AddPattern(&pattern,false);
				next=true;
			}
		}
	}
	
	//near patterns
	next=0;
	
	for(length=4; length<=MAX_PAT_LEN; length++)
	{
		if(next>2) break;

		next++;

		pattern.length=length;
		pattern.freq=2;
		
		for(int index1=0; index1<msg_len-length-2; index1++)
			for(int index2=index1+length; index2<msg_len-length; index2++)
			{
				if(pat_match(cipher+index1,cipher+index2,pattern.string,length))
				{
					pattern.positions[0]=index1;
					pattern.positions[1]=index2;					
					AddPattern(&pattern,true);
					next=0;
				}
			}	
	}

	//sort patterns
	int swap;
	int found;
	do
	{
		found=false;

		for(int cur_pat=0; cur_pat<num_patterns-1; cur_pat++)
		{
			swap=false;

			//if(patterns[cur_pat].freq<patterns[cur_pat+1].freq) swap=true;
			if(strcmp(patterns[cur_pat].string,patterns[cur_pat+1].string)>0) swap=true;
			
			if(swap)
			{
				memcpy(&pattern,&patterns[cur_pat],sizeof(NGRAM));
				memcpy(&patterns[cur_pat],&patterns[cur_pat+1],sizeof(NGRAM));
				memcpy(&patterns[cur_pat+1],&pattern,sizeof(NGRAM));
				found=true;
			}
		}
	} 
	while(found);
}

int Message::GetPattern(int index, NGRAM *pattern) 
{
	if(index<0 || index>num_patterns) return 0;
	memcpy(pattern,&patterns[index],sizeof(NGRAM));
	return 1;
}

