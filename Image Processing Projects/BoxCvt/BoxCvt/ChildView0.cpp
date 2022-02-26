
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "BoxCvt.h"
#include "ChildView.h"
#include "DlgSize.h"
#include "Image.h"
#include "Propagate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <fstream>
using namespace std;
// CChildView
#define DEFAULT_INHIBIT 0.00f
CChildView::CChildView():m_size(800),m_Image(NULL),m_ImageConv(NULL),m_scale(1.0f),m_scanl(0),m_scanlx(0),m_bShow(false),m_bShowAdjust(false),m_adj(0,0),m_adjmode(0),m_finhibit(DEFAULT_INHIBIT),m_bDbgMode(false),m_file(NULL),m_dbg(false)
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
	if(dlg.DoModal() == IDOK)
	{
		m_size = atoi(T2A(dlg.m_size));
		if(m_size<1)m_size=1;
		if(m_size>5000)m_size=5000;
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
	bool tmp=m_bShowAdjust;
	m_bShowAdjust=false;
	m_Compensation = ComputeNormal(m_Image);
	cvReleaseImage(&m_ImageConv);
	m_ImageConv = cvCreateImage(cvGetSize(m_Image),IPL_DEPTH_32F,2);
	AdjustImage(m_Image, m_ImageConv, m_Compensation);
	m_bShowAdjust=true;
	m_CompensationT = ComputeNormal(m_ImageConv);
	m_bShowAdjust=tmp;
}

void CChildView::AdjustImage(IplImage* src,IplImage* dest, Vec4 comp)
{
	Vec3 lvl(-0.45f,0.f,0.f);
	float max_adj=0.2f;
	float max_adj2=0.4f;
	float max_adj3=0.5f;
	float slopelvl=-0.15f;
	if(m_adjmode>=2)
	{
		if(m_adjmode==2)
			lvl = Vec3(-1.05f,1.0f,0.0f);
		else
			lvl = Vec3(-0.85f,1.0f,0.0f);
	}
	double diffx;
	double diffy;
	double diffz;
	double diffk;
	double slopex;
	double slopey;
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
				if(i>=hw*(1-m_finhibit)||j>=hh*(1-m_finhibit))
					f=f2=0;
			}
			else if(m_adjmode==3||m_adjmode==4)
			{
				f=powf(i*i*lvla+i*j*lvlb+j*j*lvlc,diffk/2)/llvl;
				f2=powf(i*i*lvla-i*j*lvlb+j*j*lvlc,diffk/2)/llvl;
				float f3=max(1,powf(i,slopex))*max(1,powf(j,slopey));
				f*=f3,f2*=f3;
				if(i==0&&j==0)
					f=f2=1;
				if(i>=hw*(1-m_finhibit)||j>=hh*(1-m_finhibit))
					f=f2=0;
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
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

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
	// TODO: Add your message handler code here
}

void CChildView::Draw()
{
	CClientDC dc(this);
	if(!m_Image)
	{
		CRect rcClient;
		GetClientRect(rcClient);
		dc.FillSolidRect(rcClient, RGB(255,255,255));
		CString strDisp;
		strDisp.Format(_T("freguency model:%d"),m_adjmode);
		dc.TextOut(0,0,strDisp);
		if(m_bDbgMode)
			dc.TextOut(0,60,_T("debug mode:on"));
		return;
	}
	CRect rcClient;
	GetClientRect(rcClient);
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rcClient.Width(),rcClient.Height());
	CBitmap* oldbmp = dcMem.SelectObject(&bmp);
	Draw(&dcMem);
	dc.BitBlt(0,0,rcClient.Width(),rcClient.Height(),&dcMem,0,0,SRCCOPY);
	dcMem.SelectObject(oldbmp);
	dcMem.DeleteDC();
	bmp.DeleteObject();
}

