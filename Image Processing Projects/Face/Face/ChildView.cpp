
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "Face.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define sqr(x) ((x)*(x))

#define MOUTH_CROSS 
#define DIM_FACE_IMG 400
#define CLASS_THRESH 1.2

#define EYE_GRID_X  0//70
#define EYE_GRID_Y  80
#define EYE_GRID_NX  1//5
#define EYE_GRID_NY  5
#define MOUTH_GRID_X 80
#define MOUTH_GRID_Y 50
#define MOUTH_GRID_NX 3
#define MOUTH_GRID_NY 3
#ifdef  MOUTH_CROSS
#define MOUTH_GRID_N  5
#else
#define MOUTH_GRID_N  9
#endif
#define MIN_EYE_WIDTH  100
#define MIN_EYE_HEIGHT 50
#define DEFAULT_EYE_WIDTH 50
#define DEFAULT_EYE_HEIGHT 60
#define DEFAULT_MOUTH_WIDTH 100
#define DEFAULT_MOUTH_HEIGHT 50
#define X_INIT 200
#define Y_INIT 200
#define W_INIT 100//, 20, 0.004,0.0;
#define H_INIT 20
#define SLP_INIT 0.0
#define CRV_INIT 0//0.004

#define X_INIT_LEYE  125
#define X_INIT_REYE  275
#define Y_INIT_EYE   153//157

#define X_INIT_MOUTH 200
#define Y_INIT_MOUTH 328//338

#define W_INIT_MOUTH 122
#define H_INIT_MOUTH 33

#define POS_SHIFT 1
#define SZ_SHIFT  1
#define CRV_SHIFT 0.0002
#define SLP_SHIFT 0.01
#define MAX_CRV   0.01

#define MAX_EYE_OFFSET 5
#define MAX_MOUTH_TRPZ 20

#define FC_INNER_EYE  1
#define FC_OUTER_EYE  2
#define FC_MOUTH_EDGE 4

#define RANGE_INNER_EYE  0.07
#define RANGE_OUTER_EYE  0.07
#define RANGE_MOUTH_EDGE 0.07

#ifdef _DEBUG
#define FACE_DEB
#endif
#define FACE_DEB
FaceData lEGrid[EYE_GRID_NX*EYE_GRID_NY];
FaceData rEGrid[EYE_GRID_NX*EYE_GRID_NY];
FaceData MGrid[/*MOUTH_GRID_NX*MOUTH_GRID_NY*/MOUTH_GRID_N];

// CChildView
Vec3 Pos(0,0,-3);
Mat rot(1,0,0,0,1,0,0,0,1);
Mat rotx,roty;
Vec2 g_vMoveLEyel(-0.05,-0.15);
Vec2 g_vMoveREyer(0.05, -0.15);
Vec2 g_vMoveLEyer(0.03, -0.15);
Vec2 g_vMoveREyel(-0.03,-0.15);
Vec2 g_vMoveMouthl(-0.1,-0.3);
Vec2 g_vMoveMouthr(0.1, -0.3);

CChildView* pObj=NULL;
Rect2D ViewPort;
float rela=0,rela2=0,rela3=0;
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

void DrawEyeLid(IplImage* image, CvPoint2D32f center,float w, float h, float curvation, float slope)
{
	for(int i=floor(-w/2+center.x);i<=ceil(w/2+center.x);i++)
	{
		float cy=(i-center.x)*slope+center.y;
		for(int j=floor(-h/2+cy+min(0,curvation*w*w/4));j<=ceil(h/2+cy+max(0,curvation*w*w/4));j++)
		{
			CvPoint ptDraw=cvPoint(i, j);
			if(ptDraw.x>=0&&ptDraw.x<image->width&&ptDraw.y>=0&&ptDraw.y<image->height)
			{
				float x=i-center.x;
				float y=j-cy-curvation*x*x;
				float outpix=exp(-(x*x/w/w*16+y*y/h/h*16));
				uchar* pix=PTR_PIX(*image, ptDraw.x, ptDraw.y);
				//if(outpix>4./255)
				*pix=*pix*(1-outpix)+outpix*255;
			}
		}
	}
}

float FitEyeLid(IplImage* image, CvPoint2D32f center, float w, float h, float curvation, float slope)
{
	float acc=0, accp=0, acco=0;
	int accn=0;
	for(int i=floor(-w/2+center.x);i<=ceil(w/2+center.x);i++)
	{
		float cy=(i-center.x)*slope+center.y;
		for(int j=floor(-h/2+cy+min(0,curvation*w*w/4));j<=ceil(h/2+cy+max(0,curvation*w*w/4));j++)
		{
			CvPoint ptDraw=cvPoint(i, j);
			if(ptDraw.x>=0&&ptDraw.x<image->width&&ptDraw.y>=0&&ptDraw.y<image->height)
			{
				float x=i-center.x;
				float y=j-cy-curvation*x*x;
				float outpix=exp(-(x*x/w/w*16+y*y/h/h*16));
				if(outpix>4./255)
				{
					accp+=*(uchar*)PTR_PIX(*image, ptDraw.x, ptDraw.y)*outpix;
					acc+=pow((float)*(uchar*)PTR_PIX(*image, ptDraw.x, ptDraw.y),2.0f);
					acco+=pow(outpix, 2.0f);
					accn++;
				}
			}
		}
	}
	float result=accp*accp/acc/acco;//accn;
	rela=acc/accn;
	rela2=1./(1+exp(-0.1*(rela-100)));
	rela3=result*rela2;
	return rela3;
}

float HoleFunc(float x, float y, FaceRegion region)
{
	float cxl=(region.topleft.x+region.bottomleft.x)/2;
	float cxr=(region.topright.x+region.bottomright.x)/2;
	float cyl=(region.topleft.y+region.bottomleft.y)/2;
	float cyr=(region.topright.y+region.bottomright.y)/2;
	float hl=(region.bottomleft.y-region.topleft.y)/2;
	float hr=(region.bottomright.y-region.topright.y)/2;
	float cx=(cxl+cxr)/2;
	float cy=(cyl+cyr)/2;
	float w=(cxr-cxl)/2;
	float h=(hl+hr)/2;

	cx/=DIM_FACE_IMG;
	cy/=DIM_FACE_IMG;
	w/=DIM_FACE_IMG;
	h/=DIM_FACE_IMG;

	float slope=(cyl-cyr)/(cxl-cxr);
	float expt=-(pow((x-cx)/w*0.8,2)+pow(((y-cy)-slope*(x-cx))/h*0.8,2));
	return exp(expt);
}

