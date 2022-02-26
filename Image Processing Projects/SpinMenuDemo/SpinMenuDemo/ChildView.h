
// ChildView.h : interface of the CChildView class
//


#pragma once


// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	CBitmap m_bmObj[10];
	CBitmap m_bmBuf,*oldbmp;
	CDC     m_mdc;
	CDC     m_objdc;
	float   m_Angle;
	float   m_Speed;
	int     m_iItem;
// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();
	void Draw(CDC* pDC);
	void DrawItem(CDC* pDC, float fAngle, int index);
	float ComputeDepth(float fAngle);

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
};

