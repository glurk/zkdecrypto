#pragma warning( disable : 4244)	// STOP MSVS2005 WARNINGS
/*Dialog Info Functions*/

//add line breaks to text
void BreakText(char *dest, const char *src)
{
	int src_index, dest_index=0, src_len=(int)strlen(src);
	
	for(src_index=0; src_index<src_len; src_index++)
	{
		dest[dest_index++]=src[src_index];
		
		if(((src_index+1)%iLineChars)==0)
			{dest[dest_index++]='\r'; dest[dest_index++]='\n';}
	}
	
	dest[dest_index]='\0';
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

//set window title
void SetTitle()
{
	sprintf(szTitle, "%s %s",PROG_NAME,PROG_VER);

	if(bMsgLoaded) 
	{
		sprintf(szText," - %s",szCipherBase);
		strcat(szTitle,szText);
	}
	
	SetWindowText(hMainWnd,szTitle);
}

/*Text Display*/

//reset text scrollbar
void SetScrollBar()
{	
	//number of lines that can be displayed at once
	iDispLines=iTextHeight/iCharHeight;
	
	//total lines required for line width
	iLines=message.GetLength()/iLineChars;
	if(message.GetLength()%iLineChars) iLines++;	
	
	//scroll range
	iMaxScroll=iLines-iDispLines;
	if(iMaxScroll<0) iMaxScroll=0;
	iScrollPos=0;
	
	SetScrollRange(hScroll,SB_CTL,0,iMaxScroll,false);
	SetScrollPos(hScroll,SB_CTL,iScrollPos,true);

	//show/hide scroll bar
	if(!iMaxScroll) ShowWindow(hScroll,SW_HIDE);
	else ShowWindow(hScroll,SW_SHOWNORMAL);
}

//draw outline/underline for range of characters
void OutlineChars(HDC hDC, HPEN hPen, int iStart, int iEnd, int bOutline=false)
{
	RECT rOutRect;
	int iRow, iCol;
	int iShift=-2+iTextBorder;
	
	SelectObject(hDC,hPen);
	
	for(int iChar=iStart; iChar<iEnd; iChar++)
	{
		iRow=(iChar/iLineChars)-iScrollPos;
		iCol=iChar%iLineChars;
		
		if(iRow<0) continue;
	
		//outline rect
		if(bOutline)
		{
			rOutRect.left=iCol*iCharWidth+2+iShift;
			rOutRect.top=iRow*iCharHeight+1+iShift;
			rOutRect.right=rOutRect.left+iCharWidth;
			rOutRect.bottom=rOutRect.top+iCharHeight;
		}
		
		//underline rect
		else
		{
			rOutRect.left=iCol*iCharWidth+2+iShift;
			rOutRect.top=(iRow+1)*iCharHeight+iShift;
			rOutRect.right=rOutRect.left+iCharWidth;
			rOutRect.bottom=rOutRect.top+2;
		}
		
		Rectangle(hDC,rOutRect.left,rOutRect.top,rOutRect.right,rOutRect.bottom);
	}
}

//draw all the necessary outlines/underlines
void DrawOutlines()
{
	int iStart, iEnd, iFreq, cur_symbol, word_len;
	char szPattern[32];
	const char *word_ptr;
	SYMBOL symbol;
	NGRAM pattern;

	//pattern outlines
	if(iCurPat>-1)
	{
		SendDlgItemMessage(hMainWnd,IDC_PATTERNS,LB_GETTEXT,iCurPat,(LPARAM)szPattern);
		if(iSortBy) sscanf(szPattern," %i %s ",&iFreq,pattern.string);
		else sscanf(szPattern," %s %i ",pattern.string,&iFreq);
		
		if(message.GetPattern(&pattern))
			for(int cur_pos=0; cur_pos<pattern.freq; cur_pos++)
			{
				iStart=pattern.positions[cur_pos];
				iEnd=iStart+pattern.length;
				
				//outside of display range
				if(!IS_BETWEEN(iStart,iDispStart,iDispEnd)) continue;
				
				OutlineChars(hCipherDC,hGreenPen,iStart,iEnd);
				OutlineChars(hPlainDC,hGreenPen,iStart,iEnd);
			}
	}
	
	//word outlines
	if(iCurWord>-1)
	{
		SendDlgItemMessage(hMainWnd,IDC_WORD_LIST,LB_GETTEXT,iCurWord,(LPARAM)szPattern);
		
		word_len=(int)strlen(szPattern);
		
		word_ptr=szPlain+iDispStart;
		
		while(word_ptr=strstr(word_ptr,szPattern))
		{
			iStart=word_ptr-szPlain;
			if(iStart>iDispEnd) break;
			iEnd=iStart+word_len;
			
			//outside of display range
			if(!IS_BETWEEN(iStart,iDispStart,iDispEnd)) continue;
			
			OutlineChars(hCipherDC,hOrangePen,iStart,iEnd);
			OutlineChars(hPlainDC,hOrangePen,iStart,iEnd);
			
			word_ptr+=word_len;
		} 
	}
	
	//symbol outlines
	if(iCurSymbol>-1 && message.cur_map.GetSymbol(iCurSymbol,&symbol))
		for(cur_symbol=iDispStart; cur_symbol<iDispEnd; cur_symbol++)
			if(szCipher[cur_symbol]==symbol.cipher)
			{
				OutlineChars(hCipherDC,hBluePen,cur_symbol,cur_symbol+1);
				OutlineChars(hPlainDC,hBluePen,cur_symbol,cur_symbol+1);
			}

	//selected character
	if(IS_BETWEEN(iTextSel,0,message.GetLength()))
	{
		OutlineChars(hCipherDC,hRedPen,iTextSel,iTextSel+1);
		OutlineChars(hPlainDC,hRedPen,iTextSel,iTextSel+1);
	}
}

void OutputText(int bSection)
{
	const char *szString;
	int iIndex, iLength, iXPos, iYPos;
	HWND hWnd;
	HDC hDC;
	DWORD dwBG;
	COLORREF crBG;
	
	iLength=message.GetLength();
		
	//plain/cipher
	if(bSection) {hWnd=hPlain; hDC=hPlainDC; szString=szPlain; }
	else {hWnd=hCipher; hDC=hCipherDC; szString=szCipher;}

	dwBG=GetSysColor(COLOR_WINDOW);
	crBG=RGB(GetRValue(dwBG),GetGValue(dwBG),GetBValue(dwBG));
	
	for(int row=0; row<iDispLines; row++)
		for(int col=0; col<iLineChars; col++)
		{
			iIndex=(row+iScrollPos)*iLineChars+col;
			if(iIndex>=iLength) break;
				
			if(message.cur_map.GetLock(message.cur_map.FindByCipher(szCipher[iIndex])))
				SetBkColor(hDC,crYellow);
			else SetBkColor(hDC,crBG);	
			
			iXPos=(col*iCharWidth)+iTextBorder;
			iYPos=(row*iCharHeight)+iTextBorder;
	
			TextOut(hDC,iXPos,iYPos,szString+iIndex,1);		
		}
}

//refresh text display
void SetText()
{
	int msg_len=message.GetLength();
	HWND hStatus;
	
	iDispStart=iScrollPos*iLineChars;
	iDispEnd=iDispStart+(iDispLines*iLineChars);
	if(iDispEnd>msg_len) iDispEnd=msg_len;
	
	OutputText(0);
	OutputText(1);
	DrawOutlines();
	
	sprintf(szText,"");

	sprintf(szText,"LANG: %s",szLanguage);
	hStatus = GetDlgItem(hTextWnd, IDC_TEXT_STATUS);
    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)szText);

	if(iRowSel+1) sprintf(szText,"ROW: %i",iRowSel+1);
		else sprintf(szText,"ROW: ");
	SendMessage(hStatus, SB_SETTEXT, 1, (LPARAM)szText);

	if(iColSel+1) sprintf(szText,"COL: %i",iColSel+1);
		else sprintf(szText,"COL: ");
	SendMessage(hStatus, SB_SETTEXT, 2, (LPARAM)szText);

	if(iTextSel+1) sprintf(szText,"CHAR: %i",iTextSel+1);
		else sprintf(szText,"CHAR: ");
	SendMessage(hStatus, SB_SETTEXT, 3, (LPARAM)szText);

}

