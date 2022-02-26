
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "BoxCvt.h"
#include "ChildView.h"
#include "DlgSize.h"
#include "Image.h"
#include "Propagate.h"
#include "ProcessImage.h"
#include "ImageProcess3D.h"
#include "CutEdge.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <fstream>
using namespace std;
// CChildView
#define DEFAULT_INHIBIT 0.00f
CChildView::CChildView():m_size(800),m_seg(15),m_overlap(0.2f),m_Image(NULL),
	m_ImageConv(NULL),m_ImageOrg(NULL),m_scale(1.0f),m_scanl(0),m_scanlx(0),
	m_bShow(false),m_bShowAdjust(false),m_adj(0,0),m_adjmode(0),m_finhibit(DEFAULT_INHIBIT),
	m_bSupressRipple(false),m_bShowRipple(false),m_bFillHoles(false),
	m_bDbgMode(false),m_file(NULL),m_dbg(false),m_xIntense(NULL),m_yIntense(NULL),
	m_nx(0),m_ny(0),m_xDev(NULL),m_yDev(NULL),m_Pos(0,0),m_topcut(0),m_botcut(0),
	m_leftcut(0),m_rightcut(0),m_cutimage(NULL),m_bShowCut(false),m_ncore(1),
	m_rTable(NULL),m_CosineTable(NULL),m_Grad(NULL),m_bShowGrad(false),m_vDir(0,0),m_certainty(0),
	m_GradIntense(NULL),m_GradIntenseAdj(NULL),m_nGradIntense(0),m_bShowGradIntense(false),
	m_kBaseAmple(0),m_kRippleAmple(0),m_kBasePower(0),m_kT(0),m_kTheta(0),m_kNoise(0),m_kCenter(0),
	m_kCenterAsc(0),m_kAmpleAsc(0),m_bAdjustMotion(false),m_RotImg(NULL),m_RotConv(NULL),m_bShowRotate(false),
	m_adjmotionmode(0),m_bSinglePass(false)
{
	ZeroMemory(m_CompExt,sizeof(m_CompExt));
	ZeroMemory(m_CompExtT,sizeof(m_CompExtT));
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_COMMAND(ID_FILE_OPEN2, &CChildView::OnFileOpen2)
	ON_COMMAND(ID_FILE_SETSIZE, &CChildView::OnFileSetsize)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_FILE_OPENTEX, &CChildView::OnFileOpentex)
	ON_COMMAND(ID_FILE_PROCESSTOTAL, &CChildView::OnFileProcesstotal)
	ON_COMMAND(ID_FILE_THRESHOLD, &CChildView::OnFileThreshold)
	ON_COMMAND(ID_FILE_DENOISE, &CChildView::OnFileDenoise)
	ON_COMMAND(ID_FILE_THOROUGHPROCESS, &CChildView::OnFileThoroughprocess)
	ON_COMMAND(ID_FILE_DLL, &CChildView::OnFileDll)
	ON_COMMAND(ID_FILE_SHOWAXISDISTRIBUTION, &CChildView::OnFileShowaxisdistribution)
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_FILE_DLLCUTEDGE, &CChildView::OnFileDllcutedge)
	ON_COMMAND(ID_CORE_1, &CChildView::OnCore1)
	ON_COMMAND(ID_CORE_2, &CChildView::OnCore2)
	ON_COMMAND(ID_CORE_4, &CChildView::OnCore4)
	ON_UPDATE_COMMAND_UI(ID_CORE_1, &CChildView::OnUpdateCore1)
	ON_UPDATE_COMMAND_UI(ID_CORE_2, &CChildView::OnUpdateCore2)
	ON_UPDATE_COMMAND_UI(ID_CORE_4, &CChildView::OnUpdateCore4)
	ON_COMMAND(ID_CORE_SINGLEPASS, &CChildView::OnCoreSinglepass)
	ON_UPDATE_COMMAND_UI(ID_CORE_SINGLEPASS, &CChildView::OnUpdateCoreSinglepass)
	ON_COMMAND(ID_FILE_RESIZEIMAGE, &CChildView::OnFileResizeimage)
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

void CChildView::OnFileOpen2()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Image Files|*.bmp;*.jpg;*.png||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strOut = _T(".\\out");
		if(!PathFileExists(strOut))
			CreateDirectory(strOut, NULL);
		CString strPath = dlg.GetPathName();
		CString strName = strPath;
		if(strName.ReverseFind(_T('.'))!=-1)
			strName = strName.Left(strName.ReverseFind(_T('.')));
		IplImage* image = cvLoadImage(T2A(strPath));
		CString strBox = strName+_T(".box");
		if(image!=NULL && PathFileExists(strBox))
		{
			Convert(image, strName, strOut);
			MessageBox(_T("Cvt succeeded!"));
		}
		else
			MessageBox(_T("Load file failed!"));
		if(image)
			cvReleaseImage(&image);
	}
}

void CChildView::Convert(IplImage*& image, CString strBox,CString strOut)
{
	USES_CONVERSION;
	int width = image->width;
	int height = image->height;
	int size = m_size;
	float scale = 1.0f;
	if(width>size||height>size)
	{
		float xscale = (float)size/width;
		float yscale = (float)size/height;
		scale = min(xscale, yscale);
		width = cvRound(width*scale);
		height = cvRound(height*scale);
		IplImage* old = image;
		image = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 3);
		cvResize(old, image);
		cvReleaseImage(&old);
	}
	CString boxname = strBox;
	int iSlash;
	if((iSlash = boxname.ReverseFind(_T('\\')))!=-1)
		boxname = boxname.Right(boxname.GetLength()-1-iSlash);
	CString strOutImg = strOut+_T("\\")+boxname+_T(".png");
	CString strOutBox = strOut+_T("\\")+boxname+_T(".box");
	strBox+=_T(".box");
	cvSaveImage(T2A(strOutImg), image);
	ifstream file;
	file.open(T2A(strBox), ios::in|ios::_Nocreate);
	ofstream ofile;
	ofile.open(T2A(strOutBox));
	char num[500];
	while(!file.eof())
	{
		file.getline(num, 500);
		int l,t,r,b,p;
		char txt[50];
		int i=0;
		char* ptr = num;
		while(*ptr!=0&&*ptr!=' '&&*ptr!='\t')
			txt[i++] = *(ptr++);
		if(i==0)
			continue;
		txt[i] = 0;
		if(*ptr != 0)
			ptr++;
		sscanf_s(ptr, "%d%d%d%d%d", &l,&t,&r,&b,&p);
		l*=scale;
		t*=scale;
		r*=scale;
		b*=scale;
		sprintf_s(num, "%s %d %d %d %d %d", txt,l,t,r,b,p);
		ofile<<num<<endl;
	}
	file.close();
	ofile.close();
}

void CChildView::OnFileSetsize()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CDlgSize dlg;
	dlg.m_size.Format(_T("%d"), m_size);
	dlg.m_seg.Format(_T("%d"), m_seg);
	dlg.m_overlap.Format(_T("%5.3f"),m_overlap);
	if(dlg.DoModal() == IDOK)
	{
		m_size = atoi(T2A(dlg.m_size));
		if(m_size<1)m_size=1;
		if(m_size>5000)m_size=5000;
		m_seg = atoi(T2A(dlg.m_seg));
		if(m_seg<2)m_seg=2;
		if(m_seg>100)m_seg=100;
		m_overlap = atof(T2A(dlg.m_overlap));
		if(m_overlap<0)m_overlap=0;
		if(m_overlap>0.5)m_overlap=0.5;
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

void CChildView::Process(IplImage* image)
{
	IplImage* gray = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
	cvCvtColor(image,gray, CV_RGB2GRAY);
	IplImage* grayf = cvCreateImage(cvGetSize(gray), IPL_DEPTH_32F, 1);
	cvCvtScale(gray,grayf);
	cvReleaseImage(&gray);
	IplImage* fft = cvCreateImage(cvGetSize(grayf), IPL_DEPTH_32F, 2);
	cvZero(fft);
	cvMerge(grayf, NULL, NULL, NULL, fft);
	cvReleaseImage(&grayf);
	cvReleaseImage(&m_Image);
	m_Image = cvCreateImage(cvGetSize(fft), IPL_DEPTH_32F, 2);
	cvDFT(fft, m_Image, CV_DXT_FORWARD);
	cvReleaseImage(&fft);
	cvReleaseImage(&m_ImageOrg);
	m_ImageOrg = cvCreateImage(cvGetSize(m_Image), IPL_DEPTH_32F, 2);
	cvReleaseImage(&m_Grad);
	m_Grad=cvCreateImage(cvGetSize(m_Image), IPL_DEPTH_32F, 2);
	cvCopyImage(m_Image, m_ImageOrg);
	if(m_bSupressRipple)
		SupressRipple(m_ImageOrg, m_Image);
	bool tmp = m_bShowAdjust;
	bool tmp2 = m_bAdjustMotion;
	m_bShowAdjust = false;
	m_bAdjustMotion = false;
	m_Compensation = ComputeNormal(m_ImageOrg);
	m_bAdjustMotion=tmp2;
	if(m_bAdjustMotion&&m_certainty>0.1&&m_kT>0)
	{
		cvReleaseImage(&m_RotConv);
		m_RotConv=cvCreateImage(cvGetSize(m_RotImg), IPL_DEPTH_32F, 2);
		AdjustMotion(m_RotImg, m_RotConv);
		RotateImage(m_Image, m_vDir, -1);
	}
	cvReleaseImage(&m_ImageConv);
	m_ImageConv = cvCreateImage(cvGetSize(m_Image),IPL_DEPTH_32F,2);
	m_bAdjustMotion = tmp2;
	AdjustImage(m_Image, m_ImageConv, m_Compensation);
	m_bShowAdjust = true;
	m_bAdjustMotion = true;
	m_CompensationT = ComputeNormal(m_ImageConv);
	m_bShowAdjust = tmp;
	m_bAdjustMotion = tmp2;
	if(m_finhibit>0)
	{
		IplImage* tmp=cvCloneImage(m_ImageConv);
		SupressHigh(tmp, m_ImageConv);
		cvReleaseImage(&tmp);
	}
}

void CChildView::ComputeGrad(IplImage* src, IplImage* dest)
{
	for(int i=0;i<src->width;i++)
	for(int j=0;j<src->height;j++)
	{
		float* pix=(float*)PTR_PIX(*dest, i, j);
		int pi=(i+src->width-1)%src->width;
		int ni=(i+1)%src->width;
		int pj=(j+src->height-1)%src->height;
		int nj=(j+1)%src->height;
		double dx=(*(double*)PTR_PIX(*src, ni, j)-*(double*)PTR_PIX(*src, pi, j))/2;
		double dy=(*(double*)PTR_PIX(*src, i, nj)-*(double*)PTR_PIX(*src, i, pj))/2;
		pix[0]=dx;
		pix[1]=dy;
	}
}

void CChildView::AdjustImage(IplImage* src,IplImage* dest, Vec4 comp)
{
	IplImage* tmpimg=NULL;
	Vec3 lvl(-0.45f,0.f,0.f);
	float max_adj=0.2f;
	float max_adj2=0.4f;
	float max_adj3=0.5f;
	float slopelvl=-0.45f;
	/*if(m_finhibit>0)
	{
		max_adj+=min(0.25,(m_finhibit*0.8));
		max_adj3+=min(0.25,(m_finhibit*0.8));
	}*/
	if(m_adjmode>=2)
	{
		if(m_adjmode==2)
			lvl = Vec3(-1.05f,1.0f,0.0f);
		else
			lvl = Vec3(-0.35f,1.0f,0.0f);
	}
	double diffx=0;
	double diffy=0;
	double diffz=0;
	double diffk=0;
	double slopex=0;
	double slopey=0;
	double lvla,lvlb,lvlc,llvl;
	if(m_adjmode<2)
	{
		diffx = max(0,min(max_adj,lvl.x-comp.x))+m_adj.x;
		diffy = max(0,min(max_adj,lvl.x-comp.y))+m_adj.y;
	}
	else
	{
		if(m_adjmode==3)
		{
			comp.x=m_CompExt[0];
			comp.y=m_CompExt[1];
			comp.z=m_CompExt[2];
			comp.t=m_CompExt[3];
			slopex=slopey=m_CompExt[4];
			slopex=max(0,min(max_adj,slopelvl-slopex));
			slopey=max(0,min(max_adj,slopelvl-slopey));
		}
		else if(m_adjmode==4)
		{
			comp.x=m_CompExt[0];
			comp.y=m_CompExt[1];
			comp.z=m_CompExt[2];
			comp.t=m_CompExt[3];
			slopex=m_CompExt[4];
			slopey=m_CompExt[5];
			slopex=max(0,min(max_adj,slopelvl-slopex));
			slopey=max(0,min(max_adj,slopelvl-slopey));
		}
		diffk = max(0,min(max_adj3,lvl.x-comp.x));
		diffx = max(-max_adj2,min(max_adj2,lvl.y-comp.y));
		diffy = max(-max_adj2,min(max_adj2,lvl.z-comp.z));
		diffz = max(-max_adj2,min(max_adj2,lvl.y-comp.t));
		lvla=lvl.y-diffx;
		lvlb=-diffy;
		lvlc=lvl.y-diffz;
		double thresh=0.9;
		if(lvlb*lvlb>4*lvla*lvlc*thresh*thresh)
		{
			lvlb=(lvlb>=0?1:-1)*sqrt(lvla*lvlc)*2*thresh;
		}
		llvl=(-sqrtf(powf(lvla-lvlc,2)+powf(lvlb,2))+lvla+lvlc)/2;
		//llvl*=3;
		if(m_dbg)
		{
			fprintf_s(m_file,"dk=%f,dx=%f,dy=%f,dz=%f,sx=%f,sy=%f\n",diffk, diffx, diffy, diffz, slopex, slopey);
		}
	}
	int hw=(src->width+2)/2;
	int hh=(src->height+2)/2;
	for(int i=0;i<hw;i++)
	{
		for(int j=0;j<hh;j++)
		{
			double f,f2;
			if(m_adjmode==0)
			{
				double fx = powf(i,diffx);
				if(i==0)fx=1;
				double fy = powf(j,diffy);
				if(j==0)fy=1;
				f=fx*fy;
				f2=f;
			}
			else if(m_adjmode==1)
			{
				f=powf(i*i+j*j,(diffx+diffy)/4);
				if(i==0&&j==0)f=1;
				f2=f;
			}
			else if(m_adjmode==2)
			{
				f=powf(i*i*lvla+i*j*lvlb+j*j*lvlc,diffk/2)/llvl;
				f2=powf(i*i*lvla-i*j*lvlb+j*j*lvlc,diffk/2)/llvl;
				if(i==0&&j==0)
					f=f2=1;
				//if(i>=hw*(1-m_finhibit)||j>=hh*(1-m_finhibit))
				{
				}
			}
			else if(m_adjmode==3||m_adjmode==4)
			{
				f=powf(i*i*lvla+i*j*lvlb+j*j*lvlc,diffk/2)/llvl;
				f2=powf(i*i*lvla-i*j*lvlb+j*j*lvlc,diffk/2)/llvl;
				float f3=max(1,powf(i,slopex))*max(1,powf(j,slopey));
				f*=f3,f2*=f3;
				if(i==0&&j==0)
					f=f2=1;
				//if(i>=hw*(1-m_finhibit)||j>=hh*(1-m_finhibit))
				{
				}
			}
			int x[2]={i,src->width-i};
			int y[2]={j,src->height-j};
			for(int k=0;k<2;k++)
			for(int l=0;l<2;l++)
			{
				if(x[k]==src->width||y[l]==src->height)
					continue;
				float* pixs = (float*)PTR_PIX(*src,x[k],y[l]);
				float* pix = (float*)PTR_PIX(*dest,x[k],y[l]);
				if((k+l)%2==0)
				{
					pix[0] = pixs[0]*f;
					pix[1] = pixs[1]*f;
				}
				else
				{
					pix[0] = pixs[0]*f2;
					pix[1] = pixs[1]*f2;
				}
			}
		}
	}
	cvReleaseImage(&tmpimg);
}

void CChildView::AdjustMotion(IplImage* src, IplImage* dest)
{
	Vec2 dir(0,1);//=m_vDir;
	float maxl=0;
	Vec2 org(0,0);
	//Adjust motion
	if((m_vDir.x>=0&&m_vDir.y>=0)||(m_vDir.x<0&&m_vDir.y<0))
	{
		maxl=fabs(dot(Vec2(src->width,src->height), dir));
	}
	else
	{
		maxl=fabs(dot(Vec2(src->width,-src->height), dir));
	}
	float f0=(cosf(-m_kTheta*CV_PI/m_kT)*m_kRippleAmple+m_kCenter)/m_kBaseAmple;
	for(int i=0;i<src->width;i++)
	for(int j=0;j<src->height;j++)
	{
		float s=j;//fabs(dot(Vec2(i,j)-org, dir));
		if(s>maxl/2)
			s=maxl-s;
		float fdenum=m_kBaseAmple*powf(s,m_kBasePower)+m_kNoise;
		float fnum;
		float hf;
		if(m_adjmotionmode==0)
		{
			fnum=(cosf((s-m_kTheta)*CV_PI/m_kT)*(m_kRippleAmple+s*m_kAmpleAsc)+(m_kCenter+s*m_kCenterAsc))*powf(s,m_kBasePower)+m_kNoise;
			hf=fnum/fdenum;
			hf/=f0;
			if(s==0.0f)
				hf=1.0f;
			if(hf<0.001f)
				hf=0.001f;
		}
		else
		{
			int index=(int)floorf(s);
			int index2=index+1;
			float frac=s-index;
			index=max(0,min(m_nGradIntense-1,index));
			index2=max(0,min(m_nGradIntense-1,index2));
			fnum=m_GradIntense[index]*(1-frac)+m_GradIntense[index2]*frac;
			hf=fnum/fdenum;
			if(s==0.0f)
				hf=1.0f;
			if(hf<0.2f)
				hf=0.2f;
		}
		float invsnr=0.01f;//m_kNoise/(hf*m_kBaseAmple*powf(l, m_kBasePower));
		float coeff=(hf)/(hf*hf+invsnr);
		float* pixs = (float*)PTR_PIX(*src,i,j);
		float* pix = (float*)PTR_PIX(*dest,i,j);
		pix[0]=pixs[0]*coeff;
		pix[1]=pixs[1]*coeff;
	}
}

extern HANDLE g_hEvent[4];
int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	for(int i=0;i<4;i++)
		g_hEvent[i]=CreateEvent(NULL, TRUE, FALSE, _T(""));
	m_rTable=cvCreateImage(cvSize(100,100),IPL_DEPTH_32F,1);
	m_CosineTable=cvCreateImage(cvSize(100,1),IPL_DEPTH_32F,1);
	for(int i=0;i<m_rTable->width;i++)
	{
		for(int j=0;j<m_rTable->height;j++)
		{
			float fx=(float)i/m_rTable->width;
			float fy=(float)j/m_rTable->height;
			float r=sqrtf(fx*fx+fy*fy);
			*(float*)PTR_PIX(*m_rTable, i, j)=r;
		}
	}
	for(int i=0;i<m_CosineTable->width;i++)
	{
		float x=(float)i/m_CosineTable->width;
		float cosine=(cosf(x*CV_PI)+1)/2;
		*(float*)PTR_PIX(*m_CosineTable, i, 0)=cosine;
	}
	// TODO:  Add your specialized creation code here
	/*IplImage* image = cvLoadImage("jitter.jpg");
	Process(image);
	cvReleaseImage(&image);*/
	return 0;
}