Vec2 CChildView::ExpressionFunc(float x, float y, float ext, int corner)
{
	Vec2 shift(0,0);
	if(corner&FC_OUTER_EYE)
	{
		CvPoint2D32f lc = cvPoint2D32f((m_rgnlEadj.topleft.x+m_rgnlEadj.bottomleft.x)/2/DIM_FACE_IMG,
			(m_rgnlEadj.topleft.y+m_rgnlEadj.bottomleft.y)/2/DIM_FACE_IMG);
		shift-=m_vMoveLEyel*ext*exp(-(sqr((x-lc.x)/RANGE_OUTER_EYE)+sqr((y-lc.y)/RANGE_OUTER_EYE/2)));

		CvPoint2D32f rc = cvPoint2D32f((m_rgnrEadj.topright.x+m_rgnrEadj.bottomright.x)/2/DIM_FACE_IMG,
			(m_rgnrEadj.topright.y+m_rgnrEadj.bottomright.y)/2/DIM_FACE_IMG);
		shift-=m_vMoveREyer*ext*exp(-(sqr((x-rc.x)/RANGE_OUTER_EYE)+sqr((y-rc.y)/RANGE_OUTER_EYE/2)));
	}
	if(corner&FC_INNER_EYE)
	{
		CvPoint2D32f lc = cvPoint2D32f((m_rgnlEadj.topright.x+m_rgnlEadj.bottomright.x)/2/DIM_FACE_IMG,
			(m_rgnlEadj.topright.y+m_rgnlEadj.bottomright.y)/2/DIM_FACE_IMG);
		shift-=m_vMoveLEyer*ext*exp(-(sqr((x-lc.x)/RANGE_INNER_EYE)+sqr((y-lc.y)/RANGE_INNER_EYE/2)));

		CvPoint2D32f rc = cvPoint2D32f((m_rgnrEadj.topleft.x+m_rgnrEadj.bottomleft.x)/2/DIM_FACE_IMG,
			(m_rgnrEadj.topleft.y+m_rgnrEadj.bottomleft.y)/2/DIM_FACE_IMG);
		shift-=m_vMoveREyel*ext*exp(-(sqr((x-rc.x)/RANGE_INNER_EYE)+sqr((y-rc.y)/RANGE_INNER_EYE/2)));
	}
	if(corner&FC_MOUTH_EDGE)
	{
		CvPoint2D32f lc = cvPoint2D32f((m_rgnMadj.topleft.x+m_rgnMadj.bottomleft.x)/2/DIM_FACE_IMG-RANGE_MOUTH_EDGE/2,
									   (m_rgnMadj.topleft.y+m_rgnMadj.bottomleft.y)/2/DIM_FACE_IMG);
		shift-=m_vMoveMouthl*ext*exp(-(sqr((x-lc.x)/RANGE_MOUTH_EDGE)+sqr((y-lc.y)/RANGE_MOUTH_EDGE/2)));
		CvPoint2D32f rc = cvPoint2D32f((m_rgnMadj.topright.x+m_rgnMadj.bottomright.x)/2/DIM_FACE_IMG+RANGE_MOUTH_EDGE/2,
									   (m_rgnMadj.topright.y+m_rgnMadj.bottomright.y)/2/DIM_FACE_IMG);
		shift-=m_vMoveMouthr*ext*exp(-(sqr((x-rc.x)/RANGE_MOUTH_EDGE)+sqr((y-rc.y)/RANGE_MOUTH_EDGE/2)));
	}
	return shift;
}

void PixelShader(Point2D pos, Vec3* color, Vec2* tex, float* userdata, Vec3* colorout, float* depth)
{
	Vec2 stex=*tex-Vec2(0.5,0.5);
	uchar* pix=PTR_PIX(*(pObj->m_ImgBk), pos.x, pos.y);
	Vec3 cbk(*pix,*(pix+1),*(pix+2));
	cbk/=255;
	float radius=sqrt(stex.x*stex.x+stex.y*stex.y);
	float alpha=FrameFunc(0.5+radius, 0.2);
	if(pObj->m_bDet)
	{
		float alphahole=0.5;
		alphahole+=HoleFunc(tex->x,tex->y,pObj->m_rgnlEadj);
		alphahole+=HoleFunc(tex->x,tex->y,pObj->m_rgnrEadj);
		alphahole+=HoleFunc(tex->x,tex->y,pObj->m_rgnMadj);
		if(alphahole>1)alphahole=1;
		alpha*=alphahole;
	}
	Vec3 smp=Sample(pObj->m_texture, *tex)/255**color;
	
	*colorout=smp*alpha+cbk*(1-alpha);
}

CChildView::CChildView():m_oldPt(0,0)
{
	m_bShowVert = false;
	m_bShowImage = true;
	m_bDown = false;
	m_bMove = false;
	m_bFit = false;
	m_bAuto = false;
	m_bShowGrid = false;
	m_bShowDetBox = false;
	m_bShowAdj = false;
	m_bShowClrImg = true;
	m_bDet = false;
	m_bAnimation = false;
	m_nPostureEnable[0] = 1;
	m_nPostureEnable[1] = 0;
	m_nPostureEnable[2] = 1;

	m_nPostureSel = 0;

	m_Image = NULL;
	m_imgObj = NULL;
	m_ImgBk = NULL;
	m_face = NULL;
	m_fgray = NULL;
	m_deBuf = NULL;
	m_Det = NULL;

	m_EyeX = X_INIT;
	m_EyeY = Y_INIT;
	m_EyeW = W_INIT;//, 20, 0.004,0.0;
	m_EyeH = H_INIT;
	m_EyeSlp = SLP_INIT;
	m_EyeCrv = CRV_INIT;
	oldbmp = NULL;

	m_vMoveLEyel = g_vMoveLEyel; 
	m_vMoveREyer = g_vMoveREyer; 
	m_vMoveLEyer = g_vMoveLEyer; 
	m_vMoveREyel = g_vMoveREyel; 
	m_vMoveMouthl =g_vMoveMouthl;
	m_vMoveMouthr =g_vMoveMouthr;

	m_fMorph = 0;
}

