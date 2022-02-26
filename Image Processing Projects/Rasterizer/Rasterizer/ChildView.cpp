
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "ImageProcess3D.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int IMAGE_WIDTH  =800;
int IMAGE_HEIGHT =600;
// CChildView
#define SAMPLE
IplImage* g_texture=NULL;
LARGE_INTEGER lnFreg = {0};
void Pixel(RawImage* target,CvPoint pos, int count, float* pdata)
{
	CvPoint2D32f tex=cvPoint2D32f(pdata[0],pdata[1]);
	CPoint pt(tex.x, tex.y);
	if(pt.x<0)pt.x=0;
	else if(pt.x>=g_texture->width)pt.x=g_texture->width-1;
	if(pt.y<0)pt.y=0;
	else if(pt.y>=g_texture->height)pt.y=g_texture->height-1;
#ifdef SAMPLE
	CvScalar pSamp = Sample(g_texture, tex);
	*((uchar*)(target->imgdata)+pos.y*target->widthStep+pos.x*3)  = pSamp.val[0];
	*((uchar*)(target->imgdata)+pos.y*target->widthStep+pos.x*3+1)= pSamp.val[1];
	*((uchar*)(target->imgdata)+pos.y*target->widthStep+pos.x*3+2)= pSamp.val[2];

#else
	*((uchar*)(target->imgdata)+pos.y*target->widthStep+pos.x*3)=
	*((uchar*)(g_texture->imageData)+pt.y*g_texture->widthStep+pt.x*3);
	*((uchar*)(target->imgdata)+pos.y*target->widthStep+pos.x*3+1)=
	*((uchar*)(g_texture->imageData)+pt.y*g_texture->widthStep+pt.x*3+1);
	*((uchar*)(target->imgdata)+pos.y*target->widthStep+pos.x*3+2)=
	*((uchar*)(g_texture->imageData)+pt.y*g_texture->widthStep+pt.x*3+2);
#endif
}

CChildView::CChildView()
{
/*
	CvPoint2D32f vlist[NVERT]={{100, 50}, {300, 50}, {300, 250}, {100, 250}};
	for(int i=0;i<NVERT;i++)
		m_vlist[i] = vlist[i];
	int ib[3*NFACE] = {0,1,2,2,3,0};
	for(int i=0;i<3*NFACE;i++)
		m_ibuf[i] = ib[i];
*/
	for(int i=0;i<NSEG+1;i++)
		for(int j=0;j<NSEG+1;j++)
		{
			m_vlist[i*(NSEG+1)+j] = cvPoint2D32f(400+720*i/NSEG, 20+720*j/NSEG);
			m_texbuf[i*(NSEG+1)+j] = cvPoint2D32f(80*i, 80*j);
		}
	for(int i=0;i<NSEG;i++)
		for(int j=0;j<NSEG;j++)
		{
			m_ibuf[6*(i*NSEG+j)  ]= i   *(NSEG+1)+ j;
			m_ibuf[6*(i*NSEG+j)+1]=(i+1)*(NSEG+1)+ j;
			m_ibuf[6*(i*NSEG+j)+2]=(i+1)*(NSEG+1)+(j+1);
			m_ibuf[6*(i*NSEG+j)+3]=(i+1)*(NSEG+1)+(j+1);
			m_ibuf[6*(i*NSEG+j)+4]= i   *(NSEG+1)+(j+1);
			m_ibuf[6*(i*NSEG+j)+5]= i   *(NSEG+1)+ j;
		}
	m_Image = NULL;
	m_bDown = false;
	m_nSel = -1;
	oldbmp = NULL;
	m_bShowVert = true;
	texture = NULL;
}

CChildView::~CChildView()
{
	if(m_Image != NULL)
	{
		cvReleaseImage(&m_Image);
	}
	if(texture != NULL)
	{
		cvReleaseImage(&texture);
	}
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
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
	// TODO: Add your message handler code here
	Draw();
	// Do not call CWnd::OnPaint() for painting messages
}

