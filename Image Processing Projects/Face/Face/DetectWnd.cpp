// DetectWnd.cpp : implementation file
//

#include "stdafx.h"
#include "Face.h"
#include "DetectWnd.h"


// CDetectWnd

IMPLEMENT_DYNCREATE(CDetectWnd, CFrameWnd)

CDetectWnd::CDetectWnd()
{
	m_bDestroyed=false;
	m_Image = NULL;
}

CDetectWnd::~CDetectWnd()
{
}


BEGIN_MESSAGE_MAP(CDetectWnd, CFrameWnd)
	ON_WM_DESTROY()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CDetectWnd message handlers

void CDetectWnd::OnDestroy()
{
	CFrameWnd::OnDestroy();
	m_bDestroyed = true;
	// TODO: Add your message handler code here
}

void CDetectWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CFrameWnd::OnPaint() for painting messages
	if(m_Image)
		DispImage(&dc, m_Image, CPoint(0,0));
}

void CDetectWnd::DispImage(CDC* pDC, IplImage* image, CPoint ptBase)
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