CChildView::~CChildView()
{
	cvReleaseImage(&m_ImgBk);
	cvReleaseImage(&m_Image);
	cvReleaseImage(&m_imgObj);
	cvReleaseImage(&m_face);
	cvReleaseImage(&m_deBuf);
	cvReleaseImage(&m_fgray);
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_TIMER()
	ON_COMMAND(ID_FILE_OPEN, &CChildView::OnFileOpen)
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

#define FACERECT_SCALE  1

void CChildView::Draw()
{
	CClientDC dc(this);

	CRect rcClt;
	GetClientRect(rcClt);
	if(!m_bAnimation)
		Process();

#ifdef FACE_DEB
	IplImage* output=cvCreateImage(cvGetSize(m_face),IPL_DEPTH_8U, 3);
	if(!m_bShowClrImg)
	{
		IplImage* tmp=cvCloneImage(m_fgray);
		if(!m_bShowGrid)
			DrawEyeLid(tmp, cvPoint2D32f(m_EyeX,m_EyeY), m_EyeW, m_EyeH, m_EyeCrv, m_EyeSlp);
		else
		{
			for(int i=0;i<EYE_GRID_NX*EYE_GRID_NY;i++)
			{
				DrawEyeLid(tmp, cvPoint2D32f(lEGrid[i].x, lEGrid[i].y), lEGrid[i].w, lEGrid[i].h, lEGrid[i].c, lEGrid[i].s);
				DrawEyeLid(tmp, cvPoint2D32f(rEGrid[i].x, rEGrid[i].y), rEGrid[i].w, rEGrid[i].h, rEGrid[i].c, rEGrid[i].s);
			}
			for(int i=0;i</*MOUTH_GRID_NX*MOUTH_GRID_NY*/MOUTH_GRID_N;i++)
			{
				DrawEyeLid(tmp, cvPoint2D32f(MGrid[i].x, MGrid[i].y), MGrid[i].w, MGrid[i].h, MGrid[i].c, MGrid[i].s);
			}
		}
		cvMerge(tmp, tmp, tmp, NULL, output);
		cvReleaseImage(&tmp);
	}
	else
	{
		if(m_bDet&&m_bAnimation)
		{
			cvZero(output);
			int posture=0;
			if(m_nPostureEnable[0])posture|=FC_OUTER_EYE;
			if(m_nPostureEnable[1])posture|=FC_INNER_EYE;
			if(m_nPostureEnable[2])posture|=FC_MOUTH_EDGE;
			for(int i=0;i<output->height;i++)
			{
				for(int j=0;j<output->width;j++)
				{
					float x=(float)j/output->width;
					float y=(float)i/output->height;

					Vec2 shift=ExpressionFunc(x, y, m_fMorph, posture);
					Vec3 smp=Sample(m_face, Vec2(x,y)+shift);
					uchar* pix=PTR_PIX(*output, j, i);
					*pix = smp.x;
					*(pix+1) = smp.y;
					*(pix+2) = smp.z;
				}
			}
		}
		else
			cvCopyImage(m_face, output);
	}

	float corel=FitEyeLid(m_fgray, cvPoint2D32f(m_EyeX,m_EyeY), m_EyeW, m_EyeH, m_EyeCrv, m_EyeSlp);

	DispImage(&m_mdc, output, CPoint(0,0));
	cvReleaseImage(&output);
	if(m_bShowDetBox)
	{
		DrawDetectRegion(&m_mdc);
	}

	if(!m_bAnimation)
	{
		CString str,str1,str2,str3,str4,str5,str6;
		str.Format("%10.4f",corel);
		str1.Format("%10.4f",rela);
		str2.Format("%10.4f", rela2);
		str3.Format("fitval=%6.4f", rela3);
		str4.Format("X=%6.2f,Y=%6.2f", m_EyeX,m_EyeY);
		str5.Format("W=%6.2f,H=%6.2f", m_EyeW,m_EyeH);
		str6.Format("Crv=%6.4f,Slp=%6.4f", m_EyeCrv, m_EyeSlp);

		m_mdc.SetBkMode(TRANSPARENT);
		m_mdc.SetTextColor(RGB(255,255,255));
/*		int tw=m_mdc.GetTextExtent(str).cx;
		m_mdc.TextOut(70-tw,0,str);
		tw=m_mdc.GetTextExtent(str1).cx;
		m_mdc.TextOut(70-tw,20,str1);
		tw=m_mdc.GetTextExtent(str2).cx;
		m_mdc.TextOut(70-tw,40,str2);
		tw=m_mdc.GetTextExtent(str3).cx;
*/		m_mdc.TextOut(0, 0,str3);
		m_mdc.TextOut(0, 20, str4);
		m_mdc.TextOut(0, 40, str5);
		m_mdc.TextOut(0, 60, str6);
		if(m_bAuto)
		{
			m_mdc.TextOut(0,80,"Auto Detect On");
		}
		else
		{
			m_mdc.TextOut(0,80,"Auto Detect Off");
		}
		if(m_bFit)
		{
			m_mdc.TextOut(0,100,"Fit success");
		}
	}
	else
	{
		CString str,str2;
		str.Format("posture selected: %d", m_nPostureSel+1);
		if(m_nPostureEnable[m_nPostureSel]==0)
			str2.Format("posture state: disabled");
		else if(m_nPostureEnable[m_nPostureSel]==1)
			str2.Format("posture state: positive");
		else if(m_nPostureEnable[m_nPostureSel]==-1)
			str2.Format("posture state: negative");

		m_mdc.SetBkMode(TRANSPARENT);
		m_mdc.SetTextColor(RGB(255,255,255));

		m_mdc.TextOut(0, 0, str);
		m_mdc.TextOut(0, 20, str2);
	}
#else
	pObj=this;
	m_mesh.SetViewPort(&ViewPort);
	m_mesh.SetPixelShader(PixelShader);
	m_mesh.SetMatrix(rot, -Pos);
	m_mesh.Render(m_Image, &m_zBuf);
	pObj=NULL;
	DispImage(&m_mdc, m_Image, CPoint(0,0));
#endif

	m_mdc.SelectStockObject(NULL_BRUSH);
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

void CChildView::Process()
{
	cvCopyImage(m_ImgBk, m_Image);
	cvZero(m_deBuf);
	//cvResize(m_imgObj,m_Image);
	int cx=m_rcFace.x+m_rcFace.width/2;
	int cy=m_rcFace.y+m_rcFace.height/2;
	m_rcFace.x=cx-m_rcFace.width/2*FACERECT_SCALE;
	m_rcFace.y=cy-m_rcFace.height/2*FACERECT_SCALE;
	m_rcFace.width*=FACERECT_SCALE;
	m_rcFace.height*=FACERECT_SCALE;
	//float hs=DIM_FACE_IMG./rcFace.width;
	//float vs=300./rcFace.height;

	if(m_rcFace.width>0&&m_rcFace.height>0)
	{
		cvSetImageROI(m_imgObj, m_rcFace);
		cvResize(m_imgObj, m_face);
		cvResetImageROI(m_imgObj);
	}

	IplImage* integ=cvCreateImage(cvSize(m_fgray->width+1,m_fgray->height+1), IPL_DEPTH_32S, 1);
	IplImage* integ2=cvCreateImage(cvSize(m_fgray->width+1,m_fgray->height+1), IPL_DEPTH_64F, 1);

	cvCvtColor(m_face, m_fgray, CV_RGB2GRAY);
	cvIntegral(m_fgray, integ, integ2);
	for(int i=0;i<m_fgray->height;i++)
	{
		for(int j=0;j<m_fgray->width;j++)
		{
			int ks=10;
			int pi=i-ks;
			int ni=i+ks;
			int pj=j-ks;
			int nj=j+ks;
			if(pi<0)pi=0;
			if(ni>=m_fgray->width)ni=m_fgray->width-1;
			if(pj<0)pj=0;
			if(nj>=m_fgray->height)nj=m_fgray->height-1;
			int a=*(int*)PTR_PIX(*integ, pj, pi);
			int b=*(int*)PTR_PIX(*integ, nj, pi);
			int c=*(int*)PTR_PIX(*integ, pj, ni);
			int d=*(int*)PTR_PIX(*integ, nj, ni);
			double a2=*(double*)PTR_PIX(*integ2, pj, pi);
			double b2=*(double*)PTR_PIX(*integ2, nj, pi);
			double c2=*(double*)PTR_PIX(*integ2, pj, ni);
			double d2=*(double*)PTR_PIX(*integ2, nj, ni);
			int output=0;
			if((ni-pi)*(nj-pj)!=0)
			{
				output = sqrt((d2-b2-c2+a2)/((ni-pi)*(nj-pj))-((float)(d-b-c+a)/((ni-pi)*(nj-pj)))*((float)(d-b-c+a)/((ni-pi)*(nj-pj))));
			}
			*(uchar*)PTR_PIX(*m_fgray, j, i)=output;
		}
	}
	//cvCanny(gray, gray, 10, 30, );
	//cvCmpS(gray, 10, gray, CV_CMP_GT);
	cvReleaseImage(&integ);
	cvReleaseImage(&integ2);
	//cvScale(gray, gray, 10);
	//cvZero(gray);
/*	for(int i=0;i<gray->width;i++)
	{
		for(int j=0;j<gray->height;j++)
		{
			FitEyeLid(gray,cvPoint(i,j), m_EyeW, m_EyeH, m_EyeCrv, m_EyeSlp);
			uchar* pix=PTR_PIX(*m_Det, i, j);
			*pix=255*rela3;
			*(pix+1)=255*rela3;
			*(pix+2)=255*rela3;
		}
	}*/
	//cvMerge(m_fgray, m_fgray, m_fgray, NULL, m_face);
	//EdgeProcess(m_face);

}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	IplImage* image = cvLoadImage("1.jpg");
	m_ImgBk = cvLoadImage("Koala.jpg");
	float hs=800./image->width;
	float vs=600./image->height;
	float wobj=min(hs,vs)*image->width;
	float hobj=min(hs,vs)*image->height;
	m_imgObj = cvCreateImage(cvSize(wobj, hobj),IPL_DEPTH_8U,3);
	cvResize(image, m_imgObj);
	cvReleaseImage(&image);

	int wImage = m_ImgBk->width;
	int hImage = m_ImgBk->height;
	ViewPort = Rect2D(0,0, wImage, hImage);
	LoadFile();
	m_Image = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,3);
	m_zBuf = m_deBuf = cvCreateImage(cvSize(wImage, hImage), IPL_DEPTH_32F, 1);
	m_texture = m_face = cvCreateImage(cvSize(DIM_FACE_IMG,DIM_FACE_IMG),IPL_DEPTH_8U,3);
	m_fgray = cvCreateImage(cvGetSize(m_face), IPL_DEPTH_8U, 1);

	CreateDome(&m_mesh, 1,1,0.2,20);
	//m_mesh.SetTexture(&m_texture);

	CClientDC dc(this);
	m_mdc.CreateCompatibleDC(&dc);
#ifdef FACE_DEB
	m_bitmap.CreateCompatibleBitmap(&dc, DIM_FACE_IMG,DIM_FACE_IMG/*wImage, hImage*/);
#else
	m_bitmap.CreateCompatibleBitmap(&dc, wImage, hImage);
#endif
	oldbmp = m_mdc.SelectObject(&m_bitmap);

	InitFaceDetect();
	m_rcFace = Detect(m_imgObj);

	CRect rcWnd;
	AfxGetMainWnd()->GetWindowRect(rcWnd);
	rcWnd.right=rcWnd.left+wImage;
	rcWnd.bottom=rcWnd.top+hImage;
	AfxGetMainWnd()->MoveWindow(rcWnd);
	RestoreEyeData(Grid);
#ifdef FACE_DEB
	Process();
#if 0
	m_WndDet = new CDetectWnd;
	m_WndDet->m_Image = m_Det;
	m_WndDet->Create(NULL, "Detect");
	m_WndDet->ShowWindow(SW_SHOW);
	m_WndDet->UpdateWindow();
	m_WndDet->Invalidate();
#endif
#endif
	return 0;
}

