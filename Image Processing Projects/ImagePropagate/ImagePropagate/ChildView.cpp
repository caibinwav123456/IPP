
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "ImagePropagate.h"
#include "ChildView.h"
#include "FFT.h"
#include "stdlib.h"
#include "time.h"
#include "Propagate.h"
#include "ImageProcess3D.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView():m_mat(1,0,0,0,1,0,0,0,1),m_vec(0,0,0),m_dlgMat(this),
	m_vClr(255,255,255),m_vBkClr(0,0,0)
{
	m_src = NULL;
	m_dest = NULL;
	m_xy = 0;
	m_yx = 0;
	m_seed = (int)time(NULL);
	m_fRadius = 1;
	m_fBrightness = 1;
	m_fBlackness = 0;
	m_fIntense = 0;
	m_ProcessIndex = 0;
}

CChildView::~CChildView()
{
	cvReleaseImage(&m_src);
	cvReleaseImage(&m_dest);
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_COMMAND(ID_FILE_OPEN2, &CChildView::OnFileOpen2)
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_PROCESS_COLOR, &CChildView::OnProcessColor)
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
	CRect rcClient;
	GetClientRect(rcClient);
	CDC mdc;
	mdc.CreateCompatibleDC(&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
	CBitmap* oldbmp = mdc.SelectObject(&bmp);
	mdc.FillSolidRect(rcClient, RGB(255,255,255));
	if(m_dest)
		DispImage(&mdc, m_dest, CPoint(0,0));
	CString str;
	str.Format(_T("xy=%4.2f,yx=%4.2f"), m_xy, m_yx);
	mdc.SetBkMode(TRANSPARENT);
	mdc.SetTextColor(RGB(255,0,0));
	//mdc.TextOut(0,0,str);
	dc.BitBlt(0,0,rcClient.Width(), rcClient.Height(), &mdc, 0,0,SRCCOPY);
	mdc.SelectObject(oldbmp);
	mdc.DeleteDC();
	bmp.DeleteObject();
}

void CChildView::OnFileOpen2()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("All Files|*.*||"), this);
	if(dlg.DoModal() == IDOK)
	{
		CString strName = dlg.GetPathName();
		cvReleaseImage(&m_src);
		m_src=cvLoadImage(CT2A(strName));
		if(m_src && (m_src->width>1024 || m_src->height>1024))
		{
			float xscale = 1024./m_src->width;
			float yscale = 1024./m_src->height;
			float scale = min(xscale, yscale);
			int width = cvRound(m_src->width*scale);
			int height = cvRound(m_src->height*scale);
			IplImage* tmp = m_src;
			m_src = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
			cvResize(tmp, m_src);
			cvReleaseImage(&tmp);
		}
		if(m_src)
		{
			cvReleaseImage(&m_dest);
			m_dest = cvCreateImage(cvGetSize(m_src), IPL_DEPTH_8U, 3);
			ProcessAll();
			Draw();
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
//Sketch
void CChildView::Process(IplImage* src, IplImage* dest)
{
	IplImage* gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, gray, CV_RGB2GRAY);
	CvScalar sum = cvSum(gray);
	float brightness = sum.val[0]/(gray->width*gray->height);
	IplImage* grayf = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	cvCvtScale(gray, grayf);
	cvReleaseImage(&gray);
	CvMat* kernel = cvCreateMat(1, 1, CV_32FC1);
	CvMat* kernel2 = cvCreateMat(41,41, CV_32FC1);
	CvMat* kernel3 = cvCreateMat(51,51, CV_32FC1);
	cvSet(kernel, cvScalar(1.0f/1));
	cvSet(kernel2, cvScalar(1.0f/41/41));
	cvSet(kernel3, cvScalar(1.0f/51/51));
	IplImage* smooth = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	IplImage* smooth2 = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	IplImage* smooth3 = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	cvFilter2D(grayf, smooth, kernel,cvPoint(0,0));
	cvFilter2D(grayf, smooth2, kernel2, cvPoint(20,20));
	cvFilter2D(grayf, smooth3, kernel3, cvPoint(25,25));
	IplImage* diff = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	cvSub(smooth, smooth2, diff);
	cvReleaseImage(&grayf);
	cvReleaseImage(&smooth);
	cvReleaseImage(&smooth2);
	IplImage* destg = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	float ext = (1-expf(-(brightness-93)/(188-93)*20))/(1-expf(-20));
	if(ext<0)ext=0;
	if(ext>1)ext=1;
	int level = 235+ext*(250-235);//250;//188,93|250,235
	float nmin = level;
	float nmax = 255-level;
	float scale = 1.8+ext*(2.5-1.8);//1.8;//2.5;
	float scale2 = nmax*(1.5/25);
	for(int i=0;i<diff->height;i++)
	{
		for(int j=0;j<diff->width;j++)
		{
			float* ptr = (float*)PTR_PIX(*diff, j, i);
			uchar* ptr2 = PTR_PIX(*destg, j, i);
			float* ptr3 = (float*)PTR_PIX(*smooth3, j, i);
			float re;
			float input = *ptr;//+1
			float s = *ptr3;
			if(s<10)s=10;
			input/=s/125;
			if(input<0)
			{
				re = (exp(input*scale/nmin)-1)*nmin;
			}
			else
			{
				re = (1-exp(-input*scale2/nmax))*nmax;
			}
			re+=level;
			//re+=*ptr3-220;
			*ptr2 = max(0,min(re, 255));
		}
	}
	//cvScale(diff, destg, 3.5, level);
	cvCvtColor(destg, dest, CV_GRAY2RGB);
	cvReleaseImage(&diff);
	cvReleaseImage(&destg);
	cvReleaseImage(&smooth3);
	cvReleaseMat(&kernel);
	cvReleaseMat(&kernel2);
	cvReleaseMat(&kernel3);
}

void Sketch2(IplImage* src, IplImage* dest)
{
	IplImage* gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, gray, CV_RGB2GRAY);
	CvScalar sum = cvSum(gray);
	float brightness = sum.val[0]/(gray->width*gray->height);

	IplImage* integ = cvCreateImage(cvSize(src->width+1, src->height+1), IPL_DEPTH_64F, 1);
	IplImage* sqinteg = cvCreateImage(cvSize(src->width+1, src->height+1), IPL_DEPTH_64F, 1);
	IplImage* destg = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvIntegral(gray, integ, sqinteg);
	int span = 20, span2 = 25;
	float scale = 2.2;
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
			float l1 = mean+stddev*0.5;
			float l2 = *PTR_PIX(*gray, j, i);

			float diff = l2-l1;
			float level = 250;
			if(mean2<10)mean2=10;
			diff/=mean2/125;
			float re = level+diff*scale;

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

//wood sketch
void CChildView::Process02(IplImage* src, IplImage* dest)
{
	Vec3 black(0,0,0);
	CvScalar sum = cvSum(src);
	int npx = src->width*src->height;
	float thresh = (sum.val[0]+sum.val[1]+sum.val[2])/npx/3;
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			uchar* ptr = PTR_PIX(*src, j, i);
			Vec3 vptr(*ptr, *(ptr+1), *(ptr+2));
			float gray = (1+SmoothStep(length(vptr-black)/sqrt(3.0), thresh, thresh/3))/2;
			uchar* ptrdest = PTR_PIX(*dest, j, i);
			uchar c = gray*255;
			*ptrdest = c;
			*(ptrdest+1) = c;
			*(ptrdest+2) = c;
		}
	}
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	cvReleaseImage(&m_dest);
	m_dest = cvCreateImage(cvSize(800, 800), IPL_DEPTH_8U, 3);
	Process6(m_src, m_dest);
	m_dlgMat.m_pMat = &m_mat;
	m_dlgMat.m_pVec = &m_vec;
	m_dlgMat.Create(CDialogMat::IDD, this);
	m_dlgMat.ShowWindow(SW_HIDE);
	return 0;
}
//brain tex
void CChildView::Process2(IplImage* dest)
{
	IplImage* fimg = cvCreateImage(cvGetSize(dest), IPL_DEPTH_32F, 1);
	IplImage* img = cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 1);
	srand(m_seed);
	Perlin(fimg, dest->width*1/3, dest->height*1/3);//, m_xy, m_yx
	for(int i=0;i<fimg->height;i++)
	{
		for(int j=0;j<fimg->width;j++)
		{
			float* ptr = (float*)PTR_PIX(*fimg, j, i);
			uchar* ptr2 = PTR_PIX(*img, j, i);
			*ptr2 = (uchar)(SmoothStep(fabs(*ptr*2-1), 0.3, 0.15)*255);
		}
	}
	//cvCvtScale(fimg, img, 255);
	cvMerge(img, img, img, NULL, dest);
	cvReleaseImage(&fimg);
	cvReleaseImage(&img);
}

