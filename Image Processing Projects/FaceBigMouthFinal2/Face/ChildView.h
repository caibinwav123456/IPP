
// ChildView.h : interface of the CChildView class
//


#pragma once


// CChildView window
#include "DetectWnd.h"
#include "FaceDetector.h"
#include "DialogSettings.h"
#include "tinyxml.h"

class CChildView : public CWnd
{
// Construction
public:
	CChildView();
// Attributes
public:
	IplImage* m_Image;
	IplImage* m_ImgBk;
	IplImage* m_imgObj;

	CDC m_mdc;
	CBitmap m_bitmap,*oldbmp;
	CPoint  m_oldPt;

	bool m_bShowVert;
	bool m_bShowImage;
	bool m_bDown;
	bool m_bMove;
	bool m_bAlign;
	bool m_bAnimate;
	bool m_bShowDetRegion;

	float m_fProp;
	float m_fEdge;
	float m_fScale;
	float m_fWidth;
	float m_fHeight;

	int  m_nPostureSel;
	CString m_strTempl;
	CString m_strTemplName;
	CString m_strWorkingPath;
	CString m_strBkTitle;
	CFaceDetector m_Detector;
	GUID    m_guid;
// Operations
public:
	void DispImage(CDC* pDC, IplImage* image, CPoint ptBase);
	void Draw();
	CString FormatGuid(GUID& guid);
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
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenBkImg();
	afx_msg void OnFileSettings();
	afx_msg LRESULT OnApplySettings(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGenerateTempl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnExportTempl(WPARAM wParam, LPARAM lParam);
};

