
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "PhotoFrame.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "shlobj.h"
#define PI 3.1415926535897932384626
// CChildView
int IMAGE_WIDTH=0;
int IMAGE_HEIGHT=0;
float GEO_WIDTH=2;
float GEO_HEIGHT=2;

Vec3 Pos(0,0,0);
Mat rot(1,0,0,0,1,0,0,0,1);
IplImage* g_texture = NULL;
//IplImage* g_mask = NULL;
IplImage* g_ImgBk1 = NULL;
IplImage* g_ImgBk2 = NULL;

char* g_CfgFileName = "peace";

CChildView* g_pWnd = NULL;

void Pixel(RawImage* target,Point2D pos, int count, float* pdata)
{
	Vec2 tex(pdata[0],pdata[1]);
	Vec3 pSamp;
	if(g_texture)
		pSamp = Sample(g_texture, tex);
	float gray = (pSamp.x+pSamp.y+pSamp.z)/3/255;
	//float pMask = Sample(g_mask, tex).val[0]/255;
	float pMask = FrameFunc(tex.x, g_pWnd->m_fEdge)*FrameFunc(tex.y, g_pWnd->m_fEdge)*g_pWnd->m_fAlpha;
	uchar* ptr=PTR_PIX(*target, pos.x, pos.y);
	if(!g_ImgBk1)
	{
		*ptr = 0;
		*(ptr+1) = 0;
		*(ptr+2) = 0;
	}
	else
	{
		uchar* ptrbk1=PTR_PIX(*g_ImgBk1, pos.x, pos.y);
		Vec3 cbk1(*ptrbk1, *(ptrbk1+1), *(ptrbk1+2));
		if(g_pWnd->m_bGray)
		{
			if(!g_ImgBk2)
			{
				gray = 0.5+pMask*(gray-0.5);
				if(gray<0.5)
				{
					gray = gray*2;
					pSamp = cbk1*gray;
				}
				else
				{
					gray = (1-gray)*2;
					Vec3 top(255, 255, 255);
					pSamp = top - (top - cbk1) * gray;
				}
			}
			else
			{
				uchar* ptrbk2 = PTR_PIX(*g_ImgBk2, pos.x, pos.y);
				Vec3 cbk2(*ptrbk2, *(ptrbk2+1), *(ptrbk2+2));
				pSamp = cbk2*gray+cbk1*(1-gray);
			}
		}
		else
		{
			if(!g_ImgBk2)
			{
				pSamp = pSamp*pMask+cbk1*(1-pMask);
			}
			else
			{
				uchar* ptrbk2=PTR_PIX(*g_ImgBk2, pos.x, pos.y);
				Vec3 cbk2(*ptrbk2, *(ptrbk2+1), *(ptrbk2+2));
				pSamp = Vec3(128,128,128) + pMask * (pSamp - Vec3(128,128,128));
				pSamp=cbk2*pSamp/255+cbk1*(Vec3(1,1,1)-pSamp/255);
			}
		}
	}
	uchar* pix=PTR_PIX(*target, pos.x, pos.y);
	*pix=pSamp.x;
	*(pix+1)=pSamp.y;
	*(pix+2)=pSamp.z;
}


CChildView::CChildView():m_strWorkingPath(_T("D:"))
{
	m_bShowVert = false;
	m_bShowImage = true;
	m_bDown = false;
	m_bPackage = FALSE;

	m_Image = NULL;
	m_ImgBk1 = NULL;
	m_ImgBk2 = NULL;
	m_mask = NULL;
	texture = NULL;
	oldbmp = NULL;

	InitQuad(m_quad);
	m_nTopmost = 0;

	m_strBk1 = "peaceb.jpg";
	m_strBk2 = "peacew.jpg";

	m_strBkTitle1 = "peaceb.jpg";
	m_strBkTitle2 = "peacew.jpg";

	m_strSrc = "1.jpg";

	m_strCfg = "peace.cfg";
	m_strCfgBin = "peace";

	m_orgWidth = 0;
	m_orgHeight = 0;

	m_bUseDouble = false;
	m_bUseMask = false;

	m_fEdge = 0;
	m_fAlpha = 1;
	m_bGray = FALSE;
	TCHAR buf[MAX_PATH];
	SHGetSpecialFolderPath(NULL, buf, CSIDL_DESKTOP, TRUE);
	m_strWorkingPath = buf;
	ZeroMemory(&m_guid, sizeof(GUID));
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
	/*if(m_mask != NULL)
	{
		cvReleaseImage(&m_mask);
	}*/
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_FILE_OPEN, &CChildView::OnFileOpen)
	ON_COMMAND(ID_MENU_SETTINGS, &CChildView::OnMenuSettings)
	ON_MESSAGE(WM_NOTIFY_GENERATE_CONFIG_FILE, OnGenerateTempl)
	ON_MESSAGE(WM_NOTIFY_EXPORT_CONFIG_FILE, OnExportTempl)
	ON_MESSAGE(WM_NOTIFY_APPLY_SETTINGS, OnApplySettings)
	ON_MESSAGE(WM_NOTIFY_EXPORT_PAKAGE, OnExportPakage)
	ON_COMMAND(ID_FILE_ADDQUAD, &CChildView::OnFileAddquad)
	ON_COMMAND(ID_FILE_RESETQUAD, &CChildView::OnFileResetquad)
	ON_WM_LBUTTONDBLCLK()
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

	CRect rcClient;
	GetClientRect(rcClient);
	m_mdc.FillSolidRect(rcClient, RGB(0,0,0));
	CRect rcClt;
	GetClientRect(rcClt);

	cvSet(m_Image, cvScalarAll(0));

	if(m_ImgBk1)
	{
		if(m_ImgBk2)
		{
			IplImage* tmp=cvCreateImage(cvSize(m_Image->width,m_Image->height),IPL_DEPTH_8U,3);
			IplImage* tmp2=cvCreateImage(cvSize(m_Image->width,m_Image->height),IPL_DEPTH_8U,3);
			cvScale(m_ImgBk1,tmp,0.5);
			cvScale(m_ImgBk2,tmp2,0.5);
			cvAdd(tmp, tmp2, tmp);

			cvCopyImage(tmp, m_Image);

			cvReleaseImage(&tmp);
			cvReleaseImage(&tmp2);
		}
		else
		{
			cvCopyImage(m_ImgBk1, m_Image);
		}
	}

	if(m_nTopmost>(int)m_quadextra.size())
		m_nTopmost=m_quadextra.size();
	for(int i=0;i<(int)m_quadextra.size()+1;i++)
	{
		if(i==m_nTopmost)
			continue;
		DrawQuad(i==0?m_quad:m_quadextra[i-1]);
	}
	DrawQuad(m_nTopmost==0?m_quad:m_quadextra[m_nTopmost-1]);

	DispImage(&m_mdc, m_Image, CPoint(0,0));
	dc.BitBlt(0,0,rcClt.Width(),rcClt.Height(),&m_mdc,0,0,SRCCOPY);
}

