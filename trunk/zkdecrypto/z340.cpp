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

int hillclimb(Message &msg, const char cipher[],int clength,char key[],SOLVEINFO &info, int print)
{
	#define	DO_SWAP	{ unsigned char temp=key[p1]; key[p1]=key[p2]; key[p2]=temp; }
	#define DECODE {for(y=clength;y--;) solved[y]=*decoder[(unsigned char)cipher[y]];}

	int cuniq,keylength,i,j,x,y;
	int uniq[ASCII_SIZE],uniqarr[ASCII_SIZE];
	char *solved=new char[clength+1];
	char uniqstr[ASCII_SIZE];
	char *decoder[ASCII_SIZE];

	for(i=clength;i--;) solved[i]=0;										//INITIALIZE (ZERO) ARRAYS
	for(i=ASCII_SIZE;i--;) uniq[i]=uniqstr[i]=uniqarr[i]=0;

	for(i=clength;i--;) ++uniq[(unsigned char)cipher[i]];						//COUNT # OF UNIQUE CHARS IN CIPHER

	i=4096; j=0;																		//CALCULATE AND SORT THE CIPHER UNIQUES
	for(y=0;y<4096;y++) { 
		for(x=255;x>0;x--)
			{ if(uniq[x]==i) { uniqstr[j]=x; uniqarr[j++]=i; } } i--;}

	cuniq=(int)strlen(uniqstr);
	keylength=(int)strlen(key);
	
	if(keylength < cuniq)      //THIS SHOULD NEVER HAPPEN
		{ printf("\nKEYLENGTH ERROR!! -- Key is TOO SHORT\n\n"); return(-1); } 

	//if (print) printfrequency(clength,uniqarr,uniqstr,cuniq);
	
	//make decoder, array of char* that point to the key plain text values
	//indexed by the ascii value of the cipher symbols
	//this makes decoding much faster, since only one loop and no compare is required
	for(x=cuniq;x--;) decoder[(unsigned char)uniqstr[x]]=&key[x];
	DECODE;

/****************************** START_MAIN_HILLCLIMBER_ALGORITHM **********************************/

	int score = 0, last_score=0, iterations = 0, improve = 0;
	long start_time=0, end_time=0;
	info.cur_try=0;
	info.cur_fail=0;

	//initial score & feedback
	last_score=calcscore(msg,clength,solved,info);
	info.best_score=last_score;
	memcpy(info.best_key,key,keylength);
	if(info.disp_all) info.disp_all();

	//go until max number of iterations or stop is pressed
	for(info.cur_try=0; info.cur_fail<info.max_fail; info.cur_try++) {
		
		/*feedback info*/
		info.cur_try=iterations;
		info.last_time=float(end_time-start_time)/1000;
		if(info.time_func) start_time=info.time_func();
		if(info.disp_info) info.disp_info();
		
		improve=0;
		
		for(int p1=0; p1<keylength; p1++) {

			if(info.locked[p1]) continue; //skip if symbol is locked

			for(int p2=0; p2<keylength; p2++) {
			
				if(!info.running) return 0; //stop

				if(p1>=cuniq && p2>=cuniq) continue; //don't bother swapping if both symbols are in the extra letters area
				if(info.locked[p2] || key[p1]==key[p2]) continue; //skip if symbol is locked or identical 
				
				if(info.exclude) //exclusions
				{
					if(p1<cuniq && strchr(info.exclude+(27*p1),key[p2])) continue; 
					if(p2<cuniq && strchr(info.exclude+(27*p2),key[p1])) continue;
				}

				//swap & decode
				DO_SWAP; 
				DECODE;
			
				score=calcscore(msg,clength,solved,info); 

				//undo if change made it worse than last score
				if(score<last_score) {DO_SWAP;} 
				else //change is better or same as last score
				{
					last_score=score;

					if(score>info.best_score) //this is the new best, save & display
					{
						//if (print) printcipher(clength,cipher,solved,score,key);
						
						//feedback info
						info.best_score=score;
						improve=1;
						info.cur_fail=0;
						memcpy(info.best_key,key,keylength);
						if(info.disp_all) info.disp_all();	
					}
				}
			}
		}

		// info.swaps IS INITIALIZED TO 5, WHICH IS ARBITRARY, BUT SEEMS TO WORK WELL
		for(i=info.swaps;i--;) shufflekey(key,keylength,cuniq,info);    

		if(++iterations>info.revert) { memcpy(key,info.best_key,keylength); iterations=0; }
		DECODE;
		last_score=calcscore(msg,clength,solved,info); 
		
		if(!improve) info.cur_fail++;
		
		if(info.time_func) end_time=info.time_func();

	}

	delete solved;

	return 0;
}

