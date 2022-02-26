
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "3DEffects.h"
#include "ChildView.h"
#include "3dutil.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif
#include "Image.h"
#include "ImageProcess3D.h"
#include "ImageProcess2D.h"
#include "stdlib.h"
#include "time.h"
#include "Gdiplus.h"
#include "FFT.h"
using namespace Gdiplus;

// CChildView

CChildView::CChildView()
{
	m_src = NULL;
	m_dest = NULL;
	m_tex = NULL;
	m_frame = NULL;
	m_mask = NULL;
	m_nIndex = 0;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_COMMAND(ID_FILE_OPEN2, &CChildView::OnFileOpen2)
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
	
	// TODO: Add your message handler code here
	Draw();
	// Do not call CWnd::OnPaint() for painting messages
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

void CChildView::Draw()
{
	CClientDC dc(this);
	CRect rcClient;
	GetClientRect(rcClient);
	CDC mdc;
	mdc.CreateCompatibleDC(&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
	CBitmap* oldbmp = mdc.SelectObject(&bmp);
	mdc.FillSolidRect(rcClient, RGB(255,255,255));
	if(m_dest)
		DispImage(&mdc, m_dest, CPoint(0,0));
	//CString str;
	//mdc.SetBkMode(TRANSPARENT);
	//mdc.SetTextColor(RGB(255,0,0));
	//mdc.TextOut(0,0,str);
	dc.BitBlt(0,0,rcClient.Width(), rcClient.Height(), &mdc, 0,0,SRCCOPY);
	mdc.SelectObject(oldbmp);
	mdc.DeleteDC();
	bmp.DeleteObject();
}

void Rainbow(IplImage* src, IplImage* dest);

IplImage* loadWithGdiPlus(wchar_t* imageFileName)
{
	Bitmap *pBitmap = new Bitmap(imageFileName, FALSE);
	if (pBitmap->GetLastStatus() != Ok)
	{
		TRACE(_T("%d\n"), pBitmap->GetLastStatus());
		delete pBitmap;
		return NULL;
	}

	BitmapData bmpData;
	Rect rect(0,0,pBitmap->GetWidth(),pBitmap->GetHeight());
	PixelFormat pixfmt = pBitmap->GetPixelFormat();
	pBitmap->LockBits(&rect, ImageLockModeRead, pixfmt, &bmpData);
	IplImage* tempImg = cvCreateImage(cvSize(pBitmap->GetWidth(), pBitmap->GetHeight()), IPL_DEPTH_8U, pixfmt==PixelFormat32bppARGB?4:3);
	BYTE* temp = (bmpData.Stride>0)?((BYTE*)bmpData.Scan0):((BYTE*)bmpData.Scan0+bmpData.Stride*(bmpData.Height-1));
	memcpy(tempImg->imageData, temp, abs(bmpData.Stride)*bmpData.Height);
	pBitmap->UnlockBits(&bmpData);
	//ÅÐ¶ÏTop-Down or Bottom-Up
	if (bmpData.Stride<0)
		cvFlip(tempImg, tempImg);

	delete pBitmap;

	return tempImg;
}

void CChildView::OnFileOpen2()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("All Files|*.*||"), this);
	if(dlg.DoModal() == IDOK)
	{
		CString strName = dlg.GetPathName();
		cvReleaseImage(&m_src);
		m_src=loadWithGdiPlus(CT2W(strName));
		if(m_src && (m_src->width>1024 || m_src->height>1024))
		{
			float xscale = 1024./m_src->width;
			float yscale = 1024./m_src->height;
			float scale = min(xscale, yscale);
			int width = cvRound(m_src->width*scale);
			int height = cvRound(m_src->height*scale);
			IplImage* tmp = m_src;
			m_src = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
			cvResize(tmp, m_src);
			cvReleaseImage(&tmp);
		}
		if(m_src)
		{
			cvReleaseImage(&m_dest);
			m_dest = cvCreateImage(cvGetSize(m_src), IPL_DEPTH_8U, 3);
			ProcessAll();
			Draw();
		}
	}
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_dest = cvCreateImage(cvSize(800, 800), IPL_DEPTH_8U, 3);
	m_tex = loadWithGdiPlus(L"stroke.png");
	m_frame = loadWithGdiPlus(L"block.png");
	m_mask = loadWithGdiPlus(L"blockmask.png");
	m_tri = loadWithGdiPlus(L"triangle.png");
	m_trimask = loadWithGdiPlus(L"triangle_mask.png");
	m_tri2 = loadWithGdiPlus(L"triangle2.png");
	m_trimask2 = loadWithGdiPlus(L"triangle_mask2.png");
	m_hex = loadWithGdiPlus(L"hexagon.png");
	m_hexmask = loadWithGdiPlus(L"hexagon_mask.png");
	ASSERT(m_tex->nChannels == 4);
	ASSERT(m_frame->nChannels == 4);
	cvSet(m_dest, cvScalarAll(255));
	Draw();	
	return 0;
}


void CChildView::OnDestroy()
{
	CWnd::OnDestroy();
	cvReleaseImage(&m_tex);
	cvReleaseImage(&m_frame);
	cvReleaseImage(&m_mask);
	cvReleaseImage(&m_tri);
	cvReleaseImage(&m_trimask);
	cvReleaseImage(&m_tri2);
	cvReleaseImage(&m_trimask2);
	cvReleaseImage(&m_hex);
	cvReleaseImage(&m_hexmask);
	cvReleaseImage(&m_src);
	cvReleaseImage(&m_dest);
	RawImage::ReleaseInternalHdr();
	// TODO: Add your message handler code here
}

void DrawRing(IplImage* dest, Vec2 center, float radius, Vec3 color)
{
	int xmin = center.x-radius,
		xmax = center.x+radius,
		ymin = center.y-radius,
		ymax = center.y+radius;

	xmin = max(0,min(dest->width,xmin));
	xmax = max(0,min(dest->width,xmax));
	ymin = max(0,min(dest->height,ymin));
	ymax = max(0,min(dest->height,ymax));

	for(int i=ymin;i<ymax;i++)
	{
		for(int j=xmin;j<xmax;j++)
		{
			Vec2 p(j,i);
			Vec2 polar = p-center;
			float r = polar.length();
			if(r<radius*0.9)
			{
				r=r/(radius*0.9);
				float l=powf(r,2)*0.1+0.1;
				uchar* pix = PTR_PIX(*dest, j, i);
				Vec3 v(*pix,*(pix+1),*(pix+2));
				Vec3 vd = color*l+v*(1-l);
				*pix = max(0,min(255,vd.x));
				*(pix+1) = max(0,min(255,vd.y));
				*(pix+2) = max(0,min(255,vd.z));
			}
			else if(r<radius)
			{
				r=(r-radius*0.9)/(radius*0.1);
				float l=sin(r*CV_PI)*0.1+0.2;
				uchar* pix = PTR_PIX(*dest, j, i);
				Vec3 v(*pix,*(pix+1),*(pix+2));
				Vec3 vd = color*l+v*(1-l);
				*pix = max(0,min(255,vd.x));
				*(pix+1) = max(0,min(255,vd.y));
				*(pix+2) = max(0,min(255,vd.z));
			}
		}
	}
}

void GenHueTex(IplImage* dest)
{
	ASSERT(dest->nChannels == 3 && dest->depth == IPL_DEPTH_32F);
	for(int i=0;i<dest->width;i++)
	{
		float theta = (float)i/dest->width*(2*CV_PI);
		Vec3 hue(cos(theta), cos(theta+(2*CV_PI/3)), cos(theta+(4*CV_PI/3)));
		hue = hue*128+Vec3(128,128,128);
		for(int j=0;j<3;j++)
			hue.elem[j] = max(0,min(255,hue.elem[j]));
		*(Vec3*)PTR_PIX(*dest, i, 0) = hue;
	}
}
//Sunny
void CChildView::Process(IplImage* src, IplImage* dest)
{
	float radius=0.02;
	float scale = min(src->width, src->height);
	float rs = radius*scale;
	float raymin = 0.6;
	float raymax = 0.7;
	float rayshift = 0.12;
	float raymins = raymin*scale;
	float raymaxs = raymax*scale;
	float rayshifts = rayshift*scale;
	int nray = 40;
	int nhaloray = 2;
	int nhalo = 5;
	int nring = 15;
	float haloraystride = CV_PI/2/40;
	float halorayturb = CV_PI/2/10;
	float ringstart = 0.2;
	float ringend = 1.2;
	float halostride = 0.02;
	float huestride = 0.02;
	float halostart = 0.3;
	float haloend = 1.2;
	float halofluct = 0.02;
	float haloturb = 0.002;
	float ringfluct = 0.03;
	float ringrmin1=0.02;
	float ringrmax1=0.03;
	float ringrmin2=0.06;
	float ringrmax2=0.08;
	float halorayt[2] = {CV_PI/2/40*18, CV_PI/2/40*22};

	Vec2 center(0,0);

	float* halop = new float[nhalo];
	halop[0] = halostart;
	for(int i=1;i<nhalo;i++)
	{
		halop[i] = halop[i-1]+(haloend-halostart)/nhalo;
		halop[i] += ((float)rand()/RAND_MAX*2-1)*halofluct;
	}
	float* ringp = new float[nring];
	ringp[0] = ringstart;
	for(int i=1;i<nring;i++)
	{
		ringp[i] = ringp[i-1]+(ringend-ringstart)/nring;
		ringp[i] += ((float)rand()/RAND_MAX*2-1)*ringfluct;
	}
	float* ringr = new float[nring];
	for(int i=0;i<nring;i++)
	{
		if(i>=3 && rand()%3==0)
			ringr[i] = ringrmin2+(float)rand()/RAND_MAX*(ringrmax2-ringrmin2);
		else
			ringr[i] = ringrmin1+(float)rand()/RAND_MAX*(ringrmax1-ringrmin1);
	}
	for(int i=0;i<2;i++)
	{
		halorayt[i] += ((float)rand()/RAND_MAX*2-1)*halorayturb;
	}
	IplImage* raytex = cvCreateImage(cvSize(400,1), IPL_DEPTH_32F, 1);
	IplImage* raytex2 = cvCreateImage(cvSize(1200,1), IPL_DEPTH_32F, 1);
	IplImage* huetex = cvCreateImage(cvSize(800,1), IPL_DEPTH_32F, 3);
	IplImage* halotex = cvCreateImage(cvSize(scale,1), IPL_DEPTH_32F, 1);
	for(int i=0;i<raytex->width;i++)
	{
		float frand = (float)rand()/RAND_MAX*2-1;
		*(float*)PTR_PIX(*raytex, i, 0) = frand;
	}
	for(int i=0;i<raytex2->width;i++)
	{
		float frand = (float)rand()/RAND_MAX*2-1;
		*(float*)PTR_PIX(*raytex2, i, 0) = frand;
	}
	GenHueTex(huetex);
	for(int i=0;i<halotex->width;i++)
	{
		float x = (float)i/halotex->width;
		float fmax = 0;
		for(int j=0;j<nhalo;j++)
		{
			if(x>halop[j]-halostride&&x<halop[j]+halostride)
			{
				float f = (cos((x-halop[j])/halostride*CV_PI)+1)/2*0.2;
				if(fmax<f)fmax=f;
			}
		}
		*(float*)PTR_PIX(*halotex, i, 0) = (i!=halotex->width-1?fmax:0);
	}
	float r0 = (raymaxs-raymins)/2;
	float r1 = (raymaxs+raymins)/2;
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			Vec2 p(j,i);
			Vec2 polar = p-center;
			float r = polar.length();
			float theta = acos(polar.x/r);
			if(r==0)
				theta = 0;
			else if(polar.y<0)
				theta = 2*CV_PI-theta;
			float s = cos(theta*nray)*r0+r1;
			float turb = Sample(raytex, Vec2(theta/(2*CV_PI), 0), WRAP_TYPE_REPEAT).x;
			s += turb*rayshifts;
			if(s<raymins)s=raymins;
			float l,lh=0;
			r-=rs;
			Vec3 vh(0,0,0);
			if(r<0)
			{
				l=1;
				lh=0;
			}
			else
			{
				l=exp(-pow(r/s, 2));
				if((theta>halorayt[0]-haloraystride/2&&theta<halorayt[0]+haloraystride/2)
					||(theta>halorayt[1]-haloraystride/2&&theta<halorayt[1]+haloraystride/2))
				{
					float turb2 = Sample(raytex2, Vec2(theta/(2*CV_PI), 0), WRAP_TYPE_REPEAT).x;
					lh=Sample(halotex, Vec2((r/scale+turb2*haloturb)*0.7,0)).x;
					lh *= (turb+1)/2;
					vh = Sample(huetex, Vec2((r/scale+turb2*haloturb)/huestride,0), WRAP_TYPE_REPEAT);
				}
			}
			uchar* pix = PTR_PIX(*dest, j, i);
			uchar* pixs = PTR_PIX(*src, j, i);
			Vec3 v(*pixs,*(pixs+1),*(pixs+2));
			if(l+lh>1)l=1-lh;
			Vec3 vd = Vec3(255,255,255)*l+lh*vh+v*(1-l-lh);
			*pix = max(0,min(255,vd.x));
			*(pix+1) = max(0,min(255,vd.y));
			*(pix+2) = max(0,min(255,vd.z));
		}
	}
	cvReleaseImage(&raytex);
	cvReleaseImage(&raytex2);
	cvReleaseImage(&huetex);
	cvReleaseImage(&halotex);
	delete[] halop;

	float ringt = CV_PI/2/nray*18;
	ringt += ((float)rand()/RAND_MAX*2-1)*halorayturb;
	for(int i=0;i<nring;i++)
	{
		Vec2 center(cos(ringt)*ringp[i]*scale, sin(ringt)*ringp[i]*scale);
		float radius = ringr[i]*ringp[i]*scale;
		DrawRing(dest, center, radius, Vec3(200,225,250));
	}
	delete[] ringp;
	delete[] ringr;
}

