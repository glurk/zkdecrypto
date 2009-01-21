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

//message handler for text window
LRESULT CALLBACK TextWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int iWidth, iHeight;
	MINMAXINFO *mmiInfo;

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
						case LBN_SELCHANGE:
							UpdateSelectedSymbol();
							break;

						case LBN_DBLCLK: ToggleLock(); break;
					}
					return 0;
					
				case IDC_WORD_LIST:
					switch(HIWORD(wParam))
					{
						case LBN_SELCHANGE:
							iCurWord=SendDlgItemMessage(hMainWnd,IDC_WORD_LIST,LB_GETCURSEL,0,0);
							SetText();
					}
					return 0;

				case IDC_IOC_WEIGHT_EDIT: 
					iIoCWeight=GetDlgItemInt(hMainWnd,IDC_IOC_WEIGHT_EDIT,false,false);
					SetIoCWeight(iIoCWeight);
					return 0;
					
				case IDC_WORD_MIN:
					iWordMin=GetDlgItemInt(hMainWnd,IDC_WORD_MIN,false,false);
					SetDlgInfo();
                    return 0;
                     
				case IDC_WORD_MAX:
					iWordMax=GetDlgItemInt(hMainWnd,IDC_WORD_MAX,false,false);
					SetDlgInfo();
                    return 0;
                     
                case UDM_DISPALL:
					SetDlgInfo();
					return 0;
					
				case UDM_DISPINFO:
					SetSolve();
					return 0;
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
	int iTextFlags=WS_CHILD;
	TCITEM tciTabItem;
	RECT rMainRect;
	MSG Msg;
	char filename[1024];
	
	hInst=hInstance;
	HWND hStatus;
	
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
	tciTabItem.cchTextMax=32;

	tciTabItem.pszText="Solve";
	SendMessage(hMainTab,TCM_INSERTITEM,0,(LPARAM)&tciTabItem);
	tciTabItem.pszText="Analysis";
	SendMessage(hMainTab,TCM_INSERTITEM,1,(LPARAM)&tciTabItem);
	tciTabItem.pszText="Word List";
	SendMessage(hMainTab,TCM_INSERTITEM,2,(LPARAM)&tciTabItem);
	tciTabItem.pszText="Statistics";
	SendMessage(hMainTab,TCM_INSERTITEM,3,(LPARAM)&tciTabItem);
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
	SetDlgItemInt(hMainWnd,IDC_IOC_WEIGHT_EDIT,5,false);
	EnableMenuItem(hMainMenu,IDM_FILE_OPEN_ASC,MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMainMenu,IDM_FILE_OPEN_NUM,MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMainMenu,IDM_FILE_SAVE_CIPHER,MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMainMenu,IDM_FILE_COPY_PLAIN,MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(hMainMenu,IDM_SOLVE_COPY_BEST,MF_BYCOMMAND | MF_GRAYED);
	Button_Enable(GetDlgItem(hMainWnd,IDC_SOLVE),false);
	SendDlgItemMessage(hMainWnd,IDC_MAP_VALUE,EM_LIMITTEXT,1,0);
	SetScrollBar();
	SetPriority(1);
	StopSolve();
	SetSort(0);
	
	/*init solve info*/
	memset(&siSolveInfo,0,sizeof(SOLVEINFO));
	siSolveInfo.max_fail=2000;
	siSolveInfo.swaps=5;
	siSolveInfo.revert=400;
	siSolveInfo.disp_all=disp_all;
	siSolveInfo.disp_info=disp_info;
	siSolveInfo.time_func=GetTime;
	Reset();

	//language
	iLang=0;
	LoadINI();
	SetLanguage();

	//show the windows
	GetWindowRect(hMainWnd,&rMainRect);
	SetWindowPos(hTextWnd,TEXT_POS,rMainRect.right+10,rMainRect.left,400,rMainRect.bottom-rMainRect.top,0);
	ShowWindow(hMainWnd,SW_SHOWNORMAL);
	ShowWindow(hTextWnd,SW_SHOWNORMAL);
	ShowWindow(hCipher,SW_SHOWNORMAL);
	ShowWindow(hPlain,SW_SHOWNORMAL);

	//create status bar with gripper
	hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
        hTextWnd, (HMENU)IDC_TEXT_STATUS, GetModuleHandle(NULL), NULL);
 
    //setup status bar sections
    int statwidths[] = {80, 140, 200, -1};
    SendMessage(hStatus, SB_SETPARTS, sizeof(statwidths)/sizeof(int), (LPARAM)statwidths);
	SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"LANG: ");


	//message loop
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
