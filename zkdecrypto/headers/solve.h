#pragma warning( disable : 4267)	// STOP MSVS2005 WARNINGS

/*Edit Functions*/

void SetUndo()
{
	bUndo=true;
	EnableMenuItem(hMainMenu,IDM_EDIT_UNDO,MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMainMenu,IDM_EDIT_REDO,MF_BYCOMMAND | MF_GRAYED);
	undo_message+=message;
	undo_line_size=iLineChars;
}

void Undo()
{
	bUndo=false;
	EnableMenuItem(hMainMenu,IDM_EDIT_UNDO,MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMainMenu,IDM_EDIT_REDO,MF_BYCOMMAND | MF_ENABLED);
	redo_message=message;
	message=undo_message;
	redo_line_size=iLineChars;
	iLineChars=undo_line_size;
	iLines=message.GetLength()/iLineChars;
	ClearTextAreas();
	SetText();
	SetPatterns();
}

void Redo()
{
	bUndo=true;
	EnableMenuItem(hMainMenu,IDM_EDIT_UNDO,MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMainMenu,IDM_EDIT_REDO,MF_BYCOMMAND | MF_GRAYED);
	undo_message=message;
	message=redo_message;
	iLineChars=redo_line_size;
	iLines=message.GetLength()/iLineChars;
	ClearTextAreas();
	SetText();
	SetPatterns();
}

//change letter mapped to symbol
void ChangePlain()
{
	SYMBOL symbol;

	if(iCurSymbol<0) return;

	//SetUndo();
	
	//get new letter
	GetDlgItemText(hMainWnd,IDC_MAP_VALUE,szText,10);
	
	//get and update symbol
	message.cur_map.GetSymbol(iCurSymbol,&symbol);
	symbol.plain=szText[0];
	message.cur_map.AddSymbol(symbol,0);
	
	//update info		
	UpdateSymbol(iCurSymbol);
	SetPlain();
	SetText();
	SetTable();
	SetFreq();
	SetWordList();
}

/*Solver Functions*/

//enable/disable menu items & buttons associated with a loaded cipher
void MsgEnable(int enabled)
{
	int menu_state;

	if(enabled) menu_state=MF_ENABLED;
	else menu_state=MF_GRAYED;

	if(enabled==false || bMsgLoaded) 
	{
		EnableMenuItem(hMainMenu,IDM_FILE_OPEN_ASC,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_FILE_OPEN_NUM,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_FILE_OPEN_MAP,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_FILE_SAVE_MAP,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_FILE_SAVE_PLAIN,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_SOLVE_INSERT,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_CIPHER_MERGE,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_CIPHER_SIMPLIFY,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_CIPHER_POLYIC,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_CIPHER_RC_IOC,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_CIPHER_SEQHOMO,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_CIPHER_HORZ,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_CIPHER_VERT,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_CIPHER_REV,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_CIPHER_ROT_LEFT,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_CIPHER_ROT_RIGHT,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_KEY_INIT,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_KEY_CT,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_KEY_SCRAMBLE,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_KEY_CLEAR,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_SOLVE_WORD,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_SOLVE_BRUTE,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_SOLVE_RESET,MF_BYCOMMAND | menu_state);
		Button_Enable(GetDlgItem(hMainWnd,IDC_CHANGE),enabled);
		Button_Enable(GetDlgItem(hMainWnd,IDC_RESET),enabled);
	}
	
	if(bMsgLoaded) menu_state=MF_ENABLED;
	else menu_state=MF_GRAYED;
	
	EnableMenuItem(hMainMenu,IDM_EDIT_MSG,MF_BYCOMMAND | menu_state);
	EnableMenuItem(hMainMenu,IDM_VIEW_SYMGRAPH,MF_BYCOMMAND | menu_state);
	EnableMenuItem(hMainMenu,IDM_VIEW_LTRGRAPH,MF_BYCOMMAND | menu_state);
	EnableMenuItem(hMainMenu,IDM_VIEW_MERGE_LOG,MF_BYCOMMAND | menu_state);
	EnableMenuItem(hMainMenu,IDM_VIEW_EXCLUSIONS,MF_BYCOMMAND | menu_state);
}

