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

#include "headers/z340.h"
#include "headers/z340Globals.h"
#include "mt19937ar-cok.cpp"

int hillclimb(const char cipher[],int clength,char key[],int num_symbols,const char locked[],SOLVEINFO &info, int &use_graphs, const char *exclude, int print)
{
	#define	DO_SWAP	{ int temp=key[p1]; key[p1]=key[p2]; key[p2]=temp; }

	int keylength,i,j,x,y;
	int uniq[ASCII_SIZE],uniqarr[ASCII_SIZE];
	char solved[MAX_CIPH_LENGTH],solvedtemp[MAX_CIPH_LENGTH];
	char uniqstr[ASCII_SIZE];

	for(i=0;i<MAX_CIPH_LENGTH;i++) solved[i]=solvedtemp[i]=0;						//INITIALIZE (ZERO) ARRAYS
	for(i=0;i<ASCII_SIZE;i++) uniq[i]=uniqstr[i]=uniqarr[i]=0;

	for(i=0;i<clength;i++) ++uniq[(int)cipher[i]];									//COUNT # OF UNIQUE CHARS IN CIPHER

	i=255; j=0;																		//CALCULATE AND SORT THE CIPHER UNIQUES
	for(y=0;y<255;y++) { 
		for(x=255;x>0;x--)
			{ if(uniq[x]==i) { uniqstr[j]=x; uniqarr[j++]=i; } } i--;}

	keylength=(int)strlen(key);
	
	if(keylength < num_symbols)      //THIS SHOULD NEVER HAPPEN
		{ printf("\nKEYLENGTH ERROR!! -- Key is TOO SHORT\n\n"); exit(-1); } 

	if (print) printfrequency(clength,uniqarr,uniqstr,num_symbols);

	for(x=0;x<num_symbols;x++) { for(y=0;y<clength;y++) if(cipher[y]==uniqstr[x]) solved[y]=key[x]; };

/****************************** START_MAIN_HILLCLIMBER_ALGORITHM **********************************/

	int score = 0, iterations = 0, last_score=0;
	
	long start_time=0, end_time=0;
	int improve=0;
	info.cur_try=0;
	info.cur_fail=0;
	
	//seed random generator
	init_genrand((unsigned long)time(NULL));

	//initial score & feedback
	last_score=calcscore(clength,solved,use_graphs);
	info.best_score=last_score;
	improve=1;
	info.cur_fail=0;
	memcpy(info.best_key,key,256);
	if(info.disp_all) info.disp_all();

	//go until max number of iterations or stop is pressed
	for(info.cur_try=0; info.cur_fail<info.max_fail; info.cur_try++) {
		
		//feedback info
		info.cur_try=iterations;
		info.last_time=float(end_time-start_time)/1000;
		if(info.time_func) start_time=info.time_func();
		if(info.disp_info) info.disp_info();
		
		improve=0;
		
		for(int p1=0;p1<keylength;p1++) {

			if(locked[p1]) continue; //skip if symbol is locked

			for(int p2=0;p2<keylength;p2++) {
				
			/*stop*/
			if(!info.running) return 0;
			if(locked[p2] || (p1==p2)) continue; //skip if symbol is locked or identical 
			
			//exclusions
			if(exclude)
			{
				if(p1<num_symbols && strchr(exclude+(27*p1),key[p2])) continue; 
				if(p2<num_symbols && strchr(exclude+(27*p2),key[p1])) continue;
			}

			//don't bother if both symbols are in the extra letters area
			//it won't make a difference in the score, and wastes time
			if(p1>=num_symbols && p2>=num_symbols) continue;

			//swap and score
			DO_SWAP; for(x=0;x<num_symbols;x++) { for(y=0;y<clength;y++) if(cipher[y]==uniqstr[x]) solvedtemp[y]=key[x]; }
			score=calcscore(clength,solvedtemp,use_graphs); 

			//undo if change made it worse than last score
			if(score<last_score) {DO_SWAP;}
			else //change is better or same as last score
			{
				memcpy(solved,solvedtemp,clength);
				last_score=score;

				if(score>info.best_score) //this is the new best, save & display
				{
					if (print) printcipher(clength,cipher,solved,score,key);
					
					//feedback info
					info.best_score=score;
					improve=1;
					info.cur_fail=0;
					memcpy(info.best_key,key,256);
					if(info.disp_all) info.disp_all();	
				}
			}
			}
		}

	// info.swaps IS INITIALIZED TO 5, WHICH IS ARBITRARY, BUT SEEMS TO WORK REALLY WELL
	for(i=0;i<info.swaps;i++) shufflekey(key,keylength,num_symbols,locked,exclude);
	
	iterations++; if(iterations>info.revert) { memcpy(key,info.best_key,256); iterations=0; }
	for(x=0;x<num_symbols;x++) { for(y=0;y<clength;y++) if(cipher[y]==uniqstr[x]) solved[y]=key[x]; };
	last_score=calcscore(clength,solved,use_graphs); 
	
	if(!improve) info.cur_fail++;
	
	if(info.time_func) end_time=info.time_func();

	}

	return 0;
}

/******************************* END_MAIN_HILLCLIMBER_ALGORITHM ***********************************/

//////////////////////////////////////////////////////////////////////////////////////////////////
//          Calculate a 'fitness' score for the solution based on the N-Gram counts             //
//////////////////////////////////////////////////////////////////////////////////////////////////

#define IS_LETTER(L) ((L<0 || L>25)? 0:1)

