
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

Vec3 Pos(0,0,0);
Mat rot(1,0,0,0,1,0,0,0,1);
IplImage* g_texture = NULL;
IplImage* g_mask = NULL;
IplImage* g_ImgBk1 = NULL;
IplImage* g_ImgBk2 = NULL;
void Pixel(RawImage* target,Point2D pos, int count, float* pdata)
{
	Vec2 tex(pdata[0],pdata[1]);
	CvScalar pSamp = Sample(g_texture, tex);
	float lu = (pSamp.val[0]+pSamp.val[1]+pSamp.val[2])/3;
	float pMask = Sample(g_mask, tex).val[0]/255;
	float lerp1=(pSamp.val[0]-128)*pMask/255+0.5;
	float lerp2=(pSamp.val[1]-128)*pMask/255+0.5;
	float lerp3=(pSamp.val[2]-128)*pMask/255+0.5;
	uchar* ptr=PTR_PIX(*target, pos.x, pos.y);
	uchar* ptrbk1=PTR_PIX(*g_ImgBk1, pos.x, pos.y);
	uchar* ptrbk2=PTR_PIX(*g_ImgBk2, pos.x, pos.y);
	*ptr    = *ptrbk1+(*ptrbk2-*ptrbk1)*lerp1;//pSamp.val[0];//(float)(lu-128)/256;
	*(ptr+1)= *(ptrbk1+1)+(*(ptrbk2+1)-*(ptrbk1+1))*lerp2;//pSamp.val[1];//(float)(lu-128)/256;
	*(ptr+2)= *(ptrbk1+2)+(*(ptrbk2+2)-*(ptrbk1+2))*lerp3;//pSamp.val[2];//(float)(lu-128)/256;
}


CChildView::CChildView()
{
	m_bShowVert = false;
	m_bShowImage = true;
	m_bDown = false;

	m_Image = NULL;
	m_ImgBk1 = NULL;
	m_ImgBk2 = NULL;
	m_mask = NULL;
	texture = NULL;
	oldbmp = NULL;

	m_vlist[0] = Vec3(-1,-1,3);
	m_vlist[1] = Vec3( 1,-1,3);
	m_vlist[2] = Vec3(-1, 1,3);
	m_vlist[3] = Vec3( 1, 1,3);

	m_tclist[0] = Vec2(0,0);
	m_tclist[1] = Vec2(1,0);
	m_tclist[2] = Vec2(0,1);
	m_tclist[3] = Vec2(1,1);

	m_ibuf[0]= 0;
	m_ibuf[1]= 2;
	m_ibuf[2]= 3;
	m_ibuf[3]= 3;
	m_ibuf[4]= 1;
	m_ibuf[5]= 0;

	m_nSel = -1;

}

