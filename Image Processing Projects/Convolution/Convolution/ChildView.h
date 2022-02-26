
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
	IplImage* m_src;
	IplImage* m_dest;
	IplImage* m_tex;
// Operations
public:
	void Draw();
	void DispImage(CDC* pDC, IplImage* image, CPoint ptBase);
	void Process();
	void Paint(IplImage* src, IplImage* dest);
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
	afx_msg void OnFileOpen2();
};

