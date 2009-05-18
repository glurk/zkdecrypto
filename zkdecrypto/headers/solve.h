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
	DIGRAPH digraph;

	if(iCurSymbol<0) return;
	
	GetDlgItemText(hMainWnd,IDC_MAP_VALUE,szText,10); //get new letter

	//get and update symbol
	if(DIGRAPH_MODE) 
	{
		message.digraph_map.GetDigraph(iCurSymbol,&digraph);
		digraph.plain1=szText[0];
		digraph.plain2=szText[1];
		message.digraph_map.AddDigraph(digraph,0);
		UpdateDigraph(iCurSymbol);
	}

	else 
	{
		message.cur_map.GetSymbol(iCurSymbol,&symbol);
		symbol.plain=szText[0];
		message.cur_map.AddSymbol(symbol,0);
		UpdateSymbol(iCurSymbol);
	}

	//update info
	SetPlain(); SetText();
	SetTable(); SetFreq();
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
		EnableMenuItem(hMainMenu,IDM_CIPHER_UPPER,MF_BYCOMMAND | menu_state);
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
		case SOLVE_HOMO: message.cur_map.FromKey(siSolveInfo.best_key); break;
		case SOLVE_VIG: message.SetKey(siSolveInfo.best_key); break;
		case SOLVE_BIFID: case SOLVE_PLAYFAIR: strcpy(message.bifid_array,siSolveInfo.best_key); break;
		case SOLVE_TRIFID: strcpy(message.trifid_array,siSolveInfo.best_key); break;
		case SOLVE_ANAGRAM:
		case SOLVE_COLTRANS: if(siSolveInfo.best_trans) 	message.SetCipherTrans(siSolveInfo.best_trans);	break;
		case SOLVE_RUNKEY: message.SetKey(siSolveInfo.best_key); break;
		case SOLVE_DISUB: message.digraph_map.FromKey(siSolveInfo.best_key); break;
	}

	if(siSolveInfo.best_trans)
	{
		delete siSolveInfo.best_trans;
		siSolveInfo.best_trans=NULL;
	}

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
		
		Sleep(1000); //wait 1 second
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
/*
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
*/
//on press stop, or failure of hillclimber
void StopNotify()
{
	//save best key
	if(siSolveInfo.best_score>iBatchBestScore)
	{
		iBatchBestScore=siSolveInfo.best_score;
		strcpy(lprgcBatchBestKey,siSolveInfo.best_key);
	}
	
	/*if(iBruteSymbols>0) BruteNext(iBruteSymbols-1); //in brute
	if(iBruteSymbols<0) //brute finished, display best
	{
		MessageBox(hMainWnd,"Brute Force Done","Alert",MB_OK);
		message.cur_map.FromKey(lprgcBatchBestKey);
		SetDlgInfo();
		SetDlgItemInt(hMainWnd,IDC_SCORE,iBatchBestScore,false);
		iBruteSymbols=0;
	}*/
}

//only include letters, and convert to upper case
int FormatKey(char *key)
{
	int length=strlen(key), temp_index=0;
	char cur_char, *temp=new char[length+1];

	for(int index=0; index<length; index++)
	{
		cur_char=key[index];

		if(IS_LOWER_LTR(cur_char)) cur_char-=32;
		if(!IS_UPPER_LTR(cur_char)) continue;
		temp[temp_index++]=cur_char;
	}

	temp[temp_index]='\0';
	strcpy(key,temp);
	delete temp;

	return temp_index;
}

//solve thread proc
DWORD WINAPI FindSolution(LPVOID lpVoid) 
{
	int num_symbols;
	char key[4096];
	char *exclude=NULL, *key_text;
	SYMBOL symbol;
	FILE *key_file;
	int key_text_size;
	
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
		
		hillclimb(message,szCipher,message.GetLength(),key,false);
	}

	else if(iSolveType==SOLVE_RUNKEY) //read running key text file
	{
		sprintf(szText,"%s\%s",szExeDir,"keytext.txt");
		key_file=fopen(szText,"r");
		fseek(key_file,0,SEEK_END);
		key_text_size=ftell(key_file);
		fseek(key_file,0,SEEK_SET);
		key_text=new char[key_text_size+1];
		fread(key_text,1,key_text_size,key_file);
		fclose(key_file);
		key_text[key_text_size]='\0';
		key_text_size=FormatKey(key_text);

		running_key(message,key_text);
		
		delete key_text;
	}

	else 
	{
		message.InitArrays();

		switch(iSolveType)
		{
			case SOLVE_VIG: strcpy(key,message.GetKey()); strcat(key,szExtraLtr);break;
			case SOLVE_BIFID: case SOLVE_PLAYFAIR: strcpy(key,message.bifid_array); break;
			case SOLVE_TRIFID:	strcpy(key,message.trifid_array); break;
			case SOLVE_ANAGRAM:	break;
			case SOLVE_COLTRANS: break;
			case SOLVE_KRYPTOS: break;
			case SOLVE_DISUB: message.digraph_map.ToKey(key,szExtraLtr); break;
		}

		hillclimb2(message,iSolveType,key,iLineChars);
	}

	StopSolve(); //reset window state
	
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
	hSolveThread=CreateThread(0,2048,FindSolution,0,0,0);
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
	int i=1;
	while(!feof(dictionary_file)) 
	{
		fscanf(dictionary_file,"%s",word);
		for(int x=0; x<(int)strlen(word); x++) word[x]=toupper(word[x]);
		word_str=word;
		dictionary[word_str]=i;
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

	siSolveInfo.lang_dioc=DIOC;
	siSolveInfo.lang_chi=CHI;
	siSolveInfo.lang_ent=ENT;
	
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
	
	if(DIGRAPH_MODE)
	{
		message.digraph_map.ToggleLock(iCurSymbol);
		UpdateDigraph(iCurSymbol);
	}
	
	else
	{
		message.cur_map.ToggleLock(iCurSymbol);
		UpdateSymbol(iCurSymbol);
	}

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
