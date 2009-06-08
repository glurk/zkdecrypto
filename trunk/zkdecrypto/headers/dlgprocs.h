/*Window Functions*/

//merge dialog
LRESULT CALLBACK MergeProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int num_symbols, cur_symbol;
	char cipher1, cipher2;
	long lfHeight;
	SYMBOL symbol;
    HDC hdc;

	hdc = GetDC(NULL);
    lfHeight = -MulDiv(14, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(NULL, hdc);

    hTempFont = CreateFont(lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "ZKDfont");
		
	switch(iMsg)
	{
		case WM_INITDIALOG:
			num_symbols=message.cur_map.GetNumSymbols();
			SendDlgItemMessage(hWnd,IDC_MERSYM1, WM_SETFONT, (WPARAM)hTempFont, TRUE);
			SendDlgItemMessage(hWnd,IDC_MERSYM2, WM_SETFONT, (WPARAM)hTempFont, TRUE);
			for(cur_symbol=0; cur_symbol<num_symbols; cur_symbol++)
			{
				message.cur_map.GetSymbol(cur_symbol,&symbol);
				sprintf(szText,"%c",symbol.cipher);
				SendDlgItemMessage(hWnd,IDC_MERSYM1,CB_ADDSTRING,0,(LPARAM)szText);
				SendDlgItemMessage(hWnd,IDC_MERSYM2,CB_ADDSTRING,0,(LPARAM)szText);

			}

			SetFocus(GetDlgItem(hWnd,IDC_MERSYM1));
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					//get 1st symbol cipher
					cur_symbol=SendDlgItemMessage(hWnd,IDC_MERSYM1,CB_GETCURSEL,0,0);
					message.cur_map.GetSymbol(cur_symbol,&symbol);
					cipher1=symbol.cipher;
					
					//get 2nd symbol cipher
					cur_symbol=SendDlgItemMessage(hWnd,IDC_MERSYM2,CB_GETCURSEL,0,0);
					message.cur_map.GetSymbol(cur_symbol,&symbol);
					cipher2=symbol.cipher;
					
					SetUndo();
					message.MergeSymbols(cipher1,cipher2,true);
					
					SetCipher();
					
					EndDialog(hWnd,0);
					return 1;

				case IDCANCEL: DeleteObject(hTempFont); EndDialog(hWnd,0); return 0;
			}
	}
	return 0;
}

//graphs_r dialog
LRESULT CALLBACK Graphs_R_Proc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg)
	{
		case WM_INITDIALOG:
			SetDlgItemTextW(hWnd,IDC_GRAPHS_R_SETS,(WCHAR*)szGraph);
			SetWindowText(hWnd,szGraphTitle);
			//if(strlen(szGraphTitle)==16) MoveWindow(hWnd,0,0,100,100,TRUE);
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL: EndDialog(hWnd,0); if(hLetter) hLetter=NULL; return 0;
			}
	}

	return 0;
}


//graphs dialog
LRESULT CALLBACK GraphsProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int iWidth, iHeight;

	switch(iMsg)
	{
		case WM_INITDIALOG:	
			iWidth=LOWORD(lRowCol)*(CHAR_WIDTH+1);
			iHeight=HIWORD(lRowCol)*(CHAR_HEIGHT+2)+20;
			SetWindowPos(GetDlgItem(hWnd,IDC_GRAPH),0,0,0,iWidth,iHeight,SWP_NOREPOSITION | SWP_NOMOVE);
			SetWindowPos(hWnd,0,0,0,iWidth+(iMargin<<1),iHeight+(iMargin<<1),SWP_NOREPOSITION | SWP_NOMOVE);
			SetDlgItemTextW(hWnd,IDC_GRAPH,(WCHAR*)szGraph);
			SetWindowText(hWnd,szGraphTitle);
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL: EndDialog(hWnd,0); 	if(hLetter) hLetter=NULL; return 0;
			}
	}

	return 0;
}

//number dialog
LRESULT CALLBACK NumberProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg)
	{
		case WM_INITDIALOG:
			SetFocus(GetDlgItem(hWnd,IDC_NUMBER));
			SendDlgItemMessage(hWnd,IDC_NUMBER,EM_LIMITTEXT,3,0);
			SetDlgItemInt(hWnd,IDC_NUMBER,iNumber,0);
			SetWindowText(hWnd,szNumberTitle);
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					iNumber=GetDlgItemInt(hWnd,IDC_NUMBER,0,0);
					EndDialog(hWnd,1);
					return 1;

				case IDCANCEL: EndDialog(hWnd,0); return 0;
			}
	}

	return 0;
}