void CChildView::OnDestroy()
{
	CWnd::OnDestroy();
#ifdef FACE_DEB
#if 0
	if(!m_WndDet->m_bDestroyed)
		m_WndDet->DestroyWindow();
#endif
#endif
	// TODO: Add your message handler code here
	if(oldbmp!=NULL)
		m_mdc.SelectObject(oldbmp);
	m_mdc.DeleteDC();
	m_bitmap.DeleteObject();
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SetCapture();
	m_bDown = true;
	m_oldPt = point;
#ifdef FACE_DEB
	m_EyeX=point.x;
	m_EyeY=point.y;
	StartDetect();
#endif
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
#ifdef FACE_DEB
	if(m_bDown)
	{
		m_EyeX=point.x;
		m_EyeY=point.y;
		StartDetect();
		Draw();
	}
#else
	if(m_bDown)
	{
		CPoint offset=point-m_oldPt;
		if(!m_bMove)
		{
			float HAngle = (float)offset.x/100;
			float VAngle = (float)offset.y/100;
			rotx = Mat(cos(HAngle),0,sin(HAngle),0,1,0,-sin(HAngle),0,cos(HAngle));
			roty = Mat(1,0,0,0,cos(VAngle),-sin(VAngle),0,sin(VAngle),cos(VAngle));
			rot = rot*rotx*roty;
		}
		else
		{
			ViewPort.x+=offset.x;
			ViewPort.y+=offset.y;
		}
		m_oldPt = point;
		Draw();
	}
#endif
	CWnd::OnMouseMove(nFlags, point);
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
#ifdef FACE_DEB
	Vec2* posv[6] = {&m_vMoveLEyel, &m_vMoveREyer, &m_vMoveLEyer, &m_vMoveREyel, &m_vMoveMouthl, &m_vMoveMouthr};
	Vec2* gposv[6] = {&g_vMoveLEyel, &g_vMoveREyer, &g_vMoveLEyer, &g_vMoveREyel, &g_vMoveMouthl, &g_vMoveMouthr};
#endif
	switch(nChar)
	{
	case VK_SPACE:
		m_bMove = true;
		Draw();
		break;
	case 'K':
#ifdef FACE_DEB
		if(m_bAuto)
		{
			EndDetect();
		}
		else
		{
			m_bAuto = true;
			StartDetect();
		}
#else
		m_bShowImage = !m_bShowImage;
#endif
		Draw();
		break;
#ifdef FACE_DEB
	case 'A':
		if(!m_bAnimation&&m_bDet)
		{
			m_bAnimation = true;
			m_bShowDetBox = false;
			SetTimer(1,10,NULL);
		}
		else
		{
			KillTimer(1);
			m_bAnimation = false;
			m_fMorph = 0;
		}
		break;
#else
	case 'A':
		Pos.x-=0.05;
		Draw();
		break;
	case 'D':
		Pos.x+=0.05;
		Draw();
		break;
	case 'W':
		Pos.z+=0.05;
		Draw();
		break;
	case 'S':
		Pos.z-=0.05;
		Draw();
		break;
	case 'Q':
		Pos.y-=0.05;
		Draw();
		break;
	case 'E':
		Pos.y+=0.05;
		Draw();
		break;
#endif
	case 'Z':
#ifdef FACE_DEB
		RestoreEyeData(LeftEye);
		StartDetect();
		Draw();
#else
		SaveFile();
#endif
		break;
	case 'I':
		m_bDet = false;
		m_bAnimation = false;
		KillTimer(1);
		m_fMorph = 0;
		m_rcFace=Detect(m_imgObj);
		Draw();
		break;
#ifdef FACE_DEB
	case 'F':
		if(m_bShowGrid)
		{
			m_bFit = false;
			for(int i=0;i<EYE_GRID_NX*EYE_GRID_NY;i++)
			{
				m_bFit|=Fit(&lEGrid[i]);
				m_bFit|=Fit(&rEGrid[i]);
			}
			for(int i=0;i</*MOUTH_GRID_NX*MOUTH_GRID_NY*/MOUTH_GRID_N;i++)
			{
				m_bFit|=Fit(&MGrid[i]);
			}
		}
		else
		{
			m_bFit = Fit();
		}
		Draw();
		break;
	case 'O':
		RestoreEyeData(Default);
		StartDetect();
		Draw();
		break;
	case 'X':
		RestoreEyeData(RightEye);
		StartDetect();
		Draw();
		break;
	case 'C':
		RestoreEyeData(Mouth);
		StartDetect();
		Draw();
		break;
	case 'V':
		m_bShowGrid=!m_bShowGrid;
		Draw();
		break;
	case 'B':
		RestoreEyeData(Grid);
		StartDetect();
		Draw();
		break;
	case 'N':
		m_bShowDetBox = !m_bShowDetBox;
		Draw();
		break;
	case 'G':
		m_bShowClrImg = !m_bShowClrImg;
		Draw();
		break;
	case VK_LEFT:
		if(m_bAnimation)
		{
			m_nPostureSel--;
			if(m_nPostureSel < 0)
				m_nPostureSel = 0;
		}
		else
		{
			m_bShowAdj = !m_bShowAdj;
		}
		Draw();
		break;
	case VK_RIGHT:
		if(m_bAnimation)
		{
			m_nPostureSel++;
			if(m_nPostureSel >= 3)
				m_nPostureSel = 2;
		}
		else
		{
			m_bShowAdj = !m_bShowAdj;
		}
		Draw();
		break;
	case VK_UP:
		if(m_bAnimation)
		{
			m_nPostureEnable[m_nPostureSel]++;
			if(m_nPostureEnable[m_nPostureSel]>1)
				m_nPostureEnable[m_nPostureSel]=1;
			for(int i=0;i<6;i++)
			{
				*posv[i] = *gposv[i]*m_nPostureEnable[i/2];
			}
			Draw();
		}
		break;
	case VK_DOWN:
		if(m_bAnimation)
		{
			m_nPostureEnable[m_nPostureSel]--;
			if(m_nPostureEnable[m_nPostureSel]<-1)
				m_nPostureEnable[m_nPostureSel]=-1;
			for(int i=0;i<6;i++)
			{
				*posv[i] = *gposv[i]*m_nPostureEnable[i/2];
			}
			Draw();
		}
		break;
#endif
	case 'P':
		OneTimeDetect();
		m_bShowDetBox = true;
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
		m_bMove = false;
		break;
	}
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CChildView::SaveFile()
{
	CFile file;
	file.Open("config",CFile::modeCreate|CFile::modeWrite);
	file.Write(&rot, sizeof(rot));
	file.Write(&Pos, sizeof(Pos));
	CPoint pt(ViewPort.x,ViewPort.y);
	file.Write(&pt, sizeof(pt));
	file.Close();
}

void CChildView::LoadFile()
{
	CFile file;
	if(file.Open("config",CFile::modeRead))
	{
		file.Read(&rot, sizeof(rot));
		file.Read(&Pos, sizeof(Pos));
		CPoint pt;
		file.Read(&pt, sizeof(pt));
		ViewPort.x=pt.x;
		ViewPort.y=pt.y;
		file.Close();
	}
}

void CChildView::RestoreEyeData(InitType type)
{
	switch(type)
	{
	case Default:
		m_EyeW = W_INIT;//, 20, 0.004,0.0;
		m_EyeH = H_INIT;
		m_EyeSlp = SLP_INIT;
		m_EyeCrv = CRV_INIT;
		break;
	case LeftEye:
		m_EyeX=X_INIT_LEYE;
		m_EyeY=Y_INIT_EYE;
		m_EyeW=W_INIT;
		m_EyeH=H_INIT;
		m_EyeCrv=CRV_INIT;
		m_EyeSlp=SLP_INIT;
		break;
	case RightEye:
		m_EyeX=X_INIT_REYE;
		m_EyeY=Y_INIT_EYE;
		m_EyeW=W_INIT;
		m_EyeH=H_INIT;
		m_EyeCrv=CRV_INIT;
		m_EyeSlp=SLP_INIT;
		break;
	case Mouth:
		m_EyeX=X_INIT_MOUTH;
		m_EyeY=Y_INIT_MOUTH;
		m_EyeW=W_INIT_MOUTH;
		m_EyeH=H_INIT_MOUTH;
		m_EyeCrv=CRV_INIT;
		m_EyeSlp=SLP_INIT;
		break;
	case Grid:
		for(int i=0;i<EYE_GRID_NX;i++)
		{
			for(int j=0;j<EYE_GRID_NY;j++)
			{
				lEGrid[i*EYE_GRID_NY+j].x=X_INIT_LEYE-EYE_GRID_X/2+EYE_GRID_X*i/max(1,(EYE_GRID_NX-1));
				lEGrid[i*EYE_GRID_NY+j].y=Y_INIT_EYE-EYE_GRID_Y/2+EYE_GRID_Y*j/max(1,(EYE_GRID_NY-1));
				rEGrid[i*EYE_GRID_NY+j].x=X_INIT_REYE-EYE_GRID_X/2+EYE_GRID_X*i/max(1,(EYE_GRID_NX-1));
				rEGrid[i*EYE_GRID_NY+j].y=Y_INIT_EYE-EYE_GRID_Y/2+EYE_GRID_Y*j/max(1,(EYE_GRID_NY-1));

				lEGrid[i*EYE_GRID_NY+j].w=W_INIT;
				lEGrid[i*EYE_GRID_NY+j].h=H_INIT;
				rEGrid[i*EYE_GRID_NY+j].w=W_INIT;
				rEGrid[i*EYE_GRID_NY+j].h=H_INIT;

				lEGrid[i*EYE_GRID_NY+j].c=CRV_INIT;
				lEGrid[i*EYE_GRID_NY+j].s=SLP_INIT;
				rEGrid[i*EYE_GRID_NY+j].c=CRV_INIT;
				rEGrid[i*EYE_GRID_NY+j].s=SLP_INIT;
			}
		}
		int im=0;
		for(int i=0;i<MOUTH_GRID_NX;i++)
		{
			for(int j=0;j<MOUTH_GRID_NY;j++)
			{
#ifdef MOUTH_CROSS
				if((i==0||i==2)&&(j==0||j==2))
					continue;
#endif
				MGrid[/*i*MOUTH_GRID_NY+j*/im].x=X_INIT_MOUTH-MOUTH_GRID_X/2+MOUTH_GRID_X*i/max(1,(MOUTH_GRID_NX-1));
				MGrid[/*i*MOUTH_GRID_NY+j*/im].y=Y_INIT_MOUTH-MOUTH_GRID_Y/2+MOUTH_GRID_Y*j/max(1,(MOUTH_GRID_NY-1));

				MGrid[/*i*MOUTH_GRID_NY+j*/im].w=W_INIT_MOUTH;
				MGrid[/*i*MOUTH_GRID_NY+j*/im].h=H_INIT_MOUTH;

				MGrid[/*i*MOUTH_GRID_NY+j*/im].c=CRV_INIT;
				MGrid[/*i*MOUTH_GRID_NY+j*/im].s=SLP_INIT;
				im++;
			}
		}
		m_bDet = false;
		break;
	}
}

bool CChildView::Fit()
{
	float cval=FitEyeLid(m_fgray, cvPoint2D32f(m_EyeX,m_EyeY),m_EyeW,m_EyeH,m_EyeCrv,m_EyeSlp);
	float fitval[12];
	int sign[2]={-1,1};
	float X,Y,W,H,Crv,Slp;
	float* vparam[6]={&m_EyeX,&m_EyeY,&m_EyeW,&m_EyeH,&m_EyeCrv,&m_EyeSlp};
	float* vp[6]={&X,&Y,&W,&H,&Crv,&Slp};
	float paramlist[6]={POS_SHIFT, POS_SHIFT, SZ_SHIFT, SZ_SHIFT, CRV_SHIFT, SLP_SHIFT};
	int poslist[6]={0,1,2,3,4,5};
	for(int i=0;i<12;i++)
	{
		int s=sign[i%2];
		int nparam=i/2;
		for(int j=0;j<6;j++)
		{
			*vp[j]=*vparam[j];
		}
		*vp[poslist[nparam]]+=s*paramlist[nparam];
		if(Crv>MAX_CRV)Crv=MAX_CRV;
		else if(Crv<-MAX_CRV)Crv=-MAX_CRV;
		fitval[i]=FitEyeLid(m_fgray, cvPoint2D32f(X,Y),W,H,Crv,Slp);
	}
	float vmax=0;
	int index=-1;
	for(int i=0;i<12;i++)
	{
		if(vmax<fitval[i])
		{
			vmax = fitval[i];
			index = i;
		}
	}

	if(vmax<=cval)
	{
		return false;
	}

	if(index!=-1)
	{
		int s=sign[index%2];
		int pos=index/2;
		*vparam[poslist[pos]]+=s*paramlist[pos];
		if(m_EyeCrv>MAX_CRV)m_EyeCrv=MAX_CRV;
		else if(m_EyeCrv<-MAX_CRV)m_EyeCrv=-MAX_CRV;
		return true;
	}

	return false;
}

bool CChildView::Fit(FaceData* fitdata)
{
	float cval=FitEyeLid(m_fgray, cvPoint2D32f(fitdata->x,fitdata->y),fitdata->w,fitdata->h,fitdata->c,fitdata->s);
	float fitval[12];
	int sign[2]={-1,1};
	float X,Y,W,H,Crv,Slp;
	float* vparam[6]={&(fitdata->x),&(fitdata->y),&(fitdata->w),&(fitdata->h),&(fitdata->c),&(fitdata->s)};
	float* vp[6]={&X,&Y,&W,&H,&Crv,&Slp};
	float paramlist[6]={POS_SHIFT, POS_SHIFT, SZ_SHIFT, SZ_SHIFT, CRV_SHIFT, SLP_SHIFT};
	int poslist[6]={0,1,2,3,4,5};
	for(int i=0;i<12;i++)
	{
		int s=sign[i%2];
		int nparam=i/2;
		for(int j=0;j<6;j++)
		{
			*vp[j]=*vparam[j];
		}
		*vp[poslist[nparam]]+=s*paramlist[nparam];
		if(Crv>MAX_CRV)Crv=MAX_CRV;
		else if(Crv<-MAX_CRV)Crv=-MAX_CRV;
		fitval[i]=FitEyeLid(m_fgray, cvPoint2D32f(X,Y),W,H,Crv,Slp);
	}
	float vmax=0;
	int index=-1;
	for(int i=0;i<12;i++)
	{
		if(vmax<fitval[i])
		{
			vmax = fitval[i];
			index = i;
		}
	}

	if(vmax<=cval)
	{
		return false;
	}

	if(index!=-1)
	{
		int s=sign[index%2];
		int pos=index/2;
		*vparam[poslist[pos]]+=s*paramlist[pos];
		if(fitdata->c>MAX_CRV)fitdata->c=MAX_CRV;
		else if(fitdata->c<-MAX_CRV)fitdata->c=-MAX_CRV;
		return true;
	}

	return false;

}

void CChildView::StartDetect()
{
	if(m_bAuto)
		SetTimer(0,1,NULL);
}

void CChildView::EndDetect()
{
	m_bAuto = false;
	KillTimer(0);
}

void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == 0)
	{
		if(m_bShowGrid)
		{
			m_bFit = false;
			for(int i=0;i<EYE_GRID_NX*EYE_GRID_NY;i++)
			{
				m_bFit|=Fit(&lEGrid[i]);
				m_bFit|=Fit(&rEGrid[i]);
			}
			for(int i=0;i</*MOUTH_GRID_NX*MOUTH_GRID_NY*/MOUTH_GRID_N;i++)
			{
				m_bFit|=Fit(&MGrid[i]);
			}
		}
		else
		{
			m_bFit = Fit();
		}
		Draw();
		if(!m_bFit)
		{
			KillTimer(0);
			AdjustDetRegion();
			m_bDet = true;
		}
	}
	else if(nIDEvent == 1)
	{
		if(m_bDet&&m_bAnimation)
		{
			static float speed=0.03;
			m_fMorph+=speed;
			if(speed>0&&m_fMorph>0.3)
			{
				speed=-0.03;
			}
			if(speed<0&&m_fMorph<0)
			{
				speed=0.03;
			}
			Draw();
		}
	}
	CWnd::OnTimer(nIDEvent);
}

