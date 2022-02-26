
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
	m_dest[0]=m_dest[1]=NULL;

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
	cvReleaseImage(&m_dest[0]);
	cvReleaseImage(&m_dest[1]);
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

	IplImage* disp=cvCreateImage(cvGetSize(m_dest[1-m_index]),IPL_DEPTH_8U,3);
	Process2(m_ImgBk1, disp, m_dest[1-m_index]);
//	cvCvtScale(m_dest[1-m_index], disp);
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
	//m_mask=cvCreateImage(cvSize(40,40),IPL_DEPTH_8U,1);
	//cvZero(m_mask);
	for(int i=-5;i<=5;i++)
	{
		for(int j=-5;j<=5;j++)
		{
			if(i*i+j*j<=25)
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
	size=3;//(int)m_ptlist.size()/10;
	for(int i=0;i<size;i++)
	{
		int pos=rand()*m_ptlist.size()/(RAND_MAX+1);
		m_pthigh.push_back(m_ptlist.at(pos));
		m_ptlist.erase(m_ptlist.begin()+pos);
	}
	cvResize(image, m_ImgBk1);
	cvReleaseImage(&image);

	int wImage=m_ImgBk1->width;
	int hImage=m_ImgBk1->height;

	m_Image = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,3);
	m_debuf = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,1);
	m_dest[0] = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,1);
	m_dest[1] = cvCreateImage(cvSize(wImage,hImage), IPL_DEPTH_32F,1);
	//cvSet(m_dest, cvScalarAll(255));
	m_depImg= m_debuf;
	m_texture = m_ImgBk1;
	
	for(int i=0;i<hImage;i++)
	{
		for(int j=0;j<wImage;j++)
		{
			float* pix=(float*)PTR_PIX(**m_dest, j, i);
			float* pix2=(float*)PTR_PIX(**(m_dest+1), j, i);
			float x=(float)j/wImage;
			float y=(float)i/hImage;

			float cx=x-0.5;
			float cy=y-0.5;

			float h;
			if(0.05-(cx*cx+cy*cy)*3>=0)
				h=sqrt(0.05-(cx*cx+cy*cy)*3);
			else
				h=0;
			*pix=*pix2=h;
		}
	}

	CClientDC dc(this);
	m_mdc.CreateCompatibleDC(&dc);
	m_bitmap.CreateCompatibleBitmap(&dc, wImage, hImage);
	oldbmp = m_mdc.SelectObject(&m_bitmap);
	//SetTimer(0,1,NULL);
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
	//int nP=m_dest->width*m_dest->height;
	switch(nChar)
	{
	case VK_SPACE:
		Process(m_dest[1-m_index], m_dest[m_index]);
		m_index=1-m_index;
		Invalidate();
		break;
	case 'Z':
		SetTimer(0, 1, NULL);
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
#define REFRAC_FACTOR 0.9
#define DA  0.01
Vec2 rain_vec(-0.2, 1);
int rain_max_length=30;
int rain_min_length=15;
int CChildView::Process(IplImage* src, IplImage* dest)
{
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			float* pix1=(float*)PTR_PIX(*src, j, i);
			float* pixd1=(float*)PTR_PIX(*dest, j, i);
			int h[2]={i,min(i+1,dest->height-1)};
			float* pixd2=(float*)PTR_PIX(*dest, j, h[1]);
			float* pix2=(float*)PTR_PIX(*src, j, h[1]);
			float* pix[2]={pix1,pix2};
			float* pixd[2]={pixd1, pixd2};
			CvPoint adj[4]={{1,0},{0,1},{-1,0},{0,-1}};
			float acc[2];
			acc[0]=acc[1]=0;
			for(int k=0;k<2;k++)
			{
				for(int l=0;l<4;l++)
				{
					int ni=h[k]+adj[l].y;
					int nj=j+adj[l].x;
					ni=min(ni, dest->height-1);
					ni=max(ni,0);
					nj=min(nj, dest->width-1);
					nj=max(nj,0);
					float* pixn=(float*)PTR_PIX(*src, nj, ni);
					acc[k]+=*pixn-*pix[k];
				}
			}
			if(i==283&&j==380)
			{
				int o=0;
				o++;
			}
			float da=acc[0]-acc[1];
			float ad=(acc[0]+acc[1])/2;
			*pixd[0]=*pix[0]+(da-DA)/10;//+ad/4;///**/+acc[0]/4;
			*pixd[1]=*pix[1]-(da-DA)/10;//+ad/4;///**/+acc[1]/4;
			if(i==283&&j==380)
			{
				int o=0;
				o++;
			}
		}
	}
	return 0;
}

