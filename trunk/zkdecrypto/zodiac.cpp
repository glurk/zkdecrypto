#include <stdio.h>
#include <time.h>
#include <math.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <richedit.h>
#include "resource.h"
#include "message.h"
#include "z340.h"

#define PROG_NAME	"Zodiac Code Decipher"
#define PROG_VER	"v1.0"
#define LANG_DIR	"language"
#define LANG_ENG	"eng"
#define LANG_SPA	"spa"
#define LANG_GER	"ger"

#define TEXT_HEIGHT 210

Message message;
Map mUndoMap;

char szCipherName[1024], szKeyName[1024], szGraphName[1024];
char *szCipherBase, *szKeyBase;
int bMsgLoaded=false, bMapLoaded=false, bUndo=false;
const char *szCipher=NULL, *szPlain=NULL;
int iCipherLength, iSymbols, iBestScore=0;
char szTitle[512], szText[1024], szExeDir[1024];
int iCurSymbol=-1, iCurPat=-1;
int iCharWidth, iCharHeight, iLines, iDispLines=17, iScrollPos=0, iMaxScroll;
char szLangDir[1024], szLang[8];

int flags=DT_CALCRECT | DT_EXTERNALLEADING | DT_WORDBREAK | DT_NOCLIP | DT_LEFT | DT_NOPREFIX;

SOLVEINFO siSolveInfo;
int iUseGraphs=USE_BI+USE_TRI+USE_TETRA+USE_PENTA;
int iLineChars=17, iPriority, iLang;

HWND		hMainWnd, hCipher=NULL, hPlain=NULL, hScroll=NULL;
HPEN 		hRedPen, hBluePen;
HDC 		hCipherDC=NULL, hPlainDC=NULL;
HMENU		hMainMenu;
HINSTANCE	hInst;
HANDLE		hSolveThread=NULL;

char szFileFilter[]=
	{"Text Files (*.txt)\0" "*.txt;\0"
	 "All Files (*.*)\0" "*.*\0\0"};

/*conversion functions*/
void MapToKey(Map &map, char *key)
{
	SYMBOL symbol;
	
	for(int cur_symbol=0; cur_symbol<map.GetNumSymbols(); cur_symbol++)
	{
		map.GetSymbol(cur_symbol,&symbol);
		key[cur_symbol]=symbol.plain;
	}	
	
	key[map.GetNumSymbols()]='\0';
}

void KeyToMap(Map &map, char *key)
{
	SYMBOL symbol;
	
	for(int cur_symbol=0; cur_symbol<map.GetNumSymbols(); cur_symbol++)
	{
		map.GetSymbol(cur_symbol,&symbol);
		symbol.plain=key[cur_symbol];
		map.AddSymbol(symbol,0);
	}	
}

/*Dialog Info Functions*/

int BreakText(const char *src, char *dest, int length)
{
	int index=0, line=0;
	
	if(!src || !dest) return 0;

	for(int cur_char=iScrollPos*iLineChars; cur_char<length && line<iDispLines; cur_char++)
	{
		dest[index++]=src[cur_char];
		if(!((cur_char+1)%iLineChars) && cur_char<length-1) 
		{
			dest[index++]='\r';
			dest[index++]='\n';
			line++;
		}
	}

	dest[index]='\0';
	return 1;
}

void SetTitle()
{
	sprintf(szTitle, "%s %s",PROG_NAME,PROG_VER);

	if(bMsgLoaded) 
	{
		sprintf(szText," - %s (%i characters)",szCipherBase,iCipherLength);
		strcat(szTitle,szText);
	}
	
	SetWindowText(hMainWnd,szTitle);
}

void SetScrollBar()
{
	iCharWidth=7;
	iCharHeight=12;
	
	iDispLines=TEXT_HEIGHT/iCharHeight;
	iLines=iCipherLength/iLineChars;
	if(iCipherLength%iLineChars) iLines++;	
	iMaxScroll=iLines-iDispLines;
	SetScrollRange(hScroll,SB_CTL,0,iMaxScroll,false);
	SetScrollPos(hScroll,SB_CTL,0,true);
}

