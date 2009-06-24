#pragma warning( disable : 4244)	// STOP MSVS2005 WARNINGS
#pragma warning( disable : 4311)
#pragma warning( disable : 4267)
#pragma warning( disable : 4305)

#include "headers/message.h"

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

	AllocateBuffers(size);

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
	
	SetInfo(true);

	return msg_len;
}
/*
int Message::Read(const char *filename)
{
	FILE *msgfile;
	int size;
	char cur;

	if(!(msgfile=fopen(filename,"rb"))) return 0;

	//get file size
	fseek(msgfile,0,SEEK_END);
	size=ftell(msgfile)+1;
	fseek(msgfile,0,SEEK_SET);

	AllocateBuffers(size);

	if(!cipher || !plain) return 0;

	//read from file
	msg_len=size;

	fread(cipher,1,msg_len,msgfile);

	cipher[msg_len]='\0';
	fclose(msgfile);
	
	SetInfo(true);

	return msg_len;
}
*/
int Message::ReadNumeric(const char *filename)
{
	FILE *msgfile;
	int size, iNumber;
	char number[10];
	int unicount=33, uniques[10000];

	memset(uniques,0,4*10000);

	if(!(msgfile=fopen(filename,"r"))) return 0;

	//get file size
	fseek(msgfile,0,SEEK_END);
	size=ftell(msgfile)+1;
	fseek(msgfile,0,SEEK_SET);

	AllocateBuffers(size);

	if(!cipher || !plain) return 0;

	//read from file
	msg_len=0;

	while(fscanf(msgfile,"%s",number)!=EOF)
	{
		iNumber=atoi(number);

		if(iNumber < 0 || iNumber > 9999) continue;
		
		if(uniques[iNumber]==0) 
		{
			uniques[iNumber]=unicount;
			cipher[msg_len++]=unicount;
			unicount++;
		}

		else cipher[msg_len++]=uniques[iNumber];
		
		if(unicount>254) break;
	}

	cipher[msg_len]='\0';
	fclose(msgfile);
	
	SetInfo(true);

	return msg_len;
}

int Message::Write(const char *filename)
{
	FILE *msgfile;
	
	if(!(msgfile=fopen(filename,"w"))) return 0;
	
	fputs(cipher,msgfile);	
	fclose(msgfile);
	
	return 1;
}

void Message::SetExpFreq() //set expected frequencies
{
	for(int letter=0; letter<26; letter++)
		exp_freq[letter]=ROUNDTOINT((cur_map.GetUnigraph(letter)/100)*msg_len);
}

//set all info for new cipher data
void Message::SetInfo(int set_maps)
{
	SYMBOL symbol;
	DIGRAPH digraph;
	int chr;
	char sym_a=0, sym_b=0;

	SetExpFreq(); //expected letter frequencies
	FindPatterns(true); //patterns

	if(!set_maps) {Decode(); return;}
	
	memset(&symbol,0,sizeof(SYMBOL)); //monograph map
	cur_map.Clear(CLR_ALL);
	symbol.plain='\0';
	
	for(chr=0; chr<msg_len; chr++)
	{
		symbol.cipher=cipher[chr];
		cur_map.AddSymbol(symbol,1);
	}
	
	memset(&digraph,0,sizeof(DIGRAPH)); //digraph map
	digraph_map.Clear(CLR_ALL);
	digraph.plain1=digraph.plain2='\0';

	for(chr=0; chr<msg_len; chr++)
	{
		//if(!IS_UPPER_LTR(cipher[chr])) continue;
		if(!sym_a) sym_a=cipher[chr];
		else
		{
			sym_b=cipher[chr];

			digraph.cipher1=sym_a;
			digraph.cipher2=sym_b;
			digraph_map.AddDigraph(digraph,1);
			sym_a=sym_b=0;
		}
	}

	cur_map.SortByFreq(); //sort maps
	digraph_map.SortByFreq();

	for(chr=0; chr<msg_len-1; chr++) cur_map.AddContact(cipher[chr],cipher[chr+1]); //contact analysis

	Decode();
}

//put the specifed row in dest, according to the specifiec line length 
int Message::GetRow(int row, int line_len, char *dest)
{
	int row_len, row_start;
	
	//find start
	row_start=row*line_len;
	row_len=line_len;

	if(row_start>=msg_len) return 0; //would start beyond the end of buffer

	if(row_start+row_len>msg_len) row_len=msg_len-row_start; //not enough left for a complete line
	
	memcpy(dest,cipher+row_start,row_len);
	dest[row_len]='\0';
	
	return row_len;
}

//put the specifed column in dest, according to the specifiec line length 
int Message::GetColumn(int col, int line_len, char *dest)
{
	int row, msg_index;
	
	if(col>=line_len) return 0; //columng greater than line length
	
	msg_index=col; //get character at the same index of every row possible
	
	for(row=0; msg_index<msg_len; row++)
	{
		dest[row]=cipher[msg_index];
		msg_index+=line_len;
	}
	
	dest[row]='\0';
	
	return row;
}

void Message::SetTableuAlphabet(char *key_alpha)
{
	if(strlen(key_alpha)<26) strcpy(key_alpha,"ABCDEFGHIJKLMNOPQRSTUVWXYZ"); 

	for(int index=0; index<26; index++) 
	{
		for(int index2=0; index2<26; index2++)
			vigenere_array[index][index2]=key_alpha[((index2+index)%26)];
		
		vigenere_array[index][26]='\0';
	}
}

