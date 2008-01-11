
//	STARTING KEYS - CHOOSE ONE FROM BELOW, OR USE YOUR OWN.  UNCOMMENT ONLY ONE OR PROGRAM WILL NOT COMPILE!
//	KEY *MUST* CONTAIN AT *LEAST* AS MANY LETTERS AS THE CIPHER CONTAINS UNIQUE CHARACTERS.  LONGER KEYS ARE OK!
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

//////////////////////////////////////////////////////////////////////////////////////////////////
//                  Read in the ciphertext into global array "cipher[]"                         //
//                  RETURN: The length of the cipher that has been read                         //
//	 --DEAD CODE: This is now handled by the GUI part of the program                          //
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
//                                      Update the GUI       --DEAD CODE: wxWidgets is out      //
//////////////////////////////////////////////////////////////////////////////////////////////////
/*
void updateGUI(char *solved,char *bestkey,int bestscore,wxFrame *frm) {

			wxCommandEvent upt1(EVT_UpdatePlainText,Plain_Text);
			upt1.SetString(wxString(solved));
			frm->AddPendingEvent(upt1);

			wxCommandEvent upt2(EVT_UpdateBestKey,Best_Key);
			upt2.SetString(bestkey);
			frm->AddPendingEvent(upt2);

			wxCommandEvent upt3(EVT_UpdateScore,Score_);
			upt3.SetInt(bestscore);
			frm->AddPendingEvent(upt3);
}
*/

/*
BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(EVT_UpdatePlainText, -1)
DECLARE_EVENT_TYPE(EVT_UpdateBestKey, -1)
DECLARE_EVENT_TYPE(EVT_UpdateScore, -1)
END_DECLARE_EVENT_TYPES()

enum
{
	Cipher_Text = wxID_HIGHEST + 1, // declares an id which will be used to call our button
	Plain_Text,
	Start_Key,
	Best_Key,
	Start_Button,
	MENU_New,
	MENU_Open,
	MENU_Close,
	MENU_Save,
	MENU_SaveAs,
	MENU_Quit,
	lblCipher_Text,
	lblPlain_Text,
	lblStart_Key,
	lblBest_Key,
	lblScore_,
	Score_
};
*/


//////////////////////////////////////////////////////////////////////////////////////////////////
//                        Print ERROR MESSAGE when file can not be opened                       //
//           **** This should be replaced with some sort of GUI Error Dialog Box ****           //
//     --DEAD CODE: It is now in GUI                                                            //
//////////////////////////////////////////////////////////////////////////////////////////////////

void printferror(char *name_of_file) {

	printf("ERROR - File '%s' does not exist, or could not be opened!!\n\n",name_of_file);

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//  Read the N-Graph data into global arrays "bi...", "tri...", "tetra..." and "pentagraphs[]"  //
//      -DEAD CODE: Wesley has put the Read-Ngrams into the GUI                                 //
//////////////////////////////////////////////////////////////////////////////////////////////////
/*
int read_ngraphs(char *dir="", char *lang="eng") {

	FILE *ifptr;
	int t1,t2,t3,t4,t5,gtemp;
	char temp_string[500], filename[1024];
	int i;

	// INITIALIZE (ZERO) N-GRAPH ARRAYS
	for(i=0;i<(26*26);i++) bigraphs[i]=0;
	for(i=0;i<(26*26*26);i++) trigraphs[i]=0;
	for(i=0;i<(26*26*26*26);i++) tetragraphs[i]=0;
	for(i=0;i<(26*26*26*26*26);i++) pentagraphs[i]=0;

	// READ "BIGRAPHS.TXT"
	sprintf(filename,"%s/%s/bigraphs.txt",dir,lang);
	ifptr=fopen(filename,"r"); if(ifptr==NULL) return 0;
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
	sprintf(filename,"%s/%s/trigraphs.txt",dir,lang);
	ifptr=fopen(filename,"r"); if(ifptr==NULL) return 0;
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
	sprintf(filename,"%s/%s/tetragraphs.txt",dir,lang);
	ifptr=fopen(filename,"r"); if(ifptr==NULL) return 0;
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
	sprintf(filename,"%s/%s/pentagraphs.txt",dir,lang);
	ifptr=fopen(filename,"r"); if(ifptr==NULL) return 0;
	while(!feof(ifptr)) {
		t1=fgetc(ifptr)-'A'; t2=fgetc(ifptr)-'A'; t3=fgetc(ifptr)-'A'; t4=fgetc(ifptr)-'A'; t5=fgetc(ifptr)-'A';
		fgetc(ifptr); fgetc(ifptr); fgetc(ifptr);
		fscanf(ifptr,"%i",&gtemp); fgets(temp_string,500,ifptr);
		if(t1+'A'=='*') break;
		if(_DEB) printf("\nPentaGraph: %c%c%c%c%c  Count: %4i  Index: %i",t1+'A',t2+'A',t3+'A',t4+'A',t5+'A',gtemp,(t1*456976+t2*17576+t3*676+t4*26+t5));
		pentagraphs[t1*456976+t2*17576+t3*676+t4*26+t5]=(int)(10*log((double)gtemp)); }
	if(_DEB) printf("\n");
	fclose(ifptr);
	
	return 1;
}
*/

