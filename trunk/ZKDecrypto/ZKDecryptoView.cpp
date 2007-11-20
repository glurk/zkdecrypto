// ZKDecryptoView.cpp : implementation of the CZKDecryptoView class
//

#include "stdafx.h"
#include "ZKDecrypto.h"

#include "ZKDecryptoDoc.h"
#include "ZKDecryptoView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CZKDecryptoView

IMPLEMENT_DYNCREATE(CZKDecryptoView, CFormView)

BEGIN_MESSAGE_MAP(CZKDecryptoView, CFormView)
END_MESSAGE_MAP()

// CZKDecryptoView construction/destruction

CZKDecryptoView::CZKDecryptoView()
	: CFormView(CZKDecryptoView::IDD)
{
	// TODO: add construction code here

}

CZKDecryptoView::~CZKDecryptoView()
{
}

void CZKDecryptoView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BOOL CZKDecryptoView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CZKDecryptoView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

}


// CZKDecryptoView diagnostics

#ifdef _DEBUG
void CZKDecryptoView::AssertValid() const
{
	CFormView::AssertValid();
}

void CZKDecryptoView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CZKDecryptoDoc* CZKDecryptoView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CZKDecryptoDoc)));
	return (CZKDecryptoDoc*)m_pDocument;
}
#endif //_DEBUG


// CZKDecryptoView message handlers
