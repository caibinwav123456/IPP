
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "MeshFromObj.h"
#include "ChildView.h"
#include "Gdiplus.h"
using namespace Gdiplus;

#ifdef _DEBUG
//#define new DEBUG_NEW
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
	cvCvtScale(m_dest[m_index], disp);
	CDC mdc;
	mdc.CreateCompatibleDC(&dc);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc, rcClt.Width(), rcClt.Height());
	CBitmap* oldbmp = mdc.SelectObject(&bitmap);
	mdc.FillSolidRect(0,0,rcClt.Width(),rcClt.Height(),RGB(255,255,255));
	DispImage(&mdc, disp, CPoint(0,0));
	cvReleaseImage(&disp);
	dc.BitBlt(0,0,rcClt.Width(),rcClt.Height(),&mdc,0,0,SRCCOPY);
	mdc.SelectObject(oldbmp);
	mdc.DeleteDC();
	bitmap.DeleteObject();
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
	m_dest[0] = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,3);
	m_dest[1] = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,3);
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
		{
			IplImage* tmp = cvCreateImage(cvGetSize(m_ImgBk1), IPL_DEPTH_32F, 3);
			cvCvtScale(m_ImgBk1,tmp);
			IplImage* gray = cvCreateImage(cvGetSize(m_ImgBk1), IPL_DEPTH_32F, 1);
			cvCvtColor(tmp,gray,CV_RGB2GRAY);
			cvReleaseImage(&tmp);
			IplImage* integ = cvCreateImage(cvSize(m_ImgBk1->width+1, m_ImgBk1->height+1), IPL_DEPTH_64F, 1);
			IplImage* sqinteg = cvCreateImage(cvSize(m_ImgBk1->width+1, m_ImgBk1->height+1), IPL_DEPTH_64F, 1);
			cvIntegral(gray, integ, sqinteg);
			cvReleaseImage(&gray);
			int spanx = m_ImgBk1->width/10;
			int spany = m_ImgBk1->height/10;
			IplImage* dev = cvCreateImage(cvGetSize(m_ImgBk1), IPL_DEPTH_32F, 1);
			for(int i=0;i<m_ImgBk1->height;i++)
			{
				for(int j=0;j<m_ImgBk1->width;j++)
				{
					int i0 = max(0,i-spany),i1 = min(m_ImgBk1->height,i+spany+1);
					int j0 = max(0,j-spanx),j1 = min(m_ImgBk1->width,j+spanx+1);
					if(i0==0)i1=2*spany+1;
					if(i1==m_ImgBk1->height)i0=m_ImgBk1->height-2*spany-1;
					if(j0==0)j1=2*spanx+1;
					if(j1==m_ImgBk1->width)j0=m_ImgBk1->width-2*spanx-1;

					int cnt = (i1-i0)*(j1-j0);

					double sump = *(double*)PTR_PIX(*integ, j1, i1)
						-*(double*)PTR_PIX(*integ, j0, i1)
						-*(double*)PTR_PIX(*integ, j1, i0)
						+*(double*)PTR_PIX(*integ, j0, i0);
					double sqsump =  *(double*)PTR_PIX(*sqinteg, j1, i1)
						-*(double*)PTR_PIX(*sqinteg, j0, i1)
						-*(double*)PTR_PIX(*sqinteg, j1, i0)
						+*(double*)PTR_PIX(*sqinteg, j0, i0);
					float mean = sump/cnt;
					float sq = sqsump/cnt;
					float stddev = sqrt(sq-mean*mean);
					*(float*)PTR_PIX(*dev, j, i) = stddev/3;
				}
			}
			cvReleaseImage(&integ);
			cvReleaseImage(&sqinteg);
			for(int n=0;n<8;n++)
			{
				Bilateral(m_dest[1-m_index], m_dest[m_index], dev);
				m_index=1-m_index;
			}
			cvReleaseImage(&dev);
			Invalidate();
		}
		break;
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChildView::Process(IplImage* src, IplImage* dest)
{
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
					float dd;//=ds*(ds-dmax.elem[l]*0.3)*(ds-dmax.elem[l]*0.7)/0.21/dmax.elem[l]/dmax.elem[l]/dmax.elem[l];
					if(ds>0)dd=1;
					else if(ds<0)dd=-1;
					else dd=0;
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

void CChildView::Median(IplImage* src, IplImage* dest)
{
	int heapmap[5] = {0,0,0,1,1};
	int heapmap2[5][2] = {{1,2},{3,4},{-1,-1},{-1,-1},{-1,-1}};
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			Vec3 d(0,0,0);
			POINT off[5]={{0,0},{1,0},{0,1},{-1,0},{0,-1}};
			float arr[3][5];
			for(int k=0;k<5;k++)
			{
				int nx=j+off[k].x,ny=i+off[k].y;
				if(nx<0)nx=0;
				if(nx>src->width-1)nx=src->width-1;
				if(ny<0)ny=0;
				if(ny>src->height-1)ny=src->height-1;
				float* pd=(float*)PTR_PIX(*src, nx, ny);
				for(int l=0;l<3;l++)
				{
					float ds=*(pd+l);
					arr[l][k]=ds;
					int ind = k;
					while(ind!=0)
					{
						int ind2 = heapmap[ind];
						if(arr[l][ind]<arr[l][ind2])
							swap(arr[l][ind],arr[l][ind2]);
						else
							break;
						ind = ind2;
					}
				}
			}
			float* pdest=(float*)PTR_PIX(*dest, j,i);
			for(int k=0;k<3;k++)
			{
				float minf;
				for(int l=0;l<3;l++)
				{
					minf = arr[k][0];
					if(l==2)break;
					arr[k][0] = arr[k][4-l];
					int ind = 0;
					while((heapmap2[ind][0]!=-1&&heapmap2[ind][0]<4-l)
						||(heapmap2[ind][1]!=-1&&heapmap2[ind][1]<4-l))
					{
						if(arr[k][heapmap2[ind][0]]<arr[k][ind])
						{
							swap(arr[k][heapmap2[ind][0]],arr[k][ind]);
							ind = heapmap2[ind][0];
						}
						else if(arr[k][heapmap2[ind][1]]<arr[k][ind])
						{
							swap(arr[k][heapmap2[ind][1]],arr[k][ind]);
							ind = heapmap2[ind][1];
						}
						else
							break;
					}
				}
				*(pdest+k)=minf;
			}
		}
	}
}