void CChildView::OnDestroy()
{
	CWnd::OnDestroy();
	cvReleaseImage(&m_Image);
	cvReleaseImage(&m_ImageConv);
	cvReleaseImage(&m_ImageOrg);
	cvReleaseImage(&m_cutimage);
	cvReleaseImage(&m_rTable);
	cvReleaseImage(&m_CosineTable);
	cvReleaseImage(&m_Grad);
	cvReleaseImage(&m_RotImg);
	cvReleaseImage(&m_RotConv);
	if(m_xIntense)
	{
		delete[] m_xIntense;
		m_xIntense = NULL;
	}
	if(m_yIntense)
	{
		delete[] m_yIntense;
		m_yIntense = NULL;
	}
	if(m_xDev)
	{
		delete[] m_xDev;
		m_xDev = NULL;
	}
	if(m_yDev)
	{
		delete[] m_yDev;
		m_yDev = NULL;
	}
	if(m_GradIntense)
	{
		delete[] m_GradIntense;
		m_GradIntense=NULL;
		m_nGradIntense=0;
	}
	if(m_GradIntenseAdj)
	{
		delete[] m_GradIntenseAdj;
		m_GradIntenseAdj=NULL;
	}
	for(int i=0;i<4;i++)
		CloseHandle(g_hEvent[i]);
	// TODO: Add your message handler code here
}

void CChildView::Draw()
{
	CClientDC dc(this);
	CRect rcClient;
	GetClientRect(rcClient);
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rcClient.Width(),rcClient.Height());
	CBitmap* oldbmp = dcMem.SelectObject(&bmp);
	if(!m_Image)
		DrawSub(&dcMem);
	else
		Draw(&dcMem);
	dc.BitBlt(0,0,rcClient.Width(),rcClient.Height(),&dcMem,0,0,SRCCOPY);
	dcMem.SelectObject(oldbmp);
	dcMem.DeleteDC();
	bmp.DeleteObject();
}

void CChildView::DrawSub(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(rcClient);
	pDC->FillSolidRect(rcClient, RGB(255,255,255));
	CString strDisp;
	strDisp.Format(_T("freguency model:%d"),m_adjmode);
	pDC->TextOut(0,0,strDisp);
	if(m_bSupressRipple)
		pDC->TextOut(0,20,_T("submodel:1"));
	if(m_bFillHoles)
		pDC->TextOut(0,40,_T("fill holes:on"));
	if(m_bDbgMode)
		pDC->TextOut(0,60,_T("debug mode:on"));
	if(m_cutimage&&m_bShowCut)
	{
		float xscale=(float)rcClient.Width()/m_cutimage->width;
		float yscale=(float)rcClient.Height()/m_cutimage->height;
		float scale=min(xscale, yscale);
		int width=cvRound(m_cutimage->width*scale);
		int height=cvRound(m_cutimage->height*scale);
		IplImage* disp=cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
		cvResize(m_cutimage, disp);
		DispImage(pDC, disp, CPoint(0,0));
		cvReleaseImage(&disp);
	}
	else
	{
	if(m_xIntense&&m_yIntense)
	{
		pDC->MoveTo(0, rcClient.Height()/3);
		pDC->LineTo(rcClient.Width(), rcClient.Height()/3);
		pDC->MoveTo(0, rcClient.Height()*2/3);
		pDC->LineTo(rcClient.Width(), rcClient.Height()*2/3);
		bool bfirst=true;
		for(int i=0;i<m_nx;i++)
		{
			int x=i*rcClient.Width()/max(1,(m_nx-1));
			int y=rcClient.Height()/3-m_xIntense[i];
			if(bfirst)
				pDC->MoveTo(x,y);
			else
				pDC->LineTo(x,y);
			bfirst=false;
		}
		bfirst=true;
		for(int i=0;i<m_ny;i++)
		{
			int x=i*rcClient.Width()/max(1,(m_ny-1));
			int y=rcClient.Height()*2/3-m_yIntense[i];
			if(bfirst)
				pDC->MoveTo(x,y);
			else
				pDC->LineTo(x,y);
			bfirst=false;
		}
	}
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(255,0,0));
	CPen bpen;
	bpen.CreatePen(PS_SOLID, 1, RGB(0,0,255));
	CPen* oldpen = pDC->SelectObject(&pen);
	if(m_xDev&&m_yDev)
	{
		bool bfirst=true;
		for(int i=0;i<m_nx;i++)
		{
			int x=i*rcClient.Width()/max(1,(m_nx-1));
			int y=rcClient.Height()/3-m_xDev[i]*10;
			if(bfirst)
				pDC->MoveTo(x,y);
			else
				pDC->LineTo(x,y);
			bfirst=false;
		}
		bfirst=true;
		for(int i=0;i<m_ny;i++)
		{
			int x=i*rcClient.Width()/max(1,(m_ny-1));
			int y=rcClient.Height()*2/3-m_yDev[i]*10;
			if(bfirst)
				pDC->MoveTo(x,y);
			else
				pDC->LineTo(x,y);
			bfirst=false;
		}
		pDC->SelectObject(&bpen);
		int top=m_topcut*rcClient.Width()/m_ny;
		int bottom=m_botcut*rcClient.Width()/m_ny;
		int left=m_leftcut*rcClient.Width()/m_nx;
		int right=m_rightcut*rcClient.Width()/m_nx;
		pDC->MoveTo(left, rcClient.Height()/3-100);
		pDC->LineTo(left, rcClient.Height()/3+100);
		pDC->MoveTo(right,rcClient.Height()/3-100);
		pDC->LineTo(right,rcClient.Height()/3+100);
		pDC->MoveTo(top,  rcClient.Height()*2/3-100);
		pDC->LineTo(top,  rcClient.Height()*2/3+100);
		pDC->MoveTo(bottom,rcClient.Height()*2/3-100);
		pDC->LineTo(bottom,rcClient.Height()*2/3+100);

		CString str;
		int x=m_Pos.x*m_nx/rcClient.Width();
		int y=m_Pos.x*m_ny/rcClient.Width();
		x=max(0,min(m_nx-1, x));
		y=max(0,min(m_ny-1, y));
		str.Format(_T("x:%d,xdev:%f,y:%d,ydev:%f"), x,m_xDev[x],y,m_yDev[y]);
		pDC->TextOut(0, 80, str);
	}

	pDC->SelectObject(oldpen);
	pen.DeleteObject();
	bpen.DeleteObject();
	}
}