//word plug dialog
LRESULT CALLBACK StringProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int iLength, bChange;

	switch(iMsg)
	{
		case WM_INITDIALOG:
			SetFocus(GetDlgItem(hWnd,IDC_STRING));
			SendDlgItemMessage(hWnd,IDC_STRING,EM_LIMITTEXT,127,0);
			SetDlgItemText(hWnd,IDC_STRING,szString);
			SetWindowText(hWnd,szStringTitle);
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_STRING: 
					if(HIWORD(wParam)==EN_CHANGE)
					{
						GetDlgItemText(hWnd,IDC_STRING,szString,128);
						iLength=(int)strlen(szString);

						bChange=false;
						
						//remove spaces from string
						for(int iChar=0; iChar<iLength; iChar++)
							if(szString[iChar]==' ')
							{
								memmove(szString+iChar,szString+iChar+1,iLength-iChar);
								iChar--; iLength--;
								bChange=true;
							}
						
						//reset string
						if(bChange)
						{
							SetDlgItemText(hWnd,IDC_STRING,szString);
							SendDlgItemMessage(hWnd,IDC_STRING,EM_SETSEL,iLength,iLength);
						}
					}
					return 0;

				case IDOK:
					GetDlgItemText(hWnd,IDC_STRING,szString,128);
					EndDialog(hWnd,1);
					return 1;

				case IDCANCEL: EndDialog(hWnd,0); return 0;
			}
	}

	return 0;
}

//options dialog
LRESULT CALLBACK OptionsProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int iPrevLang, iPrevSolveType;
//	int iMaxKeyLen;

	switch(iMsg)
	{
		case WM_INITDIALOG: //init values
			//hillclimber parameters
			SetDlgItemInt(hWnd,IDC_MAXFAIL,siSolveInfo.max_tabu,0);
			SetDlgItemInt(hWnd,IDC_SWAPS,siSolveInfo.swaps,0);
			SetDlgItemInt(hWnd,IDC_REVERT,siSolveInfo.max_tol,0);

			//language		
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"English");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"Spanish");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"German");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"Italian");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"French");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_SETCURSEL,iLang,0);

			//solve type		
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Monographic Substitution");
			//SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Digraphic Substitution");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Playfair");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Double Playfair");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Vigenere/Quagmire3");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Dictionary Vigenere/Quagmire3");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Running Key");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Bifid");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Trifid");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Permutation");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Columar Transposition");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Double Columnar Transposition");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Triple Columnar Transposition");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"ADFGX");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"ADFGVX");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"CEMOPRTU");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Substitution + Permutation");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Substitution + Double Transposition");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_ADDSTRING,0,(LPARAM)"Vigenere + Double Transposition");
			SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_SETCURSEL,iSolveType,0);

			//word length
			SetDlgItemInt(hWnd,IDC_WORD_MIN,iWordMin,false);
			SetDlgItemInt(hWnd,IDC_WORD_MAX,iWordMax,false);

			//tableu alphabet
			SendDlgItemMessage(hWnd,IDC_TABLEU_EDIT,EM_LIMITTEXT,26,0);
			SetDlgItemText(hWnd,IDC_TABLEU_EDIT,message.GetTableuAlphabet());	

			//extra letters
			SendDlgItemMessage(hWnd,IDC_EXTRA_LTR,EM_LIMITTEXT,MAX_EXTRA,0);
			SetDlgItemText(hWnd,IDC_EXTRA_LTR,szExtraLtr);

			//transposition type
			if(message.GetTransType()) SetDlgItemText(hWnd,IDC_TRANS_TYPE,"Bottom Up");
			else SetDlgItemText(hWnd,IDC_TRANS_TYPE,"Top Down");

			if(siSolveInfo.running) //disable options that should not be changed when running
			{
				EnableWindow(GetDlgItem(hWnd,IDC_SOLVE_TYPE),false);
				EnableWindow(GetDlgItem(hWnd,IDC_LANG),false);
			}

			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_EXTRA_ALPHABET: SetDlgItemText(hWnd,IDC_EXTRA_LTR,"ABCDEFGHIJKLMNOPQRSTUVWXYZ"); return 0;
				case IDC_EXTRA_ALPHABET_C: 	SetDlgItemText(hWnd,IDC_EXTRA_LTR,""); return 0;

				case IDC_TRANS_TYPE:
					message.SetTransType(!message.GetTransType());
					if(message.GetTransType()) SetDlgItemText(hWnd,IDC_TRANS_TYPE,"Bottom Up");
					else SetDlgItemText(hWnd,IDC_TRANS_TYPE,"Top Down");
					return 0;
					
				case IDOK: //get new values
					//hillclimber parameters
					siSolveInfo.max_tabu=GetDlgItemInt(hWnd,IDC_MAXFAIL,0,0);
					siSolveInfo.swaps=GetDlgItemInt(hWnd,IDC_SWAPS,0,0);
					siSolveInfo.max_tol=GetDlgItemInt(hWnd,IDC_REVERT,0,0);

					//language
					iPrevLang=iLang;
					iLang=SendDlgItemMessage(hWnd,IDC_LANG,CB_GETCURSEL,0,0);
					if(iLang!=iPrevLang) SetLanguage();

					GetDlgItemText(hWnd,IDC_TABLEU_EDIT,szText,30); //tableu alphabet	
					message.SetTableuAlphabet(szText);

					GetDlgItemText(hWnd,IDC_EXTRA_LTR,szExtraLtr,MAX_EXTRA); //extra letters

					if(!siSolveInfo.running) //can't change solve time during solve
					{
						iPrevSolveType=iSolveType;
						iSolveType=SendDlgItemMessage(hWnd,IDC_SOLVE_TYPE,CB_GETCURSEL,0,0); //solve type
						if(iSolveType!=iPrevSolveType) SetSolveTypeFeatures();

						if(bMsgLoaded) //update display
						{
							SetScrollBar();					
							SetDlgInfo();
							ClearTextAreas();
							SetPlain();
							SetText();
						}
					}

				case IDCANCEL: EndDialog(hWnd,0); return 0;
			}
	}

	return 0;
}

