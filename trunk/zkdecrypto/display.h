#pragma warning( disable : 4244)	// STOP MSVS2005 WARNINGS

/*Dialog Info Functions*/

//get subset of string needed for display, based on scroll position & chars/line
int GetDisplayText(const char *src, char *dest)
{
	int dest_index=0, line=0;
	
	if(!src || !dest) return 0;

	for(int cur_char=iScrollPos*iLineChars; cur_char<iCipherLength; cur_char++)
	{
		if(line>=iDispLines) break;
		
		dest[dest_index++]=src[cur_char];
		
		//end of line
		if(!((cur_char+1)%iLineChars)) 
		{
			dest[dest_index++]='\r';
			dest[dest_index++]='\n';
			line++;
		}
	}

	dest[dest_index]='\0';
	return 1;
}

//set window title
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

//reset text scrollbar
void SetScrollBar()
{	
	//number of lines that can be displayed at once
	iDispLines=TEXT_HEIGHT/iCharHeight;
	
	//total lines required for line width
	iLines=iCipherLength/iLineChars;
	if(iCipherLength%iLineChars) iLines++;	
	
	//scroll range
	iMaxScroll=iLines-iDispLines;
	if(iMaxScroll<0) iMaxScroll=0;
	iScrollPos=0;
	
	SetScrollRange(hScroll,SB_CTL,0,iMaxScroll,false);
	SetScrollPos(hScroll,SB_CTL,iScrollPos,true);
}

//draw outline/underline for range of characters
void OutlineChars(HDC hDC, HPEN hPen, int iStart, int iEnd, int bOutline=false)
{
	RECT rOutRect;
	int iRow, iCol;
	int iShift=-2;
	
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
	int iStart, iEnd;
	SYMBOL symbol;
	NGRAM pattern;

	//pattern outlines
	if(hCipherDC && message.GetPattern(iCurPat,&pattern))
		for(int cur_pos=0; cur_pos<pattern.freq; cur_pos++)
		{
			iStart=pattern.positions[cur_pos];
			iEnd=iStart+pattern.length;
			OutlineChars(hCipherDC,hGreenPen,iStart,iEnd);
			OutlineChars(hPlainDC,hGreenPen,iStart,iEnd);
		}
	
	//symbol outlines
	if(hCipherDC && hPlainDC && message.cur_map.GetSymbol(iCurSymbol,&symbol))
		for(int cur_symbol=0; cur_symbol<iCipherLength; cur_symbol++)
		{
			if(szCipher[cur_symbol]==symbol.cipher)
			{
				OutlineChars(hCipherDC,hBluePen,cur_symbol,cur_symbol+1);
				OutlineChars(hPlainDC,hBluePen,cur_symbol,cur_symbol+1);
			}
		}

	//selected character
	if(IS_BETWEEN(iTextSel,0,iCipherLength))
	{
		OutlineChars(hCipherDC,hRedPen,iTextSel,iTextSel+1);
		OutlineChars(hPlainDC,hRedPen,iTextSel,iTextSel+1);
	}
}

//refresh text display
void SetText()
{
	if(hCipher && GetDisplayText(szCipher,szText))
		SetWindowText(hCipher,szText);
		
	if(hPlain && GetDisplayText(szPlain,szText))
		SetWindowText(hPlain,szText);

	DrawOutlines();
}

