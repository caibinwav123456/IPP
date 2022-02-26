
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

	for(int i=0;i<256;i++)
	{
		table[i]=(int)i-((int)i%64)*(1);
	}
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

/*	for(int i=0;i<m_Image->height;i++)
	{
		for(int j=0;j<m_Image->width;j++)
		{
			uchar* pix=PTR_PIX(*m_Image,j,i);
			uchar* spix=PTR_PIX(*m_ImgBk1,j,i);
			*pix=table[*spix];
			*(pix+1)=table[*(spix+1)];
			*(pix+2)=table[*(spix+2)];
		}
	}*/

	IplImage* hsv=cvCreateImage(cvGetSize(m_Image), IPL_DEPTH_8U, 3);
	IplImage* s=cvCreateImage(cvGetSize(m_Image), IPL_DEPTH_8U, 1);
	IplImage* h=cvCreateImage(cvGetSize(m_Image), IPL_DEPTH_8U, 1);
//	cvCvtColor(m_ImgBk1, hsv, CV_RGB2HSV);
/*	cvSplit(hsv, NULL, s, NULL, NULL);
	cvSplit(hsv, h, NULL, NULL, NULL);
	cvAndS(s, cvScalarAll(0xc0), s);
	cvAndS(h, cvScalarAll(0xc0), h);
	//cvSet(s, cvScalarAll(255));
	cvMerge(NULL, s, NULL, NULL, hsv);
	cvMerge(h, NULL, NULL, NULL, hsv);
*///cvCvtColor(hsv, m_Image, CV_HSV2RGB);
	Process(m_ImgBk1, m_Image);
	DispImage(&m_mdc, m_Image, CPoint(0,0));
	cvReleaseImage(&hsv);
	cvReleaseImage(&s);
	cvReleaseImage(&h);
					
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
	IplImage* image1 = cvLoadImage("peaceb.jpg");
	IplImage* image2 = cvLoadImage("peacew.jpg");

	float xscale = 1024./image1->width;
	float yscale = 768./image1->height;
	float scale = min(xscale, yscale);
	if(scale > 1)scale = 1;
	int wImage=image1->width * scale;
	int hImage=image1->height * scale;
	m_ImgBk1 = cvCreateImage(cvSize(wImage,hImage), IPL_DEPTH_8U, 3);
	m_ImgBk2 = cvCreateImage(cvSize(wImage,hImage), IPL_DEPTH_8U, 3);
	cvResize(image1, m_ImgBk1);
	cvResize(image2, m_ImgBk2);
	cvReleaseImage(&image1);
	cvReleaseImage(&image2);

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
#define FG
#ifdef FG
//不缓存，最小遇到（不平均），输出不缓存
void CChildView::Process(IplImage* src, IplImage* dest)
{
	cvZero(dest);
	IplImage* imgcnt = cvCreateImage(cvGetSize(dest),IPL_DEPTH_32S, 1);
	IplImage* imgacc = cvCreateImage(cvGetSize(dest),IPL_DEPTH_32S, 3);
	IplImage* imgc = cvCreateImage(cvGetSize(dest),IPL_DEPTH_8U, 3);
	cvZero(imgcnt);
	cvZero(imgacc);
	cvZero(imgc);
	POINT shift[2] = {/*{-1,-1},*/{0,-1},/*{1,-1},*/{-1,0}};
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			uchar* pix = PTR_PIX(*src, j, i);
			int* pacc = (int*)PTR_PIX(*imgacc, j, i);
			uchar* pc = PTR_PIX(*imgc, j, i);
			Vec3 psrc(*pix,*(pix+1),*(pix+2));
			Vec3 adjc(0,0,0);
			Vec3 adj[2];
			Vec3 adj2[2];
			for(int k=0;k<2;k++)
			{
				int ni = i+shift[k].y;
				int nj = j+shift[k].x;
				if(ni<0)ni=0;
				else if(ni>=dest->height)ni=dest->height-1;
				if(nj<0)nj=0;
				else if(nj>=dest->width)nj=dest->width-1;
				int* adjpix = (int*)PTR_PIX(*imgacc, nj, ni);
				uchar* adjcpix = PTR_PIX(*dest, nj, ni);
				adj[k] = Vec3(*adjpix, *(adjpix+1), *(adjpix+2));
				adj2[k] = Vec3(*adjcpix, *(adjcpix+1), *(adjcpix+2));
			}

			uchar* pdest = PTR_PIX(*dest, j, i);

			float mindif = length(adj[0] - psrc);
			int index = 0;
			for(int k=0;k<2;k++)
			{
				if(mindif>length(adj[k] - psrc))
				{
					mindif = length(adj[k] - psrc);
					index = k;
				}
			}
			if(mindif < 20)
			{
				*pdest     = adj2[index].x;
				*(pdest+1) = adj2[index].y;
				*(pdest+2) = adj2[index].z;

				Vec3 nacc = 1*adj[index]+0.*psrc;

				*pacc     = nacc.x;
				*(pacc+1) = nacc.y;
				*(pacc+2) = nacc.z;
			}
			else
			{
				*pdest     = psrc.x;
				*(pdest+1) = psrc.y;
				*(pdest+2) = psrc.z;
				
				*pacc     = psrc.x;
				*(pacc+1) = psrc.y;
				*(pacc+2) = psrc.z;
			}
		}
	}

	cvReleaseImage(&imgcnt);
	cvReleaseImage(&imgacc);
	cvReleaseImage(&imgc);
}
#else
#ifdef VB
//不缓存，一次遇到，输出不缓存
void CChildView::Process(IplImage* src, IplImage* dest)
{
	cvZero(dest);
	IplImage* imgcnt = cvCreateImage(cvGetSize(dest),IPL_DEPTH_32S, 1);
	IplImage* imgacc = cvCreateImage(cvGetSize(dest),IPL_DEPTH_32S, 3);
	IplImage* imgc = cvCreateImage(cvGetSize(dest),IPL_DEPTH_8U, 3);
	cvZero(imgcnt);
	cvZero(imgacc);
	cvZero(imgc);
	POINT shift[4] = {{-1,-1},{0,-1},{1,-1},{-1,0}};
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			uchar* pix = PTR_PIX(*src, j, i);
			int* pacc = (int*)PTR_PIX(*imgacc, j, i);
			uchar* pc = PTR_PIX(*imgc, j, i);
			Vec3 psrc(*pix,*(pix+1),*(pix+2));
			Vec3 adjc(0,0,0);
			Vec3 adj[4];
			Vec3 adj2[4];
/*			for(int k=0;k<4;k++)
			{
				int ni = i+shift[k].y;
				int nj = j+shift[k].x;
				if(ni<0)ni=0;
				else if(ni>=dest->height)ni=dest->height-1;
				if(nj<0)nj=0;
				else if(nj>=dest->width)nj=dest->width-1;
				int* adjpix = (int*)PTR_PIX(*imgacc, nj, ni);
				uchar* adjcpix = PTR_PIX(*dest, nj, ni);
				adj[k] = Vec3(*adjpix, *(adjpix+1), *(adjpix+2));
//				adjc += adj[k];
				adj2[k] = Vec3(*adjcpix, *(adjcpix+1), *(adjcpix+2));
				if(length(adj[k]-psrc)<30)
				{
					Vec3 nacc = adj[k]*0.8+psrc*0.2;

					*pacc = nacc.x;
					*(pacc+1) = nacc.y;
					*(pacc+2) = nacc.z;

					*pc = adjc2.x;
					*(pc+1) = adjc2.y;
					*(pc+2) = adjc2.z;

					break;
				}

			}
*//*		adjc/=4;
			uchar* pdest = PTR_PIX(*dest, j, i);
			if(length(psrc-adjc)>40)
			{
				*pdest=psrc.x;
				*(pdest+1)=psrc.y;
				*(pdest+2)=psrc.z;

				*pacc = psrc.x;
				*(pacc+1) = psrc.y;
				*(pacc+2) = psrc.z;
			}
			else
			{
				Vec3 nacc = adjc*0.6+psrc*0.4;
				*pacc = nacc.x;
				*(pacc+1) = nacc.y;
				*(pacc+2) = nacc.z;
				float mindif=length(adj[0]-psrc);
				int index=0;
				for(int k=0;k<4;k++)
				{
					if(length(adj[k]-psrc)<mindif)
					{
						mindif = length(adj[k]-psrc);
						index = k;
					}
				}
				*pdest=adj2[index].x;
				*(pdest+1)=adj2[index].y;
				*(pdest+2)=adj2[index].z;
			}
*/

			bool bPro = false;
			for(int k=0;k<4;k++)
			{
				int ni = i+shift[k].y;
				int nj = j+shift[k].x;
				if(ni<0)ni=0;
				else if(ni>=dest->height)ni=dest->height-1;
				if(nj<0)nj=0;
				else if(nj>=dest->width)nj=dest->width-1;
				uchar* pndest = PTR_PIX(*dest, nj, ni);
				adj[k] = Vec3(*pndest, *(pndest+1), *(pndest+2));
				if(length(psrc - adj[k])<30)
				{
					uchar* pdest = PTR_PIX(*dest, j, i);
					*pdest = adj[k].x;
					*(pdest+1) = adj[k].y;
					*(pdest+2) = adj[k].z;
					bPro = true;
					break;
				}
			}
			if(!bPro)
			{
				uchar* pdest = PTR_PIX(*dest, j, i);
				*pdest=psrc.x;
				*(pdest+1)=psrc.y;
				*(pdest+2)=psrc.z;
			}
		}
	}

