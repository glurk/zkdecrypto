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
	siSolveInfo.best_key[0]='\0';

	//setup window
	bUndo=false;
	MsgEnable(true);
	MapEnable(false);
	EnableMenuItem(hMainMenu,IDM_FILE_SAVE_CIPHER,MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMainMenu,IDM_FILE_COPY_PLAIN,MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMainMenu,IDM_EDIT_UNDO,MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMainMenu,IDM_EDIT_REDO,MF_BYCOMMAND | MF_GRAYED);
	Button_Enable(GetDlgItem(hMainWnd,IDC_SOLVE),true);
	
	iCurSymbol=-1;
	iTextSel=-1;
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,iCurSymbol,0);
	ClearTextAreas();
	SetScrollBar();
	SetTitle();
	SetCipher();
	SetPatterns();
	SetDlgInfo();
	
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

//set given text to clipboard
void SetClipboardText(const char *szClipText)
{
	HGLOBAL hgClipboard;
	char *szClipboard;
	
	//allocate clipboard data
	hgClipboard=GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE,strlen(szClipText)+1);
	szClipboard=(char*)GlobalLock(hgClipboard);
	strcpy(szClipboard,szClipText);
	GlobalUnlock(hgClipboard);

	//set clipboard
	OpenClipboard(hMainWnd);
	EmptyClipboard();
	SetClipboardData(CF_TEXT,(void*)hgClipboard);
	CloseClipboard();
}

//save plaintext to file/clipboard
int SavePlain(char *filename)
{
	FILE *pfPlain;
	char *szPlainText;
	int msg_len=message.GetLength();
	
	szPlainText=new char[msg_len+((iLines+1)*3)+1];
	BreakText(szPlainText,message.GetPlain());
	
	//save as file
	if(filename) 
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

//read configuration file
int LoadINI()
{
	FILE *ini_file;
	char filename[1024], option[32], value[1024];
	char *comment;
	int read;

	sprintf(filename,"%s\\zodiac.ini",szExeDir);

	ini_file=fopen(filename,"r");

	if(!ini_file) return 0;

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
		else if(!stricmp(option,"revert")) siSolveInfo.revert=atoi(value);
		else if(!stricmp(option,"use")) iUseGraphs=atoi(value);
		else if(!stricmp(option,"line")) iLineChars=atoi(value);
		else if(!stricmp(option,"lang")) iLang=atoi(value);
		else if(!stricmp(option,"minword")) iWordMin=atoi(value);
		else if(!stricmp(option,"maxword")) iWordMax=atoi(value);
		else if(!stricmp(option,"extra")) strcpy(szExtraLtr,value);
	}

	fclose(ini_file);

	return 1;
}

//save configuration file
int SaveINI()
{
	FILE *ini_file;
	char filename[1024];

	sprintf(filename,"%s\\zodiac.ini",szExeDir);

	ini_file=fopen(filename,"w");

	if(!ini_file) return 0;

	fprintf(ini_file,"# Generated by %s %s\n\n",PROG_NAME,PROG_VER);

	fprintf(ini_file,"cipher = %s\n",szCipherName);
	fprintf(ini_file,"key = %s\n",szKeyName);
	fprintf(ini_file,"plain = %s\n",szPlainName);
	fprintf(ini_file,"fail = %i\n",siSolveInfo.max_fail);
	fprintf(ini_file,"swap = %i\n",siSolveInfo.swaps);
	fprintf(ini_file,"revert = %i\n",siSolveInfo.revert);
	fprintf(ini_file,"use = %i\n",iUseGraphs);
	fprintf(ini_file,"line = %i\n",iLineChars);
	fprintf(ini_file,"lang = %i\n",iLang);
	fprintf(ini_file,"minword = %i\n",iWordMin);
	fprintf(ini_file,"maxword = %i\n",iWordMax);
	fprintf(ini_file,"extra = %s\n",szExtraLtr);

	fclose(ini_file);

	return 1;
}

//convert numeric cipher to ascii
int Num2Asc(char *in_name, char *out_name)
{
    FILE *in_file, *out_file;
    char *number;
    int ascii;
    
    in_file=fopen(in_name,"r");
    out_file=fopen(out_name,"w");
    
    fseek(in_file,0,SEEK_END);
    number=new char[ftell(in_file)+1];
    fseek(in_file,0,SEEK_SET);
    
    if(!in_file || !out_file) return 0;
    
    while(fscanf(in_file,"%s",number)!=EOF)
    {
		ascii=atoi(number);
		
		if(ascii>0 && ascii<128) 
			putc(char(ascii+0x20),out_file);
	}
	
	fclose(in_file);
	fclose(out_file);
	
	delete number;
    
    return 1;
}

//generate random cipher
void RandCipher(int length, int symbols)
{
	FILE *rand_cipher;

	sprintf(szText,"%s%s",szExeDir,"random.txt");
	if(!(rand_cipher=fopen(szText,"w"))) return;
	for(int x=0; x<340; x++) putc((rand()%63)+0x21,rand_cipher);
	fclose(rand_cipher);
	LoadMessage(szText);
}
