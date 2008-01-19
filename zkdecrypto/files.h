/*File Functions*/

void GetBaseName(const char *filename, char *&basename) 
{
	if(filename[0]!='\0') basename=strrchr(filename,'\\')+1;
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

	//get info
	iBestScore=0;
	bMapLoaded=true;

	//setup window
	bUndo=false;
	MapEnable(true);
	SetDlgInfo();

	return 1;
}

int SavePlain(char *filename)
{
	FILE *pfPlain;

	pfPlain=fopen(filename,"w");

	if(!pfPlain) return 0;
	
	if(!fprintf(pfPlain,message.GetPlain())) return 0;

	fclose(pfPlain);

	return 1;
}

int LoadINI()
{
	FILE *ini_file;
	char filename[1024], option[32], value[1024];

	sprintf(filename,"%s\\zodiac.ini",szExeDir);

	ini_file=fopen(filename,"r");

	if(!ini_file) return 0;

	while(fscanf(ini_file,"%s = %s",option,value)!=EOF)
	{
		if(!stricmp(option,"cipher")) strcpy(szCipherName,value);
		else if(!stricmp(option,"key")) strcpy(szKeyName,value);
		else if(!stricmp(option,"fail")) siSolveInfo.max_fail=atoi(value);
		else if(!stricmp(option,"swap")) siSolveInfo.swaps=atoi(value);
		else if(!stricmp(option,"revert")) siSolveInfo.revert=atoi(value);
		else if(!stricmp(option,"use")) iUseGraphs=atoi(value);
		else if(!stricmp(option,"line")) iLineChars=atoi(value);
		else if(!stricmp(option,"lang")) iLang=atoi(value);
		else if(!stricmp(option,"extra")) strcpy(szExtraLtr,value);
	}

	fclose(ini_file);

	return 1;
}

int SaveINI()
{
	FILE *ini_file;
	char filename[1024];

	sprintf(filename,"%s\\zodiac.ini",szExeDir);

	ini_file=fopen(filename,"w");

	if(!ini_file) return 0;

	fprintf(ini_file,"cipher = %s\n",szCipherName);
	fprintf(ini_file,"key = %s\n",szKeyName);
	fprintf(ini_file,"fail = %i\n",siSolveInfo.max_fail);
	fprintf(ini_file,"swap = %i\n",siSolveInfo.swaps);
	fprintf(ini_file,"revert = %i\n",siSolveInfo.revert);
	fprintf(ini_file,"use = %i\n",iUseGraphs);
	fprintf(ini_file,"line = %i\n",iLineChars);
	fprintf(ini_file,"lang = %i\n",iLang);
	fprintf(ini_file,"extra = %s\n",szExtraLtr);

	fclose(ini_file);

	return 1;
}