void CChildView::Draw(CDC* pDC)
{
	IplImage* sImage;
	if(m_bShowRotate)
	{
		if(m_bShowAdjust&&m_RotConv)
			sImage = m_RotConv;
		else
			sImage = m_RotImg;
	}
	else
	{
		if(m_bShowAdjust)
			sImage = m_ImageConv;
		else
			sImage = m_Image;
	}
	CRect rcClient;
	GetClientRect(rcClient);
	pDC->FillSolidRect(rcClient, RGB(255,255,255));
	IplImage* image = cvCreateImage(cvGetSize(sImage),IPL_DEPTH_32F,3);
	IplImage* r=cvCreateImage(cvGetSize(sImage),IPL_DEPTH_32F,1);
	IplImage* g=cvCreateImage(cvGetSize(sImage),IPL_DEPTH_32F,1);
	cvSplit(sImage, r,g,NULL,NULL);
	cvZero(image);
	cvMerge(r,NULL,NULL, NULL, image);
	cvMerge(NULL,g,NULL, NULL, image);
	IplImage* disp = cvCreateImage(cvGetSize(sImage),IPL_DEPTH_8U,3);
	cvCvtScale(image, disp, m_scale/20);
	DispImage(pDC, disp, CPoint(0,0));
	IplImage* oimage = cvCreateImage(cvGetSize(sImage),IPL_DEPTH_32F,2);
	cvDFT(sImage, oimage, CV_DXT_INV_SCALE);
	cvSplit(oimage, r,g,NULL,NULL);
	cvReleaseImage(&oimage);
	cvZero(image);
	cvMerge(r,NULL,NULL, NULL, image);
	cvMerge(NULL,r,NULL, NULL, image);
	cvMerge(NULL,NULL,r, NULL, image);
	//cvMerge(NULL,g,NULL, NULL, image);
	cvCvtScale(image, disp);
	DispImage(pDC, disp, CPoint(100+sImage->width,0));
	cvReleaseImage(&image);
	cvReleaseImage(&r);
	cvReleaseImage(&g);
	cvReleaseImage(&disp);
	pDC->MoveTo(0,500);
	pDC->LineTo(sImage->width, 500);
	int base2 = sImage->width+100;
	pDC->MoveTo(base2,500);
	pDC->LineTo(base2+sImage->height, 500);
	bool b = true;
	if(m_bShow)
	{
		for(int x=0;x<sImage->width;x++)
		{
			float* pix = (float*)PTR_PIX(*sImage, x, m_scanl);
			int y=500-(int)(*pix*m_scale/1000);
			if(b)
				pDC->MoveTo(x,y);
			else
				pDC->LineTo(x,y);
			b=false;
		}
		b = true;
		for(int x=0;x<sImage->height;x++)
		{
			float* pix = (float*)PTR_PIX(*sImage, m_scanlx, x);
			int y=500-(int)(*pix*m_scale/1000);
			if(b)
				pDC->MoveTo(x+base2,y);
			else
				pDC->LineTo(x+base2,y);
			b=false;
		}
	}
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(255,0,0));
	CPen* oldpen = pDC->SelectObject(&pen);
	if(m_bShow)
	{
		b = true;
		for(int x=0;x<sImage->width;x++)
		{
			float* pix = (float*)PTR_PIX(*sImage, x, m_scanl);
			int y=500-(int)(pix[1]*m_scale/1000);
			if(b)
				pDC->MoveTo(x,y);
			else
				pDC->LineTo(x,y);
			b=false;
		}
		b = true;
		for(int x=0;x<sImage->height;x++)
		{
			float* pix = (float*)PTR_PIX(*sImage, m_scanlx, x);
			int y=500-(int)(pix[1]*m_scale/1000);
			if(b)
				pDC->MoveTo(x+base2,y);
			else
				pDC->LineTo(x+base2,y);
			b=false;
		}
	}
	pDC->MoveTo(0,m_scanl);
	pDC->LineTo(sImage->width,m_scanl);
	pDC->MoveTo(m_scanlx,0);
	pDC->LineTo(m_scanlx,sImage->height);
	CPen bpen;
	bpen.CreatePen(PS_SOLID, 1, RGB(0,0,255));
	pDC->SelectObject(&bpen);
	double* xc=new double[sImage->width];
	double* yc=new double[sImage->height];
	double* xc2=new double[sImage->width];
	double* yc2=new double[sImage->height];
	b = true;
	for(int x=0;x<sImage->width;x++)
	{
		double ac=0,ac2=0;
		int gaugei=min(20,min(m_scanl,sImage->height-m_scanl)*0.25);
		int gaugej=min(20,min(x,sImage->width-x)*0.25);
		for(int i=m_scanl-5-gaugei;i<m_scanl+6+gaugei;i++)
		for(int j=x-5-gaugej;j<x+6+gaugej;j++)
		{
			int x2=j;
			if(x2<0)x2+=sImage->width;
			if(x2>=sImage->width)x2-=sImage->width;
			int y2=i;
			if(y2<0)y2+=sImage->height;
			if(y2>=sImage->height)y2-=sImage->height;
			float* pix = (float*)PTR_PIX(*sImage, x2, y2);
			double amp=sqrt((pow(pix[0],2)+pow(pix[1],2))/2);
			ac2+=amp;
			if(i>=m_scanl-5&&i<m_scanl+6&&j>=x-5&&j<x+6)
				ac+=amp;
		}
		ac/=11*11;
		ac2/=(11+2*gaugei)*(11+2*gaugej);
		xc[x]=ac;
		xc2[x]=ac2;
		int y=500-(int)(ac*m_scale/1000);
		if(b)
			pDC->MoveTo(x,y);
		else
			pDC->LineTo(x,y);
		b=false;
	}
	b = true;
	for(int x=0;x<sImage->height;x++)
	{
		double ac=0,ac2=0;
		int gaugei=min(20,min(m_scanlx,sImage->width-m_scanlx)*0.25);
		int gaugej=min(20,min(x,sImage->height-x)*0.25);
		for(int i=m_scanlx-5-gaugei;i<m_scanlx+6+gaugei;i++)
		for(int j=x-5-gaugej;j<x+6+gaugej;j++)
		{
			int x2=j;
			if(x2<0)x2+=sImage->height;
			if(x2>=sImage->height)x2-=sImage->height;
			int y2=i;
			if(y2<0)y2+=sImage->width;
			if(y2>=sImage->width)y2-=sImage->width;
			float* pix = (float*)PTR_PIX(*sImage, y2, x2);
			double amp=sqrt((pow(pix[0],2)+pow(pix[1],2))/2);
			ac2+=amp;
			if(i>=m_scanlx-5&&i<m_scanlx+6&&j>=x-5&&j<x+6)
				ac+=amp;
		}
		ac/=11*11;
		ac2/=(11+2*gaugei)*(11+2*gaugej);
		yc[x]=ac;
		yc2[x]=ac2;
		int y=500-(int)(ac*m_scale/1000);
		if(b)
			pDC->MoveTo(x+base2,y);
		else
			pDC->LineTo(x+base2,y);
		b=false;
	}
	b = true;
	for(int i=0;i<sImage->width/2;i++)
	{
		if(i==0)continue;
		float x=log((double)i/(sImage->width/2))*20+sImage->width/2;
		float y=600-log(xc[i])*20;
		if(b)
			pDC->MoveTo(x,y);
		else
			pDC->LineTo(x,y);
		b=false;
	}
	b=true;
	for(int i=0;i<sImage->height/2;i++)
	{
		if(i==0)continue;
		float x=log((double)i/(sImage->height/2))*20+sImage->height/2;
		float y=600-log(yc[i])*20;
		if(b)
			pDC->MoveTo(x+base2,y);
		else
			pDC->LineTo(x+base2,y);
		b=false;
	}
	CPen ppen;
	ppen.CreatePen(PS_SOLID, 1, RGB(255,0,255));
	pDC->SelectObject(&ppen);
	if(m_bShowRipple)
	{
		b = true;
		for(int i=0;i<sImage->width;i++)
		{
			int y=500-(int)(xc2[i]*m_scale/1000);
			if(b)
				pDC->MoveTo(i,y);
			else
				pDC->LineTo(i,y);
			b=false;
		}
		b = true;
		for(int i=0;i<sImage->height;i++)
		{
			int y=500-(int)(yc2[i]*m_scale/1000);
			if(b)
				pDC->MoveTo(i+base2,y);
			else
				pDC->LineTo(i+base2,y);
			b=false;
		}
	}
	Vec4 comp;
	double* comp2;
	if(!m_bShowAdjust)
	{
		comp = m_Compensation;
		comp2 = m_CompExt;
	}
	else
	{
		comp = m_CompensationT;
		comp2 = m_CompExtT;
	}
	int pos1=0,pos2x=sImage->width/2,pos2y=sImage->height/2;
	if(m_adjmode<2)
	{
		pDC->MoveTo(pos1,600-(comp.z+comp.x*pos1));
		pDC->LineTo(pos2x,600-(comp.z+comp.x*pos2x));
		pDC->MoveTo(pos1+base2,600-(comp.z+comp.y*pos1));
		pDC->LineTo(pos2y+base2,600-(comp.z+comp.y*pos2y));
	}
	else if(m_adjmode<3)
	{
		pDC->MoveTo(pos1,600-(comp.x*pos1));
		pDC->LineTo(pos2x,600-(comp.x*pos2x));
		pDC->MoveTo(pos1+base2,600-(comp.x*pos1));
		pDC->LineTo(pos2y+base2,600-(comp.x*pos2y));
	}
	else
	{
		pDC->MoveTo(pos1,600-(comp2[0]*pos1));
		pDC->LineTo(pos2x,600-(comp2[0]*pos2x));
		pDC->MoveTo(pos1+base2,600-(comp2[0]*pos1));
		pDC->LineTo(pos2y+base2,600-(comp2[0]*pos2y));
	}
	CPen gpen;
	gpen.CreatePen(PS_SOLID, 1, RGB(0,128,0));
	pDC->SelectObject(&gpen);
	if(m_bShowRipple)
	{
		b = true;
		for(int i=0;i<sImage->width;i++)
		{
			int y=500-(int)((xc[i]-xc2[i])/xc2[i]*m_scale*100);
			if(b)
				pDC->MoveTo(i,y);
			else
				pDC->LineTo(i,y);
			b=false;
		}
		b = true;
		for(int i=0;i<sImage->height;i++)
		{
			int y=500-(int)((yc[i]-yc2[i])/yc2[i]*m_scale*100);
			if(b)
				pDC->MoveTo(i+base2,y);
			else
				pDC->LineTo(i+base2,y);
			b=false;
		}
	}
	delete[] xc;
	delete[] yc;
	delete[] xc2;
	delete[] yc2;
	if(m_bShowGrad&&m_Grad)
	{
		pDC->SelectObject(ppen);
		CPoint center=rcClient.CenterPoint();
		for(int i=0;i<m_Grad->width;i++)
		for(int j=0;j<m_Grad->height;j++)
		{
			float* pix=(float*)PTR_PIX(*m_Grad, i, j);
			CPoint v=center+CPoint(pix[0]/20,pix[1]/20);
			pDC->MoveTo(center);
			pDC->LineTo(v);
		}
		Vec2 vdisp = m_vDir*m_certainty*1000;
		CPoint vd=center+CPoint(vdisp.x, vdisp.y);
		pDC->SelectObject(&gpen);
		pDC->MoveTo(center);
		pDC->LineTo(vd);
	}
	if(m_bShowGradIntense)
	{
		float* vdisp=NULL;
		if(!m_bShowAdjust&&m_GradIntense)
			vdisp=m_GradIntense;
		else if(m_bShowAdjust&&m_GradIntenseAdj)
			vdisp=m_GradIntenseAdj;
		b=true;
		int ybase2=700;
		if(ybase2>rcClient.Height()-100)
			ybase2=rcClient.Height()-100;
		pDC->SelectObject(&gpen);
		pDC->MoveTo(0,ybase2);
		pDC->LineTo(m_nGradIntense, ybase2);
		pDC->SelectObject(&pen);
		if(vdisp)
		for(int i=0;i<m_nGradIntense;i++)
		{
			CPoint pt(i, ybase2-vdisp[i]*m_scale*0.1);
			if(b)
				pDC->MoveTo(pt);
			else
				pDC->LineTo(pt);
			b=false;
		}
		if(m_kBasePower!=0)
		{
			b=true;
			pDC->SelectObject(&bpen);
			for(int i=0;i<m_nGradIntense;i++)
			{
				float v=m_kBaseAmple*powf(i, m_kBasePower)+m_kNoise;
				CPoint pt(i, ybase2-v*m_scale*0.1);
				if(b)
					pDC->MoveTo(pt);
				else
					pDC->LineTo(pt);
				b=false;
			}
			b=true;
			pDC->SelectObject(&gpen);
			if(vdisp)
			for(int i=0;i<m_nGradIntense;i++)
			{
				float v=(i==0?0:(vdisp[i]-m_kNoise)*powf(i,-m_kBasePower));
				CPoint pt(i, ybase2-v*m_scale*0.1);
				if(b)
					pDC->MoveTo(pt);
				else
					pDC->LineTo(pt);
				b=false;
			}
			if(m_kT>0)
			{
				pDC->SelectObject(&ppen);
				b=true;
				for(int i=0;i<m_nGradIntense;i++)
				{
					float v=cosf((i-m_kTheta)*CV_PI/m_kT)*(m_kRippleAmple+i*m_kAmpleAsc)+(m_kCenter+i*m_kCenterAsc);
					CPoint pt(i, ybase2-v*m_scale*0.1);
					if(b)
						pDC->MoveTo(pt);
					else
						pDC->LineTo(pt);
					b=false;
				}
				pDC->SelectObject(oldpen);
				b=true;
				if(vdisp)
				for(int i=0;i<m_nGradIntense;i++)
				{
					//float v=(i==0?0:(vdisp[i]-m_kNoise)*powf(i,-m_kBasePower));
					//v-=cosf((i-m_kTheta)*CV_PI/m_kT)*(m_kRippleAmple+i*m_kAmpleAsc)+(m_kCenter+i*m_kCenterAsc);
					float v=vdisp[i];
					float f0=(cosf(-m_kTheta*CV_PI/m_kT)*m_kRippleAmple+m_kCenter)/m_kBaseAmple;
					float f1=m_kBaseAmple*powf(i,m_kBasePower)+m_kNoise;
					float f2=(cosf((i-m_kTheta)*CV_PI/m_kT)*(m_kRippleAmple+i*m_kAmpleAsc)+(m_kCenter+i*m_kCenterAsc))*powf(i,m_kBasePower)+m_kNoise;
					float f=f2/f1/f0;
					if(i==0)
						f=1.0f;
					f=max(0.01f,f);
					float f3=f/(f*f+0.01f);
					v*=f3;
					CPoint pt(i, ybase2-v*m_scale*0.1);
					if(b)
						pDC->MoveTo(pt);
					else
						pDC->LineTo(pt);
					b=false;
				}
			}
		}
		pDC->SelectObject(&ppen);
		for(int i=0;i<(int)m_highs.size();i++)
		{
			pDC->MoveTo(m_highs[i], ybase2-50);
			pDC->LineTo(m_highs[i], ybase2+50);
		}
		pDC->SelectObject(&gpen);
		for(int i=0;i<(int)m_lows.size();i++)
		{
			pDC->MoveTo(m_lows[i], ybase2-50);
			pDC->LineTo(m_lows[i], ybase2+50);
		}
	}
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(255,0,255));
	CString strDisp;
	if(m_adjmode<=2)
		strDisp.Format(_T("%f,%f,%f,%f"),comp.x,comp.y,comp.z,comp.t);
	else
		strDisp.Format(_T("%f,%f,%f,%f,%f,%f"),comp2[0],comp2[1],comp2[2],comp2[3],comp2[4],comp2[5]);
	pDC->TextOut(0,sImage->height,strDisp);
	strDisp.Format(_T("%f"),m_finhibit);
	pDC->TextOut(0,sImage->height+20,strDisp);
	if(m_bShowAdjust)
	{
		strDisp.Format(_T("%f,%f"),m_adj.x,m_adj.y);
		pDC->TextOut(0,sImage->height+40,strDisp);
	}
	strDisp.Format(_T("model:%d"),m_adjmode);
	pDC->TextOut(0,sImage->height+60,strDisp);

	pDC->SelectObject(oldpen);
	pen.DeleteObject();
	bpen.DeleteObject();
	ppen.DeleteObject();
	gpen.DeleteObject();
}

BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	if(zDelta>0)
		m_scale*=1.2f;
	else
		m_scale/=1.2f;
	if(m_scale>10000.f)m_scale=10000.f;
	else if(m_scale<0.0001f)m_scale=0.0001f;
	Draw();
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if(!m_Image)
	{
		switch(nChar)
		{
		case 'T':
			m_adjmode=(m_adjmode+1)%5;
			break;
		case 'N':
			m_bSupressRipple=!m_bSupressRipple;
			break;
		case 'M':
			m_bFillHoles=!m_bFillHoles;
			break;
		case 'L':
			m_bDbgMode = !m_bDbgMode;
			break;
		case 'I':
			m_bShowCut = !m_bShowCut;
			break;
		}
		Draw();
		return;
	}
	switch(nChar)
	{
	case VK_UP:
		m_scanl--;
		break;
	case VK_DOWN:
		m_scanl++;
		break;
	case VK_LEFT:
		m_scanlx--;
		break;
	case VK_RIGHT:
		m_scanlx++;
		break;
	case 'A':
		m_bShow = !m_bShow;
		break;
	case 'Z':
		m_bShowAdjust = !m_bShowAdjust;
		break;
	case 'W':
		m_adj.x+=0.01f;
		break;
	case 'S':
		m_adj.x-=0.01f;
		break;
	case 'E':
		m_adj.y+=0.01f;
		break;
	case 'D':
		m_adj.y-=0.01f;
		break;
	case 'R':
		m_adj = Vec2(0,0);
		break;
	case 'T':
		m_adjmode=(m_adjmode+1)%5;
		break;
	case 'C':
		m_finhibit-=0.01f;
		break;
	case 'V':
		m_finhibit+=0.01f;
		break;
	case 'B':
		m_bShowRipple = !m_bShowRipple;
		break;
	case 'N':
		m_bSupressRipple=!m_bSupressRipple;
		break;
	case 'M':
		m_bFillHoles=!m_bFillHoles;
		break;
	case 'L':
		m_bDbgMode = !m_bDbgMode;
		break;
	case 'X':
		m_bShowGrad=!m_bShowGrad;
		break;
	case 'F':
		m_bShowGradIntense=!m_bShowGradIntense;
		break;
	case 'Q':
		m_bAdjustMotion=!m_bAdjustMotion;
		break;
	case 'G':
		if(m_vDir.length()>0)
		{
			DebugAdjust(m_ImageOrg);
		}
		break;
	case 'H':
		m_bShowRotate=!m_bShowRotate;
		break;
	case 'J':
		m_adjmotionmode=1-m_adjmotionmode;
		break;
	}
	if(m_scanl<0)m_scanl=0;
	if(m_scanl>=m_Image->height)m_scanl=m_Image->height-1;
	if(m_scanlx<0)m_scanlx=0;
	if(m_scanlx>=m_Image->width)m_scanlx=m_Image->width-1;
	if(m_finhibit<0)m_finhibit=0;
	if(m_finhibit>1)m_finhibit=1;
	if(nChar=='W'||nChar=='S'||nChar=='E'||nChar=='D'||nChar=='R'||nChar=='T'||nChar=='C'||nChar=='V'||nChar=='N'||nChar=='Q'||nChar=='J')
	{
		cvReleaseImage(&m_ImageConv);
		m_ImageConv = cvCreateImage(cvGetSize(m_Image),IPL_DEPTH_32F,2);
		bool tmp=m_bShowAdjust;
		bool tmp2=m_bAdjustMotion;
		if(m_bSupressRipple)
			SupressRipple(m_ImageOrg, m_Image);
		else
			cvCopyImage(m_ImageOrg, m_Image);
		m_bShowAdjust=false;
		m_bAdjustMotion=false;
		m_Compensation = ComputeNormal(m_ImageOrg);
		m_bAdjustMotion = tmp2;
		if(m_bAdjustMotion&&m_certainty>0.1&&m_kT>0)
		{
			cvReleaseImage(&m_RotConv);
			m_RotConv=cvCreateImage(cvGetSize(m_RotImg), IPL_DEPTH_32F, 2);
			AdjustMotion(m_RotImg, m_RotConv);
			RotateImage(m_Image, m_vDir, -1);
		}
		AdjustImage(m_Image, m_ImageConv, m_Compensation);
		m_bShowAdjust=true;
		m_bAdjustMotion=true;
		m_CompensationT=ComputeNormal(m_ImageConv);
		m_bShowAdjust=tmp;
		m_bAdjustMotion=tmp2;
		if(m_finhibit>0)
		{
			IplImage* tmp=cvCloneImage(m_ImageConv);
			SupressHigh(tmp, m_ImageConv);
			cvReleaseImage(&tmp);
		}
	}
	Draw();
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChildView::OnFileOpentex()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Image Files|*.bmp;*.jpg;*.png||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strPath = dlg.GetPathName();
		IplImage* image = cvLoadImage(T2A(strPath));
		m_scanl=m_scanlx=0;
		m_finhibit=DEFAULT_INHIBIT;
		m_adj = Vec2(0,0);
		Process(image);
		cvReleaseImage(&image);
		Draw();
	}
}

void CChildView::RotateImage(IplImage* image, Vec2 dir, int d)
{
	Vec2 dx(dir.y, dir.x);
	Vec2 dy(-dir.x, dir.y);
	Vec2 dx2=dx*image->width;
	Vec2 dy2=dy*image->height;
	float lx=fabs(dx2.x)+fabs(dy2.x);
	float ly=fabs(dx2.y)+fabs(dy2.y);
	float px=(dy2.x<0?-dy2.x:0)+(dx2.x<0?-dx2.x:0);
	float py=(dy2.y<0?-dy2.y:0)+(dx2.y<0?-dx2.y:0);
	Vec2 p(px,py);
	if(d>0)
	{
		cvReleaseImage(&m_RotImg);
		m_RotImg = cvCreateImage(cvSize(lx,ly), IPL_DEPTH_32F, 2);
		IplImage* org=cvCreateImage(cvGetSize(image), IPL_DEPTH_32F, 2);
		IplImage* rotorg=cvCreateImage(cvSize(lx,ly), IPL_DEPTH_32F, 2);
		dx.y=-dx.y;
		Vec2 p2(-p.x*dx.x+p.y*dx.y, -p.x*dx.y-p.y*dx.x);
		p=p2;
		cvDFT(image, org, CV_DXT_INV_SCALE);
		RotateImage(org, rotorg, dx, p);
		cvDFT(rotorg, m_RotImg, CV_DXT_FORWARD);
		cvReleaseImage(&org);
		cvReleaseImage(&rotorg);
	}
	else if(d<0)
	{
		ASSERT(m_RotImg&&m_RotImg->width==(int)lx&&m_RotImg->height==(int)ly);
		IplImage* org=cvCreateImage(cvSize(lx,ly), IPL_DEPTH_32F, 2);
		IplImage* rotorg=cvCreateImage(cvGetSize(image), IPL_DEPTH_32F, 2);
		cvDFT(m_RotConv, org, CV_DXT_INV_SCALE);
		RotateImage(org, rotorg, dx, p);
		cvDFT(rotorg, image, CV_DXT_FORWARD);
		cvReleaseImage(&org);
		cvReleaseImage(&rotorg);
	}
}

IplImage* CChildView::ComputeAmp(IplImage* image, IplImage** ripple)
{
	IplImage* amp2 = cvCreateImage(cvSize(image->width+10,image->height+10),IPL_DEPTH_32F,1);
	for(int i=0;i<amp2->width;i++)
	{
		for(int j=0;j<amp2->height;j++)
		{
			int x=(i-5+image->width)%image->width;
			int y=(j-5+image->height)%image->height;
			float* pix = (float*)PTR_PIX(*image, x, y);
			float* pixd = (float*)PTR_PIX(*amp2, i, j);
			*pixd = sqrt((pow(pix[0],2)+pow(pix[1],2))/2);
		}
	}
	IplImage* integ = cvCreateImage(cvSize(amp2->width+1,amp2->height+1),IPL_DEPTH_64F,1);
	cvIntegral(amp2,integ);
	cvReleaseImage(&amp2);

	IplImage* amp = cvCreateImage(cvGetSize(image), IPL_DEPTH_64F, 1);
	if(ripple)
		*ripple=cvCreateImage(cvGetSize(image), IPL_DEPTH_64F, 1);
	for(int x=0;x<image->width;x++)
	for(int y=0;y<image->height;y++)
	{
		double ac=0,ac2=0;
		int gaugex=min(20,min(x,image->width-x)*0.25);
		int gaugey=min(20,min(y,image->height-y)*0.25);
		double* tl=(double*)PTR_PIX(*integ,x,y);
		double* tr=(double*)PTR_PIX(*integ,x+11,y);
		double* bl=(double*)PTR_PIX(*integ,x,y+11);
		double* br=(double*)PTR_PIX(*integ,x+11,y+11);
		double* tl2=(double*)PTR_PIX(*integ,x-gaugex,y-gaugey);
		double* tr2=(double*)PTR_PIX(*integ,x+gaugex+11,y-gaugey);
		double* bl2=(double*)PTR_PIX(*integ,x-gaugex,y+gaugey+11);
		double* br2=(double*)PTR_PIX(*integ,x+gaugex+11,y+gaugey+11);
		ac=*br-*bl-*tr+*tl;
		ac2=*br2-*bl2-*tr2+*tl2;
/*
		for(int i=x-5;i<x+6;i++)
		for(int j=y-5;j<y+6;j++)
		{
			int x2=i;
			if(x2<0)x2+=image->width;
			if(x2>=image->width)x2-=image->width;
			int y2=j;
			if(y2<0)y2+=image->height;
			if(y2>=image->height)y2-=image->height;

			float* pix = (float*)PTR_PIX(*image, x2, y2);
			ac+=sqrt((pow(pix[0],2)+pow(pix[1],2))/2);
		}
*/
		ac/=11*11;
		ac2/=(11+2*gaugex)*(11+2*gaugey);
		*(double*)PTR_PIX(*amp, x, y) = ac;
		if(ripple)
			*(double*)PTR_PIX(**ripple, x, y)=ac2/ac;
	}
	cvReleaseImage(&integ);
	return amp;
}

