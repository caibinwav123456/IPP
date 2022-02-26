
// ChildView.h : interface of the CChildView class
//


#pragma once


// CChildView window
#include "gdiplus.h"
using namespace Gdiplus;

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:

// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	ULONG_PTR gdiplustoken;
	GdiplusStartupInput input;
	Image* m_Image;
	int m_nFrames;
	GUID m_nDimID;
	int m_iFrame;
	void Draw(CDC* pDC);
// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