//update when the selected symbol changes
void UpdateSelectedSymbol()
{
	SYMBOL symbol;
	
	iCurSymbol=SendDlgItemMessage(hMainWnd,IDC_MAP,LB_GETCURSEL,0,0);
	if(iCurSymbol<0) return;

	message.cur_map.GetSymbol(iCurSymbol,&symbol);
	sprintf(szText,"%c",symbol.plain);
	SetDlgItemText(hMainWnd,IDC_MAP_VALUE,szText);
	SetText();
}

//handle click in text area
int TextClick(int click_x, int click_y)
{
	int text_row, text_col, text_index;
	char click_char;

	//row/column for clicked character
	text_row=(click_y-iTextBorder)/iCharHeight;
	text_col=(click_x-iTextBorder)/iCharWidth;

	//check if row/col is outside of text bounds
	if(!IS_BETWEEN(text_row,0,iLines-1)) return 0;
	if(!IS_BETWEEN(text_col,0,iLineChars-1)) return 0;
	
	//index into string for clicked character
	text_index=(text_row+iScrollPos)*iLineChars+text_col;
	
	//check array bounds & get cipher character
	if(!IS_BETWEEN(text_index,0,message.GetLength())) return 0;
	click_char=szCipher[text_index];

	//get the symbol for this cipher character
	iCurSymbol=message.cur_map.FindByCipher(click_char);
	if(iCurSymbol<0) return 0;

	//set selected symbol and draw text
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,iCurSymbol,0);

	//set selection info
	iTextSel=text_index;
	iRowSel=text_row+iScrollPos;
	iColSel=text_col;
	
	UpdateSelectedSymbol();

	return 1;
}

