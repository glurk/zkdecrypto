// ZKDecryptoView.cpp : implementation of the CZKDecryptoView class
//

#include "stdafx.h"
#include "z340.h"
#include "ZKDecrypto.h"

#include "ZKDecryptoDoc.h"
#include "ZKDecryptoView.h"
#include <sstream>
#include <string>
#define  _CRT_SECURE_NO_WARNINGS 1
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CZKDecryptoView

IMPLEMENT_DYNCREATE(CZKDecryptoView, CFormView)

BEGIN_MESSAGE_MAP(CZKDecryptoView, CFormView)
	ON_BN_CLICKED(IDC_BEGINGHILLCLIMBER, &CZKDecryptoView::OnBnClickedBeginghillclimber)
	ON_MESSAGE(WM_USER_THREAD_UPDATE_BESTSCORE, OnThreadUpdateBestScore)
	ON_MESSAGE(WM_USER_THREAD_UPDATE_PLAINTEXT, OnThreadUpdatePlaintext)
END_MESSAGE_MAP()


// CZKDecryptoView construction/destruction

CZKDecryptoView::CZKDecryptoView()
	: CFormView(CZKDecryptoView::IDD)
	, CipherText(_T(""))
	, Key(_T(""))
{
	// TODO: add construction code here
//	CWinThread pthread = new CWinThread();
	//z340 zodiac;


}


CZKDecryptoView::~CZKDecryptoView()
{
//	hillclimb();

}




void CZKDecryptoView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PLAINTEXT, txtPlainText);
	DDX_Control(pDX, IDC_BESTSCORE, txtBestScore);
	DDX_Text(pDX, IDC_CIPHERTEXT, CipherText);
	DDX_Text(pDX, IDC_KEY, Key);
	DDX_Control(pDX, IDC_CIPHERTEXT, txtCipherText);
	DDX_Control(pDX, IDC_KEY, txtKey);
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

void CZKDecryptoView::OnBnClickedBeginghillclimber()
{
	// TODO: Add your control notification handler code here
	//cipher = new z340();
	CWinThread* pThread = AfxBeginThread(hc, this,THREAD_PRIORITY_IDLE);
	//pThread->SuspendThread();
	/*
	while(1)
	{

		LPCTSTR str = (cipher->solvedsav);
		this->txtPlainText.SetWindowTextA(str);
		this->UpdateWindow();
		for(int i = 0; i < 100000; i++)
		{}
	}
	*/
	
}

UINT CZKDecryptoView::hc(LPVOID p)
{
	CFormView * cfv = (CFormView*)p;
    CZKDecryptoView * me = (CZKDecryptoView *)p;
    me->hc(cfv);
    return 0;
}

void CZKDecryptoView::hc(CFormView * cfv)
{
	//const int len1 = Key.GetLength() + 1;
	txtCipherText.GetWindowTextA(CipherText);
	txtKey.GetWindowTextA(Key);
	char key[ASCII_SIZE];
	char ciphertext[MAX_CIPH_LENGTH];
	for(int i=0;i<MAX_CIPH_LENGTH;i++) ciphertext[i]=0;						//INITIALIZE (ZERO) ARRAYS
	for(int i=0;i<ASCII_SIZE;i++) key[i]=0;
	strcpy(ciphertext,CipherText.GetString());
	strcpy(key,Key.GetString());
	
	hillclimb(ciphertext,key,strlen(ciphertext),cfv);
}


LRESULT CZKDecryptoView::OnThreadUpdateBestScore(WPARAM wParam, LPARAM lParam)
{
//m_progress.SetPos(100*(int)wParam/(int)lParam);
//	int
	//LPCTSTR str = ();
	//this->BestScore = int(wParam);
	//this->UpdateWindow();

	this->txtBestScore.SetWindowTextA(IntToString(int(wParam)).c_str());
return 0;
}

LRESULT CZKDecryptoView::OnThreadUpdatePlaintext(WPARAM wParam, LPARAM lParam)
{
//m_progress.SetPos(100*(int)wParam/(int)lParam);
//	int
	//LPCTSTR str = ();
	//this->BestScore = int(wParam);
	//this->UpdateWindow();
	char * a = (char *)wParam;
	this->txtPlainText.SetWindowTextA(a);
return 0;
}


std::string CZKDecryptoView::IntToString(int i)
{
	std::ostringstream s;
	s<<i;
	//CString s;
	return s.str();
}
