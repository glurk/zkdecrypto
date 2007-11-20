// ZKDecryptoDoc.cpp : implementation of the CZKDecryptoDoc class
//

#include "stdafx.h"
#include "ZKDecrypto.h"

#include "ZKDecryptoDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CZKDecryptoDoc

IMPLEMENT_DYNCREATE(CZKDecryptoDoc, CDocument)

BEGIN_MESSAGE_MAP(CZKDecryptoDoc, CDocument)
END_MESSAGE_MAP()


// CZKDecryptoDoc construction/destruction

CZKDecryptoDoc::CZKDecryptoDoc()
{
	// TODO: add one-time construction code here

}

CZKDecryptoDoc::~CZKDecryptoDoc()
{
}

BOOL CZKDecryptoDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CZKDecryptoDoc serialization

void CZKDecryptoDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CZKDecryptoDoc diagnostics

#ifdef _DEBUG
void CZKDecryptoDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CZKDecryptoDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CZKDecryptoDoc commands
