
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

// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

	void CvtA2M();
	void CvtM2A();
	void CvtX2M();
	void CvtM2X();
	inline float Delta();

	void GenerateEllipse();
	void DrawEllipse();
	void DrawBoundingBox();
	// Generated message map functions
protected:

	float a, b, talpha;
	float m, n, p;
	float X, Y, S;
	float x[100],y[100];

	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

