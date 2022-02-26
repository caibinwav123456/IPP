#pragma once
#include "afxwin.h"


// CDlgParams dialog

class CDlgParams : public CDialog
{
	DECLARE_DYNAMIC(CDlgParams)

public:
	CDlgParams(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgParams();

// Dialog Data
	enum { IDD = IDD_DIALOG_PARAMS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strCharSpace;
	CString m_strLineSpace;
	CString m_strHMargin;
	CString m_strVMargin;
	BOOL m_bOutputRowStats;
	CButton m_ChkOutputRowStats;
};
