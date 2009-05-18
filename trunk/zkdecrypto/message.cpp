#pragma warning( disable : 4244)	// STOP MSVS2005 WARNINGS

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

int Message::ReadNumeric(const char *filename)
{
	FILE *msgfile;
	int size, iNumber;
	char number[10];
	int x, unicount=33, uniques[10000];

	memset(uniques,0,4*10000);

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
	
	SetInfo();

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

void Message::SetCipher(const char *new_cipher)
{
	msg_len=(int)strlen(new_cipher);
	
	if(cipher) delete[] cipher;
	cipher=new char[msg_len+1];

	strcpy(cipher,new_cipher);
	SetInfo();
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
}

void Message::SetExpFreq() //set expected frequencies
{
	for(int letter=0; letter<26; letter++)
		exp_freq[letter]=ROUNDTOINT((cur_map.GetUnigraph(letter)/100)*msg_len);
}

//set all info for new cipher data
void Message::SetInfo()
{
	SYMBOL symbol;
	DIGRAPH digraph;
	int chr;
	char sym_a=0, sym_b=0;
	
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
	
	SetExpFreq(); //expected letter frequencies
	FindPatterns(true); //patterns
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

void Message::InitArrays()
{
	if(strlen(bifid_array)!=25) strcpy(bifid_array,"ABCDEFGHIKLMNOPQRSTUVWXYZ");
	if(strlen(trifid_array)!=27) strcpy(trifid_array,"ABCDEFGHIJKLMNOPQRSTUVWXYZ.");
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
		freq[plain[chr]-'A']++;
}

void Message::Flip(int flip_dir, int row_len)
{
	unsigned long *xfm=new unsigned long[msg_len<<1];
	int num_xfm=0;

	if(flip_dir == 3)
	{
		Reverse(cipher);
		delete[] xfm;
		FindPatterns(true);
		return;
	}

	if(msg_len % row_len)
	{
	//	MessageBox(hMainWnd,"Cipher MUST be rectangular to perform flip!","Cipher Transform Status",MB_ICONINFORMATION);
		delete[] xfm;
		return;
	} 

	if(flip_dir == 1) FlipHorz(xfm,num_xfm,msg_len,row_len);
	if(flip_dir == 2) FlipVert(xfm,num_xfm,msg_len,row_len);

	Transform(cipher,xfm,num_xfm);

	delete[] xfm;
	FindPatterns(true);
}

int Message::Rotate(int row_len, int direction)
{
	char *rot=new char[msg_len+1];
	int row, col, lines;

	if(msg_len % row_len) {
		delete rot;
		return 0;
	}

	if(msg_len<row_len) row_len=msg_len;

	lines=msg_len/row_len;

	for(row=0; row<lines; row++)
		for(col=0; col<row_len; col++)
			if(direction) rot[col*lines+(lines-row-1)]=cipher[row*row_len+col];
			else rot[(row_len-col-1)*lines+row]=cipher[row*row_len+col];

	memcpy(cipher,rot,msg_len);
	delete rot;
	FindPatterns(true);
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
}

void Message::SwapRows(int iRowA, int iRowB, int iLineChars)
{
	SwapStringRows(cipher,iRowA,iRowB,iLineChars);
	SwapStringRows(plain,iRowA,iRowB,iLineChars);
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
				for(int index2=index1+length; index2<msg_len-length; index2++)
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


//decode cipher into plain
void Message::DecodeHomo()
{
	int cur_symbol, num_symbols;
	char decoder[256];
	SYMBOL symbol;

	num_symbols=cur_map.GetNumSymbols();

	//setup decoding table
	for(cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
	{
		cur_map.GetSymbol(cur_symbol,&symbol);
		if(symbol.plain) decoder[(unsigned char)symbol.cipher]=symbol.plain;
		else decoder[(unsigned char)symbol.cipher]=BLANK;
	}

	//decode string
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
	int c1, c2, c1X, c1Y, c2X, c2Y;
	int p1, p2, p1X, p1Y, p2X, p2Y;
	DIGRAPH digraph;

	num_digraphs=digraph_map.GetNumDigraphs();

	for(cur_digraph=0; cur_digraph<num_digraphs; cur_digraph++)
	{
		digraph_map.GetDigraph(cur_digraph,&digraph);

		c1=(int)strchr(bifid_array,digraph.cipher1);
		c2=(int)strchr(bifid_array,digraph.cipher2);

		if(!c1 || !c2) continue; //digraph contains an invalid character
		
		c1-=(int)bifid_array; c2-=(int)bifid_array; //cipher idexes into array
		c1Y=c1/5; c1X=c1%5; c2Y=c2/5; c2X=c2%5; //cipher x,y in square
		
		if(c1Y==c2Y)		{p1Y=p2Y=c1Y; p1X=(c1X>0? c1X-1:4); p2X=(c2X>0? c2X-1:4);} //same row, one left
		else if(c1X==c2X)	{p1X=p2X=c1X; p1Y=(c1Y>0? c1Y-1:4); p2Y=(c2Y>0? c2Y-1:4);} //same column, one up
		else				{p1X=c2X; p1Y=c1Y; p2X=c1X; p2Y=c2Y;} //square, opposite corners
		
		p1=p1Y*5+p1X; p2=p2Y*5+p2X; //set digraph plain letters
		digraph.plain1=bifid_array[p1];
		digraph.plain2=bifid_array[p2];
		digraph_map.AddDigraph(digraph,0);
	}

	DecodeDigraphic();
}

void Message::DecodeVigenere() //any tableau can be used
{
	int iCipherIndex, iKeyIndex=0, iCipherCol, iKeyRow;
	char *lpcCipherInKeyRow;

	if(!key_len) return;

	for(iCipherIndex=0; iCipherIndex<msg_len; iCipherIndex++)
	{
		if(!strchr(vigenere_array[0],cipher[iCipherIndex])) //invalid character
		{
			plain[iCipherIndex]=cipher[iCipherIndex];
			continue;
		}

		//find key row
		for(iKeyRow=0; iKeyRow<26; iKeyRow++)
			if(vigenere_array[iKeyRow][0]==key[iKeyIndex]) break;

		if(iKeyRow>25) {if(++iKeyIndex>=key_len) iKeyIndex=0; iCipherIndex--; continue;}

		//find cipher column
		lpcCipherInKeyRow=strchr(vigenere_array[iKeyRow],cipher[iCipherIndex]);
		if(!lpcCipherInKeyRow) continue;

		iCipherCol=int(lpcCipherInKeyRow-vigenere_array[iKeyRow]);

		//get plain text character in key row 0
		plain[iCipherIndex]=vigenere_array[0][iCipherCol];
	
		if(++iKeyIndex>=key_len) iKeyIndex=0;
	}

	plain[msg_len]='\0';
}

int Message::FindBifidIndex(char symbol, int &x, int &y)
{
	for(x=0; x<5; x++)
		for(y=0; y<5; y++)
			if(symbol==bifid_array[5*x+y])
				return 1;

	return 0;
}

int Message::FindTrifidIndex(char symbol, int &x, int &y, int &z)
{
	for(x=0; x<3; x++)
		for(y=0; y<3; y++)
			for(z=0; z<3; z++)
				if(symbol==trifid_array[9*x+3*z+y])
					return 1;

	return 0;
}
	
void Message::DecodeXfid(int fract_size)
{
	int x,y,z, bFound, iBlockIndex=0, iCipherIndex, iPlainIndex=0, index_rw=0, org_block_size=block_size;
	char cCurSymbol, *index_string;

	//init plain string
	memset(plain,'-',msg_len);
	plain[msg_len]='\0';

	if(block_size<2) return;

	index_string=new char[block_size*fract_size];

	for(iCipherIndex=0; iCipherIndex<msg_len; iCipherIndex++)
	{
		cCurSymbol=cipher[iCipherIndex]; //find cipher symbol in array

		if(fract_size==2) bFound=FindBifidIndex(cCurSymbol,x,y);
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
				while(iPlainIndex<msg_len && plain[iPlainIndex]!='-') 	iPlainIndex++;
				if(iPlainIndex>=msg_len) break;

				x=index_string[iBlockIndex]; y=index_string[iBlockIndex+block_size];
				if(fract_size==2) plain[iPlainIndex++]=bifid_array[5*x+y];
				else {z=index_string[iBlockIndex+(block_size<<1)]; plain[iPlainIndex++]=trifid_array[9*x+3*z+y];}
			}

			iBlockIndex=index_rw=0;
		}
	}

	block_size=org_block_size;

	delete index_string;
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