//	cvCopyImage(imgc, dest);
	cvReleaseImage(&imgcnt);
	cvReleaseImage(&imgacc);
	cvReleaseImage(&imgc);
}
#else
#ifdef NB
//缓存，最小遇到，输出不缓存
void CChildView::Process(IplImage* src, IplImage* dest)
{
	cvZero(dest);
	IplImage* imgcnt = cvCreateImage(cvGetSize(dest),IPL_DEPTH_32S, 1);
	IplImage* imgacc = cvCreateImage(cvGetSize(dest),IPL_DEPTH_32S, 3);
	IplImage* imgc = cvCreateImage(cvGetSize(dest),IPL_DEPTH_8U, 3);
	cvZero(imgcnt);
	cvZero(imgacc);
	cvZero(imgc);
	POINT shift[4] = {{-1,-1},{0,-1},{1,-1},{-1,0}};
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			uchar* pix = PTR_PIX(*src, j, i);
			int* pacc = (int*)PTR_PIX(*imgacc, j, i);
			uchar* pc = PTR_PIX(*imgc, j, i);
			Vec3 psrc(*pix,*(pix+1),*(pix+2));
			Vec3 adjc(0,0,0);
			Vec3 adj[4];
			Vec3 adj2[4];
			for(int k=0;k<4;k++)
			{
				int ni = i+shift[k].y;
				int nj = j+shift[k].x;
				if(ni<0)ni=0;
				else if(ni>=dest->height)ni=dest->height-1;
				if(nj<0)nj=0;
				else if(nj>=dest->width)nj=dest->width-1;
				int* adjpix = (int*)PTR_PIX(*imgacc, nj, ni);
				uchar* adjcpix = PTR_PIX(*dest, nj, ni);
				adj[k] = Vec3(*adjpix, *(adjpix+1), *(adjpix+2));
				adjc += adj[k];
				adj2[k] = Vec3(*adjcpix, *(adjcpix+1), *(adjcpix+2));
/*				if(length(adj[k]-psrc)<30)
				{
					Vec3 nacc = adj[k]*0.8+psrc*0.2;

					*pacc = nacc.x;
					*(pacc+1) = nacc.y;
					*(pacc+2) = nacc.z;

					*pc = adjc2.x;
					*(pc+1) = adjc2.y;
					*(pc+2) = adjc2.z;

					break;
				}
*/
			}
			adjc/=4;
			uchar* pdest = PTR_PIX(*dest, j, i);
			if(length(psrc-adjc)>40)
			{
				*pdest=psrc.x;
				*(pdest+1)=psrc.y;
				*(pdest+2)=psrc.z;

				*pacc = psrc.x;
				*(pacc+1) = psrc.y;
				*(pacc+2) = psrc.z;
			}
			else
			{
				Vec3 nacc = adjc*0.6+psrc*0.4;
				*pacc = nacc.x;
				*(pacc+1) = nacc.y;
				*(pacc+2) = nacc.z;
				float mindif=length(adj[0]-psrc);
				int index=0;
				for(int k=0;k<4;k++)
				{
					if(length(adj[k]-psrc)<mindif)
					{
						mindif = length(adj[k]-psrc);
						index = k;
					}
				}
				*pdest=adj2[index].x;
				*(pdest+1)=adj2[index].y;
				*(pdest+2)=adj2[index].z;
			}

/*			if(!bPro)
			{
				*pacc = psrc.x;
				*(pacc+1) = psrc.y;
				*(pacc+2) = psrc.z;

				*pc = psrc.x;
				*(pc+1) = psrc.y;
				*(pc+2) = psrc.z;
			}
*/
/*			bool bPro = false;
			for(int k=0;k<4;k++)
			{
				int ni = i+shift[k].y;
				int nj = j+shift[k].x;
				if(ni<0)ni=0;
				else if(ni>=dest->height)ni=dest->height-1;
				if(nj<0)nj=0;
				else if(nj>=dest->width)nj=dest->width-1;
				uchar* pndest = PTR_PIX(*dest, nj, ni);
				adj[k] = Vec3(*pndest, *(pndest+1), *(pndest+2));
				if(length(psrc - adj[k])<30)
				{
					uchar* pdest = PTR_PIX(*dest, j, i);
					*pdest = adj[k].x;
					*(pdest+1) = adj[k].y;
					*(pdest+2) = adj[k].z;
					bPro = true;
					break;
				}
				adjc+=adj[k];
			}
			adjc/=4;
			if(!bPro)
			{
				uchar* pdest = PTR_PIX(*dest, j, i);
				*pdest=psrc.x;
				*(pdest+1)=psrc.y;
				*(pdest+2)=psrc.z;
			}*/
		}
	}
