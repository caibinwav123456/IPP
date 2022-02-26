
// MeshConverterDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CMeshConverterDlg dialog
class CMeshConverterDlg : public CDialog
{
// Construction
public:
	CMeshConverterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MESHCONVERTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	bool m_b32Bit;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	CString m_strSrc;
	CString m_strDest;
	afx_msg void OnBnClickedButtonBrowseSrc();
	afx_msg void OnBnClickedButtonBrowseDest();
	afx_msg void OnBnClickedButtonConvert();
	afx_msg void OnBnClickedCheck32bit();
	CButton m_Check;
};