/******************************* END_MAIN_HILLCLIMBER_ALGORITHM ***********************************/

//////////////////////////////////////////////////////////////////////////////////////////////////
//          Calculate a 'fitness' score for the solution based on the N-Gram counts             //
//////////////////////////////////////////////////////////////////////////////////////////////////

#define IS_LETTER(L) ((L<0 || L>25)? 0:1)

/*inline float FastIoC(const char *string, int length)
{
	int index, freqs[256];
	float ic=0;
	
	memset(freqs,0,256*sizeof(int));
	
	for(index=length; index--;) 
		freqs[(unsigned char)string[index]]++;

	for(index=256; index--;)
		if(freqs[index]>1) 
			ic+=(freqs[index])*(freqs[index]-1); 

	ic/=(length)*(length-1);

	return ic;
}

inline float FastDIoC(const char* string, int length)
{
	int index, freqs[65536];
	float ic=0;
	
	memset(freqs,0,65536*sizeof(int));
	
	for(index=length-1; index--;)
		freqs[(unsigned char)string[index]*(unsigned char)string[index+1]]++;

	for(index=65536; index--;)
		if(freqs[index]>1) 
			ic+=(freqs[index])*(freqs[index]-1); 

	ic/=(length)*(length-1);

	return ic;
}

inline float FastChiSquare(const char *string)
{
	int freqs[256], length, unique;
	float chi2=0, prob_mass, cur_calc;

	if(!string) return 0;

	length=(int)strlen(string);
	
	if(length<0) return 0;

	unique=GetUniques(string,NULL,freqs);

	//calculate chi2
	for(int sym_index=0; sym_index<unique; sym_index++)
	{
		prob_mass=(float)(length*(1.0/unique));
		cur_calc=freqs[sym_index]-prob_mass;
		cur_calc*=cur_calc;
		cur_calc/=prob_mass;

		chi2+=cur_calc;
	}

	return chi2/length;
}
*/
inline int calcscore(Message &msg, const int length_of_cipher,const char *solv, SOLVEINFO &info) {

	int t1,t2,t3,t4,t5;
	int biscore=0,triscore=0,tetrascore=0,pentascore=0;
	int score, remaining;

	//get inital characters in for ngrams
	t1=solv[0]-'A'; t2=solv[1]-'A'; t3=solv[2]-'A'; t4=solv[3]-'A'; t5=solv[4]-'A';

	//letters in text left for ngrams
	remaining=length_of_cipher;

	for(int c=0; c<length_of_cipher-1; c++) 
	{
		//only score an ngram which is all letters,
		//enough characters remain in the text (i.e. not close to the end)

		if(IS_LETTER(t1) && IS_LETTER(t2)) {
             biscore+=bigraphs[t1][t2];

			if(IS_LETTER(t3) && remaining>2) {
                 triscore+=trigraphs[t1][t2][t3];
			
				if(IS_LETTER(t4) && remaining>3) {
                     tetrascore+=tetragraphs[t1][t2][t3][t4];
				
					if(IS_LETTER(t5) && remaining>4)
                         pentascore+=pentagraphs[t1][t2][t3][t4][t5];
				}
			}
		}
		
		t1=t2; t2=t3; t3=t4; t4=t5; t5=solv[c+5]-'A'; //shift letters & get next
		remaining--; 
	}

	biscore=biscore>>3; triscore=triscore>>2; tetrascore=tetrascore>>1;

	score=pentascore+tetrascore+triscore+biscore;

	
	//Letter Frequencies
	if(info.freq_weight)
	{

		int lprgiActFreq[26], lprgiExpFreq[26], letter;
		float freq_diff=0;
		
		memset(lprgiActFreq,0,26*sizeof(int));
		for(letter=0; letter<length_of_cipher; letter++) lprgiActFreq[solv[letter]-'A']++;
		msg.GetExpFreq(lprgiExpFreq);
		for(letter=0; letter<26; letter++) freq_diff+=ABS(lprgiExpFreq[letter]-lprgiActFreq[letter]);

		score*=(float)1.05-((info.freq_weight/500)*freq_diff);
	}
	

	//IC
	float cur_ioc=IoC(solv,length_of_cipher);
    float score_mult=(float)1.05-(info.ioc_weight*ABS(cur_ioc-info.lang_ioc));
    score=int(score*score_mult);

/*          This is ONLY valid for English, and REALLY slow, so I have commented it out
	//DIC
	cur_ioc=DIoC(solv,length_of_cipher);
    score_mult=(float)1.05-(info.ioc_weight*ABS(cur_ioc-.0078));
    score=int(score*score_mult);
*/
        
	//Chi^2
    float cur_chi=ChiSquare(solv,length_of_cipher);
    score_mult=(float)(1.05-(((float)info.chi_weight/100.0)*ABS(cur_chi-(float).52)));
    score=int(score*score_mult);

	//Entropy
	float cur_ent=Entropy(solv,length_of_cipher);
    score_mult=(float)(1.05-(((float)info.ent_weight/100.0)*ABS(cur_ent-(float)4.1)));
    score=int(score*score_mult);

	//Cribs
	for(int cur_crib=0; cur_crib<info.num_cribs; cur_crib++)
		if(strstr(solv,info.cribs[cur_crib])) score*=1.025;

//	printf("2graph: %d - 3graph: %d - 4graph: %d 5graph: %d\n",biscore,triscore,tetrascore,pentascore);	//FOR VALUE TESTING PURPOSES
	
	return(score);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//        Mutate the char array "key[]" by swapping two unlocked, unexcluded letters            //
//////////////////////////////////////////////////////////////////////////////////////////////////

inline void shufflekey(char *key,const int keylength,const int cuniq,SOLVEINFO &info) {

	int x,y,z,canswap=0;

	//check if all characters are locked to avoid infinite loop
	for(int symbol=keylength; symbol--;) if(!info.locked[symbol]) { canswap=1; break; }

	if(canswap)
	{
		do {x=rand()%keylength;} while(info.locked[x]);
		do {y=rand()%keylength;} while(info.locked[y]);

		/*exclusions*/
		if(info.exclude)
		{
			if(x<cuniq && strchr(info.exclude+(27*x),key[y])) return; 
			if(y<cuniq && strchr(info.exclude+(27*y),key[x])) return;
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

int WordPlug(Message &msg, const char *word, SOLVEINFO &info)
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
	old_ioc_weight=info.ioc_weight;
	info.ioc_weight=0;
	info.exclude=NULL;

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
		cur_score=calcscore(msg,msg_len,plain,info);
		
		if(cur_score>best_score)
		{
			best_score=cur_score;
			best_map=msg.cur_map;
		}
	}

	msg.cur_map=best_map;

	//restore old ioc weight
	info.ioc_weight=old_ioc_weight;
	
	return best_score;
}

//K3
//int key_n[]={ 5<<8 | 40, 4<<8 | 40, 3<<8 | 40, 2<<8 | 40, 1<<8 | 40, 0<<8 | 40, 7<<8 | 41,  6<<8 | 41, 5<<8 | 41,  4<<8 | 41,  3<<8 | 41,  2<<8 | 41, 1<<8 | 41, 	0<<8 | 41};
	
//K4 14 cols
//int key[]={4<<8 | 12, 3<<8 | 12, 2<<8 | 12, 1<<8 | 12, 	0<<8 | 12, 5<<8 | 11, 6<<8 | 13,  5<<8 | 13,  4<<8 | 13,  3<<8 | 13,  2<<8 | 13, 1<<8 | 13, 0<<8 | 13, 5<<8 | 12};

//K3 7 Cols
//int key[]={3<<8 | 6, 10<<8 | 6, 1<<8 | 6, 2<<8 | 6, 8<<8 | 6, 7<<8 | 6, 11<<8 | 6,  6<<8 | 6,  4<<8 | 6,  0<<8 | 6, 5<<8 | 6, 12<<8 | 6,  13<<8 | 6, 9<<8 | 6};	

/*for(int ki=0; ki<14; ki++)
{

}*/

void KryptosMatrix4(char *cipher, char *solved, int *key, int iLineChars, int enc_dec)
{
	int cipher_len=strlen(cipher);
	int iKeyIndex=0, iNewIndex, iLines;
	int cur_row, cur_col=-1;

	memset(solved,1,cipher_len);
	
	if(cipher_len==336) {iLineChars=42;}
	//if(cipher_len==98) {iLineChars=7;}

	iLines=cipher_len/iLineChars;

	cur_col=key[iKeyIndex] & 0x000000FF;
	cur_row=key[iKeyIndex]>>8 & 0x000000FF;
	if(++iKeyIndex==14) iKeyIndex=0;
	
	for(int iCipherIndex=0; iCipherIndex<cipher_len; iCipherIndex++)
	{
		//set cipher//plain character
		iNewIndex=iLineChars*cur_row+cur_col;

		if(enc_dec) {if(solved[iNewIndex]!=1) continue; solved[iNewIndex]=cipher[iCipherIndex];}
		else solved[iCipherIndex]=cipher[iNewIndex];

		if(iLines>8) //K4 7 cols
		{
			cur_col--;
			cur_row-=8;
			if(cur_row<0) cur_row+=iLines;
		}

		else //K3
		{
			cur_row-=2; cur_col-=2; //move to next hole

			if(cur_row<0) 
			{
				cur_row+=iLines;
				
				if((iLines%2)) //odd # lines 
				{
					if(cur_row==iLines-1) {cur_row--; cur_col--;} //on the last line, go up & left
					else if(cur_row==iLines-2) {cur_row++; cur_col++;} //on next to last, down & right
				}
				
				else cur_col++;	//just go right on even # rows
			}
		}

		if(cur_col<0) //start next shift position
		{
			cur_col=key[iKeyIndex] & 0x000000FF;
			cur_row=key[iKeyIndex]>>8 & 0x000000FF;
			if(++iKeyIndex==14) iKeyIndex=0;
		} 
	}

	solved[cipher_len]='\0';
}

#define MSG_SWAP switch(solve_type) { \
					case SOLVE_VIG:	case SOLVE_BIFID: case SOLVE_TRIFID: case SOLVE_KRYPTOS: temp=key[p1]; key[p1]=key[p2]; key[p2]=temp; break; \
					case SOLVE_ANAGRAM:		temp=cipher[p1]; cipher[p1]=cipher[p2]; cipher[p2]=temp;; break; \
					case SOLVE_COLTRANS:	SwapStringColumns(cipher,p1,p2,iLineChars); break;}

#define MSG_DECODE	switch(solve_type) { \
					case SOLVE_VIG:			msg.SetKey(key); msg.DecodeVigenere(); break; \
					case SOLVE_BIFID:		strcpy(msg.bifid_array,key); msg.DecodeXfid(2); break; \
					case SOLVE_TRIFID:		strcpy(msg.trifid_array,key); msg.DecodeXfid(3); break; \
					case SOLVE_ANAGRAM:		msg.DecodeHomo(); break; \
					case SOLVE_COLTRANS:	msg.DecodeHomo(); break; \
					case SOLVE_KRYPTOS:		KryptosMatrix4(cipher,solved,(int*)key,iLineChars,1); strcpy(cipher,solved); msg.DecodeHomo(); break;}
		