//draw white rects on text areas, and set up cliping paths
void ClearTextAreas()
{
	RECT rTextRect;

	//fill white
	GetClientRect(hCipher,&rTextRect);
	rTextRect.left+=iTextBorder; rTextRect.top+=iTextBorder;
	//rTextRect.right-=iTextBorder; rTextRect.bottom-=iTextBorder;
	FillRect(hCipherDC,&rTextRect,hWhiteBrush);

	//set clipping path
	SelectObject(hPlainDC,hWhitePen);
	BeginPath(hCipherDC);
	Rectangle(hCipherDC,rTextRect.left,rTextRect.top,rTextRect.right,rTextRect.bottom);
    EndPath(hCipherDC); 
    SelectClipPath(hCipherDC,RGN_COPY); 

	//fill white
	GetClientRect(hPlain,&rTextRect);
	rTextRect.left+=iTextBorder; rTextRect.top+=iTextBorder;
	//rTextRect.right-=iTextBorder; rTextRect.bottom-=iTextBorder;
	FillRect(hPlainDC,&rTextRect,hWhiteBrush);

	//set clipping path
	SelectObject(hPlainDC,hWhitePen);
	BeginPath(hPlainDC);
	Rectangle(hPlainDC,rTextRect.left,rTextRect.top,rTextRect.right,rTextRect.bottom);
	EndPath(hPlainDC); 
    SelectClipPath(hPlainDC,RGN_COPY);
}

//set font size
void SetCharSize()
{
	if(iCharSize<1) iCharSize=1;
	if(iCharSize>3) iCharSize=3;
	
	//calculate new char dimensions
	iCharWidth=ROUNDTOINT(iCharSize*CHAR_WIDTH);
	iCharHeight=ROUNDTOINT(iCharSize*CHAR_HEIGHT);

	//load font into windows
	if(hTextFont) CloseHandle(hTextFont);
	hTextFont=CreateFont(iCharHeight,iCharWidth,0,0,FW_NORMAL,0,0,0,0,0,0,0,0,"ZKDfont");
	if(hCipher) SelectObject(hCipherDC,hTextFont);
	if(hPlain) SelectObject(hPlainDC,hTextFont);

	//draw text
	SetScrollBar();
	ClearTextAreas();
	SetText();
}

//call when key is changed to decode and display plain text
void SetPlain()
{
	szPlain=message.GetPlain();
}

/*Solve Tab Display*/