#define EXTREME_EXCLUDE  1
void Insert(float val, float* list, int& cnt,int order)
{
	if(cnt==0)
	{
		list[0]=val;
		cnt++;
	}
	else
	{
		if(order==0)
		{
			if(cnt<EXTREME_EXCLUDE)
			{
				cnt++;
				list[cnt-1]=val;
			}
			else if(val<list[cnt-1])
			{
				list[cnt-1]=val;
			}
			else
				return;
			for(int i=cnt-2;i>=0;i--) 
			{
				if(val<list[i])
				{
					list[i+1]=list[i];
					list[i]=val;
				}
			}
		}
		else
		{
			if(cnt<EXTREME_EXCLUDE)
			{
				cnt++;
				list[cnt-1]=val;
			}
			else if(val>list[cnt-1])
			{
				list[cnt-1]=val;
			}
			else
				return;
			for(int i=cnt-2;i>=0;i--)
			{
				if(val>list[i])
				{
					list[i+1]=list[i];
					list[i]=val;
				}
			}
		}
	}
}

FaceRegion CChildView::GetDetectedRegion(DetectType type)
{
	float xmin=DIM_FACE_IMG,xmax=0;
	float yminl=DIM_FACE_IMG,ymaxl=0;
	float yminr=DIM_FACE_IMG,ymaxr=0;
	float xmintop[EXTREME_EXCLUDE];
	float xmaxtop[EXTREME_EXCLUDE];
	float yminltop[EXTREME_EXCLUDE];
	float yminrtop[EXTREME_EXCLUDE];
	float ymaxltop[EXTREME_EXCLUDE];
	float ymaxrtop[EXTREME_EXCLUDE];

	int nxmin=0;
	int nxmax=0;
	int nyminl=0;
	int nyminr=0;
	int nymaxl=0;
	int nymaxr=0;
	float cx,cy;
	float w,h;
	FaceData* grid=NULL;
	int dim=0;
	switch(type)
	{
	case Det_LeftEye:
		grid = (FaceData*)lEGrid;
		dim = EYE_GRID_NX*EYE_GRID_NY;
		cx = X_INIT_LEYE;
		cy = Y_INIT_EYE;
		w = DEFAULT_EYE_WIDTH;
		h = DEFAULT_EYE_HEIGHT;

		break;
	case Det_RightEye:
		grid = (FaceData*)rEGrid;
		dim = EYE_GRID_NX*EYE_GRID_NY;
		cx = X_INIT_REYE;
		cy = Y_INIT_EYE;
		w = DEFAULT_EYE_WIDTH;
		h = DEFAULT_EYE_HEIGHT;
		break;
	case Det_Mouth:
		grid = (FaceData*)MGrid;
		dim = MOUTH_GRID_N;
		cx = X_INIT_MOUTH;
		cy = Y_INIT_MOUTH;
		w = DEFAULT_MOUTH_WIDTH;
		h = DEFAULT_MOUTH_HEIGHT;
		break;
	}
	if(grid == NULL)
		return FaceRegion();
	bool* blist=new bool[dim];
	Classify(grid, blist, dim);
	for(int i=0;i<dim;i++)
	{
		FaceData* data=grid+i;
		if(data->x>cx-w/2&&data->x<cx+w/2&&data->y>cy-h/2&&data->y<cy+h/2)
		{
			blist[i]=true;
		}
	}
	for(int i=0;i<dim;i++)
	{
		if(!blist[i])continue;
		FaceData facedata=grid[i];
		if(facedata.x-facedata.w/2<xmin)
			xmin=facedata.x-facedata.w/2;
		if(facedata.x+facedata.w/2>xmax)
			xmax=facedata.x+facedata.w/2;
		if(facedata.y-facedata.h/2-facedata.s*facedata.w/2<yminl)
			yminl=facedata.y-facedata.h/2-facedata.s*facedata.w/2;
		if(facedata.y+facedata.h/2-facedata.s*facedata.w/2>ymaxl)
			ymaxl=facedata.y+facedata.h/2-facedata.s*facedata.w/2;
		if(facedata.y-facedata.h/2+facedata.s*facedata.w/2<yminr)
			yminr=facedata.y-facedata.h/2+facedata.s*facedata.w/2;
		if(facedata.y+facedata.h/2+facedata.s*facedata.w/2>ymaxr)
			ymaxr=facedata.y+facedata.h/2+facedata.s*facedata.w/2;

		Insert(facedata.x-facedata.w/2, xmintop, nxmin,0);
		Insert(facedata.x+facedata.w/2, xmaxtop, nxmax,1);
		Insert(facedata.y-facedata.h/2-facedata.s*facedata.w/2, yminltop, nyminl,0);
		Insert(facedata.y+facedata.h/2-facedata.s*facedata.w/2, ymaxltop, nymaxl,1);
		Insert(facedata.y-facedata.h/2+facedata.s*facedata.w/2, yminrtop, nyminr,0);
		Insert(facedata.y+facedata.h/2+facedata.s*facedata.w/2, ymaxrtop, nymaxr,1);
	}

	delete[] blist;
	FaceRegion detectregion;
	detectregion.topleft = cvPoint2D32f(xmintop[EXTREME_EXCLUDE-1],yminltop[EXTREME_EXCLUDE-1]);
	detectregion.topright = cvPoint2D32f(xmaxtop[EXTREME_EXCLUDE-1],yminrtop[EXTREME_EXCLUDE-1]);
	detectregion.bottomright = cvPoint2D32f(xmaxtop[EXTREME_EXCLUDE-1],ymaxrtop[EXTREME_EXCLUDE-1]);
	detectregion.bottomleft = cvPoint2D32f(xmintop[EXTREME_EXCLUDE-1],ymaxltop[EXTREME_EXCLUDE-1]);
	if(detectregion.topright.x-detectregion.topleft.x<MIN_EYE_WIDTH)
	{
		float width=detectregion.topright.x-detectregion.topleft.x;
		detectregion.topleft.x-=(MIN_EYE_WIDTH-width)/2;
		detectregion.topright.x+=(MIN_EYE_WIDTH-width)/2;
	}
	if(detectregion.bottomright.x-detectregion.bottomleft.x<MIN_EYE_WIDTH)
	{
		float width=detectregion.bottomright.x-detectregion.bottomleft.x;
		detectregion.bottomleft.x-=(MIN_EYE_WIDTH-width)/2;
		detectregion.bottomright.x+=(MIN_EYE_WIDTH-width)/2;
	}
	if(detectregion.bottomleft.y-detectregion.topleft.y<MIN_EYE_HEIGHT)
	{
		float height=detectregion.bottomleft.y-detectregion.topleft.y;
		detectregion.topleft.y-=(MIN_EYE_HEIGHT-height)/2;
		detectregion.bottomleft.y+=(MIN_EYE_HEIGHT-height)/2;
	}
	if(detectregion.bottomright.y-detectregion.topright.y<MIN_EYE_HEIGHT)
	{
		float height=detectregion.bottomright.y-detectregion.topright.y;
		detectregion.topright.y-=(MIN_EYE_HEIGHT-height)/2;
		detectregion.bottomright.y+=(MIN_EYE_HEIGHT-height)/2;
	}
	return detectregion;
}