void LetterDist(int target, HWND hWnd)
{
	int letter, set_symbols=0, max_letter;
	double n[26];

	//calculate real number of occurances for each letter
	for(letter=0; letter<26; letter++)
	{
		n[letter]=(message.cur_map.GetUnigraph(letter)/100)*target;
		set_symbols+=int(n[letter]);
	}

	while(set_symbols<target)
	{
		//find the letter with the highest decimal
		max_letter=0;

		for(letter=0; letter<26; letter++)
			if(DECIMAL(n[letter])>DECIMAL(n[max_letter])) 
				max_letter=letter;
	
		//set that letter to the next whole number
		n[max_letter]=int(n[max_letter])+1;
		set_symbols++;
	}

	if(hWnd)
		for(letter=0; letter<26; letter++)
			SetDlgItemInt(hWnd,lprgiInitID[letter],int(n[letter]),false);
}


//init key dialog
LRESULT CALLBACK InitProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int letter, num_symbols, target, total;

	switch(iMsg)
	{
		case WM_INITDIALOG:
			num_symbols=message.cur_map.GetNumSymbols();
			target=num_symbols;

			SetDlgItemInt(hWnd,IDC_INIT_TARGET_EDIT,target,false);

			//set up/down control ranges
			SendDlgItemMessage(hWnd,IDC_INIT_A_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_B_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_C_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_D_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_E_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_F_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_G_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_H_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_I_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_J_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_K_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_L_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_M_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_N_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_O_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_P_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_Q_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_R_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_S_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_T_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_U_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_V_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_W_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_X_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_Y_SPIN,UDM_SETRANGE,0,num_symbols);
			SendDlgItemMessage(hWnd,IDC_INIT_Z_SPIN,UDM_SETRANGE,0,num_symbols);

			SendDlgItemMessage(hWnd,IDC_INIT_TARGET_SPIN,UDM_SETRANGE,0,num_symbols);

		case UDM_INIT_TARGET:
			target=GetDlgItemInt(hWnd,IDC_INIT_TARGET_EDIT,false,false);			
			LetterDist(target,hWnd);		

		case UDM_INIT_TOTAL:
			num_symbols=message.cur_map.GetNumSymbols();
			target=GetDlgItemInt(hWnd,IDC_INIT_TARGET_EDIT,false,false);
			total=0;

			//count total homophones
			for(letter=0; letter<26; letter++)
				total+=lprgiInitKey[letter]=GetDlgItemInt(hWnd,lprgiInitID[letter],false,false);

			if(total>num_symbols) //reduce last edited letter if too many
			{
				lprgiInitKey[wParam]-=(total-num_symbols);
				SetDlgItemInt(hWnd,lprgiInitID[wParam],lprgiInitKey[wParam],false);
				total=target;
			}			

			//set remaining available homohpones
			SetDlgItemInt(hWnd,IDC_INIT_REM,num_symbols-total,false);
			return 0;
			

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				//homophones for a letter edited, recalculate total
				case IDC_INIT_A_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,0,0); return 0;
				case IDC_INIT_B_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,1,0); return 0;
				case IDC_INIT_C_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,2,0); return 0;
				case IDC_INIT_D_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,3,0); return 0;
				case IDC_INIT_E_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,4,0); return 0;
				case IDC_INIT_F_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,5,0); return 0;
				case IDC_INIT_G_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,6,0); return 0;
				case IDC_INIT_H_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,7,0); return 0;
				case IDC_INIT_I_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,8,0); return 0;
				case IDC_INIT_J_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,9,0); return 0;
				case IDC_INIT_K_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,10,0); return 0;
				case IDC_INIT_L_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,11,0); return 0;
				case IDC_INIT_M_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,12,0); return 0;
				case IDC_INIT_N_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,13,0); return 0;
				case IDC_INIT_O_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,14,0); return 0;
				case IDC_INIT_P_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,15,0); return 0;
				case IDC_INIT_Q_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,16,0); return 0;
				case IDC_INIT_R_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,17,0); return 0;
				case IDC_INIT_S_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,18,0); return 0;
				case IDC_INIT_T_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,19,0); return 0;
				case IDC_INIT_U_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,20,0); return 0;
				case IDC_INIT_V_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,21,0); return 0;
				case IDC_INIT_W_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,22,0); return 0;
				case IDC_INIT_X_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,23,0); return 0;
				case IDC_INIT_Y_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,24,0); return 0;
				case IDC_INIT_Z_EDIT: SendMessage(hWnd,UDM_INIT_TOTAL,25,0); return 0;
				
				case IDC_INIT_TARGET_EDIT: SendMessage(hWnd,UDM_INIT_TARGET,0,0); return 0;
				
				case IDOK: EndDialog(hWnd,1); return 0;				
				case IDCANCEL: EndDialog(hWnd,0); return 0;	
			}				

			return 0;
	}

	return 0;
}

