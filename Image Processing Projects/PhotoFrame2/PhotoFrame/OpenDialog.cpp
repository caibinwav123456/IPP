// OpenDialog.cpp : implementation file
//

#include "stdafx.h"
#include "PhotoFrame.h"
#include "OpenDialog.h"


// COpenDialog dialog

IMPLEMENT_DYNAMIC(COpenDialog, CDialog)

COpenDialog::COpenDialog(CWnd* pParent /*=NULL*/)
	: CDialog(COpenDialog::IDD, pParent)
	, m_strSrc(_T(""))
	, m_strBkGnd(_T(""))
	, m_strBk2(_T(""))
	, m_strMask(_T(""))
{
	m_bUseDoubleImage = false;
	m_bUseMask = false;
}

COpenDialog::~COpenDialog()
{
}

void COpenDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SOURCE, m_strSrc);
	DDX_Text(pDX, IDC_EDIT_BKGND, m_strBkGnd);
	DDX_Text(pDX, IDC_EDIT1, m_strBk2);
	DDX_Control(pDX, IDC_CHECK1, m_ChkUseDouble);
	DDX_Control(pDX, IDC_EDIT1, m_editBk2);
	DDX_Control(pDX, IDC_BUTTON_BROWSE_BK2, m_btnBrowseBk2);
	DDX_Text(pDX, IDC_EDIT_MASK, m_strMask);
	DDX_Control(pDX, IDC_EDIT_MASK, m_editMask);
	DDX_Control(pDX, IDC_BUTTON_BROWSE_MASK, m_btnBrowseMask);
	DDX_Control(pDX, IDC_CHECK_ENABLE_MASK, m_chkMask);
}


BEGIN_MESSAGE_MAP(COpenDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SOURCE, &COpenDialog::OnBnClickedButtonBrowseSource)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_BKGND, &COpenDialog::OnBnClickedButtonBrowseBkgnd)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_BK2, &COpenDialog::OnBnClickedButtonBrowseBk2)
	ON_BN_CLICKED(IDC_CHECK1, &COpenDialog::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_MASK, &COpenDialog::OnBnClickedButtonBrowseMask)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_MASK, &COpenDialog::OnBnClickedCheckEnableMask)
END_MESSAGE_MAP()


// COpenDialog message handlers

void COpenDialog::OnBnClickedButtonBrowseSource()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(true, NULL, NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Image Files|*.bmp;*.jpg;*.png|All Files|*.*||", this);
	if(dlg.DoModal() == IDOK)
	{
		m_strSrc = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

void COpenDialog::OnBnClickedButtonBrowseBkgnd()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(true, NULL, NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Image Files|*.bmp;*.jpg;*.png|All Files|*.*||", this);
	if(dlg.DoModal() == IDOK)
	{
		m_strBkGnd = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

void COpenDialog::OnBnClickedButtonBrowseBk2()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(true, NULL, NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Image Files|*.bmp;*.jpg;*.png|All Files|*.*||", this);
	if(dlg.DoModal() == IDOK)
	{
		m_strBk2 = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

void COpenDialog::OnBnClickedCheck1()
{
	// TODO: Add your control notification handler code here
	if(BST_CHECKED == m_ChkUseDouble.GetCheck())
	{
		m_bUseDoubleImage = true;
		m_editBk2.EnableWindow(TRUE);
		m_btnBrowseBk2.EnableWindow(TRUE);
	}
	else if(BST_UNCHECKED == m_ChkUseDouble.GetCheck())
	{
		m_bUseDoubleImage = false;
		m_editBk2.EnableWindow(FALSE);
		m_btnBrowseBk2.EnableWindow(FALSE);
	}
}

BOOL COpenDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_ChkUseDouble.SetCheck(BST_UNCHECKED);
	m_editBk2.EnableWindow(FALSE);
	m_btnBrowseBk2.EnableWindow(FALSE);
	m_chkMask.SetCheck(BST_UNCHECKED);
	m_editMask.EnableWindow(FALSE);
	m_btnBrowseMask.EnableWindow(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void COpenDialog::OnBnClickedButtonBrowseMask()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(true, NULL, NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Image Files|*.bmp;*.jpg;*.png|All Files|*.*||", this);
	if(dlg.DoModal() == IDOK)
	{
		m_strMask = dlg.GetPathName();
		UpdateData(FALSE);
	}
}

void COpenDialog::OnBnClickedCheckEnableMask()
{
	// TODO: Add your control notification handler code here
	if(BST_CHECKED == m_chkMask.GetCheck())
	{
		m_bUseMask = true;
		m_editMask.EnableWindow(TRUE);
		m_btnBrowseMask.EnableWindow(TRUE);
	}
	else if(BST_UNCHECKED == m_chkMask.GetCheck())
	{
		m_bUseMask = false;
		m_editMask.EnableWindow(FALSE);
		m_btnBrowseMask.EnableWindow(FALSE);
	}
}
