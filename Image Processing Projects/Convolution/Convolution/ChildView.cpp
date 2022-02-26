
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "Convolution.h"
#include "ChildView.h"
#include "ImageProcess3D.h"
#include "ImageProcess2D.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#endif

#include "Gdiplus.h"
using namespace Gdiplus;

// CChildView

CChildView::CChildView()
{
	m_src = NULL;
	m_dest = NULL;
	m_tex = NULL;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_OPEN2, &CChildView::OnFileOpen2)
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
	CRect rcClient;
	GetClientRect(rcClient);
	CDC mdc;
	mdc.CreateCompatibleDC(&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
	CBitmap* oldbmp = mdc.SelectObject(&bmp);
	mdc.FillSolidRect(rcClient, RGB(255,255,255));
	//cvSet(m_dest, cvScalarAll(255));
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

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_dest = cvCreateImage(cvSize(800, 800), IPL_DEPTH_8U, 3);
	cvSet(m_dest, cvScalarAll(255));
	m_tex = loadWithGdiPlus(L"stroke.png");
	ASSERT(m_tex->nChannels == 4);
	Process();
	Draw();	

	return 0;
}


void CChildView::OnDestroy()
{
	CWnd::OnDestroy();

	// TODO: Add your message handler code here
	cvReleaseImage(&m_src);
	cvReleaseImage(&m_dest);
	cvReleaseImage(&m_tex);
	RawImage::ReleaseInternalHdr();
}

void CChildView::Process()
{
	IplImage* img = cvCreateImage(cvGetSize(m_dest), IPL_DEPTH_32F, 2);
	for(int i=0;i<img->height;i++)
	{
		for(int j=0;j<img->width;j++)
		{
			int m = i;
			int n = j;
			if(m>img->height/2)m-=img->height;
			if(n>img->width/2)n-=img->width;
			float x = (float)n/img->width;
			float y = (float)m/img->height;
			x*=5;
			y*=5;
			float val = x/(x*x+y*y);
			if(x*x+y*y==0)
			{
				val = 0;
			}
			float* pix = (float*)PTR_PIX(*img, j, i);
			*pix = val;
			*(pix+1) = 0;
		}
	}
	IplImage* img2 = cvCreateImage(cvGetSize(m_dest), IPL_DEPTH_32F, 2);
	cvDFT(img, img2, CV_DXT_FORWARD);
	for(int i=0;i<img->height;i++)
	{
		for(int j=0;j<img->width;j++)
		{
			uchar* pix = PTR_PIX(*m_dest, j, i);
			float* pixs = (float*)PTR_PIX(*img2, j, i);
			*pix = max(0,min(255,*pixs/800*255));
			*(pix+1) = max(0,min(255,-*(pixs+1)/800*255));
			*(pix+2) = max(0,min(255,*(pixs+1)/800*255));
		}
	}
	cvReleaseImage(&img);
	cvReleaseImage(&img2);
}

void CChildView::Paint(IplImage* src, IplImage* dest)
{
	int KernelSize = 20;
	int nFilterScale = 10;

	IplImage* tmp;
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
	tmp = gray;
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

	IplImage* conv[2] = {NULL, NULL};

	for(int n=0;n<2;n++)
	{
		IplImage* imgfourierx = cvCreateImage(cvGetSize(spectrum), IPL_DEPTH_32F, 1);
		IplImage* imgfouriery = cvCreateImage(cvGetSize(spectrum), IPL_DEPTH_32F, 1);

		if(n==0)
		{
			cvSplit(spectrum, imgfourierx, NULL, NULL, NULL);	
			cvSplit(spectrum, NULL, imgfouriery, NULL, NULL);
		}
		else
		{
			cvSplit(spectrum, NULL, NULL, imgfourierx, NULL);	
			cvSplit(spectrum, NULL, NULL, NULL, imgfouriery);
		}

		tmp = imgfourierx;
		imgfourierx = cvCreateImage(cvGetSize(spectrum), IPL_DEPTH_32F, 2);
		cvZero(imgfourierx);
		cvMerge(tmp, NULL, NULL, NULL, imgfourierx);
		cvReleaseImage(&tmp);
		tmp = imgfouriery;
		imgfouriery = cvCreateImage(cvGetSize(spectrum), IPL_DEPTH_32F, 2);
		cvZero(imgfouriery);
		cvMerge(tmp, NULL, NULL, NULL, imgfouriery);
		cvReleaseImage(&tmp);

		cvDFT(imgfourierx, imgfourierx, CV_DXT_FORWARD);
		cvDFT(imgfouriery, imgfouriery, CV_DXT_FORWARD);

		for(int i=0;i<spectrum->height;i++)
		{
			for(int j=0;j<spectrum->width;j++)
			{
				float* pixx = (float*)PTR_PIX(*imgfourierx, j, i);
				float* pixy = (float*)PTR_PIX(*imgfouriery, j, i);

				Vec2 vx(*pixx,*(pixx+1));
				Vec2 vy(*pixy,*(pixy+1));

				int m = i;
				int n = j;
				if(m>spectrum->height/2)m-=spectrum->height;
				if(n>spectrum->width/2)n-=spectrum->width;
				float x = (float)n/spectrum->width;
				float y = (float)m/spectrum->height;
				x*=5;
				y*=5;
				float valx = x/(x*x+y*y);
				float valy = y/(x*x+y*y);
				if(x*x+y*y==0)
				{
					valx = 0;
					valy = 0;
				}

				*pixx = vx.y*valx;
				*(pixx+1) = -vx.x*valx;
				*pixy = vy.y*valy;
				*(pixy+1) = -vy.x*valy;
			}
		}

		cvDFT(imgfourierx, imgfourierx, CV_DXT_INV_SCALE);
		cvDFT(imgfouriery, imgfouriery, CV_DXT_INV_SCALE);

		conv[n] = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
		tmp = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
		cvSplit(imgfourierx, conv[n], NULL, NULL, NULL);
		cvSplit(imgfouriery, tmp, NULL, NULL, NULL);
		cvAdd(conv[n], tmp, conv[n]);
		cvReleaseImage(&tmp);

		cvReleaseImage(&imgfourierx);
		cvReleaseImage(&imgfouriery);
	}
	cvReleaseImage(&spectrum);
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			float theta1 = *(float*)PTR_PIX(*conv[0], j, i)/16;//4
			float theta2 = *(float*)PTR_PIX(*conv[1], j, i)/16;//4
			Vec3 hue1(cos(theta1), cos(theta1+(2*CV_PI/3)), cos(theta1+(4*CV_PI/3)));
			Vec3 hue2(cos(theta2), cos(theta2+(2*CV_PI/3)), cos(theta2+(4*CV_PI/3)));
			//Vec3 hue(0,SmoothStep(cos(theta1), 0.8, 0.1),SmoothStep(cos(theta2), 0.8, 0.1));//=(hue1+hue2)*64+Vec3(128,128,128);
			//hue*=255;
			Vec4 hue = SampleEx(m_tex, Vec2(theta1, theta2), WRAP_TYPE_REPEAT);
			float alpha = hue.t/255;
			uchar* pix = PTR_PIX(*dest, j, i);
			*pix = max(0,min(255,hue.x*alpha));
			*(pix+1) = max(0,min(255,hue.y*alpha));
			*(pix+2) = max(0,min(255,hue.z*alpha));
		}
	}
	cvReleaseImage(&conv[0]);
	cvReleaseImage(&conv[1]);
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
			Paint(m_src, m_dest);
			Draw();
		}
	}
}