CChildView::~CChildView()
{
	if(m_Image != NULL)
	{
		cvReleaseImage(&m_Image);
	}
	if(texture != NULL)
	{
		cvReleaseImage(&texture);
	}
	if(m_ImgBk1 != NULL)
	{
		cvReleaseImage(&m_ImgBk1);
	}
	if(m_ImgBk2 != NULL)
	{
		cvReleaseImage(&m_ImgBk2);
	}
	if(m_mask != NULL)
	{
		cvReleaseImage(&m_mask);
	}
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

	IplImage* tmp=cvCreateImage(cvSize(m_Image->width,m_Image->height),IPL_DEPTH_8U,3);
	IplImage* tmp2=cvCreateImage(cvSize(m_Image->width,m_Image->height),IPL_DEPTH_8U,3);

	cvScale(m_ImgBk1,tmp,0.5);
	cvScale(m_ImgBk2,tmp2,0.5);
	cvAdd(tmp, tmp2, tmp);

	cvCopyImage(tmp, m_Image);

	cvReleaseImage(&tmp);
	cvReleaseImage(&tmp2);

	VertexProcess();
	if(m_bShowImage)
	{
		for(int i=0;i<2;i++)
		{
			for(int j=0;j<3;j++)
			{
				m_vbuf[j] = m_tvlist[m_ibuf[i*3+j]];
				m_texbuf[0][j] = m_tclist[m_ibuf[i*3+j]].x;
				m_texbuf[1][j] = m_tclist[m_ibuf[i*3+j]].y;
			}
			Rect2D szVp(0,0,m_Image->width,m_Image->height);
			Triangle3D(m_Image, m_vbuf, 2, m_texbuf, Pixel, &szVp);
		}
	}


	if(m_bShowVert)
	{
		for(int i=0;i<4;i++)
		{
			if(m_tvlist[i].z>0)
				cvCircle(m_Image, cvPoint(m_tvlist[i].x*m_Image->height/2+m_Image->width/2,
				-m_tvlist[i].y*m_Image->height/2+m_Image->height/2), 3, cvScalar(255,0,255), -1);
		}
		int index[4][2]={{0,1},{2,3},{0,2},{1,3}};
		for(int i=0;i<4;i++)
		{
			CvPoint pt1=cvPoint(m_tvlist[index[i][0]].x*m_Image->height/2+m_Image->width/2,
				-m_tvlist[index[i][0]].y*m_Image->height/2+m_Image->height/2);
			CvPoint pt2=cvPoint(m_tvlist[index[i][1]].x*m_Image->height/2+m_Image->width/2,
				-m_tvlist[index[i][1]].y*m_Image->height/2+m_Image->height/2);
			DrawLine(pt1,pt2,i<2?cvScalar(0,0,255):cvScalar(255,0,0));
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

	texture = cvLoadImage("1.jpg");
	g_texture = texture;
	g_ImgBk1 = m_ImgBk1 = cvLoadImage("peaceb.jpg");
	g_ImgBk2 = m_ImgBk2 = cvLoadImage("peacew.jpg");

	IMAGE_WIDTH=m_ImgBk1->width;
	IMAGE_HEIGHT=m_ImgBk1->height;

	m_Image = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT),IPL_DEPTH_8U,3);
	m_mask=cvCreateImage(cvSize(texture->width,texture->height),IPL_DEPTH_8U,1);
	g_mask = m_mask;
	for(int i=0;i<texture->height;i++)
	{
		for(int j=0;j<texture->width;j++)
		{
			uchar* ppix=PTR_PIX(*m_mask,j,i);
			float scale = FrameFunc((float)i/texture->height, 0.1)*FrameFunc((float)j/texture->width, 0.1);//(cos((float)(i-texture->height/2)/texture->height*2*PI)+1)/2
			*ppix=*(ppix+1)=*(ppix+2)=scale*255;
		}
	}

	// TODO:  Add your specialized creation code here
	CClientDC dc(this);
	m_mdc.CreateCompatibleDC(&dc);
	m_bitmap.CreateCompatibleBitmap(&dc, IMAGE_WIDTH, IMAGE_HEIGHT);
	oldbmp = m_mdc.SelectObject(&m_bitmap);

	//::SetWindowPos(m_hWnd, HWND_TOPMOST, 0,0,IMAGE_WIDTH,IMAGE_HEIGHT,SWP_NOMOVE);
	CFile file;
	if(file.Open("GeoData",CFile::modeRead))
	{
		for(int i=0;i<2;i++)
		{
			Point2D pt;
			file.Read((void*)&pt, sizeof(pt));
			float tx1=(pt.x-(float)m_Image->width/2)/(m_Image->height/2);
			float ty1=-(pt.y-(float)m_Image->height/2)/(m_Image->height/2);
			file.Read((void*)&pt, sizeof(pt));
			float tx2=(pt.x-(float)m_Image->width/2)/(m_Image->height/2);
			float ty2=-(pt.y-(float)m_Image->height/2)/(m_Image->height/2);
			float z=GEO_WIDTH/fabs(tx1-tx2);
			m_vlist[i*2].x=tx1*z;
			m_vlist[i*2].y=ty1*z;
			m_vlist[i*2].z=z;
			m_vlist[i*2+1].x=tx2*z;
			m_vlist[i*2+1].y=ty2*z;
			m_vlist[i*2+1].z=z;
		}
		file.Close();
	}
	MoveWindow(100,100,IMAGE_WIDTH,IMAGE_HEIGHT);
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
	for(int i=0;i<4;i++)
	{
		Vec3 vert = m_vlist[i];
		Vec3 tvec = vert*rot;
		tvec-=Pos;
		m_tvlist[i]=Vec3(tvec.x/tvec.z,tvec.y/tvec.z,1./tvec.z);
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
	else if(nChar == 'S')
	{
		if(GetAsyncKeyState(VK_CONTROL)&0x8000)
		{
			cvSaveImage("output.jpg", m_Image);
		}
		else
		{
			CFile file;
			file.Open("GeoData",CFile::modeCreate|CFile::modeWrite);
			for(int i=0;i<4;i++)
			{
				Point2D pt(m_tvlist[i].x*m_Image->height/2+m_Image->width/2
					,m_tvlist[i].y*m_Image->height/2+m_Image->height/2);
				file.Write((void*)&pt, sizeof(pt));
			}
			file.Close();
		}
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SetCapture();
	m_bDown = true;

	CRect rcClt;
	GetClientRect(rcClt);
	bool bHit=false;
	for(int i=0;i<4;i++)
	{
		CRect rcHit(-3,-3,3,3);
		CPoint ptVert(m_tvlist[i].x*m_Image->height/2+m_Image->width/2,
			m_tvlist[i].y*m_Image->height/2+m_Image->height/2);
		rcHit+=ptVert;
		if(rcHit.PtInRect(point))
		{
			bHit = true;
			m_nSel = i;
		}
	}
	if(!bHit)
	{
		m_nSel=-1;
	}
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
	if(m_bDown && m_nSel != -1 && m_bShowVert)
	{
		float tx=(point.x-(float)m_Image->width/2)/(m_Image->height/2);
		float ty=-(point.y-(float)m_Image->height/2)/(m_Image->height/2);
		float bw=fabs(tx-m_tvlist[m_nSel^1].x);
		float nz=GEO_WIDTH/bw;

		m_vlist[m_nSel].z=m_vlist[m_nSel^1].z=nz;
		m_vlist[m_nSel].x=nz*tx;
		m_vlist[m_nSel^1].x=m_tvlist[m_nSel^1].x*nz;
		m_vlist[m_nSel].y=m_vlist[m_nSel^1].y=nz*ty;
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