Vec4 CChildView::ComputeNormal(IplImage* image)
{
	IplImage* amp=ComputeAmp(image);
	IplImage* grad=cvCreateImage(cvGetSize(amp),IPL_DEPTH_32F,2);
	ComputeGrad(amp, grad);
	NormalizeGrad(grad);
	{
		cvReleaseImage(&m_Grad);
		m_Grad=cvCloneImage(grad);
	}
	if(m_bAdjustMotion)
	{
		int tmp=m_nGradIntense;
		if(m_GradIntenseAdj)
		{
			delete[] m_GradIntenseAdj;
			m_GradIntenseAdj=NULL;
			m_nGradIntense=0;
		}
		if(m_RotConv)
		{
			IplImage* amp2=ComputeAmp(m_RotConv);
			ComputeGradIntense(amp2, Vec2(0,1), &m_GradIntenseAdj, &m_nGradIntense);
			cvReleaseImage(&amp2);
			ASSERT(tmp==m_nGradIntense);
		}
	}
	else
	{
		ComputeMotionDirection(grad, &m_vDir, &m_certainty);
		if(m_GradIntense)
		{
			delete[] m_GradIntense;
			m_GradIntense=NULL;
			m_nGradIntense=0;
		}
		RotateImage(image, m_vDir, 1);
		IplImage* amp2=ComputeAmp(m_RotImg);
		ComputeGradIntense(amp2, Vec2(0,1), &m_GradIntense, &m_nGradIntense);
		cvReleaseImage(&amp2);
		EstimateKernel(m_GradIntense, m_nGradIntense, &m_kBaseAmple, &m_kBasePower, &m_kRippleAmple, &m_kT, &m_kTheta, &m_kNoise, &m_kCenter, &m_kCenterAsc, &m_kAmpleAsc);
	}
	cvReleaseImage(&grad);
	int mincx = 0/*image->width/2/10*/,mincy = 0/*image->height/2/10*/;
	double Tx=0,Ty=0,Tz=0,T=0,XY=0,XZ=0,YZ=0,X=0,Y=0,Z=0,X2=0,Y2=0,Z2=0;
	int n=0;
	double coeff[6][6];
	double coeff2[6];
	ZeroMemory(coeff,sizeof(coeff));
	ZeroMemory(coeff2,sizeof(coeff2));
	for(int i=0;i<image->width/2;i++)
	{
		if(i==0)
			continue;
		for(int j=0;j<image->height/2;j++)
		{
			if(i<mincx&&j<mincy)
				continue;
			if(j==0)
				continue;
			//The i2,j2 and i,j region and the i,j2 and i2,j region are central symmetric, so ignore half.
			int i2=image->width-i;
			if(m_adjmode<2)
			{
				float x=log((double)i/(image->width/2))*20+image->width/2;
				float y=log((double)j/(image->height/2))*20+image->height/2;
				float t=log(*(double*)PTR_PIX(*amp, i, j))*20
					+log(*(double*)PTR_PIX(*amp, i2, j))*20;
				if(*(double*)PTR_PIX(*amp, i, j)<=0.0
					||*(double*)PTR_PIX(*amp, i2, j)<=0.0)
					continue;
				Tx+=t*x;
				Ty+=t*y;
				T+=t;
				XY+=2*x*y;
				X+=2*x;
				Y+=2*y;
				X2+=2*x*x;
				Y2+=2*y*y;
				n+=2;
			}
			else if(m_adjmode==2)
			{
				Vec2 v(i,j);
				float l=v.length();
				Vec2 vn=v.normalize();
				float x=log(l);
				float y=powf(vn.x,2)-powf(vn.y,2);
				float z=vn.x*vn.y;
				float z2=-z;
				float t=log(*(double*)PTR_PIX(*amp, i, j));
				float t2=log(*(double*)PTR_PIX(*amp, i2, j));
				//logf(powf(1.6*i*i+1.8*j*j+0.2*i*j, -3.7/2));
				if(*(double*)PTR_PIX(*amp, i, j)<=0.0
					||*(double*)PTR_PIX(*amp, i2, j)<=0.0)
					continue;
				Tx+=(t+t2)*x;
				Ty+=(t+t2)*y;
				Tz+=t*z+t2*z2;
				T+=t+t2;
				X2+=2*x*x;
				Y2+=2*y*y;
				Z2+=z*z+z2*z2;
				XY+=2*x*y;
				XZ+=0;//z+z2=0
				YZ+=0;//z+z2=0
				X+=2*x;
				Y+=2*y;
				Z+=0;//z+z2=0
				n+=2;
			}
			else if(m_adjmode==3)
			{
				Vec2 v(i,j);
				float l=v.length();
				Vec2 vn=v.normalize();
				float co[6],co2[6];
				co[0]=co2[0]=log(l);
				co[1]=co2[1]=powf(vn.x,2)-powf(vn.y,2);
				co[2]=vn.x*vn.y;
				co2[2]=-co[2];
				co[3]=co2[3]=logf(i)+logf(j);
				co[4]=co2[4]=1.0f;
				co[5]=log(*(double*)PTR_PIX(*amp, i, j));
				co2[5]=log(*(double*)PTR_PIX(*amp, i2, j));
				if(*(double*)PTR_PIX(*amp, i, j)<=0.0
					||*(double*)PTR_PIX(*amp, i2, j)<=0.0)
					continue;
				for(int k=0;k<5;k++)
				for(int l=k;l<5;l++)
				{
					coeff[k][l]+=co[k]*co[l];
				}
				for(int k=0;k<5;k++)
				{
					coeff2[k]+=co[5]*co[k];
				}
				for(int k=0;k<5;k++)
				for(int l=k;l<5;l++)
				{
					coeff[k][l]+=co2[k]*co2[l];
				}
				for(int k=0;k<5;k++)
				{
					coeff2[k]+=co2[5]*co2[k];
				}
			}
			else if(m_adjmode==4)
			{
				Vec2 v(i,j);
				float l=v.length();
				Vec2 vn=v.normalize();
				float co[7],co2[7];
				co[0]=co2[0]=log(l);
				co[1]=co2[1]=powf(vn.x,2)-powf(vn.y,2);
				co[2]=vn.x*vn.y;
				co2[2]=-co[2];
				co[3]=co2[3]=logf(i);
				co[4]=co2[4]=logf(j);
				co[5]=co2[5]=1.0f;
				co[6]=log(*(double*)PTR_PIX(*amp, i, j));
				co2[6]=log(*(double*)PTR_PIX(*amp, i2, j));
				if(*(double*)PTR_PIX(*amp, i, j)<=0.0
					||*(double*)PTR_PIX(*amp, i2, j)<=0.0)
					continue;
				for(int k=0;k<6;k++)
				for(int l=k;l<6;l++)
				{
					coeff[k][l]+=co[k]*co[l];
				}
				for(int k=0;k<6;k++)
				{
					coeff2[k]+=co[6]*co[k];
				}
				for(int k=0;k<6;k++)
				for(int l=k;l<6;l++)
				{
					coeff[k][l]+=co2[k]*co2[l];
				}
				for(int k=0;k<6;k++)
				{
					coeff2[k]+=co2[6]*co2[k];
				}
			}
		}
	}
	cvReleaseImage(&amp);
	Vec4 vret;
	if(m_adjmode<2)
	{
		Mat m(X2,XY,X,XY,Y2,Y,X,Y,n);
		Vec3 v(Tx,Ty,T);
		Vec3 vo = v*m.inv();
		vret = Vec4(vo.x,vo.y,vo.z,0);
	}
	else if(m_adjmode==2)
	{
		Mat4 m(X2,XY,XZ,X,XY,Y2,YZ,Y,XZ,YZ,Z2,Z,X,Y,Z,n);
		Vec4 v(Tx,Ty,Tz,T);
		Vec4 va = v*m.inv();
		//double e=exp(va.t*2/va.x);
		double k=va.x;
		double ea=tanh(va.y*2/va.x);
		double a=(1+ea);//*e;
		double b=tanh(va.z*2/va.x);//*e;
		double c=(1-ea);//*e;
		vret = Vec4(k,a,b,c);
	}
	else if(m_adjmode==3||m_adjmode==4)
	{
		int n;
		if(m_adjmode==3)n=5;
		else n=6;
		double* comp;
		if(m_bShowAdjust)
		{
			comp=m_CompExtT;
			ZeroMemory(m_CompExtT, sizeof(m_CompExtT));
		}
		else
		{
			comp=m_CompExt;
			ZeroMemory(m_CompExt, sizeof(m_CompExt));
		}

		IplImage* mat = cvCreateImage(cvSize(n,n),IPL_DEPTH_64F,1);
		for(int i=0;i<n;i++)
		for(int j=0;j<n;j++)
		{
			*(double*)PTR_PIX(*mat,i,j)=coeff[min(i,j)][max(i,j)];
		}
		IplImage* invmat = cvCreateImage(cvSize(n,n),IPL_DEPTH_64F,1);
		cvInvert(mat,invmat,CV_SVD_SYM);
#ifdef _DEBUG
		double deb[6][6];
		ZeroMemory(deb,sizeof(deb));
		for(int i=0;i<n;i++)
		for(int j=0;j<n;j++)
		{
			double ac=0.0;
			for(int k=0;k<n;k++)
			{
				ac+=*(double*)PTR_PIX(*mat,i,k)**(double*)PTR_PIX(*invmat,k,j);
			}
			deb[i][j]=ac;
		}
#endif
		double co[6];
		ZeroMemory(co,sizeof(co));
		for(int i=0;i<n;i++)
		{
			for(int j=0;j<n;j++)
			{
				co[i]+=*(double*)PTR_PIX(*invmat,i,j)*coeff2[j];
			}
		}
		cvReleaseImage(&mat);
		cvReleaseImage(&invmat);
		if(m_adjmode==3)
		{
			comp[0]=co[0];
			double ea=tanh(co[1]*2/co[0]);
			comp[1]=1+ea;
			comp[2]=tanh(co[2]*2/co[0]);
			comp[3]=1-ea;
			comp[4]=co[3];
		}
		else
		{
			comp[0]=co[0];
			double ea=tanh(co[1]*2/co[0]);
			comp[1]=1+ea;
			comp[2]=tanh(co[2]*2/co[0]);
			comp[3]=1-ea;
			comp[4]=co[3];
			comp[5]=co[4];
		}
	}
	return vret;
}

void CChildView::NormalizeGrad(IplImage* grad)
{
	float asp=(float)grad->width/grad->height;
	for(int i=0;i<grad->width;i++)
	for(int j=0;j<grad->height;j++)
	{
		*(float*)PTR_PIX(*grad, i, j)*=asp;
	}
}

void CChildView::ComputeMotionDirection(IplImage* src, Vec2* dir, float* certainty)
{
	float XX=0,YY=0,XY=0,N=0;
	for(int i=0;i<src->width;i++)
	for(int j=0;j<src->height;j++)
	{
		float* pix=(float*)PTR_PIX(*src,i,j);
		Vec2 v(pix[0],pix[1]);
		if(v.length()>20000)
			continue;
		float fx=(float)i/src->width*2,fy=(float)j/src->height*2;
		if(fx>1)
			fx=2-fx;
		if(fy>1)
			fy=2-fy;
		float f=fx*fy;
		XX+=v.x*v.x*f;
		YY+=v.y*v.y*f;
		XY+=v.x*v.y*f;
		N+=f;
	}
	XX/=N;
	XY/=N;
	YY/=N;
	float delta=sqrtf((XX-YY)*(XX-YY)+4*XY*XY);
	float c=delta/(XX+YY);
	Vec2 veigen((YY-XX-delta)/2, -XY);
	if(veigen.length()>0)
		veigen=veigen.normalize();
	*certainty=c;
	*dir=veigen;
}

void CChildView::ProcessTotal(IplImage* src,IplImage* dest)
{
	int min_blk_size=128;
	int seg=10;
	int blk_size=min(src->width,src->height)/seg;
	if(blk_size<min_blk_size)blk_size=min_blk_size;
	int segx=max(1,src->width/blk_size);
	int segy=max(1,src->height/blk_size);
	int blkx=src->width/segx+1;
	int blky=src->height/segy+1;
	if(blkx>src->width)blkx=src->width;
	if(blky>src->height)blky=src->height;
	int cellx=2*segx-1;
	int celly=2*segy-1;
	IplImage* gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src,gray, CV_RGB2GRAY);
	IplImage* grayf = cvCreateImage(cvGetSize(gray), IPL_DEPTH_32F, 1);
	cvCvtScale(gray,grayf);
	cvReleaseImage(&gray);
	IplImage* blk = cvCreateImage(cvSize(blkx,blky),IPL_DEPTH_32F,1);
	IplImage* prefft = cvCreateImage(cvSize(blkx,blky),IPL_DEPTH_32F,2);
	IplImage* fft = cvCreateImage(cvSize(blkx,blky),IPL_DEPTH_32F,2);
	IplImage* fft2 = cvCreateImage(cvSize(blkx,blky),IPL_DEPTH_32F,2);
	cvZero(prefft);
	Vec4* params = new Vec4[cellx*celly];
	double (*paramext)[6] = new double[cellx*celly][6];
	IplImage* destf=cvCreateImage(cvGetSize(dest), IPL_DEPTH_32F, 1);
	StepParam sparam;
	sparam.segx=segx;
	sparam.segy=segy;
	sparam.cellx=cellx;
	sparam.celly=celly;
	sparam.blkx=blkx;
	sparam.blky=blky;
	sparam.blk=blk;
	sparam.prefft=prefft;
	sparam.fft=fft;
	sparam.fft2=fft2;
	sparam.dest=destf;
	sparam.params=params;
	sparam.paramext=paramext;
	if(m_bDbgMode)
	{
		USES_CONVERSION;
		CString fname=m_dbgPath+_T("\\param.txt");
		fopen_s(&m_file,T2A(fname),"w");
	}
	PerformStep(&CChildView::step1,grayf,&sparam);
	if(m_bDbgMode)
	{
		fclose(m_file);
		m_file=NULL;
	}
	for(int n=0;n<3;n++)
	for(int i=1;i<cellx-1;i++)
	{
		for(int j=1;j<celly-1;j++)
		{
			CvPoint pad[4]={cvPoint(1,0),cvPoint(0,1),cvPoint(-1,0),cvPoint(0,-1)};
			if(m_adjmode<3)
			{
				Vec4 t(0,0,0,0);
				for(int k=0;k<4;k++)
				{
					t+=params[(i+pad[k].x)*celly+(j+pad[k].y)];
				}
				t=t/4-params[i*celly+j];
				params[i*celly+j]+=t*0.5;
			}
			else
			{
				for(int k=0;k<6;k++)
				{
					double t=0;
					for(int l=0;l<4;l++)
					{
						t+=paramext[(i+pad[l].x)*celly+(j+pad[l].y)][k];
					}
					t=t/4-paramext[i*celly+j][k];
					paramext[i*celly+j][k]+=t*0.5;
				}
			}
		}
	}
	if(m_bDbgMode)
	{
		USES_CONVERSION;
		CString fname=m_dbgPath+_T("\\paramT.txt");
		fopen_s(&m_file,T2A(fname),"w");
	}
	cvZero(destf);
	PerformStep(&CChildView::step2, grayf, &sparam);
	if(m_bDbgMode)
	{
		fclose(m_file);
		m_file=NULL;
	}
	for(int i=0;i<dest->width;i++)
	{
		for(int j=0;j<dest->height;j++)
		{
			uchar* pix=PTR_PIX(*dest,i,j);
			float* pixs=(float*)PTR_PIX(*destf,i,j);
			pix[0]=pix[1]=pix[2]=max(0,min(255,*pixs));
		}
	}
	//Clean up
	cvReleaseImage(&destf);
	cvReleaseImage(&grayf); 
	cvReleaseImage(&blk); 
	cvReleaseImage(&prefft);
	cvReleaseImage(&fft);
	cvReleaseImage(&fft2);
	delete[] params;
	delete[] paramext;
}

