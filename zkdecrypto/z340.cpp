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

#define DO_SWAP		std::swap(key[p1],key[p2]);
#define DECODE_A	{for(y=0; y<clength; y++) solved[y]=*decoder[(unsigned char)cipher[y]];}
#define DECODE_B	{msg.SetKey(key); solved=msg.GetPlain();}

#define TABU_STR_A(KEY)		key_str.assign(KEY,cuniq); 

#define TABU_STR_B(KEY)		{if(solve_type==SOLVE_SUBPERM || solve_type==SOLVE_SUBCOL) {key_str.assign(KEY,msg.cur_map.GetNumSymbols()); key_str.append(KEY+split_points[0]);} \
							else if(solve_type==SOLVE_COLVIG) {key_str.assign(KEY,msg.GetKeyLength()); key_str.append(KEY+split_points[0]);} \
							else key_str.assign(KEY,use_key_len);}

#define ADD_TEMP_TABU		temp_tabu[key_str]=temp_tabu.size(); temp_tabu_end=temp_tabu.end();
#define CLEAR_TEMP_TABU		temp_tabu.clear(); temp_tabu_end=temp_tabu.end();

#define ADD_OPTIMA_TABU		(*(info->optima_tabu))[key_str]=info->optima_tabu->size(); optima_tabu_end=info->optima_tabu->end();
#define CLEAR_OPTIMA_TABU	info->optima_tabu->clear(); optima_tabu_end=info->optima_tabu->end();
					
#define SET_SCORE(SCR,DEC)	if(temp_tabu.find(key_str)!=temp_tabu_end || info->optima_tabu->find(key_str)!=optima_tabu_end) SCR=-100000; else {DEC; SCR=calcscore(msg,clength,solved);} 

#define LOG_BEST(SCR)		info->get_words(solved); fprintf(log_file,"*****\nScore: %i\t NumWords: %i\t WTF: %i\t\nKEY: %s\nSolution: %s\n\n",SCR,info->num_words,info->stray_letters,key_str.data(),solved); fflush(log_file);

#define CLEAR_TABU_PROB 80

int hillclimb(Message &msg, const char cipher[],int clength,char key[],int print)
{
	int cuniq,keylength,i,j,x,y;
	int uniq[ASCII_SIZE],uniqarr[ASCII_SIZE];
	char uniqstr[ASCII_SIZE];
	char *decoder[ASCII_SIZE];

	char *solved=new char[clength+1];

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

/****************************** START_MAIN_HILLCLIMBER_ALGORITHM **********************************/

	//init info
	info->cur_tabu=info->cur_tol=0;

	CLEAR_TEMP_TABU; CLEAR_OPTIMA_TABU; //clear tabu lists
	optima_tabu_end=info->optima_tabu->end();

	//initial score & feedback
	DECODE_A;
	cur_best_score=info->best_score=last_score=calcscore(msg,clength,solved);
	strcpy(info->best_key,key);
	strcpy(cur_best_key,key);
	if(info->disp_all) info->disp_all();
	
	log_file=fopen(info->log_name,"w"); //open log file

	while(info->running) { //go until max number of iterations or stop is pressed

		//feedback info
		info->last_time=float(end_time-start_time)/1000;
		if(info->time_func) start_time=info->time_func();
		if(info->disp_info) info->disp_info();

		improve=0;
		
		for(p1=0; p1<keylength; p1++) { //do an iteration

			if(info->locked[p1]) continue; //skip if symbol is locked

			for(p2=0; p2<keylength; p2++) {
			
				if(!info->running) goto EXIT; //stop

				if(p1>=cuniq && p2>=cuniq) continue; //skip if both symbols are in the extra letters area
				if(info->locked[p2] || key[p1]==key[p2]) continue; //skip if symbol is locked or identical 
				
				if(info->exclude) //exclusions
				{
					if(p1<cuniq && strchr(info->exclude+(27*p1),key[p2])) continue; 
					if(p2<cuniq && strchr(info->exclude+(27*p2),key[p1])) continue;
				}

				DO_SWAP; TABU_STR_A(key); SET_SCORE(score,DECODE_A); ADD_TEMP_TABU; //swap, decode, score

				//tolerance of going downhill starts out at max, and decreases with each iteration without improve
				if(info->max_tol) tolerance=rand()%(info->max_tol-info->cur_tol+1);

				if(score<(last_score-tolerance)) {DO_SWAP;} //undo swap if beyond tolerance
				else //change is better or same as last score
				{
					last_score=score; 

					if(score>info->best_score) //this is the best seen so far, save & display
					{						
						//if (print) printcipher(clength,cipher,solved,score,key);
						improve=1;
						info->best_score=score;
						strcpy(info->best_key,key);
						if(info->disp_all) info->disp_all();
					}

					if(score>cur_best_score) //best since last tabu & restart
					{
						cur_best_score=score;
						strcpy(cur_best_key,key);
					}
				}
			}
		}

		if(!improve) 
		{
			if(++info->cur_tol>=info->max_tol) info->cur_tol=0; //reset downhill score tolerance
				
			if(info->max_tabu && ++info->cur_tabu>=info->max_tabu) //blacklist best key since last restart, reset current best score, 50/50 back to best or random restart
			{	
				TABU_STR_A(cur_best_key); ADD_OPTIMA_TABU;
				if(log_file) {strcpy(key,cur_best_key); DECODE_A; LOG_BEST(cur_best_score);}
				cur_best_score=-10000;

				if(rand()%2) for(i=0; i<100; i++) shufflekey(key,keylength,cuniq); //random restart
				else strcpy(key,info->best_key); //back to best
				info->cur_tabu=0;
			}
		}

		else info->cur_tol=info->cur_tabu=0; //improvment, reset variables
		
		for(i=0; i<info->swaps; i++) shufflekey(key,keylength,cuniq); //random swaps at end of iteration
		TABU_STR_A(key); SET_SCORE(last_score,DECODE_A); //score at end of iteration

		if(!(rand()%CLEAR_TABU_PROB)) CLEAR_TEMP_TABU; //clear tabu memory

		if(info->time_func) end_time=info->time_func();
		if(!info->max_tol) tolerance=0; //reset tolerance
	}

EXIT:
	delete solved;
	info->running=0;
	if(log_file) fclose(log_file);
	return 0;
}

