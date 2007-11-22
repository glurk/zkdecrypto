#pragma once


// ZKDECRYPTO_FORM dialog

class ZKDECRYPTO_FORM : public CDialog
{
	DECLARE_DYNAMIC(ZKDECRYPTO_FORM)

public:
	ZKDECRYPTO_FORM(CWnd* pParent = NULL);   // standard constructor
	virtual ~ZKDECRYPTO_FORM();

// Dialog Data
	enum { IDD = IDD_ZKDECRYPTO_FORM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
