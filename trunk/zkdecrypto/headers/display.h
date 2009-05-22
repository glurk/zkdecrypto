#pragma warning( disable : 4244)	// STOP MSVS2005 WARNINGS
#pragma warning( disable : 4267)

/*Dialog Info Functions*/

//add line breaks to text
void BreakText(char *dest, const char *src)
{
	int dest_index=0;
	
	for(int src_index=0; src[src_index]; src_index++)
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
void SetTitle() {sprintf(szTitle, "%s %s",PROG_NAME,PROG_VER); SetWindowText(hMainWnd,szTitle);}

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

	if(DEFRACTION_TYPE && hDC==hPlainDC) return;

	SelectObject(hDC,hPen);
	
	for(int iChar=iStart; iChar<iEnd; iChar++)
	{
		iRow=(iChar/iLineChars)-iScrollPos;
		iCol=iChar%iLineChars;
		
		if(iRow<0) continue;
		
		if(bOutline) //outline rect
		{
			rOutRect.left=iCol*iCharWidth+2+iShift;
			rOutRect.top=iRow*iCharHeight+1+iShift;
			rOutRect.right=rOutRect.left+iCharWidth;
			rOutRect.bottom=rOutRect.top+iCharHeight;
		}
		
		else //underline rect
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
	int iStart, iEnd, iFreq, cur_symbol, word_len, iDiIndex;
	char szPattern[32];
	const char *word_ptr;
	SYMBOL symbol;
	DIGRAPH digraph;
	NGRAM pattern;

	if(iCurPat>-1) //pattern outlines
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
	
	if(iCurWord>-1) //word outlines
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

	if(DIGRAPH_MODE)
	{
		//symbol outlines
		if(iCurSymbol>-1 && message.digraph_map.GetDigraph(iCurSymbol,&digraph))
			for(cur_symbol=iDispStart; cur_symbol<iDispEnd; cur_symbol++)
			{
				if((cur_symbol%2)==0) iDiIndex=cur_symbol; //first symbol in digraph
				else iDiIndex=cur_symbol-1; //second symbol in digraph 

				if(szCipher[iDiIndex]==digraph.cipher1 && szCipher[iDiIndex+1]==digraph.cipher2)
				{
					OutlineChars(hCipherDC,hBluePen,cur_symbol,cur_symbol+1);
					OutlineChars(hPlainDC,hBluePen,cur_symbol,cur_symbol+1);
				}
			}
	}	

	else
	{
		//symbol outlines
		if(iCurSymbol>-1 && message.cur_map.GetSymbol(iCurSymbol,&symbol))
			for(cur_symbol=iDispStart; cur_symbol<iDispEnd; cur_symbol++)
				if(szCipher[cur_symbol]==symbol.cipher)
				{
					OutlineChars(hCipherDC,hBluePen,cur_symbol,cur_symbol+1);
					OutlineChars(hPlainDC,hBluePen,cur_symbol,cur_symbol+1);
				}
	}

	//selected symbol
	if(IS_BETWEEN(iTextSel,0,message.GetLength()))
	{
		OutlineChars(hCipherDC,hRedPen,iTextSel,iTextSel+1+DIGRAPH_MODE);
		OutlineChars(hPlainDC,hRedPen,iTextSel,iTextSel+1+DIGRAPH_MODE);
	}
}

void OutputText(int bSection)
{
	const char *szString;
	int iIndex, iLength, iXPos, iYPos, iDiIndex;
	HWND hWnd;
	HDC hDC;
	DWORD dwBG;
	COLORREF crBG;
	
	iLength=message.GetLength();
		
	//plain/cipher
	if(bSection) {hWnd=hPlain; hDC=hPlainDC; szString=szPlain;}
	else {hWnd=hCipher; hDC=hCipherDC; szString=szCipher;}

	dwBG=GetSysColor(COLOR_WINDOW);
	crBG=RGB(GetRValue(dwBG),GetGValue(dwBG),GetBValue(dwBG));
	
	for(int row=0; row<iDispLines; row++) //output each character
		for(int col=0; col<iLineChars; col++)
		{
			iIndex=(row+iScrollPos)*iLineChars+col;
			if(iIndex>=iLength) break;
			if(DEFRACTION_TYPE && bSection && iIndex>=iLength>>1) goto EXIT; //don't output plain past 1/2 if on a fractionated cipher
			
			SetBkColor(hDC,crBG); //default background

			//locked highlights
			if(DIGRAPH_MODE)
			{
				if((iIndex%2)==0) iDiIndex=iIndex; //first symbol in digraph
				else iDiIndex=iIndex-1; //second symbol in digraph 

				if(message.digraph_map.GetLock(message.digraph_map.FindByCipher(szCipher[iDiIndex],szCipher[iDiIndex+1])))
						SetBkColor(hDC,crYellow);
			}

			else if(message.cur_map.GetLock(message.cur_map.FindByCipher(szCipher[iIndex])))
					SetBkColor(hDC,crYellow);
			
			//window coordinates for character
			iXPos=(col*iCharWidth)+iTextBorder;
			iYPos=(row*iCharHeight)+iTextBorder;
	
			TextOut(hDC,iXPos,iYPos,szString+iIndex,1);		
		}

EXIT:
	return;
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

	if(iTextSel+1) sprintf(szText,"ASC: %i",(unsigned char)szCipher[iTextSel]);
		else sprintf(szText,"ASC: ");
	SendMessage(hStatus, SB_SETTEXT, 4, (LPARAM)szText);

}

//update when the selected symbol changes
void UpdateSelectedSymbol()
{
	SYMBOL symbol;
	DIGRAPH digraph;
	
	iCurSymbol=SendDlgItemMessage(hMainWnd,IDC_MAP,LB_GETCURSEL,0,0);
	if(iCurSymbol<0) return;

	if(DIGRAPH_MODE)
	{
		message.digraph_map.GetDigraph(iCurSymbol,&digraph);
		sprintf(szText,"%c%c",digraph.plain1,digraph.plain2);
	}

	else
	{
		message.cur_map.GetSymbol(iCurSymbol,&symbol);
		sprintf(szText,"%c",symbol.plain);
	}
	
	SetDlgItemText(hMainWnd,IDC_MAP_VALUE,szText);
	SetText();
}

//handle click in text area
int TextClick(int click_x, int click_y)
{
	int text_row, text_col, text_index;
	char click_char, click_char2;

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

	if(DIGRAPH_MODE && text_index%2) text_index--; //clicked on second symbol of digraph
	click_char=szCipher[text_index];
	click_char2=szCipher[text_index+1];

	//get the symbol for this cipher character
	if(DIGRAPH_MODE) iCurSymbol=message.digraph_map.FindByCipher(click_char,click_char2);
	else iCurSymbol=message.cur_map.FindByCipher(click_char);
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

void SetTextSel(int iIndex)
{
	if(iIndex<0) iIndex+=message.GetLength();
	if(iIndex>=message.GetLength()) iIndex-=message.GetLength();

	iTextSel=iIndex;
	iRowSel=iTextSel/iLineChars;
	iColSel=iTextSel%iLineChars;
	UpdateSelectedSymbol();
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
	if(siSolveInfo.running) return;
	szPlain=message.GetPlain();
	
	if(!siSolveInfo.running) //display new score score
	{
		iBestScore=calcscore(message,message.GetLength(),szPlain);
		SetDlgItemInt(hMainWnd,IDC_SCORE,iBestScore,true);
	}
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
void SetMonoKey()
{
	int cur_sel, num_symbols;
	SYMBOL symbol;
	
	SetText();
	
	if(iCurTab!=0) return;
	
	num_symbols=message.cur_map.GetNumSymbols();

	//title
	sprintf(szText,"Key (%i symbols)",num_symbols);
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

void SetDiKey()
{
	int cur_sel, num_digraphs;
	DIGRAPH digraph;
	
	SetText();
	
	if(iCurTab!=0) return;
	
	num_digraphs=message.digraph_map.GetNumDigraphs();

	//title
	sprintf(szText,"Key (%i digraphs)",num_digraphs);
	SetDlgItemText(hMainWnd,IDC_MAP_TITLE,szText);

	//clear list
	cur_sel=SendDlgItemMessage(hMainWnd,IDC_MAP,LB_GETCURSEL,0,0);
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_RESETCONTENT,0,0);
	
	//add digraphs
	for(int cur_digraph=0; cur_digraph<num_digraphs; cur_digraph++)
	{
		message.digraph_map.GetDigraph(cur_digraph,&digraph);
		if(!digraph.plain1) digraph.plain1=BLANK;
		if(!digraph.plain2) digraph.plain2=BLANK;
		
		//symbol is locked
		if(message.digraph_map.GetLock(cur_digraph)) sprintf(szText,"%c%c[%c%c]%5i",digraph.cipher1,digraph.cipher2,digraph.plain1,digraph.plain2,digraph.freq);
		else sprintf(szText,"%c%c %c%c %5i",digraph.cipher1,digraph.cipher2,digraph.plain1,digraph.plain2,digraph.freq);
		
		SendDlgItemMessage(hMainWnd,IDC_MAP,LB_ADDSTRING,0,(LPARAM)szText);
	}
	
	//reset selected symbol
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,cur_sel,0);
}

void SetKey()
{
	if(DIGRAPH_MODE) SetDiKey();
	else SetMonoKey();
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
	
	if(message.cur_map.GetLock(index)) sprintf(szText,"%c [%c] %5i",symbol.cipher,plain,symbol.freq);
	else sprintf(szText,"%c  %c  %5i",symbol.cipher,plain,symbol.freq);

	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_DELETESTRING,index,0);
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_INSERTSTRING,index,(LPARAM)szText);
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,index,0);
}