void OutlineChars(HDC hDC, HPEN hPen, int iStart, int iEnd)
{
	RECT rOutRect;
	int iRow, iCol;
	
	SelectObject(hDC,hPen);
	
	for(int iChar=iStart; iChar<iEnd; iChar++)
	{
		iRow=(iChar/iLineChars)-iScrollPos;
		iCol=iChar%iLineChars;
		
		if(iRow<0) continue;
	
		/*rOutRect.left=iCol*iCharWidth;
		rOutRect.top=iRow*iCharHeight;
		rOutRect.right=rOutRect.left+iCharWidth;
		rOutRect.bottom=rOutRect.top+iCharHeight;*/
		
		rOutRect.left=iCol*iCharWidth+2;
		rOutRect.top=(iRow+1)*iCharHeight;
		rOutRect.right=rOutRect.left+iCharWidth;
		rOutRect.bottom=rOutRect.top+2;
		
		Rectangle(hDC,rOutRect.left,rOutRect.top,rOutRect.right,rOutRect.bottom);
	}
}

void SetText()
{
	NGRAM pattern;
	int iStart, iEnd;
	SYMBOL symbol;

	//set text	
	if(hCipher && BreakText(szCipher,szText,iCipherLength))
		SetWindowText(hCipher,szText);
		
	if(hPlain && BreakText(szPlain,szText,iCipherLength))
		SetWindowText(hPlain,szText);

	//pattern outlines
	if(hCipherDC && message.GetPattern(iCurPat,&pattern))
		for(int cur_pos=0; cur_pos<pattern.freq; cur_pos++)
		{
			iStart=pattern.positions[cur_pos];
			iEnd=iStart+pattern.length;
			OutlineChars(hCipherDC,hBluePen,iStart,iEnd);
		}
	
	//symbol outlines
	if(hCipherDC && hPlainDC && message.cur_map.GetSymbol(iCurSymbol,&symbol))
		for(int cur_symbol=0; cur_symbol<iCipherLength; cur_symbol++)
		{
			if(szCipher[cur_symbol]==symbol.cipher)
			{
				OutlineChars(hCipherDC,hRedPen,cur_symbol,cur_symbol+1);
				OutlineChars(hPlainDC,hRedPen,cur_symbol,cur_symbol+1);
			}
		}
}

void SetCipher()
{	
	sprintf(szText,"Cipher Text (Strength: %.2f)",message.GetStrength());
	SetDlgItemText(hMainWnd,IDC_CIPHER_TITLE,szText);
		
	SetText();
}

void SetPlain()
{
	int iShift;
	
	szPlain=message.GetPlain();
	
	SetText();
}

void SetPatterns()
{
	NGRAM pattern;
	int num_patterns=message.GetNumPatterns();
	
	SendDlgItemMessage(hMainWnd,IDC_PATTERNS,LB_RESETCONTENT,0,0);
	
	for(int cur_pat=0; cur_pat<num_patterns; cur_pat++)
	{
		message.GetPattern(cur_pat,&pattern);
		sprintf(szText,"%-12s %2i",pattern.string,pattern.freq);
		SendDlgItemMessage(hMainWnd,IDC_PATTERNS,LB_ADDSTRING,0,(WPARAM)szText);
	}

	sprintf(szText,"Patterns (%i)",num_patterns);
	SetDlgItemText(hMainWnd,IDC_PAT_TITLE,szText);
}

void SetTable()
{
	message.cur_map.SymbolTable(szText);
	SetDlgItemText(hMainWnd,IDC_TABLE,szText);
}

void SetKey()
{
	int cur_sel;
	SYMBOL symbol;

	sprintf(szText,"Key (%i symbols)",iSymbols);
	SetDlgItemText(hMainWnd,IDC_MAP_TITLE,szText);

	cur_sel=SendDlgItemMessage(hMainWnd,IDC_MAP,LB_GETCURSEL,0,0);
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_RESETCONTENT,0,0);
	
	for(int cur_symbol=0; cur_symbol<iSymbols; cur_symbol++)
	{
		message.cur_map.GetSymbol(cur_symbol,&symbol);
		if(!symbol.plain) symbol.plain='-';
		
		if(message.cur_map.GetLock(cur_symbol))
			sprintf(szText,"%c  [%c]  %3i",symbol.cipher,symbol.plain,symbol.freq);
		
		else sprintf(szText,"%c   %c   %3i",symbol.cipher,symbol.plain,symbol.freq);
		
		//SendDlgItemMessage(hMainWnd,IDC_MAP,LB_DELETESTRING,cur_symbol,0);
		//SendDlgItemMessage(hMainWnd,IDC_MAP,LB_INSERTSTRING,cur_symbol,(LPARAM)szText);
		SendDlgItemMessage(hMainWnd,IDC_MAP,LB_ADDSTRING,0,(LPARAM)szText);
		SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,cur_sel,0);
	}
	
	SetTable();
}

