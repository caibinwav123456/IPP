// DialogSettings.cpp : implementation file
//

#include "stdafx.h"
#include "Face.h"
#include "DialogSettings.h"


// CDialogSettings dialog

IMPLEMENT_DYNAMIC(CDialogSettings, CDialog)

CDialogSettings::CDialogSettings(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogSettings::IDD, pParent)
	, m_strProp(_T(""))
	, m_strEdge(_T(""))
	, m_strWorkingPath(_T(""))
	, m_strTemplName(_T(""))
	, m_strScale(_T(""))
	, m_strFaceWidth(_T(""))
	, m_strFaceHeight(_T(""))
{
	m_pWndParent = pParent;
}

CDialogSettings::~CDialogSettings()
{
}

void CDialogSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SPIN1, m_SpinProp);
	DDX_Control(pDX, IDC_SPIN2, m_SpinEdge);
	DDX_Text(pDX, IDC_EDIT_FACE_PROP, m_strProp);
	DDX_Text(pDX, IDC_EDIT_EDGE, m_strEdge);
	DDX_Text(pDX, IDC_EDIT_WORKING_FOLDER, m_strWorkingPath);
	DDX_Text(pDX, IDC_EDIT_TEMPL_NAME, m_strTemplName);
	DDX_Control(pDX, IDC_SPIN3, m_SpinScale);
	DDX_Text(pDX, IDC_EDIT_SCALE, m_strScale);
	DDX_Control(pDX, IDC_SPIN4, m_SpinWidth);
	DDX_Control(pDX, IDC_SPIN5, m_SpinHeight);
	DDX_Text(pDX, IDC_EDIT_WIDTH, m_strFaceWidth);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_strFaceHeight);
}


BEGIN_MESSAGE_MAP(CDialogSettings, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CDialogSettings::OnBnClickedButtonBrowse)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CDialogSettings::OnDeltaposSpin1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, &CDialogSettings::OnDeltaposSpin2)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CDialogSettings::OnBnClickedButtonApply)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN3, &CDialogSettings::OnDeltaposSpin3)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN4, &CDialogSettings::OnDeltaposSpin4)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN5, &CDialogSettings::OnDeltaposSpin5)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_TEMPL, &CDialogSettings::OnBnClickedButtonGenerateTempl)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_TEMPL, &CDialogSettings::OnBnClickedButtonExportTempl)
END_MESSAGE_MAP()


// CDialogSettings message handlers

BOOL CDialogSettings::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_SpinProp.SetBuddy(GetDlgItem(IDC_EDIT_FACE_PROP));
	m_SpinProp.SetRange(0, 100);
	m_SpinEdge.SetBuddy(GetDlgItem(IDC_EDIT_EDGE));
	m_SpinEdge.SetRange(0, 100);
	m_SpinScale.SetBuddy(GetDlgItem(IDC_EDIT_SCALE));
	m_SpinScale.SetRange(0, 100);
	m_SpinWidth.SetBuddy(GetDlgItem(IDC_EDIT_WIDTH));
	m_SpinWidth.SetRange(0, 100);
	m_SpinHeight.SetBuddy(GetDlgItem(IDC_EDIT_HEIGHT));
	m_SpinHeight.SetRange(0, 100);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
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

void CDialogSettings::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
	USES_CONVERSION;
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	float fProp = atof(T2A((LPTSTR)(LPCTSTR)m_strProp));
	if(pNMUpDown->iDelta > 0)
	{
		fProp+=0.01;
		if(fProp>1.5)fProp=1.5;
	}
	else
	{
		fProp-=0.01;
		if(fProp<0.2)fProp=0.2;
	}
	m_strProp.Format(_T("%4.2f"), fProp);
	UpdateData(FALSE);
	OnBnClickedButtonApply();
	*pResult = 0;
}

void CDialogSettings::OnDeltaposSpin2(NMHDR *pNMHDR, LRESULT *pResult)
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
	OnBnClickedButtonApply();
	*pResult = 0;
}

void CDialogSettings::OnDeltaposSpin3(NMHDR *pNMHDR, LRESULT *pResult)
{
	USES_CONVERSION;
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	float fScale = atof(T2A((LPTSTR)(LPCTSTR)m_strScale));
	if(pNMUpDown->iDelta > 0)
	{
		fScale+=0.01;
		if(fScale>1.5)fScale=1.5;
	}
	else
	{
		fScale-=0.01;
		if(fScale<0.8)fScale=0.8;
	}
	m_strScale.Format(_T("%4.2f"), fScale);
	UpdateData(FALSE);
	OnBnClickedButtonApply();
	*pResult = 0;
}

void CDialogSettings::OnDeltaposSpin4(NMHDR *pNMHDR, LRESULT *pResult)
{
	USES_CONVERSION;
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	float fWidth = atof(T2A((LPTSTR)(LPCTSTR)m_strFaceWidth));
	if(pNMUpDown->iDelta > 0)
	{
		fWidth+=0.01;
	}
	else
	{
		fWidth-=0.01;
		if(fWidth<0)fWidth=0;
	}
	m_strFaceWidth.Format(_T("%4.2f"), fWidth);
	UpdateData(FALSE);
	OnBnClickedButtonApply();
	*pResult = 0;
}

void CDialogSettings::OnDeltaposSpin5(NMHDR *pNMHDR, LRESULT *pResult)
{
	USES_CONVERSION;
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	float fHeight = atof(T2A((LPTSTR)(LPCTSTR)m_strFaceHeight));
	if(pNMUpDown->iDelta > 0)
	{
		fHeight+=0.01;
	}
	else
	{
		fHeight-=0.01;
		if(fHeight<0)fHeight=0;
	}
	m_strFaceHeight.Format(_T("%4.2f"), fHeight);
	UpdateData(FALSE);
	OnBnClickedButtonApply();
	*pResult = 0;
}

void CDialogSettings::OnBnClickedButtonApply()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(m_pWndParent)
	{
		m_pWndParent->SendMessage(WM_NOTIFY_APPLY_SETTINGS, (WPARAM)this);
	}
}



void CDialogSettings::OnBnClickedButtonGenerateTempl()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(m_pWndParent)
	{
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
}

void CDialogSettings::OnBnClickedButtonExportTempl()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(m_pWndParent)
	{
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
}
