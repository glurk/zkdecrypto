//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This program attempts to solve homophonic ciphers                                                                                                        //
//                                                                                                                                                          //
// Big thanks to Chris McCarthy for many good ideas and saving a lot of work in converting the RayN and Zodiac 340 ciphers to ASCII                         //
// Also thanks to Glen from the ZK message board (http://www.zodiackiller.com/mba/zc/121.html) for an ASCII encoding of the solved 408 cipher.              //
//                                                                                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2008 Brax Sisco, Wesley Hopper, Michael Eaton
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along
//    with this program; if not, write to the Free Software Foundation, Inc.,
//    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#pragma warning( disable : 4267)
#pragma warning( disable : 4244)	// STOP MSVS2005 WARNINGS

#include "headers/z340.h"
#include "headers/z340Globals.h"
#include "headers/strarray.h"

//pointer to info struct in main module
SOLVEINFO *info;
void SetInfo(SOLVEINFO *main_info) {info=main_info;}

FILE *log_file;

int hillclimb(Message &msg, const char cipher[],int clength,char key[],int print)
{
	#define	DO_SWAP	{ unsigned char temp=key[p1]; key[p1]=key[p2]; key[p2]=temp; }
	#define DECODE {for(y=clength;y--;) solved[y]=*decoder[(unsigned char)cipher[y]];}

	int cuniq,keylength,i,j,x,y;
	int uniq[ASCII_SIZE],uniqarr[ASCII_SIZE];
	char *solved=new char[clength+1];
	char uniqstr[ASCII_SIZE];
	char *decoder[ASCII_SIZE];

	solved[clength]='\0'; 													
	for(i=0; i<ASCII_SIZE; i++) uniq[i]=uniqstr[i]=uniqarr[i]=0;					//INITIALIZE (ZERO) ARRAYS

	for(i=0; i<clength; i++) ++uniq[(unsigned char)cipher[i]];						//COUNT # OF UNIQUE CHARS IN CIPHER

	i=4096; j=0;																		//CALCULATE AND SORT THE CIPHER UNIQUES
	for(y=0;y<4096;y++) { 
		for(x=255;x>0;x--)
			{ if(uniq[x]==i) { uniqstr[j]=x; uniqarr[j++]=i; } } i--;}

	cuniq=(int)strlen(uniqstr);
	keylength=(int)strlen(key);
	
	if(keylength < cuniq)      //THIS SHOULD NEVER HAPPEN
		{ printf("\nKEYLENGTH ERROR!! -- Key is TOO SHORT\n\n"); return(-1); } 

	//if (print) printfrequency(clength,uniqarr,uniqstr,cuniq);
	
	//make decoder, array of char* that point to the key plain text values indexed by the ascii value of the cipher symbols
	for(x=0; x<cuniq; x++) decoder[(unsigned char)uniqstr[x]]=&key[x];
	DECODE;

/****************************** START_MAIN_HILLCLIMBER_ALGORITHM **********************************/

	int score = 0, last_score=0, improve=0, tolerance=0;
	long start_time=0, end_time=0;
	std::string key_str;

	//init info
	info->cur_tol=0;
	info->cur_tabu=0;
	info->tabu_end=info->tabu->end();
info->tabu_syms=cuniq;

	//initial score & feedback
	last_score=calcscore(msg,clength,solved);
	info->best_score=last_score;
	memcpy(info->best_key,key,keylength);
	if(info->disp_all) info->disp_all();
	
	log_file=fopen(info->log_name,"w"); //open log file

	while(info->running) { //go until max number of iterations or stop is pressed

		//feedback info
		info->last_time=float(end_time-start_time)/1000;
		if(info->time_func) start_time=info->time_func();
		if(info->disp_info) info->disp_info();

		improve=0;
		
		for(int p1=0; p1<keylength; p1++) { //do an iteration

			if(info->locked[p1]) continue; //skip if symbol is locked

			for(int p2=0; p2<keylength; p2++) {
			
				if(!info->running) goto EXIT; //stop

				if(p1>=cuniq && p2>=cuniq) continue; //skip if both symbols are in the extra letters area
				if(info->locked[p2] || key[p1]==key[p2]) continue; //skip if symbol is locked or identical 
				
				if(info->exclude) //exclusions
				{
					if(p1<cuniq && strchr(info->exclude+(27*p1),key[p2])) continue; 
					if(p2<cuniq && strchr(info->exclude+(27*p2),key[p1])) continue;
				}

				DO_SWAP; DECODE; //swap, decode, score

				key_str.assign(key,info->tabu_syms); //check for tabu
				if(info->tabu->find(key_str)!=info->tabu_end) score=-100000;
				else score=calcscore(msg,clength,solved);

				//tolerance of going downhill starts out at max, and decreases with each iteration without improve
				if(info->max_tol) tolerance=rand()%(info->max_tol-info->cur_tol+1);

				if(score<(last_score-tolerance)) {DO_SWAP;} //undo swap if beyond tolerance
				else //change is better or same as last score
				{
					last_score=score; 

					if(score>info->best_score) //this is the new best, save & display
					{						
						//if (print) printcipher(clength,cipher,solved,score,key);
						improve=1;
						info->best_score=score;
						memcpy(info->best_key,key,keylength);
						if(info->disp_all) info->disp_all();	
					}
				}
			}
		}

		if(!improve) 
		{
			if(++info->cur_tol>=info->max_tol) info->cur_tol=0; //reset downhill score tolerance
				
			if(info->max_tabu && ++info->cur_tabu>=info->max_tabu) //blacklist best key and reset best score
			{
				key_str.assign(info->best_key,info->tabu_syms);
				(*info->tabu)[key_str]=info->tabu->size();
				info->tabu_end=info->tabu->end();
				info->disp_tabu();

				memcpy(key,info->best_key,keylength);
				
				if(log_file) //log local optima
				{
					DECODE;
					info->get_words(solved);
					fprintf(log_file,"%i\t%i\t%i\t%s\t%s\n",info->best_score,info->num_words,info->stray_letters,key_str.data(),solved);
					fflush(log_file);
				}

				for(i=0; i<100; i++) shufflekey(key,keylength,cuniq); //random restart hillclimbing 
				info->best_score=-100000;
				info->cur_tabu=0;
			}
		}

		else info->cur_tol=info->cur_tabu=0; //improvment, reset variables

		// info->swaps IS INITIALIZED TO 5, WHICH IS ARBITRARY, BUT SEEMS TO WORK WELL
		for(i=0; i<info->swaps; i++) shufflekey(key,keylength,cuniq);  
		DECODE;

		key_str.assign(key,info->tabu_syms); //check for tabu
		if(info->tabu->find(key_str)!=info->tabu_end) last_score=-100000;
		else last_score=calcscore(msg,clength,solved); //last score at end of iteration

		if(info->time_func) end_time=info->time_func();
		if(!info->max_tol) tolerance=0; //reset tolerance
	}

EXIT:
	delete solved;
	info->running=0;
	if(log_file) fclose(log_file);
	info->tabu->clear();
	info->tabu_end=info->tabu->end();
	info->disp_tabu();

	return 0;
}