void SetFreq()
{
	int letter, diff, total_diff=0;
	int lprgiActFreq[26], lprgiExpFreq[26];
	int iActFreq, iExpFreq;
	int cur_sel;
	float act_vowel, exp_vowel;

	message.GetActFreq(lprgiActFreq);
	message.GetExpFreq(lprgiExpFreq);
	
	//letter frequencies
	cur_sel=SendDlgItemMessage(hMainWnd,IDC_LTRFREQ,LB_GETCURSEL,0,0);
	SendDlgItemMessage(hMainWnd,IDC_LTRFREQ,LB_RESETCONTENT,0,0);

	for(letter=0; letter<26; letter++)
	{
		iActFreq=lprgiActFreq[letter];
		iExpFreq=lprgiExpFreq[letter];
		
		diff=iActFreq-iExpFreq;
		if(diff<0) diff*=-1;
		total_diff+=diff;					

		sprintf(szText,"%c   %3i   %3i   %3i",letter+'A',iActFreq,iExpFreq,diff);
		//SendDlgItemMessage(hMainWnd,IDC_LTRFREQ,LB_DELETESTRING,letter,0);
		//SendDlgItemMessage(hMainWnd,IDC_LTRFREQ,LB_INSERTSTRING,letter,(LPARAM)szText);
		SendDlgItemMessage(hMainWnd,IDC_LTRFREQ,LB_ADDSTRING,0,(LPARAM)szText);
		SendDlgItemMessage(hMainWnd,IDC_LTRFREQ,LB_SETCURSEL,cur_sel,0);
	}

	SetDlgItemInt(hMainWnd,IDC_DIFF,total_diff,0);

	//vowel percentage
	act_vowel=float(lprgiActFreq[0]+lprgiActFreq[4]+lprgiActFreq[8]+lprgiActFreq[14]+lprgiActFreq[20]);
	act_vowel=(100*act_vowel)/iCipherLength;
	exp_vowel=float(unigraphs[0]+unigraphs[4]+unigraphs[8]+unigraphs[14]+unigraphs[20]);
	sprintf(szText,"%.2f%% / %.2f%%",act_vowel,exp_vowel);
	SetDlgItemText(hMainWnd,IDC_VOWEL,szText);
}

void SetSolve()
{
	//iteration & time
	sprintf(szText,"%i (%.2fs)",siSolveInfo.cur_try,siSolveInfo.last_time);
	SetDlgItemText(hMainWnd,IDC_TRY,szText);
	
	//failures
	sprintf(szText,"%i of %i",siSolveInfo.cur_fail,siSolveInfo.max_fail);
	SetDlgItemText(hMainWnd,IDC_FAIL,szText);
	
	//best score
	if(siSolveInfo.running) iBestScore=siSolveInfo.best_score;
	SetDlgItemInt(hMainWnd,IDC_SCORE,iBestScore,0);
}

void SetDlgInfo()
{
	if(!bMsgLoaded) return;
	
	//set key to hillclimber best if running
	if(siSolveInfo.running)
		KeyToMap(message.cur_map,siSolveInfo.best_key);
		
	SetPlain();
	SetKey();
	SetFreq();
	SetSolve();
}

/*Solve Functions*/

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
		EnableMenuItem(hMainMenu,IDM_EDIT_INIT,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_EDIT_SCRAMBLE,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_EDIT_CLEAR,MF_BYCOMMAND | menu_state);
		EnableMenuItem(hMainMenu,IDM_SOLVE_WORD,MF_BYCOMMAND | menu_state);
		Button_Enable(GetDlgItem(hMainWnd,IDC_CHANGE),enabled);
		Button_Enable(GetDlgItem(hMainWnd,IDC_RESET),enabled);
	}
	
	if(bMsgLoaded) menu_state=MF_ENABLED;
	else menu_state=MF_GRAYED;
	
	EnableMenuItem(hMainMenu,IDM_EDIT_MSG,MF_BYCOMMAND | menu_state);
}

void MapEnable(int enabled)
{
	int menu_state;

	if(enabled) menu_state=MF_ENABLED;
	else menu_state=MF_GRAYED;
			
	if(enabled==false || bMapLoaded)
	{
		EnableMenuItem(hMainMenu,IDM_EDIT_RELOAD,MF_BYCOMMAND | menu_state);
	}

	if(enabled && bUndo) menu_state=MF_ENABLED;
	else menu_state=MF_GRAYED;

	EnableMenuItem(hMainMenu,IDM_EDIT_UNDO,MF_BYCOMMAND | menu_state);
	
	if(bMapLoaded) menu_state=MF_ENABLED;
	else menu_state=MF_GRAYED;

	EnableMenuItem(hMainMenu,IDM_EDIT_MAP,MF_BYCOMMAND | menu_state);
}

