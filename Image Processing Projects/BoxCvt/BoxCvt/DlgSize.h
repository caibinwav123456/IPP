#pragma once


// CDlgSize dialog

class CDlgSize : public CDialog
{
	DECLARE_DYNAMIC(CDlgSize)

public:
	CDlgSize(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSize();

// Dialog Data
	enum { IDD = IDD_DIALOG_SIZE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_size;
	virtual void OnOK();
	CString m_seg;
	CString m_overlap;
};