//enable/disable menu items & buttons associated with a loaded key
void MapEnable(int enabled)
{
	int menu_state;

	if(enabled) menu_state=MF_ENABLED;
	else menu_state=MF_GRAYED;
			
	if(enabled==false || bMapLoaded)
	{
		EnableMenuItem(hMainMenu,IDM_FILE_RELOAD,MF_BYCOMMAND | menu_state);
	}

	if(enabled && bUndo) menu_state=MF_ENABLED;
	else menu_state=MF_GRAYED;

	EnableMenuItem(hMainMenu,IDM_EDIT_UNDO,MF_BYCOMMAND | menu_state);
	EnableMenuItem(hMainMenu,IDM_EDIT_REDO,MF_BYCOMMAND | menu_state);
	
	if(bMapLoaded) menu_state=MF_ENABLED;
	else menu_state=MF_GRAYED;

	EnableMenuItem(hMainMenu,IDM_EDIT_MAP,MF_BYCOMMAND | menu_state);
}

//reset window state when solve stops
void StopSolve()
{
	siSolveInfo.running=false;
	TerminateThread(hTimerThread,0);
	hTimerThread=NULL;
	SetDlgItemText(hMainWnd,IDC_SOLVE,"Start");
	MsgEnable(true);
	MapEnable(true);

	switch(iSolveType)
	{
		case SOLVE_HOMO: break;
		case SOLVE_VIG: message.SetKey(siSolveInfo.best_key); break;
		case SOLVE_BIFID: strcpy(message.bifid_array,siSolveInfo.best_key); break;
		case SOLVE_TRIFID: strcpy(message.trifid_array,siSolveInfo.best_key); break;
		case SOLVE_ANAGRAM:
		case SOLVE_COLTRANS: if(siSolveInfo.best_trans) 	message.SetCipherTrans(siSolveInfo.best_trans);	break;
	}

	delete siSolveInfo.best_trans;
	siSolveInfo.best_trans=NULL;

	SetDlgInfo();
}

//timer thread proc
DWORD WINAPI Timer(LPVOID lpVoid)
{
	char szTime[8];
	int hr=0, min=0, sec=0;
	
	while(siSolveInfo.running)
	{
		sprintf(szTime,"%02i:%02i:%02i",hr,min,sec);
		SetDlgItemText(hMainWnd,IDC_TIME,szTime);
		
		//increment time
		if(++sec==60) {min++; sec=0;}
		if(min==60) {hr++; min=0;}
		
		//wait 1 second
		Sleep(1000);
	}

	hTimerThread=NULL;
	ExitThread(0);
	return 0;
}

char FirstAvailable(char *exclude)
{
	for(int letter=0; letter<26; letter++)
		if(!strchr(exclude,letter+'A'))
			return letter+'A';
	
	return 'Z'+1;
}

void BatchBest()
{
	char *szPlainText;
	
	Message best_msg;
	
	szPlainText=new char[message.GetLength()+((iLines+1)*3)+1];
	BreakText(szPlainText,message.GetPlain());
	
	best_msg=message;
	best_msg.cur_map.FromKey(lprgcBatchBestKey);
	BreakText(szPlainText,best_msg.GetPlain());
	SetClipboardText(szPlainText);
	
	delete[] szPlainText;
}

//begin brute force
void BruteStart()
{
	SYMBOL symbol;
	
	if(!iBruteSymbols) return;
	
	iBatchBestScore=0;
	message.cur_map.ToKey(lprgcBatchBestKey,"");		
	
	for(int iCurSymbol=0; iCurSymbol<iBruteSymbols; iCurSymbol++)
	{
		message.cur_map.SetLock(iCurSymbol,false);
		message.cur_map.GetSymbol(iCurSymbol,&symbol);
		
		symbol.plain=FirstAvailable(symbol.exclude);
		if(symbol.plain>'Z') continue;		
		
		message.cur_map.AddSymbol(symbol,false);
		message.cur_map.SetLock(iCurSymbol,true);
	}
	
	SendMessage(hMainWnd,WM_COMMAND,IDC_SOLVE,0);
}

