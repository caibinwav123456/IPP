// DlgClassify.cpp : implementation file
//

#include "stdafx.h"
#include "TextClassifier.h"
#include "DlgClassify.h"
#include "afxdialogex.h"


// CDlgClassify dialog

IMPLEMENT_DYNAMIC(CDlgClassify, CDialog)

CDlgClassify::CDlgClassify(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgClassify::IDD, pParent)
	, m_strText(_T("")),m_Classify(NULL)
{

}

CDlgClassify::~CDlgClassify()
{
}

void CDlgClassify::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CLASSIFY, m_strText);
}


BEGIN_MESSAGE_MAP(CDlgClassify, CDialog)
END_MESSAGE_MAP()


// CDlgClassify message handlers


void CDlgClassify::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	USES_CONVERSION;
	UpdateData(TRUE);
	WCHAR* w=T2W((LPTSTR)(LPCTSTR)m_strText);
	int lstr = lstrlenW(w)+1;
	char* buf = (char*)alloca(lstr*sizeof(WCHAR));
	*buf = 0;
	int nRet = WideCharToMultiByte(CP_UTF8, 0, w, -1, buf, lstr*sizeof(WCHAR), NULL, FALSE);
	ClassifyData corr;
	char* attrib=m_Classify->GetAttribute(buf, &corr);
	lstr = lstrlenA(corr.corr.c_str())+1;
	WCHAR* wbuf = (WCHAR*)alloca(lstr*sizeof(WCHAR));
	*wbuf = 0;
	nRet = MultiByteToWideChar(CP_UTF8, 0, corr.corr.c_str(), -1, wbuf, lstr);
	m_strText=W2T(wbuf);
	UpdateData(FALSE);
	if(attrib)
		MessageBox(A2T(attrib));
	else
		MessageBox(_T("no attrib found"));
	//CDialog::OnOK();
}
