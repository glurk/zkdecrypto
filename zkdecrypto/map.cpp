#include "headers/map.h"

int Map::Read(const char *filename)
{
	FILE *mapfile;
	char map_cipher[256], map_letter[256], map_locked[256], map_exclude[256];
	int num_symbols, cur_symbol, has_locked=false;
	SYMBOL symbol;

	if(!(mapfile=fopen(filename,"r"))) return 0;

	//cipher, letters, & locked
	fscanf(mapfile,"%s\n",&map_cipher);
	fscanf(mapfile,"%s\n",&map_letter);
	if(fscanf(mapfile,"%s\n",&map_locked)!=EOF) has_locked=true;
	
	num_symbols=(int)strlen(map_cipher);
	
	for(cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
	{
		//blank exclude if not read
		if(fscanf(mapfile,"%s\n",&map_exclude)==EOF)
			map_exclude[0]='\0';
		
		symbol.cipher=map_cipher[cur_symbol];
		symbol.plain=map_letter[cur_symbol];
		strcpy(symbol.exclude,map_exclude);
		AddSymbol(symbol,0);
		
		if(has_locked) locked[cur_symbol]=map_locked[cur_symbol]-'0';
		else locked[cur_symbol]=0;
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

	//cipher symbols
	for(cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		putc(symbols[cur_symbol].cipher,mapfile);

	putc('\n',mapfile);

	//letters
	for(cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
	{
		if(!symbols[cur_symbol].plain) putc(BLANK,mapfile);
		else putc(symbols[cur_symbol].plain,mapfile);
	}
	
	putc('\n',mapfile);
	
	//locks
	for(cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		fprintf(mapfile,"%i",locked[cur_symbol]);
		
	putc('\n',mapfile);

	//exclusions
	for(cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		fprintf(mapfile,"%s\n",symbols[cur_symbol].exclude);

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
		memset(locked,0,MAX_SYM);
		merge_log[0]='\0';
		return;
	}
	
	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++) 
	{
		if(locked[cur_symbol]) continue;
		
		if(mode & CLR_CIPHER) symbols[cur_symbol].cipher=0;
		if(mode & CLR_PLAIN) symbols[cur_symbol].plain=0;
		if(mode & CLR_FREQ) symbols[cur_symbol].freq=0;
		if(mode & CLR_EXCLUDE) symbols[cur_symbol].exclude[0]='\0';
	}
}

//initialize key to array of integers, homophones for each letter
void Map::Init(int *ltr_homo)
{
	int cur_letter=0;

	memset(locked,0,num_symbols);

	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
	{
		//skip letter if no homophones left
		while(!ltr_homo[cur_letter] && cur_letter<26) cur_letter++; 

		if(cur_letter<26) //if all letters have not been used
		{
			symbols[cur_symbol].plain=cur_letter+'A'; //set to letter
			ltr_homo[cur_letter]--; //dec remaining homophones for this letter
		}

		else //all letters have been used, remaining symbols are blank
		{
			symbols[cur_symbol].plain=0;
			locked[cur_symbol]=true;
		}
	}
}

//make plain text characters same as cipher characters (for transposition)
void Map::AsCipher()
{
	memset(locked,0,num_symbols);

	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		symbols[cur_symbol].plain=symbols[cur_symbol].cipher;
}

//add/update a symbol; if inc_freq is true, 
//when an existing symbol is updated its frequency is incremented
int Map::AddSymbol(SYMBOL &symbol, int inc_freq)
{
	int index;

	if(num_symbols>=MAX_SYM) return 0;

	//search for symbol, and update if it exists
	index=FindByCipher(symbol.cipher);
	
	if(index>-1) //existing symbol
	{
		strcpy(symbols[index].exclude,symbol.exclude);
		
		if(locked[index]) return num_symbols;
		//if(IS_ASCII(symbol.plain)) 
		symbols[index].plain=symbol.plain;
		if(inc_freq) symbols[index].freq++;
		return num_symbols;
	}

	symbols[num_symbols].cipher=symbol.cipher;
	symbols[num_symbols].plain=symbol.plain;
	symbols[num_symbols].freq=1;
	strcpy(symbols[num_symbols].exclude,symbol.exclude);
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
	if(locked[swap1] || locked[swap2]) return;
	if(strchr(symbols[swap1].exclude,symbols[swap2].plain)) return; 
	if(strchr(symbols[swap2].exclude,symbols[swap1].plain)) return;
	std::swap(symbols[swap1].plain,symbols[swap2].plain);
}

//replace all instances of symbol2 with symbol1, reset info
void Map::MergeSymbols(char symbol1, char symbol2)
{
	int index1, index2;
	char log[3];

	index1=FindByCipher(symbol1);
	index2=FindByCipher(symbol2);

	num_symbols--;
	symbols[index1].freq+=symbols[index2].freq;
	memmove(&symbols[index2],&symbols[index2+1],(num_symbols-index2)*sizeof(SYMBOL));
	memmove(&locked[index2],&locked[index2+1],num_symbols-index2);

	//append to log
	sprintf(log,"%c%c",symbol1,symbol2);
	strcat(merge_log,log);

	SortByFreq();
}

//sort the symbols in the same order that hillclimber expects
void Map::SortByFreq()
{
	SYMBOL temp_sym;
	int freq1, freq2;
	unsigned char cipher1, cipher2;
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
				memcpy(&temp_sym,&symbols[cur_symbol],sizeof(SYMBOL));
				memcpy(&symbols[cur_symbol],&symbols[cur_symbol+1],sizeof(SYMBOL));
				memcpy(&symbols[cur_symbol+1],&temp_sym,sizeof(SYMBOL));
				std::swap(locked[cur_symbol],locked[cur_symbol+1]);
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

long Map::SymbolGraph(wchar *dest)
{
	int max, step, rows=0, cur_symbol;
	int dest_index=0;
	char level[64];

	max=symbols[0].freq;
	
	//calculate rows, step
	step=ROUNDUP(float(max)/MAX_GRA_ROW);
	max+=step-(max%step);

	dest[0]=0;

	//line numbers and bars
	for(int row=max; row>0; row-=step)
	{
		sprintf(level,"%4i ",row);
		ustrcat(dest,level);
		ustrcat(dest,UNI_VERTBAR);

		for(cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		{
			if(symbols[cur_symbol].freq>=row) ustrcat(dest,UNI_HALFBAR);
			else ustrcat(dest,0x0020); 
		}

		ustrcat(dest,0x000D); 
		ustrcat(dest,0x000A); 
		
		rows++;
	}

	//bottom line
	ustrcat(dest,"     ");

	for(cur_symbol=0; cur_symbol<=num_symbols; cur_symbol++)
		ustrcat(dest,UNI_HORZBAR);

	//symbols
	ustrcat(dest,"\r\n      ");

	for(cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		ustrcat(dest,(unsigned char)symbols[cur_symbol].cipher);

	//rows in the high word, cols in the low
	return (rows+2)<<16 | (num_symbols+8);
}

long Map::GetMergeLog(wchar *dest)
{
	char temp[64];
	int length;
	
	length=(int)strlen(merge_log)>>1;

	dest[0]='\0';

	for(int cur_merge=0; cur_merge<length; cur_merge++)
	{
		sprintf(temp,"%2i. %c %c\n",cur_merge+1,merge_log[cur_merge<<1],merge_log[(cur_merge<<1)+1]);
		ustrcat(dest,temp);
	}
	
	if(dest[0]=='\0') return 0;
	else return (length+2)<<16 | 15;
}

long Map::GetExclusions(wchar *dest, int num_cols)
{
	char temp[64];
	int rows=0, col=0;
	
	dest[0]='\0';
	
	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
	{
		if(symbols[cur_symbol].exclude[0]!=0) {
		sprintf(temp,"%c  %-26s  \r\n",symbols[cur_symbol].cipher,symbols[cur_symbol].exclude);
		ustrcat(dest,temp);
		col++; }
	}
	
//	return ((rows<<1)+2)<<16 | (num_cols*32);
	return (long)col;
}

//hillclimb key <-> Map class conversion
void Map::ToKey(char *key, char *extra)
{
	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
	{
		if(symbols[cur_symbol].plain) key[cur_symbol]=symbols[cur_symbol].plain;
		else key[cur_symbol]=BLANK;
	}
	
	key[num_symbols]='\0';

	strcat(key,extra);
}

void Map::FromKey(char *key)
{
	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		symbols[cur_symbol].plain=key[cur_symbol];
}






//clear map data, you can | together mode values, or use CLR_ALL
void DiMap::Clear(int mode)
{
	if(mode==CLR_ALL) 
	{
		num_digraphs=0;
		memset(digraphs,0,MAX_DI*sizeof(DIGRAPH));
		memset(locked,0,MAX_DI);
		return;
	}
	
	for(int cur_digraph=0; cur_digraph<num_digraphs; cur_digraph++) 
	{
		if(locked[cur_digraph]) continue;

		if(mode & CLR_CIPHER) digraphs[cur_digraph].cipher1=digraphs[cur_digraph].cipher2=0;
		if(mode & CLR_PLAIN) digraphs[cur_digraph].plain1=digraphs[cur_digraph].plain2=0;
		if(mode & CLR_FREQ) digraphs[cur_digraph].freq=0;
	}
}

void DiMap::Init(int num)
{
	int cur_letter=0;

	memset(locked,0,num_digraphs);

	for(int cur_digraph=0; cur_digraph<num_digraphs; cur_digraph++)
	{
		if(cur_digraph<num)
		{
			digraphs[cur_digraph].plain1=((cur_letter++)%26)+'A';
			digraphs[cur_digraph].plain2=((cur_letter++)%26)+'A';
		}

		else locked[cur_digraph]=true;
	}
}

void DiMap::AsCipher()
{
	memset(locked,0,num_digraphs);

	for(int cur_digraph=0; cur_digraph<num_digraphs; cur_digraph++)
	{
		digraphs[cur_digraph].plain1=digraphs[cur_digraph].cipher1;
		digraphs[cur_digraph].plain2=digraphs[cur_digraph].cipher2;
	}
}

//add/update a digraph; if inc_freq is true, 
//when an existing symbol is updated its frequency is incremented
int DiMap::AddDigraph(DIGRAPH &digraph, int inc_freq)
{
	if(num_digraphs>=MAX_DI) return 0;

	//search for digraph, and update if it exists
	int index=FindByCipher(digraph.cipher1,digraph.cipher2);
	
	if(index>-1) //existing symbol
	{
		if(locked[index]) return num_digraphs;

		digraphs[index].plain1=digraph.plain1;
		digraphs[index].plain2=digraph.plain2;
		if(inc_freq) digraphs[index].freq++;
		return num_digraphs;
	}

	digraphs[num_digraphs].cipher1=digraph.cipher1;
	digraphs[num_digraphs].cipher2=digraph.cipher2;
	digraphs[num_digraphs].plain1=digraph.plain1;
	digraphs[num_digraphs].plain2=digraph.plain2;
	digraphs[num_digraphs].freq=1;
	num_digraphs++;

	return 1;
}

//return index of symbol with specified cipher character
int DiMap::FindByCipher(char cipher1, char cipher2)
{
	for(int cur_digraph=0; cur_digraph<num_digraphs; cur_digraph++)
		if(digraphs[cur_digraph].cipher1==cipher1 && digraphs[cur_digraph].cipher2==cipher2)
			return cur_digraph;

	return -1;
}

//get digraph at index
int DiMap::GetDigraph(int index, DIGRAPH *digraph)
{
	if(index<0 || index>num_digraphs) return 0;
	memcpy(digraph,&digraphs[index],sizeof(DIGRAPH));
	return 1;
}

//swap the plain text letters for two digraphs
void DiMap::SwapSymbols(int swap1, int swap2)
{
	if(locked[swap1] || locked[swap2]) return;

	std::swap(digraphs[swap1].plain1,digraphs[swap2].plain1);
	std::swap(digraphs[swap1].plain2,digraphs[swap2].plain2);
}

//hillclimb key <-> Map class conversion
void DiMap::ToKey(char *key, char *extra)
{
	for(int cur_digraph=0; cur_digraph<num_digraphs; cur_digraph++)
	{
		if(digraphs[cur_digraph].plain1) key[cur_digraph<<1]=digraphs[cur_digraph].plain1;
		else key[cur_digraph<<1]=BLANK;
		if(digraphs[cur_digraph].plain2) key[(cur_digraph<<1)+1]=digraphs[cur_digraph].plain2;
		else key[(cur_digraph<<1)+1]=BLANK;
	}
	
	key[num_digraphs<<1]='\0';

	strcat(key,extra);
}

void DiMap::FromKey(char *key)
{
	for(int cur_digraph=0; cur_digraph<num_digraphs; cur_digraph++)
	{
		digraphs[cur_digraph].plain1=key[cur_digraph<<1];
		digraphs[cur_digraph].plain2=key[(cur_digraph<<1)+1];
	}
}

//sort the symbols in the same order that hillclimber expects
void DiMap::SortByFreq()
{
	DIGRAPH temp_di;
	int freq1, freq2;
	unsigned short cipher_val1, cipher_val2;
	int next, swap;

	do //buble sort
	{
		next=false;

		for(int cur_digraph=0; cur_digraph<num_digraphs-1; cur_digraph++)
		{
			cipher_val1=digraphs[cur_digraph].cipher1<<8 | digraphs[cur_digraph].cipher2;
			cipher_val2=digraphs[cur_digraph+1].cipher1<<8 | digraphs[cur_digraph+1].cipher2;
			freq1=digraphs[cur_digraph].freq;
			freq2=digraphs[cur_digraph+1].freq;
			
			if(freq1<freq2) swap=true;
			else if(freq1==freq2 && cipher_val1<cipher_val2) swap=true;
			else swap=false;
			
			if(swap)
			{
				memcpy(&temp_di,&digraphs[cur_digraph],sizeof(DIGRAPH));
				memcpy(&digraphs[cur_digraph],&digraphs[cur_digraph+1],sizeof(DIGRAPH));
				memcpy(&digraphs[cur_digraph+1],&temp_di,sizeof(DIGRAPH));
				std::swap(locked[cur_digraph],locked[cur_digraph+1]);
				next=true;
			}
		}
	} 
	while(next);
}