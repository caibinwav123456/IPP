#pragma once
#include "afxcmn.h"


// CDialogSettings dialog

class CDialogSettings : public CDialog
{
	DECLARE_DYNAMIC(CDialogSettings)

public:
	CDialogSettings(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogSettings();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CSpinButtonCtrl m_SpinProp;
	CSpinButtonCtrl m_SpinEdge;
	CWnd* m_pWndParent;
	virtual BOOL OnInitDialog();
	CString m_strProp;
	CString m_strEdge;
	CString m_strWorkingPath;
	CString m_strTemplName;
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpin2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonApply();
	CSpinButtonCtrl m_SpinScale;
	CString m_strScale;
	afx_msg void OnDeltaposSpin3(NMHDR *pNMHDR, LRESULT *pResult);
	CSpinButtonCtrl m_SpinWidth;
	CSpinButtonCtrl m_SpinHeight;
	CString m_strFaceWidth;
	CString m_strFaceHeight;
	afx_msg void OnDeltaposSpin4(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpin5(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonGenerateTempl();
	afx_msg void OnBnClickedButtonExportTempl();
};
