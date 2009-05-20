/*File Functions*/

void GetBaseName(const char *filename, char *&basename) 
{
	if(filename[0]!='\0') basename=(char*)strrchr(filename,'\\')+1;
	else basename=NULL;
}

//open/save file name dialog
int GetFilename(char *szName, const char *szInitDir, int bSave)
{
	OPENFILENAME ofn;
	char szNameTemp[1024]="\0";
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
		if(bSave==1 && szKeyBase) strcpy(szNameTemp,szKeyBase);
		
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

int LoadMessage(char *filename, int type)
{
	int loaderror=false;

	switch(type) 
	{
		case 0: if(!message.Read(filename)) loaderror=true; break; //read as ascii
		case 1: if(!message.ReadNumeric(filename)) loaderror=true; break; //read as numeric
	}

	if(loaderror) //error loading file
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
	siSolveInfo.best_key[0]='\0';

	//setup window
	bUndo=false;
	MsgEnable(true);
	MapEnable(false);
	EnableMenuItem(hMainMenu,IDM_FILE_SAVE_CIPHER,MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMainMenu,IDM_FILE_COPY_PLAIN,MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMainMenu,IDM_EDIT_UNDO,MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMainMenu,IDM_EDIT_REDO,MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMainMenu,IDM_SOLVE_COPY_BEST,MF_BYCOMMAND | MF_ENABLED);
	Button_Enable(GetDlgItem(hMainWnd,IDC_SOLVE),true);
	
	iCurSymbol=-1;
	iTextSel=-1;
	iLineChars=message.CalcBestWidth(message.GetLength());
	tabu_map.clear();
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,iCurSymbol,0);
	SendDlgItemMessage(hMainWnd,IDC_BLOCK_SPIN,UDM_SETRANGE,1,message.GetLength());
	ClearTextAreas();
	SetScrollBar();
	SetTitle();
	SetCipher();
	SetPatterns();
	SetDlgInfo();
	SetDlgItemInt(hMainWnd,IDC_BLOCK_EDIT,message.GetLength(),false);

	return 1;
}