void StopSolve()
{
	siSolveInfo.running=false;
	SetDlgItemText(hMainWnd,IDC_SOLVE,"Start");
	MsgEnable(true);
	MapEnable(true);
}

inline DWORD GetTime() {return GetTickCount();}

DWORD WINAPI Timer(LPVOID lpVoid)
{
	char szTime[8];
	int hr=0, min=0, sec=0;
	
	while(siSolveInfo.running)
	{
		sprintf(szTime,"%02i:%02i:%02i",hr,min,sec);
		SetDlgItemText(hMainWnd,IDC_TIME,szTime);
		if(++sec==60) {min++; sec=0;}
		if(min==60) {hr++; min=0;}
		Sleep(1000);
	}

	ExitThread(0);
	return 0;
}

DWORD WINAPI FindSolution(LPVOID lpVoid) //Solve thread function
{
	char key[256];
	
	if(!bMsgLoaded) return 0;
	
	SetThreadPriority(hSolveThread,iPriority);
	
	MapToKey(message.cur_map,key);
	
	//iBestScore=Solve(message,siSolveInfo,iUseGraphs);	
	hillclimb(message.GetCipher(),iCipherLength,key,
			  message.cur_map.GetLocked(),siSolveInfo,iUseGraphs);

	StopSolve();
	SetDlgInfo();

	hSolveThread=NULL;
	ExitThread(0);
	return 0;
}

void StartSolve()
{
	siSolveInfo.running=true;
	SetDlgItemText(hMainWnd,IDC_SOLVE,"Stop");
	MsgEnable(false);
	MapEnable(false);
	hSolveThread=CreateThread(0,1024,FindSolution,0,0,0);
	CreateThread(0,128,Timer,0,0,0);
}

void Reset() //init solve parameters
{
	siSolveInfo.cur_try=0;
	siSolveInfo.cur_fail=0;
	siSolveInfo.last_time=0;
	SetDlgItemText(hMainWnd,IDC_TIME,"00:00:00");
	iBestScore=0;
	SetDlgInfo();
}

void SetPriority(int iNewPriority)
{
	if(iNewPriority==3)
	{
		iPriority=THREAD_PRIORITY_ABOVE_NORMAL;//TTHREAD_PRIORITY_HIGHEST;
		CheckMenuItem(hMainMenu,IDM_EDIT_TP_HIGH,MF_BYCOMMAND | MF_CHECKED);
	}

	else CheckMenuItem(hMainMenu,IDM_EDIT_TP_HIGH,MF_BYCOMMAND | MF_UNCHECKED);

	if(iNewPriority==2)
	{
		iPriority=THREAD_PRIORITY_NORMAL;
		CheckMenuItem(hMainMenu,IDM_EDIT_TP_NORM,MF_BYCOMMAND | MF_CHECKED);
	}

	else CheckMenuItem(hMainMenu,IDM_EDIT_TP_NORM,MF_BYCOMMAND | MF_UNCHECKED);

	if(iNewPriority==1)
	{
		iPriority=THREAD_PRIORITY_BELOW_NORMAL;//THREAD_PRIORITY_IDLE;
		CheckMenuItem(hMainMenu,IDM_EDIT_TP_LOW,MF_BYCOMMAND | MF_CHECKED);
	}

	else CheckMenuItem(hMainMenu,IDM_EDIT_TP_LOW,MF_BYCOMMAND | MF_UNCHECKED);

	if(hSolveThread) 
		if(!SetThreadPriority(hSolveThread,iPriority))
			MessageBox(hMainWnd,"Could not set Priority","Error",MB_ICONEXCLAMATION);
}

void SetLanguage()
{
	sprintf(szLangDir,"%s%s\\",szExeDir,LANG_DIR);

	switch(iLang)
	{
		case 0: strcpy(szLang,LANG_ENG); break;
		//case 1: strcpy(szLang,LANG_SPA); break;
		//case 2: strcpy(szLang,LANG_GER); break;
	}

	if(!read_ngraphs(szLangDir,szLang))
	{
		sprintf(szText,"Could not open language data from \"%s%s\"",szLangDir,szLang);
		MessageBox(hMainWnd,szText,"Error",MB_ICONEXCLAMATION);
		SendMessage(hMainWnd,WM_CLOSE,0,0);
	}
}

