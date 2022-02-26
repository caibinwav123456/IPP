
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "3DCollageConfig.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define FACE_ATTRIBUTE_COMMENT "groupindex:int"
#define GROUP_ATTRIBUTE_COMMENT "center:float3,tanu:float3,tanv:float3"

// CChildView

CChildView::CChildView():m_rot(1,0,0,0,1,0,0,0,1),m_pos(0,0,-3),m_Vport(0,0,1,1),m_specl(1,-1,1)
{	
	m_bShowVert = false;
	m_bShowImage = true;
	m_bDown = false;
	m_bSpcDown = false;
	m_bFDown = false;
	m_bGDown = false;
	m_Image = NULL;
	m_ImgBk1 = NULL;
	m_ImgBk2 = NULL;
	m_debuf = NULL;
	m_fProj = 1;
	m_bSpec = false;
	m_specl = m_specl.normalize();
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_FILE_OPENMESH, &CChildView::OnFileOpenmesh)
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_COMMAND(ID_FILE_OPENIMAGE, &CChildView::OnFileOpenimage)
END_MESSAGE_MAP()

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

	CRect rcClt;
	GetClientRect(rcClt);

	cvSet(m_Image, cvScalarAll(0));
	cvSet(m_debuf, cvScalar(0));

	cvCopyImage(m_ImgBk1, m_Image);

	//Vec2 v2[3]={Vec2(10,10),Vec2(100,10),Vec2(55,100)};
	//Triangle2D(m_Image, v2);

	if(m_bShowImage)
	{
		mesh.SetCullMode(CULL_MODE_CCW);
		mesh.SetMatrix(m_rot, -m_pos);
		Rect2D vp(m_Vport.x*m_Image->width,m_Vport.y*m_Image->height,m_Vport.cx*m_Image->width,m_Vport.cy*m_Image->height);
		mesh.SetViewPort(&vp);
		if(!m_bSpec)
		{
			if(mesh.m_nVertexFormat & VF_TEXCOORD)
				mesh.SetVSOutFormat(VFT_XYZ|VFT_COLOR|VFT_TEXCOORD);
			else
				mesh.SetVSOutFormat(VFT_XYZ|VFT_COLOR|VFT_USER(3));
		}
		else
		{
			if(mesh.m_nVertexFormat & VF_TEXCOORD)
				mesh.SetVSOutFormat(VFT_XYZ|VFT_COLOR|VFT_TEXCOORD|VFT_USER(6));
			else
				mesh.SetVSOutFormat(VFT_XYZ|VFT_COLOR|VFT_USER(9));
		}
		mesh.SetVertexShader(VertexShader);
		mesh.SetPixelShader(PixelShader);
		mesh.SetUserParam(this);
		mesh.Render(m_Image, &m_depImg);
	}
	if(m_bShowVert)
	{
		mesh.DrawWireFrame(m_Image, -1);
	}

	CDC mdc;
	mdc.CreateCompatibleDC(&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rcClt.Width(), rcClt.Height());
	CBitmap* oldbmp = mdc.SelectObject(&bmp);
	DispImage(&mdc, m_Image, CPoint(0,0));
	CString str;
	str.Format(_T("lx=%f,ly=%f,lz=%f"),m_specl.x,m_specl.y,m_specl.z);
	mdc.SetTextColor(RGB(255,0,0));
	mdc.SetBkMode(TRANSPARENT);
	mdc.TextOut(0,0,str);
	dc.BitBlt(0,0,rcClt.Width(),rcClt.Height(),&mdc,0,0,SRCCOPY);
	mdc.SelectObject(oldbmp);
	mdc.DeleteDC();
	bmp.DeleteObject();
}

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



void CChildView::OnDestroy()
{
	CWnd::OnDestroy();
	cvReleaseImage(&m_Image);
	cvReleaseImage(&m_ImgBk1);
	cvReleaseImage(&m_ImgBk2);
	cvReleaseImage(&m_debuf);
	cvReleaseImage(&m_ImgTex);
	RawImage::ReleaseInternalHdr();
	// TODO: Add your message handler code here
}


