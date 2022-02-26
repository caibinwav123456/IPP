#pragma once
#include "afxcmn.h"


// DlgProgress dialog
#include "ChildView.h"

class DlgProgress : public CDialog
{
	DECLARE_DYNAMIC(DlgProgress)

public:
	DlgProgress(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgProgress();

// Dialog Data
	enum { IDD = IDD_DIALOG_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_Progress;
	CChildView* m_View;
	CString m_BatchFile;
	bool m_bEnd;
	HANDLE m_hEvent;
	void ParseArgs(char* p);
	void HandleStats(bool bSave, bool bClean);
	bool m_bSaveRowStats;
	int m_CharSpace;
	int m_LineSpace;
	CPoint m_org;
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	LRESULT OnSetPos(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonCancel();
};
