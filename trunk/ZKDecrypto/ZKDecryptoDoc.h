// ZKDecryptoDoc.h : interface of the CZKDecryptoDoc class
//


#pragma once


class CZKDecryptoDoc : public CDocument
{
protected: // create from serialization only
	CZKDecryptoDoc();
	DECLARE_DYNCREATE(CZKDecryptoDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CZKDecryptoDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


