#ifndef _ZODIAC_H

#include "message.h"
#include "z340.h"

//program
#define PROG_NAME	"Zodiac Decrypto"
#define PROG_VER	"v1.0 Beta 1"

//language
#define LANG_DIR	"language"
#define LANG_ENG	"eng"
#define LANG_SPA	"spa"
#define LANG_GER	"ger"

//index of coincidence for languages
#define ENG_IOC		.0665 //float(1.73/26) 
#define SPA_IOC 	.0746 //float(1.94/26) 
#define GER_IOC 	.0788 //float(2.05/26) 

#define FRE_IOC		.0777 
#define ITA_IOC 	.0746
#define POR_IOC 	.0746
#define RUS_IOC 	.0677

//text constants
#define CHAR_WIDTH  7
#define CHAR_HEIGHT 12
#define TEXT_POS	0 //HWND_TOPMOST

#define MAX_EXTRA	26

//macros
#define IS_BETWEEN(X,Y,Z) (X>=Y && X<=Z)
#define IN_RECT(X,Y,L,T,W,H) (IS_BETWEEN(X,L,L+W) && IS_BETWEEN(Y,T,T+H))

//cipher/key data & files
Message message; //cipher & main key
Message undo_message; //undo key
char szCipherName[1024], szKeyName[1024], szGraphName[1024]; //filenames
char *szCipherBase, *szKeyBase; //file basenames
int bMsgLoaded=false, bMapLoaded=false, bUndo=false;
const char *szCipher=NULL, *szPlain=NULL; //strings for display

//GUI data
char szTitle[64], szText[1024], szExeDir[1024]; 
int iCurSymbol=-1, iCurPat=-1, iTextSel=-1; //selections
int iCharWidth=CHAR_WIDTH, iCharHeight=CHAR_HEIGHT; //font size
int iSortBy=0;

//text gui
float iCharSize=1.0; //font size multiplier
int iLineChars=17, iLines, iDispLines; //text line data
int iScrollPos, iMaxScroll; //scrollbar
int iDispStart, iDispEnd; //index of the start/end characters being displayed
int iMargin=10; //window margin size
int iCipherX=3*iMargin, iCipherY=3*iMargin; //cipher wnd position
int iPlainX, iPlainY=3*iMargin; //plain wnd position
int iTextWidth=100, iTextHeight=100; //dimensions of cipher and plain
POINT pntClickPoint; //click point

//graphs
wchar szGraph[10240];
long lRowCol;

//solver data
SOLVEINFO siSolveInfo;
int iUseGraphs=USE_BI+USE_TRI+USE_TETRA+USE_PENTA;
int iPriority, iLang, iBestScore=0;
char szExtraLtr[MAX_EXTRA+1]="WBVKXZQJ";

//Win32 object handles
HWND		hMainWnd, hMainTab, hTextWnd, hCipher=NULL, hPlain=NULL, hGraph=NULL, hScroll;
HACCEL		hAccel;
HPEN 		hRedPen, hGreenPen, hBluePen;
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

//index of conincidence of a string
float IoC(const char *string)
{
	int freqs[256], length;
	float ic=0;

	if(!string) return 0;

	length=(int)strlen(string);
	if(length<2) return 0;
	memset(freqs,0,256*sizeof(int));

	//count frequencies
	for(int index=0; index<length; index++)
		freqs[string[index]]++;

	//calculate index of conincidence
	for(int sym_index=0; sym_index<256; sym_index++)
		if(freqs[sym_index]>1) 
			ic+=(freqs[sym_index])*(freqs[sym_index]-1); 

	ic/=(length)*(length-1);

	return ic;
}

#endif