//increment symbols in brute force recursivly
void BruteNext(int iSymbol)
{
	SYMBOL symbol;
	
	if(iSymbol<0) //done
	{
		iBruteSymbols=-1;
		return;
	}
		
	//increment symbol
	message.cur_map.GetSymbol(iSymbol,&symbol);
	symbol.plain++;
	
	//go past excludes
	while(strchr(symbol.exclude,symbol.plain) && symbol.plain<='Z')
		symbol.plain++;
			
	message.cur_map.SetLock(iSymbol,false); //unlock symbol
	
	if(symbol.plain>'Z') //turn back to 'A' and carry to previous symbol
	{
		symbol.plain='A';
		message.cur_map.AddSymbol(symbol,false);
		BruteNext(iSymbol-1);
	}
	
	else message.cur_map.AddSymbol(symbol,false);
	
	message.cur_map.SetLock(iSymbol,true); //relock symbol
	
	if(iBruteSymbols<1) return;
	
	//start solving if this is the last brute symbol
	if(iSymbol==iBruteSymbols-1) 
	{
		SendMessage(hMainWnd,WM_COMMAND,IDM_KEY_SCRAMBLE,0);
		SendMessage(hMainWnd,WM_COMMAND,IDC_SOLVE,0);
	}
}

//on press stop, or failure of hillclimber
void StopNotify()
{
	//save best key
	if(siSolveInfo.best_score>iBatchBestScore)
	{
		iBatchBestScore=siSolveInfo.best_score;
		strcpy(lprgcBatchBestKey,siSolveInfo.best_key);
	}
	
	if(iBruteSymbols>0) BruteNext(iBruteSymbols-1); //in brute
	if(iBruteSymbols<0) //brute finished, display best
	{
		MessageBox(hMainWnd,"Brute Force Done","Alert",MB_OK);
		message.cur_map.FromKey(lprgcBatchBestKey);
		SetDlgInfo();
		SetDlgItemInt(hMainWnd,IDC_SCORE,iBatchBestScore,false);
		iBruteSymbols=0;
	}
}

//solve thread proc
DWORD WINAPI FindSolution(LPVOID lpVoid) 
{
	int num_symbols, use_key_len;
	char key[KEY_SIZE];
	char *exclude=NULL;
	SYMBOL symbol;
	
	if(!bMsgLoaded) return 0;
	
	SetThreadPriority(hSolveThread,iPriority);

	if(iSolveType==SOLVE_HOMO)
	{
		num_symbols=message.cur_map.GetNumSymbols();
		
		//if best key is blank, set it to empty symbols + extra letters
		if(!strlen(siSolveInfo.best_key)) 
			message.cur_map.ToKey(siSolveInfo.best_key,szExtraLtr);
		
		//key=program map + additional chars of best key
		message.cur_map.ToKey(key,siSolveInfo.best_key+num_symbols);
		
		//setup exclude list
		exclude=new char[27*num_symbols];

		for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
		{
			message.cur_map.GetSymbol(cur_symbol,&symbol);
			strcpy(exclude+(27*cur_symbol),symbol.exclude);
		}
		
		siSolveInfo.locked=(char*)message.cur_map.GetLocked();
		siSolveInfo.exclude=exclude;
		
		hillclimb(message,szCipher,message.GetLength(),key,siSolveInfo,false);
	}

	else 
	{
		if(iSolveType==SOLVE_KRYPTOS) hillclimb2(message,siSolveInfo,iSolveType,key,iLineChars);

		else
		{
			switch(iSolveType)
			{
				case SOLVE_VIG: strcpy(key,message.GetKey()); strcat(key,szExtraLtr);break;
				case SOLVE_BIFID: strcpy(key,message.bifid_array); break;
				case SOLVE_TRIFID:	strcpy(key,message.trifid_array); break;
				case SOLVE_ANAGRAM:	break;
				case SOLVE_COLTRANS: break;
			}

			hillclimb2(message,siSolveInfo,iSolveType,key,iLineChars);
		}
	}

	//reset window state
	StopSolve();
	SetDlgInfo();
	
	hSolveThread=NULL;
	if(exclude) delete[] exclude;
	
	StopNotify();
	
	ExitThread(0);
	return 0;
}

//reset window state when solve starts
void StartSolve()
{
	if(hSolveThread) return;

	siSolveInfo.running=true;
	SetDlgItemText(hMainWnd,IDC_SOLVE,"Stop");
	MsgEnable(false);
	MapEnable(false);
	hSolveThread=CreateThread(0,1024,FindSolution,0,0,0);
	hTimerThread=CreateThread(0,128,Timer,0,0,0);
}