/*File Functions*/

void GetBaseName(const char *filename, char *&basename) 
{
	if(filename[0]!='\0') basename=strrchr(filename,'\\')+1;
	else basename=NULL;
}

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

/*Edit Functions*/

void SetUndo()
{
	bUndo=true;
	EnableMenuItem(hMainMenu,IDM_EDIT_UNDO,MF_BYCOMMAND | MF_ENABLED);
	mUndoMap=message.cur_map;
}

void Undo()
{
	bUndo=false;
	EnableMenuItem(hMainMenu,IDM_EDIT_UNDO,MF_BYCOMMAND | MF_GRAYED);
	message.cur_map=mUndoMap;
}

void SetSymbol(int index)
{
	SYMBOL symbol;
	char plain;

	message.cur_map.GetSymbol(index,&symbol);
	if(symbol.plain) plain=symbol.plain;
	else plain='-';
	
	if(message.cur_map.GetLock(index))
			sprintf(szText,"%c  [%c]  %3i",symbol.cipher,plain,symbol.freq);
		
	else sprintf(szText,"%c   %c   %3i",symbol.cipher,plain,symbol.freq);

	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_DELETESTRING,index,0);
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_INSERTSTRING,index,(LPARAM)szText);
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,index,0);
}

void ChangePlain()
{
	SYMBOL symbol;

	if(iCurSymbol<0) return;

	SetUndo();
	GetDlgItemText(hMainWnd,IDC_MAP_VALUE,szText,10);
	message.cur_map.GetSymbol(iCurSymbol,&symbol);
	symbol.plain=szText[0];
	message.cur_map.AddSymbol(symbol,0);
			
	SetSymbol(iCurSymbol);
	SetPlain();
	SetTable();
	SetFreq();
}

/*Window Functions*/
/*
LRESULT CALLBACK OutlineProc(HWND hWnd, UINT iMsg, 
						    WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	
	switch(iMsg)
	{
		case WM_PAINT:
			BeginPaint(hWnd,&ps);
			SetText();
			EndPaint(hWnd,&ps);
			return 0;
	}

	return(DefWindowProc(hWnd, iMsg, wParam, lParam));
}
*/
LRESULT CALLBACK AboutProc(HWND hWnd, UINT iMsg, 
						   WPARAM wParam, LPARAM lParam)
{
	switch(iMsg)
	{
		case WM_INITDIALOG:
			sprintf(szText,"%s %s",PROG_NAME,PROG_VER);
			SetDlgItemText(hWnd,IDC_PROG,szText);
			strcpy(szText,"Wesley Hopper (hopperw2000@yahoo.com)");
			SetDlgItemText(hWnd,IDC_ABOUT1,szText);
			strcpy(szText,"Brax Sisco (xenex@bardstowncable.net)");
			SetDlgItemText(hWnd,IDC_ABOUT2,szText);
			strcpy(szText,"Michael Eaton");
			SetDlgItemText(hWnd,IDC_ABOUT3,szText);
			return 0;

		case WM_LBUTTONDOWN:
			EndDialog(hWnd,0);
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDCANCEL:
					EndDialog(hWnd,0);
					return 0;
			}
	}

	return 0;
}