void CChildView::DrawQuad(Quad& quad)
{
	VertexProcess(quad);
	if(m_bShowImage)
	{
		for(int i=0;i<2;i++)
		{
			for(int j=0;j<3;j++)
			{
				quad.m_vbuf[j] = quad.m_tvlist[quad.m_ibuf[i*3+j]];
				quad.m_texbuf[0][j] = quad.m_tclist[quad.m_ibuf[i*3+j]].x;
				quad.m_texbuf[1][j] = quad.m_tclist[quad.m_ibuf[i*3+j]].y;
			}
			Rect2D szVp(0,0,m_Image->width,m_Image->height);
			g_pWnd = this;
			Triangle3D(m_Image, quad.m_vbuf, 2, quad.m_texbuf, Pixel, &szVp);
			g_pWnd = NULL;
		}
	}


	if(m_bShowVert)
	{
		for(int i=0;i<4;i++)
		{
			if(quad.m_tvlist[i].z>0)
				cvCircle(m_Image, cvPoint(quad.m_tvlist[i].x*m_Image->height/2+m_Image->width/2,
				-quad.m_tvlist[i].y*m_Image->height/2+m_Image->height/2), 3, cvScalar(255,0,255), -1);
		}
		int index[4][2]={{0,1},{2,3},{0,2},{1,3}};
		for(int i=0;i<4;i++)
		{
			CvPoint pt1=cvPoint(quad.m_tvlist[index[i][0]].x*m_Image->height/2+m_Image->width/2,
				-quad.m_tvlist[index[i][0]].y*m_Image->height/2+m_Image->height/2);
			CvPoint pt2=cvPoint(quad.m_tvlist[index[i][1]].x*m_Image->height/2+m_Image->width/2,
				-quad.m_tvlist[index[i][1]].y*m_Image->height/2+m_Image->height/2);
			DrawLine(pt1,pt2,i<2?cvScalar(0,0,255):cvScalar(255,0,0));
		}
	}
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
	CClientDC dc(this);
	m_mdc.CreateCompatibleDC(&dc);
	Init();
/*	g_texture = texture = cvLoadImage(m_strSrc);
	g_ImgBk1 = m_ImgBk1 = cvLoadImage(m_strBk1);
	g_ImgBk2 = m_ImgBk2 = cvLoadImage(m_strBk2);

	IMAGE_WIDTH=m_ImgBk1->width;
	IMAGE_HEIGHT=m_ImgBk1->height;

	m_Image = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT),IPL_DEPTH_8U,3);
	g_mask = m_mask=cvCreateImage(cvSize(texture->width,texture->height),IPL_DEPTH_8U,1);

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
	if(file.Open(m_strCfgBin,CFile::modeRead))
	{
		for(int i=0;i<2;i++)
		{
			Point2D pt;
			file.Read((void*)&pt, sizeof(pt));
			float tx1=(pt.x-(float)m_Image->width/2)/(m_Image->height/2);
			float ty1=-(pt.y-(float)m_Image->height/2)/(m_Image->height/2);
			m_vlist2d[i*2]=Vec2(tx1,ty1);
			file.Read((void*)&pt, sizeof(pt));
			float tx2=(pt.x-(float)m_Image->width/2)/(m_Image->height/2);
			float ty2=-(pt.y-(float)m_Image->height/2)/(m_Image->height/2);
			m_vlist2d[i*2+1]=Vec2(tx2,ty2);
		}
		file.Close();
	}
	Reform(m_vlist2d, m_vlist);*/
	MoveWindow(100,100,IMAGE_WIDTH,IMAGE_HEIGHT);
/*	IplImage* black = cvLoadImage("underwaterb.jpg");
	IplImage* white = cvLoadImage("underwaterw.jpg");
	cvScale(black, black, 0.5);
	cvScale(white, white, 0.5);
	cvAdd(black, white, white);
	cvSaveImage("underwater.jpg", white);*/
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

void CChildView::VertexProcess(Quad& quad)
{
	for(int i=0;i<4;i++)
	{
		Vec3 vert = quad.m_vlist[i];
		Vec3 tvec = vert*rot;
		tvec-=Pos;
		quad.m_tvlist[i]=Vec3(tvec.x/tvec.z,tvec.y/tvec.z,1./tvec.z);
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
			SaveLocalData();
		}
	}
	else if(nChar == 'D')
	{
		CFile file;
		CString strFileName(m_strCfg);
		file.Open(strFileName, CFile::modeCreate|CFile::modeWrite);
		CString strOut, str;
		for(int n=0;n<(int)m_quadextra.size()+1;n++)
		{
			Quad& quad=(n==0?m_quad:m_quadextra[n-1]);
			for(int i=0;i<4;i++)
			{
				//	str.Format("%d,%d\r\n", (int)(m_tvlist[i].x*m_Image->height/2+m_Image->width/2),
				//		(int)(-m_tvlist[i].y*m_Image->height/2+m_Image->height/2));
				str.Format("%d,%d\r\n", (int)(quad.m_tvlist[i].x*m_orgHeight/2+m_orgWidth/2),
					(int)(-quad.m_tvlist[i].y*m_orgHeight/2+m_orgHeight/2));
				strOut+=str;
			}
		}
		char* s = (LPSTR)(LPCTSTR)strOut;
		file.Write(s, strlen(s));
		file.Close();
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChildView::SaveLocalData()
{
	CFile file;
	file.Open(m_strCfgBin,CFile::modeCreate|CFile::modeWrite);

	for(int n=0;n<(int)m_quadextra.size()+1;n++)
	{
		Quad& quad=(n==0?m_quad:m_quadextra[n-1]);
		for(int i=0;i<4;i++)
		{
			//	Point2D pt(m_tvlist[i].x*m_Image->height/2+m_Image->width/2
		//		,m_tvlist[i].y*m_Image->height/2+m_Image->height/2);
			Point2D pt(quad.m_tvlist[i].x*m_orgHeight/2+m_orgWidth/2
				,-quad.m_tvlist[i].y*m_orgHeight/2+m_orgHeight/2);

			file.Write((void*)&pt, sizeof(pt));
		}
	}
	file.Close();
}

bool CChildView::HitTest(Quad& quad, POINT point)
{
	bool bHit=false;
	for(int i=0;i<4;i++)
	{
		CRect rcHit(-3,-3,3,3);
		CPoint ptVert(quad.m_tvlist[i].x*m_Image->height/2+m_Image->width/2,
			-quad.m_tvlist[i].y*m_Image->height/2+m_Image->height/2);
		rcHit+=ptVert;
		if(rcHit.PtInRect(point))
		{
			bHit = true;
			quad.m_nSel = i;
		}
	}
	if(!bHit)
	{
		quad.m_nSel=-1;
	}
	return quad.m_nSel!=-1;
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SetCapture();
	m_bDown = true;
	int topmost=m_nTopmost;
	if(!HitTest(m_nTopmost==0?m_quad:m_quadextra[m_nTopmost-1],point))
	for(int i=0;i<(int)m_quadextra.size()+1;i++)
	{
		if(i==topmost)
			continue;
		if(HitTest(i==0?m_quad:m_quadextra[i-1],point))
			m_nTopmost=i;
	}
	Draw();
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
	Quad& quad=(m_nTopmost==0?m_quad:m_quadextra[m_nTopmost-1]);
	if(m_bDown && quad.m_nSel!=-1 && m_bShowVert)
	{
		float tx=(point.x-(float)m_Image->width/2)/(m_Image->height/2);
		float ty=-(point.y-(float)m_Image->height/2)/(m_Image->height/2);
		quad.m_vlist2d[quad.m_nSel]=Vec2(tx,ty);
		Reform(quad.m_vlist2d, quad.m_vlist);
/*
		float bw=fabs(tx-m_tvlist[m_nSel^1].x);
		float nz=GEO_WIDTH/bw;

		m_vlist[m_nSel].z=m_vlist[m_nSel^1].z=nz;
		m_vlist[m_nSel].x=nz*tx;
		m_vlist[m_nSel^1].x=m_tvlist[m_nSel^1].x*nz;
		m_vlist[m_nSel].y=m_vlist[m_nSel^1].y=nz*ty;
*/
		Draw();
	}
	CWnd::OnMouseMove(nFlags, point);
}

float FrameFunc(float x, float edge)
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

void CChildView::Reform(Vec2 vertin[4], Vec3 vertout[4])
{
	float mat1[2][2]={{vertin[1].y-vertin[0].y, -(vertin[1].x-vertin[0].x)}, {vertin[3].y-vertin[2].y, -(vertin[3].x-vertin[2].x)}};
	float vec1[2]   ={vertin[0].x*(vertin[1].y-vertin[0].y)-vertin[0].y*(vertin[1].x-vertin[0].x), vertin[2].x*(vertin[3].y-vertin[2].y)-vertin[2].y*(vertin[3].x-vertin[2].x)};

	float mat2[2][2]={{vertin[2].y-vertin[0].y, -(vertin[2].x-vertin[0].x)}, {vertin[3].y-vertin[1].y, -(vertin[3].x-vertin[1].x)}};
	float vec2[2]   ={vertin[0].x*(vertin[2].y-vertin[0].y)-vertin[0].y*(vertin[2].x-vertin[0].x), vertin[1].x*(vertin[3].y-vertin[1].y)-vertin[1].y*(vertin[3].x-vertin[1].x)};

	float det1=mat1[0][0]*mat1[1][1]-mat1[0][1]*mat1[1][0];
	float x1=vec1[0]*mat1[1][1]-vec1[1]*mat1[0][1];
	float y1=mat1[0][0]*vec1[1]-mat1[1][0]*vec1[0];

	float det2=mat2[0][0]*mat2[1][1]-mat2[0][1]*mat2[1][0];
	float x2=vec2[0]*mat2[1][1]-vec2[1]*mat2[0][1];
	float y2=mat2[0][0]*vec2[1]-mat2[1][0]*vec2[0];

	Vec3 pole1,pole2;
	if(det1 != 0)
		pole1 = Vec3(x1/det1, y1/det1, 1);
	else
		pole1 = Vec3(x1,y1,0);
	if(det2 != 0)
		pole2 = Vec3(x2/det2, y2/det2, 1);
	else
		pole2 = Vec3(x2,y2,0);

	Vec3  v00(vertin[0].x, vertin[0].y, 1);
	Vec3  normal=cross(pole1, pole2);

	for(int i=0;i<4;i++)
	{
		Vec3 tv=Vec3(vertin[i].x, vertin[i].y, 1);
		float zv=dot(tv,normal);
		float zn=dot(v00,normal);
		float nv1=dot(-v00, pole1);
		float nv2=dot(-v00, pole2);
		float v1=dot(tv, pole1)*zn/zv;
		float v2=dot(tv, pole2)*zn/zv;
		v1+=nv1;
		v2+=nv2;
		Vec3 num(v1,v2,0);
		Mat  den(pole1,pole2,normal);
		Vec3 s=num*den.trans().inv();
		Vec3 v=s+v00;
		vertout[i]=v;
	}
}

void CChildView::OnFileOpen()
{
	// TODO: Add your command handler code here
	COpenDialog dlg;
	dlg.m_strSrc = m_strSrc;
	dlg.m_strBkGnd = m_strBk1;
	dlg.m_strBk2 = m_strBk2;
	if(dlg.DoModal() == IDOK)
	{
		m_strSrc = dlg.m_strSrc;
		m_strBk1 = dlg.m_strBkGnd;
		m_strBk2 = dlg.m_strBk2;
		m_bUseDouble = dlg.m_bUseDoubleImage;
		m_strBkTitle1 = m_strBk1.Right(m_strBk1.GetLength() - 1 - m_strBk1.ReverseFind(_T('\\')));
		m_strBkTitle2 = m_strBk2.Right(m_strBk2.GetLength() - 1 - m_strBk2.ReverseFind(_T('\\')));
		m_bUseMask = dlg.m_bUseMask;
		m_strMask = dlg.m_strMask;
		m_strMaskTitle = m_strMask.Right(m_strMask.GetLength() - 1 - m_strMask.ReverseFind(_T('\\')));
		Init();
	}

	m_strTemplName = _T("");
	ZeroMemory(&m_guid, sizeof(GUID));
	Draw();
}

void CChildView::Init()
{
	USES_CONVERSION;
	ResetQuad();
	if(m_strBk1 != "")
	{
		m_strCfgBin = m_strBk1.Left(m_strBk1.ReverseFind('.'));
		m_strCfg = m_strCfgBin + ".cfg";
		cvReleaseImage(&m_ImgBk1);
		g_ImgBk1 = m_ImgBk1 = cvLoadImage(m_strBk1);
	}
	else
	{
		m_strCfgBin = "";
		m_strCfg = "";
	}

	if(m_strSrc != "")
	{
		cvReleaseImage(&texture);
		g_texture = texture = cvLoadImage(m_strSrc);
	}

	cvReleaseImage(&m_ImgBk2);
	g_ImgBk2 = NULL;
	if(m_strBk2 != "" && m_bUseDouble)
		g_ImgBk2 = m_ImgBk2 = cvLoadImage(m_strBk2);
	cvReleaseImage(&m_mask);
	if(m_strMask != "" && m_bUseMask)
		m_mask = cvLoadImage(m_strMask);

	if(m_ImgBk1)
	{
		m_orgWidth = m_ImgBk1->width;
		m_orgHeight = m_ImgBk1->height;
		if(m_orgWidth <= 1024 && m_orgHeight <= 768)
		{
			IMAGE_WIDTH=m_orgWidth;
			IMAGE_HEIGHT=m_orgHeight;
		}
		else
		{
			float xscale = 1024./m_orgWidth;
			float yscale = 768./m_orgHeight;
			float scale = min(xscale, yscale);
			IMAGE_WIDTH = m_orgWidth * scale;
			IMAGE_HEIGHT = m_orgHeight * scale;
			IplImage* tmp = m_ImgBk1;
			g_ImgBk1 = m_ImgBk1 = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, 3);
			cvResize(tmp, m_ImgBk1);
			cvReleaseImage(&tmp);
		}
		if(m_ImgBk2 && !(m_ImgBk2->width == m_ImgBk1->width && m_ImgBk2->height == m_ImgBk1->height))
		{
			IplImage* tmp = m_ImgBk2;
			g_ImgBk2 = m_ImgBk2 = cvCreateImage(cvSize(IMAGE_WIDTH, IMAGE_HEIGHT), IPL_DEPTH_8U, 3);
			cvResize(tmp, m_ImgBk2);
			cvReleaseImage(&tmp);
		}
	}
	else
	{
		IMAGE_WIDTH = 800;
		IMAGE_HEIGHT = 600;
		m_orgWidth = 800;
		m_orgHeight = 600;
	}

	cvReleaseImage(&m_Image);
	m_Image = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT),IPL_DEPTH_8U,3);