void CChildView::Moon(IplImage* src, IplImage* dest)
{
	float radius=0.05;
	float scale = min(src->width, src->height);
	float rs = radius*scale;
	Vec2 center(rs*2,rs*2);
	Mat b(150,0,0,110,75,55,100,70,50);
	Vec3 m(255,250,245);
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			Vec2 p(j,i);
			Vec2 polar = p-center;
			float r = polar.length();
			float theta = acos(polar.x/r);
			if(r==0)
				theta = 0;
			else if(polar.y<0)
				theta = 2*CV_PI-theta;
			r-=rs;
			float l;
			Vec3 vh(0,0,0);
			float s = scale/3;
			if(r<0)
			{
				l=1;
			}
			else
			{
				l=exp(-pow(r/s/2, 2))*0.8+exp(-pow(r/s*40, 2))*0.2;
			}
			uchar* pix = PTR_PIX(*dest, j, i);
			uchar* pixs = PTR_PIX(*src, j, i);
			Vec3 v(*pixs,*(pixs+1),*(pixs+2));
			Vec3 vd = m*l+v*b/255*(1-l);
			*pix = max(0,min(255,vd.x));
			*(pix+1) = max(0,min(255,vd.y));
			*(pix+2) = max(0,min(255,vd.z));
		}
	}
}

Vec2 RandVec()
{
	return Vec2(2*(float)rand()/RAND_MAX-1,2*(float)rand()/RAND_MAX-1);
}

struct C3DProcData
{
	IplImage* src;
	IplImage* tex;
	IplImage* mask;
	Vec3 pos;
	float radius;
	Vec3 cl;
	Vec3 acc;
	float acc2;
};

void PixelFunc(RawImage* target, Point2D pos, int count, float* pdata, void* p)
{
	IplImage* imgsrc = ((C3DProcData*)p)->src;
	IplImage* imgtex = ((C3DProcData*)p)->tex;
	ASSERT(count==2);
	ASSERT(imgsrc != NULL);
	ASSERT(imgtex != NULL);
	Vec2* tex = (Vec2*)pdata;
	Vec2 text = (*tex-Vec2(0.5,0.5))*2;
	//text.x/=2;
	float l = exp(-4*dot(text,text));
	uchar* pix=PTR_PIX(*target, pos.x, pos.y);
	uchar* pixs=PTR_PIX(*imgsrc, pos.x, pos.y);
	Vec3 cs(*pixs, *(pixs+1), *(pixs+2));
	Vec3 ch = Sample(imgtex, Vec2(tex->y*1.3, 0), WRAP_TYPE_REPEAT);
	Vec3 cd = ch*l+cs*(1-l);
	*pix=max(0,min(255,cd.x));
	*(pix+1)=max(0,min(255,cd.y));
	*(pix+2)=max(0,min(255,cd.z));
}

void Stroke(IplImage* dest, Vec2 pt, Vec2 dir, float arc, float w, float l, int nSeg, C3DProcData* pData)
{
	float radius = 1./arc;
	Vec2 polar(dir.y, -dir.x);
	float theta = acos(-polar.x/polar.length());
	if(polar.y>0)theta=-theta;
	polar = polar.normalize();
	dir=dir.normalize();
	Vec2 c = pt+polar*radius;
	float dtheta = l/2/radius;
	if(arc!=0)
	{
		for(int i=0;i<nSeg;i++)
		{
			Vec2 tri[2][3];
			float vdata[2][2][3];
			float r1 = radius-w/2;
			float r2 = radius+w/2;
			float th1=theta-dtheta+dtheta/nSeg*2*i;
			float th2=th1+dtheta/nSeg*2;
			tri[0][0] = c+r1*Vec2(cos(th1),sin(th1));
			tri[0][1] = c+r2*Vec2(cos(th1),sin(th1));
			tri[0][2] = c+r2*Vec2(cos(th2),sin(th2));
			tri[1][0] = tri[0][2];
			tri[1][1] = c+r1*Vec2(cos(th2),sin(th2));
			tri[1][2] = tri[0][0];
			vdata[0][0][0]=1./nSeg*i;
			vdata[0][1][0]=0;
			vdata[0][0][1]=1./nSeg*i;
			vdata[0][1][1]=1;
			vdata[0][0][2]=1./nSeg*(i+1);
			vdata[0][1][2]=1;
			vdata[1][0][0]=1./nSeg*(i+1);
			vdata[1][1][0]=1;
			vdata[1][0][1]=1./nSeg*(i+1);
			vdata[1][1][1]=0;
			vdata[1][0][2]=1./nSeg*i;
			vdata[1][1][2]=0;
			for(int j=0;j<2;j++)
				Triangle2D(dest, tri[j], 2, vdata[j], PixelFunc, pData);
		}
	}
	else
	{
		for(int i=0;i<nSeg;i++)
		{
			Vec2 tri[2][3];
			float vdata[2][2][3];
			tri[0][0] = pt-l/2*dir+l/nSeg*i*dir-polar*w/2;
			tri[0][1] = pt-l/2*dir+l/nSeg*i*dir+polar*w/2;
			tri[0][2] = pt-l/2*dir+l/nSeg*(i+1)*dir+polar*w/2;
			tri[1][0] = tri[0][2];
			tri[1][1] = pt-l/2*dir+l/nSeg*(i+1)*dir-polar*w/2;
			tri[1][2] = tri[0][0];
			vdata[0][0][0]=1./nSeg*i;
			vdata[0][1][0]=0;
			vdata[0][0][1]=1./nSeg*i;
			vdata[0][1][1]=1;
			vdata[0][0][2]=1./nSeg*(i+1);
			vdata[0][1][2]=1;
			vdata[1][0][0]=1./nSeg*(i+1);
			vdata[1][1][0]=1;
			vdata[1][0][1]=1./nSeg*(i+1);
			vdata[1][1][1]=0;
			vdata[1][0][2]=1./nSeg*i;
			vdata[1][1][2]=0;
			for(int j=0;j<2;j++)
				Triangle2D(dest, tri[j], 2, vdata[j], PixelFunc, pData);
		}
	}
}

void Rainbow(IplImage* src, IplImage* dest)
{
	//cvSet(dest, cvScalarAll(255));
	cvCopyImage(src, dest);

	IplImage* huetex = cvCreateImage(cvSize(800,1), IPL_DEPTH_32F, 3);
	GenHueTex(huetex);

	C3DProcData data;
	data.src = src;
	data.tex = huetex;
	float scale = max(src->width, src->height);
	float w=scale/10;
	float l=src->width;
	float arc = -1.0/scale;
	Vec2 dir(1,0);
	Vec2 pt(src->width/2, src->height/5);
	int nSeg = l/10;
	Stroke(dest, pt, dir, arc, w, l, nSeg, &data);
	cvReleaseImage(&huetex);
}

void PixelFunc2(RawImage* target, Point2D pos, int count, float* pdata, void* p)
{
	IplImage* imgtex = ((C3DProcData*)p)->tex;
	ASSERT(count==2);
	ASSERT(imgtex!=NULL);
	ASSERT(imgtex->nChannels==4);
	Vec2* tex = (Vec2*)pdata;
	Vec4 c = SampleEx(imgtex, *tex);
	uchar* pix = PTR_PIX(*target, pos.x, pos.y);
	Vec3 cs(*pix,*(pix+1),*(pix+2));
	float gray = (c.x+c.y+c.z)/3;
	Vec3 ct = ((C3DProcData*)p)->cl*(gray/255)*1.5;
	float alpha = c.t/255;
	Vec3 cd = ct*alpha+cs*(1-alpha);
	*pix = max(0,min(255,cd.x));
	*(pix+1) = max(0,min(255,cd.y));
	*(pix+2) = max(0,min(255,cd.z));
}