void AddPattern(NGRAM *pattern)
{
	if(iSortBy) sprintf(szText,"%5i %-15s",pattern->freq,pattern->string);
	else sprintf(szText,"%-15s %5i",pattern->string,pattern->freq);

	SendDlgItemMessage(hMainWnd,IDC_PATTERNS,LB_ADDSTRING,0,(WPARAM)szText);
}

//refresh pattern list
void SetPatterns()
{
	int num_patterns;
	
	//clear list
	SendDlgItemMessage(hMainWnd,IDC_PATTERNS,LB_RESETCONTENT,0,0);
	
	//add patterns to list
	num_patterns=message.PrintPatterns(AddPattern);
	
	//title
	sprintf(szText,"Patterns (%i)",num_patterns);
	SetDlgItemText(hMainWnd,IDC_PAT_TITLE,szText);
}

void SetSort(int iNewSort)
{
	iSortBy=iNewSort;
	CheckMenuItem(hMainMenu,IDM_VIEW_BYSTRING,MF_BYCOMMAND | (iSortBy? MF_UNCHECKED:MF_CHECKED));
	CheckMenuItem(hMainMenu,IDM_VIEW_BYFREQ,MF_BYCOMMAND | (iSortBy? MF_CHECKED:MF_UNCHECKED));
	SetPatterns();
}

//refresh key list
inline void SetKey()
{
	int cur_sel, num_symbols;
	SYMBOL symbol;
	
	SetText();
	
	if(iCurTab!=0) return;
	
	num_symbols=message.cur_map.GetNumSymbols();

	//title
	sprintf(szText,"Key (%i symbols)",message.cur_map.GetNumSymbols());
	SetDlgItemText(hMainWnd,IDC_MAP_TITLE,szText);

	//clear list
	cur_sel=SendDlgItemMessage(hMainWnd,IDC_MAP,LB_GETCURSEL,0,0);
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_RESETCONTENT,0,0);
	
	//add symbols
	for(int cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
	{
		message.cur_map.GetSymbol(cur_symbol,&symbol);
		if(!symbol.plain) symbol.plain=BLANK;
		
		//symbol is locked
		if(message.cur_map.GetLock(cur_symbol))
			sprintf(szText, "%c [%c] %5i",symbol.cipher,symbol.plain,symbol.freq);
		
		//not locked
		else sprintf(szText,"%c  %c  %5i",symbol.cipher,symbol.plain,symbol.freq);
		
		SendDlgItemMessage(hMainWnd,IDC_MAP,LB_ADDSTRING,0,(LPARAM)szText);
	}
	
	//reset selected symbol
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,cur_sel,0);
}

//update symbol in list
void UpdateSymbol(int index)
{
	SYMBOL symbol;
	char plain;

	//get symbol info
	message.cur_map.GetSymbol(index,&symbol);
	if(symbol.plain) plain=symbol.plain;
	else plain=BLANK;
	
	if(message.cur_map.GetLock(index))
			sprintf(szText,"%c [%c] %5i",symbol.cipher,plain,symbol.freq);
		
	else sprintf(szText,"%c  %c  %5i",symbol.cipher,plain,symbol.freq);

	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_DELETESTRING,index,0);
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_INSERTSTRING,index,(LPARAM)szText);
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,index,0);
}

//refresh solver info
inline void SetSolve()
{
	if(iCurTab!=0) return;
	
	//iteration & time
	sprintf(szText,"%i (%.2fs)",siSolveInfo.cur_try,siSolveInfo.last_time);
	SetDlgItemText(hMainWnd,IDC_TRY,szText);
	
	//failures
	sprintf(szText,"%i of %i",siSolveInfo.cur_fail,siSolveInfo.max_fail);
	SetDlgItemText(hMainWnd,IDC_FAIL,szText);
	
	//best score
	if(siSolveInfo.running) iBestScore=siSolveInfo.best_score;
	SetDlgItemInt(hMainWnd,IDC_SCORE,iBestScore,true);
}

/*Analysis Tab Display*/

//refresh symbol table
inline void SetTable()
{
	if(iCurTab!=1) return;
	message.cur_map.SymbolTable(szText);
	SetDlgItemText(hMainWnd,IDC_TABLE,szText);
}