/*	cvReleaseImage(&m_mask);
	g_mask = m_mask=cvCreateImage(cvSize(texture->width,texture->height),IPL_DEPTH_8U,1);

	for(int i=0;i<texture->height;i++)
	{
		for(int j=0;j<texture->width;j++)
		{
			uchar* ppix=PTR_PIX(*m_mask,j,i);
			float scale = FrameFunc((float)i/texture->height, 0.1)*FrameFunc((float)j/texture->width, 0.1);//(cos((float)(i-texture->height/2)/texture->height*2*PI)+1)/2
			*ppix=*(ppix+1)=*(ppix+2)=scale*255;
		}
	}
*/
	// TODO:  Add your specialized creation code here
	CClientDC dc(this);
	if(oldbmp != NULL)
	{
		m_mdc.SelectObject(oldbmp);
		m_bitmap.DeleteObject();
	}
	m_bitmap.CreateCompatibleBitmap(&dc, IMAGE_WIDTH, IMAGE_HEIGHT);
	oldbmp = m_mdc.SelectObject(&m_bitmap);

	//::SetWindowPos(m_hWnd, HWND_TOPMOST, 0,0,IMAGE_WIDTH,IMAGE_HEIGHT,SWP_NOMOVE);
	m_quad.m_vlist2d[0] = Vec2(-0.5,-0.5);
	m_quad.m_vlist2d[1] = Vec2( 0.5,-0.5);
	m_quad.m_vlist2d[2] = Vec2(-0.5, 0.5);
	m_quad.m_vlist2d[3] = Vec2( 0.5, 0.5);

	CFile file;
	if(file.Open(m_strCfgBin,CFile::modeRead))
	{
		int length=file.GetLength();
		ASSERT(length%(4*sizeof(Point2D))==0);
		int nquad=length/(4*sizeof(Point2D));
		for(int n=0;n<nquad;n++)
		{
			Quad quad;
			InitQuad(quad);
			for(int i=0;i<2;i++)
			{
				Point2D pt;
				file.Read((void*)&pt, sizeof(pt));
				float tx1=(pt.x-(float)m_orgWidth/2)/(m_orgHeight/2);
				float ty1=-(pt.y-(float)m_orgHeight/2)/(m_orgHeight/2);
				quad.m_vlist2d[i*2]=Vec2(tx1,ty1);
				file.Read((void*)&pt, sizeof(pt));
				float tx2=(pt.x-(float)m_orgWidth/2)/(m_orgHeight/2);
				float ty2=-(pt.y-(float)m_orgHeight/2)/(m_orgHeight/2);
				quad.m_vlist2d[i*2+1]=Vec2(tx2,ty2);
				/*
				float z=GEO_WIDTH/fabs(tx1-tx2);
				m_vlist[i*2].x=tx1*z;
				m_vlist[i*2].y=ty1*z;
				m_vlist[i*2].z=z;
				m_vlist[i*2+1].x=tx2*z;
				m_vlist[i*2+1].y=ty2*z;
				m_vlist[i*2+1].z=z;
				*/
			}
			Reform(quad.m_vlist2d, quad.m_vlist);

			if(n==0)
				m_quad=quad;
			else
				m_quadextra.push_back(quad);
		}
		file.Close();
	}
	if(file.Open("config", CFile::modeRead))
	{
		int len = file.GetLength();
		char* buf = new char[len+1];
		file.Read(buf, len);
		buf[len] = 0;
		m_strWorkingPath = A2T(buf);
		delete[] buf;
		file.Close();
	}
	CRect rcClient;
	GetClientRect(rcClient);
	dc.FillSolidRect(rcClient, RGB(255,255,255));
}