void CChildView::Draw()
{
	CClientDC dc(this);

	cvSet(m_Image, cvScalarAll(0));

	for(int i=0;i<NFACE;i++)
	{
		for(int j=0;j<3;j++)
		{
			m_vbuf[j] = m_vlist[m_ibuf[i*3+j]];
			texcoord[0][j] = m_texbuf[m_ibuf[i*3+j]].x;
			texcoord[1][j] = m_texbuf[m_ibuf[i*3+j]].y;
		}
/*
		LARGE_INTEGER tick,oldtick;
		QueryPerformanceCounter(&tick);
*/
		Triangle(m_Image, m_vbuf, 2, texcoord, Pixel);
/*
		oldtick=tick;
		QueryPerformanceCounter(&tick);
		LARGE_INTEGER off;
		off.HighPart=tick.HighPart-oldtick.HighPart;
		if(tick.LowPart>=oldtick.LowPart)
		{
			off.LowPart = tick.LowPart-oldtick.LowPart;
		}
		else
		{
			off.LowPart = oldtick.LowPart-tick.LowPart;
			off.LowPart = ~off.LowPart+1;
			off.HighPart--;
		}

		float foff=off.HighPart*pow(2.,32)+off.LowPart;
		float ffreq=lnFreg.HighPart*pow(2.,32)+lnFreg.LowPart;
		float time = foff/ffreq;
		int o=0;
*/
	}
	if(m_bShowVert)
	{
		for(int i=0;i<NVERT;i++)
		{
			cvCircle(m_Image, cvPoint(m_vlist[i].x,m_vlist[i].y), 3, cvScalar(255,0,255), -1);
		}
	}

	DispImage(&m_mdc, m_Image, CPoint(0,0));
	dc.BitBlt(0,0,IMAGE_WIDTH,IMAGE_HEIGHT,&m_mdc,0,0,SRCCOPY);
}

void CChildView::DispImage(CDC* pDC, IplImage* image, CPoint ptBase)
{
	ASSERT(image->depth==IPL_DEPTH_8U && image->nChannels == 3);

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
int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	texture = cvLoadImage("1.jpg");
	g_texture = texture;

	for(int i=0;i<(NSEG+1);i++)
		for(int j=0;j<(NSEG+1);j++)
		{
			m_texbuf[i*(NSEG+1)+j] = cvPoint2D32f(texture->width*i/NSEG, texture->height*j/NSEG);
		}

	HDC ddc = CreateDC("DISPLAY",NULL,NULL,NULL);
	IMAGE_WIDTH = GetDeviceCaps(ddc, HORZRES);
	IMAGE_HEIGHT = GetDeviceCaps(ddc, VERTRES);
	m_Image = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT),IPL_DEPTH_8U,3);
	//EdgeProcess(m_Image);

	// TODO:  Add your specialized creation code here
	CClientDC dc(this);
	m_mdc.CreateCompatibleDC(&dc);
	m_bitmap.CreateCompatibleBitmap(&dc, IMAGE_WIDTH, IMAGE_HEIGHT);
	oldbmp = m_mdc.SelectObject(&m_bitmap);
	QueryPerformanceFrequency(&lnFreg);
	return 0;
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SetCapture();
	m_bDown = true;
	CRect rcHit(-3,-3,3,3);
	bool bHit=false;
	for(int i=0;i<NVERT;i++)
	{
		CRect rcTest = rcHit + CPoint(m_vlist[i].x,m_vlist[i].y);
		if(rcTest.PtInRect(point))
		{
			m_nSel = i;
			bHit = true;
			break;
		}
	}
	if(!bHit)
		m_nSel=-1;
	
	CWnd::OnLButtonDown(nFlags, point);
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	ReleaseCapture();
	m_bDown = false;
	CWnd::OnLButtonUp(nFlags, point);
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if(m_bDown && m_nSel != -1)
	{
		m_vlist[m_nSel].x=point.x;
		m_vlist[m_nSel].y=point.y;
	}
	Draw();
	CWnd::OnMouseMove(nFlags, point);
}

void CChildView::OnDestroy()
{
	CWnd::OnDestroy();
	if(oldbmp!=NULL)
		m_mdc.SelectObject(oldbmp);
	m_mdc.DeleteDC();
	m_bitmap.DeleteObject();
	RawImage::ReleaseInternalHdr();
	// TODO: Add your message handler code here
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if(nChar==VK_SPACE)
	{
		m_bShowVert = !m_bShowVert;
		Draw();
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