void CChildView::Classify(float* val,bool* cls, int n)
{
	for(int i=0;i<n;i++)
	{
		cls[i]=true;
	}
	bool bChanged=false;

	do 
	{
		bChanged=false;
		float sum=0,sqsum=0;
		int cnt=0;
		for(int i=0;i<n;i++)
		{
			if(cls[i])
			{
				sum+=val[i];
				sqsum+=val[i]*val[i];
				cnt++;
			}
		}
		float mean=sum/cnt;
		float stddev=sqrt(sqsum/cnt-mean*mean);
		for(int i=0;i<n;i++)
		{
			if(fabs(val[i]-mean)>max(CLASS_THRESH*stddev,10))
			{
				if(cls[i])
				{
					cls[i]=false;
					bChanged=true;
				}
			}
			else
			{
				if(!cls[i])
				{
					cls[i]=true;
					bChanged=true;
				}
			}
		}
	}
	while(bChanged);
}

void CChildView::Classify(FaceData* val, bool* cls, int n)
{
	for(int i=0;i<n;i++)
	{
		cls[i]=true;
	}
	bool bChanged=false;

	do 
	{
		bChanged=false;
		CvPoint2D32f sum=cvPoint2D32f(0,0);
		float sqsum=0;
		int cnt=0;
		for(int i=0;i<n;i++)
		{
			if(cls[i])
			{
				sum.x+=val[i].x;
				sum.y+=val[i].y;
				sqsum+=val[i].x*val[i].x+val[i].y*val[i].y;
				cnt++;
			}
		}
		float meanx=sum.x/cnt;
		float meany=sum.y/cnt;
		float stddev=sqrt(sqsum/cnt-meanx*meanx-meany*meany);
		for(int i=0;i<n;i++)
		{
			if(sqrt(pow(val[i].x-meanx,2)+pow(val[i].y-meany,2))>max(CLASS_THRESH*stddev,10))
			{
				if(cls[i])
				{
					cls[i]=false;
					bChanged=true;
				}
			}
			else
			{
				if(!cls[i])
				{
					cls[i]=true;
					bChanged=true;
				}
			}
		}
	}
	while(bChanged);
}

