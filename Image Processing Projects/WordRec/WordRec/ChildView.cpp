
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "WordRec.h"
#include "ChildView.h"
#include "stdlib.h"
#include "search.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define ALGOR1

// CChildView
int g_deb = 0;
CChildView::CChildView()
{
	m_nSelCt = -1;
	m_nSel = -1;
	m_bShowAllContours = false;
	m_bRegressStarted = false;
	m_bAutoRun = false;
	m_lines = NULL;
	m_nline = 0;
	m_bShowConnLines = false;
	m_nShowRect = -1;
}

CChildView::~CChildView()
{
	if(m_lines)
		delete[] m_lines;
}

BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_OPEN, &CChildView::OnFileOpen)
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
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
	CRect rcClient;
	GetClientRect(rcClient);
	// TODO: Add your message handler code here
	CBitmap* oldBmp = m_dc.SelectObject(&m_bmp);
	m_dc.FillSolidRect(rcClient, RGB(255,255,255));
	Draw(&m_dc);
	dc.BitBlt(0,0,rcClient.Width(), rcClient.Height(), &m_dc, 0,0,SRCCOPY);
	m_dc.SelectObject(oldBmp);
	// Do not call CWnd::OnPaint() for painting messages
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_Recog.LoadImage("truck.jpg");
#ifdef ALGOR1
	m_Recog.Process(FIND_COMPS_ALGORITHM1 | FIND_LINES_ALGORITHM1);
#else
	m_Recog.Process(FIND_COMPS_ALGORITHM2 | FIND_LINES_ALGORITHM2);
#endif
	CClientDC dc(this);
	m_dc.CreateCompatibleDC(&dc);
	m_bmp.CreateCompatibleBitmap(&dc, 1024, 768);

	return 0;
}

