
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "MeshFromObj.h"
#include "ChildView.h"
#include <time.h>
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

	IplImage* img=cvCreateImage(cvSize(512,512),IPL_DEPTH_32F,2);
	cvZero(img);
	IplImage* hsv=cvCreateImage(cvSize(512,512),IPL_DEPTH_8U,3);
	IplImage* gray=cvCreateImage(cvSize(512,512),IPL_DEPTH_8U,1);
	IplImage* tmp=cvCreateImage(cvSize(512,512),IPL_DEPTH_32F,1);
	IplImage* disp=cvCreateImage(cvSize(512,512),IPL_DEPTH_8U,3);
	Perlin(tmp,16,16);
	cvCvtScale(tmp,gray,255);
	//cvCvtScale(tmp, tmp, 511,-256);
	//cvAbs(tmp,tmp);
	//cvCvtScale(tmp,gray);
	cvMerge(gray, gray, gray, NULL, disp);

/*	cvCvtColor(m_ImgBk1, hsv, CV_RGB2HSV);
	cvSplit(hsv, NULL, NULL, gray, NULL);
	cvCvtScale(gray,tmp);

	cvMerge(tmp,NULL,NULL,NULL, img);
	FFT2D(img, img);
//	FFT2D(img, img);
	cvSplit(img, tmp,NULL,NULL,NULL);
	cvCvtScale(tmp, gray,1./512/512);

	cvMerge(gray, gray,gray,NULL, disp);
*/

	DispImage(&m_mdc, disp, CPoint(0,0));
	cvReleaseImage(&img);
	cvReleaseImage(&hsv);
	cvReleaseImage(&gray);
	cvReleaseImage(&tmp);
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
	IplImage* image=cvLoadImage("peaceb.jpg");
	m_ImgBk1 = cvCreateImage(cvSize(512,512),IPL_DEPTH_8U, 3);
	cvResize(image, m_ImgBk1);
	cvReleaseImage(&image);
	m_ImgBk2 = cvLoadImage("peacew.jpg");

	int wImage=m_ImgBk1->width;
	int hImage=m_ImgBk1->height;

	m_Image = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,3);
	m_debuf = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,1);
	m_depImg= m_debuf;
	m_texture = m_ImgBk1;

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
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChildView::Perlin(IplImage* img, int nx, int ny)
{
	ASSERT(img!=NULL);
	ASSERT(img->depth==IPL_DEPTH_32F);
	ASSERT(img->nChannels==1);

	srand(time(NULL));
	IplImage* grid=cvCreateImage(cvSize(nx,ny), IPL_DEPTH_32F, 1);
	IplImage* imgfft=cvCreateImage(cvGetSize(grid),IPL_DEPTH_32F, 2);
	IplImage* imgout=cvCreateImage(cvGetSize(img), IPL_DEPTH_32F, 2);

	cvZero(imgfft);
	cvZero(imgout);
	cvZero(img);

	for(int i=0;i<nx;i++)
	{
		for(int j=0;j<ny;j++)
		{
			*(float*)(grid->imageData+grid->widthStep*j+i*4)=(float)rand()/RAND_MAX;
		}
	}
	cvMerge(grid, NULL, NULL, NULL, imgfft);
	cvDFT(imgfft, imgfft, CV_DXT_FORWARD);
	//FFT2D(imgfft,imgfft);

	for(int i=0;i<2;i++)
	{
		for(int j=0;j<2;j++)
		{
			cvSetImageROI(imgout,cvRect(i*(img->width-nx/2),j*(img->height-ny/2), nx/2, ny/2));
			cvSetImageROI(imgfft, cvRect(i*nx/2,j*ny/2,nx/2,ny/2));
			cvCopyImage(imgfft, imgout);
		}
	}
	cvResetImageROI(imgout);
	cvResetImageROI(imgfft);

	//FFT2D(imgout, imgout);
	cvDFT(imgout, imgout, CV_DXT_INVERSE);
	cvSplit(imgout, img, NULL, NULL, NULL);
	cvCvtScale(img, img, 1./nx/ny);

	cvReleaseImage(&grid);
	cvReleaseImage(&imgfft);
	cvReleaseImage(&imgout);
}
/*
void CChildView::Perlin(IplImage* img, int nx, int ny)
{
	ASSERT(img!=NULL);
	ASSERT(img->depth==IPL_DEPTH_32F);
	ASSERT(img->nChannels==1);

	srand(time(NULL));
	IplImage* grid=cvCreateImage(cvSize(nx,ny), IPL_DEPTH_32F, 1);
	IplImage* imgfft=cvCreateImage(cvGetSize(grid),IPL_DEPTH_32F, 2);
	IplImage* output=cvCreateImage(cvGetSize(grid), IPL_DEPTH_32F, 2);
	IplImage* imgout=cvCreateImage(cvGetSize(grid),IPL_DEPTH_32F, 2);

	cvZero(imgfft);

	for(int i=0;i<nx;i++)
	{
		for(int j=0;j<ny;j++)
		{
			*(float*)(grid->imageData+grid->widthStep*j+i*4)=(float)rand()/RAND_MAX;
		}
	}
	cvMerge(grid, NULL, NULL, NULL, imgfft);
	FFT2D(imgfft,imgout);

	for(int i=0;i<nx;i++)
	{
		for(int j=0;j<ny;j++)
		{
			Complex* comp=(Complex*)(imgfft->imageData+j*imgfft->widthStep+i*4*2);
			Complex* compd=(Complex*)(imgout->imageData+((j+ny/2)%ny)*imgout->widthStep+((i+nx/2)%nx)*4*2);
			*comp=*compd;
		}
	}
	cvCopyImage(imgfft, imgout);
	cvZero(img);
	for(int i=0;i<img->width/nx;i++)
	{
		for(int j=0;j<img->height/ny;j++)
		{
			cvCopyImage(imgout,imgfft);
			for(int k=0;k<nx;k++)
			{
				for(int l=0;l<ny;l++)
				{
					Complex& comp=*(Complex*)(imgfft->imageData+l*imgfft->widthStep+k*4*2);
					comp=comp*Complex::Exp(2*PI*(i*(k-nx/2))/img->width)*Complex::Exp(2*PI*(j*(l-ny/2))/img->height);
				}
			}
			FFT2D(imgfft, imgfft);
			for(int k=0;k<nx;k++)
			{
				for(int l=0;l<ny;l++)
				{
					Complex* comp=(Complex*)(imgfft->imageData+l*imgfft->widthStep+k*4*2);
					*(float*)(img->imageData+(j+l*(img->width/nx))*img->widthStep+(i+k*(img->height/ny))*4)=
						(*comp*Complex::Exp(2*PI*k/2)*Complex::Exp(2*PI*l/2)).r/nx/ny;
				}
			}
		}
	}
	cvReleaseImage(&grid);
	cvReleaseImage(&imgfft);
	cvReleaseImage(&output);
	cvReleaseImage(&imgout);
}
*/