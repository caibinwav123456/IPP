
// ChildView.h : interface of the CChildView class
//


#pragma once

#include "LicenseRecog.h"
// CChildView window

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

// Implementation
public:
	virtual ~CChildView();
	CLicenseRecog m_Recog;
	void Draw(CDC* pDC);
	void Draw();
	void DrawDetLines(CDC* pDC);
	void DispImage(CDC* pDC, IplImage* image, CPoint ptBase = CPoint(0,0));
	void FindLines(int index);
	int m_nSelCt;
	CDC m_dc;
	CBitmap m_bmp;
	bool m_bShowAllContours;
	bool m_bShowConnLines;
	int m_nSel;
	bool m_bRegressStarted;
	bool m_bAutoRun;
	Line *m_lines;
	int  m_nline;
	int m_nShowRect;
	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnFileOpen();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