void PixelFunc21(RawImage* target, Point2D pos, int count, float* pdata, void* p)
{
	IplImage* imgtex = ((C3DProcData*)p)->tex;
	ASSERT(count==2);
	ASSERT(imgtex!=NULL);
	ASSERT(imgtex->nChannels==4);
	Vec2* tex = (Vec2*)pdata;
	Vec4 c = SampleEx(imgtex, *tex);
	uchar* pix = PTR_PIX(*((C3DProcData*)p)->src, pos.x, pos.y);
	Vec3 cs(*pix,*(pix+1),*(pix+2));
	((C3DProcData*)p)->acc+=cs*c.t/255;
	((C3DProcData*)p)->acc2+=c.t/255;
}
//Paint
void CChildView::Process2(IplImage* src, IplImage* dest)
{
	cvCopyImage(src, dest);
	int KernelSize = 20;
	int nFilterScale = 10;
	CvScalar st = cvSum(m_tex);
	float mt = (st.val[0]+st.val[1]+st.val[2])/3/st.val[3];

	CvMat* kernelx = cvCreateMat(2 * KernelSize + 1, 2 * KernelSize + 1, CV_32FC1);
	CvMat* kernely = cvCreateMat(2 * KernelSize + 1, 2 * KernelSize + 1, CV_32FC1);

	float ksum = 0;
	for(int i = -KernelSize; i <= KernelSize; i++)
	{
		for(int j = -KernelSize; j <= KernelSize; j++)
		{
			float sqrk = nFilterScale*nFilterScale;
			float coeff = exp( -(float)(i*i + j*j) / sqrk );
			ksum += coeff;
			cvSet2D(kernelx, i + KernelSize, j + KernelSize, cvScalar(-2*j * coeff / sqrk));
			cvSet2D(kernely, i + KernelSize, j + KernelSize, cvScalar(-2*i * coeff / sqrk));
		}
	}
	cvScale(kernelx, kernelx, 1/ksum);
	cvScale(kernely, kernely, 1/ksum);

	IplImage* gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage* sum = cvCreateImage(cvSize(src->width+1,src->height+1), IPL_DEPTH_32S, 3);
	cvIntegral(src, sum);
	cvCvtColor(src, gray, CV_RGB2GRAY);
	IplImage* tmp = gray;
	gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	cvCvtScale(tmp, gray);
	cvReleaseImage(&tmp);

	IplImage* dx = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	IplImage* dy = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);

	cvFilter2D(gray, dx, kernelx, cvPoint(KernelSize, KernelSize));
	cvFilter2D(gray, dy, kernely, cvPoint(KernelSize, KernelSize));

	cvReleaseImage(&gray);

	IplImage* dxx = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	IplImage* dyy = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	IplImage* dxy = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);

	cvMul(dx,dx,dxx);
	cvMul(dy,dy,dyy);
	cvMul(dx,dy,dxy);

	cvReleaseImage(&dx);
	cvReleaseImage(&dy);
	cvReleaseMat(&kernelx);
	cvReleaseMat(&kernely);

	IplImage* sdxx = cvCreateImage(cvSize(src->width+1,src->height+1), IPL_DEPTH_64F, 1);
	IplImage* sdyy = cvCreateImage(cvSize(src->width+1,src->height+1), IPL_DEPTH_64F, 1);
	IplImage* sdxy = cvCreateImage(cvSize(src->width+1,src->height+1), IPL_DEPTH_64F, 1);

	cvIntegral(dxx, sdxx);
	cvIntegral(dyy, sdyy);
	cvIntegral(dxy, sdxy);

	cvReleaseImage(&dxx);
	cvReleaseImage(&dyy);
	cvReleaseImage(&dxy);

	int span = 20;
	int span2 = 5;
	Vec2 veigenold[2] = {Vec2(10,0),Vec2(0,10)};

	Vec2 texcoord[4] = {Vec2(0,0),Vec2(1,0),Vec2(0,1),Vec2(1,1)};
	int index[2][3] = {{0,1,3},{3,2,0}};

	C3DProcData data;
	data.tex = m_tex;
	data.src = src;
	for(int i=0;i<src->height;i+=10)
	{
		for(int j=0;j<src->width;j+=10)
		{
			int i0 = max(0,i-span),i1 = min(src->height,i+span);
			int j0 = max(0,j-span),j1 = min(src->width,j+span);
			int i02 = max(0,i-span2),i12 = min(src->height,i+span2);
			int j02 = max(0,j-span2),j12 = min(src->width,j+span2);

			float xx=0,xy=0,yy=0;
			//Vec3 mean(0,0,0);
			int cnt = (i1-i0)*(j1-j0);
			int cnt2 = (i12-i02)*(j12-j02);
			xx = *(double*)PTR_PIX(*sdxx, j1, i1)
				-*(double*)PTR_PIX(*sdxx, j0, i1)
				-*(double*)PTR_PIX(*sdxx, j1, i0)
				+*(double*)PTR_PIX(*sdxx, j0, i0);
			xy = *(double*)PTR_PIX(*sdxy, j1, i1)
				-*(double*)PTR_PIX(*sdxy, j0, i1)
				-*(double*)PTR_PIX(*sdxy, j1, i0)
				+*(double*)PTR_PIX(*sdxy, j0, i0);
			yy = *(double*)PTR_PIX(*sdyy, j1, i1)
				-*(double*)PTR_PIX(*sdyy, j0, i1)
				-*(double*)PTR_PIX(*sdyy, j1, i0)
				+*(double*)PTR_PIX(*sdyy, j0, i0);

			int* tl = (int*)PTR_PIX(*sum, j02, i02);
			int* tr = (int*)PTR_PIX(*sum, j12, i02);
			int* bl = (int*)PTR_PIX(*sum, j02, i12);
			int* br = (int*)PTR_PIX(*sum, j12, i12);
			Vec3 cl = Vec3(*br,*(br+1),*(br+2))
					-Vec3(*bl,*(bl+1),*(bl+2))
					-Vec3(*tr,*(tr+1),*(tr+2))
					+Vec3(*tl,*(tl+1),*(tl+2));

			//mean/=cnt;
			xx/=cnt;
			yy/=cnt;
			xy/=cnt;
			cl/=cnt2;

			float delta = sqrt(pow(xx-yy,2)+4*pow(xy,2));
			Vec2 veigen[2] = {Vec2(-xy,(xx-yy+delta)/2), Vec2(-xy,(xx-yy-delta)/2)};
			float len[2] = {0,0};
			Vec2 norm[2];
			if((veigen[0].length()==0
				|| veigen[1].length()==0) && xx!=yy)
			{
				veigen[0] = Vec2(1,0);
				veigen[1] = Vec2(0,1);
			}
			if(xx!=yy)
			{
				for(int k=0;k<2;k++)
				{
					norm[k] = veigen[k].normalize();
					len[k] = xx*powf(norm[k].x,2)+yy*powf(norm[k].y,2)+2*xy*norm[k].x*norm[k].y;
				}
			}
			if(len[0]>len[1])
			{
				swap(veigen[0], veigen[1]);
				swap(len[0], len[1]);
				swap(norm[0], norm[1]);
			}
			if(xx!=yy)
			{
				for(int k=0;k<2;k++)
				{
					if(len[k] != 0)
						len[k] = max(10,min(40,sqrt(1./len[k])*10));
					else
						len[k] = 40;
					veigen[k] = norm[k]*len[k];
				}
			}
			else
			{
				veigen[0] = veigenold[0];
				veigen[1] = veigenold[1];
			}
			veigenold[0] = veigen[0];
			veigenold[1] = veigen[1];

			//Draw
#if 0
			for(int k=0;k<2;k++)
			{
				float len = veigen[k].length();
				Vec2 norm = veigen[k];
				if(len>0)
					norm = veigen[k].normalize();

				for(int l=-len;l<=len;l++)
				{
					Vec2 pt = Vec2(j,i)+norm*l;
					int x=max(0,min(dest->width-1,pt.x));
					int y=max(0,min(dest->height-1,pt.y));
					uchar* pix = PTR_PIX(*dest, x, y);
					*pix = 0;
					*(pix+1) = 0;
					*(pix+2) = 0;
				}
			}
#else
			Vec2 center(j,i);
			Vec2 vert[4] = {center-veigen[0]-veigen[1], center+veigen[0]-veigen[1],
				center-veigen[0]+veigen[1], center+veigen[0]+veigen[1]};
			data.cl = cl;
			data.acc = Vec3(0,0,0);
			data.acc2 = 0;
			for(int k=0;k<2;k++)
			{
				Vec2 v[3];
				float tex[2][3];
				for(int l=0;l<3;l++)
				{
					v[l] = vert[index[k][l]];
					tex[0][l] = texcoord[index[k][l]].x;
					tex[1][l] = texcoord[index[k][l]].y;
				}
				Triangle2D(dest, v, 2, tex, PixelFunc21, &data);
			}
			data.cl = data.acc;
			if(data.acc2!=0)
				data.cl/=data.acc2;
			data.cl/=mt;
			for(int k=0;k<2;k++)
			{
				Vec2 v[3];
				float tex[2][3];
				for(int l=0;l<3;l++)
				{
					v[l] = vert[index[k][l]];
					tex[0][l] = texcoord[index[k][l]].x;
					tex[1][l] = texcoord[index[k][l]].y;
				}
				Triangle2D(dest, v, 2, tex, PixelFunc2, &data);
			}
#endif
		}
	}
	cvReleaseImage(&sum);
	cvReleaseImage(&sdxx);
	cvReleaseImage(&sdyy);
	cvReleaseImage(&sdxy);
}

