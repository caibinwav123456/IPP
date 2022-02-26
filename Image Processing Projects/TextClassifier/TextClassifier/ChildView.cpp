
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "TextClassifier.h"
#include "ChildView.h"
#include "DlgClassify.h"
#include "ClassifyMain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView():m_bExcludeTags(false)
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_COMMAND(ID_FILE_CLASSIFY, &CChildView::OnFileClassify)
	ON_COMMAND(ID_FILE_LOAD, &CChildView::OnFileLoad)
	ON_COMMAND(ID_FILE_TESTOCR, &CChildView::OnFileTestocr)
	ON_COMMAND(ID_OPTIONS_EXCLUDETAGS, &CChildView::OnOptionsExcludetags)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_EXCLUDETAGS, &CChildView::OnUpdateOptionsExcludetags)
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
}



void CChildView::OnFileClassify()
{
	// TODO: Add your command handler code here
	CDlgClassify dlg;
	dlg.m_Classify=&m_Classify;
	dlg.DoModal();
}


void CChildView::OnFileLoad()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("All Files|*.*||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strPath=dlg.GetPathName();
		if(!m_Classify.Load(T2A(strPath)))
			MessageBox(_T("Load Config Failed"));
	}
}


void CChildView::OnFileTestocr()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Text Files|*.txt||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strPath=dlg.GetPathName();
		CString strBase;
		int idot=strPath.ReverseFind('.');
		if(idot!=-1)
			strBase=strPath.Left(idot);
		else
			strBase=strPath;
		TestOCR(T2A(strBase));
	}
}

void CChildView::TestOCR(char* textbase)
{
	ClassifyInterface inf;
	ClassifyRes res;
	inf.SetBasePath(".\\");
	inf.SetExcludeTag(m_bExcludeTags);
	inf.LoadConfig();
	inf.MasterClassify(textbase, res);
	string output_file=string(textbase)+"_seg.txt";
	if(FILE* fp=fopen(output_file.c_str(), "w"))
	{
		for(int i=0;i<(int)res.sorted_records.size();i++)
		{
			ClassRecord& rec=res.sorted_records[i];
			fprintf(fp, "%s\n\n", rec.tclass.c_str());
			for(int j=0;j<(int)rec.records.size();j++)
			{
				fprintf(fp, "\t%s\n", rec.records[j]->str.c_str());
			}
			fprintf(fp, "\n");
		}
		fclose(fp);
		MessageBox(_T("Classify complete!"));
	}
}

void CChildView::OnOptionsExcludetags()
{
	// TODO: Add your command handler code here
	m_bExcludeTags=!m_bExcludeTags;
}


void CChildView::OnUpdateOptionsExcludetags(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if(m_bExcludeTags)
		pCmdUI->SetCheck(BST_CHECKED);
	else
		pCmdUI->SetCheck(BST_UNCHECKED);
}