struct fill_data
{
	int least_non_filled;
	int npt;
	Vec3 acc;
	CvScalar sqacc;
	IplImage* src;
	IplImage* dest;
	IplImage* mask;
};

#define STDDEV_THRESH 15
bool SpreadFunc(void* param, CvPoint pt, CvPoint ptorg, IplImage* pMask, UINT8 flags)
{
	fill_data& data= *(fill_data*)param;
	int n = data.npt+1;
	uchar* ptr = PTR_PIX(*data.src, pt.x, pt.y);
	Vec3 val(*ptr, *(ptr+1), *(ptr+2));
	Vec3 acc2 = data.acc + val;
	CvScalar sq = cvScalar(powf(val.x, 2), powf(val.y, 2), powf(val.z, 2));
	CvScalar sqacc2 = cvScalar(data.sqacc.val[0]+sq.val[0], data.sqacc.val[1]+sq.val[1], data.sqacc.val[2]+sq.val[2]);

	double stddev = sqrt((sqacc2.val[0]+sqacc2.val[1]+sqacc2.val[2])/n-dot(acc2, acc2)/n/n);
	if(stddev<=STDDEV_THRESH || data.npt==0)
	{
		data.npt = n;
		data.acc = acc2;
		data.sqacc = sqacc2;
		ASSERT(pt.y*data.src->width+pt.x >= data.least_non_filled);
		if(pt.y*data.src->width+pt.x == data.least_non_filled)
		{
			int i;
			for(i=data.least_non_filled+1;i<data.src->width*data.src->height;i++)
			{
				if(*PTR_PIX(*pMask, i%data.src->width, i/data.src->width) == 0)
					break;
			}
			data.least_non_filled = i;
		}
		return true;
	}
	else
	{
		return false;
	}
}
#define STDDEV_THRESH2 3
bool SpreadFunc7(void* param, CvPoint pt, CvPoint ptorg, IplImage* pMask, UINT8 flags)
{
	fill_data& data= *(fill_data*)param;
	int n = data.npt+1;

	uchar* pix = PTR_PIX(*data.src, pt.x, pt.y);
	uchar* pixorg = PTR_PIX(*data.src, ptorg.x, ptorg.y);
	Vec3 c(*pix, *(pix+1), *(pix+2));
	Vec3 corg(*pixorg, *(pixorg+1), *(pixorg+2));
	Vec3 acc = data.acc + c;
	float diff = (c-corg).length();
	float cl = corg.length();
	if(diff < STDDEV_THRESH2*cl/255 || data.npt==0)
	{
		data.npt = n;
		data.acc = acc;
		ASSERT(pt.y*data.src->width+pt.x >= data.least_non_filled);
		if(pt.y*data.src->width+pt.x == data.least_non_filled)
		{
			int i;
			for(i=data.least_non_filled+1;i<data.src->width*data.src->height;i++)
			{
				if(*PTR_PIX(*pMask, i%data.src->width, i/data.src->width) == 0)
					break;
			}
			data.least_non_filled = i;
		}
		return true;
	}
	else
	{
		return false;
	}
}
#define PIXEL_THRESH  100
bool SpreadFunc2(void* param, CvPoint pt, CvPoint ptorg, IplImage* pMask, UINT8 flags)
{
	fill_data& data= *(fill_data*)param;
	if(data.npt>PIXEL_THRESH)
		return false;
	int n = data.npt+1;
	uchar* ptr = PTR_PIX(*data.src, pt.x, pt.y);
	Vec3 val(*ptr, *(ptr+1), *(ptr+2));
	Vec3 acc2 = data.acc + val;
	if(*PTR_PIX(*data.mask, pt.x, pt.y)>0)
	{
		data.npt = n;
		data.acc = acc2;
		ASSERT(pt.y*data.src->width+pt.x >= data.least_non_filled);
		if(pt.y*data.src->width+pt.x == data.least_non_filled)
		{
			int i;
			for(i=data.least_non_filled+1;i<data.src->width*data.src->height;i++)
			{
				if(*PTR_PIX(*pMask, i%data.src->width, i/data.src->width) == 0
					&& *PTR_PIX(*data.mask, i%data.src->width, i/data.src->width)>0)
					break;
			}
			data.least_non_filled = i;
		}
		return true;
	}
	else
	{
		return false;
	}
}