void Paint2(IplImage* src, IplImage* dest/*, IplImage* tex*/)
{
	cvCopyImage(src, dest);
	int KernelSize = 20;
	int nFilterScale = 10;

	CvMat* kernelx = cvCreateMat(2 * KernelSize + 1, 2 * KernelSize + 1, CV_32FC1);
	CvMat* kernely = cvCreateMat(2 * KernelSize + 1, 2 * KernelSize + 1, CV_32FC1);

	float ksum = 0;
	for(int i = -KernelSize; i <= KernelSize; i++)
	{
		for(int j = -KernelSize; j <= KernelSize; j++)
		{
			float sqrk = nFilterScale*nFilterScale;
			float coeff = exp( -(float)(i*i + j*j) / sqrk );
			ksum += coeff;
			cvSet2D(kernelx, i + KernelSize, j + KernelSize, cvScalar(-2*j * coeff / sqrk));
			cvSet2D(kernely, i + KernelSize, j + KernelSize, cvScalar(-2*i * coeff / sqrk));
		}
	}
	cvScale(kernelx, kernelx, 1/ksum);
	cvScale(kernely, kernely, 1/ksum);

	IplImage* gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage* sum = cvCreateImage(cvSize(src->width+1,src->height+1), IPL_DEPTH_32S, 3);
	cvIntegral(src, sum);
	cvCvtColor(src, gray, CV_RGB2GRAY);
	IplImage* tmp = gray;
	gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	cvCvtScale(tmp, gray);
	cvReleaseImage(&tmp);

	IplImage* dx = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	IplImage* dy = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);

	cvFilter2D(gray, dx, kernelx, cvPoint(KernelSize, KernelSize));
	cvFilter2D(gray, dy, kernely, cvPoint(KernelSize, KernelSize));

	cvReleaseImage(&gray);

	IplImage* dxx = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	IplImage* dyy = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	IplImage* dxy = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);

	cvMul(dx,dx,dxx);
	cvMul(dy,dy,dyy);
	cvMul(dx,dy,dxy);

	cvReleaseImage(&dx);
	cvReleaseImage(&dy);
	cvReleaseMat(&kernelx);
	cvReleaseMat(&kernely);

	IplImage* sdxx = cvCreateImage(cvSize(src->width+1,src->height+1), IPL_DEPTH_64F, 1);
	IplImage* sdyy = cvCreateImage(cvSize(src->width+1,src->height+1), IPL_DEPTH_64F, 1);
	IplImage* sdxy = cvCreateImage(cvSize(src->width+1,src->height+1), IPL_DEPTH_64F, 1);

	cvIntegral(dxx, sdxx);
	cvIntegral(dyy, sdyy);
	cvIntegral(dxy, sdxy);

	cvReleaseImage(&dxx);
	cvReleaseImage(&dyy);
	cvReleaseImage(&dxy);

	int span = 20;
	int span2 = 5;
	Vec2 veigenold[2] = {Vec2(1,0),Vec2(0,1)};

	Vec2 texcoord[4] = {Vec2(0,0),Vec2(1,0),Vec2(0,1),Vec2(1,1)};
	int index[2][3] = {{0,1,3},{3,2,0}};

	IplImage* spectrum = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 4);
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			int i0 = max(0,i-span),i1 = min(src->height,i+span);
			int j0 = max(0,j-span),j1 = min(src->width,j+span);
			int i02 = max(0,i-span2),i12 = min(src->height,i+span2);
			int j02 = max(0,j-span2),j12 = min(src->width,j+span2);

			float xx=0,xy=0,yy=0;
			//Vec3 mean(0,0,0);
			int cnt = (i1-i0)*(j1-j0);
			int cnt2 = (i12-i02)*(j12-j02);
			xx = *(double*)PTR_PIX(*sdxx, j1, i1)
				-*(double*)PTR_PIX(*sdxx, j0, i1)
				-*(double*)PTR_PIX(*sdxx, j1, i0)
				+*(double*)PTR_PIX(*sdxx, j0, i0);
			xy = *(double*)PTR_PIX(*sdxy, j1, i1)
				-*(double*)PTR_PIX(*sdxy, j0, i1)
				-*(double*)PTR_PIX(*sdxy, j1, i0)
				+*(double*)PTR_PIX(*sdxy, j0, i0);
			yy = *(double*)PTR_PIX(*sdyy, j1, i1)
				-*(double*)PTR_PIX(*sdyy, j0, i1)
				-*(double*)PTR_PIX(*sdyy, j1, i0)
				+*(double*)PTR_PIX(*sdyy, j0, i0);

			int* tl = (int*)PTR_PIX(*sum, j02, i02);
			int* tr = (int*)PTR_PIX(*sum, j12, i02);
			int* bl = (int*)PTR_PIX(*sum, j02, i12);
			int* br = (int*)PTR_PIX(*sum, j12, i12);
			Vec3 cl = Vec3(*br,*(br+1),*(br+2))
				-Vec3(*bl,*(bl+1),*(bl+2))
				-Vec3(*tr,*(tr+1),*(tr+2))
				+Vec3(*tl,*(tl+1),*(tl+2));

			//mean/=cnt;
			xx/=cnt;
			yy/=cnt;
			xy/=cnt;
			cl/=cnt2;

			float delta = sqrt(pow(xx-yy,2)+4*pow(xy,2));
			Vec2 veigen[2] = {Vec2(-xy,(xx-yy+delta)/2), Vec2(-xy,(xx-yy-delta)/2)};
			float len[2] = {0,0};
			Vec2 norm[2];
			if((veigen[0].length()==0
				|| veigen[1].length()==0) && xx!=yy)
			{
				veigen[0] = Vec2(1,0);
				veigen[1] = Vec2(0,1);
			}
			if(xx!=yy)
			{
				for(int k=0;k<2;k++)
				{
					norm[k] = veigen[k].normalize();
					len[k] = xx*powf(norm[k].x,2)+yy*powf(norm[k].y,2)+2*xy*norm[k].x*norm[k].y;
				}
			}
			if(fabs(dot(norm[0], veigenold[0].normalize()))<0.5)
			{
				swap(veigen[0], veigen[1]);
				swap(len[0], len[1]);
				swap(norm[0], norm[1]);
			}
			if(xx!=yy)
			{
				for(int k=0;k<2;k++)
				{
					veigen[k] = norm[k]*len[k];
				}
			}
			else
			{
				veigen[0] = veigenold[0];
				veigen[1] = veigenold[1];
			}
			veigenold[0] = veigen[0];
			veigenold[1] = veigen[1];

			Vec2* p = (Vec2*)PTR_PIX(*spectrum, j, i);
			p[0] = veigen[0];
			p[1] = veigen[1];
		}
	}
	cvReleaseImage(&sum);
	cvReleaseImage(&sdxx);
	cvReleaseImage(&sdyy);
	cvReleaseImage(&sdxy);
	IplImage* imgtexcoord = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 2);
	for(int i=0;i<src->height;i++)
	{
		float x=0;
		if(i==0)
		{
			for(int j=0;j<src->width;j++)
			{
				float* px = (float*)PTR_PIX(*imgtexcoord, j, i);
				Vec2* pv = (Vec2*)PTR_PIX(*spectrum, j, i);
				x+=dot(pv[0],Vec2(1,0));
				px[0] = x;
			}
		}
		else
		{
			for(int j=0;j<src->width;j++)
			{
				int pi=i-1;
				int nj=min(j+1, src->width-1);
				float* px = (float*)PTR_PIX(*imgtexcoord, j, i);
				float* ppx = (float*)PTR_PIX(*imgtexcoord, j, pi);
				float* pnx = (float*)PTR_PIX(*imgtexcoord, nj, pi);
				Vec2* pv = (Vec2*)PTR_PIX(*spectrum, j, i);
				float dx=pnx[0]-ppx[0];
				float sx = dot(Vec2(1,0), pv[0]);
				float sy = dot(Vec2(0,1), pv[0]);
				float dy;
				if(sx == 0)
				{
					dy = sy;
				}
				else
				{
					dy = max(-5,min(5,dx/sx*sy));
				}
				x = x*0.8+0.2*(ppx[0]+dy);
				px[0] = x;
			}
		}
	}
	for(int i=0;i<src->width;i++)
	{
		float y=0;
		if(i == 0)
		{
			for(int j=0;j<src->height;j++)
			{
				float* py = (float*)PTR_PIX(*imgtexcoord, i, j);
				Vec2* pv = (Vec2*)PTR_PIX(*spectrum, i, j);
				y+=dot(pv[1],Vec2(0,1));
				py[1] = y;
			}
		}
		else
		{
			for(int j=0;j<src->height;j++)
			{
				int pi=i-1;
				int nj=min(j+1, src->height-1);
				float* py = (float*)PTR_PIX(*imgtexcoord, i, j);
				float* ppy = (float*)PTR_PIX(*imgtexcoord, pi, j);
				float* pny = (float*)PTR_PIX(*imgtexcoord, pi, nj);
				Vec2* pv = (Vec2*)PTR_PIX(*spectrum, i, j);
				float dy=pny[1]-ppy[1];
				float sx = dot(Vec2(1,0), pv[1]);
				float sy = dot(Vec2(0,1), pv[1]);
				float dx;
				if(sy == 0)
				{
					dx = sx;
				}
				else
				{
					dx = max(-5,min(5,dy/sy*sx));
				}
				y = 0.2*y+0.8*(ppy[1]+dx);
				py[1] = y;
			}
		}
	}
	cvReleaseImage(&spectrum);
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			Vec2 pv = *(Vec2*)PTR_PIX(*imgtexcoord, j, i);
			uchar* pix = PTR_PIX(*dest, j, i);
			pv/=60;
			float r=(1+cos(pv.x*2*CV_PI))*0.5*255;
			float g=(1+cos(pv.y*2*CV_PI))*0.5*255;
			*pix = 0;
			*(pix+1) = max(0,min(255,g));
			*(pix+2) = max(0,min(255,r));
		}
	}
	cvReleaseImage(&imgtexcoord);
}

void CChildView::Process3(IplImage* src, IplImage* dest)
{
	IplImage* tex = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 4);
	cvResize(m_tex, tex);
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			uchar* pixs = PTR_PIX(*src, j, i);
			uchar* pix = PTR_PIX(*dest, j, i);
			uchar* pixt = PTR_PIX(*tex, j, i);
			float alpha = (float)*(pixt+3)/255;
			Vec3 cs(*pixs,*(pixs+1),*(pixs+2));
			Vec3 ct(*pixt,*(pixt+1),*(pixt+2));
			Vec3 cd = ct*alpha+cs*(1-alpha);
			*pix = cd.x;
			*(pix+1) = cd.y;
			*(pix+2) = cd.z;
		}
	}
	cvReleaseImage(&tex);
}
//water glass
void CChildView::Process4(IplImage* src, IplImage* dest)
{
	IplImage* noise = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	float scalex = 2;
	float scaley = 2;
	float scale = min(scalex,scaley);
	float nx = (float)noise->width/scalex;
	float ny = (float)noise->height/scaley;
	Perlin(noise, nx, ny);
	Vec3 vl(-0.2,-0.2,1);
	vl = vl.normalize();
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			float h=*(float*)PTR_PIX(*noise, j, i);
			float hx=*(float*)PTR_PIX(*noise, min(j+1,noise->width-1),i);
			float hy=*(float*)PTR_PIX(*noise, j, min(i+1,noise->height-1));

			Vec2  norm(h-hx,h-hy);
			Vec3 norm3(norm.x,norm.y,2./scale);
			norm3=norm3.normalize();
			float l=dot(norm3, vl);
			float spec = pow(l,100);
			l=fabs(l);

			Vec2 offset(0,0);
			float ffrac=0.75;
			Vec3 lin(0,0,1);
			Vec3 offin=lin-dot(lin, norm3)*norm3;
			Vec3 offout=offin*ffrac;
			float frac=sqrt(1-dot(offout,offout));
			Vec3 refrac=frac*norm3+offout;
			offset=Vec2(refrac.x,refrac.y)/refrac.z;//*h;

			int ni=i+offset.x*scale*50;
			int nj=j+offset.y*scale*50;
			if(ni<0)ni=0;
			else if(ni>=dest->height)ni=dest->height-1;
			if(nj<0)nj=0;
			else if(nj>=dest->width)nj=dest->width-1;

			uchar* pixs = PTR_PIX(*src, nj, ni);
			uchar* pix = PTR_PIX(*dest, j, i);
			//float alpha = *(float*)PTR_PIX(*noise, j, i);
			Vec3 v(*pixs,*(pixs+1),*(pixs+2));
			Vec3 vd = v*l+Vec3(spec,spec,spec)*70;
			*pix = max(0,min(255,vd.x));
			*(pix+1) = max(0,min(255,vd.y));
			*(pix+2) = max(0,min(255,vd.z));
		}
	}
	cvReleaseImage(&noise);
}
//colorfull mood
void CChildView::Process5(IplImage* src, IplImage* dest)
{
	IplImage* fnoise = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	float scale = min(src->width, src->height);
	float fDense = 2;
	float fIntense = 0.5;
	float nx = fDense*src->width/scale;
	float ny = fDense*src->height/scale;
	Perlin(fnoise, nx, ny);
	IplImage* noise3c = cvCreateImage(cvGetSize(m_dest), IPL_DEPTH_8U, 3);
	for(int i=0;i<noise3c->height;i++)
	{
		for(int j=0;j<noise3c->width;j++)
		{
			uchar* ptr3c = PTR_PIX(*noise3c, j, i);
			float f = *(float*)PTR_PIX(*fnoise, j, i);
			f*=2*CV_PI;
			float f2 = f+2*CV_PI/3;
			float f3 = f-2*CV_PI/3;
			Vec3 v(128+128*fIntense*cosf(f), 128+128*fIntense*cosf(f2), 128+128*fIntense*cosf(f3));
			*ptr3c = max(0, min(255, v.x));
			*(ptr3c+1) = max(0, min(255, v.y));
			*(ptr3c+2) = max(0, min(255, v.z));
		}
	}
	cvMul(noise3c, src, dest, 1.0/128);
	cvReleaseImage(&noise3c);
	cvReleaseImage(&fnoise);
}

