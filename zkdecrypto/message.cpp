#pragma warning( disable : 4244)	// STOP MSVS2005 WARNINGS

#include "headers/message.h"

/*Map*/

//count frequencies of chars in a string
void GetFreqs(const char *string, int *freqs)
{
	int length=(int)strlen(string);
	
	memset(freqs,0,256*sizeof(int));
	
	for(int index=0; index<length; index++)
		freqs[string[index]]++;
}

//index of conincidence of a string
float IoC(const char *string)
{
	int freqs[256], length;
	float ic=0;

	if(!string) return 0;

	length=(int)strlen(string);

	if(length<2) return 0;	

	GetFreqs(string,freqs);

	//calculate index of conincidence
	for(int sym_index=0; sym_index<256; sym_index++)
		if(freqs[sym_index]>1) 
			ic+=(freqs[sym_index])*(freqs[sym_index]-1); 

	ic/=(length)*(length-1);

	return ic;
}

float Entropy(const char *string)
{
	int freqs[256], length;
	float entropy=0, prob_mass, log2;

	if(!string) return 0;

	length=(int)strlen(string);
	
	if(length<0) return 0;

	GetFreqs(string,freqs);

	//for log base conversion
	log2=log(2);

	//calculate entropy
	for(int sym_index=0; sym_index<256; sym_index++)
		if(freqs[sym_index])
		{
			prob_mass=float(freqs[sym_index])/length;
			entropy+=prob_mass*(log(prob_mass)/log2);
		}

	return (-1*entropy);
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
		if(cur_symbol>127) continue;
		
		if(cur==' ') //blank 
		{
			if(!in_chars) continue;
			cur=0x00;
		}
		
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
		memset(locked,0,MAX_SYM);
		return;
	}
	
	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++) 
	{
		if(locked[cur_symbol]) continue;
		
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
		locked[cur_symbol]=true;
		cur_symbol++;
	}
}