void CChildView::DrawDetectRegion(CDC* pDC)
{
	DetectType RegionType[3]={Det_LeftEye, Det_RightEye, Det_Mouth};
	FaceRegion* detregion[3]={&m_rgnlEye, &m_rgnrEye, &m_rgnMouth};
	FaceRegion* adjregion[3]={&m_rgnlEadj, &m_rgnrEadj, &m_rgnMadj};
	CPen pen,*oldpen=NULL;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	oldpen = pDC->SelectObject(&pen);

	for(int i=0;i<3;i++)
	{
		FaceRegion region;
		if(!m_bDet)
			region = GetDetectedRegion(RegionType[i]);
		else if(!m_bShowAdj)
			region = *detregion[i];
		else
			region = *adjregion[i];
		pDC->MoveTo(region.topleft.x,region.topleft.y);
		pDC->LineTo(region.topright.x,region.topright.y);
		pDC->LineTo(region.bottomright.x,region.bottomright.y);
		pDC->LineTo(region.bottomleft.x,region.bottomleft.y);
		pDC->LineTo(region.topleft.x,region.topleft.y);
	}

	pDC->SelectObject(oldpen);
}

void CChildView::OneTimeDetect()
{
	RestoreEyeData(Grid);
	for(int i=0;i<EYE_GRID_NX*EYE_GRID_NY;i++)
	{
		while(Fit(&lEGrid[i]));
		while(Fit(&rEGrid[i]));
	}
	for(int i=0;i<MOUTH_GRID_N;i++)
	{
		while(Fit(&MGrid[i]));
	}

	AdjustDetRegion();
	m_bDet = true;
}

