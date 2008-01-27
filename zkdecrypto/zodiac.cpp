#include <stdio.h>
#include <time.h>
#include <math.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <richedit.h>
#include "headers/resource.h"
#include "headers/zodiac.h"
#include "headers/display.h"
#include "headers/solve.h"
#include "headers/files.h"

/*Edit Functions*/

void SetUndo()
{
	bUndo=true;
	EnableMenuItem(hMainMenu,IDM_EDIT_UNDO,MF_BYCOMMAND | MF_ENABLED);
	undo_message+=message;
}

void Undo()
{
	bUndo=false;
	EnableMenuItem(hMainMenu,IDM_EDIT_UNDO,MF_BYCOMMAND | MF_GRAYED);
	message=undo_message;
	SetCipher();
}

//change letter mapped to symbol
void ChangePlain()
{
	SYMBOL symbol;

	if(iCurSymbol<0) return;

	SetUndo();
	
	//get new letter
	GetDlgItemText(hMainWnd,IDC_MAP_VALUE,szText,10);
	
	//get and update symbol
	message.cur_map.GetSymbol(iCurSymbol,&symbol);
	symbol.plain=szText[0];
	message.cur_map.AddSymbol(symbol,0);
	
	//update info		
	UpdateSymbol(iCurSymbol);
	SetPlain();
	SetTable();
	SetFreq();
}

/*Window Functions*/

//merge dialog
LRESULT CALLBACK MergeProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int num_symbols, cur_symbol;
	char cipher1, cipher2;
	SYMBOL symbol;
	
	switch(iMsg)
	{
		case WM_INITDIALOG:
			num_symbols=message.cur_map.GetNumSymbols();
			for(cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
			{
				message.cur_map.GetSymbol(cur_symbol,&symbol);
				sprintf(szText,"%c",symbol.cipher);
				SendDlgItemMessage(hWnd,IDC_MERSYM1,CB_ADDSTRING,0,(LPARAM)szText);
				SendDlgItemMessage(hWnd,IDC_MERSYM2,CB_ADDSTRING,0,(LPARAM)szText);
			}

			SetFocus(GetDlgItem(hWnd,IDC_MERSYM1));
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					//get 1st symbol cipher
					cur_symbol=SendDlgItemMessage(hWnd,IDC_MERSYM1,CB_GETCURSEL,0,0);
					message.cur_map.GetSymbol(cur_symbol,&symbol);
					cipher1=symbol.cipher;
					
					//get 2nd symbol cipher
					cur_symbol=SendDlgItemMessage(hWnd,IDC_MERSYM2,CB_GETCURSEL,0,0);
					message.cur_map.GetSymbol(cur_symbol,&symbol);
					cipher2=symbol.cipher;
					
					message.MergeSymbols(cipher1,cipher2,true);
					
					SetCipher();
					
					EndDialog(hWnd,0);
					return 1;

				case IDCANCEL:
					EndDialog(hWnd,0);
					return 0;
			}
	}

	return 0;
}

//graphs dialog
LRESULT CALLBACK GraphsProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int iWidth, iHeight;

	switch(iMsg)
	{
		case WM_INITDIALOG:	
			iWidth=LOWORD(lRowCol)*CHAR_WIDTH;
			iHeight=HIWORD(lRowCol)*CHAR_HEIGHT+20;
			SetWindowPos(GetDlgItem(hWnd,IDC_GRAPH),0,0,0,iWidth,iHeight,SWP_NOREPOSITION | SWP_NOMOVE);
			SetWindowPos(hWnd,0,0,0,iWidth+(iMargin<<1),iHeight+(iMargin<<1),SWP_NOREPOSITION | SWP_NOMOVE);
			SetDlgItemTextW(hWnd,IDC_GRAPH,(WCHAR*)szGraph);
			SetWindowText(hWnd,szGraphTitle);
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					EndDialog(hWnd,0);
					hGraph=NULL;
					return 0;
			}
	}

	return 0;
}