void CChildView::Draw(CDC* pDC)
{
#ifdef ALGOR1
	IplImage* image = cvCreateImage(cvGetSize(m_Recog.m_bin), IPL_DEPTH_8U, 3);
	cvMerge(m_Recog.m_bin, m_Recog.m_bin, m_Recog.m_bin, NULL, image);
#else
	IplImage* tmp = cvCreateImage(cvGetSize(m_Recog.m_src), IPL_DEPTH_8U, 1);
	cvResize(m_Recog.m_seg, tmp);
	IplImage* image = cvCreateImage(cvGetSize(m_Recog.m_src), IPL_DEPTH_8U, 3);
	cvMerge(tmp, tmp, tmp, NULL, image);
	cvReleaseImage(&tmp);
#endif
	DispImage(pDC, image);
	CPen rPen, yPen, gPen, bPen;
	rPen.CreatePen(PS_SOLID, 1, RGB(255,0,0));
	yPen.CreatePen(PS_SOLID, 1, RGB(128,128,0));
	gPen.CreatePen(PS_SOLID, 1, RGB(0,255,0));
	bPen.CreatePen(PS_SOLID, 1, RGB(0,0,255));
	CPen* oldPen = pDC->SelectObject(&gPen);
	vector<Quad>& quadlist = m_Recog.m_QuadList;
	for(int i=0;i<(int)quadlist.size();i++)
	{
		Vec2 v00 = quadlist[i]._00;
		Vec2 v01 = quadlist[i]._01;
		Vec2 v10 = quadlist[i]._10;
		Vec2 v11 = quadlist[i]._11;

		pDC->MoveTo(CPoint(v00.x, v00.y));
		pDC->LineTo(CPoint(v01.x, v01.y));
		pDC->LineTo(CPoint(v11.x, v11.y));
		pDC->LineTo(CPoint(v10.x, v10.y));
		pDC->LineTo(CPoint(v00.x, v00.y));
	}
	pDC->SelectObject(&rPen);
	if(m_bShowAllContours)
		m_Recog.DrawComps(pDC);
	if(m_nSelCt != -1)
	{
		pDC->SelectObject(&yPen);
		m_Recog.DrawComps(pDC, m_nSelCt, CPoint(0,0), false);
		Line* lines = NULL;
		int   nline = 0;
#ifdef ALGOR1
		lines = m_Recog.m_lines[m_nSelCt];
		nline = m_Recog.m_nlines[m_nSelCt];
		if(lines)
		{
			//pDC->SelectObject(&yPen);
			for(int i=0;i<nline;i++)
			{
				Line line = lines[i];
				CPoint start(line.dist*cos(line.theta)-line.start*sin(line.theta),
					line.dist*sin(line.theta)+line.start*cos(line.theta));
				CPoint end(line.dist*cos(line.theta)-line.end*sin(line.theta),
					line.dist*sin(line.theta)+line.end*cos(line.theta));

				pDC->MoveTo(start);
				pDC->LineTo(end);
			}
		}
#endif
		pDC->SelectObject(&rPen);
		lines = m_Recog.m_reflines[m_nSelCt];
		nline = m_Recog.m_nreflines[m_nSelCt];
		if(lines)
		{
			for(int i=0;i<nline;i++)
			{
				Line line = lines[i];
				CPoint start(line.dist*cos(line.theta)-line.start*sin(line.theta),
					line.dist*sin(line.theta)+line.start*cos(line.theta));
				CPoint end(line.dist*cos(line.theta)-line.end*sin(line.theta),
					line.dist*sin(line.theta)+line.end*cos(line.theta));

				pDC->MoveTo(start);
				pDC->LineTo(end);
			}
		}
	}
	if(m_bShowConnLines)
	{
		pDC->SelectObject(&yPen);
		if(g_deb < 0)
			g_deb = 0;
		int start = (int)m_Recog.m_AllLines.size() - g_deb;
		if(start < 0)start = 0;
		//for(int i=0;i<start;i++)
		for(int i=start;i<(int)m_Recog.m_AllLines.size();i++)
		{
			Line line = m_Recog.m_AllLines[i];
			CPoint start(line.dist*cos(line.theta)-line.start*sin(line.theta),
				line.dist*sin(line.theta)+line.start*cos(line.theta));
			CPoint end(line.dist*cos(line.theta)-line.end*sin(line.theta),
				line.dist*sin(line.theta)+line.end*cos(line.theta));

			pDC->MoveTo(start);
			pDC->LineTo(end);
		}
	}
	if(m_nShowRect<0 || m_nShowRect>(int)m_Recog.m_QuadList.size())
	{
		m_nShowRect = -1;
	}
	else
	{
		pDC->SelectObject(&rPen);
		Vec2 v00 = quadlist[m_nShowRect]._00;
		Vec2 v01 = quadlist[m_nShowRect]._01;
		Vec2 v10 = quadlist[m_nShowRect]._10;
		Vec2 v11 = quadlist[m_nShowRect]._11;

		pDC->MoveTo(CPoint(v00.x, v00.y));
		pDC->LineTo(CPoint(v01.x, v01.y));
		pDC->LineTo(CPoint(v11.x, v11.y));
		pDC->LineTo(CPoint(v10.x, v10.y));
		pDC->LineTo(CPoint(v00.x, v00.y));

	}
#ifndef ALGOR1
	if(m_nSel)
	{
		pDC->SelectObject(&bPen);
		m_Recog.DrawComps(pDC, m_nSel);
	}
/*	if(m_lines)
	{
		pDC->SelectObject(&gPen);
		for(int i=0;i<m_nline;i++)
		{
			Line& line = m_lines[i];
			CPoint ptstart(line.dist*cos(line.theta)-line.start*sin(line.theta), 
				line.dist*sin(line.theta)+line.start*cos(line.theta));
			CPoint ptend(line.dist*cos(line.theta)-line.end*sin(line.theta),
				line.dist*sin(line.theta)+line.end*cos(line.theta));
			pDC->MoveTo(ptstart);
			pDC->LineTo(ptend);
		}
	}
*/	pDC->SelectObject(&rPen);
	m_Recog.DrawSpots(pDC, m_nSel);
	pDC->SelectObject(&gPen);
	DrawDetLines(pDC);
#endif
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(255,255,0));
	CString str;
	str.Format("%d, %d", m_nSelCt, m_nShowRect);
	pDC->TextOut(0,0,str);
//	m_Recog.DrawComps(pDC);
	pDC->SelectObject(oldPen);
	cvReleaseImage(&image);
}

void CChildView::DrawDetLines(CDC* pDC)
{
	LineListNode* linelist = m_Recog.m_linelist;
	CPen wPen;
	wPen.CreatePen(PS_SOLID, 1, RGB(255,255,255));

	if(linelist != NULL)
	{
		LineListNode* iter = linelist;
		do 
		{
			Line& line = iter->line;
			CPoint ptstart(line.dist*cos(line.theta)-line.start*sin(line.theta), 
						   line.dist*sin(line.theta)+line.start*cos(line.theta));
			CPoint ptend(line.dist*cos(line.theta)-line.end*sin(line.theta),
						 line.dist*sin(line.theta)+line.end*cos(line.theta));
			CPen* oldPen = NULL;
			if(iter == m_Recog.m_curline)
			{
				oldPen = pDC->SelectObject(&wPen);
			}
			pDC->MoveTo(ptstart);
			pDC->LineTo(ptend);

			if(oldPen)
			{
				pDC->SelectObject(oldPen);
			}

			iter = iter->next;
		} while (iter != linelist);
	}
}