void CChildView::OnMenuSettings()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CDialogSettings dlg(this);
	dlg.m_strTemplName = m_strTemplName;
	dlg.m_strWorkingPath = m_strWorkingPath;
	dlg.m_strEdge.Format(_T("%4.2f"), m_fEdge);
	dlg.m_strAlpha.Format(_T("%4.2f"), m_fAlpha);
	dlg.m_bChkGray = m_bGray;
	if(IDOK == dlg.DoModal())
	{
		m_strWorkingPath = dlg.m_strWorkingPath;
		if(m_strWorkingPath.Right(1) == _T("\\"))
			m_strWorkingPath = m_strWorkingPath.Left(m_strWorkingPath.GetLength()-1);
		m_strTemplName = dlg.m_strTemplName;
		m_fEdge = atof(T2A((LPTSTR)(LPCTSTR)dlg.m_strEdge));
		m_fAlpha = atof(T2A((LPTSTR)(LPCTSTR)dlg.m_strAlpha));
		m_bGray = dlg.m_bChkGray;
		CFile file;
		if(file.Open("config", CFile::modeCreate|CFile::modeWrite))
		{
			char* str = T2A((LPTSTR)(LPCTSTR)m_strWorkingPath);
			file.Write(str, strlen(str));
			file.Close();
		}
		Draw();
	}
}