int hillclimb2(Message &msg, SOLVEINFO &info, int solve_type, char *key , int iLineChars)
{
	int i, p1, p2, clength, temp;
	int score=0, last_score=0, iterations=0, improve=0;
	long start_time=0, end_time=0;
	char *cipher, *solved;
	int use_key_len=msg.GetKeyLength();
	int full_key_len=strlen(key);

	info.cur_try=0;
	info.cur_fail=0;

	clength=msg.GetLength();
	cipher=msg.GetCipher();
	solved=msg.GetPlain();

	full_key_len=strlen(key);

	switch(solve_type)
	{
		case SOLVE_BIFID: use_key_len=full_key_len; break;
		case SOLVE_TRIFID: use_key_len=full_key_len; break;
		case SOLVE_ANAGRAM: use_key_len=full_key_len=clength; break;
		case SOLVE_COLTRANS: use_key_len=full_key_len=iLineChars<<1; break;
		case SOLVE_KRYPTOS: use_key_len=full_key_len=14; break;
	}
 
	info.best_trans=new char[clength+1];

	key[full_key_len]='\0';

	//initial score, save best, & feedback
	MSG_DECODE;
	last_score=calcscore(msg,clength,solved,info);
	info.best_score=last_score;
	strcpy(info.best_key,key);
	strcpy(info.best_trans,cipher);
	if(info.disp_all) info.disp_all();

	//go until max number of iterations or stop is pressed
	for(info.cur_try=0; info.cur_fail<info.max_fail; info.cur_try++) {
		
		//feedback info
		info.cur_try=iterations;
		info.last_time=float(end_time-start_time)/1000;
		if(info.time_func) start_time=info.time_func();
		if(info.disp_info) info.disp_info();
		
		improve=0;
		
		for(p1=0; p1<full_key_len; p1++) {
			for(p2=0; p2<full_key_len; p2++) {
			
				if(!info.running) return 0;	
				if(p1==p2) continue;
				if(p1>use_key_len && p2>use_key_len) continue;
			
				//swap, decode, score
				MSG_SWAP;
				MSG_DECODE; 
				score=calcscore(msg,clength,solved,info);

				if(score<last_score) {MSG_SWAP;} //undo if change made it worse than last score
	
				else //change is better or same as last score
				{
					last_score=score;

					if(score>info.best_score) //this is the new best, save & display
					{
						//save best, & feedback info
						improve=1;
						info.cur_fail=0;
						info.best_score=score;
						strcpy(info.best_key,key);
						strcpy(info.best_trans,cipher);
						if(info.disp_all) info.disp_all();	
					}
				}

				if(solve_type==90) return 0; //for some reason this has to be here, or there are weird errors in release
			}
		}

		// info.swaps IS INITIALIZED TO 5, WHICH IS ARBITRARY, BUT SEEMS TO WORK WELL
		for(i=info.swaps;i--;) 	{p1=rand()%full_key_len; p2=rand()%full_key_len; MSG_SWAP;}    

		if(++iterations>info.revert) { strcpy(key,info.best_key); /*strcpy(cipher,info.best_trans);*/ iterations=0; }
		MSG_DECODE;
		last_score=calcscore(msg,clength,solved,info);
		
		if(!improve) info.cur_fail++;
		
		if(info.time_func) end_time=info.time_func();
	}	
	
	return 0;
}


/*
void KryptosMatrix4(char *cipher, char *solved, char *key, int enc_dec)
{
	int cipher_len=strlen(cipher);
	int iKeyIndex=0, key_len=6;
	int iNewIndex=-1;
	int line_len=cipher_len/key_len;
	int line_diff;
	
	for(int iCipherIndex=0; iCipherIndex<cipher_len; iCipherIndex++)
	{
		if(iKeyIndex) line_diff=(key[iKeyIndex]-'0')-(key[iKeyIndex-1]-'0');
		else line_diff=(key[iKeyIndex]-'0');
		
		iNewIndex+=line_len*line_diff;
		if(iKeyIndex%2) iNewIndex--;

		if(iNewIndex<0) {iNewIndex+=cipher_len; iNewIndex++;}
		if(iNewIndex>=cipher_len) {iNewIndex-=cipher_len; iNewIndex--;}
			
		if(enc_dec) solved[iCipherIndex]=cipher[iNewIndex];
		else solved[iNewIndex]=cipher[iCipherIndex];

		if(++iKeyIndex==key_len) iKeyIndex=0;
	}

	solved[cipher_len]='\0';
}
*/