//refresh letter frequency list
void SetFreq()
{
	int letter, diff, total_diff=0;
	int lprgiActFreq[26], lprgiExpFreq[26];
	int iActFreq, iExpFreq;
	float act_vowel, exp_vowel;
	char msg[1024]="";
	
	if(iCurTab!=1) return;
	if(!bMsgLoaded) return;

	//actual and expected frequencies
	message.GetActFreq(lprgiActFreq);
	message.GetExpFreq(lprgiExpFreq);
	
	//add each letter
	for(letter=0; letter<26; letter++)
	{
		iActFreq=lprgiActFreq[letter];
		iExpFreq=lprgiExpFreq[letter];
		
		//current and total frequency difference
		diff=iActFreq-iExpFreq;					

		sprintf(szText,"%c  %5i  %5i  %5i\r\n",letter+'A',iActFreq,iExpFreq,diff);
		strcat(msg,szText);

		if(diff<0) total_diff+=-1*diff;
		else total_diff+=diff;
	}
	
	//reset selected letter
	SetDlgItemText(hMainWnd,IDC_LTRFREQ,msg);

	//total difference
	SetDlgItemInt(hMainWnd,IDC_DIFF,total_diff,0);

	//vowel percentage
	act_vowel=float(lprgiActFreq[0]+lprgiActFreq[4]+lprgiActFreq[8]+lprgiActFreq[14]+lprgiActFreq[20]);
	act_vowel=(100*act_vowel)/message.GetLength();
	exp_vowel=message.cur_map.GetUnigraph(0);
	exp_vowel+=message.cur_map.GetUnigraph(4);
	exp_vowel+=message.cur_map.GetUnigraph(8);
	exp_vowel+=message.cur_map.GetUnigraph(14);
	exp_vowel+=message.cur_map.GetUnigraph(20);
	
	sprintf(szText,"%.2f%%",act_vowel);
	SetDlgItemText(hMainWnd,IDC_VOWEL_ACT,szText);

	sprintf(szText,"%.2f%%",exp_vowel);
	SetDlgItemText(hMainWnd,IDC_VOWEL_EXP,szText);
	
	sprintf(szText,"%.4f",IoC(szPlain));
	SetDlgItemText(hMainWnd,IDC_IOC_ACT,szText);
	
	sprintf(szText,"%.4f",fLangIoC);
	SetDlgItemText(hMainWnd,IDC_IOC_EXP,szText);

	sprintf(szText,"%.4f",ChiSquare(szPlain));
	SetDlgItemText(hMainWnd,IDC_ENT_ACT,szText);
}

/*Word List Display*/

int GetWordList(const char *text, StringArray &word_list)
{
	int msg_len=message.GetLength();
	char word[64];
	std::string word_str;
	DICTMAP::iterator iter;
	
	for(int index=0; index<msg_len; index++)
		for(int word_len=iWordMin; word_len<=iWordMax; word_len++)
		{
			if((msg_len-index)<word_len) break;

			//get string of word
			memcpy(word,text+index,word_len);
			word[word_len]='\0';
			word_str=word;

			//find word
			if(dictionary.find(word_str)!=dictionary.end()) //is in dictionary
				word_list.AddString(word);
		}
		
	word_list.RemoveDups();
	return word_list.GetNumStrings();	
}

void SetWordList()
{
	int cur_sel, rows=0, col=0;
	int num_words;
	char word[64];
	StringArray word_list;
	
	if(iCurTab!=2) return;
	
	//clear list
	cur_sel=SendDlgItemMessage(hMainWnd,IDC_WORD_LIST,LB_GETCURSEL,0,0);
	SendDlgItemMessage(hMainWnd,IDC_WORD_LIST,LB_RESETCONTENT,0,0);

	//set list
	num_words=GetWordList(szPlain,word_list);
	
	for(int cur_word=0; cur_word<num_words; cur_word++)
	{
		word_list.GetString(cur_word,word);
		SendDlgItemMessage(hMainWnd,IDC_WORD_LIST,LB_ADDSTRING,0,(LPARAM)word);
	}
		
	//title
	sprintf(szText,"Word List (%i words)",num_words);
	SetDlgItemText(hMainWnd,IDC_WORD_TITLE,szText);
}