void CChildView::Draw(CDC* pDC)
{
	IplImage* sImage;
	if(m_bShowAdjust)
		sImage = m_ImageConv;
	else
		sImage = m_Image;
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
	b = true;
	for(int x=0;x<sImage->width;x++)
	{
		double ac=0;
		for(int i=m_scanl-5;i<m_scanl+6;i++)
		for(int j=x-5;j<x+6;j++)
		{
			int x2=j;
			if(x2<0)x2+=sImage->width;
			if(x2>=sImage->width)x2-=sImage->width;
			int y2=i;
			if(y2<0)y2+=sImage->height;
			if(y2>=sImage->height)y2-=sImage->height;
			float* pix = (float*)PTR_PIX(*sImage, x2, y2);
			ac+=sqrt((pow(pix[0],2)+pow(pix[1],2))/2);
		}
		ac/=11*11;
		xc[x]=ac;
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
		double ac=0;
		for(int i=m_scanlx-5;i<m_scanlx+6;i++)
		for(int j=x-5;j<x+6;j++)
		{
			int x2=j;
			if(x2<0)x2+=sImage->height;
			if(x2>=sImage->height)x2-=sImage->height;
			int y2=i;
			if(y2<0)y2+=sImage->width;
			if(y2>=sImage->width)y2-=sImage->width;
			float* pix = (float*)PTR_PIX(*sImage, y2, x2);
			ac+=sqrt((pow(pix[0],2)+pow(pix[1],2))/2);
		}
		ac/=11*11;
		yc[x]=ac;
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
	delete[] xc;
	delete[] yc;
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
		case 'L':
			m_bDbgMode = !m_bDbgMode;
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
	case 'L':
		m_bDbgMode = !m_bDbgMode;
		break;
	}
	if(nChar=='W'||nChar=='S'||nChar=='E'||nChar=='D'||nChar=='R'||nChar=='T'||nChar=='C'||nChar=='V')
	{
		cvReleaseImage(&m_ImageConv);
		m_ImageConv = cvCreateImage(cvGetSize(m_Image),IPL_DEPTH_32F,2);
		bool tmp=m_bShowAdjust;
		m_bShowAdjust=false;
		m_Compensation = ComputeNormal(m_Image);
		AdjustImage(m_Image, m_ImageConv, m_Compensation);
		m_bShowAdjust=true;
		m_CompensationT=ComputeNormal(m_ImageConv);
		m_bShowAdjust=tmp;
	}
	if(m_scanl<0)m_scanl=0;
	if(m_scanl>=m_Image->height)m_scanl=m_Image->height-1;
	if(m_scanlx<0)m_scanlx=0;
	if(m_scanlx>=m_Image->width)m_scanlx=m_Image->width-1;
	if(m_finhibit<0)m_finhibit=0;
	if(m_finhibit>1)m_finhibit=1;
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

IplImage* CChildView::ComputeAmp(IplImage* image)
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
	for(int x=0;x<image->width;x++)
	for(int y=0;y<image->height;y++)
	{
		double ac=0;
		double* tl=(double*)PTR_PIX(*integ,x,y);
		double* tr=(double*)PTR_PIX(*integ,x+11,y);
		double* bl=(double*)PTR_PIX(*integ,x,y+11);
		double* br=(double*)PTR_PIX(*integ,x+11,y+11);
		ac=*br-*bl-*tr+*tl;
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
		*(double*)PTR_PIX(*amp, x, y) = ac;
	}
	cvReleaseImage(&integ);
	return amp;
}