//handle click in text area
int TextClick(int click_x, int click_y)
{
	int window_x, window_y;
	int text_row, text_col, text_index;
	char click_char;

	///click in cipher text
	if(IN_RECT(click_x,click_y,CIPHER_X,CIPHER_Y,TEXT_WIDTH,TEXT_HEIGHT))
	{
		window_x=CIPHER_X;
		window_y=CIPHER_Y;
	}

	//click in plain text
	else if(IN_RECT(click_x,click_y,PLAIN_X,PLAIN_Y,TEXT_WIDTH,TEXT_HEIGHT))
	{
		window_x=PLAIN_X;
		window_y=PLAIN_Y;
	}

	else return 0;

	//row/column for clicked character
	text_row=(click_y-window_y)/iCharHeight;
	text_col=(click_x-window_x)/iCharWidth;

	//check if row/col is outside of text bounds
	if(!IS_BETWEEN(text_row,0,iLines-1)) return 0;
	if(!IS_BETWEEN(text_col,0,iLineChars-1)) return 0;
	
	//index into string for clicked character
	text_index=(text_row+iScrollPos)*iLineChars+text_col;
	
	//check array bounds & get cipher character
	if(!IS_BETWEEN(text_index,0,iCipherLength)) return 0;
	click_char=szCipher[text_index];

	//get the symbol for this cipher character
	iCurSymbol=message.cur_map.FindByCipher(click_char);
	if(iCurSymbol<0) return 0;

	//set selected symbol and draw text
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,iCurSymbol,0);

	//set selection info
	iTextSel=text_index;
	sprintf(szText,"Row %i, Column %i, Character %i",text_row+1,text_col+1,iTextSel+1);
	SetDlgItemText(hMainWnd,IDC_TEXTINFO,szText);

	SetText();

	return 1;
}

//set font size
void SetCharSize()
{
	if(iCharSize<1) iCharSize=1;
	if(iCharSize>2) iCharSize=2;
	
	//calculate new char dimensions
	iCharWidth=ROUNDTOINT(iCharSize*CHAR_WIDTH);
	
	if(iCharWidth*iLineChars>TEXT_WIDTH)
	{
		iCharSize=(TEXT_WIDTH/float(iLineChars))/CHAR_WIDTH;
		iCharWidth=ROUNDTOINT(iCharSize*CHAR_WIDTH);
	}

	iCharHeight=ROUNDTOINT(iCharSize*CHAR_HEIGHT);

	//load font into windows
	if(hTextFont) CloseHandle(hTextFont);
	hTextFont=CreateFont(iCharHeight,iCharWidth,0,0,FW_NORMAL,0,0,0,0,0,0,0,0,"Lucida Console");
	if(hCipher) SendMessage(hCipher,WM_SETFONT,(WPARAM)hTextFont,0);
	if(hPlain) SendMessage(hPlain,WM_SETFONT,(WPARAM)hTextFont,0);

	//draw text
	SetScrollBar();
	SetText();
}

//refresh pattern list
void SetPatterns()
{
	int num_patterns;
	NGRAM pattern;
	
	num_patterns=message.GetNumPatterns();
	
	//title
	sprintf(szText,"Patterns (%i)",num_patterns);
	SetDlgItemText(hMainWnd,IDC_PAT_TITLE,szText);
	
	//clear list
	SendDlgItemMessage(hMainWnd,IDC_PATTERNS,LB_RESETCONTENT,0,0);
	
	//add patterns to list
	for(int cur_pat=0; cur_pat<num_patterns; cur_pat++)
	{
		message.GetPattern(cur_pat,&pattern);
		sprintf(szText,"%-12s %2i",pattern.string,pattern.freq);
		SendDlgItemMessage(hMainWnd,IDC_PATTERNS,LB_ADDSTRING,0,(WPARAM)szText);
	}
}

//refresh symbol table
void SetTable()
{
	message.cur_map.SymbolTable(szText);
	SetDlgItemText(hMainWnd,IDC_TABLE,szText);
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
			sprintf(szText,"%c  [%c]  %3i",symbol.cipher,plain,symbol.freq);
		
	else sprintf(szText,"%c   %c   %3i",symbol.cipher,plain,symbol.freq);

	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_DELETESTRING,index,0);
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_INSERTSTRING,index,(LPARAM)szText);
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,index,0);
}