void UpdateDigraph(int index)
{
	DIGRAPH digraph;
	char plain1, plain2;

	//get symbol info
	message.digraph_map.GetDigraph(index,&digraph);
	if(digraph.plain1) plain1=digraph.plain1;
	else plain1=BLANK;
	if(digraph.plain2) plain2=digraph.plain2;
	else plain2=BLANK;
	
	if(message.digraph_map.GetLock(index)) sprintf(szText,"%c%c[%c%c]%5i",digraph.cipher1,digraph.cipher2,plain1,plain2,digraph.freq);
	else sprintf(szText,"%c%c %c%c %5i",digraph.cipher1,digraph.cipher2,plain1,plain2,digraph.freq);

	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_DELETESTRING,index,0);
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_INSERTSTRING,index,(LPARAM)szText);
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,index,0);
}

//refresh solver info
inline void SetSolve()
{
	if(iCurTab!=0 && iCurTab!=2 && iCurTab!=3 && iCurTab!=4) return;
	
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
		
	sprintf(szText,"%.4f",IoC(szPlain,message.GetLength()));
	SetDlgItemText(hMainWnd,IDC_IOC_ACT,szText);
	
	sprintf(szText,"%.4f",siSolveInfo.lang_ioc);
	SetDlgItemText(hMainWnd,IDC_IOC_EXP,szText);

	//sprintf(szText,"%.4f",ChiSquare(szPlain,message.GetLength()));
	//SetDlgItemText(hMainWnd,IDC_ENT_ACT,szText);
}

