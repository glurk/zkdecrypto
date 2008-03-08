#ifndef _ZODIAC_H

#include "message.h"
#include "z340.h"
#include <map>
#include <string>

//program
#define PROG_NAME	"Zodiac Decrypto"
#define PROG_VER	"v1.0 Beta 3"

//language
#define LANG_DIR	"language"
#define LANG_ENG	"eng"
#define LANG_SPA	"spa"
#define LANG_GER	"ger"
#define LANG_ITA	"ita"

//text constants
#define CHAR_WIDTH  7
#define CHAR_HEIGHT 12
#define TEXT_POS	0 //HWND_TOPMOST

#define MAX_EXTRA	52

//macros
#define IN_RECT(X,Y,R) (IS_BETWEEN(X,R.left,R.right) && IS_BETWEEN(Y,R.top,R.bottom))

//cipher/key data & files
Message message; //cipher & main key
Message undo_message; //undo key
char szCipherName[1024], szKeyName[1024], szGraphName[1024]; //filenames
char *szCipherBase, *szKeyBase; //file basenames
char szLanguage[32];
int bMsgLoaded=false, bMapLoaded=false, bUndo=false;
const char *szCipher=NULL, *szPlain=NULL; //strings for display
int iNumber, iCurTab;
char szString[128], szStringTitle[128], szNumberTitle[128]; //word, exclude string
typedef std::map<std::string,int> DICTMAP;
DICTMAP dictionary;

//GUI data
char szTitle[64], szText[2048], szExeDir[1024]; 
int iCurSymbol=-1, iCurPat=-1, iCurWord=-1, iTextSel=-1, iRowSel=-1, iColSel=-1; //selections
int iCharWidth=CHAR_WIDTH, iCharHeight=CHAR_HEIGHT; //font size
int iSortBy=0, iWordMin=4, iWordMax=20;
RECT rPatRect, rKeyRect, rWordRect;
int lprgiInitKey[26];
int lprgiInitID[26]=
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
float iCharSize=1.0; //font size multiplier
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
int iUseGraphs=USE_BI+USE_TRI+USE_TETRA+USE_PENTA;
int iPriority, iLang, iBestScore=0;
char szExtraLtr[MAX_EXTRA+1]="";

//Win32 object handles
HWND		hMainWnd, hPat, hKey, hWord, hMainTab, hTextWnd, hCipher=NULL, hPlain=NULL, hLetter=NULL, hHomo=NULL, hScroll;
HACCEL		hAccel;
HPEN 		hRedPen, hGreenPen, hBluePen, hOrangePen, hWhitePen;
HBRUSH		hWhiteBrush;
HDC 		hCipherDC=NULL, hPlainDC=NULL;
HFONT		hTextFont=NULL;
HMENU		hMainMenu, hTextMenu;
HINSTANCE	hInst;
HANDLE		hSolveThread=NULL, hTimerThread=NULL;

//open/save file filter
char szFileFilter[]=
	{"Text Files (*.txt)\0" "*.txt;\0"
	 "All Files (*.*)\0" "*.*\0\0"};
	 
//callback functions for hillclimber
inline void disp_all()  {SendMessage(hMainWnd,WM_COMMAND,UDM_DISPALL,0);}
inline void disp_info() {SendMessage(hMainWnd,WM_COMMAND,UDM_DISPINFO,0);}
inline DWORD GetTime()	{return GetTickCount();}

#endif