int LoadMap(char *filename)
{
	Map temp_map;
	
	if(!temp_map.Read(filename))
	{
		sprintf(szText,"Cannot open %s",filename);
		MessageBox(hMainWnd,szText,"Error",MB_OK | MB_ICONERROR);
		return 0;
	}

	siSolveInfo.best_key[0]='\0';
	
	//update symbols from loaded map
	message.cur_map+=temp_map;

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

//save plaintext to file/clipboard
int SavePlain(char *filename)
{
	FILE *pfPlain;
	char *szPlainText;
	int msg_len=message.GetLength();
	
	szPlainText=new char[msg_len+((iLines+1)*3)+1];
	BreakText(szPlainText,message.GetPlain());
	
	if(filename) //save as file
	{
		pfPlain=fopen(filename,"w");

		if(!pfPlain) {delete[] szPlainText; return 0;}
		fprintf(pfPlain,szPlainText);
		fclose(pfPlain);
	}
	
	else SetClipboardText(szPlainText);
	
	delete[] szPlainText;

	return 1;
}

//load zodiac font zkdfont.ttf
int LoadFONT()
{
	sprintf(szText,"%s\\help\\images\\zkdfont.ttf",szExeDir);
	if(!AddFontResource(szText)) return 0;
	return 1;
}

//remove zodiac font zkdfont.ttf
int RemoveFONT()
{
	sprintf(szText,"%s\\help\\images\\zkdfont.ttf",szExeDir);
	if(!RemoveFontResource(szText)) return 0;
	return 1;
}

//read cribs text file
void LoadCribs()
{
	FILE *ini_file;
	char crib[128];
	int read;

	sprintf(szText,"%s\\cribs.txt",szExeDir);

	ini_file=fopen(szText,"r");

	siSolveInfo.num_cribs=0;

	if(!ini_file) 
	{
		ini_file=fopen(szText,"w");
		if(ini_file) fclose(ini_file);
		return;
	}

	while((read=fscanf(ini_file,"%s",crib))!=EOF)
	{
		strcpy(siSolveInfo.cribs[siSolveInfo.num_cribs],crib);
		strupr(siSolveInfo.cribs[siSolveInfo.num_cribs]);
		siSolveInfo.num_cribs++;
	}

	fclose(ini_file);

}

//read configuration file
int LoadINI()
{
	FILE *ini_file;
	char option[32], value[1024];
	char *comment;
	int read;

	sprintf(szText,"%s\\zodiac.ini",szExeDir);

	if(!(ini_file=fopen(szText,"r"))) return 0;

	while((read=fscanf(ini_file,"%s = %[^\n]\n",option,value))!=EOF)
	{
		if(read==1) value[0]='\0';

		//end option/value at comment symbol
		comment=strchr(option,'#'); if(comment) *comment='\0';
		comment=strchr(option,'#'); if(comment) *comment='\0';

		if(!stricmp(option,"cipher")) strcpy(szCipherName,value);
		else if(!stricmp(option,"key")) strcpy(szKeyName,value);
		else if(!stricmp(option,"plain")) strcpy(szPlainName,value);
		else if(!stricmp(option,"fail")) siSolveInfo.max_fail=atoi(value);
		else if(!stricmp(option,"swap")) siSolveInfo.swaps=atoi(value);
		else if(!stricmp(option,"revert")) siSolveInfo.max_try=atoi(value);
		else if(!stricmp(option,"tabu_syms")) siSolveInfo.tabu_syms=atoi(value);
		else if(!stricmp(option,"lang")) iLang=atoi(value);
		else if(!stricmp(option,"minword")) iWordMin=atoi(value);
		else if(!stricmp(option,"maxword")) iWordMax=atoi(value);
		else if(!stricmp(option,"extra")) {if(value[0]=='*') value[0]='\0'; strcpy(szExtraLtr,value);}
		else if(!stricmp(option,"solve")) iSolveType=atoi(value);
		else if(!stricmp(option,"key_len")) message.SetKeyLength(atoi(value));
		else if(!stricmp(option,"tableu_alpha")) {message.SetTableuAlphabet(value);}
	}

	fclose(ini_file);

	return 1;
}

//save configuration file
int SaveINI()
{
	FILE *ini_file;

	sprintf(szText,"%s\\zodiac.ini",szExeDir);

	ini_file=fopen(szText,"w");

	if(!ini_file) return 0;

	fprintf(ini_file,"# Generated by %s %s\n\n",PROG_NAME,PROG_VER);

	fprintf(ini_file,"cipher = %s\n",szCipherName);
	fprintf(ini_file,"key = %s\n",szKeyName);
	fprintf(ini_file,"plain = %s\n",szPlainName);
	fprintf(ini_file,"fail = %i\n",siSolveInfo.max_fail);
	fprintf(ini_file,"swap = %i\n",siSolveInfo.swaps);
	fprintf(ini_file,"revert = %i\n",siSolveInfo.max_try);
	fprintf(ini_file,"tabu_syms = %i\n",siSolveInfo.tabu_syms);
	fprintf(ini_file,"lang = %i\n",iLang);
	fprintf(ini_file,"minword = %i\n",iWordMin);
	fprintf(ini_file,"maxword = %i\n",iWordMax);
	if(szExtraLtr[0]) fprintf(ini_file,"extra = %s\n",szExtraLtr);
	else fprintf(ini_file,"extra = %s\n","*");
	fprintf(ini_file,"solve = %i\n",iSolveType);
	fprintf(ini_file,"key_len = %i\n",message.GetKeyLength());
	fprintf(ini_file,"tableu_alpha = %s\n",message.GetTableuAlphabet());

	fclose(ini_file);

	return 1;
}