//self-glowing
void CChildView::Process6(IplImage* src, IplImage* dest)
{
	int range = 10;
	float thresh = 100;
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			int ni = i+((float)rand()/RAND_MAX*2-1)*range;
			int nj = j+((float)rand()/RAND_MAX*2-1)*range;
			ni = max(0,min(src->height-1,ni));
			nj = max(0,min(src->width-1,nj));
			uchar* pixs = PTR_PIX(*src, j, i);
			uchar* pixn = PTR_PIX(*src, nj, ni);
			uchar* pix = PTR_PIX(*dest, j, i);
			Vec3 cs(*pixs,*(pixs+1),*(pixs+2));
			Vec3 cd;
			int gray = ((int)*pixn+(int)*(pixn+1)+(int)*(pixn+2))/3;
			if(gray>=thresh)
				cd = cs+Vec3(50,50,50);
			else
				cd = cs;
			*pix = max(0,min(255,cd.x));
			*(pix+1) = max(0,min(255,cd.y));
			*(pix+2) = max(0,min(255,cd.z));
		}
	}
}
//self-glowing2
void CChildView::Process7(IplImage* src, IplImage* dest)
{
	int range = 10;
	float w=20;
	CvScalar mean = cvSum(src);
	Vec3 mc(mean.val[0],mean.val[1],mean.val[2]);
	mc/=(src->width*src->height);
	float thresh = (mc.x+mc.y+mc.z)/3;
	IplImage* sum = cvCreateImage(cvSize(src->width+1,src->height+1), IPL_DEPTH_32S, 3);
	cvIntegral(src, sum);
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			int i0 = max(0,i-range),i1 = min(src->height,i+range);
			int j0 = max(0,j-range),j1 = min(src->width,j+range);
			int cnt = (i1-i0)*(j1-j0);
			int* tl = (int*)PTR_PIX(*sum, j0, i0);
			int* tr = (int*)PTR_PIX(*sum, j1, i0);
			int* bl = (int*)PTR_PIX(*sum, j0, i1);
			int* br = (int*)PTR_PIX(*sum, j1, i1);
			Vec3 cl = Vec3(*br,*(br+1),*(br+2))
				-Vec3(*bl,*(bl+1),*(bl+2))
				-Vec3(*tr,*(tr+1),*(tr+2))
				+Vec3(*tl,*(tl+1),*(tl+2));
			cl/=cnt;
			float gray = (cl.x+cl.y+cl.z)/3;
			float intense = SmoothStep(gray, thresh, w)*0.5;
			uchar* pixs = PTR_PIX(*src, j, i);
			uchar* pix = PTR_PIX(*dest, j, i);
			Vec3 c(*pixs, *(pixs+1), *(pixs+2));
			Vec3 cd = cl*intense+c*(1-intense);
			*pix = max(0,min(255,cd.x));
			*(pix+1) = max(0,min(255,cd.y));
			*(pix+2) = max(0,min(255,cd.z));
		}
	}
	cvReleaseImage(&sum);
}

void ComputeShiftImg(IplImage* src, IplImage* dest, IplImage* himg, float scale)
{
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			float dx,dy;
			float h = *(float*)PTR_PIX(*himg, j, i);
			if(i == src->height-1)
			{
				dy = 0;
			}
			else
			{
				float nh = *(float*)PTR_PIX(*himg, j, i+1);
				dy = nh-h;
			}

			if(j == src->width-1)
			{
				dx = 0;
			}
			else
			{
				float nh = *(float*)PTR_PIX(*himg, j+1, i);
				dx = nh-h;
			}

			float ni = i-dy*scale;
			float nj = j-dx*scale;

			if(ni<0)ni=0;
			else if(ni>=src->height)ni = src->height-1;
			if(nj<0)nj=0;
			else if(nj>=src->width)nj = src->width-1;

			float* pix = (float*)PTR_PIX(*dest, j, i);
			float p = Sample(src, Vec2(nj,ni), WRAP_TYPE_CLAMP, TEX_FILTER_LINEAR, false).x;

			*pix = p;
		}
	}
}

void GenWaterMap(IplImage* dest, float scalex, float scaley)
{
	float nx = (float)dest->width/scalex;
	float ny = (float)dest->height/scaley;
	Perlin(dest, nx, ny);
}

void GenWaterMap2(IplImage* dest, float scalex, float scaley)
{
	IplImage* noise = cvCreateImage(cvGetSize(dest), IPL_DEPTH_32F, 1);
	IplImage* noise2 = cvCreateImage(cvGetSize(dest), IPL_DEPTH_32F, 1);
	float nx = (float)noise->width/scalex;
	float ny = (float)noise->height/scaley;
	float nx2 = nx*2;
	float ny2 = ny*2;
	Perlin(noise, nx, ny);
	Perlin(noise2, nx2, ny2);
	cvScale(noise2,noise2,0.5);
	cvAdd(noise, noise2, dest);
	cvScale(dest,dest,0.6);
	cvReleaseImage(&noise2);
	cvReleaseImage(&noise);
}
//wave
void CChildView::Process8(IplImage* src, IplImage* dest)
{
	float scalex = 20;
	float scaley = 20;
	float scale = min(scalex,scaley);
	IplImage* noise = cvCreateImage(cvGetSize(dest), IPL_DEPTH_32F, 1);
	GenWaterMap(noise,scalex,scaley);
	IplImage* wave = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	ComputeShiftImg(noise, wave, noise, -scale*scale/8);
	cvReleaseImage(&noise);
	Vec3 vl(0.2,0.2,1);
	vl = vl.normalize();
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			float h=*(float*)PTR_PIX(*wave, j, i);
			float hx=*(float*)PTR_PIX(*wave, min(j+1,wave->width-1),i);
			float hy=*(float*)PTR_PIX(*wave, j, min(i+1,wave->height-1));

			Vec2  norm(h-hx,h-hy);
			Vec3 norm3(norm.x,norm.y,2./scale);
			norm3=norm3.normalize();
			float l=dot(norm3, vl);
			float spec = pow(l,100);
			l=fabs(l);

			Vec2 offset(0,0);
			float ffrac=0.75;
			Vec3 lin(0,0,1);
			Vec3 offin=lin-dot(lin, norm3)*norm3;
			Vec3 offout=offin*ffrac;
			float frac=sqrt(1-dot(offout,offout));
			Vec3 refrac=frac*norm3+offout;
			offset=Vec2(refrac.x,refrac.y)/refrac.z;//*h;

			int ni=i+offset.x*scale*10;
			int nj=j+offset.y*scale*10;
			if(ni<0)ni=0;
			else if(ni>=dest->height)ni=dest->height-1;
			if(nj<0)nj=0;
			else if(nj>=dest->width)nj=dest->width-1;

			uchar* pixs = PTR_PIX(*src, nj, ni);
			uchar* pix = PTR_PIX(*dest, j, i);
			//float alpha = *(float*)PTR_PIX(*noise, j, i);
			Vec3 v(*pixs,*(pixs+1),*(pixs+2));
			Vec3 vd = v*l+Vec3(spec,spec,spec)*70;
			*pix = max(0,min(255,vd.x));
			*(pix+1) = max(0,min(255,vd.y));
			*(pix+2) = max(0,min(255,vd.z));
		}
	}
	cvReleaseImage(&wave);
}

//wave2
void CChildView::Process9(IplImage* src, IplImage* dest, float scalex, float scaley)//20,20
{
	float scale = min(scalex,scaley);
	IplImage* noise = cvCreateImage(cvGetSize(dest), IPL_DEPTH_32F, 1);
	GenWaterMap2(noise,scalex,scaley);
	IplImage* wave = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	ComputeShiftImg(noise, wave, noise, -scale*scale/8);
	cvReleaseImage(&noise);
	Vec3 vl(0.2,0.2,1);
	vl = vl.normalize();
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			float h=*(float*)PTR_PIX(*wave, j, i);
			float hx=*(float*)PTR_PIX(*wave, min(j+1,wave->width-1),i);
			float hy=*(float*)PTR_PIX(*wave, j, min(i+1,wave->height-1));

			Vec2  norm(h-hx,h-hy);
			Vec3 norm3(norm.x,norm.y,2./scale);
			norm3=norm3.normalize();
			float l=dot(norm3, vl);
			float spec = pow(l,100);
			l=fabs(l);

			Vec2 offset(0,0);
			float ffrac=0.75;
			Vec3 lin(0,0,1);
			Vec3 offin=lin-dot(lin, norm3)*norm3;
			Vec3 offout=offin*ffrac;
			float frac=sqrt(1-dot(offout,offout));
			Vec3 refrac=frac*norm3+offout;
			offset=Vec2(refrac.x,refrac.y)/refrac.z;//*h;

			int ni=i+offset.x*scale*10;
			int nj=j+offset.y*scale*10;
			if(ni<0)ni=0;
			else if(ni>=dest->height)ni=dest->height-1;
			if(nj<0)nj=0;
			else if(nj>=dest->width)nj=dest->width-1;

			uchar* pixs = PTR_PIX(*src, nj, ni);
			uchar* pix = PTR_PIX(*dest, j, i);
			//float alpha = *(float*)PTR_PIX(*noise, j, i);
			Vec3 v(*pixs,*(pixs+1),*(pixs+2));
			Vec3 vd = v*l+Vec3(spec,spec,spec)*70;
			*pix = max(0,min(255,vd.x));
			*(pix+1) = max(0,min(255,vd.y));
			*(pix+2) = max(0,min(255,vd.z));
		}
	}
	cvReleaseImage(&wave);
}

