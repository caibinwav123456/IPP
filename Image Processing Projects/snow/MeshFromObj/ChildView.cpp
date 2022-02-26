
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "MeshFromObj.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildView
Vec3 Pos(0,0,-3);
Mat rot(1,0,0,0,1,0,0,0,1);
Mat rotx,roty;
#include <stdlib.h>
#define MIRROR_EFFECT
#define PI 3.1415926535897932384626
void VertexShader(Vec3* posin, Vec3* colorin, Vec3* normalin, Vec2* texin, float* vuserdatain, Vec3* posout, Vec3* colorout, Vec2* texout, float* vuserdataout)
{
	Vec3 tv = *posin*rot;
	Vec3 tn = *normalin*rot;
	tv-=Pos;
	*posout = Vec3(tv.x/tv.z,tv.y/tv.z,1/tv.z);
	float d=dot(tn,tv);
	float l=-d/tv.length();
	Vec3 reflect = (d*tn*2-tv);
	float x=atan(reflect.x/reflect.z)/PI;
	float y=asin(reflect.y/reflect.length())/PI;
	if(reflect.z<0)
	{
		if(reflect.x>0)
		{
			x+=1;
		}
		else if(reflect.x<0)
		{
			x-=1;
		}
	}
	x=(x+1)/2;
	y+=0.5;
	
	*colorout = Vec3(l,l,l);
	texout->x=x;
	texout->y=y;
}

CChildView::CChildView()
{
	m_bShowVert = false;
	m_bShowImage = true;
	m_bDown = false;
	m_bMirror = true;
	m_index=0;

	m_Image = NULL;
	m_ImgBk1 = NULL;
	m_ImgBk2 = NULL;
	m_debuf = NULL;
	m_mask=NULL;

	oldbmp = NULL;

}

CChildView::~CChildView()
{
	if(m_Image != NULL)
	{
		cvReleaseImage(&m_Image);
	}
	if(m_ImgBk1 != NULL)
	{
		cvReleaseImage(&m_ImgBk1);
	}
	if(m_ImgBk2 != NULL)
	{
		cvReleaseImage(&m_ImgBk2);
	}
	if(m_debuf != NULL)
	{
		cvReleaseImage(&m_debuf);
	}
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
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

/*
	mesh.m_nVertexFormat = VF_XYZ|VF_TEXCOORD;
	int deb=mesh.ComputeVertLength();
	int deb1=mesh.ComputePixLength();
	int deb2=mesh.ComputetVFormat(mesh.m_nVertexFormat);
	int deb3=mesh.VertAttrOffset(VF_TEXCOORD);
	int deb4=mesh.PixAttrOffset(VFT_TEXCOORD);
*/

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	Draw();
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
}