//add/update a symbol; if inc_freq is true, 
//when an existing symbol is updated its frequency is incremented
int Map::AddSymbol(SYMBOL &symbol, int inc_freq)
{
	int index;

	if(num_symbols>=MAX_SYM) return 0;

	//search for symbol, and update if it exists
	index=FindByCipher(symbol.cipher);
	
	if(index>-1)
	{
		if(locked[index]) return num_symbols;
		//if(IS_ASCII(symbol.plain)) 
		symbols[index].plain=symbol.plain;
		locked[index]=false;
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
	if(locked[swap1] || locked[swap2]) return;
	
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
	memmove(&locked[index2],&locked[index2+1],num_symbols-index2);

	SortByFreq();
}

//sort the symbols in the same order that hillclimber expects
void Map::SortByFreq()
{
	SYMBOL temp_sym;
	int freq1, freq2;
	char cipher1, cipher2, temp_lock;
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

				temp_lock=locked[cur_symbol];
				locked[cur_symbol]=locked[cur_symbol+1];
				locked[cur_symbol+1]=temp_lock;

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
		ustrcat(dest,symbols[cur_symbol].cipher);

	//rows in the high word, cols in the low
	return (rows+2)<<16 | (num_symbols+8);
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
	msg_len=(int)strlen(new_cipher);
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
	FindPatterns(true);
}

//put the specifed row in dest, according to the specifiec line length 
int Message::GetRow(int row, int line_len, char *dest)
{
	int row_len, row_start;
	
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
void Message::MergeSymbols(char symbol1, char symbol2, int do_near)
{
	Map old_map;

	if(symbol1==symbol2) return;

	for(int index=0; index<msg_len; index++)
		if(cipher[index]==symbol2) cipher[index]=symbol1;

	//merge symbols in map and find new patterns
	cur_map.MergeSymbols(symbol1,symbol2);
	FindPatterns(do_near);
}

//try to identify homophones and merge them
int Message::Simplify(char &simp1, char &simp2)
{
	int max_patterns, old_patterns, num_symbols;
	char max1=char(0xFF), max2;
	Message test_msg;
	SYMBOL symbol1, symbol2;
	
	//only use exact patterns
	FindPatterns(false);
	old_patterns=max_patterns=good_pat;
	num_symbols=cur_map.GetNumSymbols();
	
	test_msg+=*this;

	//find the best of all possible sustitutions 
	for(int cur_sym1=0; cur_sym1<num_symbols-1; cur_sym1++)
	{
		if(cur_map.GetLock(cur_sym1)) continue;
		cur_map.GetSymbol(cur_sym1,&symbol1);

		for(int cur_sym2=cur_sym1+1; cur_sym2<num_symbols; cur_sym2++)
		{
			if(cur_map.GetLock(cur_sym2)) continue;
			cur_map.GetSymbol(cur_sym2,&symbol2);

			test_msg.MergeSymbols(symbol1.cipher,symbol2.cipher,false);
			
			//good substitution
			if(test_msg.good_pat>max_patterns)
			{
				max_patterns=test_msg.good_pat;
				max1=symbol1.cipher;
				max2=symbol2.cipher;
			}

			test_msg+=*this;
		}
	}

	simp1=max1;
	simp2=max2;
	//if(max1!=char(0xFF)) MergeSymbols(max1,max2,true);

	return (max_patterns-old_patterns);
}

//find a pattern in the tree & set dest to it
int Message::FindPattern(const char *string, NGRAM *&dest, NGRAM *cur_pat, NGRAM *last_pat=NULL)
{
	//doesn't exist
	if(!cur_pat) {dest=last_pat; return 0;}

	//follow branch
	int cmp=strcmp(string,cur_pat->string);
	if(cmp<0) return FindPattern(string,dest,cur_pat->left,cur_pat);
	if(cmp>0) return FindPattern(string,dest,cur_pat->right,cur_pat);
	
	//found
	dest=cur_pat;
	return 1;
}


int Message::FindPattern(const char *string, NGRAM *&dest)
{
	NGRAM *cur_pat, *last_pat=NULL;
	int cmp;

	cur_pat=patterns; //start at head

	while(cur_pat)
	{
		last_pat=cur_pat;

		cmp=strcmp(string,cur_pat->string);
		if(cmp<0) cur_pat=cur_pat->left;
		else if(cmp>0) cur_pat=cur_pat->right;
		else //this is it
		{
			dest=cur_pat;
			return 1;
		}
	}

	dest=last_pat; //doesn't exist, give the would be parent
	return 0;
}

int Message::AddPattern(NGRAM &new_pat, int inc_freq)
{
	NGRAM *found=NULL;
	int exists;

	//exists=FindPattern(new_pat.string,found,patterns);
	exists=FindPattern(new_pat.string,found);

	//doesn't exist, allocate a new node
	if(!exists)
	{
		if(!found) //this is the head
		{
			found=new NGRAM;
			patterns=found;
		}

		//less than, add to left
		else if(strcmp(new_pat.string,found->string)<0)
		{
			found->left=new NGRAM;
			found=found->left;
		}

		else //greater than, add to right
		{
			found->right=new NGRAM;
			found=found->right;
		}

		//copy to node & set children to null
		memcpy(found,&new_pat,sizeof(NGRAM));
		
		//allocate & copy position array
		found->pos_size=new_pat.freq;
		found->positions=new int[found->pos_size];
		memcpy(found->positions,new_pat.positions,new_pat.freq*sizeof(int));
		//found->positions[0]=new_pat.positions[0];
		found->left=found->right=NULL;
		num_patterns++;
	}

	//found pattern
	else if(inc_freq)
	{
		//must reallocate positions array
		if(found->freq>=found->pos_size)
		{
			int *temp=found->positions;

			found->pos_size+=10;
			found->positions=new int[found->pos_size];
			memcpy(found->positions,temp,found->freq*sizeof(int));

			delete[] temp;
		}
		
		//skip if position already logged
		for(int cur_pos=0; cur_pos<found->freq; cur_pos++)
			if(found->positions[cur_pos]==new_pat.positions[0])
				return num_patterns;
		
		//add new position
		found->positions[found->freq]=new_pat.positions[0];
		found->freq++;
	}

	return num_patterns;
}

//get the specified pattern
int Message::GetPattern(NGRAM *find_pat)
{
	NGRAM *found;

	if(FindPattern(find_pat->string,found))
	{
		memcpy(find_pat,found,sizeof(NGRAM));
		return 1;
	}

	return 0;
}

//call print_func for each pattern node, return number printed
long Message::ForAllPatterns(NGRAM *cur_pat, int min_freq, void (*do_func)(NGRAM*)=NULL)
{
	int done=0;

	if(!cur_pat) return 0;

	done+=ForAllPatterns(cur_pat->left,min_freq,do_func);
	done+=ForAllPatterns(cur_pat->right,min_freq,do_func);

	if(cur_pat->freq>min_freq || (cur_pat->length>2 && cur_pat->freq>(min_freq-1))) 
	{
		if(do_func) do_func(cur_pat);
		done++;
	}

	return done;
}

//interface for print pattern
long Message::PrintPatterns(void (*print_func)(NGRAM*))
{
	return ForAllPatterns(patterns,(msg_len>>10)+2,print_func);
}

long Message::WritePatterns(NGRAM *cur_pat, int length)
{
	int done=0;

	if(!cur_pat) return 0;

	done+=WritePatterns(cur_pat->left,length);
	done+=WritePatterns(cur_pat->right,length);

	if(cur_pat->length==length)
	{
		fprintf(ngram_file,"%s : %i %f\n",cur_pat->string,cur_pat->freq,float(cur_pat->freq)/msg_len);
		done++;
	}

	return done;
}

void Message::PatternsToFile(const char *filename, int length)
{
	ngram_file=fopen(filename,"w");

	WritePatterns(patterns,length);

	fclose(ngram_file);
}

void Message::ClearPatterns(NGRAM *cur_pat)
{
	if(!cur_pat) return;
	
	ClearPatterns(cur_pat->left);
	ClearPatterns(cur_pat->right);
	
	if(cur_pat->positions) delete cur_pat->positions;
	delete cur_pat;
	
	if(cur_pat==patterns) patterns=NULL;
}


//check if pat1 & pat2 are more than 50% similar, put pattern string into pat3
inline int pat_match(const char *pat1, const char *pat2, char *pat_temp, int len)
{
	int diff=0, run=0;
	int end=len-1;

	//first & last characters must be the same
	if(pat1[0]!=pat2[0] || pat1[end]!=pat2[end]) return 0;
	
	//check other characters
	for(int chr=1; chr<end; chr++)
	{
		//different
		if(pat1[chr]!=pat2[chr]) {pat_temp[chr]=BLANK; diff++; run++;}
		else {pat_temp[chr]=pat1[chr]; run=0;} //same
		if(run>2) return 0; //fail if 3 characters in a row are different
	}
	
	if(!diff) return 0; //exactly the same
	if((len-diff)<=len>>1) return 0; //at least half of the characters are different
	
	//set outside characters in the template string
	pat_temp[0]=pat1[0];
	pat_temp[end]=pat1[end];
	pat_temp[len]='\0';	
	
	return 1;
}
void Message::FindPatterns(int do_near)
{
	int length;
	NGRAM pattern;

	//delete old tree
	if(patterns) ClearPatterns(patterns);
	patterns=NULL;
	num_patterns=0;

	//allocate position array
	pattern.pos_size=2;
	pattern.positions=new int[pattern.pos_size];

	//exact patterns
	for(length=2; length<=MAX_PAT_LEN; length++)
	{	
		for(int index=0; index<msg_len-length+1; index++)
		{
			memcpy(pattern.string,cipher+index,length);
			pattern.string[length]='\0';
			pattern.length=length;
			pattern.freq=1;
			pattern.positions[0]=index;
			AddPattern(pattern,true);
		}
	}
	
	//near patterns
	if(do_near && msg_len<5000)	
	{
		for(length=4; length<=MAX_PAT_LEN; length++)
		{
			pattern.length=length;
			pattern.freq=2;
			
			for(int index1=0; index1<msg_len-length+1; index1++)
			{
				for(int index2=index1+length; index2<msg_len-length; index2++)
					if(pat_match(cipher+index1,cipher+index2,pattern.string,length))
					{
						pattern.positions[0]=index1;
						pattern.positions[1]=index2;
						AddPattern(pattern,true);
					}
			}
		}
	}

	good_pat=PrintPatterns(0);

	delete[] pattern.positions;
}

long Message::LetterGraph(wchar *dest)
{
	int max=0, step, cur_letter, rows=0;
	int dest_index=0, act_freq[26];
	char level[64];
	
	Decode();
	GetActFreq(act_freq);
	
	//find highest occurance
	for(cur_letter=0; cur_letter<26; cur_letter++)
	{
		if(exp_freq[cur_letter]>max) max=exp_freq[cur_letter];
		if(act_freq[cur_letter]>max) max=act_freq[cur_letter];
	}

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

		for(cur_letter=0; cur_letter<26; cur_letter++)
		{
			ustrcat(dest,0x0020);

			if(exp_freq[cur_letter]>=row) ustrcat(dest,UNI_LITEBAR);
			else ustrcat(dest,0x0020);

			if(act_freq[cur_letter]>=row) ustrcat(dest,UNI_FULLBAR);
			else ustrcat(dest,0x0020);
		}

		ustrcat(dest,0x0D);
		ustrcat(dest,0x0A);
		
		rows++;
	}

	//bottom line
	ustrcat(dest,"     ");

	for(cur_letter=0; cur_letter<=(26*3)+1; cur_letter++)	
		ustrcat(dest,UNI_HORZBAR);

	//letters
	ustrcat(dest,"\r\n      ");

	for(cur_letter=0; cur_letter<26; cur_letter++)
	{
		ustrcat(dest,0x0020);
		ustrcat(dest,cur_letter+'A');
		ustrcat(dest,0x0020);
	}

	//legend
	ustrcat(dest,"\r\n\r\nExpected ");
	ustrcat(dest,UNI_LITEBAR);
	ustrcat(dest,"     Actual   ");
	ustrcat(dest,UNI_FULLBAR);

	//rows in the high word, cols in the low
	return (rows+4)<<16 | ((26*3)+9);
}

//do coincidence counting on possible monoalphabets to find polyalphabet key size
long Message::PolyKeySize(wchar *dest, int max_len)
{
	int key_len, best_len, key_index, rows;
	float avg_ic, best_ic=0, max_ic=0;
	char level[64];
	
	if(max_len<=0 || max_len>=msg_len) return 0;
	
	//allocate column string
	char *ma=new char[msg_len+1];
	float *all_ic=new float[max_len+1];


	//get average ic for each key size
	for(key_len=2; key_len<=max_len; key_len++)
	{
		//average ics for each mono alphabet
		avg_ic=0;
		
		for(key_index=0; key_index<key_len; key_index++)
		{
			//add this column's IoC to average
			GetColumn(key_index,key_len,ma);
			avg_ic+=IoC(ma);
		}
		
		//average
		avg_ic/=key_len;
		all_ic[key_len]=avg_ic;
		
		if(avg_ic>max_ic) max_ic=avg_ic;
		
		//this average is the best so far
		if(CLOSER(avg_ic,best_ic,ENG_IOC))
		{
			best_ic=avg_ic;
			best_len=key_len;
		}
	}

	delete[] ma;
	
	//graph
	dest[0]=0;
	max_ic*=100;
	max_ic=ROUNDUP(max_ic);
	rows=int(max_ic*2);
	max_ic*=10;

	//line numbers and bars
	for(int row=(int)max_ic; row>0; row-=5)
	{
		sprintf(level,".%03i ",row);
		ustrcat(dest,level);
		ustrcat(dest,UNI_VERTBAR);

		for(key_len=2; key_len<=max_len; key_len++)
		{
			ustrcat(dest,0x20);
			if((all_ic[key_len]*1000)>=row) ustrcat(dest,UNI_FULLBAR);
			else ustrcat(dest,0x20);
			ustrcat(dest,0x20);
		}

		ustrcat(dest,0x0D);
		ustrcat(dest,0x0A);
	}

	//bottom line
	ustrcat(dest,"     ");

	for(key_len=2; key_len<(max_len)*3; key_len++)
		ustrcat(dest,UNI_HORZBAR);

	//key length
	ustrcat(dest,0x0D);
	ustrcat(dest,0x0A);
	ustrcat(dest,"     ");

	for(key_len=2; key_len<=max_len; key_len++)
	{
		ustrcat(dest,0x20);
		sprintf(level,"%2i",key_len);
		ustrcat(dest,level);
	}

	delete[] all_ic;

	return (rows+2)<<16 | (max_len*3)+5;
}

//calculate IoC for each row and column, and averages
long Message::RowColIoC(wchar *dest, int cols)
{
	int row, col, rows, lines, cur_rc;
	float *row_ic, *col_ic, row_avg=0, col_avg=0, max_ic=0;
	char *rc_string;
	char level[64];
	
	lines=msg_len/cols;
	if(msg_len%cols) lines++;

	row_ic=new float[lines];
	col_ic=new float[cols];

	rc_string=new char[(msg_len>>1)+1];
	
	//rows
	for(row=0; GetRow(row,cols,rc_string); row++)
	{
		row_avg+=row_ic[row]=IoC(rc_string);
		if(row_ic[row]>max_ic) max_ic=row_ic[row];
	}
	
	//rows average
	row_avg/=lines;
	
	//columns
	for(col=0; GetColumn(col,cols,rc_string); col++)
	{
		col_avg+=col_ic[col]=IoC(rc_string);
		if(col_ic[col]>max_ic) max_ic=col_ic[col];
	}
	
	//cols average
	col_avg/=cols;
	
	//graph
	dest[0]=0;
	max_ic*=100;
	max_ic=ROUNDUP(max_ic);
	rows=int(max_ic*2);
	max_ic*=10;

	//line numbers and bars
	for(row=(int)max_ic; row>0; row-=5)
	{
		sprintf(level,".%03i ",row);
		ustrcat(dest,level);
		ustrcat(dest,UNI_VERTBAR);

		for(cur_rc=0; cur_rc<lines; cur_rc++)
		{
			ustrcat(dest,0x20);
			if((row_ic[cur_rc]*1000)>=row) ustrcat(dest,UNI_FULLBAR);
			else ustrcat(dest,0x20);
			ustrcat(dest,0x20);
		}
		
		ustrcat(dest,UNI_VERTBAR);
		
		for(cur_rc=0; cur_rc<cols; cur_rc++)
		{
			ustrcat(dest,0x20);
			if((col_ic[cur_rc]*1000)>=row) ustrcat(dest,UNI_FULLBAR);
			else ustrcat(dest,0x20);
			ustrcat(dest,0x20);
		}

		ustrcat(dest,0x0D);
		ustrcat(dest,0x0A);
	}

	//bottom line
	ustrcat(dest,"     ");

	for(cur_rc=0; cur_rc<(lines+cols)*3+2; cur_rc++)
		ustrcat(dest,UNI_HORZBAR);

	//row/column
	ustrcat(dest,0x0D);
	ustrcat(dest,0x0A);
	ustrcat(dest,"     ");

	for(cur_rc=0; cur_rc<lines; cur_rc++)
	{
		ustrcat(dest,0x20);
		sprintf(level,"%2i",cur_rc+1);
		ustrcat(dest,level);
	}
	
	ustrcat(dest,0x20);
	
	for(cur_rc=0; cur_rc<cols; cur_rc++)
	{
		ustrcat(dest,0x20);
		sprintf(level,"%2i",cur_rc+1);
		ustrcat(dest,level);
	}
	
	
	delete[] row_ic;
	delete[] col_ic;
	delete[] rc_string;
	
	return (rows+2)<<16 | ((lines+cols+1)*3)+6;
}


void Message::HomophoneSet(char *msg, char letter, int avg_min, int avg_max, float freq_tol)
{
	int set_size;
	float avg_freq;
	char temp[512], set[128];
	SYMBOL symbol;

	letter-='A';

	msg[0]='\0';

	for(int num_homo=2; num_homo<10; num_homo++)
	{
		//average frequency for homophones
		avg_freq=float(exp_freq[letter])/num_homo;

		//skip if average is not within bounds
		if(!IS_BETWEEN(avg_freq,avg_min,avg_max)) continue;

		//find set
		set_size=0;

		for(int cur_symbol=0; cur_symbol<cur_map.GetNumSymbols(); cur_symbol++)
		{
			cur_map.GetSymbol(cur_symbol,&symbol);

			if(CLOSE_TO(symbol.freq,avg_freq,freq_tol))
			{
				set[set_size]=symbol.cipher;
				set_size++;
			}
		}
		
		set[set_size]='\0';

		sprintf(temp,"N - %i A - %.2f\t%s\n\n",num_homo,avg_freq,set); 
		strcat(msg,temp);
	}
}