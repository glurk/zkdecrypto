inline int CommandFile(int cmd_id)
{
	char filename[1024];

	switch(cmd_id)
	{
		case IDM_FILE_OPEN_ASC:
			if(!GetFilename(filename,szCipherName,0)) return 0;
			LoadMessage(filename,0);				
			return 0;

		case IDM_FILE_OPEN_NUM:
			if(!GetFilename(filename,szCipherName,0)) return 0;
            LoadMessage(filename,1);			
			return 0;
			
		case IDM_FILE_SAVE_CIPHER:
			if(!bMsgLoaded) return 0;
			if(GetFilename(filename,szCipherName,1)!=1) return 0;
			if(message.Write(filename)) strcpy(szCipherName,filename);
			else
			{
				sprintf(szText,"Could not save \"%s\"",filename);
				MessageBox(hMainWnd,szText,"Error",MB_ICONEXCLAMATION);
			}
			return 0;
			
		case IDM_FILE_OPEN_MAP:
			if(!GetFilename(filename,szKeyName,0)) return 0;
			LoadMap(filename);
			return 0;

		case IDM_FILE_SAVE_MAP:
			if(!bMsgLoaded) return 0;
			if(GetFilename(filename,szKeyName,1)!=1) return 0;
			if(message.cur_map.Write(filename)) 
			{
				strcpy(szKeyName,filename);
				bMapLoaded=true;
				MapEnable(true);
			}
			else
			{
				sprintf(szText,"Could not save \"%s\"",filename);
				MessageBox(hMainWnd,szText,"Error",MB_ICONEXCLAMATION);
			}
			return 0;

		case IDM_FILE_SAVE_PLAIN:
			if(!bMsgLoaded) return 0;
			if(GetFilename(filename,szPlainName,2)!=1) return 0;
			if(SavePlain(filename)) strcpy(szPlainName,filename);
			else
			{
				sprintf(szText,"Could not save \"%s\"",filename);
				MessageBox(hMainWnd,szText,"Error",MB_ICONEXCLAMATION);
			}
			return 0;
			
		case IDM_FILE_COPY_PLAIN:
			if(!bMsgLoaded) return 0;
			SavePlain(NULL);
			return 0;

		case IDM_FILE_EXIT:
			SendMessage(hMainWnd,WM_CLOSE,0,0);
			return 0;
	}

	return 0;
}

inline int CommandEdit(int cmd_id)
{
	switch(cmd_id)
	{
		case IDM_EDIT_UNDO: Undo(); SetCipher(); return 0;
		case IDM_EDIT_REDO: Redo(); SetCipher(); return 0;
		case IDM_EDIT_MSG: if(bMsgLoaded) OpenWith(szCipherName); return 0;
		case IDM_EDIT_MAP: if(bMsgLoaded) OpenWith(szKeyName); return 0;
	}

	return 0;
}