void PixelFunc3(RawImage* target, Point2D pos, int count, float* pdata, void* p)
{
	IplImage* imgsrc = ((C3DProcData*)p)->src;
	IplImage* imgtex = ((C3DProcData*)p)->tex;
	ASSERT(imgtex != NULL && imgtex->depth == IPL_DEPTH_32F && imgtex->nChannels == 3);
	ASSERT(imgsrc != NULL);
	ASSERT(count == 2);
	Vec3 veye((float)pos.x/target->width*2-1, -((float)pos.y/target->height*2-1), 1);
	veye = veye.normalize();
	Vec2* tex = (Vec2*)pdata;
	Vec3 vt = Sample(imgtex, *tex);
	Vec3 n(vt.x, vt.z, vt.y);
	float ffrac=0.9;
	float cosa = dot(veye, n);
	Vec3 offin=veye-cosa*n;
	cosa=fabs(cosa);
	Vec3 offout=offin/ffrac;
	float l2 = dot(offout,offout);
	float l=pow(fabs(dot(veye,n)), 100);
	float portion;
	Vec3 refrac;
	Vec3 reflect = 2*offin-veye;
	Vec3 crefrac(0,0,0);
	if(l2<1)//refraction
	{
		float frac=sqrt(1-l2);
		refrac=frac*n+offout;
		portion = powf((cosa-ffrac*frac)/(cosa+ffrac*frac),2);//Snell's Law
		if(refrac.z!=0)
			refrac/=refrac.z;
		Vec2 texs((refrac.x+1)/2, (-refrac.y+1)/2);
		crefrac = Sample(imgsrc, texs);
	}
	else
	{
		portion = 1;
	}
	if(reflect.z!=0)
		reflect/=reflect.z;
	Vec2 texs((reflect.x+1)/2, (-reflect.y+1)/2);
	Vec3 creflec = Sample(imgsrc, texs);
	Vec3 b(120,55,0);
	Vec3 cd = creflec*portion+crefrac*(1-portion)+l*Vec3(50,50,50);
	cd = b+(cd-b)*powf(0.2,tex->y);
	uchar* pix = PTR_PIX(*target, pos.x, pos.y);
	*pix = max(0,min(255,cd.x));
	*(pix+1) = max(0,min(255,cd.y));
	*(pix+2) = max(0,min(255,cd.z));
}
void VertexShader(Vec3* posin, Vec3* colorin, Vec3* normalin, Vec2* texin, float* vuserdatain, Vec3* posout, Vec3* colorout, Vec2* texout, float* vuserdataout, void* usrptr)
{
	Vec3 tl = normalize(Vec3(-0.2,-0.2,1));
	Vec3 tv = *posin*((C3DProcData*)usrptr)->radius+((C3DProcData*)usrptr)->pos;
	Vec3 tn = *normalin;
	*posout = Vec3(tv.x/tv.z,tv.y/tv.z,1/tv.z);
	tv = tv.normalize();
	float d=dot(tn,tv);
	float l=fabs(dot(tn,tl));
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

	*vuserdataout = l;
	texout->x=x;
	texout->y=y;
}
void PixelShader(Point2D pos, Vec3* color, Vec2* tex, float* userdata, Vec3* colorout, float* depth, void* usrptr)
{
	IplImage* imgsrc = ((C3DProcData*)usrptr)->src;
	Vec3 c = Sample(imgsrc, *tex);
	float l=*userdata;
	uchar* pixs = PTR_PIX(*imgsrc, pos.x, pos.y);
	Vec3 cs(*pixs, *(pixs+1), *(pixs+2));
	Vec3 cd = c/255*l+powf(l,100)*Vec3(1,1,1);
	cd = 0.8*cd+0.2*cs/255;
	Vec3 b(120,55,0);
	b/=255;
	*colorout = b+(cd-b)*powf(0.5, ((C3DProcData*)usrptr)->pos.z-1);
}
void CChildView::UnderWater(IplImage* src, IplImage* dest)
{
	Vec3 b(120,55,0);
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			uchar* pixs = PTR_PIX(*src, j, i);
			uchar* pix = PTR_PIX(*dest, j, i);
			Vec3 v(*pixs,*(pixs+1),*(pixs+2));
			Vec3 vd = (v-b)*0.2+b;
			*pix = vd.x;
			*(pix+1) = vd.y;
			*(pix+2) = vd.z;
		}
	}
	float breadth = 4;
	float scalex = 10;
	float scaley = 10;
	float scale = min(scalex,scaley);
	int wmap = min(600, src->width*(1+breadth));
	int hmap = wmap*breadth/(1+breadth)/2;
	IplImage* watermap = cvCreateImage(cvSize(wmap,hmap), IPL_DEPTH_32F, 1);
	IplImage* noise = cvCreateImage(cvSize(wmap,hmap), IPL_DEPTH_32F, 1);
	GenWaterMap2(noise,scalex,scaley);
	ComputeShiftImg(noise, watermap, noise, scale);
	cvReleaseImage(&noise);
	IplImage* tan_map = cvCreateImage(cvGetSize(watermap), IPL_DEPTH_32F, 3);
	for(int i=0;i<tan_map->height;i++)
	{
		for(int j=0;j<tan_map->width;j++)
		{
			float h=*(float*)PTR_PIX(*watermap, j, i);
			float hx=*(float*)PTR_PIX(*watermap, min(j+1,watermap->width-1),i);
			float hy=*(float*)PTR_PIX(*watermap, j, min(i+1,watermap->height-1));

			Vec2  norm(h-hx,h-hy);
			Vec3 norm3(norm.x,norm.y,2./scale);
			norm3=norm3.normalize();

			*(Vec3*)PTR_PIX(*tan_map, j, i) = norm3;
		}
	}
	cvReleaseImage(&watermap);

	Vec3 vert[4] = {Vec3(-(1+breadth),1,0),Vec3(-(1+breadth),1,breadth),Vec3(1+breadth,1,0),Vec3(1+breadth,1,breadth)};
	Vec2 texcoord[4] = {Vec2(0,0),Vec2(0,1),Vec2(1,0),Vec2(1,1)};
	int index[2][3] = {{0,1,3},{3,2,0}};
	C3DProcData data;
	data.tex = tan_map;
	data.src = src;
	Vec3 pos(0,0,-1);
	for(int i=0;i<4;i++)
	{
		vert[i].x*=(float)dest->width/dest->height;
		vert[i]-=pos;
		float z = vert[i].z;
		vert[i]/=z;
		vert[i].z=1./z;
	}
	for(int i=0;i<2;i++)
	{
		Vec3 v[3];
		float tex[2][3];
		for(int j=0;j<3;j++)
		{
			v[j] = vert[index[i][j]];
			tex[0][j] = texcoord[index[i][j]].x;
			tex[1][j] = texcoord[index[i][j]].y;
		}
		Triangle3D(dest, v, 2, tex, PixelFunc3, &data);
	}
	cvReleaseImage(&tan_map);

	//phase 2:draw bubbles
	CMesh mesh;
	CreateSphere(&mesh, 1, 40);
	IplImage* depth = cvCreateImage(cvGetSize(dest), IPL_DEPTH_32F, 1);
	cvZero(depth);
	mesh.SetVSOutFormat(VFT_XYZ|VFT_TEXCOORD|VFT_USER(1));
	mesh.SetVertexShader(VertexShader);
	mesh.SetPixelShader(PixelShader);

	float rmin = 0.5,rmax = 0.7;
	mesh.SetUserParam(&data);
	for(int i=0;i<10;i++)
	{
		data.pos.x = ((float)rand()/RAND_MAX*2-1)*(1+breadth/2);
		data.pos.z = (0.5+(1-0.5)*(float)rand()/RAND_MAX)*breadth;
		float height = (float)rand()/RAND_MAX;
		data.pos.y = 1+(1-height)*((-1-breadth/2)-1);
		data.radius = rmin+(float)rand()/RAND_MAX*(rmax-rmin);
		data.radius*=height;

		if(data.pos.x>data.pos.z+1)
		{
			Vec3 tmp = data.pos;
			data.pos.x = 1+tmp.z;
			data.pos.z = tmp.x-1;
		}
		else if(data.pos.x<-1-data.pos.z)
		{
			Vec3 tmp = data.pos;
			data.pos.x = -(1+tmp.z);
			data.pos.z = -tmp.x-1;
		}
		if(data.pos.z<max(data.radius+0.1,breadth*0.5))
			data.pos.z = max(data.radius+0.1,breadth*0.5);
		data.pos -= pos;
		mesh.Render(dest, &(RawImage)depth);
	}
	cvReleaseImage(&depth);

	//phase 3:draw twilight
	IplImage* tltex = cvCreateImage(cvSize(dest->width,1), IPL_DEPTH_32F, 1);
	Perlin1D(tltex, 15);
	*(float*)PTR_PIX(*tltex,0,0) = 0;
	*(float*)PTR_PIX(*tltex,tltex->width-1,0) = 0;
	Vec2 tltan(10,1);
	tltan = tltan.normalize();
	float smax = 2;
	float smin = 0.01;
	float th = 0.7;
	float lmax = 0.5;
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			Vec2 p((float)j/dest->width, (float)i/dest->height);
			float t = dot(p,tltan)/dot(Vec2(1,1),tltan);
			Vec3 ct = Sample(tltex, Vec2(t,0));
			float s = (SmoothSlopeB(ct.x, th, 0.2)-th)/(1-th)*(1-smin)+smin;
			float l = lmax*exp(-(float)i/dest->height/(s*smax));
			uchar* pix = PTR_PIX(*dest, j, i);
			Vec3 c(*pix, *(pix+1), *(pix+2));
			Vec3 cd = Vec3(255,255,255)*l+c*(1-l);
			*pix = cd.x;
			*(pix+1) = cd.y;
			*(pix+2) = cd.z;
		}
	}
	cvReleaseImage(&tltex);
}

void PixelFunc4(RawImage* target, Point2D pos, int count, float* pdata, void* p)
{
	IplImage* imgsrc = ((C3DProcData*)p)->src;
	IplImage* imgtex = ((C3DProcData*)p)->tex;
	ASSERT(imgtex != NULL && imgtex->depth == IPL_DEPTH_32F && imgtex->nChannels == 3);
	ASSERT(imgsrc != NULL);
	ASSERT(count == 2);
	Vec3 veye((float)pos.x/target->width*2-1, -((float)pos.y/target->height*2-1), 1);
	veye = veye.normalize();
	Vec2* tex = (Vec2*)pdata;
	Vec3 vt = Sample(imgtex, *tex);
	Vec3 n(vt.x, vt.z, vt.y);
	float ffrac=1.1;
	Vec3 offin=veye-dot(veye, n)*n;
	Vec3 offout=offin/ffrac;
	float l2 = dot(offout,offout);
	float l=pow(fabs(dot(veye,n)), 100);
	float frac=sqrt(1-l2);
	Vec3 refrac=frac*n+offout;
	if(refrac.z!=0)
		refrac/=refrac.z;
	Vec2 texs((refrac.x+1)/2, (-refrac.y+1)/2);
	Vec3 crefrac = Sample(imgsrc, texs);
	Vec3 cd = crefrac+l*Vec3(50,50,50);
	uchar* pix = PTR_PIX(*target, pos.x, pos.y);
	*pix = max(0,min(255,cd.x));
	*(pix+1) = max(0,min(255,cd.y));
	*(pix+2) = max(0,min(255,cd.z));
}