inline int IN_SAME_KEY(int p1, int p2)
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
	msg+=main_msg;

	//init info
	info->cur_tabu=info->cur_tol=0;
	clength=msg.GetLength();
	cipher=msg.GetCipher();
	solved=msg.GetPlain();
	full_key_len=use_key_len=strlen(key);
	CLEAR_TEMP_TABU; CLEAR_OPTIMA_TABU; //clear tabu lists
	optima_tabu_end=info->optima_tabu->end();

	//key sections
	num_splits=0;
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
	}
 
	log_file=fopen(info->log_name,"w"); //open log file

	//initial score, save best, & feedback
	DECODE_B;
	cur_best_score=info->best_score=last_score=calcscore(msg,clength,solved);
	strcpy(info->best_key,key);
	strcpy(cur_best_key,key);
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
				if(!IN_SAME_KEY(p1,p2)) continue; //in different split keys, or on split
			
				DO_SWAP; TABU_STR_B(key); SET_SCORE(score,DECODE_B); ADD_TEMP_TABU; //swap, decode, score 

				//tolerance of going downhill starts out at max, and decreases with each iteration without improve
				if(info->max_tol) tolerance=rand()%(info->max_tol-info->cur_tol+1);
				else tolerance=0;

				if(score<(last_score-tolerance)) {DO_SWAP;} //undo if change made it worse than last score
				else //change is better or same as last score
				{
					last_score=score;
							
					if(score>info->best_score) //this is the new best, save & display
					{
						//save best, & feedback info
						main_msg+=msg;
						improve=1;
						info->best_score=score;
						strcpy(info->best_key,key);
						if(info->disp_all) info->disp_all();
					}

					if(score>cur_best_score) //best since last tabu & restart
					{
						cur_best_score=score;
						strcpy(cur_best_key,key);
					}
				} 
			}
		}

		if(!improve)
		{
			if(++info->cur_tol>=info->max_tol) //reset downhill score tolerance
			{
				info->cur_tol=0; 
			}

			if(info->max_tabu && ++info->cur_tabu>=info->max_tabu) ///blacklist best key since last restart, reset current best score, 50/50 back to best or random restart
			{
				TABU_STR_B(cur_best_key); ADD_OPTIMA_TABU; 
				if(log_file) {strcpy(key,cur_best_key); DECODE_B; LOG_BEST(cur_best_score);}
				cur_best_score=-10000;
				
				if(rand()%2) 
				for(i=0; i<full_key_len<<2; i++) // random restart
				{
					p1=rand()%use_key_len; p2=rand()%use_key_len;
					if(solve_type==SOLVE_DISUB) if(msg.digraph_map.GetLock(p1>>1) || msg.digraph_map.GetLock(p2>>1)) continue;
					if(!IN_SAME_KEY(p1,p2)) continue; //in different split keys, or on split
					DO_SWAP;
				}

				else strcpy(key,info->best_key); //back to best
				info->cur_tabu=0;
			}
		}

		else {info->cur_tol=info->cur_tabu=0;} //improvment, reset variables

		for(i=0; i<info->swaps; i++) // random swaps at end of iteration
		{
			p1=rand()%use_key_len; p2=rand()%use_key_len;
			if(solve_type==SOLVE_DISUB) if(msg.digraph_map.GetLock(p1>>1) || msg.digraph_map.GetLock(p2>>1)) continue;
			if(!IN_SAME_KEY(p1,p2)) continue; //in different split keys, or on split
			DO_SWAP;
		} 
   
		TABU_STR_B(key); SET_SCORE(last_score,DECODE_B); //score at end of iteration
	
		if(!(rand()%CLEAR_TABU_PROB)) CLEAR_TEMP_TABU; //clear tabu memory

		if(info->time_func) end_time=info->time_func();
		if(!info->max_tol) tolerance=0; //reset tolerance
	}	
	