//homophone dialog
LRESULT CALLBACK HomoProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int tolerance=100, max_len=10;
	long lfHeight;
    HDC hdc;

	hdc = GetDC(NULL);
    lfHeight = -MulDiv(14, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(NULL, hdc);

    hTempFont = CreateFont(lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "ZKDfont");

	switch(iMsg)
	{
		case WM_INITDIALOG:
			SendDlgItemMessage(hWnd,IDC_HOMO_SETS, WM_SETFONT, (WPARAM)hTempFont, TRUE);
			//set up/down control ranges
			SendDlgItemMessage(hWnd,IDC_HOMO_TOL_SPIN,UDM_SETRANGE,0,100);
			SendDlgItemMessage(hWnd,IDC_HOMO_LEN_SPIN,UDM_SETRANGE,0,message.cur_map.GetNumSymbols());
			SetDlgItemInt(hWnd,IDC_HOMO_TOL_EDIT,tolerance,false);
			SetDlgItemInt(hWnd,IDC_HOMO_LEN_EDIT,max_len,false);

		case UDM_HOMO_UPDATE:
			tolerance=GetDlgItemInt(hWnd,IDC_HOMO_TOL_EDIT,false,false);
			max_len=GetDlgItemInt(hWnd,IDC_HOMO_LEN_EDIT,false,false);			
			message.SeqHomo(szGraph,szText,tolerance/100.0,max_len);
			SetDlgItemTextW(hWnd,IDC_HOMO_SETS,(WCHAR*)szGraph);
			SetClipboardText(szText);
			return 0;			

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_HOMO_TOL_EDIT: SendMessage(hWnd,UDM_HOMO_UPDATE,0,0); return 0;
				case IDC_HOMO_LEN_EDIT: SendMessage(hWnd,UDM_HOMO_UPDATE,0,0); return 0;
							
				case IDCANCEL: EndDialog(hWnd,0); hHomoWnd=NULL; return 0;
			}				
			return 0;

		case WM_DESTROY: DeleteObject(hTempFont); return 0;
	}
	return 0;
}