//refresh key list
void SetKey()
{
	int cur_sel;
	SYMBOL symbol;

	//title
	sprintf(szText,"Key (%i symbols)",iSymbols);
	SetDlgItemText(hMainWnd,IDC_MAP_TITLE,szText);

	//clear list
	cur_sel=SendDlgItemMessage(hMainWnd,IDC_MAP,LB_GETCURSEL,0,0);
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_RESETCONTENT,0,0);
	
	//add symbols
	for(int cur_symbol=0; cur_symbol<iSymbols; cur_symbol++)
	{
		message.cur_map.GetSymbol(cur_symbol,&symbol);
		if(!symbol.plain) symbol.plain='-';
		
		//symbol is locked
		if(message.cur_map.GetLock(cur_symbol))
			sprintf(szText,"%c  [%c]  %3i",symbol.cipher,symbol.plain,symbol.freq);
		
		//not locked
		else sprintf(szText,"%c   %c   %3i",symbol.cipher,symbol.plain,symbol.freq);
		
		SendDlgItemMessage(hMainWnd,IDC_MAP,LB_ADDSTRING,0,(LPARAM)szText);
	}
	
	//reset selected symbol
	SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,cur_sel,0);
	
	SetTable();
}

//refresh letter frequency list
void SetFreq()
{
	int letter, diff, total_diff=0;
	int lprgiActFreq[26], lprgiExpFreq[26];
	int iActFreq, iExpFreq;
	int cur_sel;
	float act_vowel, exp_vowel;

	//actual and expected frequencies
	message.GetActFreq(lprgiActFreq);
	message.GetExpFreq(lprgiExpFreq);
	
	//letter frequencies
	cur_sel=SendDlgItemMessage(hMainWnd,IDC_LTRFREQ,LB_GETCURSEL,0,0);
	SendDlgItemMessage(hMainWnd,IDC_LTRFREQ,LB_RESETCONTENT,0,0);

	//add each letter
	for(letter=0; letter<26; letter++)
	{
		iActFreq=lprgiActFreq[letter];
		iExpFreq=lprgiExpFreq[letter];
		
		//current and total frequency difference
		diff=iActFreq-iExpFreq;
		if(diff<0) diff*=-1;
		total_diff+=diff;					

		sprintf(szText,"%c   %3i   %3i   %3i",letter+'A',iActFreq,iExpFreq,diff);
		SendDlgItemMessage(hMainWnd,IDC_LTRFREQ,LB_ADDSTRING,0,(LPARAM)szText);
	}
	
	//reset selected letter
	SendDlgItemMessage(hMainWnd,IDC_LTRFREQ,LB_SETCURSEL,cur_sel,0);

	//total difference
	SetDlgItemInt(hMainWnd,IDC_DIFF,total_diff,0);

	//vowel percentage
	act_vowel=float(lprgiActFreq[0]+lprgiActFreq[4]+lprgiActFreq[8]+lprgiActFreq[14]+lprgiActFreq[20]);
	act_vowel=(100*act_vowel)/iCipherLength;
	exp_vowel=message.cur_map.GetUnigraph(0);
	exp_vowel+=message.cur_map.GetUnigraph(4);
	exp_vowel+=message.cur_map.GetUnigraph(8);
	exp_vowel+=message.cur_map.GetUnigraph(14);
	exp_vowel+=message.cur_map.GetUnigraph(20);
	
	sprintf(szText,"%.2f%% / %.2f%%",act_vowel,exp_vowel);
	SetDlgItemText(hMainWnd,IDC_VOWEL,szText);
}

//refresh solver info
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

//call when key is changed to decode and display plain text
void SetPlain()
{
	szPlain=message.GetPlain();
	
	SetText();
}

void SetDlgInfo()
{
	if(!bMsgLoaded) return;
	
	//set key to hillclimber best if running
	if(siSolveInfo.running)
		message.cur_map.FromKey(siSolveInfo.best_key);
		
	SetPlain();
	SetKey();
	SetFreq();
	SetSolve();
}

//call when the cipher is changed, i.e. symbol merge
void SetCipher()
{	
	sprintf(szText,"Cipher Text (Strength: %.2f)",message.GetStrength());
	SetDlgItemText(hMainWnd,IDC_CIPHER_TITLE,szText);

	iSymbols=message.cur_map.GetNumSymbols();
	szCipher=message.GetCipher();
	
	SetPatterns();
	SetDlgInfo();
}
