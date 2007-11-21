// ZKDecryptoView.h : interface of the CZKDecryptoView class
//


#pragma once

UINT hillclimb(LPVOID pParam);
class CZKDecryptoView : public CFormView
{
protected: // create from serialization only
	CZKDecryptoView();
	DECLARE_DYNCREATE(CZKDecryptoView)

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

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBeginghillclimber();
};

#ifndef _DEBUG  // debug version in ZKDecryptoView.cpp
inline CZKDecryptoDoc* CZKDecryptoView::GetDocument() const
   { return reinterpret_cast<CZKDecryptoDoc*>(m_pDocument); }
#endif

