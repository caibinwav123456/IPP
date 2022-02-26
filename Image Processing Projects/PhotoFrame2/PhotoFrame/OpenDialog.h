#pragma once
#include "afxwin.h"


// COpenDialog dialog

class COpenDialog : public CDialog
{
	DECLARE_DYNAMIC(COpenDialog)

public:
	COpenDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~COpenDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_OPEN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strSrc;
	CString m_strBkGnd;
	afx_msg void OnBnClickedButtonBrowseSource();
	afx_msg void OnBnClickedButtonBrowseBkgnd();
	CString m_strBk2;
	bool m_bUseDoubleImage;
	bool m_bUseMask;
	afx_msg void OnBnClickedButtonBrowseBk2();
	afx_msg void OnBnClickedCheck1();
	CButton m_ChkUseDouble;
	virtual BOOL OnInitDialog();
	CEdit m_editBk2;
	CButton m_btnBrowseBk2;
	CString m_strMask;
	CEdit m_editMask;
	CButton m_btnBrowseMask;
	afx_msg void OnBnClickedButtonBrowseMask();
	afx_msg void OnBnClickedCheckEnableMask();
	CButton m_chkMask;
};