#define PIXEL_THRESH2 40
bool SpreadFunc6(void* param, CvPoint pt, CvPoint ptorg, IplImage* pMask, UINT8 flags)
{
	fill_data& data= *(fill_data*)param;
	if(data.npt>PIXEL_THRESH2)
		return false;
	if(rand()%2 == 0 && data.npt!=0)
		return false;
	uchar* ptr = PTR_PIX(*data.src, pt.x, pt.y);
	Vec3 val(*ptr, *(ptr+1), *(ptr+2));
	data.npt++;
	data.acc += val;
	ASSERT(pt.y*data.src->width+pt.x >= data.least_non_filled);
	if(pt.y*data.src->width+pt.x == data.least_non_filled)
	{
		int i;
		for(i=data.least_non_filled+1;i<data.src->width*data.src->height;i++)
		{
			if(*PTR_PIX(*pMask, i%data.src->width, i/data.src->width) == 0)
				break;
		}
		data.least_non_filled = i;
	}
	return true;
}
#define PIXEL_THRESH3 5
bool SpreadFunc9(void* param, CvPoint pt, CvPoint ptorg, IplImage* pMask, UINT8 flags)
{
	fill_data& data= *(fill_data*)param;
	if(data.npt>PIXEL_THRESH3)
		return false;
	if(rand()%2 == 0 && data.npt!=0)
		return false;
	uchar* ptr = PTR_PIX(*data.src, pt.x, pt.y);
	float val=*ptr;
	data.npt++;
	val-=10;
	uchar* ptrd = PTR_PIX(*data.dest, pt.x, pt.y);
	*ptrd = max(0, min(255, val));
	ASSERT(pt.y*data.src->width+pt.x >= data.least_non_filled);
	if(pt.y*data.src->width+pt.x == data.least_non_filled)
	{
		int i;
		for(i=data.least_non_filled+1;i<data.src->width*data.src->height;i++)
		{
			if(*PTR_PIX(*pMask, i%data.src->width, i/data.src->width) == 0)
				break;
		}
		data.least_non_filled = i;
	}
	return true;
}

//carbon draw(cartoon draw)
void CChildView::Process3(IplImage* src, IplImage* dest)
{
	CvMat* mat = cvCreateMat(5,5,CV_32FC1);
	cvSet(mat, cvScalar(1./25));
	IplImage* src2 = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cvFilter2D(src, src2, mat);
	CPropagate prop;
	prop.Init(src->width, src->height);
	fill_data data;
	data.src = src2;
	data.dest = dest;
	data.least_non_filled = 0;
	prop.SetFunc(SpreadFunc);
	while(data.least_non_filled < src->width*src->height)
	{
		CvPoint pt = cvPoint(data.least_non_filled%src->width, data.least_non_filled/src->width);
		prop.SetPoint(pt);
		data.npt = 0;
		data.acc = Vec3(0,0,0);
		data.sqacc = cvScalarAll(0);
		while(prop.Propagate(PROP_MODE_RECT, true, &data));
		Vec3 mean = data.acc/prop.m_ptacc;
		for(int i=0;i<data.npt;i++)
		{
			uchar* ptr = PTR_PIX(*dest, prop.m_bufacc[i].x, prop.m_bufacc[i].y);
			*ptr = mean.x;
			*(ptr+1) = mean.y;
			*(ptr+2) = mean.z;
		}
	}
	cvReleaseImage(&src2);
	cvReleaseMat(&mat);
}
//brain pic
void CChildView::Process4(IplImage* src, IplImage* dest)
{
	cvZero(dest);//cvCopyImage(src, dest);
	IplImage* noise = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	Process2(noise);
	CPropagate prop;
	prop.Init(src->width, src->height);
	fill_data data;
	data.src = src;
	data.dest = dest;
	data.mask = noise;
	data.least_non_filled = 0;
	for(int i=0;i<noise->height;i++)
	{
		for(int j=0;j<noise->width;j++)
		{
			if(*PTR_PIX(*noise, j ,i)>0)
			{
				data.least_non_filled = i*noise->height+j;
				goto next;
			}
		}
	}
next:
	prop.SetFunc(SpreadFunc2);
	while(data.least_non_filled < src->width*src->height)
	{
		CvPoint pt = cvPoint(data.least_non_filled%src->width, data.least_non_filled/src->width);
		prop.SetPoint(pt);
		data.npt = 0;
		data.acc = Vec3(0,0,0);
		data.sqacc = cvScalarAll(0);
		while(prop.Propagate(PROP_MODE_QUAD, true, &data));
		Vec3 mean = data.acc/prop.m_ptacc;
		for(int i=0;i<data.npt;i++)
		{
			uchar* ptr = PTR_PIX(*dest, prop.m_bufacc[i].x, prop.m_bufacc[i].y);
			uchar* ptr2 = PTR_PIX(*noise, prop.m_bufacc[i].x, prop.m_bufacc[i].y);

			*ptr = mean.x**ptr2/255;
			*(ptr+1) = mean.y**ptr2/255;
			*(ptr+2) = mean.z**ptr2/255;
		}
	}
	cvReleaseImage(&noise);
}