void Reset() //init solve info
{
	siSolveInfo.cur_try=0;
	siSolveInfo.cur_fail=0;
	siSolveInfo.last_time=0;
	SetDlgItemText(hMainWnd,IDC_TIME,"00:00:00");
	iBestScore=0;
	SetDlgInfo();
}

//set thread priority
void SetPriority(int iNewPriority)
{
	if(iNewPriority==4)
	{
		iPriority=THREAD_PRIORITY_IDLE;
		CheckMenuItem(hMainMenu,IDM_SOLVE_TP_IDLE,MF_BYCOMMAND | MF_CHECKED);
	}

	else CheckMenuItem(hMainMenu,IDM_SOLVE_TP_IDLE,MF_BYCOMMAND | MF_UNCHECKED);
	
	if(iNewPriority==3)
	{
		iPriority=THREAD_PRIORITY_ABOVE_NORMAL;//TTHREAD_PRIORITY_HIGHEST;
		CheckMenuItem(hMainMenu,IDM_SOLVE_TP_HIGH,MF_BYCOMMAND | MF_CHECKED);
	}

	else CheckMenuItem(hMainMenu,IDM_SOLVE_TP_HIGH,MF_BYCOMMAND | MF_UNCHECKED);

	if(iNewPriority==2)
	{
		iPriority=THREAD_PRIORITY_NORMAL;
		CheckMenuItem(hMainMenu,IDM_SOLVE_TP_NORM,MF_BYCOMMAND | MF_CHECKED);
	}

	else CheckMenuItem(hMainMenu,IDM_SOLVE_TP_NORM,MF_BYCOMMAND | MF_UNCHECKED);

	if(iNewPriority==1)
	{
		iPriority=THREAD_PRIORITY_BELOW_NORMAL;//THREAD_PRIORITY_IDLE;
		CheckMenuItem(hMainMenu,IDM_SOLVE_TP_LOW,MF_BYCOMMAND | MF_CHECKED);
	}

	else CheckMenuItem(hMainMenu,IDM_SOLVE_TP_LOW,MF_BYCOMMAND | MF_UNCHECKED);

	if(hSolveThread) 
		if(!SetThreadPriority(hSolveThread,iPriority))
			MessageBox(hMainWnd,"Could not set Priority","Error",MB_ICONEXCLAMATION);
}

int LoadDictionary(char *filename, int show_error)
{
	FILE *dictionary_file;
	char word[64];
	std::string word_str;

	dictionary_file=fopen(filename,"r");

	if(!dictionary_file)
	{	
		if(show_error==false) return 0;
			else { sprintf(szText,"Cannot open \"%s\"",(const char*)filename);
				 MessageBox(hMainWnd,szText,"Error",MB_OK | MB_ICONERROR);
				 return 0; }
	}
	int x, i = 1;
	while(!feof(dictionary_file)) 
	{
		fscanf(dictionary_file,"%s",word);
		for(x=0; x<(int)strlen(word); x++) word[x]=toupper(word[x]);
		word_str=word;
		dictionary[word_str] = i;
		i++;
	}

	fclose(dictionary_file);
	return 1;
}

//set language and load data files
void SetLanguage()
{
	char szLang[8], szGraphBase[32];
	double unigraphs[26];

	switch(iLang)
	{
		case 0: strcpy(szLanguage,"English"); strcpy(szLang,LANG_ENG); siSolveInfo.lang_ioc=(float)IOC_ENG; break;
		case 1: strcpy(szLanguage,"Spanish"); strcpy(szLang,LANG_SPA); siSolveInfo.lang_ioc=(float)IOC_SPA; break;
		case 2: strcpy(szLanguage,"German"); strcpy(szLang,LANG_GER); siSolveInfo.lang_ioc=(float)IOC_GER; break;
		case 3: strcpy(szLanguage,"Italian"); strcpy(szLang,LANG_ITA); siSolveInfo.lang_ioc=(float)IOC_ITA; break;
		case 4: strcpy(szLanguage,"French"); strcpy(szLang,LANG_FRE); siSolveInfo.lang_ioc=(float)IOC_FRE; break;
	}
	
	for(int n=1; n<=5; n++)
	{
		if(n==1)  strcpy(szGraphBase,"unigraphs.txt");
		if(n==2)  strcpy(szGraphBase,"bigraphs.txt");
		if(n==3)  strcpy(szGraphBase,"trigraphs.txt");
		if(n==4)  strcpy(szGraphBase,"tetragraphs.txt");
		if(n==5)  strcpy(szGraphBase,"pentagraphs.txt");
		
		sprintf(szGraphName,"%s%s\\%s\\%s",szExeDir,LANG_DIR,szLang,szGraphBase);

		if(!ReadNGraphs(szGraphName,n))
		{
			sprintf(szText,"Could not open %s",szGraphName);
			MessageBox(hMainWnd,szText,"Error",MB_ICONEXCLAMATION);
			//SendMessage(hMainWnd,WM_CLOSE,0,0);
		}
	}

	dictionary.clear();
	sprintf(szGraphName,"%s%s\\%s\\%s",szExeDir,LANG_DIR,szLang,"dictionary.txt");
	LoadDictionary(szGraphName,true);
	sprintf(szGraphName,"%s%s\\%s\\%s",szExeDir,LANG_DIR,szLang,"userdict.txt");
	LoadDictionary(szGraphName,false);
	
	GetUnigraphs(unigraphs);
	message.cur_map.SetUnigraphs(unigraphs);
	message.SetExpFreq();
}

