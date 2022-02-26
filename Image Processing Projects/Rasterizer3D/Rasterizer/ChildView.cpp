
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "ImageProcess3D.h"
#include "ChildView.h"
#include "matutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int IMAGE_WIDTH  =800;
int IMAGE_HEIGHT =600;

// CChildView
//#define SAMPLE
#define EDGE 1
IplImage* g_texture=NULL;
CRect g_rcImage(500,200,900,500);//(400,20,1120,740);
LARGE_INTEGER lnFreg = {0};
Vec3 Pos(0,0,-3);
Mat g_mRot(1,0,0,0,1,0,0,0,1), rotx, roty;

void Pixel(RawImage* target,Point2D pos, int count, float* pdata)
{
	CvPoint2D32f tex=cvPoint2D32f(pdata[0],pdata[1]);
	CPoint pt(tex.x, tex.y);
	if(pt.x<0)pt.x=0;
	else if(pt.x>=g_texture->width)pt.x=g_texture->width-1;
	if(pt.y<0)pt.y=0;
	else if(pt.y>=g_texture->height)pt.y=g_texture->height-1;
#ifdef SAMPLE
	CvScalar pSamp = Sample(g_texture, tex);
	uchar* ptr=PTR_PIX(*target, pos.x, pos.y);
	*ptr  = pSamp.val[0];
	*(ptr+1)= pSamp.val[1];
	*(ptr+2)= pSamp.val[2];
#else
	uchar* ptr=PTR_PIX(*target, pos.x, pos.y);
	uchar* ptrt=PTR_PIX(*g_texture, pt.x, pt.y);
	*ptr=*ptrt;
	*(ptr+1)=*(ptrt+1);
	*(ptr+2)=*(ptrt+2);
#endif
}

