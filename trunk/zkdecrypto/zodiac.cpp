#include <stdio.h>
#include <time.h>
#include <math.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "headers/resource.h"
#include "headers/zodiac.h"
#include "headers/display.h"
#include "headers/solve.h"
#include "headers/files.h"
#include "headers/dlgprocs.h"
#include "headers/command.h"

////////////////////////////////////////////////////////////////////////////////
//          Callback Message Handler for Ciphertext/Plaintext Window          //
////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK TextWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int iWidth, iHeight;
	MINMAXINFO *mmiInfo;

	switch(iMsg)
	{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_TS_UP: iCharSize+=float(0.1); SetCharSize(); return 0;
				case IDC_TS_DOWN: iCharSize-=float(0.1); SetCharSize(); return 0;

				case IDC_RS_UP: SetLineChars(iLineChars+1); return 0;
				case IDC_RS_DOWN: SetLineChars(iLineChars-1); return 0;

				case IDC_LINE_CHARS: if(HIWORD(wParam)==EN_CHANGE) SetLineChars(GetDlgItemInt(hWnd,IDC_LINE_CHARS,false,true),false); return 0;
			}

		case WM_VSCROLL: //scroll text
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
			
			ClearTextAreas();
			SetText();
			return 0;

		case WM_SIZE:
			iWidth=LOWORD(lParam);
			iHeight=HIWORD(lParam);
			ResizeText(iWidth,iHeight);
			return 0;

		case WM_GETMINMAXINFO:
			mmiInfo=(MINMAXINFO*)lParam;
			mmiInfo->ptMinTrackSize.x=400;
			mmiInfo->ptMinTrackSize.y=300;
			return 0;

		case WM_CLOSE: StopSolve(); EndDialog(hMainWnd,0); PostMessage(hMainWnd,WM_DESTROY,0,0); return 0;
		case WM_DESTROY: StopSolve(); PostQuitMessage(0); return 0;
	}

	return 0;
}
////////////////////////////////////////////////////////////////////////////////
//               Callback Message Handler for Main Program Window             //
////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	POINT ptClick;
	HWND hList;

	switch(iMsg)
	{
		case WM_COMMAND:
			CommandFile(LOWORD(wParam));
			CommandEdit(LOWORD(wParam));
			CommandCipher(LOWORD(wParam));
			CommandKey(LOWORD(wParam));
			CommandSolve(LOWORD(wParam));
			CommandView(LOWORD(wParam));

			switch(LOWORD(wParam))
			{
				case IDM_HELP_ABOUT:
					DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),hMainWnd,(DLGPROC)AboutProc);
					return 0;

				case IDM_HELP_CONT:
					sprintf(szText,"%s%s\\%s",szExeDir,"help","index.html");
					OpenWith(szText);
					return 0;

				/*Controls*/
				case IDC_MAP_CHANGE: ChangePlain(); return 0;
				case IDC_MAP_VALUE:
					switch(HIWORD(wParam))
					{
						case EN_CHANGE: ChangePlain(); break;
					}
					return 0;

				case IDC_SOLVE:
					SetUndo();
					if(!bMsgLoaded) return 0;

					if(siSolveInfo.running) 
					{
						if(iBruteSymbols>0) iBruteSymbols=-1; //cancel brute force
						StopSolve();
					}
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
						case LBN_SELCHANGE: UpdateSelectedSymbol(); break;\
						case LBN_DBLCLK: ToggleLock(); break;
					}
					return 0;
					
				case IDC_WORD_LIST:
					switch(HIWORD(wParam))
					{
						case LBN_SELCHANGE: 
							iCurWord=SendDlgItemMessage(hMainWnd,IDC_WORD_LIST,LB_GETCURSEL,0,0); 
							SetText(); 
							break;
					}
					return 0;

				case IDC_IOC_WEIGHT_EDIT:
					if(HIWORD(wParam)!=EN_CHANGE) return 0;
					siSolveInfo.ioc_weight=GetDlgItemInt(hMainWnd,IDC_IOC_WEIGHT_EDIT,false,false);
					SetPlain();
					return 0;

				case IDC_ENT_WEIGHT_EDIT:
					if(HIWORD(wParam)!=EN_CHANGE) return 0;
					siSolveInfo.ent_weight=GetDlgItemInt(hMainWnd,IDC_ENT_WEIGHT_EDIT,false,false);
					SetPlain();
					return 0;

				case IDC_CHI_WEIGHT_EDIT: 
					if(HIWORD(wParam)!=EN_CHANGE) return 0;
					siSolveInfo.chi_weight=GetDlgItemInt(hMainWnd,IDC_CHI_WEIGHT_EDIT,false,false);
					SetPlain();
					return 0;

				case IDC_DIOC_WEIGHT_EDIT:
					if(HIWORD(wParam)!=EN_CHANGE) return 0;
					siSolveInfo.dioc_weight=GetDlgItemInt(hMainWnd,IDC_DIOC_WEIGHT_EDIT,false,false);
					SetPlain();
					return 0;

				case IDC_BLOCK_EDIT:
					if(HIWORD(wParam)!=EN_CHANGE) return 0;
					if(!bMsgLoaded) return 0;
					if(siSolveInfo.running) return 0;
					iBlockSize=GetDlgItemInt(hMainWnd,IDC_BLOCK_EDIT,false,false);
					siSolveInfo.best_block=iBlockSize;
					message.SetBlockSize(iBlockSize); message.Decode(); 
					SetPlain(); SetText();
					return 0;

				case IDC_KEY_EDIT:
					if(HIWORD(wParam)!=EN_CHANGE) return 0;
					if(!bMsgLoaded) return 0;
					GetKeyEdit();
					return 0;
					
				case IDC_WORD_MIN:
					if(HIWORD(wParam)!=EN_CHANGE) return 0;
					iWordMin=GetDlgItemInt(hMainWnd,IDC_WORD_MIN,false,false);
					if(iWordMin<0 || iWordMin>30) iWordMin=0;
					SetWordListTabInfo();
                    return 0;
                     
				case IDC_WORD_MAX:
					if(HIWORD(wParam)!=EN_CHANGE) return 0;
					iWordMax=GetDlgItemInt(hMainWnd,IDC_WORD_MAX,false,false);
					SetWordListTabInfo();
					if(iWordMax<0 || iWordMax>30) iWordMax=20;
                    return 0;
                     
                case UDM_DISPALL: SetDlgInfo(); SetWordList(); return 0;
				case UDM_DISPINFO: SetSolve(); return 0;
			}

			return 0;

		case WM_CONTEXTMENU:
			//rect of key window in screen coordinates
			GetWindowRect(GetDlgItem(hMainWnd,IDC_PATTERNS),&rPatRect);
			GetWindowRect(GetDlgItem(hMainWnd,IDC_MAP),&rKeyRect);
			GetWindowRect(GetDlgItem(hMainWnd,IDC_WORD_LIST),&rWordRect);
			
			//click point in screen coordinates
			ptClick.x=LOWORD(lParam);
			ptClick.y=HIWORD(lParam);

			hList=NULL;

			if(iCurTab==0 && IN_RECT(ptClick.x,ptClick.y,rPatRect))
			{
				memcpy(&rListRect,&rPatRect,sizeof(RECT));
				hList=hPat;
			}

			if(iCurTab==0 && IN_RECT(ptClick.x,ptClick.y,rKeyRect))
			{
				memcpy(&rListRect,&rKeyRect,sizeof(RECT));
				hList=hKey;
			}
			
			if(iCurTab==2 && IN_RECT(ptClick.x,ptClick.y,rWordRect)) 
			{
				memcpy(&rListRect,&rWordRect,sizeof(RECT));
				hList=hWord;
			}

			//convert click to key window coordinates
			lParam=(ptClick.x-rListRect.left) | (ptClick.y-rListRect.top)<<16;
				
			//send click message
			if(hList)
			{
				SendMessage(hList,WM_LBUTTONDOWN,0,lParam);
				SendMessage(hList,WM_LBUTTONUP,0,lParam);
				
				CreateTextMenu();
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
			
		case WM_CLOSE: StopSolve(); EndDialog(hMainWnd,0); PostMessage(hMainWnd,WM_DESTROY,0,0); return 0;
		case WM_DESTROY: StopSolve(); PostQuitMessage(0); return 0;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//                         WinMain for Entire Program                         //
////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpszCmdLine, int nCmdShow)
{
	int iTextFlags=WS_CHILD;
	TCITEM tciTabItem;
	RECT rMainRect;
	MSG Msg;
	char filename[1024];
	
	hInst=hInstance;

	srand(time(0));

	/*setup directories*/
	//executable's directory
	GetModuleFileName(hInst,szExeDir,1024);
	char *szEndDir;
	szEndDir=strrchr(szExeDir,'\\');
	if(szEndDir) *(szEndDir+1)='\0';
	
	//set open/save dir as their own
	sprintf(filename,"%s",szExeDir);
	strcat(filename,"cipher");
	strcpy(szCipherName,filename);
	sprintf(filename,"%s",szExeDir);
	strcat(filename,"key");
	strcpy(szKeyName,filename);
	strcpy(szPlainName,szExeDir);

	if(!LoadFONT())
	{
		MessageBox(hMainWnd, "Critical program font cannot be loaded!", "ZKDecrypto Loading Error", MB_ICONEXCLAMATION);
		exit(1);
	}

	/*create main window*/
	hMainWnd=CreateDialog(hInst,MAKEINTRESOURCE(IDD_MAIN),NULL,(DLGPROC)MainWndProc);
	SendMessage(hMainWnd,WM_SETICON,ICON_BIG,(WPARAM)LoadIcon(hInst,MAKEINTRESOURCE(IDI_ZODIAC)));
	hMainMenu=GetMenu(hMainWnd);
	hAccel=LoadAccelerators(hInst,MAKEINTRESOURCE(IDR_ACCEL));
	hPat=GetDlgItem(hMainWnd,IDC_PATTERNS);
	hKey=GetDlgItem(hMainWnd,IDC_MAP);
	hWord=GetDlgItem(hMainWnd,IDC_WORD_LIST);

	/*setup tabs*/
	hMainTab=GetDlgItem(hMainWnd,IDC_MAIN_TAB);

	tciTabItem.mask=TCIF_TEXT;
	tciTabItem.cchTextMax=15;

	tciTabItem.pszText="Solver";
	SendMessage(hMainTab,TCM_INSERTITEM,0,(LPARAM)&tciTabItem);
	tciTabItem.pszText="Freqs";
	SendMessage(hMainTab,TCM_INSERTITEM,1,(LPARAM)&tciTabItem);
	tciTabItem.pszText="Words";
	SendMessage(hMainTab,TCM_INSERTITEM,2,(LPARAM)&tciTabItem);
	tciTabItem.pszText="Stats";
	SendMessage(hMainTab,TCM_INSERTITEM,3,(LPARAM)&tciTabItem);
	tciTabItem.pszText="Contacts";
	SendMessage(hMainTab,TCM_INSERTITEM,4,(LPARAM)&tciTabItem);
	ShowTab(0);

	/*create text window*/
	hTextWnd=CreateDialog(hInst,MAKEINTRESOURCE(IDD_TEXT),NULL,(DLGPROC)TextWndProc);
	SendMessage(hTextWnd,WM_SETICON,ICON_BIG,(WPARAM)LoadIcon(hInst,MAKEINTRESOURCE(IDI_ZODIAC)));

	/*cipher/plain text controls*/
	WNDCLASSEX wcTextClass;
	memset(&wcTextClass,0,sizeof(WNDCLASSEX));
	wcTextClass.style = CS_HREDRAW | CS_VREDRAW;
	wcTextClass.cbSize = sizeof(WNDCLASSEX);
	wcTextClass.lpfnWndProc = TextProc;
	wcTextClass.hInstance = hInst;
	wcTextClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcTextClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcTextClass.lpszClassName = "ZKDTEXT";
	RegisterClassEx(&wcTextClass);

	hScroll=GetDlgItem(hTextWnd,IDC_SCROLL);
	hCipher=CreateWindowEx(WS_EX_CLIENTEDGE,"ZKDTEXT","",iTextFlags,iCipherX,iCipherY,iTextWidth,iTextHeight,hTextWnd,NULL,hInst,NULL);
	hPlain=CreateWindowEx(WS_EX_CLIENTEDGE,"ZKDTEXT","",iTextFlags,iPlainX,iPlainY,iTextWidth,iTextHeight,hTextWnd,NULL,hInst,NULL);
	hCipherDC=GetWindowDC(hCipher);
	hPlainDC=GetWindowDC(hPlain);

	/*drawing*/
	crRed=RGB(255,0,0);
	crGreen=RGB(0,196,0);
	crBlue=RGB(0,0,255);
	crOrange=RGB(196,0,196);
	crYellow=RGB(255,255,0);
	crBlack=RGB(0,0,0);
	crWhite=RGB(255,255,255);
	hRedPen=CreatePen(0,0,crRed);
	hGreenPen=CreatePen(0,0,crGreen);
	hBluePen=CreatePen(0,0,crBlue);
	hOrangePen=CreatePen(0,0,crOrange);
	hWhitePen=CreatePen(0,0,crWhite);
	hWhiteBrush=(HBRUSH)GetStockObject(WHITE_BRUSH);
	SelectObject(hCipherDC,(HBRUSH)GetStockObject(NULL_BRUSH));
	SelectObject(hPlainDC,(HBRUSH)GetStockObject(NULL_BRUSH));
	SetCharSize();

	/*setup window*/
	SetTitle();
	MsgEnable(false);
	MapEnable(false);
	SendDlgItemMessage(hMainWnd,IDC_IOC_WEIGHT_SPIN,UDM_SETRANGE,0,10);
	SendDlgItemMessage(hMainWnd,IDC_ENT_WEIGHT_SPIN,UDM_SETRANGE,0,10);
	SendDlgItemMessage(hMainWnd,IDC_CHI_WEIGHT_SPIN,UDM_SETRANGE,0,10);
	SendDlgItemMessage(hMainWnd,IDC_DIOC_WEIGHT_SPIN,UDM_SETRANGE,0,10);
	SendDlgItemMessage(hMainWnd,IDC_BLOCK_SPIN,UDM_SETRANGE,1,1);
	SetDlgItemInt(hMainWnd,IDC_BLOCK_EDIT,1,false);
	EnableMenuItem(hMainMenu,IDM_FILE_OPEN_ASC,MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMainMenu,IDM_FILE_OPEN_NUM,MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMainMenu,IDM_FILE_SAVE_CIPHER,MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMainMenu,IDM_FILE_COPY_PLAIN,MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMainMenu,IDM_SOLVE_COPY_BEST,MF_BYCOMMAND | MF_GRAYED);
	Button_Enable(GetDlgItem(hMainWnd,IDC_SOLVE),false);
	SetScrollBar();
	SetPriority(1);
	StopSolve();
	SetSort(0);
	
	/*init solve info*/
	memset(&siSolveInfo,0,sizeof(SOLVEINFO));
	siSolveInfo.ioc_weight=siSolveInfo.ent_weight=siSolveInfo.chi_weight=5;
	siSolveInfo.dioc_weight=0;
	siSolveInfo.max_tabu=300;
	siSolveInfo.swaps=5;
	siSolveInfo.max_tol=40;
	siSolveInfo.disp_all=disp_all;
	siSolveInfo.disp_info=disp_info;
	siSolveInfo.time_func=GetTime;
	siSolveInfo.get_words=GetWordList;
	sprintf(siSolveInfo.log_name,"%s\\%s",szExeDir,"log.txt");
	siSolveInfo.dictionary=&dictionary;
	siSolveInfo.optima_tabu=&tabu_list;
	SetInfo(&siSolveInfo);
	Reset();

	//language
	iLang=0;
	LoadCribs();
	LoadINI();
	SetLanguage();

	if(DIGRAPH_MODE) SendDlgItemMessage(hMainWnd,IDC_MAP_VALUE,EM_LIMITTEXT,2,0);
	else SendDlgItemMessage(hMainWnd,IDC_MAP_VALUE,EM_LIMITTEXT,1,0);

	//sovle parameters
	message.SetBlockSize(1);

	//show the windows
	GetWindowRect(hMainWnd,&rMainRect);
	SetWindowPos(hTextWnd,TEXT_POS,rMainRect.right+10,rMainRect.left,550,rMainRect.bottom-rMainRect.top,0);
	ShowWindow(hMainWnd,SW_SHOWNORMAL);
	ShowWindow(hTextWnd,SW_SHOWNORMAL);
	ShowWindow(hCipher,SW_SHOWNORMAL);
	ShowWindow(hPlain,SW_SHOWNORMAL);
	hKeyEdit=GetDlgItem(hMainWnd,IDC_KEY_EDIT);
	lKeyEditStyle=GetWindowLong(hKeyEdit,GWL_STYLE);

	//create main status bar
	hMainStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hMainWnd, (HMENU)IDC_TEXT_STATUS, GetModuleHandle(NULL), NULL);
 
    int MainStatWidths[] = {300, -1};
    SendMessage(hMainStatus, SB_SETPARTS, sizeof(MainStatWidths)/sizeof(int), (LPARAM)MainStatWidths);
	SendMessage(hMainStatus, SB_SETTEXT, 0, (LPARAM)"KEY LENGTH: ");

	//create text status bar with gripper
	hTextStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hTextWnd, (HMENU)IDC_TEXT_STATUS, GetModuleHandle(NULL), NULL);
 
    int TextStatWidths[] = {125, 225, 325, 425, -1}; //those old widths were too small, text was being cut off
    SendMessage(hTextStatus, SB_SETPARTS, sizeof(TextStatWidths)/sizeof(int), (LPARAM)TextStatWidths);
	SendMessage(hTextStatus, SB_SETTEXT, 0, (LPARAM)"LANG: ");

	SetSolveTypeFeatures();
	
////////////////////////////////////////////////////////////////////////////////
//                   Primary Message Loop for Entire Program                  //
////////////////////////////////////////////////////////////////////////////////

	while(GetMessage(&Msg,NULL,0,0))
		if(!TranslateAccelerator(hMainWnd,hAccel,&Msg))
			if(!IsDialogMessage(hMainWnd,&Msg))
			{	
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}

	SaveINI();
	RemoveFONT();

	return(Msg.wParam);
}