void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SetCapture();
	m_bDown = true;
	m_oldPt = point;

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
		if(m_bSpcDown)
		{
			m_Vport.x+=(float)offset.x/m_Image->width;
			m_Vport.y+=(float)offset.y/m_Image->height;
		}
		else if(m_bFDown)
		{
			float HAngle = (float)offset.x/100;
			Mat rotx(cos(HAngle),0,sin(HAngle),0,1,0,-sin(HAngle),0,cos(HAngle));
			m_rot=rotx*m_rot;
			m_rot = m_rot.ortho();
		}
		else if(m_bGDown)
		{
			float ZAngle = (float)offset.x/100;
			Mat rotz(cos(ZAngle),-sin(ZAngle),0,sin(ZAngle),cos(ZAngle),0,0,0,1);
			m_specl = m_specl*rotz;
			m_specl = m_specl.normalize();
		}
		else
		{
			float HAngle = (float)offset.x/100;
			float VAngle = (float)offset.y/100;
			Mat rotx(cos(HAngle),0,sin(HAngle),0,1,0,-sin(HAngle),0,cos(HAngle));
			Mat roty(1,0,0,0,cos(VAngle),-sin(VAngle),0,sin(VAngle),cos(VAngle));
			m_rot = m_rot*rotx*roty;
			m_rot = m_rot.ortho();
		}
		m_oldPt = point;
		Draw();
	}

	CWnd::OnMouseMove(nFlags, point);
}


void CChildView::OnFileOpenmesh()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Mesh Files|*.mesh||"), this);
	if(dlg.DoModal() == IDOK)
	{
		CString strName = dlg.GetPathName();
		mesh.LoadMesh(T2A(strName));
		ASSERT(mesh.GetFaceAttrLen() == 1);
		ASSERT(strcmp(mesh.GetFaceAttrDesc(), FACE_ATTRIBUTE_COMMENT) == 0);
		if(!(mesh.m_nVertexFormat & VF_TEXCOORD))
		{
			ASSERT(mesh.GetGrpAttrLen() == 9);
			ASSERT(strcmp(mesh.GetGrpAttrDesc(), GROUP_ATTRIBUTE_COMMENT) == 0);
		}
		m_strMeshName = strName.Left(strName.Find(_T('.')));
		LoadConfig();
		Invalidate();
	}
}


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_strMeshName = _T("cube.mesh");
	mesh.LoadMesh("cube.mesh");
	ASSERT(mesh.GetFaceAttrLen() == 1);
	ASSERT(strcmp(mesh.GetFaceAttrDesc(), FACE_ATTRIBUTE_COMMENT) == 0);
	if(!(mesh.m_nVertexFormat & VF_TEXCOORD))
	{
		ASSERT(mesh.GetGrpAttrLen() == 9);
		ASSERT(strcmp(mesh.GetGrpAttrDesc(), GROUP_ATTRIBUTE_COMMENT) == 0);
	}
	m_ImgBk1 = cvLoadImage("peaceb.jpg");
	m_ImgBk2 = cvLoadImage("peacew.jpg");

	int wImage=m_ImgBk1->width;
	int hImage=m_ImgBk1->height;

	m_Image = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,3);
	m_debuf = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,1);
	m_depImg= m_debuf;
	m_ImgTex = cvLoadImage("tex.jpg");
	m_texture = m_ImgTex;
	mesh.SetTexture(NULL);

	return 0;
}