void CChildView::Process2(IplImage* src, IplImage* dest, IplImage* ref)
{
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			uchar* pix=PTR_PIX(*dest, j, i);
			float x=(float)j/dest->width;
			float y=(float)i/dest->height;

			int ni=min(i+1, dest->height-1);
			int nj=min(j+1, dest->width-1);
			float* pref=(float*)PTR_PIX(*ref, j, i);
			float* prx=(float*)PTR_PIX(*ref, nj, i);
			float* pry=(float*)PTR_PIX(*ref, j, ni);
			float h=*pref;
			float hx=*prx-*pref;
			float hy=*pry-*pref;
			Vec2 dh(hx, hy);

			Vec3 normal(dh.x,dh.y,1./dest->height);
			normal=normalize(normal);

			Vec3 lin(0,0,1);
			Vec3 offin=lin-dot(lin, normal)*normal;
			Vec3 offout=offin*REFRAC_FACTOR;
			float frac=sqrt(1-dot(offout,offout));
			Vec3 refrac=frac*normal+offout;
			Vec2 offset(0,0);
			offset=Vec2(refrac.x,refrac.y)/refrac.z*h;

			float l=dot(normal, normalize(Vec3(0.25,0.25,1)));
			float lh=pow(l,100);
			if(l<0.1)l=0.1;
			Vec3 smp=Sample(src, Vec2(x,y)+offset)+Vec3(255,255,255)*lh;
			if(hx!=0)
			{
				int o=0;
				o++;
			}
			if(smp.x>255)smp.x=255;
			if(smp.y>255)smp.y=255;
			if(smp.z>255)smp.z=255;
			smp*=l;
			*pix=smp.x;
			*(pix+1)=smp.y;
			*(pix+2)=smp.z;

		}
	}
}
void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	//Process(m_ImgBk1, m_dest);
	Draw();
	CWnd::OnTimer(nIDEvent);
}
/*	for(int i=0;i<10000;i++)
	{
		int x=(float)rand()/(RAND_MAX+1)*dest->width;
		int y=(float)rand()/(RAND_MAX+1)*dest->height;

		int length=rain_min_length+(float)rand()/(RAND_MAX)*(rain_max_length-rain_min_length);
		for(int j=-length;j<length;j++)
		{
			int startx=x+j/rain_vec.length()*rain_vec.x;
			int starty=y+j/rain_vec.length()*rain_vec.y;
			if(startx<0||startx>=dest->width||starty<0||starty>=dest->height)
				continue;
			uchar* pix=PTR_PIX(*dest, startx, starty);
			*pix=255-(255-*pix)/1.1;
			*(pix+1)=255-(255-*(pix+1))/1.1;
			*(pix+2)=255-(255-*(pix+2))/1.1;
		}

	}*/

/*	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			uchar* pix=PTR_PIX(*dest, j, i);
			float x=(float)j/dest->width;
			float y=(float)i/dest->height;

			float cx=x-0.5;
			float cy=y-0.5;

			float h=0.65-(cx*cx+cy*cy)*3;
			Vec2 dh=6*h*Vec2(cx,cy);
			Vec2 norm;
			if(h>0.6)
			{
				norm=-dh/2/sqrt(h-0.6);
			}
			else
			{
				norm=Vec2(0,0);
			}
			Vec3 normal(norm.x,norm.y,1);
			normal=normalize(normal);

			Vec3 lin(0,0,1);
			Vec3 offin=lin-dot(lin, normal)*normal;
			Vec3 offout=offin*REFRAC_FACTOR;
			float frac=sqrt(1-dot(offout,offout));
			Vec3 refrac=frac*normal+offout;
			Vec2 offset(0,0);
			if(h>0.6)
				offset=Vec2(refrac.x,refrac.y)/refrac.z*sqrt(h-0.6);

			float l=dot(normal, normalize(Vec3(0.25,0.25,1)));
			float lh=pow(l,100);
			if(h<=0.6)
				l=1;
			if(l<0.1)l=0.1;
			Vec3 smp=Sample(src, Vec2(x,y)+offset)+Vec3(255,255,255)*lh;
			if(smp.x>255)smp.x=255;
			if(smp.y>255)smp.y=255;
			if(smp.z>255)smp.z=255;
			smp*=l;
			*pix=smp.x;
			*(pix+1)=smp.y;
			*(pix+2)=smp.z;
		}
	}*/

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return 0;//CWnd::OnEraseBkgnd(pDC);
}
