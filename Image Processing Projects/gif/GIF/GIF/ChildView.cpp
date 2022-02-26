
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "GIF.h"
#include "ChildView.h"
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif


// CChildView

CChildView::CChildView()
{
	gdiplustoken = 0;
	m_nFrames = 0;
	m_iFrame = 0;
	ZeroMemory(&m_nDimID, sizeof(GUID));
}

CChildView::~CChildView()
{
	
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
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
	dc.FillSolidRect(rcClient, RGB(255,255,255));
	// TODO: Add your message handler code here
	Draw(&dc);
	// Do not call CWnd::OnPaint() for painting messages
}


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	GdiplusStartup(&gdiplustoken, &input, NULL);
	m_Image = new Image(L"pageflip.gif");
	int nDimCount = m_Image->GetFrameDimensionsCount();
	GUID* nIDs = new GUID[nDimCount];
	ZeroMemory(nIDs, nDimCount*sizeof(GUID));
	m_Image->GetFrameDimensionsList(nIDs, nDimCount);
	m_nFrames = m_Image->GetFrameCount(&nIDs[0]);
	m_nDimID = nIDs[0];
	return 0;
}

void CChildView::OnDestroy()
{
	CWnd::OnDestroy();
	delete m_Image;
	GdiplusShutdown(gdiplustoken);
	// TODO: Add your message handler code here
}

void CChildView::Draw(CDC* pDC)
{
	HDC hdc = pDC->GetSafeHdc();
	Graphics graph(hdc);
	m_Image->SelectActiveFrame(&m_nDimID,m_iFrame);
	graph.DrawImage(m_Image, Rect(0,0,m_Image->GetWidth(),m_Image->GetHeight()));
	CString str;
	str.Format("%d", m_iFrame);
	pDC->SetBkMode(TRANSPARENT);
	pDC->TextOut(0,0,str);
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	CClientDC dc(this);
	CRect rcClient;
	GetClientRect(rcClient);
	dc.FillSolidRect(rcClient, RGB(255,255,255));

	switch(nChar)
	{
	case VK_LEFT:
		m_iFrame--;
		if(m_iFrame<0)m_iFrame=m_nFrames-1;
		Draw(&dc);
		break;
	case VK_RIGHT:
		m_iFrame++;
		if(m_iFrame>=m_nFrames)m_iFrame=0;
		Draw(&dc);
		break;
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
