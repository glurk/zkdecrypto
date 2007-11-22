// ZKDecryptoView.h : interface of the CZKDecryptoView class
//


#pragma once
#include "afxwin.h"
#include "z340.h"
#include <string>

//#define WM_USER_THREAD_UPDATE_PROGRESS (WM_USER+0x101)
class CZKDecryptoView : public CFormView
{
protected: // create from serialization only
	CZKDecryptoView();
	DECLARE_DYNCREATE(CZKDecryptoView)
	static UINT hc(LPVOID pParam);
	void hc(CFormView*);
	//z340* cipher;
public:
	enum{ IDD = IDD_ZKDECRYPTO_FORM };

// Attributes
public:
	CZKDecryptoDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CZKDecryptoView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	std::string IntToString(int i);

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBeginghillclimber();
	afx_msg LRESULT OnThreadUpdateBestScore(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnThreadUpdatePlaintext(WPARAM wParam, LPARAM lParam);

	CEdit txtPlainText;
	CEdit txtBestScore;
	CString CipherText;
	CString Key;
	CEdit txtCipherText;
	CEdit txtKey;
};

#ifndef _DEBUG  // debug version in ZKDecryptoView.cpp
inline CZKDecryptoDoc* CZKDecryptoView::GetDocument() const
   { return reinterpret_cast<CZKDecryptoDoc*>(m_pDocument); }
#endif