LRESULT CALLBACK OptionsProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg)
	{
		case WM_INITDIALOG:
			SetDlgItemInt(hWnd,IDC_MAXFAIL,siSolveInfo.max_fail,0);
			SetDlgItemInt(hWnd,IDC_SWAPS,siSolveInfo.swaps,0);
			SetDlgItemInt(hWnd,IDC_REVERT,siSolveInfo.revert,0);

			SendMessage(GetDlgItem(hWnd,IDC_USEBI),BM_SETCHECK,iUseGraphs & USE_BI,0);
			SendMessage(GetDlgItem(hWnd,IDC_USETRI),BM_SETCHECK,iUseGraphs & USE_TRI,0);
			SendMessage(GetDlgItem(hWnd,IDC_USETETRA),BM_SETCHECK,iUseGraphs & USE_TETRA,0);
			SendMessage(GetDlgItem(hWnd,IDC_USEPENTA),BM_SETCHECK,iUseGraphs & USE_PENTA,0);

			SetDlgItemInt(hWnd,IDC_LINECHARS,iLineChars,0);

			//add languages			
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"English");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"Spanish");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"German");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_SETCURSEL,iLang,0);
			
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					siSolveInfo.max_fail=GetDlgItemInt(hWnd,IDC_MAXFAIL,0,0);
					siSolveInfo.swaps=GetDlgItemInt(hWnd,IDC_SWAPS,0,0);
					siSolveInfo.revert=GetDlgItemInt(hWnd,IDC_REVERT,0,0);

					iUseGraphs=0;
					
					if(SendMessage(GetDlgItem(hWnd,IDC_USEBI),BM_GETCHECK,0,0))
						iUseGraphs+=USE_BI;
						
					if(SendMessage(GetDlgItem(hWnd,IDC_USETRI),BM_GETCHECK,0,0))
						iUseGraphs+=USE_TRI;
						
					if(SendMessage(GetDlgItem(hWnd,IDC_USETETRA),BM_GETCHECK,0,0))
						iUseGraphs+=USE_TETRA;
						
					if(SendMessage(GetDlgItem(hWnd,IDC_USEPENTA),BM_GETCHECK,0,0))
						iUseGraphs+=USE_PENTA;

					iLineChars=GetDlgItemInt(hWnd,IDC_LINECHARS,0,0);

					iLang=SendDlgItemMessage(hWnd,IDC_LANG,CB_GETCURSEL,0,0);
					
					SetScrollBar();					
					SetLanguage();
					SetDlgInfo();
					SetCipher();

				case IDCANCEL:
					EndDialog(hWnd,0);
					return 0;
			}
	}

	return 0;
}

