
// ChildView.h : interface of the CChildView class
//


#pragma once

#include "Mesh.h"

// CChildView window
#include "fft.h"
class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	CMesh mesh;

	IplImage* m_Image;
	IplImage* m_ImgBk1;
	IplImage* m_ImgBk2;
	IplImage* m_debuf;

	RawImage  m_depImg;
	RawImage  m_texture;

	CDC m_mdc;
	CBitmap m_bitmap,*oldbmp;
	CPoint m_oldPt;

	bool m_bShowVert;
	bool m_bShowImage;
	bool m_bDown;
	bool m_bMirror;

// Operations
public:
	void DispImage(CDC* pDC, IplImage* image, CPoint ptBase);
	void Draw();

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

	void Perlin(IplImage* img, int nx, int ny);
	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