inline int CommandCipher(int cmd_id)
{
	int new_pat, trans, cur_symbol;
	SYMBOL symbol;
	char *symbols;

	switch(cmd_id)
	{
		case IDM_CIPHER_MERGE:
			DialogBox(hInst,MAKEINTRESOURCE(IDD_MERGE),hMainWnd,(DLGPROC)MergeProc);
			return 0;
					
		case IDM_CIPHER_SIMPLIFY:
			//run simplify
			SetCursor(LoadCursor(0,IDC_WAIT));
			UpdateWindow(hMainWnd);
			new_pat=message.Simplify(szText);
			SetClipboardText(szText);
			UpdateWindow(hMainWnd);
			SetCursor(LoadCursor(0,IDC_ARROW));

			//no good substitution found
			if(!new_pat) MessageBox(hMainWnd,"No substitutions found","Pattern Analysis Status",MB_ICONINFORMATION);
			else
			{
				strcpy(szGraphTitle,"Pattern Analysis");
				ustrcpy(szGraph,szText);
				DialogBox(hInst,MAKEINTRESOURCE(IDD_GRAPHS_R),hMainWnd,(DLGPROC)Graphs_R_Proc);
			}
			return 0;

		case IDM_CIPHER_COPY_SYMBOLS:
			symbols=new char[message.cur_map.GetNumSymbols()+1];

			for(cur_symbol=0; cur_symbol<message.cur_map.GetNumSymbols(); cur_symbol++)
			{
				message.cur_map.GetSymbol(cur_symbol,&symbol);
				symbols[cur_symbol]=symbol.cipher;
			}

			symbols[cur_symbol]='\0';
			SetClipboardText(symbols);
			delete symbols;
			return 0;

		case IDM_CIPHER_COL_LEFT:
			if(!bMsgLoaded || iColSel==-1) return 0;
			if(--iColSel<0) iColSel=iLineChars-1;
			message.SwapColumns(iColSel,iColSel+1,iLineChars);
			SetTextSel(iTextSel-1);
			message.FindPatterns(1);
			SetText();
			SetPatterns();
			return 0;

		case IDM_CIPHER_COL_RIGHT:
			if(!bMsgLoaded || iColSel==-1) return 0;
			message.SwapColumns(iColSel,iColSel+1,iLineChars);
			SetTextSel(iTextSel+1);
			message.FindPatterns(1);
			SetText();
			SetPatterns();
			return 0;

		case IDM_CIPHER_ROW_UP:
			if(!bMsgLoaded || iRowSel==-1) return 0;
			if(--iRowSel<0) iRowSel=iLines-1;
			message.SwapRows(iRowSel,iRowSel+1,iLineChars);
			SetTextSel(iTextSel-iLineChars);
			message.FindPatterns(1);
			SetText();
			SetPatterns();
			return 0;

		case IDM_CIPHER_ROW_DOWN:
			if(!bMsgLoaded || iRowSel==-1) return 0;
			message.SwapRows(iRowSel,iRowSel+1,iLineChars);
			SetTextSel(iTextSel+iLineChars);
			message.FindPatterns(1);
			SetText();
			SetPatterns();
			return 0;

		case IDM_CIPHER_RAND_TRANS:
			
			//message.FindPatterns(false);
			new_pat=message.GetNumPatterns();

			for(trans=0; message.GetNumPatterns()<=new_pat && trans<100000; trans++)
			{
				message.SwapColumns(rand()%iLineChars,rand()%iLineChars,iLineChars);
				message.FindPatterns(true);
			}
			SetText();
			SetPatterns();
			return 0;

		case IDM_CIPHER_POLYIC:
			strcpy(szNumberTitle,"Max Key Length");
			iNumber=25;
			if(iNumber>message.GetLength()) iNumber=message.GetLength()-1;
			if(DialogBox(hInst,MAKEINTRESOURCE(IDD_NUMBER),hMainWnd,(DLGPROC)NumberProc))
			{
				if(iNumber<2) iNumber=2;
				lRowCol=message.PolyKeySize(szGraph,iNumber,siSolveInfo.lang_ioc);
				strcpy(szGraphTitle,"Polyalphabetic IoC Count");
				if(lRowCol)	DialogBox(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
			}
			return 0;

		case IDM_CIPHER_RC_IOC:
			if(iLineChars>message.GetLength()) return 0;
			lRowCol=message.RowColIoC(szGraph,iLineChars);
			strcpy(szGraphTitle,"Row & Column IoC Count");
			if(lRowCol) DialogBox(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
			return 0;

		case IDM_CIPHER_RANDOM: message.DecodeElgar(); SetDlgInfo(); return 0;
			
		case IDM_CIPHER_SEQHOMO:
			hHomo=CreateDialog(hInst,MAKEINTRESOURCE(IDD_SEQHOMO),hMainWnd,(DLGPROC)HomoProc);
			ShowWindow(hHomo,SW_SHOWNORMAL);
			return 0;

		case IDM_CIPHER_HORZ: 
			SetUndo();
			message.Flip(1,iLineChars);
			SetDlgInfo();
			SetPatterns();
			return 0;

		case IDM_CIPHER_VERT:
			SetUndo();
			message.Flip(2,iLineChars);
			SetDlgInfo();
			SetPatterns();
			return 0;

		case IDM_CIPHER_REV:
			SetUndo();
			message.Flip(3,iLineChars);
			SetDlgInfo();
			SetPatterns();
			return 0;

		case IDM_CIPHER_ROT_LEFT:
			if(message.Rotate(iLineChars,0)) {
				SetUndo();
				iLineChars=iLines;
				iLines=message.GetLength()/iLineChars;
				SetPatterns();
				ClearTextAreas();
				SetText();
				SetDlgInfo();
				SetPlain();
			}
			return 0;

		case IDM_CIPHER_ROT_RIGHT:
			if(message.Rotate(iLineChars,1)) {
				SetUndo();
				iLineChars=iLines;
				iLines=message.GetLength()/iLineChars;
				SetPatterns();
				ClearTextAreas();
				SetText();
				SetDlgInfo();
				SetPlain();
			}
			return 0;
	}

	return 0;
}

inline int CommandKey(int cmd_id)
{
	int swap, num_symbols, cur_sym;
	SYMBOL symbol;

	switch(cmd_id)
	{
		case IDM_KEY_INIT:
			if(DialogBox(hInst,MAKEINTRESOURCE(IDD_INITKEY),hMainWnd,(DLGPROC)InitProc))
			{
				SetUndo();
				siSolveInfo.best_key[0]='\0';
				message.cur_map.Init(lprgiInitKey);
				SetDlgInfo();
			}
			
			return 0;

		case IDM_KEY_SCRAMBLE:
			SetUndo();
			num_symbols=message.cur_map.GetNumSymbols();
			for(swap=0; swap<50000; swap++)
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
		case IDM_KEY_INVERT_LOCK: 
			num_symbols=message.cur_map.GetNumSymbols();
			for(cur_sym=0; cur_sym<num_symbols; cur_sym++)
				message.cur_map.SetLock(cur_sym,!message.cur_map.GetLock(cur_sym));
			SetDlgInfo();
			return 0;

		case IDM_KEY_EXCLUDE:
			if(iCurSymbol<0) return 0;

			message.cur_map.GetSymbol(iCurSymbol,&symbol);
			sprintf(szStringTitle,"Exclude Letters");
			strcpy(szString,symbol.exclude);
			
			if(DialogBox(hInst,MAKEINTRESOURCE(IDD_STRING),hMainWnd,(DLGPROC)StringProc))
			{
				GetUniques(szString,szText,NULL);
				strcpy(symbol.exclude,szText);
				message.cur_map.AddSymbol(symbol,false);
			}
			return 0;
			
		case IDM_KEY_CLEAR_EXCLUDE: message.cur_map.Clear(CLR_EXCLUDE); return 0;
	}

	return 0;
}

inline int CommandSolve(int cmd_id)
{
	switch(cmd_id)
	{
		case IDM_SOLVE_WORD:
			strcpy(szStringTitle,"Text to Plug");
			szString[0]='\0';
			if(DialogBox(hInst,MAKEINTRESOURCE(IDD_STRING),hMainWnd,(DLGPROC)StringProc))
			{
				SetUndo();

				iBestScore=WordPlug(message,szString,siSolveInfo);
				SetDlgInfo();
			}
			return 0;

		case IDM_SOLVE_INSERT:
			if(iCurSymbol<0) return 0;
			
			strcpy(szStringTitle,"Text to Insert");
			szString[0]='\0';
			if(DialogBox(hInst,MAKEINTRESOURCE(IDD_STRING),hMainWnd,(DLGPROC)StringProc))
			{
				SetUndo();

				message.Insert(iTextSel,szString);
				SetDlgInfo();
			}
			return 0;
			
		case IDM_SOLVE_RESET:
			//blank best key, so that additional chars are renewed
			siSolveInfo.best_key[0]='\0';
			return 0;
			
		case IDM_SOLVE_BRUTE:
			if(iBruteSymbols) return 0;
			
			strcpy(szNumberTitle,"# of Symbols");
			iNumber=1;
			
			if(DialogBox(hInst,MAKEINTRESOURCE(IDD_NUMBER),hMainWnd,(DLGPROC)NumberProc))
			{
				iBruteSymbols=iNumber;
				BruteStart();
			}
			return 0;
			
		case IDM_SOLVE_COPY_BEST: BatchBest(); return 0;

		case IDM_SOLVE_OPTIONS:
			DialogBox(hInst,MAKEINTRESOURCE(IDD_OPTIONS),hMainWnd,(DLGPROC)OptionsProc);
			return 0;

		case IDM_SOLVE_TP_IDLE: SetPriority(4); return 0;
		case IDM_SOLVE_TP_HIGH: SetPriority(3); return 0;	
		case IDM_SOLVE_TP_NORM: SetPriority(2); return 0;	
		case IDM_SOLVE_TP_LOW: SetPriority(1); return 0;
			
		case IDM_SOLVE_TRIFID:
			DialogBox(hInst,MAKEINTRESOURCE(IDD_TRIFID),hMainWnd,(DLGPROC)TrifidProc);
			return 0;
	}

	return 0;
}

inline int CommandView(int cmd_id)
{
	switch(cmd_id)
	{
		case IDM_VIEW_DESELECT:
			iCurPat=-1;
			iCurSymbol=-1;
			iCurWord=-1;
			iTextSel=-1;
			iRowSel=-1;
			iColSel=-1;
			SendDlgItemMessage(hMainWnd,IDC_PATTERNS,LB_SETCURSEL,iCurPat,0);
			SendDlgItemMessage(hMainWnd,IDC_MAP,LB_SETCURSEL,iCurSymbol,0);
			SendDlgItemMessage(hMainWnd,IDC_WORD_LIST,LB_SETCURSEL,iCurWord,0);
			SetText();
			return 0;

		case IDM_VIEW_LOCK_WORD: LockWord(true); SetKey(); return 0;
		case IDM_VIEW_UNLOCK_WORD: LockWord(false); SetKey(); return 0;

		case IDM_VIEW_SYMGRAPH:
			lRowCol=message.cur_map.SymbolGraph(szGraph);
			strcpy(szGraphTitle,"Symbol Frequencies");
			DialogBox(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
			return 0;
			
		case IDM_VIEW_LTRGRAPH:
			if(message.GetLength() < 5) return 0;
			lRowCol=message.LetterGraph(szGraph);
			strcpy(szGraphTitle,"Letter Frequencies");
			hLetter=CreateDialog(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
			ShowWindow(hLetter,SW_SHOWNORMAL);
			return 0;

		case IDM_VIEW_MERGE_LOG:
			lRowCol=message.cur_map.GetMergeLog(szGraph);
			strcpy(szGraphTitle,"Merge Log");
			if(lRowCol)	DialogBox(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
			else MessageBox(hMainWnd,"No symbols have yet been merged.","Merge Log Status",MB_ICONINFORMATION);
			return 0;
			
		case IDM_VIEW_EXCLUSIONS:
			lRowCol=message.cur_map.GetExclusions(szGraph,1);
			strcpy(szGraphTitle,"Letter Exclusions");
			if(lRowCol) DialogBox(hInst,MAKEINTRESOURCE(IDD_GRAPHS_R),hMainWnd,(DLGPROC)Graphs_R_Proc);
			else MessageBox(hMainWnd,"No letters have yet been excluded.","Letter Exclusion Status",MB_ICONINFORMATION);
			return 0;

		case IDM_VIEW_BYSTRING: SetSort(0); return 0;
		case IDM_VIEW_BYFREQ: SetSort(1); return 0;

		case IDM_TEXT_SEL_LEFT:  if(!bMsgLoaded) return 0; SetTextSel(iTextSel-1); return 0;
		case IDM_TEXT_SEL_RIGHT: if(!bMsgLoaded) return 0; SetTextSel(iTextSel+1); return 0;
		case IDM_TEXT_SEL_UP:	 if(!bMsgLoaded) return 0; SetTextSel(iTextSel-iLineChars); return 0;
		case IDM_TEXT_SEL_DOWN:  if(!bMsgLoaded) return 0; SetTextSel(iTextSel+iLineChars); return 0;


	}

	return 0;
}