LRESULT CALLBACK WordProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	char szWord[128];

	switch(iMsg)
	{
		case WM_INITDIALOG:
			SetFocus(GetDlgItem(hWnd,IDC_WORD));
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					GetDlgItemText(hWnd,IDC_WORD,szWord,128);
					SetUndo();
					iBestScore=WordPlug(message,szWord,iUseGraphs);
					SetDlgInfo();
					EndDialog(hWnd,0);
					return 1;

				case IDCANCEL:
					EndDialog(hWnd,0);
					return 0;
			}
	}

	return 0;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int swap, num_symbols;
	SYMBOL symbol;
	char filename[1024];

	switch(iMsg)
	{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDM_FILE_OPEN_MSG:
					if(!GetFilename(filename,szCipherName,0)) return 0;
					LoadMessage(filename);				
					return 0;

				case IDM_FILE_OPEN_MAP:
					if(!GetFilename(filename,szKeyName,0)) return 0;
					LoadMap(filename);
					return 0;

				case IDM_FILE_SAVE_MAP:
					if(!bMsgLoaded) return 0;
					if(GetFilename(filename,szKeyName,1)!=1) return 0;
					if(message.cur_map.Write(filename))
						strcpy(szKeyName,filename);
					
					bMapLoaded=true;
					MapEnable(true);
					SetTitle();
					return 0;

				case IDM_EDIT_UNDO:
					Undo();
					SetDlgInfo();
					return 0;

				case IDM_EDIT_MSG:
					if(!bMsgLoaded) return 0;
					OpenWith(szCipherName);
					return 0;
				
				case IDM_EDIT_MAP:
					if(!bMsgLoaded) return 0;
					OpenWith(szKeyName);
					return 0;

				case IDM_EDIT_RELOAD: LoadMap(szKeyName); return 0;

				case IDM_EDIT_INIT:
					SetUndo();
					message.cur_map.Init();
					SetDlgInfo();
					return 0;

				case IDM_EDIT_SCRAMBLE:
					num_symbols=message.cur_map.GetNumSymbols();
					for(swap=0; swap<3000; swap++)
						message.cur_map.SwapSymbols(rand()%num_symbols,rand()%num_symbols);
					SetDlgInfo();
					return 0;

				case IDM_EDIT_CLEAR:
					SetUndo();
					message.cur_map.Clear(CLR_PLAIN);
					SetDlgInfo();
					return 0;

				case IDM_EDIT_LOCK: message.cur_map.SetAllLock(true); SetKey(); return 0;
				case IDM_EDIT_UNLOCK: message.cur_map.SetAllLock(false); SetKey(); return 0;

				case IDM_SOLVE_OPTIONS:
					DialogBox(hInst,MAKEINTRESOURCE(IDD_OPTIONS),hMainWnd,(DLGPROC)OptionsProc);
					return 0;

				case IDM_SOLVE_TP_HIGH: SetPriority(3); return 0;	
				case IDM_SOLVE_TP_NORM: SetPriority(2); return 0;	
				case IDM_SOLVE_TP_LOW: SetPriority(1); return 0;										

				case IDM_SOLVE_WORD:
					DialogBox(hInst,MAKEINTRESOURCE(IDD_WORD),hMainWnd,(DLGPROC)WordProc);
					return 0;

				case IDM_HELP_ABOUT:
					DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),hMainWnd,(DLGPROC)AboutProc);
					return 0;

				case IDC_CHANGE:
					ChangePlain();
					return 0;

				case IDC_SOLVE:
					SetUndo();
					if(!bMsgLoaded) return 0;

					if(siSolveInfo.running) StopSolve();
					else StartSolve();
					return 0;

				case IDC_RESET: Reset(); return 0;

				case IDC_PATTERNS:
					switch(HIWORD(wParam))
					{
						case LBN_SELCHANGE:
							iCurPat=SendDlgItemMessage(hMainWnd,IDC_PATTERNS,LB_GETCURSEL,0,0);
							SetText();
					}
					return 0;
					
				case IDC_MAP:
					switch(HIWORD(wParam))
					{
						case LBN_SELCHANGE:
							iCurSymbol=SendDlgItemMessage(hMainWnd,IDC_MAP,LB_GETCURSEL,0,0);
							if(iCurSymbol<0) return 0;

							message.cur_map.GetSymbol(iCurSymbol,&symbol);
							sprintf(szText,"%c",symbol.plain);
							SetDlgItemText(hMainWnd,IDC_MAP_VALUE,szText);
							SetText();
							break;

						case LBN_DBLCLK:
							iCurSymbol=SendDlgItemMessage(hMainWnd,IDC_MAP,LB_GETCURSEL,0,0);
							if(iCurSymbol<0) return 0;
							message.cur_map.ToggleLock(iCurSymbol);
							SetSymbol(iCurSymbol);
							break;
					}
					return 0;
					
				case UDM_DISPALL:
					SetDlgInfo();
					return 0;
					
				case UDM_DISPINFO:
					SetSolve();
					return 0;
			}

			return 0;
			
		case WM_VSCROLL:
			switch(LOWORD(wParam))
			{
				case SB_PAGEDOWN:	iScrollPos+=5; break;
				case SB_LINEDOWN:	iScrollPos++; break;
				case SB_PAGEUP:		iScrollPos-=5; break;
				case SB_LINEUP:		iScrollPos--; break;
				case SB_TOP:		iScrollPos=0; break;
				case SB_BOTTOM:		iScrollPos=iLines-iDispLines; break;
				case SB_THUMBTRACK:	iScrollPos=HIWORD(wParam); break;
			}
			
			if(iScrollPos<0) iScrollPos=0;
			if(iScrollPos>iLines-iDispLines)
				iScrollPos=iLines-iDispLines;
				
			SetScrollPos(hScroll,SB_CTL,iScrollPos,true);
			
			SetText();
			return 0;
			
        case WM_DROPFILES:
			DragQueryFile((HDROP)wParam,0,szText,sizeof(szText));
			LoadMessage(szText);
			DragFinish((HDROP)wParam);
			return 0;
			
		case WM_CLOSE:
			EndDialog(hMainWnd,0);
			PostMessage(hMainWnd,WM_DESTROY,0,0);
			return 0;

		case WM_DESTROY: 
			PostQuitMessage(0); 
			return 0;
	}

	return 0;
}