inline void SetGraph()
{
	if(!hLetter) return;

	lRowCol=message.LetterGraph(szGraph);
	SendMessage(hLetter,WM_INITDIALOG,0,0);
}

inline void SetSolveTabInfo()
{
	SetKey();
	SetSolve();
}

inline void SetAnalysisTabInfo()
{
	SetTable();
	SetFreq();
}

inline void SetWordListTabInfo()
{
	SetWordList();
}

inline void SetDlgInfo()
{
	if(!bMsgLoaded) return;
	
	//set key to hillclimber best if running
	if(siSolveInfo.running)
		message.cur_map.FromKey(siSolveInfo.best_key);
		
	SetPlain();
	
	//info on tabs
	SetSolveTabInfo();
	SetAnalysisTabInfo();
	SetWordListTabInfo();
	
	SetGraph();
}

//call when the cipher is changed, i.e. symbol merge
void SetCipher()
{	
	sprintf(szText,"N=%i, M=%.3f, H=%.3f, IC=%.3f, X2=%.3f",
					message.GetLength(),
					message.Multiplicity(),
					Entropy(message.GetCipher()),
					IoC(message.GetCipher()),
					ChiSquare(message.GetCipher()));

	SetWindowText(hTextWnd,szText);

	szCipher=message.GetCipher();
	
	SetPatterns();
	SetDlgInfo();
}

//resize the children in the text window
void ResizeText(int iNewWidth, int iNewHeight)
{
	int iMargin=10, iX=0, iY=0, iW, iH;

	//border 
	iX=iMargin;
	iY=iMargin;
	iW=iNewWidth-(iMargin<<1);
	iH=iNewHeight-(iMargin<<1);
	SetWindowPos(GetDlgItem(hTextWnd,IDC_CIPHER_BORDER),0,iX,iY,iW,iH,SWP_NOZORDER);

	//cipher 
	iCipherX=(iX+=iMargin);
	iCipherY=(iY+=iMargin+6);
	iTextWidth=(iW=(iW>>1)-(iMargin<<1));
	iTextHeight=(iH=iH-(iMargin<<1)-26);
	SetWindowPos(hCipher,0,iX,iY,iW,iH,SWP_NOZORDER);

	//plain 
	iPlainX=(iX=iX+iW+19);
	iPlainY=iY;
	SetWindowPos(hPlain,0,iX,iY,iW,iH,SWP_NOZORDER);

	//scrollbar
	iX=iX+iW+2;
	iW=18;
	SetWindowPos(hScroll,0,iX,iY,iW,iH,SWP_NOZORDER);

	//status size 
    SendMessage(GetDlgItem(hTextWnd, IDC_TEXT_STATUS), WM_SIZE, 0, 0);

	ClearTextAreas();
	SetScrollBar();
	SetText();
}