void CChildView::PerformStep(ProcessStep step, IplImage* image, StepParam* ptr)
{
	for(int i=0;i<ptr->cellx;i++)
	{
		for(int j=0;j<ptr->celly;j++)
		{
			CvRect rc = cvRect(i*image->width/(2*ptr->segx),j*image->height/(2*ptr->segy),ptr->blkx,ptr->blky);
			ptr->offx=ptr->offy=0;
			if(rc.x+rc.width>image->width)
			{
				ptr->offx=rc.x+rc.width-image->width;
				rc.x=image->width-rc.width;
			}
			if(rc.y+rc.height>image->height)
			{
				ptr->offy=rc.y+rc.height-image->height;
				rc.y=image->height-rc.height;
			}
			cvSetImageROI(image, rc);
			cvCopyImage(image, ptr->blk);
			cvMerge(ptr->blk, NULL, NULL, NULL, ptr->prefft);
			cvDFT(ptr->prefft, ptr->fft, CV_DXT_FORWARD);
			(this->*step)(image,i,j,ptr,rc);
		}
	}
}

void CChildView::step1(IplImage* image, int i, int j, StepParam* ptr, CvRect rc)
{
	if(m_bDbgMode)
	{
		USES_CONVERSION;
		CString num;
		num.Format(_T("\\%d_%d.jpg"),i,j);
		num=m_dbgPath+num;
		IplImage* save=cvCreateImage(cvGetSize(ptr->blk),IPL_DEPTH_8U,1);
		cvCvtScale(ptr->blk, save);
		cvSaveImage(T2A(num),save);
		cvReleaseImage(&save);
	}
	ptr->params[i*ptr->celly+j] = ComputeNormal(ptr->fft);
	if(m_adjmode>=3)
		memcpy(ptr->paramext[i*ptr->celly+j],m_CompExt,sizeof(m_CompExt));
	if(m_bDbgMode)
	{
		if(m_adjmode<=2)
		{
			Vec4 param=ptr->params[i*ptr->celly+j];
			fprintf_s(m_file, "%d_%d,%f,%f,%f,%f\n",i,j,param.x,param.y,param.z,param.t);
		}
		else
		{
			double* param=(double*)ptr->paramext[i*ptr->celly+j];
			fprintf_s(m_file, "%d_%d,%f,%f,%f,%f,%f,%f\n",i,j,param[0],param[1],param[2],param[3],param[4],param[5],param[6]);
		}
	}
}

void CChildView::step2(IplImage* image, int i, int j, StepParam* ptr, CvRect rc)
{
	if(m_adjmode>=3)
		memcpy(m_CompExt,ptr->paramext[i*ptr->celly+j],sizeof(m_CompExt));
	if(m_bDbgMode)
	{
		if(m_adjmode<=2)
		{
			Vec4 param=ptr->params[i*ptr->celly+j];
			fprintf_s(m_file, "%d_%d,%f,%f,%f,%f;",i,j,param.x,param.y,param.z,param.t);
		}
		else
		{
			double* param=(double*)ptr->paramext[i*ptr->celly+j];
			fprintf_s(m_file, "%d_%d,%f,%f,%f,%f,%f,%f;",i,j,param[0],param[1],param[2],param[3],param[4],param[5],param[6]);
		}
	}
	AdjustImage(ptr->fft,ptr->fft2,ptr->params[i*ptr->celly+j]);
	cvDFT(ptr->fft2, ptr->fft, CV_DXT_INV_SCALE);
	float fxl=(i==0?1:0);
	float fxr=(i==ptr->cellx-1?1:0);
	float fyl=(j==0?1:0);
	float fyr=(j==ptr->celly-1?1:0);
	for(int k=0;k<ptr->blkx;k++)
	for(int l=0;l<ptr->blky;l++)
	{
		int x=k+rc.x;
		int y=l+rc.y;
		float fx,fy;
		float factorx=(float)max(k-ptr->offx,0)*2/ptr->blkx;
		float factory=(float)max(l-ptr->offy,0)*2/ptr->blky;
		if(factorx<1)
			fx=(1-factorx)*fxl+factorx;
		else
			fx=(2-factorx)+(factorx-1)*fxr;
		if(factory<1)
			fy=(1-factory)*fyl+factory;
		else
			fy=(2-factory)+(factory-1)*fyr;
		float f = fx*fy;
		float* pix = (float*)PTR_PIX(*ptr->fft, k, l);
		float* pixd = (float*)PTR_PIX(*ptr->dest, x, y);
		int dc=*pix*f;
		*pixd+=dc;
	}
}

void CChildView::OnFileProcesstotal()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Image Files|*.bmp;*.jpg;*.png||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strOut = _T(".\\out");
		if(!PathFileExists(strOut))
			CreateDirectory(strOut, NULL);
		CString strPath = dlg.GetPathName();
		IplImage* image = cvLoadImage(T2A(strPath));
		if(image)
		{
			CString strTitle = strPath;
			if(strTitle.ReverseFind(_T('\\'))!=-1)
				strTitle = strTitle.Right(strTitle.GetLength()-strTitle.ReverseFind(_T('\\')));
			strOut+=strTitle;
			if(m_bDbgMode)
			{
				m_dbgPath=strOut+_T("_");
				CreateDirectory(m_dbgPath, NULL);
				m_dbg=true;
			}
			IplImage* dest = cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,3);
			ProcessTotal(image, dest);
			cvSaveImage(T2A(strOut),dest);
			cvReleaseImage(&dest);
			cvReleaseImage(&image);
			m_dbg=false;
			MessageBox(_T("Process complete"));
		}
	}
}

float SmoothStep(float x, float thresh, float width)
{
	if(x<=thresh-width/2)
		return 0;
	else if(x>=thresh+width/2)
		return 1;
	else
	{
		float scale = (x-thresh)/(width/2);
		float ret = (scale*3-powf(scale,3))/2;
		if(ret<0)ret=0;
		if(ret>1)ret=1;
		return ret;
	}
}

void Threshold(IplImage* src, IplImage* dest)
{
	IplImage* gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, gray, CV_RGB2GRAY);
	CvScalar avg,sddv;
	cvAvgSdv(gray, &avg,&sddv);
	float step_thresh = 0.25f/0.46f*sddv.val[0]/avg.val[0];
	step_thresh = min(0.3,step_thresh);

	IplImage* integ = cvCreateImage(cvSize(src->width+1, src->height+1), IPL_DEPTH_64F, 1);
	IplImage* sqinteg = cvCreateImage(cvSize(src->width+1, src->height+1), IPL_DEPTH_64F, 1);
	IplImage* destg = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvIntegral(gray, integ, sqinteg);
	int span = 20, span2 = 25;
	float scale = 2.2f;
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			int i0 = max(0,i-span),i1 = min(src->height,i+span+1);
			int j0 = max(0,j-span),j1 = min(src->width,j+span+1);
			int i02 = max(0,i-span2),i12 = min(src->height,i+span2+1);
			int j02 = max(0,j-span2),j12 = min(src->width,j+span2+1);

			int cnt = (i1-i0)*(j1-j0);
			int cnt2 = (i12-i02)*(j12-j02);

			double sump = *(double*)PTR_PIX(*integ, j1, i1)
				-*(double*)PTR_PIX(*integ, j0, i1)
				-*(double*)PTR_PIX(*integ, j1, i0)
				+*(double*)PTR_PIX(*integ, j0, i0);
			//double sqsump =  *(double*)PTR_PIX(*sqinteg, j1, i1)
			//	-*(double*)PTR_PIX(*sqinteg, j0, i1)
			//	-*(double*)PTR_PIX(*sqinteg, j1, i0)
			//	+*(double*)PTR_PIX(*sqinteg, j0, i0);
			double sump2 =  *(double*)PTR_PIX(*integ, j12, i12)
				-*(double*)PTR_PIX(*integ, j02, i12)
				-*(double*)PTR_PIX(*integ, j12, i02)
				+*(double*)PTR_PIX(*integ, j02, i02);
			float mean = sump/cnt;
			float mean2 = sump2/cnt2;
			//float sq = sqsump/cnt;

			//float stddev = sqrt(sq-mean*mean);
			float l1 = mean;//-stddev*0.3;
			float l2 = *PTR_PIX(*gray, j, i);

			float diff = l2-l1;
			float level = 250;
			if(mean2<10)mean2=10;
			//float dev=stddev/mean2;
			diff/=mean2/125;
			//float ddev=SmoothStep(dev,step_thresh,0.05f);
			float re = 255*SmoothStep(level+diff*scale/*ddev*/,160,60)*SmoothStep(l2,avg.val[0]*0.5,10);

			uchar* ptr2 = PTR_PIX(*destg, j, i);
			*ptr2 = max(0,min(re, 255));
		}
	}

	cvReleaseImage(&integ);
	cvReleaseImage(&sqinteg);
	cvReleaseImage(&gray);
	cvCvtColor(destg, dest, CV_GRAY2RGB);
	cvReleaseImage(&destg);
}

struct FillParam
{
	IplImage* src;
	IplImage* mask;
	CPropagate* prop;
	bool bw;
};

int wthresh=200;
int bthresh=100;

bool FillFunc(void* param, CvPoint pt, CvPoint ptorg, IplImage* pMask, UINT8 flags)
{
	FillParam* fparam=(FillParam*)param;
	if(fparam->prop->m_ptacc==0)
	{
		*PTR_PIX(*fparam->mask,pt.x,pt.y)=255;
		return true;
	}
	else
	{
		if(*PTR_PIX(*fparam->mask,pt.x,pt.y)!=0)
			return false;
		if(fparam->bw)
		{
			if(*PTR_PIX(*fparam->src,pt.x,pt.y)<wthresh)
				return false;
		}
		else
		{
			if(*PTR_PIX(*fparam->src,pt.x,pt.y)>bthresh)
				return false;
		}
	}
	*PTR_PIX(*fparam->mask,pt.x,pt.y)=255;
	return true;
}

void CChildView::Denoise(IplImage* src, IplImage* dest)
{
	cvCopyImage(src,dest);
	CPropagate prop;
	prop.Init(src->width,src->height,false);
	prop.SetFunc(FillFunc);
	IplImage* mask = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	cvZero(mask);
	FillParam param;
	param.src=src;
	param.mask=mask;
	param.prop=&prop;
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			if(*PTR_PIX(*mask,j,i)!=0)
				continue;
			bool bw;
			if(*PTR_PIX(*src,j,i)>wthresh && m_bFillHoles)
				bw=true;
			else if(*PTR_PIX(*src,j,i)<bthresh)
				bw=false;
			else
				continue;
			param.bw=bw;
			prop.SetPoint(cvPoint(j,i));
			while(prop.Propagate(PROP_MODE_RECT, false,&param));
			if(prop.m_ptacc<10)
			{
				for(int k=0;k<prop.m_ptacc;k++)
				{
					CvPoint pt=prop.m_bufacc[k];
					uchar* pix=PTR_PIX(*dest, pt.x, pt.y);
					pix[0]=(bw?0:255);
				}
			}
		}
	}
	cvReleaseImage(&mask);
}

void CChildView::OnFileThreshold()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Image Files|*.bmp;*.jpg;*.png||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strOut = _T(".\\out");
		if(!PathFileExists(strOut))
			CreateDirectory(strOut, NULL);
		CString strPath = dlg.GetPathName();
		IplImage* image = cvLoadImage(T2A(strPath));
		if(image)
		{
			IplImage* dest = cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,3);
			Threshold(image, dest);
			CString strTitle = strPath;
			if(strTitle.ReverseFind(_T('\\'))!=-1)
				strTitle = CString(_T("\\th_"))+strTitle.Right(strTitle.GetLength()-1-strTitle.ReverseFind(_T('\\')));
			strOut+=strTitle;
			cvSaveImage(T2A(strOut),dest);
			cvReleaseImage(&dest);
			cvReleaseImage(&image);
			MessageBox(_T("Save succeeded"));
		}
	}
}

void CChildView::OnFileDenoise()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Image Files|*.bmp;*.jpg;*.png||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strOut = _T(".\\out");
		if(!PathFileExists(strOut))
			CreateDirectory(strOut, NULL);
		CString strPath = dlg.GetPathName();
		IplImage* image = cvLoadImage(T2A(strPath));
		if(image)
		{
			IplImage* srcg = cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,1);
			cvCvtColor(image,srcg,CV_RGB2GRAY);
			IplImage* destg = cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,1);
			Denoise(srcg, destg);
			cvReleaseImage(&srcg);
			IplImage* dest = cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,3);
			cvCvtColor(destg,dest,CV_GRAY2RGB);
			cvReleaseImage(&destg);
			CString strTitle = strPath;
			if(strTitle.ReverseFind(_T('\\'))!=-1)
				strTitle = CString(_T("\\dn_"))+strTitle.Right(strTitle.GetLength()-1-strTitle.ReverseFind(_T('\\')));
			strOut+=strTitle;
			cvSaveImage(T2A(strOut),dest);
			cvReleaseImage(&dest);
			cvReleaseImage(&image);
			MessageBox(_T("Save succeeded"));
		}
	}
}