int ToggleLock()
{
	iCurSymbol=SendDlgItemMessage(hMainWnd,IDC_MAP,LB_GETCURSEL,0,0);
	if(iCurSymbol<0) return -1;
	
	message.cur_map.ToggleLock(iCurSymbol);
	UpdateSymbol(iCurSymbol);
	SetText();

	return iCurSymbol;
}

void LockWord(int lock)
{
	int word_len, position, symbol;
	const char *word_ptr;

	if(iCurWord<0) return;

	SendDlgItemMessage(hMainWnd,IDC_WORD_LIST,LB_GETTEXT,iCurWord,(LPARAM)szText);
	
	word_len=(int)strlen(szText);
	word_ptr=szPlain;
					
	while(word_ptr=strstr(word_ptr,szText))
	{
		position=word_ptr-szPlain;

		for(int letter=0; letter<word_len; letter++)
		{
			symbol=message.cur_map.FindByCipher(szCipher[position+letter]);
			message.cur_map.SetLock(symbol,lock);
		}

		word_ptr+=word_len;
	}
}

/*

void DecodeVigenere(char *cipher, char *key, int key_len)
{
	int cipher_len=strlen(cipher);
	//int key_len=strlen(key);
	int iCipherIndex, iKeyIndex=0, iCipherCol, iKeyRow;
	char *lpcCipherInKeyRow;

	for(iCipherIndex=0; iCipherIndex<cipher_len; iCipherIndex++)
	{
		//find key row
		for(iKeyRow=0; iKeyRow<26; iKeyRow++)
			if(vigenere_array[iKeyRow][0]==key[iKeyIndex]) break;

		if(iKeyRow>25) continue;

		//find cipher column
		lpcCipherInKeyRow=strchr(vigenere_array[iKeyRow],cipher[iCipherIndex]);
		if(!lpcCipherInKeyRow) continue;

		iCipherCol=int(lpcCipherInKeyRow-vigenere_array[iKeyRow]);

		//get plain text character in key row 0
		cipher[iCipherIndex]=vigenere_array[0][iCipherCol];
	
		if(++iKeyIndex>=key_len) iKeyIndex=0;
	}
}
*/
/*void ColumnPermute(char *cipher,int length,int pos,int r)
{
   if(pos==r+1)
   {
       return; 
   }
   
   for(int index=pos-1;index<=length-1;index++)
   {
       message.SwapColumns(pos,r);
	   if(message.GetNumPatterns()>30) 
	   {
		   return;
	   }
       permute(cipher,length,pos+1,r);
       message.SwapColumns(pos,r);
   }
}*/

