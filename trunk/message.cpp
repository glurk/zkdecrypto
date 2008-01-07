#include "message.h"

/*Map*/

#define BLANK 0x97

void Map::Clear(int mode)
{
	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++) 
	{
		if(locked[cur_symbol]) continue;
		
		if(mode & CLR_CIPHER) symbols[cur_symbol].cipher=0;
		if(mode & CLR_PLAIN) symbols[cur_symbol].plain=0;
		if(mode & CLR_FREQ) symbols[cur_symbol].freq=0;
		if(mode==CLR_ALL) 
		{
			num_symbols=0;
			SetAllLock(0);
		}
	}
}

int Map::AddSymbol(SYMBOL symbol, int inc_freq)
{
	if(num_symbols>=MAX_SYM) return 0;

	//search for symbol, and update if it does
	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		if(symbols[cur_symbol].cipher==symbol.cipher)
		{
			if(locked[cur_symbol]) return num_symbols;
			if(IS_ASCII(symbol.plain)) symbols[cur_symbol].plain=symbol.plain;
			if(inc_freq) symbols[cur_symbol].freq++;
			return num_symbols;
		}

	symbols[num_symbols].cipher=symbol.cipher;
	symbols[num_symbols].plain=symbol.plain;
	symbols[num_symbols].freq=1;
	num_symbols++;

	return 1;

}

void Map::SwapSymbols(int swap1, int swap2)
{
	if(locked[swap1] || locked[swap2]) return;
	
	char temp=symbols[swap1].plain;
	symbols[swap1].plain=symbols[swap2].plain;
	symbols[swap2].plain=temp;
}

int Map::GetSymbol(int index, SYMBOL *symbol)
{
	if(index<0 || index>num_symbols) return 0;
	memcpy(symbol,&symbols[index],sizeof(SYMBOL));
	return 1;
}

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

void Map::SortByFreq()
{
	SYMBOL temp;
	int freq1, freq2;
	char cipher1, cipher2;
	int next, swap;

	do
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

#define DECIMAL(N) (N-int(N))

void Map::Init()
{
	int letter, set_symbols=0, max_letter;
	double n[26];

	//calculate real number of occurances for each letter
	for(letter=0; letter<26; letter++)
	{
		n[letter]=(unigraphs[letter]/100)*num_symbols;
		set_symbols+=int(n[letter]);
	}

	while(set_symbols<num_symbols)
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

	//read from file
	msg_len=0;

	while(fscanf(msgfile,"%c",&cur)!=EOF)
	{
		if(!IS_ASCII(cur) || cur==' ') continue;
		cipher[msg_len++]=cur;
	}

	cipher[msg_len]='\0';
	fclose(msgfile);
	
	CipherInfo();

	return msg_len;
}

void Message::CipherInfo()
{
	SYMBOL symbol;
	
	//create map
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
		exp_freq[letter]=ROUNDTOINT((unigraphs[letter]/100)*msg_len);
	
	//patterns
	FindPatterns();
}

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

inline int patcmp(const char *pat1, const char *pat2, char *pat3)
{
	int len1=strlen(pat1);
	int len2=strlen(pat2);
	int len=(len1<len2? len1:len2);
	int diff=0;
	
	for(int chr=0; chr<len; chr++)
	{
		if(pat1[chr]!=pat2[chr])
		{	
			if(chr==0 || chr==len-1) return 0;
			pat3[chr]=BLANK;
			diff++;
		}
		
		else pat3[chr]=pat1[chr];
	}
	
	pat3[len]='\0';
	
	if(!diff) return 0;
	if((float(diff)/len)>=.5) return 0;
	return 1;
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

void Message::FindPatterns()
{
	char *cur_pos;
	int found, next;
	NGRAM pattern;

	num_patterns=0;
	next=true;

	for(int length=2; next; length++)
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
			cur_pos=cipher;
			while(cur_pos=strstr(cur_pos+index,pattern.string)) 
			{
				pattern.positions[pattern.freq]=cur_pos-cipher;
				pattern.freq++;
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

	//sort patterns
	do
	{
		found=false;

		for(int cur_pat=0; cur_pat<num_patterns-1; cur_pat++)
			if(patterns[cur_pat].freq<patterns[cur_pat+1].freq)
			{
				memcpy(&pattern,&patterns[cur_pat],sizeof(NGRAM));
				memcpy(&patterns[cur_pat],&patterns[cur_pat+1],sizeof(NGRAM));
				memcpy(&patterns[cur_pat+1],&pattern,sizeof(NGRAM));
				found=true;
			}
	} 
	while(found);
	
	//near patterns
	NGRAM pattern1, pattern2, pattern3;
	next=true;
	
	for(int length=5; next && length<=MAX_PAT_LEN; length++)
	{
		next=false;
		
		pattern3.length=length;
		pattern3.freq=2;
		
		pattern1.string[length]='\0';
		pattern2.string[length]='\0';
		
		for(int index1=0; index1<msg_len-length-1; index1++)
		{
			memcpy(pattern1.string,cipher+index1,length);
			
			for(int index2=index1+length; index2<msg_len-length-1; index2++)
			{
				memcpy(pattern2.string,cipher+index2,length);
				
				if(patcmp(pattern1.string,pattern2.string,pattern3.string))
				{
					pattern3.positions[0]=index1;
					pattern3.positions[1]=index2;					
					
					AddPattern(&pattern3,true);
					next=true;
				}
			}			
		}
	}
}

int Message::GetPattern(int index, NGRAM *pattern) 
{
	if(index<0 || index>num_patterns) return 0;
	memcpy(pattern,&patterns[index],sizeof(NGRAM));
	return 1;
}
