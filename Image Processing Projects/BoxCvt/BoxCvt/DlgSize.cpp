// DlgSize.cpp : implementation file
//

#include "stdafx.h"
#include "BoxCvt.h"
#include "DlgSize.h"
#include "afxdialogex.h"


// CDlgSize dialog

IMPLEMENT_DYNAMIC(CDlgSize, CDialog)

CDlgSize::CDlgSize(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSize::IDD, pParent)
	, m_size(_T(""))
	, m_seg(_T(""))
	, m_overlap(_T(""))
{

}

CDlgSize::~CDlgSize()
{
}

void CDlgSize::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_size);
	DDX_Text(pDX, IDC_EDIT2, m_seg);
	DDX_Text(pDX, IDC_EDIT3, m_overlap);
}


BEGIN_MESSAGE_MAP(CDlgSize, CDialog)
END_MESSAGE_MAP()


// CDlgSize message handlers


void CDlgSize::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	UpdateData(TRUE);
	CDialog::OnOK();
}