bool SpreadFunc3(void* param, CvPoint pt, CvPoint ptorg, IplImage* pMask, UINT8 flags)
{
	fill_data& data= *(fill_data*)param;
	if(data.npt>PIXEL_THRESH)
		return false;
	if(rand()%2 == 0)
		return false;
	data.npt++;
	ASSERT(pt.y*data.src->width+pt.x >= data.least_non_filled);
	if(pt.y*data.src->width+pt.x == data.least_non_filled)
	{
		int i;
		for(i=data.least_non_filled+1;i<data.src->width*data.src->height;i++)
		{
			if(*PTR_PIX(*pMask, i%data.src->width, i/data.src->width) == 0)
				break;
		}
		data.least_non_filled = i;
	}
	*PTR_PIX(*data.mask, pt.x, pt.y) = 255;
	return true;
}

float g_lumin[4] = {20, -20, 15, -15};
UINT8 g_flags[4] = {PROP_FLAG_TOP, PROP_FLAG_BOTTOM, PROP_FLAG_LEFT, PROP_FLAG_RIGHT};

bool SpreadFunc4(void* param, CvPoint pt, CvPoint ptorg, IplImage* pMask, UINT8 flags)
{
	fill_data& data= *(fill_data*)param;
	float c = 0;
	int nc = 0;
	for(int i=0;i<4;i++)
	{
		if(flags & g_flags[i])
		{
			c+=g_lumin[i];
			nc++;
		}
	}
	c/=nc;
	uchar* pix = PTR_PIX(*data.dest, ptorg.x, ptorg.y);
	Vec3 v(*pix,*(pix+1),*(pix+2));
	v+=Vec3(c,c,c);
	*pix = max(0,min(255, v.x));
	*(pix+1) = max(0,min(255, v.y));
	*(pix+2) = max(0,min(255, v.z));
/*	ASSERT(pt.y*data.src->width+pt.x >= data.least_non_filled);
	if(pt.y*data.src->width+pt.x == data.least_non_filled)
	{
		int i;
		for(i=data.least_non_filled+1;i<data.src->width*data.src->height;i++)
		{
			if(*PTR_PIX(*pMask, i%data.src->width, i/data.src->width) == 0)
				break;
		}
		data.least_non_filled = i;
	}
*/	return false;
}

bool SpreadFunc5(void* param, CvPoint pt, CvPoint ptorg, IplImage* pMask, UINT8 flags)
{
	fill_data& data= *(fill_data*)param;
	uchar* pix = PTR_PIX(*data.dest, pt.x, pt.y);
	return true;
}
bool SpreadFunc8(void* param, CvPoint pt, CvPoint ptorg, IplImage* pMask, UINT8 flags)
{
	fill_data& data= *(fill_data*)param;
	uchar* pix=PTR_PIX(*data.dest, pt.x, pt.y);
	*pix=max(0,255-(255-*pix)*2);
	*(pix+1)=max(0,255-(255-*(pix+1))*2);
	*(pix+2)=max(0,255-(255-*(pix+2))*2);
	return true;
}
//collage
void CChildView::Process5(IplImage* src, IplImage* dest)
{
	cvCopyImage(src,dest);
	CvMat* mat = cvCreateMat(5,5,CV_32FC1);
	cvSet(mat, cvScalar(1./25));
	IplImage* src2 = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cvFilter2D(src, src2, mat);
	IplImage* mask = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvZero(mask);
	CPropagate prop;
	prop.Init(src->width, src->height);
	fill_data data;
	data.src = src2;
	data.dest = dest;
	data.mask = mask;
	data.least_non_filled = 0;
	int p0 = src->width*src->height;
	while(data.least_non_filled < src->width*src->height)
	{
		CvPoint pt;
		do 
		{
			int x = rand()%src->width;
			int y = rand()%src->height;
			int p = (y*src->width+x)%(p0-data.least_non_filled)+data.least_non_filled;
			pt = cvPoint(p%src->width, p/src->width);
		} while (*PTR_PIX(*prop.m_mask, pt.x, pt.y)!=0);
		
		prop.SetPoint(pt);
		data.npt = 0;
		data.acc = Vec3(0,0,0);
		data.sqacc = cvScalarAll(0);
		prop.SetFunc(SpreadFunc3);
		while(prop.Propagate(PROP_MODE_RECT, true, &data));
		prop.m_pts = prop.m_ptacc;
		memcpy(prop.m_buf, prop.m_bufacc, prop.m_pts*sizeof(CvPoint));
		IplImage* internalmask = prop.m_mask;
		prop.m_mask = mask;
		prop.SetFunc(SpreadFunc4);
		prop.Propagate(PROP_MODE_RECT, false, &data);
		prop.m_mask = internalmask;
		for(int i=0;i<prop.m_ptacc;i++)
		{
			*PTR_PIX(*mask, prop.m_bufacc[i].x, prop.m_bufacc[i].y)=0;
		}
	}
	cvReleaseImage(&src2);
	cvReleaseImage(&mask);
	cvReleaseMat(&mat);
}

