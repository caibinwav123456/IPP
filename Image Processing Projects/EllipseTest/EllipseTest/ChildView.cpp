
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "EllipseTest.h"
#include "ChildView.h"
#include "math.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning(disable:4244)
#pragma warning(disable:4305)
// CChildView

CChildView::CChildView()
{
	a=1;
	b=1;
	talpha = 0;

	m=1;
	n=1;
	p=0;

	X=1;
	Y=1;
	S=0;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
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
	
	// TODO: Add your message handler code here
	GenerateEllipse();
	DrawEllipse();
	DrawBoundingBox();
	// Do not call CWnd::OnPaint() for painting messages
}

float CChildView::Delta()
{
	return sqrt((m-n)*(m-n)+p*p)-(m-n);
}

void CChildView::CvtA2M()
{
	float sin2 = talpha*talpha/(1+talpha*talpha);
	float cos2 = 1/(talpha*talpha+1);
	float sincos = talpha/(1+talpha*talpha);
	m=cos2/a/a+sin2/b/b;
	n=sin2/a/a+cos2/b/b;
	p=2*sincos*(1/a/a-1/b/b);
}

void CChildView::CvtM2A()
{
	float delta = Delta();
	float sqra = (p*p+2*n*delta)/2/delta;
	if(delta == 0)
		sqra = n;
	a = sqrt(1/sqra);
	float sqrb = (-p*p+2*m*delta)/2/delta;
	if(delta == 0)
		sqrb = m;
	b = sqrt(1/sqrb);
	talpha = delta/p;
}

void CChildView::CvtX2M()
{
	m = S*S/Y/Y+1/X/X;
	n = 1/Y/Y;
	p = -2*S/Y/Y;
}

void CChildView::CvtM2X()
{
	float sqrX = m-p*p/4/n;
	X = sqrt(1/sqrX);
	float sqrY = n;
	Y = sqrt(1/sqrY);
	S = -p/2/n;
}

#define PI 3.1415926535897932384626
void CChildView::GenerateEllipse()
{
	float sina = talpha/sqrt(talpha*talpha+1);
	float cosa = 1/sqrt(talpha*talpha+1);

	for(int i=0;i<100;i++)
	{
		float theta = 2*PI*i/100;
		float tmpx = cos(theta)*a;
		float tmpy = sin(theta)*b;
		x[i] = tmpx*cosa-tmpy*sina;
		y[i] = tmpx*sina+tmpy*cosa;
	}
}

#define UNIT 50
void CChildView::DrawEllipse()
{
	CRect rcClient;
	GetClientRect(rcClient);
	CClientDC dc(this);
	CPoint *ptlist = new CPoint[100];
	for(int i=0;i<100;i++)
	{
		ptlist[i].x = rcClient.Width()/2+x[i]*UNIT;
		ptlist[i].y = rcClient.Height()/2+y[i]*UNIT;
	}
	dc.SelectStockObject(NULL_BRUSH);
	dc.Polygon(ptlist, 100);
	delete[] ptlist;
}

void CChildView::DrawBoundingBox()
{
	CRect rcClient;
	GetClientRect(rcClient);
	CClientDC dc(this);
	CPoint ptlist[4];
	ptlist[0] = CPoint(-X*UNIT, (Y-X*S)*UNIT);
	ptlist[1] = CPoint(X*UNIT, (Y+X*S)*UNIT);
	ptlist[2] = CPoint(X*UNIT, (-Y+X*S)*UNIT);
	ptlist[3] = CPoint(-X*UNIT, (-Y-X*S)*UNIT);
	for(int i=0;i<4;i++)
	{
		ptlist[i]+=rcClient.CenterPoint();
	}
	dc.SelectStockObject(NULL_BRUSH);
	dc.Polygon(ptlist, 4);
}


void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	switch(nChar)
	{
	case 'A':
		a+=0.01;
		break;
	case 'Z':
		a-=0.01;
		break;
	case 'S':
		b+=0.01;
		break;
	case 'X':
		b-=0.01;
		break;
	case 'D':
		talpha = tan(atan(talpha)+0.01);
		break;
	case 'C':
		talpha = tan(atan(talpha)-0.01);
		break;
	case 'F':
		X+=0.01;
		break;
	case 'V':
		X-=0.01;
		break;
	case 'G':
		Y+=0.01;
		break;
	case 'B':
		Y-=0.01;
		break;
	case 'H':
		S+=0.01;
		break;
	case 'N':
		S-=0.01;
		break;
	}

	switch(nChar)
	{
	case 'A':
	case 'Z':
	case 'S':
	case 'X':
	case 'D':
	case 'C':
		CvtA2M();
		CvtM2X();
		break;
	case 'F':
	case 'V':
	case 'G':
	case 'B':
	case 'H':
	case 'N':
		CvtX2M();
		CvtM2A();
	}
	Invalidate();
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
