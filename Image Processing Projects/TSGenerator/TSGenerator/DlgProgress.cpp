// DlgProgress.cpp : implementation file
//

#include "stdafx.h"
#include "TSGenerator.h"
#include "DlgProgress.h"
#include "afxdialogex.h"
#include "TextUtility.h"

#define WM_SET_PROGRESS (WM_USER+100)
UINT ThreadFunc(LPVOID param);
// DlgProgress dialog

IMPLEMENT_DYNAMIC(DlgProgress, CDialog)

DlgProgress::DlgProgress(CWnd* pParent /*=NULL*/)
	: CDialog(DlgProgress::IDD, pParent)
{
	m_bEnd=false;
	m_hEvent=0;
}

DlgProgress::~DlgProgress()
{
}

void DlgProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
}


BEGIN_MESSAGE_MAP(DlgProgress, CDialog)
	ON_MESSAGE(WM_SET_PROGRESS, OnSetPos)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &DlgProgress::OnBnClickedButtonCancel)
END_MESSAGE_MAP()

UINT ThreadFunc(LPVOID param)
{
	USES_CONVERSION;
	DlgProgress* pProg=(DlgProgress*)param;
	CChildView* pView=pProg->m_View;
	char buf[4096];
	char buf2[4096];
	CString BasePath=pProg->m_BatchFile;
	int iSlash=BasePath.ReverseFind('\\');
	if(iSlash!=-1)
		BasePath=BasePath.Left(iSlash)+_T("\\");
	if(FILE* file=fopen(T2A(pProg->m_BatchFile), "r"))
	{
		int cnt=0;
		while(fgets(buf, 4096, file))
		{
			if(IsLineEnd(buf))
				continue;
			cnt++;
		}
		cnt*=2;
		rewind(file);
		int n=0;
		pProg->HandleStats(true, false);
		while(fgets(buf, 4096, file))
		{
			if(IsLineEnd(buf))
				continue;
			char* p=buf;
			GetNextToken(p, buf2);
			CString FontPath=W2T((WCHAR*)CharToWchar(buf2).c_str());
			if(FontPath.Find(':')==-1)
				FontPath=BasePath+FontPath;
			GetNextToken(p, buf2);
			CString OutPath=W2T((WCHAR*)CharToWchar(buf2).c_str());
			if(OutPath.Find(':')==-1)
				OutPath=BasePath+OutPath;
			pProg->HandleStats(false, true);
			pProg->ParseArgs(p);
			CString info;
			n++;
			pProg->PostMessage(WM_SET_PROGRESS, n*100/cnt);
			info.Format(_T("Loading Font File...%s"), FontPath);
			pProg->GetDlgItem(IDC_STATIC)->SetWindowText(info);
			if(!pView->LoadFace(FontPath, true))
			{
				pProg->GetDlgItem(IDC_STATIC)->SetWindowText(_T("Load Font File Failed."));
				continue;
			}
			if(pProg->m_bEnd)
				break;
			n++;
			pProg->PostMessage(WM_SET_PROGRESS, n*100/cnt);
			info.Format(_T("Saving Output File...%s"), OutPath);
			pProg->GetDlgItem(IDC_STATIC)->SetWindowText(info);
			if(!pView->Save(OutPath))
				pProg->GetDlgItem(IDC_STATIC)->SetWindowText(_T("Save Output Failed."));
			if(pProg->m_bEnd)
				break;
		}
		fclose(file);
		pProg->HandleStats(false,false);
		pProg->PostMessage(WM_CLOSE);
		SetEvent(pProg->m_hEvent);
	}
	return 0;
}
// DlgProgress message handlers


BOOL DlgProgress::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_Progress.SetRange(0, 100);
	m_Progress.SetPos(0);
	m_hEvent=CreateEvent(NULL, TRUE, FALSE, _T("fg"));
	AfxBeginThread(ThreadFunc, this);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void DlgProgress::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	m_bEnd=true;
	if(m_hEvent)
		WaitForSingleObject(m_hEvent, INFINITE);
	CloseHandle(m_hEvent);
	m_hEvent=0;
	CDialog::OnCancel();
}


LRESULT DlgProgress::OnSetPos(WPARAM wParam, LPARAM lParam)
{
	m_Progress.SetPos(wParam);
	return 0;
}

void DlgProgress::ParseArgs(char* p)
{
	char buf[4096];
	while(GetNextToken(p, buf))
	{
		if(strcmp(buf, "-bl")==0)
			m_View->m_bSaveRowStats=true;
		else if(strcmp(buf, "-cs")==0)
		{
			GetNextToken(p, buf);
			sscanf(buf, "%d", &m_View->m_CharSpace);
			if(m_View->m_CharSpace<0)
				m_View->m_CharSpace=0;
		}
		else if(strcmp(buf, "-ls")==0)
		{
			GetNextToken(p, buf);
			sscanf(buf, "%d", &m_View->m_LineSpace);
			if(m_View->m_LineSpace<0)
				m_View->m_LineSpace=0;
		}
		else if(strcmp(buf, "-hm")==0)
		{
			GetNextToken(p, buf);
			sscanf(buf, "%d", &m_View->m_org.x);
			if(m_View->m_org.x<0)
				m_View->m_org.x=0;
		}
		else if(strcmp(buf, "-vm")==0)
		{
			GetNextToken(p, buf);
			sscanf(buf, "%d", &m_View->m_org.y);
			if(m_View->m_org.y<0)
				m_View->m_org.y=0;
		}
	}
}

void DlgProgress::HandleStats(bool bSave, bool bClean)
{
	if(bClean)
	{
		m_View->m_bSaveRowStats=false;
		m_View->m_CharSpace=20;
		m_View->m_LineSpace=20;
		m_View->m_org=CPoint(20,20);
	}
	else if(bSave)
	{
		m_bSaveRowStats=m_View->m_bSaveRowStats;
		m_CharSpace=m_View->m_CharSpace;
		m_LineSpace=m_View->m_LineSpace;
		m_org=m_View->m_org;
	}
	else
	{
		m_View->m_bSaveRowStats=m_bSaveRowStats;
		m_View->m_CharSpace=m_CharSpace;
		m_View->m_LineSpace=m_LineSpace;
		m_View->m_org=m_org;
	}
}

void DlgProgress::OnBnClickedButtonCancel()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_STATIC)->SetWindowText(_T("Canceling Operations..."));
	OnCancel();
}