LRESULT CChildView::OnGenerateTempl(WPARAM wParam, LPARAM lParam)
{
	USES_CONVERSION;
	if (((CDialogSettings*)wParam)->m_strTemplName.IsEmpty())
	{
		return 3;
	}
	m_strTemplName = ((CDialogSettings*)wParam)->m_strTemplName;
	CoCreateGuid(&m_guid);
	TiXmlDocument DocTempl(T2A((LPTSTR)(LPCTSTR)(m_strTemplName+_T(".tmpl"))));

	TiXmlDocument* eleRoot = &DocTempl;
	TiXmlElement* eletmpls = new TiXmlElement("templates");
	TiXmlElement* eletmpl = new TiXmlElement("template");
	eletmpl->SetAttribute("name", T2A((LPTSTR)(LPCTSTR)m_strTemplName));
	eletmpl->SetAttribute("category", "frame");
	eletmpl->SetAttribute("guid", T2A((LPTSTR)(LPCTSTR)FormatGuid(m_guid)));


	TiXmlElement* eleFilter = new TiXmlElement("filter");
	eleFilter->SetAttribute("name", m_mask?"ModalCollageTemplate":"FrameTemplate");
	TiXmlElement* eleParam = new TiXmlElement("param");
	eleParam->SetDoubleAttribute("edge", m_fEdge);
	eleParam->SetDoubleAttribute("alpha", m_fAlpha);
	if(m_bGray)
		eleParam->SetAttribute("gray", "1");
	char pt[20];
	for(int i=0;i<2;i++)
	{
		for(int j=0;j<2;j++)
		{
			sprintf_s(pt, "point%d%d", i, j);
			TiXmlElement* elePt = new TiXmlElement(pt);
			int x = (int)(m_quad.m_tvlist[(1-i)*2+j].x*m_orgHeight/2+m_orgWidth/2);
			int y = (int)(-m_quad.m_tvlist[(1-i)*2+j].y*m_orgHeight/2+m_orgHeight/2);
			elePt->SetAttribute("x", x);
			elePt->SetAttribute("y", y);
			eleParam->LinkEndChild(elePt);
		}
	}
	if(m_mask)
	{		
		eleParam->SetDoubleAttribute("aspect", GetAsp(m_quad));
		TiXmlElement* eleClr=new TiXmlElement("maskclr");
		eleClr->SetDoubleAttribute("r", m_quad.m_clr.x);
		eleClr->SetDoubleAttribute("g", m_quad.m_clr.y);
		eleClr->SetDoubleAttribute("b", m_quad.m_clr.z);
		eleParam->LinkEndChild(eleClr);
	}
	for(int n=0;n<(int)m_quadextra.size();n++)
	{
		sprintf_s(pt, "extra%d", n);
		TiXmlElement* eleExtra = new TiXmlElement(pt);
		for(int i=0;i<2;i++)
		{
			for(int j=0;j<2;j++)
			{
				sprintf_s(pt, "point%d%d", i, j);
				TiXmlElement* elePt = new TiXmlElement(pt);
				int x = (int)(m_quadextra[n].m_tvlist[(1-i)*2+j].x*m_orgHeight/2+m_orgWidth/2);
				int y = (int)(-m_quadextra[n].m_tvlist[(1-i)*2+j].y*m_orgHeight/2+m_orgHeight/2);
				elePt->SetAttribute("x", x);
				elePt->SetAttribute("y", y);
				eleExtra->LinkEndChild(elePt);
			}
		}
		if(m_mask)
		{
			eleExtra->SetDoubleAttribute("aspect",GetAsp(m_quadextra[n]));
			TiXmlElement* eleClr=new TiXmlElement("maskclr");
			eleClr->SetDoubleAttribute("r", m_quadextra[n].m_clr.x);
			eleClr->SetDoubleAttribute("g", m_quadextra[n].m_clr.y);
			eleClr->SetDoubleAttribute("b", m_quadextra[n].m_clr.z);
			eleExtra->LinkEndChild(eleClr);
		}
		eleParam->LinkEndChild(eleExtra);
	}
	TiXmlElement* eleFilter2 = new TiXmlElement("filter");
	eleFilter2->SetAttribute("name", "FrameFilter");
	TiXmlElement* eleParam2 = new TiXmlElement("param");
	CString strBk = CString(_T(".\\media\\Images\\")) + m_strBkTitle1;
	eleParam2->SetAttribute("background", T2A((LPTSTR)(LPCTSTR)strBk));
	if(m_bUseDouble)
	{
		CString strBk2 = CString(_T(".\\media\\Images\\")) + m_strBkTitle2;
		eleParam2->SetAttribute("background2", T2A((LPTSTR)(LPCTSTR)strBk2));
	}
	if(m_bUseMask)
	{
		CString strMask = CString(_T(".\\media\\Images\\")) + m_strMaskTitle;
		eleParam2->SetAttribute("mask", T2A((LPTSTR)(LPCTSTR)strMask));
	}
	eleFilter2->LinkEndChild(eleParam2);
	eleFilter->LinkEndChild(eleParam);
	eleFilter->LinkEndChild(eleFilter2);

	eletmpls->LinkEndChild(eletmpl);
	eletmpls->LinkEndChild(eleFilter);

	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "UTF-8", "");

	eleRoot->LinkEndChild(decl);
	eleRoot->LinkEndChild(eletmpls);

	SaveLocalData();

	if(DocTempl.SaveFile(T2A((LPTSTR)(LPCTSTR)(m_strTemplName+_T(".tmpl")))))
		return S_OK;
	else
		return S_FALSE;
}