//init dialog
LRESULT CALLBACK PolyProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int iPolySize;
	
	switch(iMsg)
	{
		case WM_INITDIALOG:
			SetFocus(GetDlgItem(hWnd,IDC_NUMBER));
			SendDlgItemMessage(hWnd,IDC_NUMBER,EM_LIMITTEXT,3,0);
			SetDlgItemInt(hWnd,IDC_NUMBER,25,0);
			SetWindowText(hWnd,"Max Key Length");
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					iPolySize=GetDlgItemInt(hWnd,IDC_NUMBER,0,0);
					EndDialog(hWnd,0);
					lRowCol=message.PolyKeySize(szGraph,iPolySize);
					strcpy(szGraphTitle,"Polyalphabetic IoC Count");
					DialogBox(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
					return 1;

				case IDCANCEL:
					EndDialog(hWnd,0);
					return 0;
			}
	}

	return 0;
}

//init dialog
LRESULT CALLBACK InitProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int iInitSize;
	
	switch(iMsg)
	{
		case WM_INITDIALOG:
			SetFocus(GetDlgItem(hWnd,IDC_NUMBER));
			SendDlgItemMessage(hWnd,IDC_NUMBER,EM_LIMITTEXT,3,0);
			SetDlgItemInt(hWnd,IDC_NUMBER,message.cur_map.GetNumSymbols(),0);
			SetWindowText(hWnd,"Symbols to Set");
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					iInitSize=GetDlgItemInt(hWnd,IDC_NUMBER,0,0);
					SetUndo();
					message.cur_map.Init(iInitSize);
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

//word plug dialog
LRESULT CALLBACK WordProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	char szWord[128];
	int iLength, bChange;

	switch(iMsg)
	{
		case WM_INITDIALOG:
			SetFocus(GetDlgItem(hWnd,IDC_WORD));
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_WORD: 
					if(HIWORD(wParam)==EN_CHANGE)
					{
						GetDlgItemText(hWnd,IDC_WORD,szWord,128);
						iLength=(int)strlen(szWord);
						
						//remove spaces from string
						for(int iChar=0; iChar<iLength; iChar++)
							if(szWord[iChar]==' ')
							{
								memmove(szWord+iChar,szWord+iChar+1,iLength-iChar);
								iChar--; iLength--;
								bChange=true;
							}
						
						//reset string
						if(bChange)
						{
							SetDlgItemText(hWnd,IDC_WORD,szWord);
							SendDlgItemMessage(hWnd,IDC_WORD,EM_SETSEL,iLength,iLength);
						}
					}
					return 0;
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

//options dialog
LRESULT CALLBACK OptionsProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int iPrevLang;

	switch(iMsg)
	{
		case WM_INITDIALOG: //init values
			//hillclimber parameters
			SetDlgItemInt(hWnd,IDC_MAXFAIL,siSolveInfo.max_fail,0);
			SetDlgItemInt(hWnd,IDC_SWAPS,siSolveInfo.swaps,0);
			SetDlgItemInt(hWnd,IDC_REVERT,siSolveInfo.revert,0);

			//score parameters
			SendMessage(GetDlgItem(hWnd,IDC_USEBI),BM_SETCHECK,iUseGraphs & USE_BI,0);
			SendMessage(GetDlgItem(hWnd,IDC_USETRI),BM_SETCHECK,iUseGraphs & USE_TRI,0);
			SendMessage(GetDlgItem(hWnd,IDC_USETETRA),BM_SETCHECK,iUseGraphs & USE_TETRA,0);
			SendMessage(GetDlgItem(hWnd,IDC_USEPENTA),BM_SETCHECK,iUseGraphs & USE_PENTA,0);

			//display options
			SetDlgItemInt(hWnd,IDC_LINECHARS,iLineChars,0);

			//language		
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"English");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"Spanish");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"German");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_SETCURSEL,iLang,0);

			//extra letters
			SendDlgItemMessage(hWnd,IDC_EXTRA_LTR,EM_LIMITTEXT,MAX_EXTRA,0);
			SetDlgItemText(hWnd,IDC_EXTRA_LTR,szExtraLtr);
			
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK: //get new values
					//hillclimber parameters
					siSolveInfo.max_fail=GetDlgItemInt(hWnd,IDC_MAXFAIL,0,0);
					siSolveInfo.swaps=GetDlgItemInt(hWnd,IDC_SWAPS,0,0);
					siSolveInfo.revert=GetDlgItemInt(hWnd,IDC_REVERT,0,0);

					//score parameters
					iUseGraphs=0;
					
					if(SendMessage(GetDlgItem(hWnd,IDC_USEBI),BM_GETCHECK,0,0))
						iUseGraphs+=USE_BI;
						
					if(SendMessage(GetDlgItem(hWnd,IDC_USETRI),BM_GETCHECK,0,0))
						iUseGraphs+=USE_TRI;
						
					if(SendMessage(GetDlgItem(hWnd,IDC_USETETRA),BM_GETCHECK,0,0))
						iUseGraphs+=USE_TETRA;
						
					if(SendMessage(GetDlgItem(hWnd,IDC_USEPENTA),BM_GETCHECK,0,0))
						iUseGraphs+=USE_PENTA;

					//display options
					iLineChars=GetDlgItemInt(hWnd,IDC_LINECHARS,0,0);

					//language
					iPrevLang=iLang;
					iLang=SendDlgItemMessage(hWnd,IDC_LANG,CB_GETCURSEL,0,0);
					if(iLang!=iPrevLang) SetLanguage();

					//extra letters
					GetDlgItemText(hWnd,IDC_EXTRA_LTR,szExtraLtr,MAX_EXTRA);
					
					//0 chars per line
					if(!iLineChars)
					{
						MessageBox(hWnd,"Line length must be greather than 0","Notice",MB_ICONEXCLAMATION);
						return 0;
					}
					
					//no ngrams are checked
					if(!iUseGraphs)
					{
						MessageBox(hWnd,"At least one set of ngrams must be selected","Notice",MB_ICONEXCLAMATION);
						return 0;
					}
					
					//update display
					SetScrollBar();					
					SetDlgInfo();
					SetCipher();

				case IDCANCEL:
					EndDialog(hWnd,0);
					return 0;
			}
	}

	return 0;
}