inline int MatchStringTemplate(std::string sTemp, std::string sWord)
{
	int length=sTemp.length();

	if(length!=sWord.length()) return 0;

	for(int i=0; i<length; i++)
	{
		if(sTemp.at(i)=='*') continue; //wild card
		
		else if(sTemp.at(i)>='0' && sTemp.at(i)<='9') //pattern number
			for(int j=0; j<length; j++) //compare word letter at each position
			{
				if(sTemp.at(j)<'0' || sTemp.at(j)>'9') continue; //not a pattern number
				if(sTemp.at(j)==sTemp.at(i) && sWord.at(j)!=sWord.at(i)) return 0; //same pattern number, different word letter
				if(sTemp.at(j)!=sTemp.at(i) && sWord.at(j)==sWord.at(i)) return 0; //different pattern number, same word letter
			}

		else if(sTemp.at(i)!=sWord.at(i)) return 0; //exact letter	
	}

	return 1;
}

//Trifid Decoding
LRESULT CALLBACK WordFindProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
//	int iWordLen;
	long lfHeight;
	std::string word_str;
	STRMAP::iterator iter;

	HDC hdc=GetDC(NULL);
    lfHeight=-MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(NULL, hdc);

    hTempFont=CreateFont(lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "ZKDfont");

	switch(iMsg)
	{
		case WM_INITDIALOG:
			SetFocus(GetDlgItem(hWnd,IDC_WORD_FIND));
			SendDlgItemMessage(hWnd,IDC_WORD_LIST, WM_SETFONT, (WPARAM)hTempFont, TRUE);
			return 0;

		case UDM_HOMO_UPDATE:

			SendDlgItemMessage(hWnd,IDC_WORD_LIST,LB_RESETCONTENT,0,0);
			GetDlgItemText(hWnd,IDC_WORD_FIND,szText,30);
			strupr(szText);
			word_str=szText;

			for(iter=dictionary.begin(); iter!=dictionary.end(); ++iter) 
			{
				if(MatchStringTemplate(word_str,iter->first))
					SendDlgItemMessage(hWnd,IDC_WORD_LIST,LB_ADDSTRING,0, (WPARAM)std::string(iter->first).data() );
			}
			
			return 0;			

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_WORD_FIND: SendMessage(hWnd,UDM_HOMO_UPDATE,0,0); return 0;				
				case IDCANCEL: EndDialog(hWnd,0); hWordWnd=NULL; return 0;
			}				
			return 0;

		case WM_DESTROY: DeleteObject(hTempFont); return 0;
	}
	
	return 0;
}

//about dialog
LRESULT CALLBACK AboutProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg)
	{
		case WM_INITDIALOG:
			sprintf(szText,"%s %s",PROG_NAME,PROG_VER);
			SetDlgItemText(hWnd,IDC_PROG,szText);
			
			strcpy(szText,"Wesley Hopper (hopperw2000@yahoo.com)\r\n\r\n");
			strcat(szText,"Brax Sisco (brax_sisco@hotmail.com)\r\n\r\n");
			strcat(szText,"Michael Eaton (michaeleaton@gmail.com)\r\n\r\n");

			SetDlgItemText(hWnd,IDC_ABOUT,szText);
			return 0;

		case WM_LBUTTONDOWN: EndDialog(hWnd,0); return 0;
	}

	return 0;
}

LRESULT CALLBACK TextProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	switch(iMsg)
	{
		//click on text
		case WM_LBUTTONDOWN: TextClick(LOWORD(lParam),HIWORD(lParam)); return 0;
		case WM_MBUTTONDOWN: TextClick(LOWORD(lParam),HIWORD(lParam)); ToggleLock(); return 0;
		case WM_RBUTTONDOWN: TextClick(LOWORD(lParam),HIWORD(lParam)); CreateTextMenu(); return 0;
		case WM_PAINT: BeginPaint(hWnd,&ps); SetText();	EndPaint(hWnd,&ps); return 0; //redraw text windows
	}

	return DefWindowProc(hWnd,iMsg,wParam,lParam);
}
