//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Written by Brax Sisco (Developed starting 9/14/2006)                                                                                                     //
// This program attempts to solve the November 8th, 1969 unsolved Zodiac 340 character cipher                                                               //
//                                                                                                                                                          //
// Big thanks to Chris McCarthy for many good ideas and saving me a lot of work in converting the RayN and Zodiac 340 ciphers to ASCII                      //
// Also thanks to Glen from the ZK message board (http://www.zodiackiller.com/mba/zc/121.html) for an ASCII encoding of the solved 408 cipher.              //
//                                                                                                                                                          //
// This variant of my zodiac program uses a "genetic algorithm" of my own design to solve homophonic substitution ciphers.  It works, eventually.           //
// Program has been adapted to use the "Mersenne Twister" PRNG because of its speed and long period.                                                        //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2007 Brax Sisco
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
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif 
#define  _CRT_SECURE_NO_WARNINGS 1
#include "z340.h"
#include "z340Globals.h"
#include "mt19937ar-cok.cpp"

int hillclimb(char ciph[], char key[],int len, wxFrame * frm)
{

//	STARTING KEYS - CHOOSE ONE FROM BELOW, OR USE YOUR OWN.  UNCOMMENT ONLY ONE OR PROGRAM WILL NOT COMPILE!
//	KEY *MUST* CONTAIN AT *LEAST* AS MANY LETTERS AS THE CIPHER CONTAINS UNIQUE CHARACTERS.  LONGER KEYS ARE OK!
	//char key = ke;
/*************************************************************************************************************************************************************/
//	char key[ASCII_SIZE]="MILGLUITCEIOEBIEHEHTTWYTERRONFAPLEOOVNAESNSSKARSNDFAD";			//CORRECT 53-CHAR KEY FOR SOLVED ZODIAC 408 CIPHER
//	char key[ASCII_SIZE]="LIMGLUITCEIOEBIEHEHTTOOVNAESWYTERRONFAPLENSSKARSNDFAD";			//MIXED 53-CHAR KEY FOR SOLVED ZODIAC 408 CIPHER
//	char key[ASCII_SIZE]="NUTAFOAIYRPECEDONAETSFEOSHKTMIEISEBLVNATGSHNLWDLIRREO";			//*REALLY* SCRAMBLED 408 KEY
/*************************************************************************************************************************************************************/
//	char key[ASCII_SIZE]="EYENAGMNDREEDNRSWREETFCTTHHTFILVTLOOOIHAOAASSSOBHUKPUPJ";		//CORRECT 55-CHAR KEY FOR SOLVED RAY_N 378 CIPHER
//	char key[ASCII_SIZE]="EYENAGMNDREEDNRSWREETLOOOIHAOAASSSOBHTFCTTHHTFILVUKPUPJ";		//MIXED 55-CHAR KEY FOR SOLVED RAY_N 378 CIPHER
//	char key[ASCII_SIZE]="TAESMOIEOVETANSLIRPFTENNDDUUSELRYLIASETWOBCHGJKMNYOHCTR";           //*REALLY* SCRAMBLED 378 KEY
//	char key[ASCII_SIZE]="ESENADESTREETNRSNNOITFSTDHRTWALVCLBOOAHIOEIKYCYMLUGMUPJ";		//PROGRAM-IMPROVED KEY (4416)
//	char key[ASCII_SIZE]="BCFGJKMPQVWXYZELEEEETTTTTAAAAELOOLOIIIINNNNSOSSOHHRRRDDDASUU";      //KEY THAT WORKED IN SOLVING 378
/*************************************************************************************************************************************************************/
//                          +BpcOIFz2R5lMK(^WVLG<.!ykdUTNC4-)#tfZYSJHD>98b_PE;761/qjXA:3&%@
//	char key[ASCII_SIZE]="BCFFGJKMPWVWNYIELTAOTTTSTTAAAELOOLOIIIINNNNSOSSOHHRRRDDDASUUYEA";   //VARIOUS 340 KEY ATTEMPTS
//	char key[ASCII_SIZE]="BCFFGJKMPQVWNYIELEEETTTTTTAAAELOOLOIIIINNNNSOSSOHHRRRDDDASUUYEA";
//	char key[ASCII_SIZE]="TTAAISTISEOMTITAIENAESDOSSENWLLHRNSRAPEHNUBUIODSDFYATLKUFSGRLYC";
//	char key[ASCII_SIZE]="LNAESNSTRONFAPLSTOMGNDFAUSHTTWYTTRISDDASILULICEIOEBIEHSKARSOYEA";
//	char key[ASCII_SIZE]="ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPRSTUWYEEETTTAAAOOIINN";
//	char key[ASCII_SIZE]="TSREEESTNEISNOTAATSRAITSIDTSIDNOHAIHLLLDAEWNYPSFLUSKMGFARCBUOYU";
//	char key[ASCII_SIZE]="ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPRSTUVWYADEGHILMNOPRSTUWEEETTAAOOII";
/*************************************************************************************************************************************************************/
//	char key[ASCII_SIZE]="TNSSNSTRONFAPASEMIEDFAUHHLEWYILTRIDDVHGLULICOIORBWAHMKPRCOYUBETAOSHDLUBCFGJKMEE"; //330 KEY
//	char key[ASCII_SIZE]="AAAABCDDEEEEEEEFFGHHHHIIIIJKLLLMNNNNOOOOOPPRRRSSSSTTTTTUUVWY";	//ONE KEY FOR 378 AND 408
/*************************************************************************************************************************************************************/

	int x,y,clength,cuniq;
	int uniq[ASCII_SIZE],uniqarr[ASCII_SIZE];
	char solved[MAX_CIPH_LENGTH],solvedsav[MAX_CIPH_LENGTH];
	char keysav[ASCII_SIZE],uniqstr[ASCII_SIZE],bestkey[ASCII_SIZE];

	for(int i=0;i<MAX_CIPH_LENGTH;i++) cipher[i]=solved[i]=solvedsav[i]=0;						//INITIALIZE (ZERO) ARRAYS
	for(int i=0;i<ASCII_SIZE;i++) uniq[i]=uniqstr[i]=uniqarr[i]=0;

	strcpy(bestkey,key);
	keylength=(int)strlen(key);
	char filename[1024]="408.ascii.txt";

	init_genrand((unsigned long)time(NULL));											//SEED RANDOM GENERATOR

	if(SCRAMBLESTARTKEY)
		for(int i=0;i<100000;i++)
			shufflekey(key);

	strcpy(cipher,ciph);
	//if(argc==2) strcpy(filename,argv[1]);
	clength = len;//readcipher(filename);

	for(int i=0;i<26;i++)
		freqs[i]=(freqs[i]*clength+48999)/100000;


	for(int i=0;i<clength;i++) ++uniq[(int)cipher[i]];									//COUNT # OF UNIQUE CHARS IN CIPHER

	int i=255,j=0;
	for(y=0;y<255;y++) { 
		for(x=255;x>0;x--)
			{ if(uniq[x]==i) { uniqstr[j]=x; uniqarr[j++]=i; } } i--;}

	cuniq=(int)strlen(uniqstr);
	if(keylength < cuniq)
		{ printf("\nKEYLENGTH ERROR!! -- Key is TOO SHORT\n\n"); return(-1); }

	printf("\nZodiac Code Decipher v%s\n-------------------------\n\n",VERSION);					//PRINT VERSION NUMBER
	printf("Parsing Cipher: %s\n",filename);											//PRINT FILENAME
	printf("Cipher Length:  %d characters\n",clength);									//PRINT CIPHER LENGTH
	printf("Cipher Uniques: %d unique characters\n\n",cuniq);								//PRINT NUMBER OF UNIQUE CHARACTERS
	read_ngraphs();															//READ IN THE N-GRAPH DATA
	printfrequency(clength,uniqarr,uniqstr);

	SETSOLVED;

/****************************** START_MAIN_HILLCLIMBER_ALGORITHM **********************************/

	int score = 0;
	int bestscore = 0;
	int iterations = 0;

	while(1) {

	for(int p1=0;p1<keylength;p1++) {
		for(int p2=0;p2<keylength;p2++) {

		if((score=(calcscore(clength,solved)))>bestscore) {
			printcipher(clength,cipher,solved);
			printvowels(clength,solved);
			printf("Best Score = %d\n",bestscore=score);
			//pnl->Pos
			//cfv->PostMessageA(WM_USER_THREAD_UPDATE_BESTSCORE,score,0);
			//cfv->PostMessageA(WM_USER_THREAD_UPDATE_PLAINTEXT,(WPARAM)solved,0);

			wxCommandEvent upt(EVT_UpdatePlainText,Plain_Text);
			upt.SetString(wxString(solved));
			frm->AddPendingEvent(upt);

			wxCommandEvent upt2(EVT_UpdateBestKey,Best_Key);
			upt2.SetString(key);
			frm->AddPendingEvent(upt2);

			wxCommandEvent upt3(EVT_UpdateScore,Score_);
			upt3.SetInt(score);
			frm->AddPendingEvent(upt3);


			strcpy(bestkey,key);
			printf("\nKey: '%s'\n\n",key); 
			}

		strcpy(keysav,key); strcpy(solvedsav,solved);
		int temp=key[p1]; key[p1]=key[p2]; key[p2]=temp;
		SETSOLVED;

		if((calcscore(clength,solved))<score) { strcpy(key,keysav); strcpy(solved,solvedsav); }

		}

	}

//	int z=key[0] ; for(int x=0;x<strlen(key)-1;x++) key[x]=key[x+1];		// THIS IS JUST EXPERIMENTAL
//	key[strlen(key)-1]=z; // shufflekey(key); 					//
//	printf("\n%s",key);									//

	for(int i=0;i<5;i++) shufflekey(key);	// THE '5' IS ARBITRARY, BUT SEEMS TO WORK REALLY WELL

	iterations++; if(iterations>120*5-1) { printf("*"); strcpy(key,bestkey); iterations=0; }
	SETSOLVED;

//	printf(".");
	}
}

