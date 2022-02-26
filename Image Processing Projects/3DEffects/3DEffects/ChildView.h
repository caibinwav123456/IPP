
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
	IplImage* m_frame;
	IplImage* m_mask;
	IplImage* m_tri;
	IplImage* m_trimask;
	IplImage* m_tri2;
	IplImage* m_trimask2;
	IplImage* m_hex;
	IplImage* m_hexmask;
	int m_nIndex;
// Operations
public:

	void DispImage(CDC* pDC, IplImage* image, CPoint ptBase);
	void Draw();
	void Process(IplImage* src, IplImage* dest);
	void Process2(IplImage* src, IplImage* dest);
	void Process3(IplImage* src, IplImage* dest);
	void Process4(IplImage* src, IplImage* dest);
	void Process5(IplImage* src, IplImage* dest);
	void Process6(IplImage* src, IplImage* dest);
	void Process7(IplImage* src, IplImage* dest);
	void Process8(IplImage* src, IplImage* dest);
	void Process9(IplImage* src, IplImage* dest, float scalex, float scaley);
	void UnderWater(IplImage* src, IplImage* dest);
	void AboveWater(IplImage* src, IplImage* dest);
	void Process10(IplImage* src, IplImage* dest);
	void Process11(IplImage* src, IplImage* dest);
	void Moon(IplImage* src, IplImage* dest);
	void Process12(IplImage* src, IplImage* dest);
	void Process13(IplImage* src, IplImage* dest);
	void Process14(IplImage* src, IplImage* dest);
	void Process15(IplImage* src, IplImage* dest);
	void ProcessAll();
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
	afx_msg void OnFileOpen2();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