//about dalog
LRESULT CALLBACK AboutProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg)
	{
		case WM_INITDIALOG:
			sprintf(szText,"%s %s",PROG_NAME,PROG_VER);
			SetDlgItemText(hWnd,IDC_PROG,szText);
			
			strcpy(szText,"Wesley Hopper (hopperw2000@yahoo.com)\r\n\r\n");
			strcat(szText,"Brax Sisco (xenex@bardstowncable.net)\r\n\r\n");
			strcat(szText,"Michael Eaton (michaeleaton@gmail.com)\r\n\r\n");

			SetDlgItemText(hWnd,IDC_ABOUT,szText);
			return 0;

		case WM_LBUTTONDOWN:
			EndDialog(hWnd,0);
			return 0;
	}

	return 0;
}

//message handler for text window
LRESULT CALLBACK TextWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int iWidth, iHeight;
	MINMAXINFO *mmiInfo;
	PAINTSTRUCT ps;

	switch(iMsg)
	{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_TS_UP:
					iCharSize+=float(0.1);
					SetCharSize();
					return 0;

				case IDC_TS_DOWN:
					iCharSize-=float(0.1);
					SetCharSize();
					return 0;
			}

	//scroll text
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
			if(iScrollPos>iMaxScroll) iScrollPos=iMaxScroll;
				
			SetScrollPos(hScroll,SB_CTL,iScrollPos,true);
			
			SetText();
			return 0;

		//click on text
		case WM_LBUTTONDOWN:
			TextClick(LOWORD(lParam),HIWORD(lParam));
			return 0;	

		case WM_RBUTTONDOWN:
			CreateTextMenu();
			return 0;

		//redraw window
		case WM_PAINT:
			BeginPaint(hWnd,&ps);
			DrawOutlines();
			EndPaint(hWnd,&ps);
			return 0;

		case WM_GETMINMAXINFO:
			mmiInfo=(MINMAXINFO*)lParam;
			mmiInfo->ptMinTrackSize.x=400;
			mmiInfo->ptMinTrackSize.y=300;
			return 0;

		case WM_SIZE:
			iWidth=LOWORD(lParam);
			iHeight=HIWORD(lParam);
			ResizeText(iWidth,iHeight);
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