/******************************* END_MAIN_HILLCLIMBER_ALGORITHM ***********************************/

//////////////////////////////////////////////////////////////////////////////////////////////////
//          Calculate a 'fitness' score for the solution based on the N-Graph counts            //
//////////////////////////////////////////////////////////////////////////////////////////////////

inline int calcscore(const int length_of_cipher,const char *solv) {

	int t1,t2,t3,t4,t5;
	int biscore=0,triscore=0,tetrascore=0,pentascore=0;

	t1=solv[0]-'A'; t2=solv[1]-'A'; t3=solv[2]-'A'; t4=solv[3]-'A'; t5=solv[4]-'A';

	for(int c=0; c<length_of_cipher; c++) {

		if(c<length_of_cipher-1) { biscore += bigraphs[t1*26+t2]; }
		if(c<length_of_cipher-2) { triscore += trigraphs[t1*676+t2*26+t3]; }
		if(c<length_of_cipher-3) { tetrascore += tetragraphs[t1*17576+t2*676+t3*26+t4]; }
		if(c<length_of_cipher-4) { pentascore += pentagraphs[t1*456976+t2*17576+t3*676+t4*26+t5]; }

		t1=t2; t2=t3; t3=t4; t4=t5; t5=solv[c+5]-'A';
	}

	biscore=biscore>>5;
	triscore=triscore>>4;
	tetrascore=tetrascore>>3;
	pentascore=pentascore>>2;

//	printf("2graph: %d - 3graph: %d - 4graph: %d 5graph: %d\n",biscore,triscore,tetrascore,pentascore);	//FOR VALUE TESTING PURPOSES

	return((pentascore+tetrascore+triscore+biscore));

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                       Calculate the "Longest String Of Consonants"                           //
//////////////////////////////////////////////////////////////////////////////////////////////////

inline int calclsoc(const int length_of_cipher,const char *solv) {

	int lsoc=0,lsocmax=0;

	for(int i=0;i<length_of_cipher;i++) {
		if(lsocdata[solv[i]-'A']) { lsoc++; if(lsoc>lsocmax) lsocmax=lsoc; }
		else lsoc=0; }

	return(lsocmax);

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                                Mutate the char array "key[]"                                 //
//////////////////////////////////////////////////////////////////////////////////////////////////

inline void shufflekey(char *key) {

	int x,y,z;

	z=key[x=(genrand_int32()%keylength)];
	key[x]=key[y=(genrand_int32()%keylength)];
	key[y]=z;

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                        Print ERROR MESSAGE when file can not be opened                       //
//////////////////////////////////////////////////////////////////////////////////////////////////

void printferror(char *name_of_file) {

	printf("ERROR - File '%s' does not exist, or could not be opened!!\n\n",name_of_file);

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                        Print the cipher "block" and the solution "block"                     //
//////////////////////////////////////////////////////////////////////////////////////////////////

void printcipher(int length_of_cipher,char *ciph,char *solv) {

	int c=0;
	int s=0;
	int width;
	int height;

	switch(length_of_cipher) {
		case 330: { width=30; height=11; } break;
		case 340: { width=17; height=20; } break;
		case 378: { width=18; height=21; } break;
		case 408: { width=17; height=24; } break;
		default: { printf("Sorry, this program will (currently) only work on 330, 340, 378, and 408 ciphers\n"); exit(1); } }

	printf("\n--------------------------------------------------------------------------------------------------------------------\n\n");

	for(int y=0;y<height;y++) { 
		for(int x=0;x<width;x++) printf("%c",ciph[c++]);
		printf("   =   ");
		for(int x=0;x<width;x++) printf("%c",solv[s++]);
		printf("\n"); }

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//            Print the character frequency table of the cipher and a few statistics            //
//////////////////////////////////////////////////////////////////////////////////////////////////

void printfrequency(int length_of_cipher, int *unique_array,char *unique_string) {

	int f=0;
	int z=(int)strlen(unique_string);
	char zee[10];

	printf("Frequency Table for Cipher:\n");
	for(int i=0;i<z;i++) printf("-"); printf("\n");
	for(int i=0;i<z;i++) if(unique_array[i]/100 != 0) printf("%1d",unique_array[i]/100); if(unique_array[0]>=100) printf("\n");
	for(int i=0;i<z;i++) { sprintf(zee,"%d",unique_array[i]); if(unique_array[i]/10 != 0) printf("%c",zee[strlen(zee)-2]); } printf("\n");
	for(int i=0;i<z;i++) { printf("%1d",unique_array[i] % 10); f = f + (unique_array[i] * (unique_array[i]-1)); } printf("\n");
	for(int i=0;i<z;i++) printf("-");

	printf("\n%s\n\n",unique_string);

	printf("Phi(O) = %i\n",f);
	printf("Phi(P) = %f\n",(.0675) * length_of_cipher * (length_of_cipher - 1));
	printf("Phi(R) = %f\n\n",(.0385) * length_of_cipher * (length_of_cipher - 1));
	printf("DeltIC = %f\n\n",f/((.0385) * length_of_cipher * (length_of_cipher - 1)));

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                 Calculate and print the percentage of vowels in the solution                 //
//                NOTE: Normal English text normally contains approx. 40% vowels                //
//////////////////////////////////////////////////////////////////////////////////////////////////

void	printvowels(int length_of_cipher, char *solv) {

	int y,solv_freqs[26]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int diff_tot=0;
	for(int i=0;i<length_of_cipher;i++) solv_freqs[solv[i]-'A']++;
	printf("\n\n              'A   B   C   D  'E   F   G   H  'I   J   K   L   M   N  'O   P   Q   R   S   T  'U   V   W   X   Y   Z");
	printf("\n  Expected: ");	for(int i=0;i<26;i++) printf("%4d",freqs[i]);
	printf("\n     Found: ");	for(int i=0;i<26;i++) printf("%4d",solv_freqs[i]);
	printf("\nDifference: ");	for(int i=0;i<26;i++) { printf("%4d",y=abs(freqs[i]-solv_freqs[i])); diff_tot+=y; }
	printf("\n\nDifference Total: %d    --    Deviation From Expected: %f",diff_tot,100*((float)diff_tot/length_of_cipher));
	printf("\n\nVowel Pcg. = %f    --    ",100*((solv_freqs[0]+solv_freqs[4]+solv_freqs[8]+solv_freqs[14]+solv_freqs[20])/(float)length_of_cipher));

	printf("Longest String Of Consonants: %d\n\n",calclsoc(length_of_cipher,solv));

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                  Read in the ciphertext into global array "cipher[]"                         //
//                  RETURN: The length of the cipher that has been read                         //
//////////////////////////////////////////////////////////////////////////////////////////////////

int readcipher(char *filename) {

	FILE *ifptr;
	ifptr=fopen(filename,"rb");
	if(ifptr==NULL) { printferror(filename); exit(1); }
	fgets(cipher,MAX_CIPH_LENGTH,ifptr);
	fclose(ifptr);
	return((int)strlen(cipher));

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//  Read the N-Graph data into global arrays "bi...", "tri...", "tetra..." and "pentagraphs[]"  //
//////////////////////////////////////////////////////////////////////////////////////////////////

void read_ngraphs(void) {

	FILE *ifptr;
	int t1,t2,t3,t4,t5,gtemp;
	char temp_string[500];

	// INITIALIZE (ZERO) N-GRAPH ARRAYS
	for(int i=0;i<(26*26);i++) bigraphs[i]=0;
	for(int i=0;i<(26*26*26);i++) trigraphs[i]=0;
	for(int i=0;i<(26*26*26*26);i++) tetragraphs[i]=0;
	for(int i=0;i<(26*26*26*26*26);i++) pentagraphs[i]=0;

	// READ "BIGRAPHS.TXT"
	ifptr=fopen("eng/bigraphs.txt","r"); if(ifptr==NULL) { printferror("bigraphs.txt"); exit(1); }
	while(!feof(ifptr)) {
		t1=fgetc(ifptr)-'A'; t2=fgetc(ifptr)-'A';
		fgetc(ifptr); fgetc(ifptr); fgetc(ifptr);
		fscanf(ifptr,"%i",&gtemp); fgets(temp_string,500,ifptr);
		if(t1+'A'=='*') break;
		if(_DEB) printf("\nBiGraph: %c%c  Count: %4i  Index: %i",t1+'A',t2+'A',gtemp,(t1*26+t2));
		bigraphs[t1*26+t2]=(int)(10*log((double)gtemp)); }
	if(_DEB) printf("\n");
	fclose(ifptr);
	
	// READ "TRIGRAPHS.TXT"
	ifptr=fopen("eng/trigraphs.txt","r"); if(ifptr==NULL) { printferror("trigraphs.txt"); exit(1); }
	while(!feof(ifptr)) {
		t1=fgetc(ifptr)-'A'; t2=fgetc(ifptr)-'A'; t3=fgetc(ifptr)-'A';
		fgetc(ifptr); fgetc(ifptr); fgetc(ifptr);
		fscanf(ifptr,"%i",&gtemp); fgets(temp_string,500,ifptr);
		if(t1+'A'=='*') break;
		if(_DEB) printf("\nTriGraph: %c%c%c  Count: %4i  Index: %i",t1+'A',t2+'A',t3+'A',gtemp,(t1*676+t2*26+t3));
		trigraphs[t1*676+t2*26+t3]=(int)(10*log((double)gtemp)); }
	if(_DEB) printf("\n");
	fclose(ifptr);

	// READ "TETRAGRAPHS.TXT"
	ifptr=fopen("eng/tetragraphs.txt","r"); if(ifptr==NULL) { printferror("tetragraphs.txt"); exit(1); }
	while(!feof(ifptr)) {
		t1=fgetc(ifptr)-'A'; t2=fgetc(ifptr)-'A'; t3=fgetc(ifptr)-'A'; t4=fgetc(ifptr)-'A';
		fgetc(ifptr); fgetc(ifptr); fgetc(ifptr);
		fscanf(ifptr,"%i",&gtemp); fgets(temp_string,500,ifptr);
		if(t1+'A'=='*') break;
		if(_DEB) printf("\nTetraGraph: %c%c%c%c  Count: %4i  Index: %i",t1+'A',t2+'A',t3+'A',t4+'A',gtemp,(t1*17576+t2*676+t3*26+t4));
		tetragraphs[t1*17576+t2*676+t3*26+t4]=(int)(10*log((double)gtemp)); }
	if(_DEB) printf("\n");
	fclose(ifptr);

	// READ "PENTAGRAPHS.TXT"
	ifptr=fopen("eng/pentagraphs.txt","r"); if(ifptr==NULL) { printferror("pentagraphs.txt"); exit(1); }
	while(!feof(ifptr)) {
		t1=fgetc(ifptr)-'A'; t2=fgetc(ifptr)-'A'; t3=fgetc(ifptr)-'A'; t4=fgetc(ifptr)-'A'; t5=fgetc(ifptr)-'A';
		fgetc(ifptr); fgetc(ifptr); fgetc(ifptr);
		fscanf(ifptr,"%i",&gtemp); fgets(temp_string,500,ifptr);
		if(t1+'A'=='*') break;
		if(_DEB) printf("\nPentaGraph: %c%c%c%c%c  Count: %4i  Index: %i",t1+'A',t2+'A',t3+'A',t4+'A',t5+'A',gtemp,(t1*456976+t2*17576+t3*676+t4*26+t5));
		pentagraphs[t1*456976+t2*17576+t3*676+t4*26+t5]=(int)(10*log((double)gtemp)); }
	if(_DEB) printf("\n");
	fclose(ifptr);

}