void CChildView::VertexShader(Vec3* posin, Vec3* colorin, Vec3* normalin, Vec2* texin, float* vuserdatain, Vec3* posout, Vec3* colorout, Vec2* texout, float* vuserdataout, void* usrptr)
{
	CChildView* pObj = (CChildView*)usrptr;
	Mat rot(1,0,0,0,1,0,0,0,1);
	Vec3 Pos(0,0,-3);
	if(pObj)
	{
		rot=pObj->m_rot;
		Pos=pObj->m_pos;
	}
	Vec3 tv = *posin*rot;
	Vec3 tn = *normalin*rot;
	tn=normalize(tn);
	tv-=Pos;
	float proj=pObj->m_fProj;
	*posout = Vec3(tv.x/tv.z/proj,tv.y/tv.z/proj,1/tv.z);
	float d=dot(tn,tv.normalize());
	float l=fabs(d);

	if(!(pObj->m_bSpec))
	{
		if(pObj->mesh.m_nVertexFormat & VF_TEXCOORD)
			*texout = *texin;
		else
			*(Vec3*)vuserdataout = *posin;
	}
	else
	{
		if(pObj->mesh.m_nVertexFormat & VF_TEXCOORD)
		{
			*texout = *texin;
			*(Vec3*)vuserdataout = tn;
			*(Vec3*)(vuserdataout+3) = tv;
		}
		else
		{
			*(Vec3*)vuserdataout = *posin;
			*(Vec3*)(vuserdataout+3) = tn;
			*(Vec3*)(vuserdataout+6) = tv;
		}
	}
	*colorout = Vec3(l,l,l);
}