void Message::GetExpFreq(int *freq)
{
	memcpy(freq,exp_freq,26*sizeof(int));
}

void Message::GetActFreq(int *freq)
{
	memset(freq,0,26*sizeof(int));
	
	for(int chr=0; chr<msg_len; chr++)
	{
		if(IS_LOWER_LTR(plain[chr])) freq[(unsigned char)plain[chr]-'a']++; 
		else if(IS_UPPER_LTR(plain[chr]))  freq[(unsigned char)plain[chr]-'A']++; 
	}
}


//cipher modifing functions

void Message::SetCipher(const char *new_cipher)
{
	int new_len=(int)strlen(new_cipher);
	
	if(new_len>msg_len) //realloacte buffers 
	{
		DeleteBuffers(); 
		AllocateBuffers(new_len);
	}

	msg_len=new_len;

	strcpy(cipher,new_cipher);
	SetInfo(true);
}

void Message::Insert(int position, const char *string)
{
	int length, sym_index;
	SYMBOL symbol;
	
	length=(int)strlen(string);
	
	for(int cur_char=0; cur_char<length; cur_char++)
	{
		sym_index=cur_map.FindByCipher(cipher[position+cur_char]);
		cur_map.GetSymbol(sym_index,&symbol);
		symbol.plain=string[cur_char];
		cur_map.SetLock(sym_index,false);
		cur_map.AddSymbol(symbol,false);
		cur_map.SetLock(sym_index,true);
	}

	SetInfo();
}

void Message::Flip(int flip_dir, int row_len)
{
	unsigned long *xfm=new unsigned long[msg_len<<1];
	int num_xfm=0;

	if(flip_dir == 3) Reverse(cipher);
		
	else 
	{
		if(msg_len % row_len)
		{
		//	MessageBox(hMainWnd,"Cipher MUST be rectangular to perform flip!","Cipher Transform Status",MB_ICONINFORMATION);
			delete[] xfm;
			return;
		} 

		if(flip_dir == 1) FlipHorz(xfm,num_xfm,msg_len,row_len);
		if(flip_dir == 2) FlipVert(xfm,num_xfm,msg_len,row_len);

		Transform(cipher,xfm,num_xfm);
	}

	delete[] xfm;
	SetInfo(true);
}

void Message::RotateString(char *string, int row_len, int direction)
{
	int row, col, lines;

	if(msg_len<row_len) row_len=msg_len;

	lines=msg_len/row_len;

	for(row=0; row<lines; row++)
		for(col=0; col<row_len; col++)
			if(direction) msg_temp[col*lines+(lines-row-1)]=string[row*row_len+col]; //right
			else msg_temp[(row_len-col-1)*lines+row]=string[row*row_len+col]; //left
	
	msg_temp[msg_len]='\0';
	strcpy(string,msg_temp);
}

int Message::Rotate(int row_len, int direction)
{
	if(msg_len % row_len) return 0;

	RotateString(cipher,row_len,direction);
	SetInfo(); 
	return 1;
}

void SwapStringColumns(char *string, int iColumnA, int iColumnB, int iLineChars)
{
	int length=(int)strlen(string);
	int iLines=length/iLineChars;

	if(iColumnA>=iLineChars) iColumnA=0;
	if(iColumnB>=iLineChars) iColumnB=0;

	for(int iRow=0; iRow<iLines; iRow++)
	{
		int iRowIndex=(iRow*iLineChars);
		std::swap(string[iRowIndex+iColumnA],string[iRowIndex+iColumnB]);
	}
}

void SwapStringRows(char *string, int iRowA, int iRowB, int iLineChars)
{
	int length=(int)strlen(string);
	int iLines=length/iLineChars;

	if(iRowA>=iLines) iRowA=0;
	if(iRowB>=iLines) iRowB=0;

	int iRowAIndex=(iRowA*iLineChars);
	int iRowBIndex=(iRowB*iLineChars);

	for(int iColumn=0; iColumn<iLineChars; iColumn++)
		std::swap(string[iRowAIndex+iColumn],string[iRowBIndex+iColumn]);
}


void Message::SwapColumns(int iColumnA, int iColumnB, int iLineChars)
{
	SwapStringColumns(cipher,iColumnA,iColumnB,iLineChars);
	SwapStringColumns(plain,iColumnA,iColumnB,iLineChars);
	SetInfo();
}