void CChildView::AdjustDetRegion()
{
	m_rgnlEadj = m_rgnlEye = GetDetectedRegion(Det_LeftEye);
	m_rgnrEadj = m_rgnrEye = GetDetectedRegion(Det_RightEye);
	m_rgnMadj = m_rgnMouth = GetDetectedRegion(Det_Mouth);

	CvPoint2D32f* left[2][2] = {{&m_rgnlEadj.topleft, &m_rgnlEadj.bottomleft}, {&m_rgnrEadj.topleft, &m_rgnrEadj.bottomleft}};
	CvPoint2D32f* right[2][2] = {{&m_rgnlEadj.topright, &m_rgnlEadj.bottomright}, {&m_rgnrEadj.topright, &m_rgnrEadj.bottomright}};
	for(int i=0;i<2;i++)
	{
		float CenterlE = (left[0][i]->y+right[0][i]->y)/2;
		float CenterrE = (left[1][i]->y+right[1][i]->y)/2;
		float offsetE = CenterlE-CenterrE;
		float adjE=0;
		if(offsetE<-MAX_EYE_OFFSET)
		{
			adjE = (-MAX_EYE_OFFSET-offsetE)/2;
		}
		else if(offsetE>MAX_EYE_OFFSET)
		{
			adjE = (MAX_EYE_OFFSET-offsetE)/2;
		}
		left[0][i]->y+=adjE;
		right[0][i]->y+=adjE;
		left[1][i]->y-=adjE;
		right[1][i]->y-=adjE;
	}

	float lhM = m_rgnMadj.bottomleft.y-m_rgnMadj.topleft.y;
	float rhM = m_rgnMadj.bottomright.y-m_rgnMadj.topright.y;
	float offsetM=lhM-rhM;
	float adjM=0;
	if(offsetM < -MAX_MOUTH_TRPZ)
	{
		adjM = (-MAX_MOUTH_TRPZ-offsetM)/2;
	}
	else if(offsetM > MAX_MOUTH_TRPZ)
	{
		adjM = (MAX_MOUTH_TRPZ-offsetM)/2;
	}
	m_rgnMadj.topleft.y-=adjM;
	m_rgnMadj.bottomleft.y+=adjM;
	m_rgnMadj.topright.y+=adjM;
	m_rgnMadj.bottomright.y-=adjM;
}

void CChildView::OnFileOpen()
{
	// TODO: Add your command handler code here
	char* strFilter = "All Image Files|*.bmp;*.jpg;*.png|Bitmap Files|*.bmp|JPEG Files|*.jpg|Potable Net Graph|*.png|All Files|*.*||";
	CFileDialog dlg(true, NULL, NULL, 0, strFilter, this);
	if(dlg.DoModal()==IDOK)
	{
		CString name = dlg.GetPathName();
		IplImage* image = cvLoadImage((LPCTSTR)name);
		float hs=800./image->width;
		float vs=600./image->height;
		float wobj=min(hs,vs)*image->width;
		float hobj=min(hs,vs)*image->height;
		cvReleaseImage(&m_imgObj);
		m_imgObj = cvCreateImage(cvSize(wobj, hobj),IPL_DEPTH_8U,3);
		cvResize(image, m_imgObj);
		cvReleaseImage(&image);
		ClearBuf();
		cvZero(m_face);
		cvZero(m_fgray);
		m_bDet = false;
		m_bAnimation = false;
		KillTimer(1);
		m_fMorph = 0;
		m_rcFace = Detect(m_imgObj);
		Draw();
	}
}
