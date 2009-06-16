#ifndef _ZODIAC_H

#include "map.h"
#include "message.h"
#include "z340.h"
#include <map>
#include <string>

//program
#define PROG_NAME	"ZKDecrypto"
#define PROG_VER	"Version 1.2"

//language
#define LANG_DIR	"language"
#define LANG_ENG	"eng"
#define LANG_SPA	"spa"
#define LANG_GER	"ger"
#define LANG_ITA	"ita"
#define LANG_FRE	"fre"

//language IoC
#define IOC_ENG		.0665
#define IOC_SPA		.0746
#define IOC_GER		.0788
#define IOC_ITA		.0746
#define IOC_FRE		.0777
#define IOC_POR		.0746
#define IOC_RUS		.0677
#define IOC_RAND	.0385

#define DIOC		.0075
#define CHI			.55
#define ENT			4.1

//text constants
#define CHAR_WIDTH  7
#define CHAR_HEIGHT 12
#define TEXT_POS	0 //HWND_TOPMOST

#define MAX_EXTRA	104

//macros
#define IN_RECT(X,Y,R)	(IS_BETWEEN(X,R.left,R.right) && IS_BETWEEN(Y,R.top,R.bottom))
#define DIGRAPH_MODE	((iSolveType==SOLVE_DISUB || iSolveType==SOLVE_PLAYFAIR || iSolveType==SOLVE_DBLPLAY)? 1:0) //digraph key and display
#define DEFRACTION_TYPE (iSolveType==SOLVE_ADFGX || iSolveType==SOLVE_ADFGVX || iSolveType==SOLVE_CEMOPRTU) //plain text half as long as cipher
#define ASCIPHER_TYPE	(DEFRACTION_TYPE) //set key as cipher characters
#define TRANSPOSE_TYPE	(iSolveType==SOLVE_PERMUTE || iSolveType==SOLVE_COLTRANS || iSolveType==SOLVE_DOUBLE || iSolveType==SOLVE_TRIPPLE)
#define ALLOW_LOWERCASE (TRANSPOSE_TYPE || iSolveType==SOLVE_CEMOPRTU || iSolveType==SOLVE_SUBPERM || iSolveType==SOLVE_COLVIG)

//cipher/key data & files
Message message; //cipher & main key
Message undo_message, redo_message; //undo/redo messages
int undo_line_size, redo_line_size;
char szCipherName[1024], szKeyName[1024], szPlainName[1024], szGraphName[1024]; //filenames
char *szCipherBase, *szKeyBase; //file basenames
char szLanguage[32];
int bMsgLoaded=false, bMapLoaded=false, bUndo=false;
int iNumber, iCurTab;
char szString[128], szStringTitle[128], szNumberTitle[128]; //word, exclude string
typedef std::map<std::string,int> STRMAP;
STRMAP dictionary, tabu_list, word_list;

//GUI data
char szTitle[64], szText[40960], szExeDir[1024], szOldKey[4096]=""; 
int iCurSymbol=-1, iCurPat=-1, iCurWord=-1, iTextSel=-1, iRowSel=-1, iColSel=-1; //selections
int iCharWidth=CHAR_WIDTH, iCharHeight=CHAR_HEIGHT; //font size
int iSortBy=0, iWordMin=4, iWordMax=20;
RECT rPatRect, rKeyRect, rWordRect, rListRect;
int lprgiInitKey[26];
int lprgiInitID[26]= //homophonic init key
{
IDC_INIT_A_EDIT,IDC_INIT_B_EDIT,IDC_INIT_C_EDIT,IDC_INIT_D_EDIT,
IDC_INIT_E_EDIT,IDC_INIT_F_EDIT,IDC_INIT_G_EDIT,IDC_INIT_H_EDIT,
IDC_INIT_I_EDIT,IDC_INIT_J_EDIT,IDC_INIT_K_EDIT,IDC_INIT_L_EDIT,
IDC_INIT_M_EDIT,IDC_INIT_N_EDIT,IDC_INIT_O_EDIT,IDC_INIT_P_EDIT,
IDC_INIT_Q_EDIT,IDC_INIT_R_EDIT,IDC_INIT_S_EDIT,IDC_INIT_T_EDIT,
IDC_INIT_U_EDIT,IDC_INIT_V_EDIT,IDC_INIT_W_EDIT,IDC_INIT_X_EDIT,
IDC_INIT_Y_EDIT,IDC_INIT_Z_EDIT
};

//text gui
COLORREF crRed, crGreen, crBlue, crOrange, crYellow, crBlack, crWhite;
float iCharSize=1.5; //font size multiplier
int iLineChars=17, iLines, iDispLines; //text line data
int iScrollPos, iMaxScroll; //scrollbar
int iDispStart, iDispEnd; //index of the start/end characters being displayed
int iMargin=10; //window margin size
int iCipherX=3*iMargin, iCipherY=3*iMargin; //cipher wnd position
int iPlainX, iPlainY=3*iMargin; //plain wnd position
int iTextWidth=100, iTextHeight=100; //dimensions of cipher and plain
POINT pntClickPoint; //click point
int iTextBorder=4;

//graphs
wchar szGraph[40960];//[20480];
char szGraphTitle[128];
long lRowCol;

//solver data
SOLVEINFO siSolveInfo;
int iPriority, iLang;
char szExtraLtr[MAX_EXTRA+1]="";
int iBruteSymbols, iBatchBestScore;
char lprgcBatchBestKey[4096];
int iSolveType=0, iBlockSize=0;

//Win32 object handles
HWND		hMainWnd, hPat, hKey, hWord, hMainTab, hTextWnd, hHomoWnd=NULL, hWordWnd=NULL, hCipher=NULL, hPlain=NULL, hLetter=NULL, hScroll;
HACCEL		hAccel;
HPEN 		hRedPen, hGreenPen, hBluePen, hOrangePen, hWhitePen;
HBRUSH		hWhiteBrush;
HDC 		hCipherDC=NULL, hPlainDC=NULL;
HFONT		hTextFont=NULL, hTempFont=NULL;
HMENU		hMainMenu, hTextMenu;
HINSTANCE	hInst;
HANDLE		hSolveThread=NULL, hTimerThread=NULL;
HWND		hMainStatus, hTextStatus;

HWND hKeyEdit;
DWORD lKeyEditStyle;

//open/save file filter
char szFileFilter[]= 
	{"Text Files (*.txt)\0" "*.txt;\0"
	 "All Files (*.*)\0" "*.*\0\0"};
	 
//callback functions for hillclimber
inline void disp_all()  {SendMessage(hMainWnd,WM_COMMAND,UDM_DISPALL,0);}
inline void disp_info() {SendMessage(hMainWnd,WM_COMMAND,UDM_DISPINFO,0);}
inline DWORD GetTime()	{return GetTickCount();}

#endif