/*Word List Display*/

//put all dictionary words in text into the StringArray
int GetWordList(const char *src_text)
{
	int msg_len;
	std::string word_str;
	   
	if(!src_text) return 0;

	word_list.clear();

	msg_len=strlen(src_text);
	char *text=new char[msg_len+1];
	strcpy(text,src_text);
	strupr(text);

	for(int index=0; index<msg_len; index++)
		for(int word_len=iWordMin; word_len<=iWordMax; word_len++)
		{
			if((msg_len-index)<word_len) break;

			word_str.assign(text+index,word_len); //set word & serach dictionary
			if(dictionary.find(word_str)!=dictionary.end()) word_list[word_str.c_str()]=word_list.size();
		}
		  
	delete text;	

	return word_list.size();	  
}

//set the word list box
void SetWordList()
{
	int cur_sel, rows=0, col=0;
  
	//set list
	siSolveInfo.num_words=GetWordList(szPlain);

	if(iCurTab!=2) return;
	   
	//clear list
	cur_sel=SendDlgItemMessage(hMainWnd,IDC_WORD_LIST,LB_GETCURSEL,0,0);
	SendDlgItemMessage(hMainWnd,IDC_WORD_LIST,LB_RESETCONTENT,0,0);

	for(STRMAP::iterator iter=word_list.begin(); iter!=word_list.end(); ++iter)
	{
		SendDlgItemMessage(hMainWnd,IDC_WORD_LIST,LB_ADDSTRING,0,(LPARAM)std::string(iter->first).c_str());
	}
		   
	//title
	sprintf(szText,"Word List (%i words)",siSolveInfo.num_words);
	SetDlgItemText(hMainWnd,IDC_WORD_TITLE,szText);
}