//carbon tex
void CChildView::Process11(IplImage* src, IplImage* dest)
{
	cvZero(dest);//cvCopyImage(src, dest);
	CPropagate prop;
	prop.Init(src->width, src->height);
	fill_data data;
	data.src = src;
	data.dest = dest;
	data.least_non_filled = 0;
	prop.SetFunc(SpreadFunc6);
	int p0 = src->width*src->height;
	while(data.least_non_filled < src->width*src->height)
	{
		CvPoint pt;
		do 
		{
			int x = rand()%src->width;
			int y = rand()%src->height;
			int p = (y*src->width+x)%(p0-data.least_non_filled)+data.least_non_filled;
			pt = cvPoint(p%src->width, p/src->width);
		} while (*PTR_PIX(*prop.m_mask, pt.x, pt.y)!=0);

		prop.SetPoint(pt);
		data.npt = 0;
		data.acc = Vec3(0,0,0);
		data.sqacc = cvScalarAll(0);
		while(prop.Propagate(PROP_MODE_QUAD, true, &data));
		Vec3 mean = data.acc/prop.m_ptacc;
		for(int i=0;i<data.npt;i++)
		{
			uchar* ptr = PTR_PIX(*dest, prop.m_bufacc[i].x, prop.m_bufacc[i].y);

			*ptr = mean.x;
			*(ptr+1) = mean.y;
			*(ptr+2) = mean.z;
		}
	}
}
//carbon draw2
void CChildView::Process12(IplImage* src, IplImage* dest)
{
	CvMat* mat = cvCreateMat(5,5,CV_32FC1);
	cvSet(mat, cvScalar(1./25));
	IplImage* src2 = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cvFilter2D(src, src2, mat);
	CPropagate prop;
	prop.Init(src->width, src->height);
	fill_data data;
	data.src = src2;
	data.dest = dest;
	data.least_non_filled = 0;
	prop.SetFunc(SpreadFunc7);
	while(data.least_non_filled < src->width*src->height)
	{
		CvPoint pt = cvPoint(data.least_non_filled%src->width, data.least_non_filled/src->width);
		prop.SetPoint(pt);
		data.npt = 0;
		data.acc = Vec3(0,0,0);
		data.sqacc = cvScalarAll(0);
		while(prop.Propagate(PROP_MODE_RECT, true, &data));
		Vec3 mean = data.acc/prop.m_ptacc;
		for(int i=0;i<data.npt;i++)
		{
			uchar* ptr = PTR_PIX(*dest, prop.m_bufacc[i].x, prop.m_bufacc[i].y);
			*ptr = mean.x;
			*(ptr+1) = mean.y;
			*(ptr+2) = mean.z;
		}
	}
	cvReleaseImage(&src2);
	cvReleaseMat(&mat);
}

//paper tex
void Paper(IplImage* dest)
{
	//cvSet(dest, cvScalarAll(255));

	int nTread = dest->width*dest->height/400;
	int max_length = dest->width/2;
	int min_length = dest->width/10;
	IplImage* mask = cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 1);
	CPropagate prop;
	prop.Init(dest->width, dest->height);
	fill_data data;
	data.src = NULL;
	data.dest = dest;
	data.least_non_filled = 0;
	cvZero(mask);
	for(int i=0;i<nTread;i++)
	{
		int x=(float)rand()/(RAND_MAX+1)*dest->width;
		int y=(float)rand()/(RAND_MAX+1)*dest->height;
		int length=min_length+(float)rand()/(RAND_MAX)*(max_length-min_length);
		Vec2 v;
		if(rand()%2)
		{
			v.x=1;
			v.y=0.2*rand()/(RAND_MAX+1)-0.1;
		}
		else
		{
			v.y=1;
			v.x=0.2*rand()/(RAND_MAX+1)-0.1;
		}
		Vec2 v2 = v;
		v.x = v2.x+0.2*v2.y;
		v.y = -0.2*v2.x+v2.y;
		prop.m_pts = 0;
		prop.m_ptacc = 0;
		for(int j=-length;j<length;j+=(rand()%3?1:2))
		{
			int startx=x+j/v.length()*v.x;
			int starty=y+j/v.length()*v.y;
			if(startx<0||startx>=dest->width||starty<0||starty>=dest->height)
				continue;
			if(*PTR_PIX(*mask, startx, starty)==0
				&& *PTR_PIX(*prop.m_mask, startx, starty)==0)
			{
				uchar* pix=PTR_PIX(*dest, startx, starty);
				*pix=max(0,(int)*pix-40);//255-(255-*pix)*2);
				*(pix+1)=max(0,(int)*(pix+1)-40);//255-(255-*(pix+1))*2);
				*(pix+2)=max(0,(int)*(pix+2)-40);//255-(255-*(pix+2))*2);
				*PTR_PIX(*mask, startx, starty) = 255;
				prop.m_buf[prop.m_pts] = cvPoint(startx, starty);
				prop.m_pts++;
			}
		}
		prop.SetFunc(SpreadFunc8);
		//for(int j=0;j<0;j++)
		//	prop.Propagate(PROP_MODE_RECT, true, &data);
		prop.SetFunc(SpreadFunc5);
		prop.Propagate(PROP_MODE_RECT, true, &data);
	}
	cvReleaseImage(&mask);
}
//paper
void CChildView::Process6(IplImage* src, IplImage* dest)
{
	cvSet(dest, cvScalar(100, 200, 200));
	Paper(dest);
}

void Ink(IplImage* src, IplImage* dest)
{
	cvCopyImage(src, dest);
	CPropagate prop;
	prop.Init(dest->width, dest->height);
	fill_data data;
	data.src = src;
	data.dest = dest;
	data.least_non_filled = 0;
	prop.SetFunc(SpreadFunc9);
	int p0 = dest->width*dest->height;
	int acc = 0;
	while(acc < dest->width*dest->height/3)
	{
		CvPoint pt;
		do 
		{
			int x = rand()%dest->width;
			int y = rand()%dest->height;
			int p = (y*dest->width+x)%(p0-data.least_non_filled)+data.least_non_filled;
			pt = cvPoint(p%dest->width, p/dest->width);
		} while (*PTR_PIX(*prop.m_mask, pt.x, pt.y)!=0);

		prop.SetPoint(pt);
		data.npt = 0;
		data.acc = Vec3(0,0,0);
		data.sqacc = cvScalarAll(0);
		while(prop.Propagate(PROP_MODE_QUAD, true, &data));
		acc+=prop.m_ptacc;
	}
}

void Ink2(IplImage* src, IplImage* dest)
{
	IplImage* texf = cvCreateImage(cvGetSize(dest), IPL_DEPTH_32F, 1);
	Perlin(texf, dest->width/2, dest->height/2, 0.9, 0.9);
	IplImage* tex = cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 1);
	cvCvtScale(texf, tex, 20);
	cvReleaseImage(&texf);
	cvSub(src, tex, dest);
	cvReleaseImage(&tex);
}

