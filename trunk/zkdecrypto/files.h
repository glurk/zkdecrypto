/*File Functions*/

void GetBaseName(const char *filename, char *&basename) 
{
	if(filename[0]!='\0') basename=(char *)strrchr(filename,'\\')+1;
	else basename=NULL;
}

//open/save file name dialog
int GetFilename(char *szName, const char *szInitDir, int bSave)
{
	OPENFILENAME ofn;
	char szNameTemp[128]="\0";
	int result;

	memset(&ofn,0,sizeof(OPENFILENAME));
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=hMainWnd;
	ofn.lpstrInitialDir=szInitDir;
	ofn.lpstrFilter=szFileFilter;
	ofn.nMaxFile=0x7ff;
	ofn.Flags=OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;

	if(!bSave) //open
	{		
		ofn.lpstrFile=szNameTemp;
		ofn.lpstrTitle="Open File";
		result=GetOpenFileName(&ofn);
	}

	else //save
	{
		if(szKeyBase) strcpy(szNameTemp,szKeyBase);
		ofn.lpstrFile=szNameTemp;
		ofn.lpstrTitle="Save File";
		ofn.Flags|=OFN_OVERWRITEPROMPT;
		result=GetSaveFileName(&ofn); 
	}

	if(!result) return 0; //if error or cancel, bail

	strcpy(szName,szNameTemp);
	return ofn.nFilterIndex;
}

//shell execute file
void OpenWith(char *szFileName)
{
	SHELLEXECUTEINFO seiShellExec;

	memset(&seiShellExec,0,sizeof(SHELLEXECUTEINFO));
	seiShellExec.cbSize=sizeof(SHELLEXECUTEINFO);
	seiShellExec.lpFile=szFileName;
	seiShellExec.nShow=SW_SHOWNORMAL;
	ShellExecuteEx(&seiShellExec);
}

int LoadMessage(char *filename)
{
	if(!message.Read(filename)) 
	{
		sprintf(szText,"Cannot open %s",(const char*)filename);
		MessageBox(hMainWnd,szText,"Error",MB_OK | MB_ICONERROR);
		return 0;
	}

	//get message filename
	strcpy(szCipherName,filename);
	GetBaseName(szCipherName,szCipherBase);

	//get info
	szCipher=message.GetCipher();
	iCipherLength=message.GetLength();
	iSymbols=message.cur_map.GetNumSymbols();
	iBestScore=0;
	iCurPat=-1;
	bMsgLoaded=true;
	bMapLoaded=false;

	//setup window
	bUndo=false;
	MsgEnable(true);
	MapEnable(false);
	EnableMenuItem(hMainMenu,IDM_EDIT_UNDO,MF_BYCOMMAND | MF_GRAYED);
	Button_Enable(GetDlgItem(hMainWnd,IDC_SOLVE),true);
	
	iCurSymbol=-1;
	iTextSel=-1;
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,iCurSymbol,0);
	SetScrollBar();
	SetTitle();
	SetCipher();
	SetPatterns();
	SetDlgInfo();
	
	return 1;
}

int LoadMap(char *filename)
{
	if(!message.cur_map.Read(filename))
	{
		sprintf(szText,"Cannot open %s",filename);
		MessageBox(hMainWnd,szText,"Error",MB_OK | MB_ICONERROR);
		return 0;
	}

	//get map filename
	strcpy(szKeyName,filename);
	GetBaseName(szKeyName,szKeyBase);
	iSymbols=message.cur_map.GetNumSymbols();

	//get info
	iBestScore=0;
	bMapLoaded=true;

	//setup window
	bUndo=false;
	MapEnable(true);
	SetDlgInfo();

	return 1;
}
