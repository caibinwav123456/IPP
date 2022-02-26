// DialogSettings.cpp : implementation file
//

#include "stdafx.h"
#include "PhotoFrame.h"
#include "DialogSettings.h"


// CDialogSettings dialog

IMPLEMENT_DYNAMIC(CDialogSettings, CDialog)

CDialogSettings::CDialogSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogSettings::IDD, pParent)
	, m_strWorkingPath(_T("C:\\"))
	, m_strEdge(_T(""))
	, m_strAlpha(_T(""))
	, m_bChkGray(FALSE)
	, m_strTemplName(_T(""))
{
	m_pWndParent = pParent;
}

CDialogSettings::~CDialogSettings()
{
}

void CDialogSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_WORKING_PATH, m_strWorkingPath);
	DDX_Text(pDX, IDC_EDIT2, m_strEdge);
	DDX_Text(pDX, IDC_EDIT3, m_strAlpha);
	DDX_Check(pDX, IDC_CHECK1, m_bChkGray);
	DDX_Text(pDX, IDC_EDIT1, m_strTemplName);
	DDX_Control(pDX, IDC_SPIN1, m_Spin1);
	DDX_Control(pDX, IDC_SPIN2, m_Spin2);
	DDX_Control(pDX, IDC_EDIT2, m_EditEdge);
	DDX_Control(pDX, IDC_EDIT3, m_EditAlpha);
}


BEGIN_MESSAGE_MAP(CDialogSettings, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE, &CDialogSettings::OnBnClickedButtonGenerate)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT, &CDialogSettings::OnBnClickedButtonExport)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CDialogSettings::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CDialogSettings::OnBnClickedButtonApply)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CDialogSettings::OnDeltaposSpin1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, &CDialogSettings::OnDeltaposSpin2)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_PACKAGE, &CDialogSettings::OnBnClickedButtonExportPackage)
END_MESSAGE_MAP()


// CDialogSettings message handlers

void CDialogSettings::OnBnClickedButtonGenerate()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	switch(m_pWndParent->SendMessage(WM_NOTIFY_GENERATE_CONFIG_FILE, (WPARAM)this))
	{
	case S_OK:
		MessageBox(_T("Generate Success"));
		break;
	case S_FALSE:
		MessageBox(_T("Generate Failed"));
		break;
	case 3:
		MessageBox(_T("Please specify a template file name"));
		break;
	}
}

void CDialogSettings::OnBnClickedButtonExport()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	switch(m_pWndParent->SendMessage(WM_NOTIFY_EXPORT_CONFIG_FILE, (WPARAM)this))
	{
	case S_OK:
		MessageBox(_T("Export sucess"));
		break;
	case S_FALSE:
		MessageBox(_T("Generate Templ File Failed"));
		break;
	case 2:
		MessageBox(_T("Copy file failed, please check if file already exists"));
		break;
	case 3:
		MessageBox(_T("Please specify a template file name"));
		break;
	case 4:
		MessageBox(_T("Save config file failed"));
		break;
	case ERROR_UNKNOWN:
		break;
	}
}

void CDialogSettings::OnBnClickedButtonBrowse()
{
	// TODO: Add your control notification handler code here
	TCHAR folder[MAX_PATH];
	BROWSEINFO info;
	ZeroMemory(&info, sizeof(info));
	info.hwndOwner = m_hWnd;
	info.lpszTitle = _T("Select Base Folder(typically AppDir\\media)");
	info.pidlRoot = NULL;
	info.pszDisplayName = folder;
	info.ulFlags = BIF_USENEWUI;
	PIDLIST_ABSOLUTE pId = SHBrowseForFolder(&info);
	if(NULL != pId)
	{
		TCHAR buf[MAX_PATH];
		SHGetPathFromIDList(pId, buf);
		m_strWorkingPath = buf;
		UpdateData(FALSE);
	}
	CoTaskMemFree(pId);
}

void CDialogSettings::OnBnClickedButtonApply()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_pWndParent->SendMessage(WM_NOTIFY_APPLY_SETTINGS, (WPARAM)this);
}

BOOL CDialogSettings::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	m_Spin1.SetBuddy(&m_EditEdge);
	m_Spin1.SetRange(0, 100);
	m_Spin2.SetBuddy(&m_EditAlpha);
	m_Spin2.SetRange(0, 100);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDialogSettings::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
	USES_CONVERSION;
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	float fEdge = atof(T2A((LPTSTR)(LPCTSTR)m_strEdge));
	if(pNMUpDown->iDelta > 0)
	{
		fEdge+=0.01;
		if(fEdge>1)fEdge=1;
	}
	else
	{
		fEdge-=0.01;
		if(fEdge<0)fEdge=0;
	}
	m_strEdge.Format(_T("%4.2f"), fEdge);
	UpdateData(FALSE);
	*pResult = 0;
}

void CDialogSettings::OnDeltaposSpin2(NMHDR *pNMHDR, LRESULT *pResult)
{
	USES_CONVERSION;
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	float fAlpha = atof(T2A((LPTSTR)(LPCTSTR)m_strAlpha));
	if(pNMUpDown->iDelta > 0)
	{
		fAlpha+=0.01;
		if(fAlpha>1)fAlpha=1;
	}
	else
	{
		fAlpha-=0.01;
		if(fAlpha<0)fAlpha=0;
	}
	m_strAlpha.Format(_T("%4.2f"), fAlpha);
	UpdateData(FALSE);
	*pResult = 0;
}

void CDialogSettings::OnBnClickedButtonExportPackage()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	switch(m_pWndParent->SendMessage(WM_NOTIFY_EXPORT_PAKAGE, (WPARAM)this))
	{
	case S_OK:
		MessageBox(_T("Export sucess"));
		break;
	case S_FALSE:
		MessageBox(_T("Generate Templ File Failed"));
		break;
	case 2:
		MessageBox(_T("Copy file failed, please check if file already exists"));
		break;
	case 3:
		MessageBox(_T("Please specify a template file name"));
		break;
	case 4:
		MessageBox(_T("Save config file failed"));
		break;
	case 5:
		MessageBox(_T("Packaging failed"));
		break;
	case ERROR_UNKNOWN:
		break;
	}
}