LRESULT CChildView::OnExportTempl(WPARAM wParam, LPARAM lParam)
{
	USES_CONVERSION;
	CString strTemplFileName = m_strTemplName+_T(".tmpl");
	if(m_strTemplName.IsEmpty()||!PathFileExists(strTemplFileName))
	{
		LRESULT lRet = OnGenerateTempl(wParam, lParam);
		if(lRet != S_OK)
		{
			return lRet;
		}
		strTemplFileName = m_strTemplName+_T(".tmpl");
	}

	CString strTemplFileNameDest = m_strWorkingPath + _T("\\Templates\\") + m_strTemplName+_T(".tmpl");
	if(PathFileExists(strTemplFileNameDest))
	{
		CString strErr;
		strErr.Format(_T("The file \"%s\" has already existed, overwrite it?"), strTemplFileNameDest);
		if(IDCANCEL == MessageBox(strErr, _T("Overwrite files"), MB_OKCANCEL))
			return 2;
	}
	CopyFile(strTemplFileName, strTemplFileNameDest, FALSE);
	
	CString strBk1 = m_strWorkingPath  + _T("\\Images\\") +m_strBkTitle1;

	if(PathFileExists(strBk1))
	{
		CString strErr;
		strErr.Format(_T("The file \"%s\" has already existed, overwrite it?"), strBk1);
		if(IDCANCEL == MessageBox(strErr, _T("Overwrite files"), MB_OKCANCEL))
			return 2;
	}
	CopyFile(m_strBk1, strBk1, FALSE);

	if(m_bUseDouble)
	{
		CString strBk2 = m_strWorkingPath + _T("\\Images\\") + m_strBkTitle2;
		if(PathFileExists(strBk2))
		{
			CString strErr;
			strErr.Format(_T("The file \"%s\" has already existed, overwrite it?"), strBk2);
			if(IDCANCEL == MessageBox(strErr, _T("Overwrite files"), MB_OKCANCEL))
				return 2;
		}
		CopyFile(m_strBk2, strBk2, FALSE);
	}
	if(m_bUseMask)
	{
		CString strMask=m_strWorkingPath+_T("\\Images\\")+m_strMaskTitle;
		if(PathFileExists(strMask))
		{
			CString strErr;
			strErr.Format(_T("The file \"%s\" has already existed, overwrite it?"), strMask);
			if(IDCANCEL == MessageBox(strErr, _T("Overwrite files"), MB_OKCANCEL))
				return 2;
		}
		CopyFile(m_strMask, strMask, FALSE);
	}
	
	if(!m_bPackage)
	{
		CString CfgFileName = m_strWorkingPath + _T("\\config.cfg");
		TiXmlDocument docCfg(T2A((LPSTR)(LPCTSTR)CfgFileName));

		if(!docCfg.LoadFile(T2A((LPSTR)(LPCTSTR)CfgFileName)))
		{
			if(PathFileExists(CfgFileName))
			{
				MessageBox(_T("Read template database failed"));
				return ERROR_UNKNOWN;
			}
			docCfg.Clear();
		}
		TiXmlElement* eleTempl = docCfg.FirstChildElement("templates");
		if(eleTempl == NULL)
		{
			eleTempl = new TiXmlElement("templates");
			docCfg.LinkEndChild(eleTempl);
		}
		CString strCfgName = m_strTemplName;
		strCfgName.Replace(_T(' '), _T('_'));
		TiXmlElement* eleThis = eleTempl->FirstChildElement(T2A((LPTSTR)(LPCTSTR)strCfgName));
		if(eleThis != NULL)
		{
			CString strErr;
			strErr.Format(_T("The template \"%s\" already exists in the program database, overwrite it?"), m_strTemplName);
			if(IDOK == MessageBox(strErr, _T("Overwrite template"), MB_OKCANCEL))
			{
				eleTempl->RemoveChild(eleThis);
				eleThis = NULL;
			}
		}
		eleThis = new TiXmlElement(T2A((LPTSTR)(LPCTSTR)strCfgName));
		eleThis->SetAttribute("guid", T2A((LPTSTR)(LPCTSTR)FormatGuid(m_guid)));
		eleTempl->LinkEndChild(eleThis);
		if(!docCfg.SaveFile(T2A((LPSTR)(LPCTSTR)CfgFileName)))
			return 4;
	}
	return S_OK;
}