void CChildView::SupressRipple(IplImage* src, IplImage* dest)
{
	IplImage* ripple=NULL;
	IplImage* amp=ComputeAmp(src,&ripple);
	cvReleaseImage(&amp);
	for(int i=0;i<dest->width;i++)
	{
		for(int j=0;j<dest->height;j++)
		{
			float* pix=(float*)PTR_PIX(*src,i,j);
			float* pixd=(float*)PTR_PIX(*dest,i,j);
			pixd[0]=pix[0]**(double*)PTR_PIX(*ripple,i,j);
			pixd[1]=pix[1]**(double*)PTR_PIX(*ripple,i,j);
		}
	}
	cvReleaseImage(&ripple);
}

void CChildView::SupressHigh(IplImage* src, IplImage* dest)
{
	float sqrt2=sqrtf(2.0);
	float inhibitup=(m_finhibit>0.5?1:m_finhibit*2)*sqrt2;
	float inhibitdown=(m_finhibit>0.5?m_finhibit*2-1:0)*sqrt2;
	int hw=(src->width+2)/2;
	int hh=(src->height+2)/2;
	for(int i=0;i<hw;i++)
	for(int j=0;j<hh;j++)
	{
		float fx=(float)i/hw;
		float fy=(float)j/hh;
		int sx=m_rTable->width*fx;
		int sy=m_rTable->height*fy;
		sx=max(0,min(m_rTable->width-1, sx));
		sy=max(0,min(m_rTable->height-1, sy));
		float r=*(float*)PTR_PIX(*m_rTable, sx, sy);
		float factor=1;
		if(r>sqrt2-inhibitup)
		{
			factor=(inhibitup==inhibitdown?0:(r-sqrt2+inhibitup)/(inhibitup-inhibitdown));
			if(factor>1)
				factor=0;
			else
			{
				int sf=factor*m_CosineTable->width;
				sf=max(0,min(m_CosineTable->width-1,sf));
				factor=*(float*)PTR_PIX(*m_CosineTable, sf, 0);
			}
		}
		int x[2]={i,src->width-i};
		int y[2]={j,src->height-j};
		for(int k=0;k<2;k++)
		for(int l=0;l<2;l++)
		{
			if(x[k]==src->width||y[l]==src->height)
				continue;
			float* pixs = (float*)PTR_PIX(*src,x[k],y[l]);
			float* pix = (float*)PTR_PIX(*dest,x[k],y[l]);
			pix[0]=pixs[0]*factor;
			pix[1]=pixs[1]*factor;
		}
	}
}

void CChildView::OnFileThoroughprocess()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Image Files|*.bmp;*.jpg;*.png||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strOut = _T(".\\out");
		if(!PathFileExists(strOut))
			CreateDirectory(strOut, NULL);
		CString strPath = dlg.GetPathName();
		IplImage* image = cvLoadImage(T2A(strPath));
		if(image)
		{
			IplImage* dest = cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,3);
			ProcessTotal(image, dest);
			swap(image,dest);
			Threshold(image, dest);
			swap(image,dest);
			IplImage* srcg = cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,1);
			cvCvtColor(image,srcg,CV_RGB2GRAY);
			IplImage* destg = cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,1);
			Denoise(srcg, destg);
			cvReleaseImage(&srcg);
			cvCvtColor(destg,dest,CV_GRAY2RGB);
			cvReleaseImage(&destg);
			CString strTitle = strPath;
			if(strTitle.ReverseFind(_T('\\'))!=-1)
				strTitle = CString(_T("\\out_"))+strTitle.Right(strTitle.GetLength()-1-strTitle.ReverseFind(_T('\\')));
			strOut+=strTitle;
			cvSaveImage(T2A(strOut),dest);
			cvReleaseImage(&dest);
			cvReleaseImage(&image);
			MessageBox(_T("Save succeeded"));
		}
	}
}

extern Preprocess_Callback *pcallback;

void CChildView::OnFileDll()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Image Files|*.bmp;*.jpg;*.png||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strOut = _T(".\\out");
		if(!PathFileExists(strOut))
			CreateDirectory(strOut, NULL);
		CString strPath = dlg.GetPathName();
		CString strTitle = strPath;
		if(strTitle.ReverseFind(_T('\\'))!=-1)
			strTitle = CString(_T("\\dll_"))+strTitle.Right(strTitle.GetLength()-1-strTitle.ReverseFind(_T('\\')));
		strOut+=strTitle;
		if(Preprocess(T2A(strPath),T2A(strOut),m_seg,m_overlap,pcallback, m_bSinglePass?(PRE_NO_CLOSE|PRE_ADJ_SINGLEPASS):PRE_NO_CLOSE))
			MessageBox(_T("Dll exec suceeded"));
		else
			MessageBox(_T("Dll exec failed"));
	}
}

void CChildView::ComputeAxisIntensity(IplImage* src)
{
	IplImage* integ = cvCreateImage(cvSize(src->width+1,src->height+1),IPL_DEPTH_64F,1);
	cvIntegral(src, integ);
    if(m_xIntense)
		delete[] m_xIntense;
	if(m_yIntense)
		delete[] m_yIntense;
	m_xIntense = new float[src->width];
	m_yIntense = new float[src->height];
	m_nx = src->width;
	m_ny = src->height;
	for(int i=0;i<src->width;i++)
	{
		float x=*(double*)PTR_PIX(*integ, i+1,src->height)
			-*(double*)PTR_PIX(*integ, i, src->height);
		m_xIntense[i]=x/src->height;
	}
	for(int i=0;i<src->height;i++)
	{
		float y=*(double*)PTR_PIX(*integ, src->width, i+1)
			-*(double*)PTR_PIX(*integ, src->width, i);
		m_yIntense[i]=y/src->width;
	}
	ComputeDeviation();
}

void CChildView::OnFileShowaxisdistribution()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Image Files|*.bmp;*.jpg;*.png||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strPath = dlg.GetPathName();
		IplImage* image = cvLoadImage(T2A(strPath));
		IplImage* gray = cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,1);
		cvCvtColor(image,gray,CV_RGB2GRAY);
		ComputeAxisIntensity(gray);
		cvReleaseImage(&gray);
		cvReleaseImage(&m_cutimage);
		CvRect rc=cvRect(m_leftcut,m_topcut,m_rightcut-m_leftcut,m_botcut-m_topcut);
		m_cutimage=cvCreateImage(cvSize(rc.width,rc.height),IPL_DEPTH_8U,3);
		cvSetImageROI(image, rc);
		cvCopyImage(image, m_cutimage);
		cvReleaseImage(&image);
		Draw();
	}
}

void CChildView::ComputeDeviation()
{
	if(m_xIntense == NULL || m_yIntense == NULL)
		return;
	if(m_xDev)
		delete[] m_xDev;
	if(m_yDev)
		delete[] m_yDev;
	m_xDev = new float[m_nx];
	m_yDev = new float[m_ny];
	m_topcut=0;
	m_botcut=m_ny;
	m_leftcut=0;
	m_rightcut=m_nx;
	int cut;
	if(ComputeDev(m_xIntense, m_xDev, (m_nx+1)/2, 1, &cut))
		m_leftcut = cut;
	if(ComputeDev(m_xIntense+m_nx-1, m_xDev+m_nx-1, (m_nx+1)/2, -1, &cut))
		m_rightcut = m_nx-1+cut;
	if(ComputeDev(m_yIntense, m_yDev, (m_ny+1)/2, 1, &cut))
		m_topcut = cut;
	if(ComputeDev(m_yIntense+m_ny-1, m_yDev+m_ny-1, (m_ny+1)/2, -1, &cut))
		m_botcut = m_ny-1+cut;
}

bool CChildView::ComputeDev(float* intense,float* dev, int n, int dir, int* cut)
{
	float X=0,Y=0,YY=0,XY=0,XX=0,N=0;
	float deviation=0;
	float predict=0;
	bool bpredict=false;
	int lowpos=0;
	int cutpos=0;
	bool blow=false;
	bool bcut=false;
	bool bcomplete=false;
	for(int i=0;dir>0?i<n:i>-n;i+=dir)
	{
		float x=i;
		float y=intense[i];
		X+=x;
		Y+=y;
		YY+=y*y;
		XY+=x*y;
		XX+=x*x;
		N+=1;
		if(bpredict)
		{
			dev[i]=(y-predict)/max(1,deviation);
			if(!blow && fabs(dev[i])>3.0f)
			{
				blow=true;
				lowpos=i;
			}
			if(!bcomplete && !bcut && fabs(dev[i])>3.5f)
			{
				bcomplete=true;
				cutpos=i;
				if(abs(i)<n*0.66f&&blow&&abs(lowpos)>=0.8f*abs(cutpos))
					bcut=true;
			}
		}
		else
		{
			dev[i]=0;
		}
		if(N>1)
		{
			float denum = XX*N-X*X;
			float k=(XY*N-Y*X)/denum;
			float b=(XX*Y-XY*X)/denum;
			deviation = (YY+k*k*XX+N*b*b-2*k*XY-2*b*Y+2*k*b*X)/N;
			predict=(i+dir)*k+b;
			bpredict=true;
		}
	}
	if(bcut&&bcomplete)
	{
		*cut = cutpos;
		return true;
	}
	else
		return false;
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if(m_cutimage&&m_bShowCut)
	{

	}
	else if(m_xDev && m_yDev)
	{
		m_Pos = point;
		Draw();
	}
	CWnd::OnMouseMove(nFlags, point);
}

void CChildView::OnFileDllcutedge()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Image Files|*.bmp;*.jpg;*.png||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strPath = dlg.GetPathName();
		IplImage* image = cvLoadImage(T2A(strPath));
		IplImage* gray = cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,1);
		cvCvtColor(image,gray,CV_RGB2GRAY);
		CvRect rc=CutEdge(gray);
		cvReleaseImage(&gray);
		cvReleaseImage(&m_cutimage);
		m_cutimage=cvCreateImage(cvSize(rc.width,rc.height),IPL_DEPTH_8U,3);
		cvSetImageROI(image, rc);
		cvCopyImage(image, m_cutimage);
		cvReleaseImage(&image);
		m_bShowCut=true;
		CString strOut=strPath;
		int iDot=strOut.ReverseFind('.');
		if(iDot>0)
			strOut=strOut.Left(iDot)+_T("_cut")+strOut.Right(strOut.GetLength()-iDot);
		else
			strOut=strOut+_T("_cut");
		cvSaveImage(T2A(strOut), m_cutimage);
		Draw();
	}
}

CCriticalSection g_cs[4];
HANDLE g_hEvent[4];
struct ThreadData
{
	HANDLE hThread;
};

void* StartThread(unsigned int (*threadfunc)(void*), void* param)
{
	ThreadData* th=new ThreadData;
	th->hThread=AfxBeginThread(threadfunc, param)->m_hThread;
	return th;
}

void Lock(int i)
{
	g_cs[i].Lock();
}

void Unlock(int i)
{
	g_cs[i].Unlock();
}

void Wait(vector<void*>& v)
{
	HANDLE h[4];
	for(int i=0;i<(int)v.size();i++)
		h[i]=((ThreadData*)v[i])->hThread;
	WaitForMultipleObjects(v.size(), g_hEvent, TRUE, INFINITE);
	//WaitForMultipleObjects(v.size(), h, TRUE, INFINITE);
	for(int i=0;i<(int)v.size();i++)
	{
		ThreadData* th=(ThreadData*)v[i];
		//CloseHandle(th->hThread);
		delete th;
		ResetEvent(g_hEvent[i]);
	}
}

void Signal(int i)
{
	SetEvent(g_hEvent[i]);
}

Preprocess_Callback Callback={StartThread, Lock, Unlock, Wait, Signal, NULL, 4, false};
Preprocess_Callback* pcallback=NULL;

void CChildView::OnCore1()
{
	// TODO: Add your command handler code here
	pcallback=NULL;
	m_ncore=1;
}

void CChildView::OnCore2()
{
	// TODO: Add your command handler code here
	Callback.core=2;
	pcallback=&Callback;
	m_ncore=2;
}

void CChildView::OnCore4()
{
	// TODO: Add your command handler code here
	Callback.core=4;
	pcallback=&Callback;
	m_ncore=4;
}

void CChildView::OnUpdateCore1(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_ncore==1);
}

void CChildView::OnUpdateCore2(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_ncore==2);
}

void CChildView::OnUpdateCore4(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_ncore==4);
}

void CChildView::ComputeGradIntense(IplImage* src, Vec2 dir, float** intense, int* n)
{
	dir=Vec2(0,1);
	float a=(float)src->width/2;
	float b=(float)src->height/2;
	int nlength,nlengthx;
	if(src->width!=src->height)
	{
		nlength=(int)sqrtf(a*a*dir.x*dir.x+b*b*dir.y*dir.y);
		nlengthx=(int)sqrtf(a*a*dir.y*dir.y+b*b*dir.x*dir.x);
	}
	else
		nlength=nlengthx=src->width/2;
	nlength=(src->height+1)/2;
	nlengthx=src->width;
	Vec2 dirx(dir.y,-dir.x);
	float* value=new float[nlength];
	memset(value, 0, nlength*sizeof(float));
	for(int i=0;i<nlength;i++)
	{
		for(int j=0/*-nlengthx*/;j<nlengthx;j++)
		{
			//float v=Sample(src, i*dir+j*dirx, WRAP_TYPE_REPEAT, TEX_FILTER_LINEAR, false).x;
			value[i]+=*(double*)PTR_PIX(*src,j,i);//v;
		}
		value[i]/=/*2**/nlengthx;
	}
	*n=nlength;
	*intense=value;
}

void CChildView::RotateImage(IplImage* src, IplImage* dest, Vec2 rot, Vec2 shift)
{
	for(int i=0;i<dest->width;i++)
	for(int j=0;j<dest->height;j++)
	{
		float* pix=(float*)PTR_PIX(*dest, i, j);
		Vec2 p = i*rot+j*Vec2(-rot.y,rot.x)+shift;
		Vec3 s=Sample(src, p, WRAP_TYPE_BORDER, TEX_FILTER_LINEAR, false, Vec3(255,255,255));
		pix[0]=s.x,pix[1]=s.y;
	}
}