void CChildView::Bilateral(IplImage* src, IplImage* dest, IplImage* dev)
{
	int span = 5;
	float scalep = 5;
	float scalev = 20;
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			float* pixsc = (float*)PTR_PIX(*src, j, i);
			Vec3 vsc(*pixsc, *(pixsc+1), *(pixsc+2));

			int pi = max(0,i-span);
			int ni = min(src->height,i+span);
			int pj = max(0,j-span);
			int nj = min(src->width,j+span);

			float sc = 0;
			Vec3 sum(0,0,0);
			scalev = *(float*)PTR_PIX(*dev, j, i);
			for(int k=pi;k<ni;k++)
			{
				for(int l=pj;l<nj;l++)
				{
					float* pixs = (float*)PTR_PIX(*src, l, k);
					Vec3 vs(*pixs,*(pixs+1),*(pixs+2));
					Vec2 polar(l-j,k-i);
					float dotvs = dot(vs-vsc,vs-vsc);
					float dotp = dot(polar, polar);
					float coeff = expf(-dotvs/(scalev*scalev)-dotp/(scalep*scalep));
					sum+=coeff*vs;
					sc+=coeff;
				}
			}
			Vec3 vd = sum/sc;

			float* pixd = (float*)PTR_PIX(*dest, j, i);
			*pixd = vd.x;
			*(pixd+1) = vd.y;
			*(pixd+2) = vd.z;
		}
	}
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
void CChildView::OnFileOpen2()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("All Files|*.*||"), this);
	if(dlg.DoModal() == IDOK)
	{
		CString strName = dlg.GetPathName();
		cvReleaseImage(&m_ImgBk1);
		IplImage* image=loadWithGdiPlus(CT2W(strName));
		float xscale=1024./image->width;
		float yscale=768./image->height;
		float scale=min(xscale,yscale);
		m_ImgBk1=cvCreateImage(cvSize(image->width*scale,image->height*scale), IPL_DEPTH_8U, 3);
		cvResize(image, m_ImgBk1);
		cvReleaseImage(&image);

		int wImage=m_ImgBk1->width;
		int hImage=m_ImgBk1->height;

		cvReleaseImage(&m_Image);
		cvReleaseImage(&m_debuf);
		cvReleaseImage(&m_dest[0]);
		cvReleaseImage(&m_dest[1]);
		m_Image = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,3);
		m_debuf = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,1);
		m_dest[0] = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,3);
		m_dest[1] = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,3);
		m_depImg= m_debuf;
		m_texture = m_ImgBk1;
		cvCvtScale(m_ImgBk1,m_dest[0]);
		cvCvtScale(m_ImgBk1,m_dest[1]);

		Draw();
	}
}