//message handler for main window
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int swap, num_symbols, new_pat;
	char simp1,simp2;
	long time1,time2;
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
					return 0;

				case IDM_FILE_SAVE_PLAIN:
					if(!bMsgLoaded) return 0;
					if(GetFilename(filename,szCipherName,1)!=1) return 0;
					if(!SavePlain(filename))
					{
						sprintf(szText,"Could not save \"%s\"",filename);
						MessageBox(hMainWnd,szText,"Error",MB_ICONEXCLAMATION);
					}
					return 0;

				case IDM_FILE_COPY_PLAIN:
					OpenClipboard(hMainWnd);
					EmptyClipboard();
					SetClipboardData(CF_TEXT,(void*)message.GetPlain());
					CloseClipboard();
					return 0;

				case IDM_FILE_EXIT:
					SendMessage(hMainWnd,WM_CLOSE,0,0);
					return 0;
				
				/*edit menu*/
				case IDM_EDIT_UNDO: Undo(); SetCipher(); return 0;
				case IDM_EDIT_MSG: if(bMsgLoaded) OpenWith(szCipherName); return 0;
				case IDM_EDIT_MAP: if(bMsgLoaded) OpenWith(szKeyName); return 0;
				
				/*cipher menu*/
				case IDM_CIPHER_MERGE:
					SetUndo();
					DialogBox(hInst,MAKEINTRESOURCE(IDD_MERGE),hMainWnd,(DLGPROC)MergeProc);
					return 0;
					
				case IDM_CIPHER_SIMPLIFY:
					//run simplify
					SetCursor(LoadCursor(0,IDC_WAIT));
					time1=GetTickCount();
					new_pat=message.Simplify(simp1,simp2);
					time2=GetTickCount();
					SetCursor(LoadCursor(0,IDC_ARROW));

					if(simp1==char(0xFF)) //no good substituion found
						return MessageBox(hMainWnd,"No substitions found","Subsitution",MB_YESNO);

					sprintf(szText,"Merge '%c' with '%c'? (%.2fs)",simp1,simp2,double(time2-time1)/1000);
					
					//merge if yes
					if(MessageBox(hMainWnd,szText,"Subsitution",MB_YESNO | MB_ICONQUESTION)==IDYES)
					{
						SetUndo();
						message.MergeSymbols(simp1,simp2,true);
						SetCipher();
						SetDlgInfo();
					}
					return 0;

				case IDM_CIPHER_POLYIC:
					DialogBox(hInst,MAKEINTRESOURCE(IDD_NUMBER),hMainWnd,(DLGPROC)PolyProc);
					return 0;

				case IDM_CIPHER_RC_IOC:
					lRowCol=message.RowColIoC(szGraph,iLineChars);
					strcpy(szGraphTitle,"Row & Column IoC Count");
					DialogBox(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
					return 0;
					
				case IDM_CIPHER_NGRAPHS:
					sprintf(szText,"%s%s",szExeDir,"bigraphs.txt");
					message.PatternsToFile(szText,2);
					sprintf(szText,"%s%s",szExeDir,"trigraphs.txt");
					message.PatternsToFile(szText,3);
					sprintf(szText,"%s%s",szExeDir,"tetragraphs.txt");
					message.PatternsToFile(szText,4);
					sprintf(szText,"%s%s",szExeDir,"pentagraphs.txt");
					message.PatternsToFile(szText,5);
					return 0;

				/*key menu*/
				case IDM_KEY_INIT:
					SetUndo();
					DialogBox(hInst,MAKEINTRESOURCE(IDD_NUMBER),hMainWnd,(DLGPROC)InitProc);
					return 0;

				case IDM_KEY_SCRAMBLE:
					SetUndo();
					num_symbols=message.cur_map.GetNumSymbols();
					for(swap=0; swap<300000; swap++)
						message.cur_map.SwapSymbols(rand()%num_symbols,rand()%num_symbols);
					SetDlgInfo();
					return 0;

				case IDM_KEY_CLEAR:
					SetUndo();
					message.cur_map.Clear(CLR_PLAIN);
					SetDlgInfo();
					return 0;

				case IDM_KEY_LOCK: message.cur_map.SetLock(iCurSymbol,true); SetKey(); return 0;
				case IDM_KEY_UNLOCK: message.cur_map.SetLock(iCurSymbol,false); SetKey(); return 0;
				case IDM_KEY_LOCK_ALL: message.cur_map.SetAllLock(true); SetKey(); return 0;
				case IDM_KEY_UNLOCK_ALL: message.cur_map.SetAllLock(false); SetKey(); return 0;

				/*solve menu*/
				case IDM_SOLVE_WORD:
					DialogBox(hInst,MAKEINTRESOURCE(IDD_WORD),hMainWnd,(DLGPROC)WordProc);
					return 0;

				case IDM_SOLVE_OPTIONS:
					DialogBox(hInst,MAKEINTRESOURCE(IDD_OPTIONS),hMainWnd,(DLGPROC)OptionsProc);
					return 0;

				case IDM_SOLVE_TP_IDLE: SetPriority(4); return 0;
				case IDM_SOLVE_TP_HIGH: SetPriority(3); return 0;	
				case IDM_SOLVE_TP_NORM: SetPriority(2); return 0;	
				case IDM_SOLVE_TP_LOW: SetPriority(1); return 0;										


				/*view menu*/
				case IDM_VIEW_DESELECT:
					iCurPat=-1;
					iCurSymbol=-1;
					iTextSel=-1;
					SendDlgItemMessage(hMainWnd,IDC_PATTERNS,LB_SETCURSEL,iCurPat,0);
					SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,iCurSymbol,0);
					SetText();
					SetDlgItemText(hTextWnd,IDC_TEXTINFO,"");
					return 0;

				case IDM_VIEW_SYMGRAPH:
					lRowCol=message.cur_map.SymbolGraph(szGraph);
					strcpy(szGraphTitle,"Symbol Frequencies");
					DialogBox(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
					return 0;
					
				case IDM_VIEW_LTRGRAPH:
					lRowCol=message.LetterGraph(szGraph);
					strcpy(szGraphTitle,"Letter Frequencies");
					hGraph=CreateDialog(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
					ShowWindow(hGraph,SW_SHOWNORMAL);
					return 0;

				case IDM_VIEW_BYSTRING: SetSort(0); return 0;
				case IDM_VIEW_BYFREQ: SetSort(1); return 0;

				/*help menu*/
				case IDM_HELP_ABOUT:
					DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),hMainWnd,(DLGPROC)AboutProc);
					return 0;

				case IDM_HELP_CONT:
					sprintf(szText,"%s%s\\%s",szExeDir,"help","index.html");
					OpenWith(szText);
					return 0;

				/*Controls*/
				case IDC_MAP_CHANGE: ChangePlain(); return 0;

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
							UpdateSymbol(iCurSymbol);
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

		case WM_NOTIFY:
			if(LPNMHDR(lParam)->hwndFrom==hMainTab) //Control ID
				switch(LPNMHDR(lParam)->code) //Notify Code
				{
					case NM_CLICK:
						ShowTab(SendMessage(hMainTab,TCM_GETCURSEL,0,0));
						return 0;
				}
			return 0;
		
		//drag & drop loading of files
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpszCmdLine, int nCmdShow)
{
	int iTextFlags=WS_CHILD | SS_NOPREFIX | SS_LEFTNOWORDWRAP | SS_SUNKEN;
	TCITEM tciTabItem;
	RECT rMainRect;
	MSG Msg;
	
	hInst=hInstance;
	
	srand(time(0));

	/*Initialize common controls*/
    /*INITCOMMONCONTROLSEX icInitCC;
    icInitCC.dwSize=sizeof(INITCOMMONCONTROLSEX);
    icInitCC.dwICC=ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icInitCC);

	if(!LoadLibrary(TEXT("riched32.dll")))
		MessageBox(NULL,"Could not load riched32.dll","Error",MB_ICONEXCLAMATION);*/

	/*create main window*/
	hMainWnd=CreateDialog(hInst,MAKEINTRESOURCE(IDD_MAIN),NULL,(DLGPROC)MainWndProc);
	SendMessage(hMainWnd,WM_SETICON,ICON_BIG,(WPARAM)LoadIcon(hInst,MAKEINTRESOURCE(IDI_ZODIAC)));
	hMainMenu=GetMenu(hMainWnd);
	hAccel=LoadAccelerators(hInst,MAKEINTRESOURCE(IDR_ACCEL));

	/*setup tabs*/
	hMainTab=GetDlgItem(hMainWnd,IDC_MAIN_TAB);

	tciTabItem.mask=TCIF_TEXT;
	tciTabItem.cchTextMax=32;

	tciTabItem.pszText="Solve";
	SendMessage(hMainTab,TCM_INSERTITEM,0,(LPARAM)&tciTabItem);
	tciTabItem.pszText="Analysis";
	SendMessage(hMainTab,TCM_INSERTITEM,1,(LPARAM)&tciTabItem);
	ShowTab(0);

	/*create text window*/
	hTextWnd=CreateDialog(hInst,MAKEINTRESOURCE(IDD_TEXT),NULL,(DLGPROC)TextWndProc);
	SendMessage(hTextWnd,WM_SETICON,ICON_BIG,(WPARAM)LoadIcon(hInst,MAKEINTRESOURCE(IDI_ZODIAC)));

	/*cipher/plain text controls*/
	hScroll=GetDlgItem(hTextWnd,IDC_SCROLL);
	hCipher=CreateWindow("STATIC","",iTextFlags,iCipherX,iCipherY,iTextWidth,iTextHeight,hTextWnd,NULL,hInst,NULL);
	hPlain=CreateWindow("STATIC","",iTextFlags,iPlainX,iPlainY,iTextWidth,iTextHeight,hTextWnd,NULL,hInst,NULL);
	hCipherDC=GetWindowDC(hCipher);
	hPlainDC=GetWindowDC(hPlain);

	/*drawing*/
	hRedPen=CreatePen(0,0,RGB(255,0,0));
	hGreenPen=CreatePen(0,0,RGB(0,196,0));
	hBluePen=CreatePen(0,0,RGB(0,0,255));
	SelectObject(hCipherDC,(HBRUSH)GetStockObject(NULL_BRUSH));
	SelectObject(hPlainDC,(HBRUSH)GetStockObject(NULL_BRUSH));
	SetCharSize();

	/*setup window*/
	SetTitle();
	MsgEnable(false);
	MapEnable(false);
	EnableMenuItem(hMainMenu,IDM_FILE_OPEN_MSG,MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMainMenu,IDM_FILE_COPY_PLAIN,MF_BYCOMMAND | MF_GRAYED);
	Button_Enable(GetDlgItem(hMainWnd,IDC_SOLVE),false);
	SendDlgItemMessage(hMainWnd,IDC_MAP_VALUE,EM_LIMITTEXT,1,0);
	SetScrollBar();
	SetPriority(1);
	StopSolve();
	SetSort(0);
	
	/*init solve info*/
	memset(&siSolveInfo,0,sizeof(SOLVEINFO));
	siSolveInfo.max_fail=100;
	siSolveInfo.swaps=5;
	siSolveInfo.revert=120*5-1;
	siSolveInfo.disp_all=disp_all;
	siSolveInfo.disp_info=disp_info;
	siSolveInfo.time_func=GetTime;
	Reset();

	/*setup directories*/
	//executable's directory
	GetModuleFileName(hInst,szExeDir,1024);
	char *szEndDir;
	szEndDir=strrchr(szExeDir,'\\');
	if(szEndDir) *(szEndDir+1)='\0';
	
	//set open/save dir as executable dir
	strcpy(szCipherName,szExeDir);
	strcpy(szKeyName,szExeDir);
	
	//language
	iLang=0;
	SetLanguage();

	//show the windows
	GetWindowRect(hMainWnd,&rMainRect);
	SetWindowPos(hTextWnd,TEXT_POS,rMainRect.right+10,rMainRect.left,400,rMainRect.bottom-rMainRect.top,0);
	ShowWindow(hMainWnd,SW_SHOWNORMAL);
	ShowWindow(hTextWnd,SW_SHOWNORMAL);
	ShowWindow(hCipher,SW_SHOWNORMAL);
	ShowWindow(hPlain,SW_SHOWNORMAL);
	
	//Open From Command Line
	if(lpszCmdLine[0]!='\0') 
	{
		strcpy(szCipherName,lpszCmdLine); 
		message.Read(szCipherName);
	}

	LoadINI();

	//message loop
	while(GetMessage(&Msg,NULL,0,0))
		if(!TranslateAccelerator(hMainWnd,hAccel,&Msg))
			if(!IsDialogMessage(hMainWnd,&Msg))
			{	
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}

	SaveINI();

	return(Msg.wParam);
}