CChildView::CChildView()
{
	for(int i=0;i<NSEG+1;i++)
	{
		for(int j=0;j<NSEG+1;j++)
		{
			m_vlist[i*(NSEG+1)+j] = cvPoint3D32f((2.*i/NSEG-1)*EDGE, (2.*j/NSEG-1)*EDGE, 0);
			m_texbuf[i*(NSEG+1)+j] = cvPoint2D32f(0,0);
		}
	}
	for(int i=0;i<NSEG;i++)
	{
		for(int j=0;j<NSEG;j++)
		{
			m_ibuf[6*(i*NSEG+j)  ]= i   *(NSEG+1)+ j;
			m_ibuf[6*(i*NSEG+j)+1]=(i+1)*(NSEG+1)+ j;
			m_ibuf[6*(i*NSEG+j)+2]=(i+1)*(NSEG+1)+(j+1);
			m_ibuf[6*(i*NSEG+j)+3]=(i+1)*(NSEG+1)+(j+1);
			m_ibuf[6*(i*NSEG+j)+4]= i   *(NSEG+1)+(j+1);
			m_ibuf[6*(i*NSEG+j)+5]= i   *(NSEG+1)+ j;
		}
	}
	m_Image = NULL;
	m_bDown = false;
	m_nSel = -1;
	oldbmp = NULL;
	m_bShowVert = true;
	texture = NULL;
	m_bShow = true;
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
	ON_WM_ERASEBKGND()
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

	CRect rcClt;
	GetClientRect(rcClt);
	float scale = rcClt.Height()/2;

	cvSet(m_Image, cvScalarAll(0));
	VertexProcess();
	if(m_bShow)
	{
		for(int i=0;i<NFACE;i++)
		{
			for(int j=0;j<3;j++)
			{
				m_vbuf[j] = m_tvlist[m_ibuf[i*3+j]];
				texcoord[0][j] = m_texbuf[m_ibuf[i*3+j]].x;
				texcoord[1][j] = m_texbuf[m_ibuf[i*3+j]].y;
			}
			Rect2D szVp = Rect2D(0,0,rcClt.Width(),rcClt.Height());
			Triangle3D(m_Image, m_vbuf, 2, texcoord, Pixel, &szVp);
/*
			LARGE_INTEGER tick,oldtick;
			QueryPerformanceCounter(&tick);

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
	}
	if(m_bShowVert)
	{
		for(int i=0;i<NVERT;i++)
		{
			if(m_tvlist[i].z>0)
				cvCircle(m_Image, cvPoint(m_tvlist[i].x*rcClt.Height()/2+rcClt.Width()/2,-m_tvlist[i].y*rcClt.Height()/2+rcClt.Height()/2), 3, cvScalar(255,0,255), -1);
		}
/*
		for(int i=0;i<NFACE;i++)
		{
			for(int j=0;j<3;j++)
			{
				CvPoint pt1=cvPoint(m_tvlist[m_ibuf[i*3+j]].x*rcClt.Height()/2+rcClt.Width()/2,m_tvlist[m_ibuf[i*3+j]].y*rcClt.Height()/2+rcClt.Height()/2);
				CvPoint pt2=cvPoint(m_tvlist[m_ibuf[i*3+(j+1)%3]].x*rcClt.Height()/2+rcClt.Width()/2,m_tvlist[m_ibuf[i*3+(j+1)%3]].y*rcClt.Height()/2+rcClt.Height()/2);
				DrawLine(pt1,pt2,cvScalar(255,0,255));
			}
		}
*/
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

void CChildView::DrawLine(CvPoint pt1,CvPoint pt2,CvScalar color)
{
	if(pt1.x==pt2.x&&pt1.y==pt2.y)
		return;
	int offx=pt2.x-pt1.x;
	int offy=pt2.y-pt1.y;
	int x,y;
	int*p1,*p2;
	int ex=m_Image->width,ey=m_Image->height;
	int*pe1,*pe2;
	int*ps1,*ps2;
	float asc;

	if(abs(offx)>=abs(offy))
	{
		p1=&x,p2=&y;
		pe1=&ex,pe2=&ey;
		ps1=&pt1.x,ps2=&pt1.y;
		asc=(float)offy/offx;
	}
	else
	{
		p1=&y,p2=&x;
		pe1=&ey,pe2=&ex;
		ps1=&pt1.y,ps2=&pt1.x;
		asc=(float)offx/offy;
	}
	for(*p1=0;*p1<*pe1;(*p1)++)
	{
		*p2=*ps2+asc*(*p1-*ps1);
		if(*p2>=0 && *p2<*pe2)
		{
			*PTR_PIX((RawImage)m_Image,x,y)=color.val[0];
			*(PTR_PIX((RawImage)m_Image,x,y)+1)=color.val[1];
			*(PTR_PIX((RawImage)m_Image,x,y)+2)=color.val[2];
		}
/*
		*((uchar*)(m_Image->imageData)+y*m_Image->widthStep+x*m_Image->nChannels)=color.val[0];
		*(((uchar*)(m_Image->imageData)+y*m_Image->widthStep+x*m_Image->nChannels)+1)=color.val[1];
		*(((uchar*)(m_Image->imageData)+y*m_Image->widthStep+x*m_Image->nChannels)+2)=color.val[2];
*/

	}
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	texture = cvLoadImage("1.jpg");
	g_texture = texture;

	for(int i=0;i<(NSEG+1);i++)
	{
		for(int j=0;j<(NSEG+1);j++)
		{
			m_texbuf[i*(NSEG+1)+j] = cvPoint2D32f(texture->width*i/NSEG, texture->height*(NSEG-j)/NSEG);
		}
	}

	HDC ddc = CreateDC("DISPLAY",NULL,NULL,NULL);
	IMAGE_WIDTH = GetDeviceCaps(ddc, HORZRES);
	IMAGE_HEIGHT = GetDeviceCaps(ddc, VERTRES);
	m_Image = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT),IPL_DEPTH_8U,3);

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
	m_oldPt = point;
	
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
	if(m_bDown)
	{
		CPoint offset=point-m_oldPt;
		float HAngle = (float)offset.x/100;
		float VAngle = (float)offset.y/100;
		rotx = Mat(cos(HAngle),0,sin(HAngle),0,1,0,-sin(HAngle),0,cos(HAngle));
		roty = Mat(1,0,0,0,cos(VAngle),-sin(VAngle),0,sin(VAngle),cos(VAngle));
		g_mRot = g_mRot*rotx*roty;
		m_oldPt = point;
		Draw();
	}
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
	else if(nChar=='A')
	{
		Pos.x-=0.01;
		Draw();
	}
	else if(nChar=='D')
	{
		Pos.x+=0.01;
		Draw();
	}
	else if(nChar=='W')
	{
		Pos.z+=0.01;
		Draw();
	}
	else if(nChar=='S')
	{
		Pos.z-=0.01;
		Draw();
	}
	else if(nChar=='K')
	{
		m_bShow=!m_bShow;
		Draw();
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChildView::VertexProcess()
{
	for(int i=0;i<NVERT;i++)
	{
		CvPoint3D32f vert = m_vlist[i];
		Vec3 vec(vert.x,vert.y,vert.z);
		Vec3 tvec = vec*g_mRot;
		tvec-=Pos;
		m_tvlist[i]=cvPoint3D32f(tvec.x/tvec.z,tvec.y/tvec.z,1./tvec.z);
	}
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;//CWnd::OnEraseBkgnd(pDC);
}
