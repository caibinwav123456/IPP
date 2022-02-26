
// ChildView.h : interface of the CChildView class
//


#pragma once


// CChildView window
#include "complex"
using namespace std;
#include "matutil.h"
class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	float m_extent;
	Mat m_rot;
	bool m_bDown;
	CPoint m_pt;
// Operations
public:
	void Draw();
	void Draw(CDC* pDC);
	void Draw2(CDC* pDC);
	void DrawWave(CDC* pDC);
	void DrawWavyCurve(CDC* pDC);
	Vec2 WaveFunc(float t);
	Vec2 _WaveFunc(float t);
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
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

