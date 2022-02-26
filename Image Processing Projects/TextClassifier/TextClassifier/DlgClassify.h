#pragma once


// CDlgClassify dialog
#include "TextClassify.h"
class CDlgClassify : public CDialog
{
	DECLARE_DYNAMIC(CDlgClassify)

public:
	CDlgClassify(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgClassify();

// Dialog Data
	enum { IDD = IDD_DIALOG_CLASSIFY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strText;
	TextClassify* m_Classify;
	virtual void OnOK();
};