/*
	cvZero(imgacc);
	cvZero(imgcnt);
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			bool bStart = false;
			uchar* pixc = PTR_PIX(*imgc, j, i);
			int* pcnt = (int*)PTR_PIX(*imgcnt, j, i);
			int* pacc = (int*)PTR_PIX(*imgacc, j, i);
			uchar* psrc = PTR_PIX(*src, j, i);
			if(j == 0)bStart = true;
			else
			{
				int nj = j-1;
				uchar* pixnc = PTR_PIX(*imgc, nj, i);
				if(*pixc == *pixnc && *(pixc+1) == *(pixnc+1) && *(pixc+2) == *(pixnc+2))
				{
					bStart = false;
				}
				else
				{
					bStart = true;
				}
			}
			if(bStart)
			{
				*pcnt = 1;
				*pacc = *psrc;
				*(pacc+1) = *(psrc+1);
				*(pacc+2) = *(psrc+2);
			}
			else
			{
				int nj = j-1;
				int* pncnt = (int*)PTR_PIX(*imgcnt, nj, i);
				int* pnacc = (int*)PTR_PIX(*imgacc, nj, i);
				*pcnt = *pncnt+1;
				*pnacc = *pacc + *psrc;
				*(pnacc+1) = *(pacc+1) + *(psrc+1);
				*(pnacc+2) = *(pacc+2) + *(psrc+2);
			}
		}
		for(int j=dest->width-1;j>=0;j--)
		{
			bool bStart = false;
			uchar* pixc = PTR_PIX(*imgc, j, i);
			int* pcnt = (int*)PTR_PIX(*imgcnt, j, i);
			int* pacc = (int*)PTR_PIX(*imgacc, j, i);
			if(j == dest->width-1)bStart = true;
			else
			{
				int nj = j+1;
				uchar* pixnc = PTR_PIX(*imgc, nj, i);
				if(*pixc == *pixnc && *(pixc+1) == *(pixnc+1) && *(pixc+2) == *(pixnc+2))
				{
					bStart = false;
				}
				else
				{
					bStart = true;
				}
			}
			if(!bStart)
			{
				int nj = j+1;
				int* pncnt = (int*)PTR_PIX(*imgcnt, nj, i);
				int* pnacc = (int*)PTR_PIX(*imgacc, nj, i);
				*pcnt = *pncnt;
				*pacc = *pnacc;
				*(pacc+1) = *(pnacc+1);
				*(pacc+2) = *(pnacc+2);
			}
		}
	}
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			
		}
	}
*/
//	cvCopyImage(imgc, dest);
	cvReleaseImage(&imgcnt);
	cvReleaseImage(&imgacc);
	cvReleaseImage(&imgc);
}
#else
#ifdef NM
//不缓存，最小遇到，输出不缓存
void CChildView::Process(IplImage* src, IplImage* dest)
{
/*	IplImage* mask = cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 1);
	IplImage* maska = cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 1);
	cvZero(mask);
	cvZero(maska);
*/	cvZero(dest);
	POINT shift[4] = {{-1,-1},{0,-1},{1,-1},{-1,0}};
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			uchar* pix = PTR_PIX(*src, j, i);
			Vec3 psrc(*pix,*(pix+1),*(pix+2));
			Vec3 adjc(0,0,0);
			Vec3 adj[4];

			for(int k=0;k<4;k++)
			{
				int ni = i+shift[k].y;
				int nj = j+shift[k].x;
				if(ni<0)ni=0;
				else if(ni>=dest->height)ni=dest->height-1;
				if(nj<0)nj=0;
				else if(nj>=dest->width)nj=dest->width-1;
				uchar* pdest = PTR_PIX(*dest, nj, ni);
				adj[k] = Vec3(*pdest, *(pdest+1), *(pdest+2));
				adjc+=adj[k];
			}
			adjc/=4;
			uchar* pdest = PTR_PIX(*dest, j, i);
			if(length(psrc-adjc)>70)
			{
				*pdest=psrc.x;
				*(pdest+1)=psrc.y;
				*(pdest+2)=psrc.z;
			}
			else
			{
				float mindif=length(adj[0]-psrc);
				int index=0;
				for(int k=0;k<4;k++)
				{
					if(length(adj[k]-psrc)<mindif)
					{
						mindif = length(adj[k]-psrc);
						index = k;
					}
				}
				*pdest=adj[index].x;
				*(pdest+1)=adj[index].y;
				*(pdest+2)=adj[index].z;
			}
		}
	}