void CChildView::ColorM(IplImage* src, IplImage* dest)
{
	for(int i=0;i<m_src->height;i++)
	{
		for(int j=0;j<m_src->width;j++)
		{
			uchar* ptr = PTR_PIX(*src, j, i);
			Vec3 cs(*(ptr+2), *(ptr+1), *ptr);
			Vec3 cd = cs*m_mat+m_vec;
			for(int k=0;k<3;k++)
				cd.elem[k] = max(0, min(255, cd.elem[k]));
			uchar* ptrd = PTR_PIX(*dest, j, i);
			*ptrd = cd.elem[2];
			*(ptrd+1) = cd.elem[1];
			*(ptrd+2) = cd.elem[0];
		}
	}
}

//lomo frame
void CChildView::Process7(IplImage* src, IplImage* dest)
{
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			float sqrad = powf((float)(i-dest->height/2)/(dest->height/2),2)
				+ powf((float)(j-dest->width/2)/(dest->width/2),2);
			float intense = expf(-sqrad/powf(m_fRadius,2)/2);
			uchar* ptr = PTR_PIX(*src, j, i);
			uchar* ptrd = PTR_PIX(*dest, j, i);
			Vec3 cs(*ptr, *(ptr+1), *(ptr+2));
			Vec3 cd = cs*intense/255*m_vClr*m_fBrightness+m_vBkClr*(1-intense);
			*ptrd = max(0, min(255, cd.x));
			*(ptrd+1) = max(0, min(255, cd.y));
			*(ptrd+2) = max(0, min(255, cd.z));
		}
	}
}

float FrameFunc(float x, float edge, float left, float right)
{
	if(x >= left+edge && x < right-edge)
	{
		return 1;
	}
	else if(x < left || x > right)
	{
		return 0;
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

//lomo frame2
void CChildView::Process8(IplImage* src, IplImage* dest)
{
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			float intense = FrameFunc((float)i/dest->height, 1-m_fRadius, 0, 1)
				*FrameFunc((float)j/dest->width, 1-m_fRadius, 0, 1);
			uchar* ptr = PTR_PIX(*src, j, i);
			uchar* ptrd = PTR_PIX(*dest, j, i);
			Vec3 cs(*ptr, *(ptr+1), *(ptr+2));
			Vec3 cd = cs*intense*m_vClr/255*m_fBrightness+m_vBkClr*(1-intense);
			*ptrd = max(0, min(255, cd.x));
			*(ptrd+1) = max(0, min(255, cd.y));
			*(ptrd+2) = max(0, min(255, cd.z));
		}
	}
}

//lomo effect
void CChildView::Process9(IplImage* src, IplImage* dest)
{
	CvScalar sum = cvSum(src);
	int avggray = (sum.val[0]+sum.val[1]+sum.val[2])/(m_dest->width*m_dest->height)/3;
	float blackness = m_fBlackness*avggray;
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			uchar* ptr = PTR_PIX(*src, j, i);
			uchar* ptrd = PTR_PIX(*dest, j, i);
			Vec3 cs(*ptr, *(ptr+1), *(ptr+2));
			float gray = (cs.x+cs.y+cs.z)/3;
			float defect = (gray<blackness?
				powf((gray-blackness)/max(blackness, 1), 2)
				: 0)*m_fIntense;
			Vec3 cd = cs - Vec3(defect, defect, defect)*255;
			*ptrd = max(0, min(255, cd.x));
			*(ptrd+1) = max(0, min(255, cd.y));
			*(ptrd+2) = max(0, min(255, cd.z));
		}
	}
}

//Shock
void CChildView::Process10(IplImage* src, IplImage* dest)
{
	float blurf = m_fIntense;
	CvPoint pt = cvPoint(src->width/2, src->height/2);
	CvPoint pts[4] = {cvPoint(0,0), cvPoint(src->width,0), cvPoint(0,src->height), cvPoint(src->width,src->height)};
	float polar = 0;
	for(int i=0;i<4;i++)
	{
		Vec2 v(pts[i].x-pt.x,pts[i].y-pt.y);
		float p = v.length();
		if(p>polar)polar=p;
	}
	CvSize szPolarImg = cvSize(ceilf(polar*(1+max(-blurf,0)))+2, polar*2*CV_PI);
	IplImage* polaracc = cvCreateImage(szPolarImg, IPL_DEPTH_32F, 3);
	for(int i=0;i<polaracc->height;i++)
	{
		Vec3 acc(0,0,0);
		for(int j=0;j<polaracc->width;j++)
		{
			float x=(pt.x+j*cos((float)i/polar))/src->width;
			float y=(pt.y+j*sin((float)i/polar))/src->height;
			Vec2 v(x,y);
			Vec3 c = Sample(src, v);
			float* pix2 = (float*)PTR_PIX(*polaracc, j, i);
			*pix2 = acc.x;
			*(pix2+1) = acc.y;
			*(pix2+2) = acc.z;
			acc+=c;
		}
	}
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			Vec2 v(j-pt.x, i-pt.y);
			float p = v.length();
			float theta = acos(v.x/p);
			if(p==0)theta=0;
			if(v.y<0)theta=2*CV_PI-theta;
			float l=p/polaracc->width;
			float l2=p/polaracc->width-powf(p/polar,2)*polar/polaracc->width*blurf;
			Vec3 cb;
			if(fabs(l2-l)>=0.00001)
			{
				Vec3 c=Sample(polaracc, Vec2(l, theta/(2*CV_PI)));
				Vec3 c2=Sample(polaracc, Vec2(l2, theta/(2*CV_PI)));
				cb=(c2-c)/((l2-l)*polaracc->width);
			}
			else
			{
				Vec3 c=Sample(polaracc, Vec2(l+0.5/polaracc->width, theta/(2*CV_PI)));
				Vec3 c2=Sample(polaracc, Vec2(l+1.5/polaracc->width, theta/(2*CV_PI)));
				cb=c2-c;
			}
			uchar* pix = PTR_PIX(*dest, j, i);
			*pix = cb.x;
			*(pix+1) = cb.y;
			*(pix+2) = cb.z;
		}
	}
	cvReleaseImage(&polaracc);
}
IplImage* g_src = NULL;
IplImage* g_tex = NULL;
void PixelFunc(RawImage* target, Point2D pos, int count, float* pdata)
{
	ASSERT(count==2);
	Vec2* tex = (Vec2*)pdata;
	Vec2 text = (*tex-Vec2(0.5,0.5))*2;
	text.x/=2;
	float l = exp(-4*dot(text,text));
	uchar* pix=PTR_PIX(*target, pos.x, pos.y);
	uchar* pixs=PTR_PIX(*g_src, pos.x, pos.y);
	uchar* pixp=PTR_PIX(*g_tex, pos.x, pos.y);
	Vec3 c(*pix, *(pix+1), *(pix+2));
	Vec3 cs(*pixs, *(pixs+1), *(pixs+2));
	Vec3 cp(*pixp, *(pixp+1), *(pixp+2));
	Vec2 p((float)pos.x/g_src->width,(float)pos.y/g_src->height);
	p = p*2-Vec2(1,1);
	float lb = exp(-dot(p,p)*0.6);
	Vec3 w(255,255,255);
	Vec3 cd = c*(1-l)+(cp*(1-lb)+cs*lb)*l;
	*pix=max(0,cd.x);
	*(pix+1)=max(0,cd.y);
	*(pix+2)=max(0,cd.z);
}