void CChildView::AboveWater(IplImage* src, IplImage* dest)
{
	cvCopyImage(src, dest);
	float breadth = 4;
	float scalex = 10;
	float scaley = 10;
	float scale = min(scalex,scaley);
	int wmap = min(600, src->width*(1+breadth));
	int hmap = wmap*breadth/(1+breadth)/2;
	IplImage* watermap = cvCreateImage(cvSize(wmap,hmap), IPL_DEPTH_32F, 1);
	IplImage* noise = cvCreateImage(cvSize(wmap,hmap), IPL_DEPTH_32F, 1);
	GenWaterMap2(noise,scalex,scaley);
	ComputeShiftImg(noise, watermap, noise, scale);
	cvReleaseImage(&noise);
	IplImage* tan_map = cvCreateImage(cvGetSize(watermap), IPL_DEPTH_32F, 3);
	for(int i=0;i<tan_map->height;i++)
	{
		for(int j=0;j<tan_map->width;j++)
		{
			float h=*(float*)PTR_PIX(*watermap, j, i);
			float hx=*(float*)PTR_PIX(*watermap, min(j+1,watermap->width-1),i);
			float hy=*(float*)PTR_PIX(*watermap, j, min(i+1,watermap->height-1));

			Vec2  norm(h-hx,h-hy);
			Vec3 norm3(norm.x,norm.y,2./scale);
			norm3=norm3.normalize();

			*(Vec3*)PTR_PIX(*tan_map, j, i) = norm3;
		}
	}
	cvReleaseImage(&watermap);

	Vec3 vert[4] = {Vec3(-(1+breadth),-1,0),Vec3(-(1+breadth),-1,breadth),Vec3(1+breadth,-1,0),Vec3(1+breadth,-1,breadth)};
	Vec2 texcoord[4] = {Vec2(0,0),Vec2(0,1),Vec2(1,0),Vec2(1,1)};
	int index[2][3] = {{0,1,3},{3,2,0}};
	C3DProcData data;
	data.tex = tan_map;
	data.src = src;
	Vec3 pos(0,0,-1);
	for(int i=0;i<4;i++)
	{
		vert[i].x*=(float)dest->width/dest->height;
		vert[i]-=pos;
		float z = vert[i].z;
		vert[i]/=z;
		vert[i].z=1./z;
	}
	for(int i=0;i<2;i++)
	{
		Vec3 v[3];
		float tex[2][3];
		for(int j=0;j<3;j++)
		{
			v[j] = vert[index[i][j]];
			tex[0][j] = texcoord[index[i][j]].x;
			tex[1][j] = texcoord[index[i][j]].y;
		}
		Triangle3D(dest, v, 2, tex, PixelFunc4, &data);
	}
	cvReleaseImage(&tan_map);
}
//rain
void CChildView::Process10(IplImage* src, IplImage* dest)
{
	float t = (float)rand()/RAND_MAX;
	float lvl = 0;
	float mt[4] = {10,-150,20,-220};
	Vec2 mp[4] = {Vec2(12.3,-17.0),Vec2(-22.2,22.2),Vec2(-15.7,-5.3),Vec2(13.2,24.5)};
	float scale = 0.7;
	for(int i=0;i<4;i++)
	{
		mp[i]/=scale;
	}
	Vec3 ld(0,0,1);
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			uchar* pixs = PTR_PIX(*src, j, i);
			Vec3 cs(*pixs,*(pixs+1),*(pixs+2));
			Vec2 tex((float)j/src->width,(float)i/src->height);
			Vec2 p=tex*Vec2(1.5,1);
			float s = 0;
			Vec2 ds(0,0);
			for(int k=0;k<4;k++)
			{
				float sigma=dot(p,mp[k])+t*mt[k];
				s+=sin(sigma);
				ds+=mp[k]*cos(sigma);
			}
			Vec3 cd;
			if(s>lvl)
			{
				Vec2 final=ds/2/sqrt(s-lvl);
				Vec3 n=normalize(Vec3(final.x,final.y,1));
				Vec2 shift=Vec2(n.x,n.y)*n.z*0.01;
				Vec3 cs2=Sample(src, tex-shift);
				float cl=dot(n,ld);
				Vec3 w(255,255,255);
				cd=w*pow(cl,10)+(w-cs2)*cl+cs2;
			}
			else
				cd=cs;
			uchar* pix = PTR_PIX(*dest, j, i);
			*pix = max(0,min(255,cd.x));
			*(pix+1) = max(0,min(255,cd.y));
			*(pix+2) = max(0,min(255,cd.z));
		}
	}
}
//rain2
void CChildView::Process11(IplImage* src, IplImage* dest)
{
	float t = (float)rand()/RAND_MAX;
	float lvl = -1;
	float mt[4] = {100,-150,200,-220};
	Vec2 mp[4] = {Vec2(12.3,-17.0),Vec2(-22.2,22.2),Vec2(-15.7,-5.3),Vec2(0,13.2)};
	float scale = 0.7;
	for(int i=0;i<4;i++)
	{
		mp[i]/=scale;
	}
	Vec3 ld(0,0,1);
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			uchar* pixs = PTR_PIX(*src, j, i);
			Vec3 cs(*pixs,*(pixs+1),*(pixs+2));
			Vec2 tex((float)j/src->width,(float)i/src->height);
			Vec2 p=tex*Vec2(-2.5,2.5);
			float s = 0;
			Vec2 ds(0,0);
			for(int k=0;k<4;k++)
			{
				float sigma=dot(p,mp[k])+t*mt[k];
				s+=sin(sigma);
				ds+=mp[k]*cos(sigma);
			}
			Vec3 cd;
			if(s>lvl)
			{
				Vec2 final=ds/2/sqrt(s-lvl);
				Vec3 n=normalize(Vec3(final.x,final.y,1));
				Vec2 shift=Vec2(n.x,n.y)*n.z*0.01;
				Vec3 cs2=Sample(src, tex-shift);
				float cl=dot(n,ld);
				Vec3 w(255,255,255);
				cd=w*pow(cl,10)+(w-cs2)*(3*cl)+cs2;
			}
			else
				cd=cs;
			uchar* pix = PTR_PIX(*dest, j, i);
			*pix = max(0,min(255,cd.x));
			*(pix+1) = max(0,min(255,cd.y));
			*(pix+2) = max(0,min(255,cd.z));
		}
	}
}

void PixelFunc5(RawImage* target, Point2D pos, int count, float* pdata, void* p)
{
	IplImage* imgsrc = ((C3DProcData*)p)->src;
	IplImage* imgtex = ((C3DProcData*)p)->tex;
	IplImage* imgmask = ((C3DProcData*)p)->mask;
	ASSERT(count == 4);
	ASSERT(imgsrc != NULL);
	ASSERT(imgtex != NULL && imgtex->nChannels == 4);
	ASSERT(imgmask != NULL);
	Vec2* tex = (Vec2*)pdata;
	Vec2* tex2 = (Vec2*)(pdata+2);
	uchar* pix = PTR_PIX(*target, pos.x, pos.y);
	Vec3 cs(*pix, *(pix+1), *(pix+2));
	Vec3 m = Sample(imgmask, *tex2);
	float s = (m.x+m.y+m.z)/3/255;
	Vec3 c1 = Sample(imgsrc, *tex, WRAP_TYPE_CLAMP, TEX_FILTER_LINEAR, false);
	Vec4 ct = SampleEx(imgtex, *tex2);
	Vec3 c2 = Vec3(ct.x,ct.y,ct.z)*ct.t/255+cs*(1-ct.t/255);
	Vec3 c = c1*s+c2*(1-s);
	*pix = c.x;
	*(pix+1) = c.y;
	*(pix+2) = c.z;
}
//Collage
void CChildView::Process12(IplImage* src, IplImage* dest)
{
	cvSet(dest, cvScalarAll(128));
	int n = 20;
	float scale = min(src->width, src->height);
	float l = scale/n;
	int nx = (int)floor(src->width/l+0.75);
	int ny = (int)floor(src->height/l+0.75);

	int index[2][3] = {{0,1,3},{3,2,0}};
	int* buf = new int[nx*ny];
	int rst = nx*ny;
	for(int i=0;i<nx*ny;i++)
	{
		buf[i] = i;
	}
	C3DProcData data;
	data.src = src;
	data.tex = m_frame;
	data.mask = m_mask;
	while(rst>0)
	{
		int ibuf = (float)rand()/(RAND_MAX+1)*rst;
		int isel = buf[ibuf];
		if(ibuf != rst-1)
			buf[ibuf] = buf[rst-1];
		rst--;

		int i = isel/nx;
		int j = isel%nx;
		Vec2 vert[4] = {Vec2(-0.55*l,-0.55*l), Vec2(0.6*l,-0.55*l), Vec2(-0.55*l,0.6*l), Vec2(0.6*l,0.6*l)};
		Vec2 texcoord[4] = {Vec2((j-0.05)*l,(i-0.05)*l), Vec2((j+1.1)*l,(i-0.05)*l), Vec2((j-0.05)*l,(i+1.1)*l), Vec2((j+1.1)*l,(i+1.1)*l)};
		Vec2 texcoord2[4] = {Vec2(0,0), Vec2(1,0), Vec2(0,1), Vec2(1,1)};
		Vec2 c((j+0.5)*l,(i+0.5)*l);
		Vec2 vx = Vec2(1,0)+RandVec()*0.4;
		vx = vx.normalize();
		Vec2 vy(-vx.y,vx.x);
		Vec2 vs = RandVec()*0.2*l;
		for(int k=0;k<4;k++)
		{
			vert[k] = Vec2(dot(vert[k],vx),dot(vert[k],vy))+vs+c;
		}
		for(int k=0;k<2;k++)
		{
			Vec2 tvert[3];
			float tex[4][3];
			for(int l=0;l<3;l++)
			{
				tvert[l] = vert[index[k][l]];
				tex[0][l] = texcoord[index[k][l]].x;
				tex[1][l] = texcoord[index[k][l]].y;
				tex[2][l] = texcoord2[index[k][l]].x;
				tex[3][l] = texcoord2[index[k][l]].y;
			}
			Triangle2D(dest, tvert, 4, tex, PixelFunc5, &data);
		}
	}
	delete[] buf;
}