Vec4 CChildView::ComputeNormal(IplImage* image)
{
	IplImage* amp=ComputeAmp(image);
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
			if(m_adjmode<2)
			{
				float x=log((double)i/(image->width/2))*20+image->width/2;
				float y=log((double)j/(image->height/2))*20+image->height/2;
				float t=log(*(double*)PTR_PIX(*amp, i, j))*20;
				if(*(double*)PTR_PIX(*amp, i, j)<=0.0)
					continue;
				Tx+=t*x;
				Ty+=t*y;
				T+=t;
				XY+=x*y;
				X+=x;
				Y+=y;
				X2+=x*x;
				Y2+=y*y;
				n++;
			}
			else if(m_adjmode==2)
			{
				Vec2 v(i,j);
				float l=v.length();
				Vec2 vn=v.normalize();
				float x=log(l);
				float y=powf(vn.x,2)-powf(vn.y,2);
				float z=vn.x*vn.y;
				float t=log(*(double*)PTR_PIX(*amp, i, j));
				//logf(powf(1.6*i*i+1.8*j*j+0.2*i*j, -3.7/2));
				if(*(double*)PTR_PIX(*amp, i, j)<=0.0)
					continue;
				Tx+=t*x;
				Ty+=t*y;
				Tz+=t*z;
				T+=t;
				X2+=x*x;
				Y2+=y*y;
				Z2+=z*z;
				XY+=x*y;
				XZ+=x*z;
				YZ+=y*z;
				X+=x;
				Y+=y;
				Z+=z;
				n++;
			}
			else if(m_adjmode==3)
			{
				Vec2 v(i,j);
				float l=v.length();
				Vec2 vn=v.normalize();
				float co[6];
				co[0]=log(l);
				co[1]=powf(vn.x,2)-powf(vn.y,2);
				co[2]=vn.x*vn.y;
				co[3]=logf(i)+logf(j);
				co[4]=1.0f;
				co[5]=log(*(double*)PTR_PIX(*amp, i, j));
				if(*(double*)PTR_PIX(*amp, i, j)<=0.0)
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
			}
			else if(m_adjmode==4)
			{
				Vec2 v(i,j);
				float l=v.length();
				Vec2 vn=v.normalize();
				float co[7];
				co[0]=log(l);
				co[1]=powf(vn.x,2)-powf(vn.y,2);
				co[2]=vn.x*vn.y;
				co[3]=logf(i);
				co[4]=logf(j);
				co[5]=1.0f;
				co[6]=log(*(double*)PTR_PIX(*amp, i, j));
				if(*(double*)PTR_PIX(*amp, i, j)<=0.0)
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
		double a=(1+va.y*2/va.x);//*e;
		double b=va.z*2/va.x;//*e;
		double c=(1-va.y*2/va.x);//*e;
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
			comp[1]=(1+co[1]*2/co[0]);
			comp[2]=co[2]*2/co[0];
			comp[3]=(1-co[1]*2/co[0]);
			comp[4]=co[3];
		}
		else
		{
			comp[0]=co[0];
			comp[1]=(1+co[1]*2/co[0]);
			comp[2]=co[2]*2/co[0];
			comp[3]=(1-co[1]*2/co[0]);
			comp[4]=co[3];
			comp[5]=co[4];
		}
	}
	return vret;
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
	int deb2[200];
	ZeroMemory(deb2,sizeof(deb2));
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
			double sqsump =  *(double*)PTR_PIX(*sqinteg, j1, i1)
				-*(double*)PTR_PIX(*sqinteg, j0, i1)
				-*(double*)PTR_PIX(*sqinteg, j1, i0)
				+*(double*)PTR_PIX(*sqinteg, j0, i0);
			double sump2 =  *(double*)PTR_PIX(*integ, j12, i12)
				-*(double*)PTR_PIX(*integ, j02, i12)
				-*(double*)PTR_PIX(*integ, j12, i02)
				+*(double*)PTR_PIX(*integ, j02, i02);
			float mean = sump/cnt;
			float mean2 = sump2/cnt2;
			float sq = sqsump/cnt;

			float stddev = sqrt(sq-mean*mean);
			float l1 = mean-stddev*0.3;
			float l2 = *PTR_PIX(*gray, j, i);

			float diff = l2-l1;
			float level = 250;
			if(mean2<10)mean2=10;
			float dev=stddev/mean2;
			deb2[max(0,min(199,(int)(dev*100)))]++;
			diff/=mean2/125;
			float ddev=SmoothStep(dev,step_thresh,0.05f);
			float re = 255*SmoothStep(level+diff*scale*ddev,100,60)*SmoothStep(l2,avg.val[0]*0.5,10);

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

void Denoise(IplImage* src, IplImage* dest)
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
			if(*PTR_PIX(*src,j,i)>wthresh)
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