inline void SetGraph()
{
	if(!hLetter) return;

	lRowCol=message.LetterGraph(szGraph);
	SendMessage(hLetter,WM_INITDIALOG,0,0);
}

inline void SetSolveTabInfo()		{SetKey(); SetSolve();}
inline void SetAnalysisTabInfo()	{SetTable(); SetFreq(); }
inline void SetWordListTabInfo()	{SetWordList();}

inline void SetStatsTabInfo()
{
	if(!bMsgLoaded) return;

	//cipher text stats
	sprintf(szText,"Length [N]: %7i",message.GetLength());
	SetDlgItemText(hMainWnd,IDC_STATS_LENGTH,szText);
	sprintf(szText,"Multiplicity [M]: %.5f",message.Multiplicity());
	SetDlgItemText(hMainWnd,IDC_STATS_MULTI,szText);

	sprintf(szText,"Chi^2 [X2]: %.5f",ChiSquare(message.GetCipher(),message.GetLength()));
	SetDlgItemText(hMainWnd,IDC_STATS_CHI2,szText);
	sprintf(szText,"Entropy [H]: %.5f",Entropy(message.GetCipher(),message.GetLength()));
	SetDlgItemText(hMainWnd,IDC_STATS_ENTRO,szText);
	sprintf(szText,"IoC [IC,DIC,EDIC]: %.5f %.5f %.5f",IoC(message.GetCipher(),message.GetLength()),DIoC(message.GetCipher(),message.GetLength(),1),DIoC(message.GetCipher(),message.GetLength(),2));
	SetDlgItemText(hMainWnd,IDC_STATS_IOC,szText);
	
	//plain text stats
	sprintf(szText,"Chi^2 [X2]: %.5f",ChiSquare(message.GetPlain(),message.GetLength()));
	SetDlgItemText(hMainWnd,IDC_STATS_CHI2_P,szText);
	sprintf(szText,"Entropy [H]: %.5f",Entropy(message.GetPlain(),message.GetLength()));
	SetDlgItemText(hMainWnd,IDC_STATS_ENTRO_P,szText);
	sprintf(szText,"IoC [IC,DIC,EDIC]: %.5f %.5f %.5f",IoC(message.GetPlain(),message.GetLength()),DIoC(message.GetPlain(),message.GetLength(),1),DIoC(message.GetPlain(),message.GetLength(),2));
	SetDlgItemText(hMainWnd,IDC_STATS_IOC_P,szText);
}

inline void SetTabuTabInfo()
{
	int cur_disp;
	szText[26]='\0';

	if(iCurTab!=4) return;
	if(tabu_map.size()>200) return;

	STRMAP::iterator iter=tabu_map.begin();

	szText[0]='\0';
	
	if(iter!=tabu_map.end())
		for(; iter!=tabu_map.end(); ++iter) 
		{
			strcat(szText,std::string(iter->first).c_str());
			strcat(szText,"\r\n");
		}
	
	SetDlgItemText(hMainWnd,IDC_TABU,szText);
}