void CChildView::Process13(IplImage* src, IplImage* dest)
{
	cvSet(dest, cvScalarAll(128));
	int n = 20;
	float scale = min(src->width, src->height);
	float l = scale/n;
	int nx = (int)floor(src->width/l*2+0.75)+1;
	int ny = (int)floor(src->height/l+0.75);

	int index[2][3] = {{0,1,3},{3,2,0}};
	int* buf = new int[nx*ny];
	int rst = nx*ny;
	for(int i=0;i<nx*ny;i++)
	{
		buf[i] = i;
	}
	C3DProcData data;
	data.src = src;

	while(rst>0)
	{
		int ibuf = (float)rand()/(RAND_MAX+1)*rst;
		int isel = buf[ibuf];
		if(ibuf != rst-1)
			buf[ibuf] = buf[rst-1];
		rst--;

		int i = isel/nx;
		int j = isel%nx;

		Vec2 vert[4] = {Vec2(-0.55*l,-0.55*l), Vec2(0.55*l,-0.55*l), Vec2(-0.55*l,0.55*l), Vec2(0.55*l,0.55*l)};
		Vec2 texcoord[4] = {Vec2((j-0.1)*l/2,(i-0.05)*l), Vec2((j+2.1)*l/2,(i-0.05)*l), Vec2((j-0.1)*l/2,(i+1.1)*l), Vec2((j+2.1)*l/2,(i+1.1)*l)};
		Vec2 texcoord2[4] = {Vec2(0,0), Vec2(1,0), Vec2(0,1), Vec2(1,1)};
		Vec2 c(j*l/2,(i+0.5)*l);
		Vec2 vx = Vec2(1,0)+RandVec()*0.2;
		vx = vx.normalize();
		Vec2 vy(-vx.y,vx.x);
		Vec2 vs = RandVec()*0.2*l;
		for(int k=0;k<4;k++)
		{
			vert[k] = Vec2(dot(vert[k],vx),dot(vert[k],vy))+vs+c;
		}
		if((i%2==0&&j%2==0)||(i%2==1&&j%2==1))
		{
			data.tex = m_tri;
			data.mask = m_trimask;
		}
		else
		{
			data.tex = m_tri2;
			data.mask = m_trimask2;
		}

		for(int k=0;k<2;k++)
		{
			Vec2 tvert[3];
			float tex[4][3];
			for(int l=0;l<3;l++)
			{
				tvert[l] = vert[index[k][l]];
				tex[0][l] = texcoord[index[k][l]].x;
				tex[1][l] = texcoord[index[k][l]].y;
				tex[2][l] = texcoord2[index[k][l]].x;
				tex[3][l] = texcoord2[index[k][l]].y;
			}
			Triangle2D(dest, tvert, 4, tex, PixelFunc5, &data);
		}
	}
	delete[] buf;
}

void CChildView::Process14(IplImage* src, IplImage* dest)
{
	cvSet(dest, cvScalarAll(128));
	int n = 20;
	float scale = min(src->width, src->height);
	float l = scale/n;
	int nx = (int)floor(src->width/l+0.75)+1;
	int ny = (int)floor((src->height/l)*4/3+0.75);

	int index[2][3] = {{0,1,3},{3,2,0}};
	int* buf = new int[nx*ny];
	int rst = nx*ny;
	for(int i=0;i<nx*ny;i++)
	{
		buf[i] = i;
	}
	C3DProcData data;
	data.src = src;
	data.tex = m_hex;
	data.mask = m_hexmask;

	while(rst>0)
	{
		int ibuf = (float)rand()/(RAND_MAX+1)*rst;
		int isel = buf[ibuf];
		if(ibuf != rst-1)
			buf[ibuf] = buf[rst-1];
		rst--;

		int i = isel/nx;
		int j = isel%nx;

		Vec2 vert[4] = {Vec2(-0.55*l,-0.55*l), Vec2(0.55*l,-0.55*l), Vec2(-0.55*l,0.55*l), Vec2(0.55*l,0.55*l)};
		Vec2 texcoord[4] = {Vec2((j-0.05)*l,i*l*0.75-0.05*l), Vec2((j+1.05)*l,i*l*0.75-0.05*l), Vec2((j-0.05)*l,i*l*0.75+1.05*l), Vec2((j+1.05)*l,i*l*0.75+1.05*l)};
		Vec2 texcoord2[4] = {Vec2(0,0), Vec2(1,0), Vec2(0,1), Vec2(1,1)};
		Vec2 c((j+0.5)*l,i*l*0.75+0.5*l);
		if(i%2==1)
		{
			for(int i=0;i<4;i++)
				texcoord[i].x-=l*0.5;
			c.x-=l*0.5;
		}
		Vec2 vx = Vec2(1,0)+RandVec()*0.3;
		vx = vx.normalize();
		Vec2 vy(-vx.y,vx.x);
		Vec2 vs = RandVec()*0.2*l;
		for(int k=0;k<4;k++)
		{
			vert[k] = Vec2(dot(vert[k],vx),dot(vert[k],vy))+vs+c;
		}
		for(int k=0;k<2;k++)
		{
			Vec2 tvert[3];
			float tex[4][3];
			for(int l=0;l<3;l++)
			{
				tvert[l] = vert[index[k][l]];
				tex[0][l] = texcoord[index[k][l]].x;
				tex[1][l] = texcoord[index[k][l]].y;
				tex[2][l] = texcoord2[index[k][l]].x;
				tex[3][l] = texcoord2[index[k][l]].y;
			}
			Triangle2D(dest, tvert, 4, tex, PixelFunc5, &data);
		}
	}
	delete[] buf;
}

Vec3 RGB2HSV(Vec3 c)
{
	static float sqr2 = sqrtf(2);
	static float sqr3 = sqrtf(3);
	float y = sqr3*(c.y-c.z), x=2*c.x-c.y-c.z;
	float h = atan2f(y, x);
	if(y==0&&x==0)
		h=0;
	if(h<0)
		h+=2*CV_PI;
	float s = 1.0f/sqr3*sqrt(powf(c.y-c.z,2)+powf(c.z-c.x,2)+powf(c.x-c.y,2));
	float v = (c.x+c.y+c.z)/sqr3;
	return Vec3(h,s,v);
}

Vec3 HSV2RGB(Vec3 c)
{
	static float sqr2 = sqrtf(2);
	static float sqr3 = sqrtf(3);
	Vec3 l=Vec3(1,1,1)/sqr3;
	Vec3 r=Vec3(2.f/3.f, -1.f/3.f, -1.f/3.f)*sqr3/sqr2;;
	Vec3 g(0,1.f/sqr2,-1.f/sqr2);
	Vec3 v = l*c.z+c.y*(r*cosf(c.x)+g*sinf(c.x));
	return v;
}

float Ladder(float x, float w, float h, float p)
{
	float s = x/w;
	float n = (int)floorf(s);
	float fr = s-n;
	float f;
	if(fr<0.5)
		f=h/2*powf(fr*2,p);
	else
		f=h-h/2*powf((1-fr)*2,p);
	return f+n*h;
}

void CChildView::Process15(IplImage* src, IplImage* dest)
{
	float sqr2 = sqrtf(2);
	float sqr3 = sqrtf(3);
	IplImage* fimg = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 3);
	CvMat* kernel = cvCreateMat(21,21,CV_32FC1);
	CvMat* kernel2 = cvCreateMat(5,5,CV_32FC1);
	cvSet(kernel, cvScalar(1.f/21/21));
	cvSet(kernel2, cvScalar(1.f/5/5));
	cvCvtScale(src, fimg);

	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			float* pixs = (float*)PTR_PIX(*fimg, j, i);
			Vec3 vs(*pixs,*(pixs+1),*(pixs+2));
			vs/=255;
			Vec3 hsv = RGB2HSV(vs);
			*(Vec3*)pixs = hsv;
		}
	}
	IplImage* chl = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	cvSplit(fimg, NULL, chl, NULL, NULL);
	cvFilter2D(chl,chl,kernel,cvPoint(10,10));
	cvMerge(NULL, chl, NULL, NULL, fimg);
	cvSplit(fimg, chl, NULL, NULL, NULL);
	cvFilter2D(chl,chl,kernel2,cvPoint(2,2));
	cvMerge(chl, NULL, NULL, NULL, fimg);
	cvReleaseImage(&chl);
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			float* pixs = (float*)PTR_PIX(*fimg, j, i);
			Vec3 hsv(*pixs,*(pixs+1),*(pixs+2));
			Vec3 hsv2;
			hsv2.x = Ladder(hsv.x, CV_PI/3, CV_PI/3, 6);//hsv.x-sinf(hsv.x*6)/6;
			if(hsv.y>sqr2/sqr3)
				hsv2.y = sqr2/sqr3*0.7;
			else
				hsv2.y = (1-powf(1-hsv.y/sqr2*sqr3, 4))*sqr2/sqr3*0.7;
			//hsv2.y = Ladder(hsv.y-sqr2/sqr3, sqr2/sqr3*2, sqr2/sqr3*2, 3)+sqr2/sqr3;//hsv.y+sinf(hsv.y*sqr2*sqr3*CV_PI/2)/sqr2/sqr3/CV_PI*2;
			hsv2.z = hsv.z-sqr3/2/CV_PI*sinf(hsv.z/sqr3*2*CV_PI);

			Vec3 vd = HSV2RGB(hsv2);
			vd*=255;
			uchar* pix = PTR_PIX(*dest, j, i);
			*pix = max(0,min(255,vd.x));
			*(pix+1) = max(0,min(255,vd.y));
			*(pix+2) = max(0,min(255,vd.z));
		}
	}
	cvReleaseMat(&kernel);
	cvReleaseMat(&kernel2);
	cvReleaseImage(&fimg);
}

#define NUM_PROC 20
void CChildView::ProcessAll()
{
	switch(m_nIndex)
	{
	case 0:
		if(m_src)
		{
			Process(m_src, m_dest);
		}
		break;
	case 1:
		if(m_src)
		{
			Rainbow(m_src, m_dest);
		}
		break;
	case 2:
		if(m_src)
		{
			Moon(m_src, m_dest);
		}
		break;
	case 3:
		if(m_src)
		{
			Process2(m_src, m_dest);
		}
		break;
	case 4:
		if(m_src)
		{
			Process3(m_src, m_dest);
		}
		break;
	case 5:
		if(m_src)
		{
			Process4(m_src, m_dest);
		}
		break;
	case 6:
		if(m_src)
		{
			Process5(m_src, m_dest);
		}
		break;
	case 7:
		if(m_src)
		{
			Process6(m_src, m_dest);
		}
		break;
	case 8:
		if(m_src)
		{
			Process7(m_src, m_dest);
		}
		break;
	case 9:
		if(m_src)
		{
			Process8(m_src, m_dest);
		}
		break;
	case 10:
		if(m_src)
		{
			Process9(m_src, m_dest, 20, 20);
		}
		break;
	case 11:
		if(m_src)
		{
			UnderWater(m_src, m_dest);
		}
		break;
	case 12:
		if(m_src)
		{
			AboveWater(m_src, m_dest);
		}
		break;
	case 13:
		if(m_src)
		{
			Process10(m_src, m_dest);
		}
		break;
	case 14:
		if(m_src)
		{
			Process11(m_src, m_dest);
		}
		break;
	case 15:
		if(m_src)
		{
			Process12(m_src, m_dest);
		}
		break;
	case 16:
		if(m_src)
		{
			Process13(m_src, m_dest);
		}
		break;
	case 17:
		if(m_src)
		{
			Process14(m_src, m_dest);
		}
		break;
	case 18:
		if(m_src)
		{
			Paint2(m_src, m_dest);
		}
		break;
	case 19:
		if(m_src)
		{
			Process15(m_src, m_dest);
		}
		break;
	}
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	switch(nChar)
	{
	case VK_LEFT:
		m_nIndex--;
		if(m_nIndex<0)
			m_nIndex = NUM_PROC-1;
		ProcessAll();
		Draw();
		break;
	case VK_RIGHT:
		m_nIndex++;
		if(m_nIndex>=NUM_PROC)
			m_nIndex = 0;
		ProcessAll();
		Draw();
		break;
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
