
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
	m_water[0] = m_water[1] = NULL;
	m_ink[0] = m_ink[1] = NULL;
	m_sink[0] = m_sink[1] = NULL;
	m_mask[0]= m_mask[1] = NULL;

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
	cvReleaseImage(&m_dest[0]);
	cvReleaseImage(&m_dest[1]);

	for(int i=0;i<2;i++)
	{
		cvReleaseImage(&m_water[i]);
		cvReleaseImage(&m_ink[i]);
		cvReleaseImage(&m_sink[i]);
		cvReleaseImage(&m_mask[i]);
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

	//IplImage* disp=cvCreateImage(cvGetSize(m_dest[m_index]),IPL_DEPTH_8U,3);
	IplImage* water=cvCreateImage(cvSize(350,350),IPL_DEPTH_8U,3);
	IplImage* ink=cvCreateImage(cvSize(350,350), IPL_DEPTH_8U,3);
	IplImage* sink=cvCreateImage(cvSize(350,350), IPL_DEPTH_8U,3);
	IplImage*mask=cvCreateImage(cvSize(350,350), IPL_DEPTH_8U,3);
	IplImage* tmp1=cvCreateImage(cvSize(350,350), IPL_DEPTH_32F,1);
	IplImage* tmp2=cvCreateImage(cvSize(350,350), IPL_DEPTH_8U, 1);
	IplImage* tmp3=cvCreateImage(cvSize(350,350), IPL_DEPTH_32F, 3);
	cvResize(m_water[1-m_index], tmp1);
	cvCvtScale(tmp1, tmp2);
	cvMerge(tmp2,tmp2,tmp2,NULL, water);
	cvResize(m_mask[1-m_index], tmp2);
	cvMerge(tmp2,tmp2,tmp2,NULL, mask);
	cvResize(m_ink[1-m_index], tmp3);
	cvCvtScale(tmp3, ink);
	cvResize(m_sink[1-m_index], tmp3);
	cvCvtScale(tmp3, sink);

	DispImage(&m_mdc, water, CPoint(0,0));
	DispImage(&m_mdc, ink, CPoint(350,0));
	DispImage(&m_mdc, sink, CPoint(0,350));
	DispImage(&m_mdc, mask, CPoint(350,350));


	dc.BitBlt(0,0,700,700,&m_mdc,0,0,SRCCOPY);
	cvReleaseImage(&water);
	cvReleaseImage(&ink);
	cvReleaseImage(&sink);
	cvReleaseImage(&mask);
	cvReleaseImage(&tmp1);
	cvReleaseImage(&tmp2);
	cvReleaseImage(&tmp3);
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
	cvSmooth(m_ImgBk1, m_ImgBk1,CV_GAUSSIAN, 11);

	int wImage=m_ImgBk1->width;
	int hImage=m_ImgBk1->height;

	m_Image = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,3);
	m_debuf = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,1);
	m_dest[0] = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,3);
	m_dest[1] = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,3);
	for(int i=0;i<2;i++)
	{
		m_water[i]=cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,1);
		m_ink[i]  =cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,3);
		m_sink[i] =cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,3);

		cvCvtScale(m_ImgBk1, m_ink[i]);
		cvSet(m_sink[i], cvScalar(0));
		IplImage* hsv=cvCreateImage(cvGetSize(m_ImgBk1),IPL_DEPTH_32F, 3);
		cvCvtColor(m_ink[i], hsv, CV_RGB2HSV);
		cvSplit(hsv, NULL, NULL, m_water[i], NULL);
		cvSubRS(m_water[i], cvScalar(255), m_water[i]);
		cvReleaseImage(&hsv);
		m_mask[i] =cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,1);
		cvSet(m_mask[i], cvScalar(255));
	}
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
		Process(m_water[1-m_index], m_water[m_index], m_ink[1-m_index], m_ink[m_index], m_sink[1-m_index], m_sink[m_index], m_mask[1-m_index], m_mask[m_index], m_ImgBk1->width, m_ImgBk1->height);
		m_index=1-m_index;
		Invalidate();
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
#define WATERDEC  1
#define WATERF 5
#define INKF   20
#define SINKF  5
#define DISSOLVEF 0.1
#define THRESHCAPIL 60
void CChildView::Process(IplImage* waterin, IplImage* waterout, IplImage* inkin, IplImage* inkout, IplImage* sinkin, IplImage* sinkout, IplImage* maskin, IplImage* maskout, int width, int height)
{
	CvPoint offset[4]={cvPoint(1,0),cvPoint(0,1),cvPoint(-1,0),cvPoint(0,-1)};
	for(int i=0;i<height;i++)
	{
		for(int j=0;j<width;j++)
		{
			float* pwin=(float*)PTR_PIX(*waterin, j, i);
			float* pwout=(float*)PTR_PIX(*waterout, j, i);
			float* pikin=(float*)PTR_PIX(*inkin, j, i);
			float* pikout=(float*)PTR_PIX(*inkout, j, i);
			float* pskin=(float*)PTR_PIX(*sinkin, j, i);
			float* pskout=(float*)PTR_PIX(*sinkout, j, i);
			uchar* pmask=PTR_PIX(*maskin, j, i);
			uchar* pmaskout=PTR_PIX(*maskout, j, i);

			float owater=*pwin;
			Vec3 oink(*pikin, *(pikin+1), *(pikin+2));
			Vec3 osink(*pskin,*(pskin+1), *(pskin+2));
			uchar omask=*pmask;

			if(*pmask)
			{
				float water=*pwin;
				Vec3 ink(*pikin, *(pikin+1), *(pikin+2));
				float mik=(ink.x+ink.y+ink.z)/3;
				float c=mik/(mik+water);
				Vec3 dink(0,0,0);
				float dw=0;
				for(int k=0;k<4;k++)
				{
					int nx=j+offset[k].x;
					int ny=i+offset[k].y;

					nx=max(0, nx);
					nx=min(nx, width-1);
					ny=max(0, ny);
					ny=min(ny, height-1);
					uchar* npm=PTR_PIX(*maskin, nx, ny);
					if(*npm)
					{
						float* npw=(float*)PTR_PIX(*waterin, nx, ny);
						float* npik=(float*)PTR_PIX(*inkin, nx, ny);
						float nw=*npw;
						Vec3 nik(*npik,*(npik+1),*(npik+2));
						float mnik=(nik.x+nik.y+nik.z)/3;
						float nc=mnik/(nw+mnik);
						float dc=nc-c;
						if(nw+mnik>0)
						{
							dw-=WATERF*dc*nw/(nw+mnik);
							dink-=(INKF*dc/(nw+mnik))*nik;
						}	
					}
				}
				Vec3 dsink=-c*SINKF/mik*ink;
				if(mik==0)
					dsink=Vec3(0,0,0);
				Vec3 sink(*pskin,*(pskin+1),*(pskin+2));
				Vec3 ddisv=sink*DISSOLVEF;
				if(owater+dw>0)
				{
					owater=water+dw;//-WATERDEC;
					oink=ink+dink;//+dsink+ddisv;
				}
				osink=sink;//-dsink-ddisv;
				if(i==234&&j==432)
				{
					int o=0;
  					o++;
				}
				for(int k=0;k<3;k++)
				{
					if(oink.elem[k]<0)
						oink.elem[k]=0;
					if(osink.elem[k]<0)
						osink.elem[k]=0;
				}
				if(owater<0)
				{
					owater=0;
					//omask=0;
					//osink+=oink;
					//oink=Vec3(0,0,0);
				}
			}
			*pwout=owater;
			*pikout=oink.x;
			*(pikout+1)=oink.y;
			*(pikout+2)=oink.z;
			*pskout=osink.x;
			*(pskout+1)=osink.y;
			*(pskout+2)=osink.z;
			*pmaskout=omask;
			if(owater>THRESHCAPIL)
			{
/*
				for(int k=0;k<4;k++)
				{
					int nx=j+offset[k].x;
					int ny=i+offset[k].y;

					nx=max(0, nx);
					nx=min(nx, width-1);
					ny=max(0, ny);
					ny=min(ny, height-1);
					uchar* npmout=PTR_PIX(*maskout, nx, ny);
					*npmout=255;
				}
*/
			}
			uchar* npmout=PTR_PIX(*maskout, j, i);
			int o=0;
			o++;
		}
	}
}