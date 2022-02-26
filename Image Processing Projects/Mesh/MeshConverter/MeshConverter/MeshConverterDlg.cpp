
// MeshConverterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MeshConverter.h"
#include "MeshConverterDlg.h"
#include "CreateMesh.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMeshConverterDlg dialog

IDirect3DDevice9* device = NULL;


CMeshConverterDlg::CMeshConverterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMeshConverterDlg::IDD, pParent)
	, m_strSrc(_T(""))
	, m_strDest(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_b32Bit = false;
}

void CMeshConverterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SRC, m_strSrc);
	DDX_Text(pDX, IDC_EDIT_DEST, m_strDest);
	DDX_Control(pDX, IDC_CHECK_32BIT, m_Check);
}

BEGIN_MESSAGE_MAP(CMeshConverterDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SRC, &CMeshConverterDlg::OnBnClickedButtonBrowseSrc)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_DEST, &CMeshConverterDlg::OnBnClickedButtonBrowseDest)
	ON_BN_CLICKED(IDC_BUTTON_CONVERT, &CMeshConverterDlg::OnBnClickedButtonConvert)
	ON_BN_CLICKED(IDC_CHECK_32BIT, &CMeshConverterDlg::OnBnClickedCheck32bit)
END_MESSAGE_MAP()


// CMeshConverterDlg message handlers

BOOL CMeshConverterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_Check.SetCheck(BST_UNCHECKED);
	// TODO: Add extra initialization here
	InitD3D(800,600, m_hWnd, &device);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMeshConverterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMeshConverterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMeshConverterDlg::OnDestroy()
{
	CDialog::OnDestroy();
	device->Release();
	// TODO: Add your message handler code here
}

void CMeshConverterDlg::OnBnClickedButtonBrowseSrc()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(true, NULL, NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "X Files|*.x|All Files|*.*||", this);
	if(dlg.DoModal() == IDOK)
	{
		m_strSrc = dlg.GetPathName();
		int n = m_strSrc.ReverseFind('.');
		if(n!=-1)
		{
			m_strDest = m_strSrc.Left(n)+".mesh";
		}
		else
		{
			m_strDest = m_strSrc+".mesh";
		}
		UpdateData(FALSE);
	}
}

void CMeshConverterDlg::OnBnClickedButtonBrowseDest()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CFileDialog dlg(false, ".mesh", m_strDest, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Mesh Files|*.mesh|All Files|*.*||", this);
	if(dlg.DoModal() == IDOK)
	{
		m_strDest = dlg.GetPathName();
	}
}

void CMeshConverterDlg::OnBnClickedButtonConvert()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(m_strSrc != "" && m_strDest != "")
	{
		CreateMeshFile(device, (LPSTR)(LPCTSTR)m_strSrc, (LPSTR)(LPCTSTR)m_strDest, m_b32Bit ? IF_INDEX32 : IF_INDEX16);
	}
}

void CMeshConverterDlg::OnBnClickedCheck32bit()
{
	// TODO: Add your control notification handler code here
	if(BST_CHECKED == m_Check.GetCheck())
	{
		m_b32Bit = true;
	}
	else if(BST_UNCHECKED == m_Check.GetCheck())
	{
		m_b32Bit = false;
	}
}
