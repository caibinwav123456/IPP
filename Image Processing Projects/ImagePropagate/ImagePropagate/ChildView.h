
// ChildView.h : interface of the CChildView class
//


#pragma once


// CChildView window
#include "ImageProcess3D.h"
#include "DialogMat.h"

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	IplImage* m_src;
	IplImage* m_dest;
// Operations
public:
	void Process(IplImage* src, IplImage* dest);
	void Process02(IplImage* src, IplImage* dest);
	void Process2(IplImage* dest);
	void Process3(IplImage* src, IplImage* dest);
	void Process4(IplImage* src, IplImage* dest);
	void Process5(IplImage* src, IplImage* dest);
	void Process6(IplImage* src, IplImage* dest);
	void Process7(IplImage* src, IplImage* dest);
	void Process8(IplImage* src, IplImage* dest);
	void Process9(IplImage* src, IplImage* dest);
	void Process10(IplImage* src, IplImage* dest);
	void Process11(IplImage* src, IplImage* dest);
	void Process12(IplImage* src, IplImage* dest);
	void Process13(IplImage* src, IplImage* dest);
	void ProcessAll();
	void ColorM(IplImage* src, IplImage* dest);
	void DispImage(CDC* pDC, IplImage* image, CPoint ptBase);
	void Draw();
// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();
	float m_xy;
	float m_yx;
	int m_seed;
	Mat m_mat;
	Vec3 m_vec;
	CDialogMat m_dlgMat;
	float m_fRadius;
	float m_fBrightness;
	Vec3 m_vClr;
	Vec3 m_vBkClr;
	float m_fBlackness;
	float m_fIntense;
	int m_ProcessIndex;
	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileOpen2();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnProcessColor(WPARAM wParam, LPARAM lParam);
};

