#ifndef _ZODIAC_H

#include "message.h"
#include "z340.h"
#include <map>
#include <string>

//program
#define PROG_NAME	"Zodiac Decrypto"
#define PROG_VER	"v1.0 Beta 2"

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

#define MAX_EXTRA	26

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
int iNumber;
char szString[128], szStringTitle[128], szNumberTitle[128]; //word, exclude string
typedef std::map<std::string,int> DICTMAP;
DICTMAP dictionary;

//GUI data
char szTitle[64], szText[1024], szExeDir[1024]; 
int iCurSymbol=-1, iCurPat=-1, iTextSel=-1, iRowSel=-1, iColSel=-1; //selections
int iCharWidth=CHAR_WIDTH, iCharHeight=CHAR_HEIGHT; //font size
int iSortBy=0;
RECT rKeyRect;

//text gui
COLORREF crRed, crGreen, crBlue, crYellow, crBlack, crWhite;
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
HWND		hMainWnd, hKey, hMainTab, hTextWnd, hCipher=NULL, hPlain=NULL, hGraph=NULL, hScroll;
HACCEL		hAccel;
HPEN 		hRedPen, hGreenPen, hBluePen, hWhitePen;
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

