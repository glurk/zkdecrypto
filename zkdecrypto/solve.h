/*Solver Functions*/

//enable/disable menu items & buttons associated with a loaded cipher
void MsgEnable(int enabled)
{
	int menu_state;

	if(enabled) menu_state=MF_ENABLED;
	else menu_state=MF_GRAYED;

	if(enabled==false || bMsgLoaded) 
	{
		EnableMenuItem(hMainMenu,IDM_FILE_OPEN_MSG,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_FILE_OPEN_MAP,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_FILE_SAVE_MAP,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_FILE_SAVE_PLAIN,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_CIPHER_MERGE,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_CIPHER_POLYIC,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_CIPHER_RC_IOC,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_KEY_INIT,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_KEY_SCRAMBLE,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_KEY_CLEAR,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_KEY_LOCK,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_KEY_UNLOCK,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_SOLVE_WORD,MF_BYCOMMAND | menu_state);
		Button_Enable(GetDlgItem(hMainWnd,IDC_CHANGE),enabled);
		Button_Enable(GetDlgItem(hMainWnd,IDC_RESET),enabled);
	}
	
	if(bMsgLoaded) menu_state=MF_ENABLED;
	else menu_state=MF_GRAYED;
	
	EnableMenuItem(hMainMenu,IDM_EDIT_MSG,MF_BYCOMMAND | menu_state);
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
	
	if(bMapLoaded) menu_state=MF_ENABLED;
	else menu_state=MF_GRAYED;

	EnableMenuItem(hMainMenu,IDM_EDIT_MAP,MF_BYCOMMAND | menu_state);
}

//reset window state when solve stops
void StopSolve()
{
	siSolveInfo.running=false;
	SetDlgItemText(hMainWnd,IDC_SOLVE,"Start");
	MsgEnable(true);
	MapEnable(true);
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

	ExitThread(0);
	return 0;
}

//solve thread proc
DWORD WINAPI FindSolution(LPVOID lpVoid) 
{
	char key[256];
	char *locked;
	
	if(!bMsgLoaded) return 0;
	
	SetThreadPriority(hSolveThread,iPriority);
	
	//convert map to key to pass
	message.cur_map.ToKey(key);
	locked=new char[message.cur_map.GetNumSymbols()];
	message.cur_map.GetLocked(locked);
	
	hillclimb(szCipher,message.GetLength(),key,locked,siSolveInfo,iUseGraphs);

	delete[] locked;

	//reset window state
	StopSolve();
	SetDlgInfo();

	hSolveThread=NULL;
	ExitThread(0);
	return 0;
}

//reset window state when solve starts
void StartSolve()
{
	siSolveInfo.running=true;
	SetDlgItemText(hMainWnd,IDC_SOLVE,"Stop");
	MsgEnable(false);
	MapEnable(false);
	hSolveThread=CreateThread(0,1024,FindSolution,0,0,0);
	CreateThread(0,128,Timer,0,0,0);
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

//set language and load data files
void SetLanguage()
{
	char szLang[8], szGraphBase[32];
	double unigraphs[26];

	switch(iLang)
	{
		case 0: strcpy(szLang,LANG_ENG); break;
		case 1: strcpy(szLang,LANG_SPA); break;
		case 2: strcpy(szLang,LANG_GER); break;
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
	
	//sprintf(szText,"%s%s",szExeDir,LANG_DIR);
	//read_ngraphs(szText,"eng");
	
	GetUnigraphs(unigraphs);
	message.cur_map.SetUnigraphs(unigraphs);
}