EXIT:
	if(log_file) fclose(log_file);
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

	prob_mass=float(count)/unique;
}

float FastIoC()
{
	float ic=0;

	for(int index=0; index<26; index++) //calculate ioc
		if(freqs[index]>1) ic+=(freqs[index])*(freqs[index]-1); 

	ic/=(count)*(count-1);

	return ic;
}

float FastDIoC(const char* string, int length, int step=1)
{
	int freqs[676], letter1_index, letter2_index, index, count=0;
	float ic=0;
	
	memset(freqs,0,2704);
	
	for(index=0; index<length-1; index+=step) //frequency table
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

float FastEntropy()
{
	float entropy=0, prob_mass;

	for(int index=0; index<26; index++) //calculate entropy
	{
		if(!freqs[index]) continue;
		prob_mass=float(freqs[index])/count;
		entropy+=prob_mass*(log(prob_mass)/LOG2);
	}

	if(entropy==0.0) return entropy;
	return (-1*entropy);
}

float FastChiSquare()
{
	float chi2=0, cur_calc;

	for(int index=0; index<26; index++) //calculate chi2
	{
		if(!freqs[index]) continue;
		
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
		
		/****************************************************/
		/*  Shift Letters and Get Next - Saves Many Lookups */
		/****************************************************/

		t1=t2; t2=t3; t3=t4; t4=t5; t5=LETTER_INDEXS[(unsigned char)solv[c+5]];
		remaining--; 
	}

//	biscore=biscore>>3; triscore=triscore>>2; tetrascore=tetrascore>>1; // retained for reference - DO NOT DELETE

	score=pentascore+(tetrascore>>1)+(triscore>>2)+(biscore>>3);

	/*int actfreq[26], expfreq[26], diff=0;
	msg.GetActFreq(actfreq);
	msg.GetExpFreq(expfreq);

	for(int cur_ltr=0; cur_ltr<26; cur_ltr++)
		diff+=ABS(actfreq[cur_ltr]-expfreq[cur_ltr]);

	score-=diff<<7;*/

	//Stats
	GetFreqs(solv,score_len);

	if(info->ioc_weight) score_mult*=1.05-(info->ioc_weight*ABS(FastIoC()-info->lang_ioc));
		
	if(info->dioc_weight) //DIC, EDIC
	{
		//score_mult*=1.05-((info->dioc_weight>>1)*ABS(FastDIoC(solv,score_len,1)-info->lang_dioc));
		score_mult*=1.05-((info->dioc_weight>>1)*ABS(FastDIoC(solv,score_len,2)-info->lang_dioc));
	}
    	
	if(info->chi_weight) score_mult*=1.05-(info->chi_weight*ABS(FastChiSquare()-info->lang_chi))/60.0;
	if(info->ent_weight) score_mult*=1.05-(info->ent_weight*ABS(FastEntropy()-info->lang_ent))/150.0;

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
		//else ngraphs[index]=int(log((double)freq));
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
			msg.GetKey(info->best_key,"");
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
		msg.SetKeyLength(strlen(std::string(iter->first).c_str()));
		cur_score=calcscore(msg,msg_len,msg.GetPlain());
		if(cur_score>info->best_score)
		{
			info->best_score=cur_score;
			msg.GetKey(info->best_key,"");
			if(info->disp_all) info->disp_all();
		}
	}
}