void disp_all()  {SendMessage(hMainWnd,WM_COMMAND,UDM_DISPALL,0);}
void disp_info() {SendMessage(hMainWnd,WM_COMMAND,UDM_DISPINFO,0);}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpszCmdLine, int nCmdShow)
{
	hInst=hInstance;
	MSG Msg;

	/*Initialize common controls*/
    /*INITCOMMONCONTROLSEX icInitCC;
    icInitCC.dwSize=sizeof(INITCOMMONCONTROLSEX);
    icInitCC.dwICC=ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icInitCC);

	if(!LoadLibrary(TEXT("riched32.dll")))
		MessageBox(NULL,"Could not load riched32.dll","Error",MB_ICONEXCLAMATION);*/

	//create window
	hMainWnd=CreateDialog(hInst,MAKEINTRESOURCE(IDD_MAIN_DLG),NULL,(DLGPROC)MainWndProc);
	SendMessage(hMainWnd,WM_SETICON,ICON_BIG,(WPARAM)LoadIcon(hInst,MAKEINTRESOURCE(IDI_ZODIAC)));
	hMainMenu=GetMenu(hMainWnd);
	
	//hCipher=GetDlgItem(hMainWnd,IDC_CIPHER);
	//hPlain=GetDlgItem(hMainWnd,IDC_PLAIN);
	hCipher=CreateWindowEx(WS_EX_CLIENTEDGE,"STATIC","",WS_CHILD | SS_NOPREFIX,202,30,223,TEXT_HEIGHT,hMainWnd,NULL,hInst,NULL);
	hPlain=CreateWindowEx(WS_EX_CLIENTEDGE,"STATIC","",WS_CHILD | SS_NOPREFIX,426,30,223,TEXT_HEIGHT,hMainWnd,NULL,hInst,NULL);
	hCipherDC=GetWindowDC(hCipher);
	hPlainDC=GetWindowDC(hPlain);
	hScroll=GetDlgItem(hMainWnd,IDC_SCROLL);
	SetScrollRange(hScroll,SB_CTL,0,0,true);

	
	//drawing
	hRedPen=CreatePen(0,0,RGB(255,0,0));
	hBluePen=CreatePen(0,0,RGB(0,0,255));
	SelectObject(hCipherDC,(HBRUSH)GetStockObject(NULL_BRUSH));
	SelectObject(hPlainDC,(HBRUSH)GetStockObject(NULL_BRUSH));
	
	SetScrollBar();
	iCurPat=-1;

	HFONT hFont=CreateFont(iCharHeight,iCharWidth,0,0,FW_NORMAL,0,0,0,0,0,0,0,0,"Lucida Console");
	if(hCipher) SendMessage(hCipher,WM_SETFONT,(WPARAM)hFont,0);
	if(hPlain) SendMessage(hPlain,WM_SETFONT,(WPARAM)hFont,0);

	//setup window
	SetTitle();
	MsgEnable(false);
	MapEnable(false);
	EnableMenuItem(hMainMenu,IDM_FILE_OPEN_MSG,MF_BYCOMMAND | MF_ENABLED);
	Button_Enable(GetDlgItem(hMainWnd,IDC_SOLVE),false);
	SendDlgItemMessage(hMainWnd,IDC_MAP_VALUE,EM_LIMITTEXT,1,0);
	SetPriority(2);
	strcpy(szLang,"eng");
	StopSolve();
	
	//init solve info
	memset(&siSolveInfo,0,sizeof(SOLVEINFO));
	siSolveInfo.max_fail=100;
	siSolveInfo.swaps=5;
	siSolveInfo.revert=120*5-1;
	siSolveInfo.disp_all=disp_all;
	siSolveInfo.disp_info=disp_info;
	siSolveInfo.time_func=GetTime;
	Reset();

	//executable's directory
	GetModuleFileName(hInst,szExeDir,1024);
	char *szEndDir;
	szEndDir=strrchr(szExeDir,'\\');
	if(szEndDir) *(szEndDir+1)='\0';
	
	//set open/save dir as executable dir
	strcpy(szCipherName,szExeDir);
	strcpy(szKeyName,szExeDir);

	iLang=0;
	SetLanguage();
	
	/*
	for(int n=2; n<=5; n++)
	{
		if(n==2)  strcpy(szGraphBase,"bigraphs.txt");
		if(n==3)  strcpy(szGraphBase,"trigraphs.txt");
		if(n==4)  strcpy(szGraphBase,"tetragraphs.txt");
		if(n==5)  strcpy(szGraphBase,"pentagraphs.txt");
		
		sprintf(szGraphName,"%s\\data\\%s",szExeDir,szGraphBase);

		if(!ReadNGraphs(szGraphName,n))
		{
			sprintf(szText,"Could not open %s",szGraphName);
			MessageBox(hMainWnd,szText,"Error",MB_ICONEXCLAMATION);
			SendMessage(hMainWnd,WM_CLOSE,0,0);
		}
	}*/
	
	srand(time(0));

	ShowWindow(hMainWnd,SW_SHOWNORMAL);
	ShowWindow(hCipher,SW_SHOWNORMAL);
	ShowWindow(hPlain,SW_SHOWNORMAL);
	
	//Open From Command Line
	if(lpszCmdLine[0]!='\0') 
	{
		strcpy(szCipherName,lpszCmdLine); 
		message.Read(szCipherName);
	}

	while(GetMessage(&Msg,NULL,0,0))
		if(!IsDialogMessage(hMainWnd,&Msg))
		{	
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}

	return(Msg.wParam);
}
