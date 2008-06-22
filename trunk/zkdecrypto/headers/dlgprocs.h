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

				case IDCANCEL:
					DeleteObject(hTempFont);
					EndDialog(hWnd,0);
					return 0;
			}
	}
	return 0;
}

//graphs_r dialog
LRESULT CALLBACK Graphs_rProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int notused;

	switch(iMsg)
	{
		case WM_INITDIALOG:
			SetDlgItemTextW(hWnd,IDC_GRAPHS_R_SETS,(WCHAR*)szGraph);
			SetWindowText(hWnd,szGraphTitle);
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					EndDialog(hWnd,0);
					if(hLetter) hLetter=NULL;
					return 0;
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
			iWidth=LOWORD(lRowCol)*CHAR_WIDTH;
			iHeight=HIWORD(lRowCol)*CHAR_HEIGHT+20;
			SetWindowPos(GetDlgItem(hWnd,IDC_GRAPH),0,0,0,iWidth,iHeight,SWP_NOREPOSITION | SWP_NOMOVE);
			SetWindowPos(hWnd,0,0,0,iWidth+(iMargin<<1),iHeight+(iMargin<<1),SWP_NOREPOSITION | SWP_NOMOVE);
			SetDlgItemTextW(hWnd,IDC_GRAPH,(WCHAR*)szGraph);
			SetWindowText(hWnd,szGraphTitle);
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					EndDialog(hWnd,0);
					if(hLetter) hLetter=NULL;
					return 0;
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

				case IDCANCEL:
					EndDialog(hWnd,0);
					return 0;
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

				case IDCANCEL:
					EndDialog(hWnd,0);
					return 0;
			}
	}

	return 0;
}

