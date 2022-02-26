
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

	IplImage* disp=cvCreateImage(cvGetSize(m_dest[m_index]),IPL_DEPTH_8U,3);
	//cvCvtScale(m_dest[m_index], disp);
	//DispImage(&m_mdc, disp, CPoint(0,0));
	DispImage(&m_mdc, m_dest[1-m_index], CPoint(0,0));
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
	cvResize(image, m_ImgBk1);
	cvReleaseImage(&image);

	int wImage=m_ImgBk1->width;
	int hImage=m_ImgBk1->height;

	m_Image = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,3);
	m_debuf = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,1);
	m_dest[0] = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,3);
	m_dest[1] = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,3);
	m_depImg= m_debuf;
	m_texture = m_ImgBk1;
	cvCvtScale(m_ImgBk1,m_dest[0]);
	cvCvtScale(m_ImgBk1,m_dest[1]);
	

	CClientDC dc(this);
	m_mdc.CreateCompatibleDC(&dc);
	m_bitmap.CreateCompatibleBitmap(&dc, wImage, hImage);
	oldbmp = m_mdc.SelectObject(&m_bitmap);

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
	switch(nChar)
	{
	case VK_SPACE:
		//Process(m_dest[1-m_index], m_dest[m_index]);
		Process(m_dest[1-m_index], m_dest[m_index]);
		m_index=1-m_index;
		Invalidate();
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
/*
void CChildView::Process(IplImage* src, IplImage* dest)
{
	static Vec3 dmax(0,0,0);
	if(dmax==Vec3(0,0,0))
	{
		for(int i=0;i<src->height;i++)
		{
			for(int j=0;j<src->width;j++)
			{
				float* pix=(float*)PTR_PIX(*src,j,i);
				float* pixx=(float*)PTR_PIX(*src,min(j+1,src->width-1),i);
				float* pixy=(float*)PTR_PIX(*src,j,min(j+1,src->height-1));
				for(int k=0;k<3;k++)
				{
					if(abs(*(pix+k)-*(pixx+k))>dmax.elem[k])
						dmax.elem[k]=abs(*(pix+k)-*(pixx+k));
					if(abs(*(pix+k)-*(pixy+k))>dmax.elem[k])
						dmax.elem[k]=abs(*(pix+k)-*(pixy+k));
				}
			}
		}
	}
	dmax/=1.5;
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			float* p=(float*)PTR_PIX(*src, j, i);
			Vec3 d(0,0,0);
			POINT off[4]={{1,0},{0,1},{-1,0},{0,-1}};
			for(int k=0;k<4;k++)
			{
				int nx=j+off[k].x,ny=i+off[k].y;
				if(nx<0)nx=0;
				if(nx>src->width-1)nx=src->width-1;
				if(ny<0)ny=0;
				if(ny>src->height-1)ny=src->height-1;
				float* pd=(float*)PTR_PIX(*src, nx, ny);
				for(int l=0;l<3;l++)
				{
					float ds=*(pd+l)-*(p+l);
					float dd=ds*(ds-dmax.elem[l]*0.3)*(ds-dmax.elem[l]*0.7)/0.21/dmax.elem[l]/dmax.elem[l]/dmax.elem[l];
					if(ds>dmax.elem[l])dd=1;
					else if(ds<-dmax.elem[l])dd=-1;
					d.elem[l]+=dd;
				}
			}
			Vec3 force;

			for(int k=0;k<3;k++)
			{
				force.elem[k]=(*(PTR_PIX(*m_ImgBk1,j,i)+k)-*(p+k))/40;
			}

			float* pdest=(float*)PTR_PIX(*dest, j,i);
			for(int k=0;k<3;k++)
			{
				*(pdest+k)=*(p+k)+force.elem[k]+d.elem[k];
			}
		}
	}
}
*/
void CChildView::Process(IplImage* imgsrc,IplImage* imgdest)
{
	if(!imgsrc)
		return;
	IplImage* tmp1=cvCreateImage(cvGetSize(imgdest),IPL_DEPTH_32F,3);
	IplImage* tmp2=cvCreateImage(cvGetSize(imgdest),IPL_DEPTH_32F,3);
	IplImage* resize=cvCreateImage(cvGetSize(imgdest),IPL_DEPTH_8U, 3);
	cvResize(imgsrc, resize);
	cvConvert(resize, tmp1);
	cvConvert(resize, tmp2);
	IplImage* tmp[2]={tmp1,tmp2};
	int index=0;
	bool bEnd=false;
	int hj=0;
	while(!bEnd)
	{
		IplImage* src=tmp[index];
		IplImage* dest=tmp[1-index];

		Vec3 dmax(0,0,0);
		if(dmax==Vec3(0,0,0))
		{
			for(int i=0;i<src->height;i++)
			{
				for(int j=0;j<src->width;j++)
				{
					float* pix=(float*)PTR_PIX(*src,j,i);
					float* pixx=(float*)PTR_PIX(*src,min(j+1,src->width-1),i);
					float* pixy=(float*)PTR_PIX(*src,j,min(i+1,src->height-1));
					for(int k=0;k<3;k++)
					{
						if(abs(*(pix+k)-*(pixx+k))>dmax.elem[k])
							dmax.elem[k]=abs(*(pix+k)-*(pixx+k));
						if(abs(*(pix+k)-*(pixy+k))>dmax.elem[k])
							dmax.elem[k]=abs(*(pix+k)-*(pixy+k));
					}
				}
			}
		}
		dmax/=1.5;
		float criterion=0;
		for(int i=0;i<src->height;i++)
		{
			for(int j=0;j<src->width;j++)
			{
				float* p=(float*)PTR_PIX(*src, j, i);
				Vec3 d(0,0,0);
				POINT off[4]={{1,0},{0,1},{-1,0},{0,-1}};
				for(int k=0;k<4;k++)
				{
					int nx=j+off[k].x,ny=i+off[k].y;
					if(nx<0)nx=0;
					if(nx>src->width-1)nx=src->width-1;
					if(ny<0)ny=0;
					if(ny>src->height-1)ny=src->height-1;
					float* pd=(float*)PTR_PIX(*src, nx, ny);
					for(int l=0;l<3;l++)
					{
						float ds=*(pd+l)-*(p+l);
						float dd=ds*(ds-dmax.elem[l]*0.3)*(ds-dmax.elem[l]*0.7)/0.21/dmax.elem[l]/dmax.elem[l]/dmax.elem[l];
						if(ds>dmax.elem[l])dd=1;
						else if(ds<-dmax.elem[l])dd=-1;
						d.elem[l]+=dd;
					}
				}
				Vec3 force;

				for(int k=0;k<3;k++)
				{
					force.elem[k]=(*(PTR_PIX(*resize,j,i)+k)-*(p+k))/40;
				}

				float* pdest=(float*)PTR_PIX(*dest, j,i);
				for(int k=0;k<3;k++)
				{
					if(criterion<fabs(force.elem[k]+d.elem[k]))
						criterion=fabs(force.elem[k]+d.elem[k]);
					*(pdest+k)=*(p+k)+force.elem[k]+d.elem[k];
				}
			}
		}

		if(criterion<0.01)bEnd=true;

		index=1-index;
		hj++;
		if(hj>160)break;
	}
	cvConvert(tmp1, imgdest);

	cvReleaseImage(&tmp1);
	cvReleaseImage(&tmp2);
	cvReleaseImage(&resize);
}