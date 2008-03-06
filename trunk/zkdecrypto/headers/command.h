inline int CommandFile(int cmd_id)
{
	char filename[1024], num2asc_name[1024], *szBase, *szClipboard;
	HGLOBAL hgClipboard;

	switch(cmd_id)
	{
		case IDM_FILE_OPEN_NUM:
			if(!GetFilename(filename,szCipherName,0)) return 0;
			strcpy(num2asc_name,filename);

			//append suffix to file name
			szBase=strrchr(num2asc_name,'.');
			if(szBase) strcpy(szBase,".ascii.txt");
			else strcat(szCipherName,".ascii.txt");

			//convert and load
			Num2Asc(filename,num2asc_name);
			LoadMessage(num2asc_name);				
			return 0;

		case IDM_FILE_OPEN_ASC:
			if(!GetFilename(filename,szCipherName,0)) return 0;
			LoadMessage(filename);				
			return 0;

		case IDM_FILE_OPEN_MAP:
			if(!GetFilename(filename,szKeyName,0)) return 0;
			LoadMap(filename);
			return 0;

		case IDM_FILE_SAVE_MAP:
			if(!bMsgLoaded) return 0;
			if(GetFilename(filename,szKeyName,1)!=1) return 0;
			if(message.cur_map.Write(filename))
				strcpy(szKeyName,filename);
						
			bMapLoaded=true;
			MapEnable(true);
			return 0;

		case IDM_FILE_SAVE_PLAIN:
			if(!bMsgLoaded) return 0;
			if(GetFilename(filename,szCipherName,1)!=1) return 0;
			if(!SavePlain(filename))
			{
				sprintf(szText,"Could not save \"%s\"",filename);
				MessageBox(hMainWnd,szText,"Error",MB_ICONEXCLAMATION);
			}
			return 0;
			case IDM_FILE_COPY_PLAIN:
			if(!bMsgLoaded) return 0;
				//allocate clipboard data
			hgClipboard=GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE,strlen(szPlain)+1);
			szClipboard=(char*)GlobalLock(hgClipboard);
			strcpy(szClipboard,szPlain);
			GlobalUnlock(hgClipboard);

			//set clipboard
			OpenClipboard(hMainWnd);
			EmptyClipboard();
			SetClipboardData(CF_TEXT,(void*)hgClipboard);
			CloseClipboard();
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
		case IDM_EDIT_MSG: if(bMsgLoaded) OpenWith(szCipherName); return 0;
		case IDM_EDIT_MAP: if(bMsgLoaded) OpenWith(szKeyName); return 0;
	}
}

