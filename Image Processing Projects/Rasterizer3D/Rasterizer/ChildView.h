
// ChildView.h : interface of the CChildView class
//


#pragma once


// CChildView window
#define NSEG  1//9
#define NVERT ((NSEG+1)*(NSEG+1))
#define NFACE (2*NSEG*NSEG)
#include "ImageProcess3D.h"
#include "ImageProcess2D.h"
class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	IplImage* m_Image;
	IplImage* texture;
	CvPoint3D32f m_vlist[NVERT];
	CvPoint3D32f m_tvlist[NVERT];
	Vec3 m_vbuf[3];
	float    texcoord[2][3];
	CvPoint2D32f m_texbuf[NVERT];
	int m_ibuf[3*NFACE];
	bool m_bDown;
	int m_nSel;
	CPoint m_oldPt;
	CDC m_mdc;
	CBitmap m_bitmap,*oldbmp;
	bool m_bShowVert;
	bool m_bShow;
// Operations
public:
	void DispImage(CDC* pDC, IplImage* image, CPoint ptBase);
	void Draw();
	void VertexProcess();
	void DrawLine(CvPoint pt1,CvPoint pt2,CvScalar color);
// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();
	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