void CChildView::Draw()
{
	CClientDC dc(this);
	CRect rcClient;
	GetClientRect(rcClient);
	CBitmap* oldBmp = m_dc.SelectObject(&m_bmp);
	m_dc.FillSolidRect(rcClient, RGB(255,255,255));
	Draw(&m_dc);
	dc.BitBlt(0,0,rcClient.Width(), rcClient.Height(), &m_dc, 0,0,SRCCOPY);
	m_dc.SelectObject(oldBmp);
}

void CChildView::DispImage(CDC* pDC, IplImage* image, CPoint ptBase)
{
	ASSERT(image && image->depth==IPL_DEPTH_8U && image->nChannels == 3);

	if(!(image && image->depth==IPL_DEPTH_8U && image->nChannels == 3))
		return;

	BITMAPINFO bi;
	ZeroMemory(&bi,sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = image->width;
	bi.bmiHeader.biHeight = -image->height;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 24;
	bi.bmiHeader.biCompression = BI_RGB;

	SetDIBitsToDevice(pDC->m_hDC, ptBase.x, ptBase.y, image->width, image->height,
		0, 0, 0, image->height, (void*)(image->imageData),&bi, DIB_RGB_COLORS);
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_nSelCt = m_Recog.IndexFromPoint(point);
	Draw();
	CWnd::OnLButtonDown(nFlags, point);
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_nSelCt = m_Recog.IndexFromPoint(point);
#ifndef ALGOR1
	//FindLines(m_nSelCt);
#endif
	Draw();
	CWnd::OnMouseMove(nFlags, point);
}

void CChildView::OnDestroy()
{
	CWnd::OnDestroy();
	m_dc.DeleteDC();
	m_bmp.DeleteObject();
	// TODO: Add your message handler code here
}

void CChildView::OnFileOpen()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("All Image Files(*.bmp;*.jpg;*.png)|*.bmp;*.jpg;*.png||"), this);
	if(dlg.DoModal() == IDOK)
	{
		CString strFileName = dlg.GetPathName();
		m_Recog.LoadImage(T2A((LPTSTR)(LPCTSTR)strFileName));
#ifdef ALGOR1
		m_Recog.Process(FIND_COMPS_ALGORITHM1 | FIND_LINES_ALGORITHM1);
#else
		m_Recog.Process(FIND_COMPS_ALGORITHM2 | FIND_LINES_ALGORITHM2);
#endif
		m_nSelCt = m_nSel = -1;
		m_bRegressStarted = false;
		KillTimer(1);
		Invalidate();
	}
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	switch(nChar)
	{
	case 'A':
		m_bShowAllContours = !m_bShowAllContours;
		Draw();
		break;
#ifndef ALGOR1
	case 'F':
		m_Recog.EndLineRegression();
		m_bRegressStarted = false;
		m_nSel = m_nSelCt;
		if(m_nSel != -1)
		{
			m_bRegressStarted = true;
			m_Recog.StartRegression(m_nSel);
		}
		Draw();
		break;
	case 'C':
		if(m_bRegressStarted && m_nSel != -1)
		{
			m_Recog.LineRegression();
		}
		Draw();
		break;
	case 'V':
		if(m_bAutoRun)
		{
			KillTimer(1);
			m_bAutoRun = false;
		}
		else
		{
			SetTimer(1, 1, NULL);
			m_bAutoRun = true;
		}
		break;
#endif
	case 'B':
		m_bShowConnLines = !m_bShowConnLines;
		Draw();
		break;
	case 'Q':
		m_nShowRect++;
		if(m_nShowRect<0)m_nShowRect = 0;
		else if(m_nShowRect>=(int)m_Recog.m_QuadList.size())
			m_nShowRect = 0;
		Draw();
		break;
	case 'W':
		m_nShowRect--;
		if(m_nShowRect<0)
			m_nShowRect = (int)m_Recog.m_QuadList.size()-1;
		Draw();
		break;
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	if(nIDEvent == 1)
	{
		if(m_bRegressStarted && m_nSel != -1)
		{
			for(int i=0;i<20;i++)
				m_Recog.LineRegression();
			Draw();
		}
	}
	CWnd::OnTimer(nIDEvent);
}

void CChildView::FindLines(int index)
{
	if(m_lines)
	{
		delete[] m_lines;
		m_lines = NULL;
	}
	m_nline = 0;
	if(index<0||index>=(int)m_Recog.m_Comps.size())
		return;
	vector<CvPoint>* contour = m_Recog.m_Comps[index];
	if((int)contour->size() == 0)
		return;
	CvPoint* ptlist = new CvPoint[contour->size()];
	for(int i=0;i<(int)contour->size();i++)
	{
		ptlist[i] = (*contour)[i];
	}

	m_Recog.LineRegression(ptlist, (int)contour->size(), &m_lines, &m_nline);
	
	delete[] ptlist;
}