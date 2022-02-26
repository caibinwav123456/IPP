
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "PhotoFrame.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PI 3.1415926535897932384626
// CChildView
int IMAGE_WIDTH=0;
int IMAGE_HEIGHT=0;
float GEO_WIDTH=2;
float GEO_HEIGHT=2;

Vec3 Pos(0,0,-3);
Mat rot(1,0,0,0,1,0,0,0,1);
Mat rotx,roty;

IplImage* g_ImgBk1 = NULL;
IplImage* g_ImgBk2 = NULL;
IplImage* g_deBuf = NULL;
void Pixel(RawImage* target,Point2D pos, int count, float* pdata)
{
	uchar* ptr=PTR_PIX(*target, pos.x, pos.y);
	float* fptr = (float*)PTR_PIX(*g_deBuf, pos.x, pos.y);
	if(*fptr<1./pdata[1])
	{
		uchar* ptr=PTR_PIX(*target, pos.x, pos.y);
		//uchar* ptrbk1=PTR_PIX(*g_ImgBk1, pos.x, pos.y);
		//uchar* ptrbk2=PTR_PIX(*g_ImgBk2, pos.x, pos.y);

		Vec2 tex(pdata[2],pdata[3]);
		CvScalar pSamp = Sample(g_ImgBk1, tex);
		*ptr    = max(0,*pdata)*pSamp.val[0];//(float)(lu-128)/256;
		*(ptr+1)= max(0,*pdata)*pSamp.val[1];//(float)(lu-128)/256;
		*(ptr+2)= max(0,*pdata)*pSamp.val[2];//(float)(lu-128)/256;
		*fptr=1./pdata[1];
	}

}


CChildView::CChildView()
{
	m_bShowVert = false;
	m_bShowImage = true;
	m_bDown = false;

	m_Image = NULL;
	m_ImgBk1 = NULL;
	m_ImgBk2 = NULL;

	oldbmp = NULL;

	m_vlist = NULL;
	m_tvlist = NULL;
	tnormal = NULL;
	m_texbuf = NULL;
	light = NULL;
	m_ibuf = NULL;
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
	if(g_deBuf != NULL)
	{
		cvReleaseImage(&g_deBuf);
	}

	delete[] m_vlist;
	delete[] m_tvlist;
	delete[] tnormal;
	delete[] m_texbuf;
	delete[] light;
	delete[] m_ibuf;
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
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
	cvSet(g_deBuf, cvScalar(0));

	IplImage* tmp=cvCreateImage(cvSize(m_Image->width,m_Image->height),IPL_DEPTH_8U,3);
	IplImage* tmp2=cvCreateImage(cvSize(m_Image->width,m_Image->height),IPL_DEPTH_8U,3);

	cvScale(m_ImgBk1,tmp,0.5);
	cvScale(m_ImgBk2,tmp2,0.5);
	cvAdd(tmp, tmp2, tmp);

	cvCopyImage(tmp, m_Image);

	cvReleaseImage(&tmp);
	cvReleaseImage(&tmp2);

	Vec2 v2[3]={Vec2(10,10),Vec2(100,10),Vec2(55,100)};
	Triangle2D(m_Image, v2);

	VertexProcess();
	if(m_bShowImage)
	{
		for(int i=0;i<(int)nFace;i++)
		{
			for(int j=0;j<3;j++)
			{
				m_vbuf[j] = m_tvlist[m_ibuf[i*3+j]];
				m_vdata[0][j] = light[m_ibuf[i*3+j]];
				m_vdata[1][j] = 1./m_tvlist[m_ibuf[i*3+j]].z;
				m_vdata[2][j] = m_texbuf[m_ibuf[i*3+j]].x;
				m_vdata[3][j] = m_texbuf[m_ibuf[i*3+j]].y;
			}
			Triangle3D(m_Image, m_vbuf, 4, m_vdata, Pixel);
		}
	}

	if(m_bShowVert)
	{
		for(int i=0;i<(int)nFace;i++)
		{
			for(int j=0;j<3;j++)
			{
				CvPoint pt1=cvPoint(m_tvlist[m_ibuf[i*3+j]].x*m_Image->height/2+m_Image->width/2,
					-m_tvlist[m_ibuf[i*3+j]].y*m_Image->height/2+m_Image->height/2);
				CvPoint pt2=cvPoint(m_tvlist[m_ibuf[i*3+(j+1)%3]].x*m_Image->height/2+m_Image->width/2
					,-m_tvlist[m_ibuf[i*3+(j+1)%3]].y*m_Image->height/2+m_Image->height/2);
				if(m_tvlist[m_ibuf[i*3+j]].z>0 && m_tvlist[m_ibuf[i*3+(j+1)%3]].z>0)
					DrawLine(pt1,pt2,cvScalar(255,0,0));
			}
		}
	}

	DispImage(&m_mdc, m_Image, CPoint(0,0));
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

void CChildView::DrawLine(CvPoint pt1,CvPoint pt2,CvScalar color)
{
	if(pt1.x==pt2.x&&pt1.y==pt2.y)
		return;
	int offx=pt2.x-pt1.x;
	int offy=pt2.y-pt1.y;
	int x,y;
	int mx=m_Image->width,my=m_Image->height;
	int*p1,*p2;
	int*pe1,*pe2;
	int*pm1,*pm2;
	int*ps1,*ps2;
	float asc;

	if(abs(offx)>=abs(offy))
	{
		p1=&x,p2=&y;
		pe1=&pt2.x,pe2=&pt2.y;
		ps1=&pt1.x,ps2=&pt1.y;
		pm1=&mx,pm2=&my;
		asc=(float)offy/offx;
	}
	else
	{
		p1=&y,p2=&x;
		pe1=&pt2.y,pe2=&pt2.x;
		ps1=&pt1.y,ps2=&pt1.x;
		pm1=&my,pm2=&mx;
		asc=(float)offx/offy;
	}
	for(*p1=max(0,min(*ps1,*pe1));*p1<min(*pm1,max(*ps1,*pe1));(*p1)++)
	{
		*p2=*ps2+asc*(*p1-*ps1);
		if(*p2>=0 && *p2<*pm2)
		{
			*PTR_PIX(*m_Image,x,y)=color.val[0];
			*(PTR_PIX(*m_Image,x,y)+1)=color.val[1];
			*(PTR_PIX(*m_Image,x,y)+2)=color.val[2];
		}
	}
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	HDC ddc = CreateDC("DISPLAY",NULL,NULL,NULL);
	DeleteDC(ddc);

	g_ImgBk1 = m_ImgBk1 = cvLoadImage("peaceb.jpg");
	g_ImgBk2 = m_ImgBk2 = cvLoadImage("peacew.jpg");

	IMAGE_WIDTH=m_ImgBk1->width;
	IMAGE_HEIGHT=m_ImgBk1->height;

	m_Image = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT),IPL_DEPTH_8U,3);
	g_deBuf = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT),IPL_DEPTH_32F,1);

	CClientDC dc(this);
	m_mdc.CreateCompatibleDC(&dc);
	m_bitmap.CreateCompatibleBitmap(&dc, IMAGE_WIDTH, IMAGE_HEIGHT);
	oldbmp = m_mdc.SelectObject(&m_bitmap);

	//::SetWindowPos(m_hWnd, HWND_TOPMOST, 0,0,IMAGE_WIDTH,IMAGE_HEIGHT,SWP_NOMOVE);
	MoveWindow(100,100,IMAGE_WIDTH,IMAGE_HEIGHT);

	CFile mesh;
	mesh.Open("teapot.mesh", CFile::modeRead);
	mesh.Read(&nVert, 4);
	mesh.Read(&nVert, 4);
	mesh.Read(&nVert, 4);
	m_vlist = new vertex[nVert];
	m_tvlist = new Vec3[nVert];
	tnormal = new Vec3[nVert];
	light = new float[nVert];
	m_texbuf = new Vec2[nVert];
	DWORD deb=nVert*sizeof(vertex);
	DWORD deb2=mesh.Read(m_vlist, nVert*sizeof(vertex));
	mesh.Read(&nFace, 4);
	m_ibuf = new WORD[nFace*3];
	mesh.Read(m_ibuf, nFace*3*sizeof(WORD));
	mesh.Close();

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