void SetKeyEdit()
{
	int iMaxKeyLen;

	SendDlgItemMessage(hMainWnd,IDC_KEY_EDIT,EM_SETREADONLY,0,0);

	switch(iSolveType) //set key text & max length
	{
		case SOLVE_HOMO:
		case SOLVE_DISUB: strcpy(szText,""); SendDlgItemMessage(hMainWnd,IDC_KEY_EDIT,EM_SETREADONLY,1,0); break;
		case SOLVE_VIG: strcpy(szText,message.GetKey()); break;
		case SOLVE_RUNKEY: iMaxKeyLen=0; strcpy(szText,siSolveInfo.best_key); break;
		case SOLVE_PLAYFAIR: 
		case SOLVE_BIFID: iMaxKeyLen=25; strcpy(szText,message.polybius5); break;
		case SOLVE_TRIFID: iMaxKeyLen=27; strcpy(szText,message.trifid_array); break;
		case SOLVE_PERMUTE:
		case SOLVE_COLTRANS: strcpy(szText,message.coltrans_key[0]); break;
		case SOLVE_DOUBLE: sprintf(szText,"%s|%s",message.coltrans_key[0],message.coltrans_key[1]); break;
		case SOLVE_ADFGX: iMaxKeyLen=0; sprintf(szText,"%s|%s",message.polybius5,message.coltrans_key[0]); break;
		case SOLVE_ADFGVX: iMaxKeyLen=0; sprintf(szText,"%s|%s",message.polybius6,message.coltrans_key[0]);  break;
		case SOLVE_CEMOPRTU: iMaxKeyLen=0; sprintf(szText,"%s|%s",message.polybius8,message.coltrans_key[0]);  break;
	}

	SetDlgItemText(hMainWnd,IDC_KEY_EDIT,szText); 
	if(LIMITKEY_TYPE) SendDlgItemMessage(hMainWnd,IDC_KEY_EDIT,EM_SETLIMITTEXT,iMaxKeyLen,0); //limit key edit length
	else SendDlgItemMessage(hMainWnd,IDC_KEY_EDIT,EM_SETLIMITTEXT,0,0);
}

inline void SetDlgInfo()
{
	if(!bMsgLoaded) return;
	
	if(siSolveInfo.running) //set key to hillclimber best if running
	{
		if(iSolveType==SOLVE_HOMO) message.cur_map.FromKey(siSolveInfo.best_key);
		else SetKeyEdit();
	}
		
	else SetPlain();
	
	//info on tabs, tabu tab is updated only when a tabu is made
	SetSolveTabInfo(); SetAnalysisTabInfo(); SetWordListTabInfo(); SetStatsTabInfo();
		
	if(hLetter) SetGraph();
}

void GetKeyEdit()
{
	if(siSolveInfo.running) return;
	GetDlgItemText(hMainWnd,IDC_KEY_EDIT,szText,255);
					
	switch(iSolveType)
	{
		case SOLVE_VIG:		message.SetKeyLength(strlen(szText)); message.SetKey(szText); break;
		case SOLVE_BIFID: 
		case SOLVE_PLAYFAIR:memcpy(message.polybius5,szText,25); break;
		case SOLVE_TRIFID:	memcpy(message.trifid_array,szText,27); break;
		case SOLVE_PERMUTE: 
		case SOLVE_COLTRANS: 
		case SOLVE_DOUBLE:	message.SetTransKey(szText); break;
		case SOLVE_ADFGX:	message.SetSplitKey(szText,5); break;
		case SOLVE_ADFGVX:	message.SetSplitKey(szText,6); break;
		case SOLVE_CEMOPRTU:message.SetSplitKey(szText,8); break;
	}

	if(strlen(szText)) SetDlgInfo();
}