/******************************* END_MAIN_HILLCLIMBER_ALGORITHM ***********************************/

////////////////////////////////////////////////////////////////////////////////////////////////
//          Calculate a 'fitness' score for the solution based on the N-Gram counts             //
//////////////////////////////////////////////////////////////////////////////////////////////////

//index lookup for letter indexs 0-25, faster and supports lowercase
//the fast stat functions only count letters, which helps with ciphers that have other characters in the plain text

void GetFreqs(const char *string, int length)
{
	memset(freqs,0,104);
	count=unique=0;

	for(int index=0; index<length; index++) //frequency table, and uniques
	{
		int letter_index=LETTER_INDEXS[(unsigned char)string[index]];
		if(letter_index>-1) 
		{
			if(!freqs[letter_index]) unique++;
			freqs[letter_index]++;
			count++;
		}
	}
}

float FastIoC(const char *string, int length)
{
	float ic=0;
	
	GetFreqs(string,length);

	for(int index=0; index<26; index++) //calculate ioc
		if(freqs[index]>1) ic+=(freqs[index])*(freqs[index]-1); 

	ic/=(count)*(count-1);

	return ic;
}

float FastDIoC(const char* string, int length, int step=1)
{
	int freqs[676], letter1_index, letter2_index, count=0;
	float ic=0;
	
	memset(freqs,0,2704);
	
	for(int index=0; index<length-1; index+=step) //frequency table
	{
		letter1_index=LETTER_INDEXS[(unsigned char)string[index]];
		letter2_index=LETTER_INDEXS[(unsigned char)string[index+1]];
		if(letter1_index>-1 && letter2_index>-1) {freqs[(letter1_index*26)+letter2_index]++; count++;}
	}

	for(index=0; index<676; index++)
		if(freqs[index]>1) ic+=(freqs[index])*(freqs[index]-1); 

	ic/=(count)*(count-1);

	return ic;
}