void CChildView::VertexProcess()
{
	for(int i=0;i<(int)nVert;i++)
	{
		Vec3 v(m_vlist[i].x,m_vlist[i].y,m_vlist[i].z);
		Vec3 n(m_vlist[i].nx,m_vlist[i].ny,m_vlist[i].nz);
		Vec3 tv = v*rot;
		Vec3 tn = n*rot;

		tv-=Pos;
		m_tvlist[i]=Vec3(tv.x/tv.z, tv.y/tv.z, 1./tv.z);
		tnormal[i]=tn;
		float d=dot(tn,tv);
		light[i]=-d/tv.length();
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
		m_texbuf[i].x=x;
		m_texbuf[i].y=y;
	}
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if(nChar == VK_SPACE)
	{
		m_bShowVert = !m_bShowVert;
		Draw();
	}
	else if(nChar == 'K')
	{
		m_bShowImage = !m_bShowImage;
		Draw();
	}
	else if(nChar == 'A')
	{
		Pos.x-=0.05;
		Draw();
	}
	else if(nChar == 'D')
	{
		Pos.x+=0.05;
		Draw();
	}
	else if(nChar == 'W')
	{
		Pos.z+=0.05;
		Draw();
	}
	else if(nChar == 'S')
	{
		Pos.z-=0.05;
		Draw();
	}
	else if(nChar == 'Q')
	{
		Pos.y-=0.05;
		Draw();
	}
	else if(nChar == 'E')
	{
		Pos.y+=0.05;
		Draw();
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SetCapture();
	m_bDown = true;
	m_oldPt = point;
	CRect rcClt;
	GetClientRect(rcClt);
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
	CRect rcClt;
	GetClientRect(rcClt);
	if(m_bDown)
	{
		CPoint offset=point-m_oldPt;
		float HAngle = (float)offset.x/100;
		float VAngle = (float)offset.y/100;
		rotx = Mat(cos(HAngle),0,sin(HAngle),0,1,0,-sin(HAngle),0,cos(HAngle));
		roty = Mat(1,0,0,0,cos(VAngle),-sin(VAngle),0,sin(VAngle),cos(VAngle));
		rot = rot*rotx*roty;
		m_oldPt = point;
		Draw();
	}
	CWnd::OnMouseMove(nFlags, point);
}

float CChildView::FrameFunc(float x, float edge)
{
	if(x>=edge && x<1-edge)
	{
		return 1;
	}
	else
	{
		if(x<edge)
		{
			x-=edge;
		}
		else
		{
			x-=1-edge;
		}
		return (cos(x/edge*PI)+1)/2;
	}
}