void SetSolveTypeFeatures()
{
	int menu_state;
	
	//disabled exclude when in digraph mode
	if(DIGRAPH_MODE) menu_state=MF_GRAYED;
	else menu_state=MF_ENABLED;
	EnableMenuItem(hMainMenu,IDM_KEY_EXCLUDE,MF_BYCOMMAND | menu_state);
	EnableMenuItem(hMainMenu,IDM_KEY_CLEAR_EXCLUDE,MF_BYCOMMAND | menu_state);

	if(iSolveType==SOLVE_HOMO || iSolveType==SOLVE_DISUB) menu_state=MF_ENABLED;
	else menu_state=MF_GRAYED;
	EnableMenuItem(hMainMenu,IDM_FILE_SAVE_MAP,MF_BYCOMMAND | menu_state);
	EnableMenuItem(hMainMenu,IDM_SOLVE_INSERT,MF_BYCOMMAND | menu_state);
	EnableMenuItem(hMainMenu,IDM_KEY_LOCK,MF_BYCOMMAND | menu_state);
	EnableMenuItem(hMainMenu,IDM_KEY_UNLOCK,MF_BYCOMMAND | menu_state);
	EnableMenuItem(hMainMenu,IDM_KEY_LOCK_ALL,MF_BYCOMMAND | menu_state);
	EnableMenuItem(hMainMenu,IDM_KEY_UNLOCK_ALL,MF_BYCOMMAND | menu_state);
	EnableMenuItem(hMainMenu,IDM_KEY_INVERT_LOCK,MF_BYCOMMAND | menu_state);
	EnableMenuItem(hMainMenu,IDM_SOLVE_WORD,MF_BYCOMMAND | menu_state);

	if(ALLOW_LOWERCASE) SetWindowLong(hKeyEdit,GWL_STYLE,lKeyEditStyle);
	else SetWindowLong(hKeyEdit,GWL_STYLE,lKeyEditStyle | ES_UPPERCASE);

	//decoding info
	message.SetDecodeType(iSolveType);
	if(ASCIPHER_TYPE) message.cur_map.AsCipher();
	message.InitArrays();
	SetKeyEdit();
					
	//key update length
	if(DIGRAPH_MODE) SendDlgItemMessage(hMainWnd,IDC_MAP_VALUE,EM_LIMITTEXT,2,0);
	else SendDlgItemMessage(hMainWnd,IDC_MAP_VALUE,EM_LIMITTEXT,1,0);

	if(TRANSPOSE_TYPE)
	{
		SetDlgItemInt(hMainWnd,IDC_IOC_WEIGHT_EDIT,0,false);
		SetDlgItemInt(hMainWnd,IDC_ENT_WEIGHT_EDIT,0,false);
		SetDlgItemInt(hMainWnd,IDC_CHI_WEIGHT_EDIT,0,false);
		SetDlgItemInt(hMainWnd,IDC_DIOC_WEIGHT_EDIT,5,false);
	}
	
	else
	{
		SetDlgItemInt(hMainWnd,IDC_IOC_WEIGHT_EDIT,5,false);
		SetDlgItemInt(hMainWnd,IDC_ENT_WEIGHT_EDIT,5,false);
		SetDlgItemInt(hMainWnd,IDC_CHI_WEIGHT_EDIT,5,false);
		SetDlgItemInt(hMainWnd,IDC_DIOC_WEIGHT_EDIT,0,false);
	}
}