#include "Zip.h"

LRESULT CChildView::OnExportPakage(WPARAM wParam, LPARAM lParam)
{
	TCHAR Buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, Buffer);

	CString strDir = Buffer;
	if(strDir.Right(1) != _T("\\"))
		strDir+=_T("\\");

	CreateDirectory(strDir+_T("tmp"), NULL);
	CreateDirectory(strDir+_T("tmp\\media"), NULL);
	CreateDirectory(strDir+_T("tmp\\media\\Images"), NULL);
	CreateDirectory(strDir+_T("tmp\\media\\Templates"), NULL);
	CString strWorkPath = m_strWorkingPath;
	m_strWorkingPath = strDir+_T("tmp\\media");
	m_bPackage = TRUE;
	LRESULT hr = OnExportTempl(wParam, lParam);
	m_strWorkingPath = strWorkPath;
	m_bPackage = FALSE;
	if(hr !=S_OK)
	{
		return hr;
	}
	BOOL bRet = ZipToDir(m_strWorkingPath + _T("\\") + m_strTemplName + _T(".tpk"), strDir+_T("tmp"));
	DeleteDir(strDir+_T("tmp"));
	if(!bRet)
		return 5;
	return S_OK;
}

LRESULT CChildView::OnApplySettings(WPARAM wParam, LPARAM lParam)
{
	USES_CONVERSION;
	CDialogSettings* dlg = (CDialogSettings*)wParam;
	m_strWorkingPath = dlg->m_strWorkingPath;
	if(m_strWorkingPath.Right(1) == _T("\\"))
		m_strWorkingPath = m_strWorkingPath.Left(m_strWorkingPath.GetLength()-1);
	m_fEdge = atof(T2A((LPTSTR)(LPCTSTR)dlg->m_strEdge));
	m_fAlpha = atof(T2A((LPTSTR)(LPCTSTR)dlg->m_strAlpha));
	m_bGray = dlg->m_bChkGray;
	m_strTemplName = dlg->m_strTemplName;
	CFile file;
	if(file.Open("config", CFile::modeCreate|CFile::modeWrite))
	{
		char* str = T2A((LPTSTR)(LPCTSTR)m_strWorkingPath);
		file.Write(str, strlen(str));
		file.Close();
	}
	Draw();
	return 0;
}