void Stroke(IplImage* dest, Vec2 pt, Vec2 dir, float arc, float w, float l, int nSeg)
{
	float radius = 1./arc;
	Vec2 polar(dir.y, -dir.x);
	float theta = acos(-polar.x/polar.length());
	if(polar.y>0)theta=-theta;
	polar = polar.normalize();
	dir=dir.normalize();
	Vec2 c = pt+polar*radius;
	float dtheta = l/2/radius;
	if(arc!=0)
	{
		for(int i=0;i<nSeg;i++)
		{
			Vec2 tri[2][3];
			float vdata[2][2][3];
			float r1 = radius-w/2;
			float r2 = radius+w/2;
			float th1=theta-dtheta/2+dtheta/nSeg*i;
			float th2=th1+dtheta/nSeg;
			tri[0][0] = c+r1*Vec2(cos(th1),sin(th1));
			tri[0][1] = c+r2*Vec2(cos(th1),sin(th1));
			tri[0][2] = c+r2*Vec2(cos(th2),sin(th2));
			tri[1][0] = tri[0][2];
			tri[1][1] = c+r1*Vec2(cos(th2),sin(th2));
			tri[1][2] = tri[0][0];
			vdata[0][0][0]=1./nSeg*i;
			vdata[0][1][0]=0;
			vdata[0][0][1]=1./nSeg*i;
			vdata[0][1][1]=1;
			vdata[0][0][2]=1./nSeg*(i+1);
			vdata[0][1][2]=1;
			vdata[1][0][0]=1./nSeg*(i+1);
			vdata[1][1][0]=1;
			vdata[1][0][1]=1./nSeg*(i+1);
			vdata[1][1][1]=0;
			vdata[1][0][2]=1./nSeg*i;
			vdata[1][1][2]=0;
			for(int j=0;j<2;j++)
				Triangle2D(dest, tri[j], 2, vdata[j], PixelFunc);
		}
	}
	else
	{
		for(int i=0;i<nSeg;i++)
		{
			Vec2 tri[2][3];
			float vdata[2][2][3];
			tri[0][0] = pt-l/2*dir+l/nSeg*i*dir-polar*w/2;
			tri[0][1] = pt-l/2*dir+l/nSeg*i*dir+polar*w/2;
			tri[0][2] = pt-l/2*dir+l/nSeg*(i+1)*dir+polar*w/2;
			tri[1][0] = tri[0][2];
			tri[1][1] = pt-l/2*dir+l/nSeg*(i+1)*dir-polar*w/2;
			tri[1][2] = tri[0][0];
			vdata[0][0][0]=1./nSeg*i;
			vdata[0][1][0]=0;
			vdata[0][0][1]=1./nSeg*i;
			vdata[0][1][1]=1;
			vdata[0][0][2]=1./nSeg*(i+1);
			vdata[0][1][2]=1;
			vdata[1][0][0]=1./nSeg*(i+1);
			vdata[1][1][0]=1;
			vdata[1][0][1]=1./nSeg*(i+1);
			vdata[1][1][1]=0;
			vdata[1][0][2]=1./nSeg*i;
			vdata[1][1][2]=0;
			for(int j=0;j<2;j++)
				Triangle2D(dest, tri[j], 2, vdata[j], PixelFunc);
		}
	}
}

Vec2 RandVec()
{
	return Vec2(2*(float)rand()/RAND_MAX-1,2*(float)rand()/RAND_MAX-1);
}

void Strokes(IplImage* src, IplImage* dest)
{
	srand(time(NULL));
	//cvSet(dest, cvScalarAll(255));
	g_src = src;
	for(int i=0;i<3;i++)
	{
		Vec2 StrokeSet;
		while(StrokeSet==Vec2(0,0))
			StrokeSet = RandVec();
		StrokeSet*=Vec2(dest->width, dest->height);
		StrokeSet = StrokeSet.normalize();
		float ext = max(dest->width,dest->height);
		float angle = ((float)rand()/RAND_MAX*2-1)*0.01;
		Vec2 rotx = Vec2(cos(angle),sin(angle));
		Vec2 roty(-rotx.y, rotx.x);
		Vec2 dir(StrokeSet.y,-StrokeSet.x);
		int nStroke = 150;
		dir = Vec2(dir.x*cos(angle*nStroke/2)-dir.y*sin(angle*nStroke/2),
			dir.x*sin(angle*nStroke/2)+dir.y*cos(angle*nStroke/2));
		for(int j=0;j<nStroke;j++)
		{
			Vec2 dir2 = dir+RandVec()*0.1;
			dir2=dir2.normalize();
			float w=ext/80;
			float l=ext/800*(500+(float)rand()/RAND_MAX*1000);
			float arc = 1./ext*((float)rand()/RAND_MAX+0.5);
			int nSeg = l/10;
			Vec2 pt(dest->width/2, dest->height/2);
			pt+=StrokeSet*(j-nStroke/2)/nStroke*ext;
			pt+=RandVec()*40*ext/800;
			pt+=((float)rand()/RAND_MAX*2-1)*50*dir*ext/800;
			Stroke(dest, pt, dir, arc, w, l, nSeg);
			dir = Vec2(dot(dir,rotx),dot(dir,roty));
		}
	}
	g_src = NULL;
}