/*
void KryptosMatrix()
{
	int cipher_len=message.GetLength();
	char *new_cipher=new char[cipher_len+1];
	const char *cipher=message.GetCipher();
	int iAdds=-1, iAddSub=0; 
	int iNewIndex=-1, iSkipAdd=false;

	int iAdd=192, iSub=144, iMaxAdds=3;
	iAdd=48; iSub=48;
	

	memset(new_cipher,1,cipher_len);

	for(int iCipherIndex=0; iCipherIndex<cipher_len; iCipherIndex++)
	{

			iNewIndex+=iAdd;
			if(iNewIndex>=cipher_len) iNewIndex-=cipher_len;
				
		

		
		new_cipher[iNewIndex--]=cipher[iCipherIndex];
	}

	new_cipher[cipher_len]='\0';

	message.SetCipherTrans(new_cipher);
	delete new_cipher;
}
*/
/*
void KryptosMatrix(int *key, int enc_dec)
{
	int cipher_len=message.GetLength();
	char *new_cipher=new char[cipher_len+1];
	const char *cipher=message.GetCipher();
	int iKeyIndex=0, key_len=7;
	int iNewIndex=-1;
	int line_len=cipher_len/7;
	int line_diff;
	
	for(int iCipherIndex=0; iCipherIndex<cipher_len; iCipherIndex++)
	{
		if(iKeyIndex) line_diff=lines[iKeyIndex]-lines[iKeyIndex-1];
		else line_diff=lines[iKeyIndex];
		
		iNewIndex+=line_len*line_diff;
		if(iKeyIndex%2) iNewIndex--;

		if(iNewIndex<0) {iNewIndex+=cipher_len; iNewIndex++;}
		if(iNewIndex>=cipher_len) {iNewIndex-=cipher_len; iNewIndex--;}
			
		if(enc_dec) new_cipher[iCipherIndex]=cipher[iNewIndex];
		else new_cipher[iNewIndex]=cipher[iCipherIndex];

		if(++iKeyIndex==key_len) iKeyIndex=0;
	}

	new_cipher[cipher_len]='\0';

	message.SetCipherTrans(new_cipher);
	delete new_cipher;
}*/

/*
void KryptosMatrix(int *key, int key_len, int enc_dec)
{
	int cipher_len=message.GetLength();
	char *new_cipher=new char[cipher_len+1];
	const char *cipher=message.GetCipher();
	int iKeyIndex=0;
	int iNewIndex=-1;
	int line_len=cipher_len/7;
	int line_diff;
	
	for(int iCipherIndex=0; iCipherIndex<cipher_len; iCipherIndex++)
	{
		if(iKeyIndex) line_diff=key[iKeyIndex]-key[iKeyIndex-1];
		else line_diff=key[iKeyIndex];
		
		iNewIndex+=line_len*line_diff;
		if(iKeyIndex%2) iNewIndex--;

		if(iNewIndex<0) {iNewIndex+=cipher_len; iNewIndex++;}
		if(iNewIndex>=cipher_len) {iNewIndex-=cipher_len; iNewIndex--;}
			
		if(enc_dec) new_cipher[iCipherIndex]=cipher[iNewIndex];
		else new_cipher[iNewIndex]=cipher[iCipherIndex];

		if(++iKeyIndex==key_len) iKeyIndex=0;
	}

	new_cipher[cipher_len]='\0';

	message.SetCipherTrans(new_cipher);
	delete new_cipher;
}
*/

void KryptosMatrix(int *key, int enc_dec)
{
	int cipher_len=message.GetLength();
	char *new_cipher=new char[cipher_len+1];
	const char *cipher=message.GetCipher();
	int iKeyIndex=0, iNewIndex;
	int iLines;
	int cur_row, cur_col=-1;

	memset(new_cipher,1,cipher_len);
	
	if(cipher_len==336) {iLineChars=42;}
	if(cipher_len==98) {iLineChars=7;}

	iLines=cipher_len/iLineChars;

	cur_col=key[iKeyIndex] & 0x000000FF;
	cur_row=key[iKeyIndex]>>8 & 0x000000FF;
	if(++iKeyIndex==14) iKeyIndex=0;
	
	for(int iCipherIndex=0; iCipherIndex<cipher_len; iCipherIndex++)
	{
		//set cipher//plain character
		iNewIndex=iLineChars*cur_row+cur_col;

		if(enc_dec) {if(new_cipher[iNewIndex]!=1) continue; new_cipher[iNewIndex]=cipher[iCipherIndex];}
		else new_cipher[iCipherIndex]=cipher[iNewIndex];

		if(iLines>8) //K4 7 cols
		{
			cur_col--;
			cur_row-=8;
			if(cur_row<0) cur_row+=iLines;
		}

		else
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

	new_cipher[cipher_len]='\0';

	message.SetCipherTrans(new_cipher);
	delete new_cipher;
}