void Message::SwapRows(int iRowA, int iRowB, int iLineChars)
{
	SwapStringRows(cipher,iRowA,iRowB,iLineChars);
	SwapStringRows(plain,iRowA,iRowB,iLineChars);
	SetInfo();
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
int Message::Simplify(char *dest)
{
	int old_patterns, num_symbols;
	char temp[32];
	//long *best_merge;
	int num_best=0, increase;
	Message test_msg;
	SYMBOL symbol1, symbol2;
	StringArray best_list;
	
	//only use exact patterns
	FindPatterns(false);
	old_patterns=good_pat;
	num_symbols=cur_map.GetNumSymbols();
	
	//find the best of all possible sustitutions 
	for(int cur_sym1=0; cur_sym1<num_symbols-1; cur_sym1++)
	{
		if(cur_map.GetLock(cur_sym1)) continue;
		cur_map.GetSymbol(cur_sym1,&symbol1);

		for(int cur_sym2=cur_sym1+1; cur_sym2<num_symbols; cur_sym2++)
		{
			test_msg+=*this;

			if(cur_map.GetLock(cur_sym2)) continue;
			cur_map.GetSymbol(cur_sym2,&symbol2);

			test_msg.MergeSymbols(symbol1.cipher,symbol2.cipher,false);
			
			//test for possibility
			increase=test_msg.good_pat-old_patterns;
			if(increase<3) continue;
			
			sprintf(temp,"%02i - %c %c",test_msg.good_pat-old_patterns,symbol1.cipher,symbol2.cipher);
			best_list.AddString(temp);
		}
	}

	dest[0]='\0';
	best_list.SortStrings(1);
	
	for(int cur_best=0; cur_best<best_list.GetNumStrings(); cur_best++)
	{
		best_list.GetString(cur_best,temp);
		strcat(dest,temp);
		strcat(dest,"\r\n");
	}
	
	FindPatterns(true);

	return best_list.GetNumStrings();
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
		found->left=found->right=NULL;
		num_patterns++;
	}

	//found pattern
	else if(inc_freq)
	{
		//skip if position already logged
		for(int cur_pos=0; cur_pos<found->freq; cur_pos++)
			if(found->positions[cur_pos]==new_pat.positions[0])
				return num_patterns;
				
		//must reallocate positions array
		if(found->freq>=found->pos_size)
		{
			int *temp=found->positions;

			found->pos_size+=10;
			found->positions=new int[found->pos_size];
			memcpy(found->positions,temp,found->freq*sizeof(int));

			delete[] temp;
		}
		
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
	for(length=min_pat_len; length<=MAX_PAT_LEN; length++)
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
				for(int index2=index1+length; index2<msg_len-length && num_patterns<10000; index2++)
					if(pat_match(cipher+index1,cipher+index2,pattern.string,length))
					{
						pattern.positions[0]=index2;
						pattern.positions[1]=index1;
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
long Message::PolyKeySize(wchar *dest, int max_len, float lang_ic)
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
			avg_ic+=IoC(ma,(int)strlen(ma));
		}
		
		//average
		avg_ic/=key_len;
		all_ic[key_len]=avg_ic;
		
		if(avg_ic>max_ic) max_ic=avg_ic;
		
		//this average is the best so far
		if(CLOSER(avg_ic,best_ic,lang_ic))
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
	
	if(cols==0) return 0; //division by 0 is bad 
	lines=msg_len/cols;
	if(msg_len%cols) lines++;

	row_ic=new float[lines];
	col_ic=new float[cols];

	rc_string=new char[(msg_len>>1)+1];
	
	//rows
	for(row=0; GetRow(row,cols,rc_string); row++)
	{
		row_avg+=row_ic[row]=IoC(rc_string,(int)strlen(rc_string));
		if(row_ic[row]>max_ic) max_ic=row_ic[row];
	}
	
	//rows average
	row_avg/=lines;
	
	//columns
	for(col=0; GetColumn(col,cols,rc_string); col++)
	{
		col_avg+=col_ic[col]=IoC(rc_string,(int)strlen(rc_string));
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

long Message::SeqHomo(wchar *dest, char *clip, float occur_pcnt, int max_len)
{
	int cur_symbol, symbol_a, symbol_b, num_symbols, str_len;
	StringArray string_set1, string_set2, string_set4;
	char temp[1024];
	char *last_pos, *cur_pos;
	SYMBOL symbol;
	int rows=0, cols=10;

	num_symbols=cur_map.GetNumSymbols();
	cur_map.GetSymbol(0,&symbol);

	dest[0]='\0';
	if(clip) clip[0]='\0';

	//make strings for possible homophone sets
	//symbols that occur TOL% of the time between the current symbol
	//are it's possible homophones
	for(cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
	{
		cur_map.GetSymbol(cur_symbol,&symbol);

		//get strings between instances of this symbol
		last_pos=strchr(cipher,symbol.cipher);

		while(cur_pos=strchr(last_pos+1,symbol.cipher))
		{
			//add this string
			str_len=cur_pos-last_pos;
			memcpy(temp,last_pos,str_len);
			temp[str_len]='\0';
			string_set1.AddString(temp);

			//setup for next position
			last_pos=cur_pos;
		}

		//get symbols that are in all strings
		string_set1.Intersect(temp,occur_pcnt);
		string_set1.Clear();
		string_set2.AddString(temp);
		
		if(!temp[1]) continue;
		
		//if(clip) {strcat(clip,temp); strcat(clip,"\n");}
	}

	//make strings for symbols that both say are possible homophones of each other
	//i.e. throw out all symbols in possible sets that do not have a corralary
	char str_a[256], str_b[256], str_c[256];

	for(symbol_a=0; symbol_a<num_symbols; symbol_a++)
	{
		string_set2.GetString(symbol_a,str_a);
		str_c[0]=str_a[0];
		str_len=1;
		
		if(int(strlen(str_a))>max_len) continue;
		
		for(symbol_b=1; symbol_b<(int)strlen(str_a); symbol_b++)
		{
			cur_symbol=cur_map.FindByCipher(str_a[symbol_b]);
			string_set2.GetString(cur_symbol,str_b);
			
			if(int(strlen(str_b))>max_len) continue;
			
			if(strchr(str_b,str_a[0]))
				if(!strchr(str_c,str_a[symbol_b]))
					str_c[str_len++]=str_a[symbol_b];	
		}

		str_c[str_len]='\0';

		if(str_len>=2 && str_len<=10) string_set4.AddString(str_c);
	}
	
	//sort and reduce homophone sets
	//remove duplicate strings after sorting
	for(cur_symbol=0; cur_symbol<string_set4.GetNumStrings(); cur_symbol++)
		string_set4.SortString(cur_symbol);
	
	string_set4.RemoveDups();
	string_set4.SortStrings(0);
	
	//make display string
	for(cur_symbol=0; cur_symbol<200; cur_symbol++)
	{
		if(!string_set4.GetString(cur_symbol,temp)) break;
		
		if(cur_symbol) ustrcat(dest,"\r\n");
		ustrcat(dest,temp);
		rows++;
	}
	
	return (rows+2)<<16 | (cols*8)+6;
}

void Message::AutoExclude()
{
	SYMBOL symbol, *symbol_ptr;
	int cur_contact, num_contacts;
	char szExclude[512], szFind[8];
	float freq, contacts_avg_freq;
	int num_symbols=cur_map.GetNumSymbols();

	float very_hi_freq_th=.06;	//very high letter frequency %
	float hi_freq_th=.04;		//high letter frequency %
	float lo_freq_th=.02;		//low letter frequency %
	float contact_freq_th=.058;	//lowest average frequency of contact symbols 
	float contact_diff_th=.35;	//lowest % diffferce of # of preceeding & following letters
	char vowel_exc[]="BCDFGHJKLMNPQRSTVWXZ"; //exclusions for different categories
	char vhfreq_exc[]="FGPWY";
	char hfreq_exc[]="BJKQVXZ";
	char double_exc[]="ABGIJKQUVWXYZ";
		
	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
	{
		cur_map.GetSymbol(cur_symbol,&symbol);
		strcpy(szExclude,symbol.exclude);

		freq= float(symbol.freq)/msg_len;

		if(freq >= very_hi_freq_th) strcat(szExclude,vhfreq_exc); //very high frequency
		if(freq >= hi_freq_th) strcat(szExclude,hfreq_exc);	 //high frequency

		sprintf(szFind,"%c%c",symbol.cipher,symbol.cipher);	 //doubles
		if(strstr(cipher,szFind)) strcat(szExclude,hfreq_exc);
			
		//vowel check: many contacts, many low freq contacts
		contacts_avg_freq=0;
		num_contacts=0;//symbol.num_precedes+symbol.num_follows;

		for(cur_contact=0; cur_contact<symbol.num_precedes; cur_contact++)
		{
			symbol_ptr=(SYMBOL*)symbol.precedes[cur_contact].symbol;
			contacts_avg_freq+=symbol_ptr->freq*symbol.precedes[cur_contact].freq;
			num_contacts+=symbol.precedes[cur_contact].freq;
		}

		for(cur_contact=0; cur_contact<symbol.num_follows; cur_contact++)
		{
			symbol_ptr=(SYMBOL*)symbol.follows[cur_contact].symbol;
			contacts_avg_freq+=symbol_ptr->freq*symbol.precedes[cur_contact].freq;
			num_contacts+=symbol.precedes[cur_contact].freq;
		}

		contacts_avg_freq/=num_contacts;
		contacts_avg_freq/=msg_len; //average frequency of symbols this contacts with

		if(freq >= lo_freq_th) //frequency is not very low
			if(contacts_avg_freq<contact_freq_th) //average frequency of contacts is lower than consonents
				if(ABS(symbol.num_precedes-symbol.num_follows)<contact_diff_th*(symbol.num_precedes+symbol.num_follows)/2) //lower difference in # of preceeding & folling letters
					strcat(szExclude,vowel_exc); //add vowel exclusions
			
		GetUniques(szExclude,symbol.exclude,NULL);
		cur_map.AddSymbol(symbol,false);
	}
}


//decode cipher into plain
void Message::DecodeHomo()
{
	int cur_symbol;
	char decoder[256];
//	SYMBOL symbol;

	cur_map.GetDecoder(decoder);

	for(cur_symbol=0; cur_symbol<msg_len; cur_symbol++)
		plain[cur_symbol]=decoder[(unsigned char)cipher[cur_symbol]];

	plain[msg_len]='\0';
}

void Message::DecodeDigraphic()
{
	int plain_index=0, cur_digraph, num_digraphs, decoder_index;;
	short decoder[65536];
	unsigned char sym_a, sym_b;
	DIGRAPH digraph;

	num_digraphs=digraph_map.GetNumDigraphs();

	for(cur_digraph=0; cur_digraph<num_digraphs; cur_digraph++) //create decoder
	{
		if(!digraph_map.GetDigraph(cur_digraph,&digraph)) continue;

		decoder_index=((unsigned char)digraph.cipher1*26)+((unsigned char)digraph.cipher2);
		
		sym_a=sym_b=BLANK;
		if(digraph.plain1) sym_a=digraph.plain1;
		if(digraph.plain2) sym_b=digraph.plain2;
		decoder[decoder_index]=sym_a<<8 | sym_b;
	}

	sym_a=sym_b=0;

	for(int index=0; index<msg_len; index++) //decode
	{
		//if(!IS_UPPER_LTR(cipher[index])) {plain[plain_index++]=cipher[index]; continue;}
		if(!sym_a) sym_a=cipher[index];
		else
		{
			sym_b=cipher[index]; 
	
			decoder_index=((unsigned char)sym_a*26)+((unsigned char)sym_b);
			plain[plain_index++]=decoder[decoder_index]>>8 & 0xFF;
			plain[plain_index++]=decoder[decoder_index] & 0xFF;

			sym_a=sym_b=0;
		}
	}

	plain[msg_len]='\0';
}

//create digraph_map to playfair key square
void Message::DecodePlayfair()
{
	int cur_digraph, num_digraphs;
	int c1X, c1Y, c2X, c2Y;
	int p1X, p1Y, p2X, p2Y;
	DIGRAPH digraph;

	num_digraphs=digraph_map.GetNumDigraphs();

	for(cur_digraph=0; cur_digraph<num_digraphs; cur_digraph++)
	{
		digraph_map.GetDigraph(cur_digraph,&digraph);

		if(!FindPolybiusIndex(5,digraph.cipher1,c1Y,c1X)) continue;
		if(!FindPolybiusIndex(5,digraph.cipher2,c2Y,c2X)) continue;
		
		if(c1Y==c2Y)		{p1Y=p2Y=c1Y; p1X=(c1X>0? c1X-1:4); p2X=(c2X>0? c2X-1:4);} //same row, one left
		else if(c1X==c2X)	{p1X=p2X=c1X; p1Y=(c1Y>0? c1Y-1:4); p2Y=(c2Y>0? c2Y-1:4);} //same column, one up
		else				{p1X=c2X; p1Y=c1Y; p2X=c1X; p2Y=c2Y;} //square, opposite corners
		
		digraph.plain1=polybius5a[p1Y*5+p1X]; //set digraph plain letters
		digraph.plain2=polybius5a[p2Y*5+p2X];
		digraph_map.AddDigraph(digraph,0);
	}

	DecodeDigraphic();
}

void Message::DecodeDoublePlayfair()
{
	int index, half_len, block_start=0, block_index=0, full_block_size=block_size<<1;
	int c1X, c1Y, c2X, c2Y;
	int p1X, p1Y, p2X, p2Y;
	DIGRAPH digraph;

	half_len=msg_len>>1;
	memset(plain,BLANK,msg_len);
	plain[msg_len]='\0';

	for(index=0; index<half_len; index++)
	{
		digraph.cipher1=cipher[index<<1];
		digraph.cipher2=cipher[(index<<1)+1];

		for(int decode=0; decode<2; decode++) //two decodings of digraph
		{
			if(!FindPolybiusIndex(7,digraph.cipher1,c1Y,c1X)) continue;
			if(!FindPolybiusIndex(5,digraph.cipher2,c2Y,c2X)) continue;

			if(c1Y==c2Y)		{p1Y=p2Y=c1Y; p2X=(c1X<4? c1X+1:0); p1X=(c2X<4? c2X+1:0);} //same row, one left
			else				{p1X=c2X; p1Y=c1Y; p2X=c1X; p2Y=c2Y;} //square, opposite corners

			digraph.cipher1=polybius5a[p1Y*5+p1X]; 
			digraph.cipher2=polybius5b[p2Y*5+p2X];
		}

		plain[block_start+block_index]=digraph.cipher1; //set plain
		plain[block_start+block_index+(full_block_size>>1)]=digraph.cipher2;

		if(++block_index>=(full_block_size>>1))
		{
			block_index=0;
			block_start+=full_block_size;
			if(msg_len-block_start<full_block_size) //adjust for last block
				full_block_size=msg_len-block_start;
		}
	}
}


void Message::DecodeVigenere(char *string) //any tableau can be used
{
	int iCipherIndex, iKeyIndex=0, iCipherCol, iKeyRow;
	char *lpcCipherInKeyRow;

	if(!vig_key_len) return;

	for(iCipherIndex=0; iCipherIndex<msg_len; iCipherIndex++)
	{
		if(!strchr(vigenere_array[0],string[iCipherIndex])) //invalid character
		{
			msg_temp[iCipherIndex]=string[iCipherIndex];
			continue;
		}

		//find key row
		for(iKeyRow=0; iKeyRow<26; iKeyRow++)
			if(vigenere_array[iKeyRow][0]==key[iKeyIndex]) break;

		//key character not found
		if(iKeyRow>25) {if(++iKeyIndex>=vig_key_len) iKeyIndex=0; iCipherIndex--; continue;}

		//find cipher column
		lpcCipherInKeyRow=strchr(vigenere_array[iKeyRow],string[iCipherIndex]);
		if(!lpcCipherInKeyRow) continue;

		iCipherCol=int(lpcCipherInKeyRow-vigenere_array[iKeyRow]);

		//get plain text character in key row 0
		msg_temp[iCipherIndex]=vigenere_array[0][iCipherCol];
	
		if(++iKeyIndex>=vig_key_len) iKeyIndex=0;
	}

	msg_temp[msg_len]='\0';

	strcpy(plain,msg_temp);
}


char LETTER_INDEXS2[256]={
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
/*
void Message::DecodeVigenere(char *string) //any tableau can be used
{
	int iCipherIndex, iKeyIndex=0;
//	char *lpcCipherInKeyRow;

	char cCipher, cKey;
//	char cPlain;

	if(!vig_key_len) return;
	if(!strlen(key)) return;

	for(iCipherIndex=0; iCipherIndex<msg_len; iCipherIndex++)
	{
		cCipher=LETTER_INDEXS2[string[iCipherIndex]];
		cKey=LETTER_INDEXS2[key[iKeyIndex]];

		if(cCipher<0) {msg_temp[iCipherIndex]=string[iCipherIndex]; continue;} //invalid character

		if(cKey<0) {if(++iKeyIndex>=vig_key_len) iKeyIndex=0; iCipherIndex--; continue;}

		//cPlain=(cCipher - cKey);
		//if(cPlain<0) cPlain=26+cPlain;

		//cPlain=cCipher 0x0D;

//		msg_temp[iCipherIndex]=cPlain+'A';
	
		if(++iKeyIndex>=vig_key_len) iKeyIndex=0;
	}

	msg_temp[msg_len]='\0';

	FlipPlainBuffer();
}
*/
int Message::FindPolybiusIndex(int poly_size, char symbol, int &x, int &y)
{
	char *polybius;

	switch(poly_size)
	{
		case 5: polybius=polybius5a; break;
		case 7: polybius=polybius5b; poly_size=5; break;
		case 6: polybius=polybius6; break;
		case 8: polybius=polybius8; break;
		default: return 0;
	}

	for(x=0; x<poly_size; x++)
		for(y=0; y<poly_size; y++)
			if(symbol==polybius[poly_size*x+y]) return 1;

	/*c1=(int)strchr(polybius5a,digraph.cipher1);
		c2=(int)strchr(polybius5a,digraph.cipher2);

		if(!c1 || !c2) continue; //digraph contains an invalid character
		
		c1-=(int)polybius5a; c2-=(int)polybius5a; //cipher idexes into array
		c1Y=c1/5; c1X=c1%5; c2Y=c2/5; c2X=c2%5; //cipher x,y in square*/

	return 0;
}

int Message::FindTrifidIndex(char symbol, int &x, int &y, int &z)
{
	for(x=0; x<3; x++)
		for(y=0; y<3; y++)
			for(z=0; z<3; z++)
				if(symbol==trifid_array[9*x+3*z+y]) return 1;

	return 0;
}
	
void Message::DecodeXfid(int fract_size)
{
	int x,y,z, bFound, iBlockIndex=0, iCipherIndex, iPlainIndex=0, index_rw=0, org_block_size=block_size;
	char cCurSymbol, *index_string;

	//init plain string
	memset(plain,BLANK,msg_len);
	plain[msg_len]='\0';

	if(block_size<2) return;

	index_string=new char[block_size*fract_size];

	for(iCipherIndex=0; iCipherIndex<msg_len; iCipherIndex++)
	{
		cCurSymbol=cipher[iCipherIndex]; //find cipher symbol in array

		if(fract_size==2) bFound=FindPolybiusIndex(5,cCurSymbol,x,y);
		else bFound=FindTrifidIndex(cCurSymbol,x,y,z);

		if(!bFound) {plain[iCipherIndex]=cipher[iCipherIndex]; continue;}

		index_string[index_rw++]=x; index_string[index_rw++]=y; 
		if(fract_size==3) index_string[index_rw++]=z;

		iBlockIndex++;
		
		if(iCipherIndex==msg_len-1) block_size=iBlockIndex; //at end of cipher, do a short block
	
		if(iBlockIndex==block_size) //split string into sections and get plain indexes
		{
			for(iBlockIndex=0; iBlockIndex<block_size; iBlockIndex++)
			{
				while(iPlainIndex<msg_len && plain[iPlainIndex]!=BLANK) iPlainIndex++;
				if(iPlainIndex>=msg_len) break;

				x=index_string[iBlockIndex]; y=index_string[iBlockIndex+block_size];
				if(fract_size==2) plain[iPlainIndex++]=polybius5a[5*x+y];
				else {z=index_string[iBlockIndex+(block_size<<1)]; plain[iPlainIndex++]=trifid_array[9*x+3*z+y];}
			}

			iBlockIndex=index_rw=0;
		}
	}

	block_size=org_block_size;

	delete index_string;
}

void Message::DecodePermutation(char *key)
{
	int key_length, num_rows, temp_index;
	char temp_key[512];

	if(!(key_length=strlen(key))) return;

	strcpy(temp_key,key);
	RadixSort(temp_key);

	num_rows=msg_len/key_length; //number of rows in original matrix

	for(int key_index=0; key_index<key_length; key_index++) //swap where sorted key differs
	{
		if(key[key_index]==temp_key[key_index]) continue;
		temp_index=int(strchr(temp_key+key_index,key[key_index])-temp_key);
		std::swap(temp_key[key_index],temp_key[temp_index]);
		
		for(int iRow=0; iRow<num_rows; iRow++)
		{
			int iRowIndex=(iRow*key_length);
			std::swap(plain[iRowIndex+key_index],plain[iRowIndex+temp_index]);
		}
	}
}

void Message::ColumnarStage(char *key) //supports incomplete columnar
{
	int key_length, extra, num_rows, cur_row, cur_col, read_row, cipher_index, plain_index=0;
	char temp_key[512];

	if((key_length=strlen(key))<2) return;

	strcpy(temp_key,key);
	RadixSort(temp_key);

	extra=msg_len%key_length; //extra letters in incomplete matrix
	num_rows=msg_len/key_length; //number of rows in original matrix
	if(extra) num_rows++;

	for(int key_index=0; key_index<key_length; key_index++) //read columns in key order
	{
		if(key_index && temp_key[key_index]==temp_key[key_index-1]) cur_col+=ChrIndex(key+cur_col+1,temp_key[key_index])+1; //duplicate key symbol, find next column

		else cur_col=ChrIndex(key,temp_key[key_index]); //which column to read into

		for(cur_row=0; cur_row<num_rows; cur_row++)
		{
			if(trans_type) read_row=num_rows-cur_row-1; //if columns were decoded bottom to top
			else read_row=cur_row;	//normal top to bottom reading

			cipher_index=read_row*key_length+cur_col;
			if(cipher_index>=msg_len) continue; //last row in incomplete column
			msg_temp[read_row*key_length+cur_col]=plain[plain_index++];
		}

	}

	msg_temp[plain_index]='\0';
	FlipPlainBuffer();
}

void Message::DecodeColumnar(int stages) {for(int cur_stage=0; cur_stage<stages; cur_stage++) ColumnarStage(coltrans_key[stages-cur_stage-1]);}

void Message::DecodeADFGX(int square_size, char *polybius)
{
	if(square_size==6) POLYBIUS_INDEXS['X']=5;
	else POLYBIUS_INDEXS['X']=4;

	for(int index=0; index<msg_len; index+=2)
	{
		int x=POLYBIUS_INDEXS[plain[index]];
		int y=POLYBIUS_INDEXS[plain[index+1]];
		if(x>=square_size || y>=square_size || x<0 || y<0) plain[index]=plain[index+1]=BLANK;
		else plain[index>>1]=polybius[square_size*x+y];
	}

	plain[(msg_len>>1)]='\0';
}

 /*
#define BD_E3 0x01
#define BD_LF 0x02
#define BD_A- 0x03
#define BD_SP 0x04
#define BD_S' 0x05
#define BD_I8 0x06
#define BD_U7 0x07
#define BD_CR 0x08
#define BD_D$ 0x09
#define BD_R4 0x0A
#define BD_J' 0x0B
#define BD_N, 0x0C
#define BD_F!% 0x0D
#define BD_C: 0x0E
#define BD_K( 0x0F
#define BD_T5 0x10
#define BD_Z*+ 0x11
#define BD_L) 0x12
#define BD_W2 0x13
#define BD_H# 0x14
#define BD_Y6 0x15
#define BD_P0 0x16
#define BD_Q1 0x17
#define	BD_O9 0x18
#define BD_B? 0x19
#define BD_G&@ 0x1A
#define BD_FG 0x1B
#define BD_M. 0x1C
#define BD_X/ 0x1D
#define BD_V;= 0x1E
#define BD_LT 0x1F
*/

char ASCII2BAUDOT[256]={
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x05,0x04,0x04,0x02,0x04,0x04,0x08,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x0D,0x04,0x14,0x09,0x0D,0x1A,0x05,0x0F,0x12,0x11,0x11,0x0C,0x03,0x1C,0x1D,
0x16,0x17,0x13,0x01,0x0A,0x10,0x15,0x07,0x06,0x18,0x0E,0x1E,0x04,0x1E,0x04,0x19,
0x1A,0x03,0x19,0x0E,0x09,0x01,0x0D,0x1A,0x14,0x06,0x0B,0x0F,0x12,0x1C,0x0C,0x18,
0x16,0x17,0x0A,0x05,0x10,0x07,0x1E,0x13,0x1D,0x15,0x11,0x04,0x04,0x04,0x04,0x04,
0x04,0x03,0x19,0x0E,0x09,0x01,0x0D,0x1A,0x14,0x06,0x0B,0x0F,0x12,0x1C,0x0C,0x18,
0x16,0x17,0x0A,0x05,0x10,0x07,0x1E,0x13,0x1D,0x15,0x11,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x14,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04};

char BAUDOT2ASCII_LT[]={" E\nA SIU\rDRJNFCKTZLWHYPQOBG MXV "};
char BAUDOT2ASCII_FG[]={" 3\n- '87\r$4',%:(5+)2#6019?& ./= "};

void Ascii2Baudot(const char *szAscii, char *szBaudot)
{
	int ascii_len=strlen(szAscii);
	int ascii_index, baudot_index=0, lt_fg=0;

	for(ascii_index=0; ascii_index<ascii_len; ascii_index++)
	{
		char cAscii=szAscii[ascii_index];

		if(LETTER_INDEXS2[cAscii]==-1 && cAscii!='\r' && cAscii!='\n' && cAscii!=' ') 
			{if(lt_fg==0) szBaudot[baudot_index++]=0x1B; lt_fg=1;} //change to figure
		else {if(lt_fg==1) szBaudot[baudot_index++]=0x1F; lt_fg=0;} //change to letter
		
		szBaudot[baudot_index++]=ASCII2BAUDOT[cAscii];
	}

	szBaudot[baudot_index]='\0';
}

void Baudot2Ascii(const  char *szBaudot, char *szAscii)
{
	int baudot_len=strlen(szBaudot);
	int baudot_index, ascii_index=0, lt_fg=0;
	
	for(baudot_index=0; baudot_index<baudot_len; baudot_index++)
	{
		char cBaudot=szBaudot[baudot_index];

		if(cBaudot==0x1F) {lt_fg=0; continue;}
		if(cBaudot==0x1B) {lt_fg=1; continue;}

		if(lt_fg==0) szAscii[ascii_index++]=BAUDOT2ASCII_LT[cBaudot];
		else szAscii[ascii_index++]=BAUDOT2ASCII_FG[cBaudot];
	}

	szAscii[ascii_index]='\0';
}

void Message::DecodeLorenz()
{
	#define WHEEL_BIT(Wn,iWn) (Wn[iWn]=='x'? 1:0) 
	
	char K1[]="..xxxx.xx....xxx..x.x...x.xxxx.x.xxx...x."; //41
	char K2[]="..xx.xx..x..xx..xxxx.x...xxx..x"; //31
	char K3[]=".xx.xx..xx...xx...xxx.x..xx.."; //29
	char K4[]="..x....xxx.xx..xxxx..x..xx"; //26
	char K5[]="..x.x..x.xxxx....xx..xx"; //23
	char M1[]="...xxxx.xxx.xxxx.xxxx.xxxx.xxxx.xxx....xx.xxxx.xxx.xxxx.xxxx."; //61
	char M2[]="....x.x...x.x.x...x.x.x...x.x.x.x.x.x"; //37
	char S1[]="x.xx.x..x.x.x.x.x.xxxx.x.x.xxx..x.x.xx...x."; //43
	char S2[]="x..x.x.x.xx.x....x.xxx.x.xx.x..x.x.x..xxx.x.xx."; //47
	char S3[]="x.x.x..xx.x.x..x.x.xxx..x.x.x.xx.x.x....xxx..x.x.x."; //51
	char S4[]=".x.x.x.x.x.x..x.xx.x.x..x.x.xx.x...xx.x.x.xxxx...xx."; //53
	char S5[]=".x..x.xx...x.x.xxxx..xx.x.x.x.x.x.x.x.x...x.xx.x..x.xx...xx"; //59

	int iK1, iK2, iK3, iK4, iK5, iM1, iM2, iS1, iS2, iS3, iS4, iS5;

	iK1=lorenz[0]; iK2=lorenz[1]; iK3=lorenz[2]; iK4=lorenz[3]; iK5=lorenz[4];
	iM1=lorenz[5]; iM2=lorenz[6];
	iS1=lorenz[7]; iS2=lorenz[8]; iS3=lorenz[9]; iS4=lorenz[10]; iS5=lorenz[11];
	
	Ascii2Baudot(cipher,plain);
	int length=strlen(plain);

	for(int index=0; index<length; index++)
	{
		//char K=WHEEL_BIT(K5,iK5)<<4 | WHEEL_BIT(K4,iK4)<<3 | WHEEL_BIT(K3,iK3)<<2 | WHEEL_BIT(K2,iK2)<<1 | WHEEL_BIT(K1,iK1);
		//char S=WHEEL_BIT(S5,iS5)<<4 | WHEEL_BIT(S4,iS4)<<3 | WHEEL_BIT(S3,iS3)<<2 | WHEEL_BIT(S2,iS2)<<1 | WHEEL_BIT(S1,iS1);

		char K=WHEEL_BIT(K5,iK5) | WHEEL_BIT(K4,iK4)<<1 | WHEEL_BIT(K3,iK3)<<2 | WHEEL_BIT(K2,iK2)<<3 | WHEEL_BIT(K1,iK1)<<4;
		char S=WHEEL_BIT(S5,iS5) | WHEEL_BIT(S4,iS4)<<1 | WHEEL_BIT(S3,iS3)<<2 | WHEEL_BIT(S2,iS2)<<3 | WHEEL_BIT(S1,iS1)<<4;
		
		msg_temp[index]=plain[index] ^ K ^ S; if(!msg_temp[index]) msg_temp[index]=0x04; //xor with key

		if(M2[iM2]=='.') {iS1=(iS1+1)%43; iS2=(iS2+1)%47; iS3=(iS3+1)%51; iS4=(iS4+1)%53; iS5=(iS5+1)%59;} //step psi wheels
		if(M1[iM1]=='x') iM2=(iM2+1)%37; iM1=(iM1+1)%61; //step motor wheels
		iK1=(iK1+1)%41; iK2=(iK2+1)%31; iK3=(iK3+1)%29; iK4=(iK4+1)%26; iK5=(iK5+1)%23; //step chi wheels
	}

	msg_temp[length]='\0';

	msg_temp[6]==msg_temp[8];

	Baudot2Ascii(msg_temp,plain);
}

char elgar[2][25]={{"ABCDEFGHIKLMNOPQRSTUWXYZ"},{"NOPQRSTUWXYZABCDEFGHIKLM"}};

void Message::DecodeElgar()
{
	for(int index=0; index<msg_len; index++)
		for(int letter=0; letter<24; letter++)
			if(cipher[index]==elgar[0][letter]) 
				{cipher[index]=elgar[1][letter]; break;}

	SwapColumns(2,3,4); SwapColumns(0,2,4);
	SetInfo();
}
int Message::CalcBestWidth(int msg_len)
{
	int s, r, d, l;
	
	int in=0, jn=0;			// lowest difference dimensions for non-primes
	int ip=0, jp=0, rp=0;	// lowest difference dimensions and remainder for primes

	//special cases
	if(msg_len<25) return msg_len; //short
	if((!(msg_len%17)) && msg_len<210) return 17; //zodiac partials
	if(msg_len==32) return 17; //button cipher
	if(msg_len==88) return 29; //dorabella

	s = l = (int)sqrt((double)msg_len);

	for(int i=1;i<2*s;i++) {
		for(int j=2*s;j>0;j--) {
			r = abs(msg_len - i*j);
			d = abs(i - j);
			// non-primes: minimize diffs. between factors
			if((i*j == msg_len)&&(d < l)) { l = d; in = i; jn = j; }	
			// primes: minimize diffs. keeping remainder < sqrt(msg_len) 
			if((msg_len == i*j + r)&&(r < s)&&(d < l)) { l = d; ip = i; jp = j; rp = r; }
		}
	}
	if(in*jn == msg_len) return in; 
	else return ip;
}
