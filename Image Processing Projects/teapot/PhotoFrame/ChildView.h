
// ChildView.h : interface of the CChildView class
//


#pragma once


// CChildView window
#include "ImageProcess3D.h"

struct vertex
{
	float x,y,z;
	float nx,ny,nz;
};
class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	IplImage* m_Image;
	IplImage* m_ImgBk1;
	IplImage* m_ImgBk2;

	CDC m_mdc;
	CBitmap m_bitmap,*oldbmp;
	CPoint m_oldPt;

	bool m_bShowVert;
	bool m_bShowImage;
	bool m_bDown;

	Vec3 m_vbuf[3];
	float m_vdata[4][3];

	vertex *m_vlist;
	Vec3   *m_tvlist;
	Vec3   *tnormal;
	Vec2   *m_texbuf;
	float  *light;
	WORD   *m_ibuf;
	DWORD nVert;
	DWORD nFace;
// Operations
public:
	void DispImage(CDC* pDC, IplImage* image, CPoint ptBase);
	void Draw();
	void DrawLine(CvPoint pt1,CvPoint pt2,CvScalar color);
	void VertexProcess();
	float FrameFunc(float x, float edge);
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
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