inline int calcscore(const int length_of_cipher,const char *solv,int &use_graphs) {

	int t1,t2,t3,t4,t5;
	int biscore=0,triscore=0,tetrascore=0,pentascore=0;
	int score;

	t1=solv[0]-'A'; t2=solv[1]-'A'; t3=solv[2]-'A'; t4=solv[3]-'A'; t5=solv[4]-'A';

	for(int c=0; c<length_of_cipher; c++) {

		//allow to score incomplete decoding (one with --- in it)
		if(IS_LETTER(t1) && IS_LETTER(t2)) {
			if(c<length_of_cipher-1 && (use_graphs & USE_BI)) 
				{ biscore += bigraphs[t1*26+t2]; }

			if(IS_LETTER(t3)) {
				if(c<length_of_cipher-2 && (use_graphs & USE_TRI))
					{ triscore += trigraphs[t1*676+t2*26+t3]; }
		
				if(IS_LETTER(t4)) {
					if(c<length_of_cipher-3 && (use_graphs & USE_TETRA))
						{ tetrascore += tetragraphs[t1*17576+t2*676+t3*26+t4]; }
		
					if(IS_LETTER(t5)) {
						if(c<length_of_cipher-4 && (use_graphs & USE_PENTA)) 
							{ pentascore += pentagraphs[t1*456976+t2*17576+t3*676+t4*26+t5]; }
					}
				}
			}
		}
		
		t1=t2; t2=t3; t3=t4; t4=t5; t5=solv[c+5]-'A';
	}

	biscore=biscore>>3; triscore=triscore>>2; tetrascore=tetrascore>>1; //	pentascore=pentascore>>0;

	score=pentascore+tetrascore+triscore+biscore;
	score-=int(ioc_weight*10000*ABS(IoC(solv)-lang_ioc));

//	printf("2graph: %d - 3graph: %d - 4graph: %d 5graph: %d\n",biscore,triscore,tetrascore,pentascore);	//FOR VALUE TESTING PURPOSES
	
	return(score);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                       Calculate the "Longest String Of Consonants"                           //
//////////////////////////////////////////////////////////////////////////////////////////////////

inline int calclsoc(const int length_of_cipher,const char *solv) {

	int lsoc=0,lsocmax=0;
	int	lsocdata[26]={0,1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1};				//LONGEST STRING OF CONSONANTS DATA

	for(int i=0;i<length_of_cipher;i++) {
		if(lsocdata[solv[i]-'A']) { lsoc++; if(lsoc>lsocmax) lsocmax=lsoc; }
		else lsoc=0; }

	return(lsocmax);

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//              Mutate the char array "key[]" by swapping two unlocked letters                  //
//////////////////////////////////////////////////////////////////////////////////////////////////

inline void shufflekey(char *key,const int keylength,const int cuniq,const char locked[],const char *exclude) {

	int x,y,z,canswap=0;

	//check if all characters are locked to avoid infinite loop
	for(int symbol=0; symbol<keylength; symbol++) if(!locked[symbol]) { canswap=1; break; }

	if(canswap)
	{
		do {x=genrand_int32()%keylength;} while(locked[x]);
		do {y=genrand_int32()%keylength;} while(locked[y]);

		/*exclusions*/
		if(exclude)
		{
			if(x<cuniq && strchr(exclude+(27*x),key[y])) return; 
			if(y<cuniq && strchr(exclude+(27*y),key[x])) return;
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

//////////////////////////////////////////////////////////////////////////////////////////////////
//            Print the character frequency table of the cipher and a few statistics            //
//////////////////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////////////////
//           Return the value of a unigraph for use in other areas of the program                //
//////////////////////////////////////////////////////////////////////////////////////////////////

void GetUnigraphs(double *dest) {memcpy(dest,unigraphs,26*sizeof(double));}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                          Set the IoC & multipler for use in the hillclimber                              //
//////////////////////////////////////////////////////////////////////////////////////////////////

void SetIoC(float ioc) {lang_ioc=ioc;}
void SetIoCWeight(int weight) {ioc_weight=weight;}

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
	if(n==2) {ngraphs=bigraphs; nsize=BI_SIZE;}
	if(n==3) {ngraphs=trigraphs; nsize=TRI_SIZE;}
	if(n==4) {ngraphs=tetragraphs; nsize=TETRA_SIZE;}
	if(n==5) {ngraphs=pentagraphs; nsize=PENTA_SIZE;}

	//init to zero
	if(n>1) memset(ngraphs,0,nsize*sizeof(long));

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

int WordPlug(Message &msg, const char *word, int use_graphs)
{
 	const char *cipher, *plain;
	int word_len, msg_len, cur_score=0, best_score=0;
	int act_freq[26], exp_freq[26], above, fail, old_ioc_weight;
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
	old_ioc_weight=ioc_weight;
	ioc_weight=0;

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
		above=0;
	
		//for(int letter=0; letter<26; letter++)
		//	if(act_freq[letter]>2*exp_freq[letter]) fail=true;
			//if(act_freq[letter]>exp_freq[letter]) 
				//above+=act_freq[letter]-exp_freq[letter];
			
		//if(above>.1*msg_len) continue;
		
		if(fail) continue;
		
		//compare score
		cur_score=calcscore(msg_len,plain,use_graphs);
		
		if(cur_score>best_score)
		{
			best_score=cur_score;
			best_map=msg.cur_map;
		}
	}

	msg.cur_map=best_map;

	//restore old ioc weight
	ioc_weight=old_ioc_weight;
	
	return best_score;
}