void CChildView::EstimateKernel(float* v, int n, float* a, float* k, float* ample, float* t, float* theta, float* noise, float* center, float* casc, float* aasc)
{
	float A=0,B=0,K=0;
	const int max_iter_times=4;
	for(int i=0;i<max_iter_times;i++)
	{
		if(!Estimate1(v, n, &A, &B, &K))
			break;
	}
	float Ample=expf(A);
	float Noise=B*Ample;
	float Power=K;
	float *v2=new float[n];
	for(int i=0;i<n;i++)
		v2[i]=(i==0?0:(v[i]-Noise)*powf(i,-Power));
	*a=Ample,*k=Power,*noise=Noise;
	Estimate2(v2, n, t, theta, ample, center, casc, aasc);
	delete[] v2;
}

bool CChildView::Estimate1(float* v, int n, float* a,float* b, float* k)
{
	float K=*k,A,B;
	if(K==0.0f)
	{
		float XX=0,X=0,YX=0,Y=0,N=0;
		for(int i=0;i<n;i++)
		{
			float x=logf(i);
			float y=logf(v[i]);
			if(i==0||v[i]<=0)
				continue;
			XX+=x*x;
			X+=x;
			YX+=y*x;
			Y+=y;
			N+=1.0f;
		}
		float det=XX*N-X*X;
		A=(XX*Y-X*YX)/det;
		B=0;
		K=(YX*N-X*Y)/det;
	}
	else
	{
		float XX=0,XY=0,YY=0,X=0,Y=0,ZX=0,ZY=0,Z=0,N=0;
		for(int i=0;i<n;i++)
		{
			float x=logf(i);
			float y=powf(i,-K);
			float z=logf(v[i]);
			if(i==0||v[i]<=0)
				continue;
			XX+=x*x;
			XY+=x*y;
			YY+=y*y;
			X+=x;
			Y+=y;
			ZX+=z*x;
			ZY+=z*y;
			Z+=z;
			N+=1.0f;
		}
		Mat mat(XX,XY,X,XY,YY,Y,X,Y,N);
		Vec3 vec(ZX,ZY,Z);
		Vec3 result=vec*mat.inv();
		K=result.x,B=result.y,A=result.z;
	}
	if(B>=0.0f)
	{
		*a=A,*b=B,*k=K;
		return true;
	}
	return false;
}

void CChildView::Estimate2(float* v, int n, float* t, float* theta, float* a, float* center, float* casc, float* aasc)
{
	struct Peak
	{
		int i;
		bool bhigh;
		float certainty;
	};
	vector<int> highs,lows;
	vector<Peak> peaks; 
	float high=-FLT_MAX;
	float low=FLT_MAX;
	int ihigh=-1,ilow=-1;
	int ihs=0,ils=0;
	int near_span=0.05*n;
	int far_span=0.4*n;
	for(int i=0;i<n;i++)
	{
		if(peaks.size()>0)
		{
			Peak p=peaks[peaks.size()-1];
			int s=(p.bhigh?highs[p.i]:lows[p.i]);
			if(i-far_span>s)
			{
				bool b=true;
				if(peaks.size()>1)
				{
					Peak p2=peaks[peaks.size()-2];
					int s2=(p2.bhigh?highs[p2.i]:lows[p2.i]);
					if(s-s2<far_span)
						b=false;
				}
				if(b)
				{
					peaks.pop_back();
					p.bhigh?highs.pop_back():lows.pop_back();
				}
			}
		}
		if(ihigh!=-1&&i-near_span>ihigh)
		{
			if(ihigh!=ihs)
			{
				Peak p={highs.size(), true};
				highs.push_back(ihigh);
				peaks.push_back(p);
			}
			ihs=i;
			ihigh=-1;
			high=-FLT_MAX;
		}
		if(ilow!=-1&&i-near_span>ilow)
		{
			if(ilow!=ils)
			{
				Peak p={lows.size(), false};
				lows.push_back(ilow);
				peaks.push_back(p);
			}
			ils=i;
			ilow=-1;
			low=FLT_MAX;
		}
		if(v[i]>high)
		{
			high=v[i];
			ihigh=i;
		}
		if(v[i]<low)
		{
			low=v[i];
			ilow=i;
		}
	}
	m_highs=highs;
	m_lows=lows;
	int nslope=0,nrep=0;
	float T=0,P=0,V=0,A=0,Theta=0,Center=0,CenterAsc=0,AAsc=0;
	float vh=0,vl=0,vha=0,vla=0;
	vector<Peak> rfpeaks;
	for(int i=0;i<(int)peaks.size();i++)
	{
		nrep++;
		Peak& p=peaks[i];
		int s=(p.bhigh?highs[p.i]:lows[p.i]);
		P+=s;
		V+=v[s];
		if(i==peaks.size()-1||peaks[i+1].bhigh!=p.bhigh)
		{
			float mP=P/nrep;
			float mV=V/nrep;
			Peak peak;
			peak.bhigh=p.bhigh;
			peak.i=mP;
			peak.certainty=mV;
			rfpeaks.push_back(peak);
			nrep=0;
			P=0;
			V=0;
		}
	}
	if(rfpeaks.size()>2)
	{
		float XX=0,XY=0,X=0,Y=0,N=0;
		for(int i=0;i<(int)rfpeaks.size();i++)
		{
			float x=i;
			float y=rfpeaks[i].i;
			XX+=x*x;
			XY+=x*y;
			X+=x;
			Y+=y;
			N+=1.0f;
		}
		float det=XX*N-X*X;
		if(det!=0)
		{
			T=(XY*N-X*Y)/det;
			Theta=(XX*Y-X*XY)/det;
		}
	}
	if(T>0)
	{
		for(int i=0;i<(int)rfpeaks.size();i++)
			Center+=rfpeaks[i].certainty;
		Center/=rfpeaks.size();
/*
		for(int i=0;i<rfpeaks.size();i++)
		{
			if((rfpeaks[i].bhigh&&rfpeaks[i].certainty<=Center)
				||(!rfpeaks[i].bhigh&&rfpeaks[i].certainty>=Center))
			{
				rfpeaks.erase(rfpeaks.begin()+i);
				i--;
			}
		}
*/
		int nh=0,nl=0;
		float xxh=0,xh=0,vxh=0,vnh=0,xxl=0,xl=0,vxl=0,vnl=0;
		for(int i=0;i<(int)rfpeaks.size();i++)
		{
			if(rfpeaks[i].bhigh&&rfpeaks[i].certainty>Center)
			{
				float v=rfpeaks[i].certainty;
				float x=rfpeaks[i].i;
				xxh+=x*x;
				xh+=x;
				nh++;
				vxh+=v*x;
				vnh+=v;
			}
			else if(!rfpeaks[i].bhigh&&rfpeaks[i].certainty<Center)
			{
				float v=rfpeaks[i].certainty;
				float x=rfpeaks[i].i;
				xxl+=x*x;
				xl+=x;
				nl++;
				vxl+=v*x;
				vnl+=v;
			}
		}
		if(nh>0&&nl>0)
		{
			if(nh==1)
			{
				vh=vnh;
				vha=0;
			}
			else
			{
				float det = xxh*nh-xh*xh;
				vha = (vxh*nh-xh*vnh)/det;
				vh = (xxh*vnh-xh*vxh)/det;
			}
			if(nl==1)
			{
				vl=vnl;
				vla=0;
			}
			else
			{
				float det = xxl*nl-xl*xl;
				vla = (vxl*nl-xl*vnl)/det;
				vl = (xxl*vnl-xl*vxl)/det;
			}
			Center=(vh+vl)/2;
			A=(vh-vl)/2;
			CenterAsc=(vha+vla)/2;
			AAsc=(vha-vla)/2;
			vnh/=nh;
			vnl/=nl;
		}
		float c=(vnh+vnl)/2;
		float a2=(vnh-vnl)/2;
		if(a2<=0)
		{
			T=0;
			A=0;
			Theta=0;
			Center=0;
			CenterAsc=0;
			AAsc=0;
		}
		else
		{
			for(int i=0;i<(int)rfpeaks.size();i++)
			{
				if(rfpeaks[i].bhigh)
				{
					if(rfpeaks[i].certainty>vnh)
						rfpeaks[i].certainty=1;
					else if(rfpeaks[i].certainty<c)
						rfpeaks[i].certainty=0;
					else
						rfpeaks[i].certainty=(rfpeaks[i].certainty-c)/a2;
				}
				else
				{
					if(rfpeaks[i].certainty<vnl)
						rfpeaks[i].certainty=1;
					else if(rfpeaks[i].certainty>c)
						rfpeaks[i].certainty=0;
					else
						rfpeaks[i].certainty=-(rfpeaks[i].certainty-c)/a2;
				}
			}
			if(!rfpeaks[0].bhigh)
				A=-A;
			//the initial fit function is Center+A*cos((x-Theta)/T*PI)
			//an iterative method to search for the maximum probable fit function 
			float alpha=CV_PI/T,beta=-Theta*CV_PI/T;
			float probably=0.0f,step_alpha=0.0f,step_beta=1.0f/(float)rfpeaks.size(),min_alpha=alpha/2,step=1.0f;
			for(int i=0;i<(int)rfpeaks.size();i++)
				step_alpha+=rfpeaks[i].i;
			step_alpha=1.0f/step_alpha;
			Vec2 dp(0,0);
			int iter_times=0,max_iter_times=10;
			do 
			{
				float _alpha = alpha+step*step_alpha*dp.x, _beta = beta+step*step_beta*dp.y;
				if(_alpha<min_alpha)
					_alpha=min_alpha;
				Vec2 _dp(0,0);
				float _probably=0;
				for(int i=0;i<(int)rfpeaks.size();i++)
				{
					float c=(rfpeaks[i].bhigh?1:-1)*rfpeaks[i].certainty;
					_probably+=c*cosf(_alpha*rfpeaks[i].i+_beta);
					float m=-c*sinf(_alpha*rfpeaks[i].i+_beta);
					_dp.x+=m*rfpeaks[i].i;
					_dp.y+=m;
				}
				if(_probably>probably)
				{
					alpha=_alpha,beta=_beta;
					probably=_probably;
					dp=_dp;
				}
				else
					step/=2;
				iter_times++;
			} while (step>0.0001&&iter_times<max_iter_times);
			T=CV_PI/alpha,Theta=-beta/alpha;
		}
	}
	*a=A,*theta=Theta,*t=T,*center=Center,*casc=CenterAsc,*aasc=AAsc;
}

void CChildView::DebugAdjust(IplImage* src)
{
	IplImage* amp=ComputeAmp(src);

	if(m_GradIntense)
	{
		delete[] m_GradIntense;
		m_GradIntense=NULL;
		m_nGradIntense=0;
	}
	ComputeGradIntense(amp, m_vDir, &m_GradIntense, &m_nGradIntense);

	IplImage* dest = cvCreateImage(cvGetSize(src),IPL_DEPTH_32F,2);
	AdjustMotion(src, dest);

	amp=ComputeAmp(dest);

	if(m_GradIntenseAdj)
	{
		delete[] m_GradIntenseAdj;
		m_GradIntenseAdj=NULL;
		m_nGradIntense=0;
	}
	ComputeGradIntense(amp, m_vDir, &m_GradIntenseAdj, &m_nGradIntense);

	cvReleaseImage(&amp);
	cvReleaseImage(&dest);

	for(int i=0;i<m_nGradIntense;i++)
	{
		float a=m_GradIntense[i];
		float b=m_GradIntenseAdj[i];
		float f0=(cosf(-m_kTheta*CV_PI/m_kT)*m_kRippleAmple+m_kCenter)/m_kBaseAmple;
		float f1=m_kBaseAmple*powf(i,m_kBasePower)+m_kNoise;
		float f2=(cosf((i-m_kTheta)*CV_PI/m_kT)*(m_kRippleAmple+i*m_kAmpleAsc)+(m_kCenter+i*m_kCenterAsc))*powf(i,m_kBasePower)+m_kNoise;
		float f=f2/f1/f0;
		if(i==0)
			f=1.0f;
		f=max(0.01,f);
		float f3=f/(f*f+0.01f);
		a*=f3;
		a=0,b=0;
	}
}

void CChildView::OnCoreSinglepass()
{
	// TODO: Add your command handler code here
	m_bSinglePass=!m_bSinglePass;
}

void CChildView::OnUpdateCoreSinglepass(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if(m_bSinglePass)
		pCmdUI->SetCheck(BST_CHECKED);
	else
		pCmdUI->SetCheck(BST_UNCHECKED);
}


void CChildView::OnFileResizeimage()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Image Files|*.bmp;*.jpg;*.png||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strPath = dlg.GetPathName();
		IplImage* image=cvLoadImage(T2A(strPath));
		float area=image->width*image->height;
		const int stdlen = 800;
		float scale=stdlen/sqrt(area);
		int w=max(1,image->width*scale);
		int h=max(1,image->height*scale);
		IplImage* tmp=image;
		image = cvCreateImage(cvSize(w, h), image->depth, image->nChannels);
		cvResize(tmp, image);
		cvReleaseImage(&tmp);
		CString strOut=strPath;
		int iDot=strOut.ReverseFind('.');
		if(iDot>0)
			strOut=strOut.Left(iDot)+_T("_resize")+strOut.Right(strOut.GetLength()-iDot);
		else
			strOut=strOut+_T("_resize");
		cvSaveImage(T2A(strOut), image);
		cvReleaseImage(&image);
		MessageBox(_T("Resize Done"));
	}
}
