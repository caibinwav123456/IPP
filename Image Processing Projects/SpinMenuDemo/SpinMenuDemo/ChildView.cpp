
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "SpinMenuDemo.h"
#include "ChildView.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PI 3.1415926535897932384626
#define X_AXIS     40.
#define Y_AXIS      4.
#define Z_AXIS     10.
#define VIEW_ORG_Z 30.
#define MENU_WIDTH  10.
#define MENU_HEIGHT 16.
// CChildView

CChildView::CChildView()
{
	m_Angle = 0;
	m_Speed = 0;
	m_iItem = 0;
	oldbmp = NULL;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
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
	CPaintDC dc(this); // device context for painting
	
	Draw(&dc);
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
}

void CChildView::Draw(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(rcClient);
	m_mdc.FillSolidRect(0,0,rcClient.Width(),rcClient.Height(),RGB(0,0,0));
	float angle[10];
	float index[10];
	for(int i=0;i<10;i++)
	{
		angle[i]=m_Angle+i*2*PI/10;
		index[i]=i;
	}
	for(int i=0;i<9;i++)
	{
		for(int j=i+1;j<10;j++)
		{
			if(ComputeDepth(angle[i])<ComputeDepth(angle[j]))
			{
				float tmp=angle[i];
				angle[i]=angle[j];
				angle[j]=tmp;
				int ntmp=index[i];
				index[i]=index[j];
				index[j]=ntmp;
			}
		}
	}
	for(int i=0;i<10;i++)
	{
		DrawItem(&m_mdc, angle[i], index[i]);
	}
	pDC->BitBlt(0,0,rcClient.Width(),rcClient.Height(),&m_mdc,0,0,SRCCOPY);
}

void CChildView::DrawItem(CDC* pDC, float fAngle, int index)
{
	CRect rcClient;
	GetClientRect(rcClient);
	float x=X_AXIS*sin(fAngle);
	float y=-Y_AXIS*cos(fAngle);
	float z=-Z_AXIS*cos(fAngle);
	z+=VIEW_ORG_Z;

	BITMAP bm;
	m_bmObj[index].GetBitmap(&bm);
	if(z>0)
	{
		CRect rcItem( (x-MENU_WIDTH/2) /z*300+rcClient.Width()/2,
        			 -(y+MENU_HEIGHT/2)/z*300+rcClient.Height()/2,
			          (x+MENU_WIDTH/2) /z*300+rcClient.Width()/2,
					 -(y-MENU_HEIGHT/2)/z*300+rcClient.Height()/2);
		CBitmap* old = m_objdc.SelectObject(&m_bmObj[index]);

		pDC->SetStretchBltMode(COLORONCOLOR);
		pDC->StretchBlt(rcItem.left,rcItem.top,rcItem.Width(),rcItem.Height(),&m_objdc,0,0,bm.bmWidth,bm.bmHeight,SRCCOPY);

		m_objdc.SelectObject(old);
	}

}

float CChildView::ComputeDepth(float fAngle)
{
	return VIEW_ORG_Z-Z_AXIS*cos(fAngle);
}
int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	//m_bmObj.LoadBitmap(MAKEINTRESOURCE(IDB_BITMAP1));
	char name[50];
	for(int i=0;i<10;i++)
	{
		sprintf_s(name,"%d.bmp",i+1);
		m_bmObj[i].m_hObject = LoadImage(NULL, name, IMAGE_BITMAP, 400, 300, LR_LOADFROMFILE);
	}
	CClientDC dc(this);
	m_mdc.CreateCompatibleDC(&dc);
	HDC hdc=CreateDC("DISPLAY",NULL,NULL,NULL);
	int hres=GetDeviceCaps(hdc, HORZRES);
	int vres=GetDeviceCaps(hdc, VERTRES);
	m_bmBuf.CreateCompatibleBitmap(&dc, hres, vres);
	oldbmp = m_mdc.SelectObject(&m_bmBuf);

	m_objdc.CreateCompatibleDC(&dc);

	return 0;
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect rcClient;
	GetClientRect(rcClient);
	if(point.x<rcClient.Width()/2)
	{
		m_Speed = 0.03;
		m_iItem++;
		if(m_iItem>=10)
		{
			m_iItem=0;
			m_Angle-=2*PI;
		}
	}
	else
	{
		m_Speed = -0.03;
		m_iItem--;
		if(m_iItem<0)
		{
			m_iItem=9;
			m_Angle+=2*PI;
		}
	}
	SetTimer(1,1,NULL);
	CWnd::OnLButtonDown(nFlags, point);
}

void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	m_Angle+=m_Speed;
	if(m_Speed>0)
	{
		if(m_Angle>=m_iItem*2*PI/10)
		{
			m_Angle = m_iItem*2*PI/10;
			KillTimer(1);
		}
	}
	else if(m_Speed<0)
	{
		if(m_Angle<=m_iItem*2*PI/10)
		{
			m_Angle = m_iItem*2*PI/10;
			KillTimer(1);
		}
	}
	CClientDC dc(this);
	Draw(&dc);
	CWnd::OnTimer(nIDEvent);
}

void CChildView::OnDestroy()
{
	CWnd::OnDestroy();
	if(oldbmp!=NULL)
	{
		m_mdc.SelectObject(oldbmp);
	}
	m_mdc.DeleteDC();
	m_objdc.DeleteDC();
	m_bmBuf.DeleteObject();
	for(int i=0;i<10;i++)
		m_bmObj[i].DeleteObject();
	// TODO: Add your message handler code here
}