/*	IplImage* tmp = cvCloneImage(dest);


	for(int i=0;i<tmp->height;i++)
	{
		for(int j=0;j<tmp->width;j++)
		{
			uchar* pix = PTR_PIX(*tmp, j, i);
			uchar* pm = PTR_PIX(*maska, j, i);
			if(*pm == 0)
			{
				cvCopyImage(dest,tmp);
				cvFloodFill(tmp, cvPoint(j,i), cvScalarAll(0));
				cvInRangeS(tmp,cvScalarAll(0), cvScalarAll(1), mask);
				//int nP = cvCountNonZero(mask);
				CvScalar sc = cvAvg(dest, mask);
				cvFloodFill(dest,cvPoint(j,i),sc);
				cvOr(maska, mask, maska);
			}
		}
	}
	cvReleaseImage(&tmp);
	cvReleaseImage(&mask);
	cvReleaseImage(&maska);*/
}
#else
#ifdef RT
//不缓存，最小遇到，输出不缓存
void CChildView::Process(IplImage* src, IplImage* dest)
{
	IplImage* intg = cvCreateImage(cvSize(src->width+1,src->height+1), IPL_DEPTH_32S, 3);
	cvIntegral(src, intg);
	cvZero(dest);
	POINT shift[4] = {{-1,-1},{0,-1},{1,-1},{-1,0}};
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			uchar* pix = PTR_PIX(*src, j, i);
			Vec3 psrc(*pix,*(pix+1),*(pix+2));
			Vec3 adjc(0,0,0);
			Vec3 adj[4];

			for(int k=0;k<4;k++)
			{
				int ni = i+shift[k].y;
				int nj = j+shift[k].x;
				if(ni<0)ni=0;
				else if(ni>=dest->height)ni=dest->height-1;
				if(nj<0)nj=0;
				else if(nj>=dest->width)nj=dest->width-1;
				uchar* pdest = PTR_PIX(*dest, nj, ni);
				adj[k] = Vec3(*pdest, *(pdest+1), *(pdest+2));
				adjc+=adj[k];
			}
			adjc/=4;
			uchar* pdest = PTR_PIX(*dest, j, i);
			if(length(psrc-adjc)>100)
			{
				*pdest=psrc.x;
				*(pdest+1)=psrc.y;
				*(pdest+2)=psrc.z;
			}
			else
			{
				float mindif=length(adj[0]-psrc);
				int index=0;
				for(int k=0;k<4;k++)
				{
					if(length(adj[k]-psrc)<mindif)
					{
						mindif = length(adj[k]-psrc);
						index = k;
					}
				}
				*pdest=adj[index].x;
				*(pdest+1)=adj[index].y;
				*(pdest+2)=adj[index].z;
			}
		}
	}
	cvReleaseImage(&intg);
}
#else
//缓存，最小遇到，输出缓存
void CChildView::Process(IplImage* src, IplImage* dest)
{
	cvZero(dest);
	IplImage* imgcnt = cvCreateImage(cvGetSize(dest),IPL_DEPTH_32S, 1);
	IplImage* imgacc = cvCreateImage(cvGetSize(dest),IPL_DEPTH_32S, 3);
	IplImage* imgc = cvCreateImage(cvGetSize(dest),IPL_DEPTH_8U, 3);
	cvZero(imgcnt);
	cvZero(imgacc);
	cvZero(imgc);
	POINT shift[4] = {{-1,-1},{0,-1},{1,-1},{-1,0}};
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			uchar* pix = PTR_PIX(*src, j, i);
			int* pacc = (int*)PTR_PIX(*imgacc, j, i);
			uchar* pc = PTR_PIX(*imgc, j, i);
			Vec3 psrc(*pix,*(pix+1),*(pix+2));
			Vec3 adjc(0,0,0);
			Vec3 adj[4];
			Vec3 adj2[4];
			for(int k=0;k<4;k++)
			{
				int ni = i+shift[k].y;
				int nj = j+shift[k].x;
				if(ni<0)ni=0;
				else if(ni>=dest->height)ni=dest->height-1;
				if(nj<0)nj=0;
				else if(nj>=dest->width)nj=dest->width-1;
				int* adjpix = (int*)PTR_PIX(*imgacc, nj, ni);
				uchar* adjcpix = PTR_PIX(*dest, nj, ni);
				adj[k] = Vec3(*adjpix, *(adjpix+1), *(adjpix+2));
				adjc += adj[k];
				adj2[k] = Vec3(*adjcpix, *(adjcpix+1), *(adjcpix+2));
			}
			adjc/=4;
			uchar* pdest = PTR_PIX(*dest, j, i);
			if(length(psrc-adjc)>40)
			{
				*pdest=psrc.x;
				*(pdest+1)=psrc.y;
				*(pdest+2)=psrc.z;

				*pacc = psrc.x;
				*(pacc+1) = psrc.y;
				*(pacc+2) = psrc.z;
			}
			else
			{
				Vec3 nacc = adjc*1+psrc*0;
				*pacc = nacc.x;
				*(pacc+1) = nacc.y;
				*(pacc+2) = nacc.z;
				float mindif=length(adj[0]-psrc);
				int index=0;
				for(int k=0;k<4;k++)
				{
					if(length(adj[k]-psrc)<mindif)
					{
						mindif = length(adj[k]-psrc);
						index = k;
					}
				}
				*pdest=adj[index].x;
				*(pdest+1)=adj[index].y;
				*(pdest+2)=adj[index].z;
			}


		}
	}
//	cvCopyImage(imgc, dest);
	cvReleaseImage(&imgcnt);
	cvReleaseImage(&imgacc);
	cvReleaseImage(&imgc);
}
#endif
#endif
#endif
#endif
#endif