// ZKDecrypto.h : main header file for the ZKDecrypto application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CZKDecryptoApp:
// See ZKDecrypto.cpp for the implementation of this class
//

class CZKDecryptoApp : public CWinApp
{
public:
	CZKDecryptoApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CZKDecryptoApp theApp;