void CChildView::Process13(IplImage* src, IplImage* dest)
{
	IplImage* papertex = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cvSet(papertex, cvScalarAll(128));
	Paper(papertex);
	IplImage* inktex = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage* gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	IplImage* image = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	cvCvtColor(src, gray, CV_RGB2GRAY);
	Ink2(gray, inktex);
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			uchar* pix = PTR_PIX(*gray, j, i);
			uchar* pixd = PTR_PIX(*image, j, i);
			uchar* pixpt = PTR_PIX(*papertex, j, i);
			uchar* pixit = PTR_PIX(*inktex, j, i);
			//Vec3 v(*pix, *pix, *pix);
			Vec3 vp(*pixpt, *(pixpt+1), *(pixpt+2));
			Vec3 vi(*pixit, *pixit, *pixit);
			//v+=(vi-Vec3(255,255,255))*0.1;
			Vec3 v=vp+(vi-Vec3(255,255,255));
			*pixd = max(0,min(255,v.x));
			*(pixd+1) = max(0,min(255,v.y));
			*(pixd+2) = max(0,min(255,v.z));
		}
	}
	cvScale(papertex, papertex, 0.3, 255*0.7);
	cvCopyImage(papertex, m_dest);
	g_tex = papertex;
	Strokes(image, m_dest);
	g_tex = NULL;
	cvReleaseImage(&gray);
	cvReleaseImage(&papertex);
	cvReleaseImage(&inktex);
	cvReleaseImage(&image);
}

void CChildView::ProcessAll()
{
	switch(m_ProcessIndex)
	{
	case 0:
		if(m_src)
		{
			Process(m_src, m_dest);
		}
		break;
	case 1:
		if(m_src)
		{
			Sketch2(m_src, m_dest);
		}
		break;
	case 2:
		if(m_src)
		{
			Process02(m_src, m_dest);
		}
		break;
	case 3:
		Process2(m_dest);
		break;
	case 4:
		if(m_src)
		{
			Process3(m_src, m_dest);
		}
		break;
	case 5:
		if(m_src)
		{
			Process4(m_src, m_dest);
		}
		break;
	case 6:
		if(m_src)
		{
			Process5(m_src, m_dest);
		}
		break;
	case 7:
		Process6(m_src, m_dest);
		break;
	case 8:
		if(m_src)
		{
			m_dlgMat.ShowWindow(SW_SHOW);
			ColorM(m_src, m_dest);
		}
		break;
	case 9:
		if(m_src)
		{
			m_vClr = m_mat._0*255;
			m_vBkClr = m_mat._1*255;
			Process7(m_src, m_dest);
		}
		break;
	case 10:
		if(m_src)
		{
			m_vClr = m_mat._0*255;
			m_vBkClr = m_mat._1*255;
			Process8(m_src, m_dest);
		}
		break;
	case 11:
		if(m_src)
		{
			Process9(m_src, m_dest);
		}
		break;
	case 12:
		if(m_src)
		{
			Process10(m_src, m_dest);
		}
		break;
	case 13:
		if(m_src)
		{
			Process11(m_src, m_dest);
		}
		break;
	case 14:
		if(m_src)
		{
			Process12(m_src, m_dest);
		}
		break;
	case 15:
		if(m_src)
		{
			IplImage* tmp = cvCreateImage(cvGetSize(m_src),IPL_DEPTH_8U,3);
			Process(m_src, tmp);
			Process13(tmp, m_dest);
			cvReleaseImage(&tmp);
		}
		break;
	}
}

#define NUM_PROC  16
void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	switch(nChar)
	{
	case 'A':
		m_xy+=0.01;
		Process2(m_dest);
		Draw();
		break;
	case 'Z':
		m_xy-=0.01;
		Process2(m_dest);
		Draw();
		break;
	case 'S':
		m_yx+=0.01;
		Process2(m_dest);
		Draw();
		break;
	case 'X':
		m_yx-=0.01;
		Process2(m_dest);
		Draw();
		break;
	case 'D':
		m_xy+=0.01;
		m_yx+=0.01;
		Process2(m_dest);
		Draw();
		break;
	case 'C':
		m_xy-=0.01;
		m_yx-=0.01;
		Process2(m_dest);
		Draw();
		break;
	case VK_LEFT:
		m_ProcessIndex--;
		if(m_ProcessIndex == -1)
			m_ProcessIndex = NUM_PROC-1;
		ProcessAll();
		Draw();
		break;
	case VK_RIGHT:
		m_ProcessIndex++;
		if(m_ProcessIndex == NUM_PROC)
			m_ProcessIndex = 0;
		ProcessAll();
		Draw();
		break;
	case 'F':
		m_fRadius+=0.01;
		ProcessAll();
		Draw();
		break;
	case 'V':
		m_fRadius-=0.01;
		ProcessAll();
		Draw();
		break;
	case 'G':
		m_fBrightness+=0.01;
		ProcessAll();
		Draw();
		break;
	case 'B':
		m_fBrightness-=0.01;
		ProcessAll();
		Draw();
		break;
	case 'H':
		m_fBlackness+=0.01;
		ProcessAll();
		Draw();
		break;
	case 'N':
		m_fBlackness-=0.01;
		ProcessAll();
		Draw();
		break;
	case 'J':
		m_fIntense+=0.01;
		ProcessAll();
		Draw();
		break;
	case 'M':
		m_fIntense-=0.01;
		ProcessAll();
		Draw();
		break;
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CChildView::OnDestroy()
{
	CWnd::OnDestroy();
	m_dlgMat.DestroyWindow();
	RawImage::ReleaseInternalHdr();
	// TODO: Add your message handler code here
}

LRESULT CChildView::OnProcessColor(WPARAM wParam, LPARAM lParam)
{
	ProcessAll();
	Draw();
	return 0;
}