void ShowTab(int iTab)
{
	int iShowSolve=SW_HIDE;
	int iShowAnalysis=SW_HIDE;
	int iShowWord=SW_HIDE;
	
	iCurTab=iTab;

	switch(iTab)
	{
		case 0: iShowSolve=SW_SHOW; SetSolveTabInfo(); break;
		case 1: iShowAnalysis=SW_SHOW; SetAnalysisTabInfo(); break;
		case 2: iShowWord=SW_SHOW; SetWordListTabInfo(); break;
	}

	//solve
	ShowWindow(GetDlgItem(hMainWnd,IDC_PAT_TITLE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_PATTERNS),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_MAP_TITLE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_MAP),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_MAP_VALUE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_MAP_CHANGE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_SOLVE_TITLE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_TIME_TITLE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_TIME),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_TRY_TITLE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_TRY),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_FAIL_TITLE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_FAIL),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_SCORE_TITLE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_SCORE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_IOC_WEIGHT_TITLE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_IOC_WEIGHT_EDIT),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_IOC_WEIGHT_SPIN),iShowSolve);

	//analysis
	ShowWindow(GetDlgItem(hMainWnd,IDC_TABLE_TITLE),iShowAnalysis);
	ShowWindow(GetDlgItem(hMainWnd,IDC_TABLE),iShowAnalysis);
	ShowWindow(GetDlgItem(hMainWnd,IDC_LTRFREQ_TITLE),iShowAnalysis);
	ShowWindow(GetDlgItem(hMainWnd,IDC_LTRFREQ_HEADER),iShowAnalysis);
	ShowWindow(GetDlgItem(hMainWnd,IDC_LTRFREQ),iShowAnalysis);
	ShowWindow(GetDlgItem(hMainWnd,IDC_DIFF_TITLE),iShowAnalysis);
	ShowWindow(GetDlgItem(hMainWnd,IDC_DIFF),iShowAnalysis);
	ShowWindow(GetDlgItem(hMainWnd,IDC_VOWEL_ACT_TITLE),iShowAnalysis);
	ShowWindow(GetDlgItem(hMainWnd,IDC_VOWEL_ACT),iShowAnalysis);
	ShowWindow(GetDlgItem(hMainWnd,IDC_VOWEL_EXP),iShowAnalysis);
	ShowWindow(GetDlgItem(hMainWnd,IDC_IOC_ACT_TITLE),iShowAnalysis);
	ShowWindow(GetDlgItem(hMainWnd,IDC_IOC_ACT),iShowAnalysis);
	ShowWindow(GetDlgItem(hMainWnd,IDC_IOC_EXP),iShowAnalysis);
	ShowWindow(GetDlgItem(hMainWnd,IDC_ENT_ACT),iShowAnalysis);
	
	//Word List
	ShowWindow(GetDlgItem(hMainWnd,IDC_WORD_TITLE),iShowWord);
	ShowWindow(GetDlgItem(hMainWnd,IDC_WORD_LIST),iShowWord);
	ShowWindow(GetDlgItem(hMainWnd,IDC_WORD_STATIC),iShowWord);
	ShowWindow(GetDlgItem(hMainWnd,IDC_WORD_MIN),iShowWord);
	ShowWindow(GetDlgItem(hMainWnd,IDC_WORD_MAX),iShowWord);
	//word length
	SetDlgItemInt(hMainWnd,IDC_WORD_MIN,iWordMin,false);
	SetDlgItemInt(hMainWnd,IDC_WORD_MAX,iWordMax,false);

}

void CreateTextMenu()
{
	GetCursorPos(&pntClickPoint);
			
	hTextMenu=CreatePopupMenu();

	if(iCurTab==0 && IN_RECT(pntClickPoint.x,pntClickPoint.y,rPatRect))
	{
		AppendMenu(hTextMenu,0,IDM_VIEW_DESELECT,"&Deselect");
	}

	else if(iCurTab==2 && IN_RECT(pntClickPoint.x,pntClickPoint.y,rWordRect))
	{
		AppendMenu(hTextMenu,0,IDM_VIEW_LOCK_WORD,"Lock Wo&rd");
		AppendMenu(hTextMenu,0,IDM_VIEW_UNLOCK_WORD,"Unloc&k Word");
		AppendMenu(hTextMenu,MF_SEPARATOR,0,0);
		AppendMenu(hTextMenu,0,IDM_VIEW_DESELECT,"&Deselect");
	}

	else 
	{
		AppendMenu(hTextMenu,0,IDM_KEY_LOCK,"&Lock Symbol");
		AppendMenu(hTextMenu,0,IDM_KEY_UNLOCK,"&Unlock Symbol");
		AppendMenu(hTextMenu,MF_SEPARATOR,0,0);
		AppendMenu(hTextMenu,0,IDM_KEY_EXCLUDE,"E&xclude Letters");
		AppendMenu(hTextMenu,0,IDM_KEY_CLEAR_EXCLUDE,"&Clear Exclude");
		AppendMenu(hTextMenu,MF_SEPARATOR,0,0);
		AppendMenu(hTextMenu,0,IDM_VIEW_DESELECT,"&Deselect");
	}
											
	TrackPopupMenu(hTextMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,
							 pntClickPoint.x,pntClickPoint.y,0,hMainWnd,0);

	DestroyMenu(hTextMenu);
}