CString CChildView::FormatGuid(GUID& guid)
{
	CString strGuid;
	strGuid.Format(_T("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),

		guid.Data1, guid.Data2, guid.Data3,

		guid.Data4[0], guid.Data4[1],

		guid.Data4[2], guid.Data4[3],

		guid.Data4[4], guid.Data4[5],

		guid.Data4[6], guid.Data4[7]);

	return strGuid;
}

void CChildView::OnFileAddquad()
{
	// TODO: Add your command handler code here
	AddQuad();
	Draw();
}


void CChildView::InitQuad(Quad& quad)
{
	quad.m_vlist[0] = Vec3(-1,-1,3);
	quad.m_vlist[1] = Vec3( 1,-1,3);
	quad.m_vlist[2] = Vec3(-1, 1,3);
	quad.m_vlist[3] = Vec3( 1, 1,3);

	quad.m_vlist2d[0] = Vec2(-0.5,-0.5);
	quad.m_vlist2d[1] = Vec2( 0.5,-0.5);
	quad.m_vlist2d[2] = Vec2(-0.5, 0.5);
	quad.m_vlist2d[3] = Vec2( 0.5, 0.5);

	quad.m_tclist[0] = Vec2(0,1);
	quad.m_tclist[1] = Vec2(1,1);
	quad.m_tclist[2] = Vec2(0,0);
	quad.m_tclist[3] = Vec2(1,0);

	quad.m_ibuf[0]= 0;
	quad.m_ibuf[1]= 2;
	quad.m_ibuf[2]= 3;
	quad.m_ibuf[3]= 3;
	quad.m_ibuf[4]= 1;
	quad.m_ibuf[5]= 0;

	quad.m_nSel = -1;

	quad.m_vlist2d[0] = Vec2(-0.5,-0.5);
	quad.m_vlist2d[1] = Vec2( 0.5,-0.5);
	quad.m_vlist2d[2] = Vec2(-0.5, 0.5);
	quad.m_vlist2d[3] = Vec2( 0.5, 0.5);
	Reform(quad.m_vlist2d, quad.m_vlist);
}

void CChildView::AddQuad()
{
	Quad quad;
	InitQuad(quad);
	m_quadextra.push_back(quad);
	m_nTopmost = m_quadextra.size();
}

void CChildView::ResetQuad()
{
	m_quadextra.clear();
	m_nTopmost = 0;
}

void CChildView::OnFileResetquad()
{
	// TODO: Add your command handler code here
	ResetQuad();
	Draw();
}

void CChildView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	Quad& quad=(m_nTopmost==0?m_quad:m_quadextra[m_nTopmost-1]);
	if(TestQuad(quad, point))
	{
		if(m_mask&&m_bUseMask)
		{
			int x=point.x*m_orgWidth/m_ImgBk1->width;
			int y=point.y*m_orgHeight/m_ImgBk1->height;
			uchar* pix=PTR_PIX(*m_mask, x, y);
			Vec3 clr(*pix,*(pix+1),*(pix+2));
			CString str;
			str.Format(_T("id=%d,clr=%f,%f,%f"), m_nTopmost, clr.x, clr.y, clr.z);
			MessageBox(str);
			quad.m_clr = clr;
		}
	}
	CWnd::OnLButtonDblClk(nFlags, point);
}

bool CChildView::TestQuad(Quad& quad, POINT pt)
{
	CvPoint pt_[4];
	for(int i=0;i<4;i++)
	{
		if(quad.m_tvlist[i].z<=0)
			return false;
		pt_[i]=cvPoint(quad.m_tvlist[i].x*m_Image->height/2+m_Image->width/2,
			-quad.m_tvlist[i].y*m_Image->height/2+m_Image->height/2);
	}
	CvPoint ptquad[4]={pt_[0],pt_[1],pt_[3],pt_[2]};
	int cnt = 0;
	for(int j=0;j<4;j++)
	{
		CvPoint pt1 = ptquad[j], pt2 = ptquad[(j+1)%4];
		if(pt2.y==pt1.y)continue;
		float x = pt1.x+(float)(pt2.x-pt1.x)*(pt.y-pt1.y)/(pt2.y-pt1.y);
		if(x<pt.x)
		{
			if(pt1.y>=pt.y&&pt2.y<pt.y)
				cnt++;
			else if(pt1.y<=pt.y&&pt2.y>pt.y)
				cnt--;
		}
	}
	if(abs(cnt)==1)
	{
		return true;
	}
	return false;
}

float CChildView::GetAsp(Quad& quad)
{
	Vec2 pt[4];
	for(int i=0;i<4;i++)
	{
		if(quad.m_tvlist[i].z<=0)
			return false;
		pt[i]=Vec2(quad.m_tvlist[i].x*m_Image->height/2+m_Image->width/2,
			-quad.m_tvlist[i].y*m_Image->height/2+m_Image->height/2);
	}
	Vec2 x=pt[1]-pt[0];
	Vec2 y=pt[2]-pt[0];
	float asp=x.length()/y.length();
	return asp;
}