void CChildView::Draw()
{
	CClientDC dc(this);

	CRect rcClt;
	GetClientRect(rcClt);

	cvSet(m_Image, cvScalarAll(0));
	cvSet(m_debuf, cvScalar(0));

	IplImage* disp=cvCreateImage(cvGetSize(m_dest),IPL_DEPTH_8U,3);
	cvCvtScale(m_dest, disp);
	DispImage(&m_mdc, disp, CPoint(0,0));
	cvReleaseImage(&disp);
	dc.BitBlt(0,0,rcClt.Width(),rcClt.Height(),&m_mdc,0,0,SRCCOPY);
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

	// TODO:  Add your specialized creation code here
	IplImage* image = cvLoadImage("peacew.jpg");
	float xscale=1024./image->width;
	float yscale=768./image->height;
	float scale=min(xscale,yscale);
	m_ImgBk1=cvCreateImage(cvSize(image->width*scale,image->height*scale), IPL_DEPTH_8U, 3);
	m_mask=cvCreateImage(cvSize(51,51),IPL_DEPTH_32F, 1);
	for(int i=0;i<51;i++)
	{
		for(int j=0;j<51;j++)
		{
			int r2=(i-25)*(i-25)+(j-25)*(j-25);
			float fac=exp(-(float)(r2)/(25*25)*5);
			if(r2>25*25)
			{
				fac=0;
			}
			*(float*)PTR_PIX(*m_mask,i,j)=fac;
		}
	}
	//cvZero(m_mask);
	for(int i=-10;i<10;i++)
	{
		for(int j=-10;j<10;j++)
		{
			if(i+j>=-5&&i+j<5)
			{
				m_ptlist.push_back(cvPoint(i,j));
			}
		}
	}
	int size=(int)m_ptlist.size()/2;
	for(int i=0;i<size;i++)
	{
		int pos=rand()*m_ptlist.size()/(RAND_MAX+1);
		m_ptlist.erase(m_ptlist.begin()+pos);
	}
	cvResize(image, m_ImgBk1);
	cvReleaseImage(&image);

	int wImage=m_ImgBk1->width;
	int hImage=m_ImgBk1->height;

	m_Image = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,3);
	m_debuf = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,1);
	m_dest = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,3);
	cvSet(m_dest, cvScalarAll(255));
	m_depImg= m_debuf;
	m_texture = m_ImgBk1;
	

	CClientDC dc(this);
	m_mdc.CreateCompatibleDC(&dc);
	m_bitmap.CreateCompatibleBitmap(&dc, wImage, hImage);
	oldbmp = m_mdc.SelectObject(&m_bitmap);
//	SetTimer(0,1,NULL);
	return 0;
}

void CChildView::OnDestroy()
{
	CWnd::OnDestroy();

	if(oldbmp!=NULL)
		m_mdc.SelectObject(oldbmp);
	m_mdc.DeleteDC();
	m_bitmap.DeleteObject();

	// TODO: Add your message handler code here
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SetCapture();

	CWnd::OnLButtonDown(nFlags, point);
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	ReleaseCapture();

	CWnd::OnLButtonUp(nFlags, point);
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect rcClt;
	GetClientRect(rcClt);

	CWnd::OnMouseMove(nFlags, point);
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	int nP=m_dest->width*m_dest->height;
	switch(nChar)
	{
	case VK_SPACE:
		cvCopyImage(m_ImgBk1, m_dest);
		Process(m_ImgBk1, m_dest,300, 5);
		Process(m_ImgBk1, m_dest,150,10);
		Process(m_ImgBk1, m_dest,600,50);
		Invalidate();
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChildView::Process(IplImage* src, IplImage* dest, int nSnow, int rSnow)
{
	IplImage* mask=cvCreateImage(cvSize(51,51),IPL_DEPTH_32F, 1);
	for(int i=0;i<51;i++)
	{
		for(int j=0;j<51;j++)
		{
			int r2=(i-25)*(i-25)+(j-25)*(j-25);
			float fac=exp(-(float)(r2)/(25*25)*rSnow);
			if(r2>25*25)
			{
				fac=0;
			}
			*(float*)PTR_PIX(*mask,i,j)=fac;
		}
	}

	for(int k=0;k<nSnow;k++)
	{
		int x=rand()*dest->width/(RAND_MAX+1);
		int y=rand()*dest->height/(RAND_MAX+1);
		for(int i=-25;i<=25;i++)
		{
			for(int j=-25;j<=25;j++)
			{
				int nx=x+j;
				int ny=y+i;
				if(nx>=0&&nx<dest->width&&ny>=0&&ny<dest->height)
				{
					uchar* pdest=PTR_PIX(*dest, nx, ny);
					//uchar* psrc=PTR_PIX(*src, nx, ny);
					float* pmask=(float*)PTR_PIX(*mask, j+25, i+25);
					*pdest=255-(255-*pdest)*(1-*pmask);
					*(pdest+1)=255-(255-*(pdest+1))*(1-*(pmask+1));
					*(pdest+2)=255-(255-*(pdest+2))*(1-*(pmask+2));
				}
			}
		}
	}
	cvReleaseImage(&mask);
}
void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	//Process(m_ImgBk1, m_dest);
	Draw();
	CWnd::OnTimer(nIDEvent);
}
