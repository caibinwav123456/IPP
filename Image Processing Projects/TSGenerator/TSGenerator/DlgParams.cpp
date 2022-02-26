// DlgParams.cpp : implementation file
//

#include "stdafx.h"
#include "TSGenerator.h"
#include "DlgParams.h"
#include "afxdialogex.h"


// CDlgParams dialog

IMPLEMENT_DYNAMIC(CDlgParams, CDialog)

CDlgParams::CDlgParams(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgParams::IDD, pParent)
	, m_strCharSpace(_T(""))
	, m_strLineSpace(_T(""))
	, m_strHMargin(_T(""))
	, m_strVMargin(_T(""))
	, m_bOutputRowStats(FALSE)
{

}

CDlgParams::~CDlgParams()
{
}

void CDlgParams::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CHAR_SPACE, m_strCharSpace);
	DDX_Text(pDX, IDC_EDIT_LINE_SPACE, m_strLineSpace);
	DDX_Text(pDX, IDC_EDIT_H_MARGIN, m_strHMargin);
	DDX_Text(pDX, IDC_EDIT_V_MARGIN, m_strVMargin);
	DDX_Check(pDX, IDC_CHECK_ROWSTATS, m_bOutputRowStats);
	DDX_Control(pDX, IDC_CHECK_ROWSTATS, m_ChkOutputRowStats);
}


BEGIN_MESSAGE_MAP(CDlgParams, CDialog)
END_MESSAGE_MAP()

// CDlgParams message handlers
