
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "ClipPath.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
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
	CPaintDC dc1(this); // device context for painting
	
	// TODO: Add your message handler code here
	CClientDC dc(this);
	dc.BeginPath();
	dc.Ellipse(100,100,200,200);
	dc.EndPath();
	//dc.SelectStockObject(BLACK_BRUSH);
	//dc.StrokePath();
	/*int n=dc.GetPath(NULL, NULL, 0);
	CPoint* pt = new CPoint[n];
	BYTE* b = new BYTE[n];
	dc.GetPath(pt, b, n);
	for(int i=0;i<100;i++)
	{
		CPoint p=pt[i];
		BYTE bi = b[i];
		int o=0;
		o++;
	}*/
	CRect rcClient;
	GetClientRect(rcClient);
	CRgn rgn;
	BOOL bo = rgn.CreateFromPath(&dc);
	dc1.SelectClipRgn(&rgn);
	//dc1.SelectStockObject(BLACK_BRUSH);
	//dc1.Rectangle(rcClient);
	//dc1.FillSolidRect(rcClient, RGB(0,0,0));
	CDC dcobj;
	dcobj.CreateCompatibleDC(&dc);
	CBitmap* oldbmp = dcobj.SelectObject(&m_bmp);
	dc1.BitBlt(0,0,rcClient.Width(),rcClient.Height(), &dcobj, 0,0,SRCCOPY);
	dcobj.SelectObject(oldbmp);
	dcobj.DeleteDC();
	// Do not call CWnd::OnPaint() for painting messages
}



int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_bmp.Attach((HGDIOBJ)LoadImage(NULL, _T("Desert.bmp"), IMAGE_BITMAP, 1024,768, LR_LOADFROMFILE));
	CFile file;
	if(file.Open(_T("han.txt"),CFile::modeCreate|CFile::modeWrite))\
	{
		for(unsigned short i=0x4e00;i<=0x9fa5;i++)
		{
			file.Write(&i, 2);
		}
		file.Close();
	}
	return 0;
}


void CChildView::OnDestroy()
{
	CWnd::OnDestroy();
	m_bmp.DeleteObject();
	// TODO: Add your message handler code here
}