//options dialog
LRESULT CALLBACK OptionsProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int iPrevLang;

	switch(iMsg)
	{
		case WM_INITDIALOG: //init values
			//hillclimber parameters
			SetDlgItemInt(hWnd,IDC_MAXFAIL,siSolveInfo.max_fail,0);
			SetDlgItemInt(hWnd,IDC_SWAPS,siSolveInfo.swaps,0);
			SetDlgItemInt(hWnd,IDC_REVERT,siSolveInfo.revert,0);

			//score parameters
			SendMessage(GetDlgItem(hWnd,IDC_USEBI),BM_SETCHECK,siSolveInfo.use_graphs & USE_BI,0);
			SendMessage(GetDlgItem(hWnd,IDC_USETRI),BM_SETCHECK,siSolveInfo.use_graphs & USE_TRI,0);
			SendMessage(GetDlgItem(hWnd,IDC_USETETRA),BM_SETCHECK,siSolveInfo.use_graphs & USE_TETRA,0);
			SendMessage(GetDlgItem(hWnd,IDC_USEPENTA),BM_SETCHECK,siSolveInfo.use_graphs & USE_PENTA,0);

			//display options
			SetDlgItemInt(hWnd,IDC_LINECHARS,iLineChars,0);

			//language		
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"English");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"Spanish");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"German");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_ADDSTRING,0,(LPARAM)"Italian");
			SendDlgItemMessage(hWnd,IDC_LANG,CB_SETCURSEL,iLang,0);

			//word length
			SetDlgItemInt(hWnd,IDC_WORD_MIN,iWordMin,false);
			SetDlgItemInt(hWnd,IDC_WORD_MAX,iWordMax,false);

			//extra letters
			SendDlgItemMessage(hWnd,IDC_EXTRA_LTR,EM_LIMITTEXT,MAX_EXTRA,0);
			SetDlgItemText(hWnd,IDC_EXTRA_LTR,szExtraLtr);
			
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_EXTRA_ALPHABET:
					SetDlgItemText(hWnd,IDC_EXTRA_LTR,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
					return 0;
					
				case IDOK: //get new values
					//hillclimber parameters
					siSolveInfo.max_fail=GetDlgItemInt(hWnd,IDC_MAXFAIL,0,0);
					siSolveInfo.swaps=GetDlgItemInt(hWnd,IDC_SWAPS,0,0);
					siSolveInfo.revert=GetDlgItemInt(hWnd,IDC_REVERT,0,0);

					//score parameters
					siSolveInfo.use_graphs=0;
					
					if(SendMessage(GetDlgItem(hWnd,IDC_USEBI),BM_GETCHECK,0,0))
						siSolveInfo.use_graphs+=USE_BI;
						
					if(SendMessage(GetDlgItem(hWnd,IDC_USETRI),BM_GETCHECK,0,0))
						siSolveInfo.use_graphs+=USE_TRI;
						
					if(SendMessage(GetDlgItem(hWnd,IDC_USETETRA),BM_GETCHECK,0,0))
						siSolveInfo.use_graphs+=USE_TETRA;
						
					if(SendMessage(GetDlgItem(hWnd,IDC_USEPENTA),BM_GETCHECK,0,0))
						siSolveInfo.use_graphs+=USE_PENTA;

					//display options
					iLineChars=GetDlgItemInt(hWnd,IDC_LINECHARS,0,0);

					//language
					iPrevLang=iLang;
					iLang=SendDlgItemMessage(hWnd,IDC_LANG,CB_GETCURSEL,0,0);
					if(iLang!=iPrevLang) SetLanguage();

					//word length
					iWordMin=GetDlgItemInt(hWnd,IDC_WORD_MIN,false,false);
					iWordMax=GetDlgItemInt(hWnd,IDC_WORD_MAX,false,false);

					//extra letters
					GetDlgItemText(hWnd,IDC_EXTRA_LTR,szExtraLtr,MAX_EXTRA);
					
					//0 chars per line
					if(!iLineChars)
					{
						MessageBox(hWnd,"Line length must be greather than 0","Notice",MB_ICONEXCLAMATION);
						return 0;
					}
					
					//no ngrams are checked
					if(!siSolveInfo.use_graphs)
					{
						MessageBox(hWnd,"At least one set of ngrams must be selected","Notice",MB_ICONEXCLAMATION);
						return 0;
					}

					//blank best key, so that additional chars are renewed
					//siSolveInfo.best_key[0]='\0';
					
					//update display
					SetScrollBar();					
					SetDlgInfo();
					ClearTextAreas();
					SetText();

				case IDCANCEL:
					EndDialog(hWnd,0);
					return 0;
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

/*
void LetterDist(int target, HWND hWnd)
{
	int letter, set_symbols=0, max_letter, x, y;
	float n[26], unitemp[27], unitemp2[27], maxtemp=0, skewvalue=(float)0.095, unimax=0, temp;

	memset(n,0,26*sizeof(float));

	//make a temp copy of the unigraphs for sorting
	for(letter=0; letter<26; letter++) {
		temp=unitemp2[letter]=unitemp[letter]=message.cur_map.GetUnigraph(letter);
		if(temp>unimax) unimax=temp; 
	}
	
	//calculate the skew based on number of uniques
	//still have to derive the formula for this
//	skewvalue=(unimax/94)-0.040127;
//	unimax/=94;
//	skevalue*=message.cur_map.GetNumSymbols();

	//create a frequency-sorted alphabet
	for(y=0; y<26; y++) {
		letter=-1;
		maxtemp=0;
		for(x=0; x<26; x++) {
			if(unitemp[x]-skewvalue>maxtemp) { maxtemp=unitemp[x]; letter=x; }
		}
		if(letter != -1 && set_symbols<target) { unitemp[letter]=0; n[letter]++; set_symbols++; }
	}
	
	if(set_symbols<target) { 
		//calculate real number of occurences for each letter
		for(letter=0; letter<26; letter++)
		{
			temp=(message.cur_map.GetUnigraph(letter)/100)*(target);
			if((int)temp>0) temp--;
			n[letter]+=temp;
			if(temp>0) set_symbols+=(int)temp;
		}
	}
	
	while(set_symbols<target)
	{
		//find the letter with the highest freq.
		x=0;

		for(letter=0; letter<26; letter++)
			if(unitemp2[letter]>unitemp2[x]) { x=letter; unitemp2[x]--; }
	
		//set that letter to the next whole number
		n[x]++;
		set_symbols++;
	}

	for(letter=0; letter<26; letter++)
		SetDlgItemInt(hWnd,lprgiInitID[letter],int(n[letter]),false);
}
*/

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
							
				case IDCANCEL: EndDialog(hWnd,0); hHomo=NULL; return 0;
			}				
			return 0;

		case WM_DESTROY:
			DeleteObject(hTempFont);
			return 0;
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

		case WM_LBUTTONDOWN:
			EndDialog(hWnd,0);
			return 0;
	}

	return 0;
}

LRESULT CALLBACK TextProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	switch(iMsg)
	{
		//click on text
		case WM_LBUTTONDOWN:
			TextClick(LOWORD(lParam),HIWORD(lParam));
			return 0;

		case WM_MBUTTONDOWN:
			TextClick(LOWORD(lParam),HIWORD(lParam));
			ToggleLock();
			return 0;

		case WM_RBUTTONDOWN:
			TextClick(LOWORD(lParam),HIWORD(lParam));
			CreateTextMenu();
			return 0;

		case WM_PAINT: //redraw text windows
			BeginPaint(hWnd,&ps);
			SetText();
			EndPaint(hWnd,&ps);
			return 0;
	}

	return DefWindowProc(hWnd,iMsg,wParam,lParam);
}