void CChildView::PixelShader(Point2D pos, Vec3* color, Vec2* tex, float* userdata, Vec3* colorout, float* depth, int iface, void* usrptr)
{
	CChildView* pObj = (CChildView*)usrptr;
	int idgrp = *(int*)(pObj->mesh.GetFaceAttr(iface));
	Vec2 texcoord;
	if(pObj->mesh.m_nVertexFormat & VF_TEXCOORD)
		texcoord = *tex;
	else
	{
		float* faceattr = pObj->mesh.GetGrpAttr(idgrp);
		Vec3 center = *(Vec3*)faceattr;
		Vec3 tanu = *(Vec3*)(faceattr+3);
		Vec3 tanv = *(Vec3*)(faceattr+6);
		Vec3 vpos = *(Vec3*)userdata;
		Vec3 off = vpos-center;
		texcoord = Vec2(dot(off,tanu),dot(off,tanv));
		texcoord+=Vec2(0.5,0.5);
	}

	Vec2 teximg = texcoord;
	Vec3 bkclr(0, 0, 0);
	Vec3 c(128,128,128);

	float fAsp = 1/((float)pObj->m_ImgTex->width/pObj->m_ImgTex->height);
	float aspcurb = 1;
	//if(pData->nStrechType == FRAME_STRETCH_TYPE_FILL)
		aspcurb = max(fAsp,1);
	//else if(pData->nStrechType == FRAME_STRETCH_TYPE_FIT)
	//	aspcurb = min(fAsp,1);
	teximg = Vec2(0.5+(teximg.x-0.5)*fAsp/aspcurb, 0.5+(teximg.y-0.5)/aspcurb);

	c=Sample(pObj->m_texture, teximg, WRAP_TYPE_CLAMP, TEX_FILTER_LINEAR, true, bkclr);
	Vec3 ct = *color*c/255;
	if(pObj->m_bSpec)
	{
		int offset = pObj->mesh.m_nVertexFormat & VF_TEXCOORD?0:3;
		Vec3* tn = (Vec3*)(userdata+offset);
		Vec3* tp = (Vec3*)(userdata+offset+3);
		Vec3 tpn = tp->normalize();
		Vec3 reflect = 2*dot(*tn, tpn)**tn-tpn;
		float l=dot(reflect, pObj->m_specl);
		float spec = powf(max(0,l),50);
		ct += spec*Vec3(0.8,0.8,0.8);
	}

	*colorout = ct;
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	switch(nChar)
	{
	case VK_SPACE:
		m_bSpcDown = true;
		break;
	case 'B':
		m_bShowVert = !m_bShowVert;
		Draw();
		break;
	case 'K':
		m_bShowImage = !m_bShowImage;
		Draw();
		break;
	case 'A':
		m_pos.x-=0.05;
		Draw();
		break;
	case 'D':
		m_pos.x+=0.05;
		Draw();
		break;
	case 'W':
		m_pos.z+=0.05;
		Draw();
		break;
	case 'S':
		m_pos.z-=0.05;
		Draw();
		break;
	case 'Q':
		m_pos.y-=0.05;
		Draw();
		break;
	case 'E':
		m_pos.y+=0.05;
		Draw();
		break;
	case 'Z':
		{
			CvPoint2D32f center = cvPoint2D32f(m_Vport.x+m_Vport.cx/2,m_Vport.y+m_Vport.cy/2);
			m_Vport.cx*=0.99;
			m_Vport.cy*=0.99;
			m_Vport.x=center.x-m_Vport.cx/2;
			m_Vport.y=center.y-m_Vport.cy/2;
			Draw();
		}
		break;
	case 'X':
		{
			CvPoint2D32f center = cvPoint2D32f(m_Vport.x+m_Vport.cx/2,m_Vport.y+m_Vport.cy/2);
			m_Vport.cx*=1.01;
			m_Vport.cy*=1.01;
			m_Vport.x=center.x-m_Vport.cx/2;
			m_Vport.y=center.y-m_Vport.cy/2;
			Draw();
		}
		break;
	case 'C':
		m_fProj*=1.01;
		Draw();
		break;
	case 'V':
		m_fProj/=1.01;
		Draw();
		break;
	case 'N':
		SaveConfig();
		break;
	case 'F':
		m_bFDown = true;
		break;
	case 'G':
		m_bGDown = true;
		break;
	case 'M':
		m_bSpec = !m_bSpec;
		Draw();
		break;
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CChildView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	switch(nChar)
	{
	case VK_SPACE:
		m_bSpcDown = false;
		break;
	case 'F':
		m_bFDown = false;
		break;
	case 'G':
		m_bGDown = false;
		break;
	}
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}


void CChildView::OnFileOpenimage()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Image Files|*.jpg;*.bmp;*.png||"), this);
	if(dlg.DoModal() == IDOK)
	{
		CString strName = dlg.GetPathName();
		cvReleaseImage(&m_ImgBk1);
		cvReleaseImage(&m_ImgBk2);
		m_ImgBk1 = cvLoadImage(T2A(strName));
		if(m_ImgBk1->width>1024 || m_ImgBk1->height>768)
		{
			IplImage* tmp = m_ImgBk1;
			float xscale = 1024.f/m_ImgBk1->width;
			float yscale = 768.f/m_ImgBk1->height;
			float scale = min(xscale,yscale);
			CvSize szImg = cvSize(cvRound(m_ImgBk1->width*scale),cvRound(m_ImgBk1->height*scale));
			m_ImgBk1 = cvCreateImage(szImg, IPL_DEPTH_8U, 3);
			cvResize(tmp, m_ImgBk1);
			cvReleaseImage(&tmp);
		}
		m_ImgBk2 = cvCloneImage(m_ImgBk1);

		int wImage=m_ImgBk1->width;
		int hImage=m_ImgBk1->height;

		cvReleaseImage(&m_Image);
		cvReleaseImage(&m_debuf);
		m_Image = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,3);
		m_debuf = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_32F,1);
		m_depImg= m_debuf;
		Draw();
	}
}

void CChildView::SaveConfig()
{
	CString strCfg = m_strMeshName+_T(".cfg");
	CFile file;
	file.Open(strCfg, CFile::modeCreate|CFile::modeWrite);
	file.Write(&m_rot, sizeof(m_rot));
	file.Write(&m_pos, sizeof(m_pos));
	file.Write(&m_Vport, sizeof(m_Vport));
	file.Write(&m_fProj, sizeof(float));
	file.Close();
}

void CChildView::LoadConfig()
{
	CString strCfg = m_strMeshName+_T(".cfg");
	CFile file;
	if(file.Open(strCfg, CFile::modeRead))
	{
		file.Read(&m_rot, sizeof(m_rot));
		file.Read(&m_pos, sizeof(m_pos));
		file.Read(&m_Vport, sizeof(m_Vport));
		file.Read(&m_fProj, sizeof(float));
		file.Close();
	}
}