inline int CommandCipher(int cmd_id)
{
	long time1,time2;
	char simp1,simp2;
	int new_pat;

	switch(cmd_id)
	{
		case IDM_CIPHER_MERGE:
			SetUndo();
			DialogBox(hInst,MAKEINTRESOURCE(IDD_MERGE),hMainWnd,(DLGPROC)MergeProc);
			return 0;
					
		case IDM_CIPHER_SIMPLIFY:
			//run simplify
			SetCursor(LoadCursor(0,IDC_WAIT));
			time1=GetTickCount();
			new_pat=message.Simplify(simp1,simp2);
			time2=GetTickCount();
			SetCursor(LoadCursor(0,IDC_ARROW));

			if(simp1==char(0xFF)) //no good substitution found
				return MessageBox(hMainWnd,"No Substitutions found","Substitution",MB_OK);

			sprintf(szText,"Merge '%c' with '%c'? (%.2fs)",simp1,simp2,double(time2-time1)/1000);
					
			//merge if yes
			if(MessageBox(hMainWnd,szText,"Substitution",MB_YESNO | MB_ICONQUESTION)==IDYES)
			{
				SetUndo();
				message.MergeSymbols(simp1,simp2,true);
				SetCipher();
				SetDlgInfo();
			}
			return 0;

		case IDM_CIPHER_BESTSEC:
			BestSection();
			return 0;

		case IDM_CIPHER_POLYIC:
			strcpy(szNumberTitle,"Max Key Length");
			iNumber=25;
			if(DialogBox(hInst,MAKEINTRESOURCE(IDD_NUMBER),hMainWnd,(DLGPROC)NumberProc))
			{
				lRowCol=message.PolyKeySize(szGraph,iNumber);
				strcpy(szGraphTitle,"Polyalphabetic IoC Count");
				DialogBox(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
			}
			return 0;

		case IDM_CIPHER_RC_IOC:
			lRowCol=message.RowColIoC(szGraph,iLineChars);
			strcpy(szGraphTitle,"Row & Column IoC Count");
			DialogBox(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
			return 0;
			
		case IDM_CIPHER_NGRAPHS:
			sprintf(szText,"%s%s",szExeDir,"bigraphs.txt");
			message.PatternsToFile(szText,2);
			sprintf(szText,"%s%s",szExeDir,"trigraphs.txt");
			message.PatternsToFile(szText,3);
			sprintf(szText,"%s%s",szExeDir,"tetragraphs.txt");
			message.PatternsToFile(szText,4);
			sprintf(szText,"%s%s",szExeDir,"pentagraphs.txt");
			message.PatternsToFile(szText,5);
			return 0;

		case IDM_CIPHER_RANDOM:
			//RandCipher(340,63);
			message.SeqHomo(szText);
			MessageBox(hMainWnd,szText,"SeqHomo",MB_OK);
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
	}

	return 0;
}

inline int CommandKey(int cmd_id)
{
	int swap, num_symbols;
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

		case IDM_KEY_EXCLUDE:
			if(iCurSymbol<0) return 0;

			message.cur_map.GetSymbol(iCurSymbol,&symbol);
			sprintf(szStringTitle,"Exclude Letters for '%c'",symbol.cipher);
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
			strcpy(szStringTitle,"Word to Insert");
			szString[0]='\0';
			if(DialogBox(hInst,MAKEINTRESOURCE(IDD_STRING),hMainWnd,(DLGPROC)StringProc))
			{
				SetUndo();
				iBestScore=WordPlug(message,szString,iUseGraphs);
				SetDlgInfo();
			}
			return 0;

		case IDM_SOLVE_OPTIONS:
			DialogBox(hInst,MAKEINTRESOURCE(IDD_OPTIONS),hMainWnd,(DLGPROC)OptionsProc);
			return 0;

		case IDM_SOLVE_TP_IDLE: SetPriority(4); return 0;
		case IDM_SOLVE_TP_HIGH: SetPriority(3); return 0;	
		case IDM_SOLVE_TP_NORM: SetPriority(2); return 0;	
		case IDM_SOLVE_TP_LOW: SetPriority(1); return 0;			
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
			SetDlgItemText(hTextWnd,IDC_TEXTINFO,"");
			return 0;

		case IDM_VIEW_LOCK_WORD: LockWord(true); SetKey(); return 0;
		case IDM_VIEW_UNLOCK_WORD: LockWord(false); SetKey(); return 0;

		case IDM_VIEW_SYMGRAPH:
			lRowCol=message.cur_map.SymbolGraph(szGraph);
			strcpy(szGraphTitle,"Symbol Frequencies");
			DialogBox(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
			return 0;
			
		case IDM_VIEW_LTRGRAPH:
			if(hGraph) SendMessage(hGraph,WM_CLOSE,0,0);
			lRowCol=message.LetterGraph(szGraph);
			strcpy(szGraphTitle,"Letter Frequencies");
			hGraph=CreateDialog(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
			ShowWindow(hGraph,SW_SHOWNORMAL);
			return 0;

		case IDM_VIEW_MERGE_LOG:
			lRowCol=message.cur_map.GetMergeLog(szGraph);
			strcpy(szGraphTitle,"Merge Log");
			DialogBox(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
			return 0;
			
		case IDM_VIEW_EXCLUSIONS:
			lRowCol=message.cur_map.GetExclusions(szGraph,4);
			strcpy(szGraphTitle,"Letter Exclusions");
			DialogBox(hInst,MAKEINTRESOURCE(IDD_GRAPHS),hMainWnd,(DLGPROC)GraphsProc);
			//MessageBox(hMainWnd,szText,"Exclusions",MB_OK);
			return 0;

		case IDM_VIEW_BYSTRING: SetSort(0); return 0;
		case IDM_VIEW_BYFREQ: SetSort(1); return 0;
	}

	return 0;
}