//call when the cipher is changed, i.e. symbol merge
void SetCipher()
{	
	sprintf(szText,"%s",szCipherBase);
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
	int iShowStats=SW_HIDE;
	int iShowTabu=SW_HIDE;
	
	iCurTab=iTab;

	switch(iTab)
	{
		case 0: iShowSolve=SW_SHOW; SetSolveTabInfo(); break;
		case 1: iShowAnalysis=SW_SHOW; SetAnalysisTabInfo(); break;
		case 2: iShowWord=SW_SHOW; SetWordListTabInfo(); break;
		case 3: iShowStats=SW_SHOW; SetStatsTabInfo(); break;
		case 4: iShowTabu=SW_SHOW; SetTabuTabInfo(); break;
	}

	//solve
	ShowWindow(GetDlgItem(hMainWnd,IDC_PAT_TITLE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_PATTERNS),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_MAP_TITLE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_MAP),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_MAP_VALUE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_MAP_CHANGE),iShowSolve);
	ShowWindow(GetDlgItem(hMainWnd,IDC_SOLVE_TITLE),iShowSolve | iShowStats | iShowWord | iShowTabu);
	ShowWindow(GetDlgItem(hMainWnd,IDC_TIME_TITLE),iShowSolve | iShowStats | iShowWord | iShowTabu);
	ShowWindow(GetDlgItem(hMainWnd,IDC_TIME),iShowSolve | iShowStats | iShowWord | iShowTabu);
	ShowWindow(GetDlgItem(hMainWnd,IDC_TRY_TITLE),iShowSolve | iShowStats | iShowWord | iShowTabu);
	ShowWindow(GetDlgItem(hMainWnd,IDC_TRY),iShowSolve | iShowStats | iShowWord | iShowTabu);
	ShowWindow(GetDlgItem(hMainWnd,IDC_FAIL_TITLE),iShowSolve | iShowStats | iShowWord | iShowTabu);
	ShowWindow(GetDlgItem(hMainWnd,IDC_FAIL),iShowSolve | iShowStats | iShowWord | iShowTabu);
	ShowWindow(GetDlgItem(hMainWnd,IDC_SCORE_TITLE),iShowSolve | iShowStats | iShowWord | iShowTabu);
	ShowWindow(GetDlgItem(hMainWnd,IDC_SCORE),iShowSolve | iShowStats | iShowWord | iShowTabu);

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
	ShowWindow(GetDlgItem(hMainWnd,IDC_WORD_STATIC_MIN),iShowWord);
	ShowWindow(GetDlgItem(hMainWnd,IDC_WORD_STATIC_MAX),iShowWord);
	ShowWindow(GetDlgItem(hMainWnd,IDC_WORD_MIN),iShowWord);
	ShowWindow(GetDlgItem(hMainWnd,IDC_WORD_MAX),iShowWord);
	//word length
	SetDlgItemInt(hMainWnd,IDC_WORD_MIN,iWordMin,false);
	SetDlgItemInt(hMainWnd,IDC_WORD_MAX,iWordMax,false);

	//Stats Tab
	ShowWindow(GetDlgItem(hMainWnd,IDC_STATS_TITLE),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_STATS_LENGTH),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_STATS_MULTI),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_STATS_ENTRO),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_STATS_IOC),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_STATS_CHI2),iShowStats);

	ShowWindow(GetDlgItem(hMainWnd,IDC_STATS_TITLE_P),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_STATS_ENTRO_P),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_STATS_IOC_P),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_STATS_CHI2_P),iShowStats);

	ShowWindow(GetDlgItem(hMainWnd,IDC_IOC_WEIGHT_TITLE),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_IOC_WEIGHT_EDIT),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_IOC_WEIGHT_SPIN),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_ENT_WEIGHT_TITLE),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_ENT_WEIGHT_EDIT),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_ENT_WEIGHT_SPIN),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_CHI_WEIGHT_TITLE),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_CHI_WEIGHT_EDIT),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_CHI_WEIGHT_SPIN),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_DIOC_WEIGHT_TITLE),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_DIOC_WEIGHT_EDIT),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_DIOC_WEIGHT_SPIN),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_BLOCK_TITLE),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_BLOCK_EDIT),iShowStats);
	ShowWindow(GetDlgItem(hMainWnd,IDC_BLOCK_SPIN),iShowStats);

	//Tabu Tab
	ShowWindow(GetDlgItem(hMainWnd,IDC_TABU),iShowTabu);

}

//create right click menu
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
		
		if(!DIGRAPH_MODE)
		{
			AppendMenu(hTextMenu,0,IDM_KEY_EXCLUDE,"E&xclude Letters");
			AppendMenu(hTextMenu,0,IDM_KEY_CLEAR_EXCLUDE,"&Clear Exclude");
			AppendMenu(hTextMenu,MF_SEPARATOR,0,0);
		}

		AppendMenu(hTextMenu,0,IDM_VIEW_DESELECT,"&Deselect");
	}
											
	TrackPopupMenu(hTextMenu,TPM_LEFTALIGN | TPM_RIGHTBUTTON,
							 pntClickPoint.x,pntClickPoint.y,0,hMainWnd,0);

	DestroyMenu(hTextMenu);
}