float FastEntropy(const char *string, int length)
{
	float entropy=0, prob_mass;
	
	GetFreqs(string,length);

	for(int index=0; index<26; index++) //calculate entropy
	{
		if(!freqs[index]) continue;
		prob_mass=float(freqs[index])/count;
		entropy+=prob_mass*(log(prob_mass)/LOG2);
	}

	if(entropy==0.0) return entropy;
	return (-1*entropy);
}

float FastChiSquare(const char *string, int length)
{
	float chi2=0, prob_mass, cur_calc;

	GetFreqs(string,length);

	for(int index=0; index<26; index++) //calculate chi2
	{
		if(!freqs[index]) continue;
		prob_mass=float(count)/unique;
		cur_calc=freqs[index]-prob_mass;
		cur_calc*=cur_calc;
		cur_calc/=prob_mass;
		chi2+=cur_calc;
	}

	return chi2/count;
}

int calcscore(Message &msg, const int length_of_cipher,const char *solv) 
{
	int t1,t2,t3,t4,t5;
	int biscore=0,triscore=0,tetrascore=0,pentascore=0;
	int score, remaining, score_len=length_of_cipher;
	float score_mult=1.0;

	//get inital characters in for ngrams
	t1=LETTER_INDEXS[(unsigned char)solv[0]]; t2=LETTER_INDEXS[(unsigned char)solv[1]]; 
	t3=LETTER_INDEXS[(unsigned char)solv[2]]; t4=LETTER_INDEXS[(unsigned char)solv[3]]; 
	t5=LETTER_INDEXS[(unsigned char)solv[4]];

	remaining=score_len; //letters in text left for ngrams

	for(int c=0; c<score_len-1; c++) 
	{
		//only score an ngram which is all letters,
		//enough characters remain in the text (i.e. not close to the end)

		if(t1>-1 && t2>-1) { 
             biscore+=bigraphs[t1][t2];

			if(t3>-1 && remaining>2) {
                 triscore+=trigraphs[t1][t2][t3];
			
				if(t4>-1 && remaining>3) {
                     tetrascore+=tetragraphs[t1][t2][t3][t4];
				
					if(t5>-1 && remaining>4)
                         pentascore+=pentagraphs[t1][t2][t3][t4][t5];
				}
			}
		}
		
		t1=t2; t2=t3; t3=t4; t4=t5; t5=LETTER_INDEXS[(unsigned char)solv[c+5]]; //shift letters & get next
		remaining--; 
	}

	biscore=biscore>>3; triscore=triscore>>2; tetrascore=tetrascore>>1;

	score=pentascore+tetrascore+triscore+biscore;  //score=40000;

	/*int actfreq[26], expfreq[26], diff=0;
	msg.GetActFreq(actfreq);
	msg.GetExpFreq(expfreq);

	for(int cur_ltr=0; cur_ltr<26; cur_ltr++)
		diff+=ABS(actfreq[cur_ltr]-expfreq[cur_ltr]);

	score-=diff<<7;*/

	if(info->ioc_weight) score_mult*=1.05-(info->ioc_weight*ABS(FastIoC(solv,score_len)-info->lang_ioc));
		
	if(info->dioc_weight) //DIC, EDIC
	{
		//score_mult*=1.05-((info->dioc_weight>>1)*ABS(FastDIoC(solv,score_len,1)-info->lang_dioc));
		score_mult*=1.05-((info->dioc_weight>>1)*ABS(FastDIoC(solv,score_len,2)-info->lang_dioc));
	}
    	
	if(info->chi_weight) score_mult*=1.05-(info->chi_weight*ABS(FastChiSquare(solv,score_len)-info->lang_chi))/60.0;
	if(info->ent_weight) score_mult*=1.05-(info->ent_weight*ABS(FastEntropy(solv,score_len)-info->lang_ent))/150.0;

	//Cribs
	for(int cur_crib=0; cur_crib<info->num_cribs; cur_crib++)
		if(strstr(solv,info->cribs[cur_crib])) score_mult*=(float)1.025;

	//info->get_words(solv);
	//score_mult+=info->num_words/1000.0;

	return int(score*score_mult);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//        Mutate the char array "key[]" by swapping two unlocked, unexcluded letters            //
//////////////////////////////////////////////////////////////////////////////////////////////////

inline void shufflekey(char *key,const int keylength,const int cuniq) {

	int x, y, z, canswap=0;

	//check if all characters are locked to avoid infinite loop
	for(int symbol=keylength; symbol--;) if(!info->locked[symbol]) { canswap=1; break; }

	if(canswap)
	{
		do {x=rand()%keylength;} while(info->locked[x]);
		do {y=rand()%keylength;} while(info->locked[y]);

		if(info->exclude) //exclusions
		{
			if(x<cuniq && strchr(info->exclude+(27*x),key[y])) return; 
			if(y<cuniq && strchr(info->exclude+(27*y),key[x])) return;
		}
		
		z=key[x]; key[x]=key[y]; key[y]=z;
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                        Print the cipher "block" and the solution "block"                     //
//----------------------------------------------------------------------------------------------//
//  ALSO:             Calculate and print the percentage of vowels in the solution              //
//                   NOTE: Normal English text normally contains approx. 40% vowels             //
//////////////////////////////////////////////////////////////////////////////////////////////////
/*
void printcipher(int length_of_cipher,const char *ciph,char *solv,int bestscore,char *key) {

	int c=0;
	int s=0;
	int i,x,y;
	int width,height;

	switch(length_of_cipher) {
		case 153: { width=17; height=9;  } break;
		case 318: { width=53; height=6;  } break;
		case 330: { width=30; height=11; } break;
		case 340: { width=17; height=20; } break;
		case 378: { width=18; height=21; } break;
		case 408: { width=17; height=24; } break;
		default: { return; } }

	printf("\n--------------------------------------------------------------------------------------------------------------------\n\n");

	for(y=0;y<height;y++) { 
		for(x=0;x<width;x++) printf("%c",ciph[c++]);
		printf("   =   ");
		for(x=0;x<width;x++) printf("%c",solv[s++]);
		printf("\n"); }

//	printvowels section

	int diff_tot=0;
	int	freqs[26]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	for(i=0;i<26;i++) freqs[i]=(int)(unigraphs[i]*length_of_cipher)/100;					// CALCULATE EXPECTED LETT. FREQS
	int solv_freqs[26]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	for(i=0;i<length_of_cipher;i++) solv_freqs[solv[i]-'A']++;								// CALCULATE ACTUAL LETT. FREQS

	printf("\n\n              'A   B   C   D  'E   F   G   H  'I   J   K   L   M   N  'O   P   Q   R   S   T  'U   V   W   X   Y   Z");
	printf("\n  Expected: ");	for(i=0;i<26;i++) printf("%4d",freqs[i]);
	printf("\n     Found: ");	for(i=0;i<26;i++) printf("%4d",solv_freqs[i]);
	printf("\nDifference: ");	for(i=0;i<26;i++) { y=abs(freqs[i]-solv_freqs[i]); printf("%4d",y); diff_tot+=y; }

	printf("\n\nDifference Total: %d    --    Deviation From Expected: %f",diff_tot,100*((float)diff_tot/length_of_cipher));
	printf("\n\nVowel Pcg. = %f    --    ",100*((solv_freqs[0]+solv_freqs[4]+solv_freqs[8]+solv_freqs[14]+solv_freqs[20])/(float)length_of_cipher));
	printf("Longest String Of Consonants: %d\n\n",calclsoc(length_of_cipher,solv));
	printf("Best Score = %d\n",bestscore);
	printf("\nKey: '%s'\n\n",key); 

}
*/
//////////////////////////////////////////////////////////////////////////////////////////////////
//            Print the character frequency table of the cipher and a few statistics            //
//////////////////////////////////////////////////////////////////////////////////////////////////
/*
void printfrequency(int length_of_cipher, int *unique_array,char *unique_string,int cipher_uniques) {

	int i, f=0;
	int z=(int)strlen(unique_string);
	char zee[10];

	printf("Cipher Length:  %d characters\n",length_of_cipher);									//PRINT CIPHER LENGTH
	printf("Cipher Uniques: %d unique characters\n\n",cipher_uniques);							//PRINT NUMBER OF UNIQUE CHARACTERS

	printf("Frequency Table for Cipher:\n");
	for(i=0;i<z;i++) printf("-"); printf("\n");
	for(i=0;i<z;i++) if(unique_array[i]/100 != 0) printf("%1d",unique_array[i]/100); if(unique_array[0]>=100) printf("\n");
	for(i=0;i<z;i++) { sprintf(zee,"%d",unique_array[i]); if(unique_array[i]/10 != 0) printf("%c",zee[strlen(zee)-2]); } printf("\n");
	for(i=0;i<z;i++) { printf("%1d",unique_array[i] % 10); f = f + (unique_array[i] * (unique_array[i]-1)); } printf("\n");
	for(i=0;i<z;i++) printf("-");

	printf("\n%s\n\n",unique_string);

	printf("Phi(O) = %i\n",f);
	printf("Phi(P) = %f\n",(.0675) * length_of_cipher * (length_of_cipher - 1));
	printf("Phi(R) = %f\n\n",(.0385) * length_of_cipher * (length_of_cipher - 1));
	printf("DeltIC = %f\n\n",f/((.0385) * length_of_cipher * (length_of_cipher - 1)));

}
*/
//////////////////////////////////////////////////////////////////////////////////////////////////
//           Return the value of a unigraph for use in other areas of the program               //
//////////////////////////////////////////////////////////////////////////////////////////////////

void GetUnigraphs(double *dest) {memcpy(dest,unigraphs,26*sizeof(double));}

//////////////////////////////////////////////////////////////////////////////////////////////////
//             Read the specified ngram file, of size n, into the proper array                  //
//////////////////////////////////////////////////////////////////////////////////////////////////

int ReadNGraphs(const char *filename, int n) 
{
	FILE *tgfile;
	char ngraph[8];
	int *ngraphs;
	int nsize, freq, index;
	float percent;

	if(!(tgfile=fopen(filename,"r"))) return 0;

	if(n==1) {/*ngraphs=unigraphs;*/ nsize=UNI_SIZE;}
	if(n==2) {ngraphs=&bigraphs[0][0]; nsize=BI_SIZE;}
	if(n==3) {ngraphs=&trigraphs[0][0][0]; nsize=TRI_SIZE;}
	if(n==4) {ngraphs=&tetragraphs[0][0][0][0]; nsize=TETRA_SIZE;}
	if(n==5) {ngraphs=&pentagraphs[0][0][0][0][0]; nsize=PENTA_SIZE;}

	//init to zero
	if(n>1) memset(ngraphs,0,nsize*sizeof(int));

	//read file
	while(fscanf(tgfile,"%s : %i %f",ngraph,&freq,&percent)!=EOF) 
	{
		//calculate index
		index=(ngraph[n-1]-'A');
		if(n>1) index+=(ngraph[n-2]-'A')*UNI_SIZE;
		if(n>2) index+=(ngraph[n-3]-'A')*BI_SIZE;
		if(n>3) index+=(ngraph[n-4]-'A')*TRI_SIZE;
		if(n>4) index+=(ngraph[n-5]-'A')*TETRA_SIZE;
		
		if(index<0 || index>nsize) continue;
		
		//set ngraph
		if(n==1) unigraphs[index]=percent;
		//else ngraphs[index]=int(10*log((double)freq));
		else ngraphs[index]=freq;
	}

	fclose(tgfile); 

	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//  Find position where the word string inserted into the plain text produces the highest score //
//////////////////////////////////////////////////////////////////////////////////////////////////

int WordPlug(Message &msg, const char *word)
{
 	const char *cipher, *plain;
	int word_len, msg_len, cur_score=0, best_score=0;
	int act_freq[26], exp_freq[26], fail, old_ioc_weight;
	SYMBOL symbol;
	Map org_map, best_map;

	word_len=(int)strlen(word);
	cipher=msg.GetCipher();
	msg_len=msg.GetLength();
	
	msg.GetExpFreq(exp_freq);

	org_map=msg.cur_map;
	best_map=msg.cur_map;

	memset(&symbol,0,sizeof(SYMBOL));

	//backup lang_ioc, and set to 0
	old_ioc_weight=info->ioc_weight;
	info->ioc_weight=0;
	info->exclude=NULL;

	for(int position=0; position<=msg_len-word_len; position++)
	{		
		msg.cur_map=org_map;
		
		fail=false;

		//set word in current position
		for(int chr=0; chr<word_len; chr++)
		{
			int index=msg.cur_map.FindByCipher(cipher[position+chr]);
			msg.cur_map.GetSymbol(index,&symbol);
			
			//fail on exclusion
			if(strchr(symbol.exclude,word[chr])) {fail=true;  break;}
			
			symbol.plain=word[chr];
			msg.cur_map.AddSymbol(symbol,0);
			msg.cur_map.SetLock(msg.cur_map.FindByCipher(symbol.cipher),true);
		}
		
		if(fail) continue;		
		
		//check validity	
		plain=msg.GetPlain();
		if(memcmp(word,plain+position,word_len)) fail=true;
		
		//if any letter appears too often
		msg.GetActFreq(act_freq);	
	
		for(int letter=0; letter<26; letter++)
				if(act_freq[letter]>2*(exp_freq[letter]+1)) fail=true;
		
		if(fail) continue;
		
		//compare score
		cur_score=calcscore(msg,msg_len,plain);
		
		if(cur_score>best_score)
		{
			best_score=cur_score;
			best_map=msg.cur_map;
		}
	}

	msg.cur_map=best_map;

	//restore old ioc weight
	info->ioc_weight=old_ioc_weight;
	
	return best_score;
}

#define MSG_SWAP	{temp=key[p1]; key[p1]=key[p2]; key[p2]=temp;}
#define MSG_DECODE	msg.SetKey(key); msg.Decode(); if(solve_type==SOLVE_CEMOPRTU) {strupr(solved);}

inline int IN_SAME_KEY(int p1, int p2, int *split_points, int num_splits)
{
	for(int cur_split=0; cur_split<num_splits; cur_split++)
	{
		if(p1==split_points[cur_split] ||  p2==split_points[cur_split]) return 0;
		if(IS_BETWEEN(split_points[cur_split],p1,p2)) return 0;
	}

	return 1;
}

int hillclimb2(Message &main_msg, int solve_type, char *key , int iLineChars)
{
	Message msg; //decoding message
	int i, p1, p2, clength, temp, use_key_len, full_key_len;
	int score=0, last_score=0, improve=0, tolerance=0;
	long start_time=0, end_time=0;
	char *cipher, *solved;
	std::string key_str;

	msg+=main_msg;

	//init info
	info->cur_tol=0;
	info->cur_tabu=0;
	info->tabu_end=info->tabu->end();
	clength=msg.GetLength();
	cipher=msg.GetCipher();
	solved=msg.GetPlain();
	full_key_len=use_key_len=strlen(key);

	//key sections
	int split_points[10];
	int num_splits=0;

	for(int key_start=0; ; key_start++)
	{
		int key_length=ChrIndex(key+key_start,'|');
		if(key_length==-1) break;

		key_start+=key_length;

		split_points[num_splits]=key_start;
		num_splits++;
		
	}

	switch(solve_type)
	{
		case SOLVE_VIG: use_key_len=msg.GetKeyLength(); break;
		case SOLVE_DISUB: use_key_len=msg.digraph_map.GetNumDigraphs()<<1;
		//case SOLVE_COLVIG: use_key_len=split_points[0];
	}
 
	log_file=fopen(info->log_name,"w"); //open log file

	//initial score, save best, & feedback
	MSG_DECODE;
	last_score=calcscore(msg,clength,solved);
	info->best_score=last_score;
	strcpy(info->best_key,key);
	main_msg+=msg;
	if(info->disp_all) info->disp_all();

	//go until max number of iterations or stop is pressed
	while(info->running) {

		//feedback info
		info->last_time=float(end_time-start_time)/1000;
		if(info->time_func) start_time=info->time_func();
		if(info->disp_info) info->disp_info();
		
		improve=0;
		
		for(p1=0; p1<full_key_len; p1++) {
			for(p2=0; p2<full_key_len; p2++) {
			
				if(!info->running) goto EXIT; //stopped
				if(key[p1]==key[p2]) continue; //same character in key
				if(p1>use_key_len && p2>use_key_len) continue; //p1&p2 in extra letter area
				if(solve_type==SOLVE_DISUB) if(msg.digraph_map.GetLock(p1>>1) || msg.digraph_map.GetLock(p2>>1)) continue;

				if(!IN_SAME_KEY(p1,p2,split_points,num_splits)) continue; //in different split keys, or on split
			
				MSG_SWAP; MSG_DECODE; //swap, decode, score
				if(solve_type==SOLVE_SUBPERM || solve_type==SOLVE_SUBCOL) {key_str.assign(key,msg.cur_map.GetNumSymbols()); key_str.append(key+split_points[0]);}
				else if(solve_type==SOLVE_COLVIG) {key_str.assign(key,msg.GetKeyLength()); key_str.append(key+split_points[0]);}
				else key_str.assign(key,use_key_len);
				if(info->tabu->find(key_str)!=info->tabu_end) score=-100000;
				else score=calcscore(msg,clength,solved);

				//tolerance of going downhill starts out at max, and decreases with each iteration without improve
				if(info->max_tol) tolerance=rand()%(info->max_tol-info->cur_tol+1);

				if(score<(last_score-tolerance)) {MSG_SWAP;} //undo if change made it worse than last score
				else //change is better or same as last score
				{
					last_score=score; 
							
					if(score>info->best_score) //this is the new best, save & display
					{
						//save best, & feedback info
						main_msg+=msg;
						main_msg.Decode();
						improve=1;
						info->best_score=score;
						strcpy(info->best_key,key);
						if(info->disp_all) info->disp_all();	
					}
				}
			}
		}

		if(!improve)
		{
			if(++info->cur_tol>=info->max_tol) info->cur_tol=0; //reset downhill score tolerance

			if(info->max_tabu && ++info->cur_tabu>=info->max_tabu) //blacklist best key and reset best score
			{
				if(solve_type==SOLVE_SUBPERM || solve_type==SOLVE_SUBCOL) {key_str.assign(info->best_key,msg.cur_map.GetNumSymbols()); key_str.append(info->best_key+split_points[0]);}
				else if(solve_type==SOLVE_COLVIG) {key_str.assign(info->best_key,msg.GetKeyLength()); key_str.append(info->best_key+split_points[0]);}

				else key_str.assign(info->best_key,use_key_len);
				(*info->tabu)[key_str]=info->tabu->size();
				info->tabu_end=info->tabu->end();
				info->disp_tabu();

				memcpy(key,info->best_key,full_key_len);
				
				if(log_file) //log local optima
				{
					MSG_DECODE;
					info->get_words(solved);
					fprintf(log_file,"%i\t%i\t%i\t%s\t%s\n",info->best_score,info->num_words,info->stray_letters,key_str.data(),solved);
					fflush(log_file);
				}

				info->best_score=-100000;
				info->cur_tabu=0;
			}
		}

		else info->cur_tol=info->cur_tabu=0; //improvment, reset variables

		for(i=0; i<info->swaps; i++) // info->swaps IS INITIALIZED TO 5, WHICH IS ARBITRARY, BUT SEEMS TO WORK WELL
		{
			p1=rand()%use_key_len; p2=rand()%use_key_len;
			if(solve_type==SOLVE_DISUB) if(msg.digraph_map.GetLock(p1>>1) || msg.digraph_map.GetLock(p2>>1)) continue;
			if(!IN_SAME_KEY(p1,p2,split_points,num_splits)) continue; //in different split keys, or on split
			MSG_SWAP;
		} 
   
		MSG_DECODE; //last score at end of iteration

		if(solve_type==SOLVE_SUBPERM || solve_type==SOLVE_SUBCOL) {key_str.assign(key,msg.cur_map.GetNumSymbols()); key_str.append(key+split_points[0]);}
		else if(solve_type==SOLVE_COLVIG) {key_str.assign(key,msg.GetKeyLength()); key_str.append(key+split_points[0]);}
		else key_str.assign(key,use_key_len);
		if(info->tabu->find(key_str)!=info->tabu_end) last_score=-100000;
		else last_score=calcscore(msg,clength,solved); //last score at end of iteration
		
		if(info->time_func) end_time=info->time_func();
		if(!info->max_tol) tolerance=0; //reset tolerance
	}	
	
EXIT:
	if(log_file) fclose(log_file);
	info->tabu->clear();
	info->tabu_end=info->tabu->end();
	info->disp_tabu();
	return 0;
}

void running_key(Message &msg, char *key_text)
{
	int key_text_len=strlen(key_text);
	int msg_len=msg.GetLength();
	int cur_score=0;

	info->best_score=-100000;

	msg.SetKeyLength(msg_len);
	info->cur_tabu=key_text_len-msg_len;

	for(info->cur_tol=0; info->cur_tol<info->cur_tabu; info->cur_tol++)
	{
		if(info->disp_info) info->disp_info();

		if(!info->running) break;

		msg.SetKey(key_text+info->cur_tol);
		cur_score=calcscore(msg,msg_len,msg.GetPlain());
		if(cur_score>info->best_score)
		{
			info->best_score=cur_score;
			strcpy(info->best_key,msg.key);
			if(info->disp_all) info->disp_all();
		}

	}
}

void dictionary_vigenere(Message &msg)
{
	int msg_len=msg.GetLength();
	int cur_score=0;
	std::map<std::string,int>::iterator iter=info->dictionary->begin();
	info->best_score=-100000;
	info->cur_tabu=info->dict_words;

	for(info->cur_tol=0; iter!=info->dictionary->end(); info->cur_tol++, ++iter)
	{
		if(info->disp_info) info->disp_info();

		if(!info->running) break;

		msg.SetKey(std::string(iter->first).c_str());
		msg.SetKeyLength(strlen(msg.key));
		cur_score=calcscore(msg,msg_len,msg.GetPlain());
		if(cur_score>info->best_score)
		{
			info->best_score=cur_score;
			strcpy(info->best_key,msg.key);
			if(info->disp_all) info->disp_all();
		}
	}
}
