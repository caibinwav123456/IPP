#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CDialogSettings dialog

class CDialogSettings : public CDialog
{
	DECLARE_DYNAMIC(CDialogSettings)

public:
	CDialogSettings(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogSettings();

// Dialog Data
	enum { IDD = IDD_DIALOG_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strWorkingPath;
	CString m_strEdge;
	CString m_strAlpha;
	BOOL m_bChkGray;
	CWnd* m_pWndParent;
	afx_msg void OnBnClickedButtonGenerate();
	afx_msg void OnBnClickedButtonExport();
	CString m_strTemplName;
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedButtonApply();
	CSpinButtonCtrl m_Spin1;
	CSpinButtonCtrl m_Spin2;
	CEdit m_EditEdge;
	CEdit m_EditAlpha;
	virtual BOOL OnInitDialog();
	afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpin2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonExportPackage();
};
