
// ChildView.h : interface of the CChildView class
//


#pragma once


// CChildView window
#include "ImageProcess3D.h"
#include "OpenDialog.h"
#include "DialogSettings.h"
#include "tinyxml.h"
#include <vector>
using namespace std;

float FrameFunc(float x, float edge);
struct Quad
{
	Vec3 m_vlist[4];
	Vec3 m_tvlist[4];
	Vec2 m_vlist2d[4];
	Vec3 m_vbuf[3];
	Vec2 m_tclist[4];
	float m_texbuf[2][3];
	int m_ibuf[6];
	int m_nSel;
	Vec3 m_clr;
};

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	IplImage* m_Image;
	IplImage* texture;
	IplImage* m_ImgBk1;
	IplImage* m_ImgBk2;
	IplImage* m_mask;
	int m_orgWidth;
	int m_orgHeight;

	bool m_bUseDouble;
	bool m_bUseMask;

	CDC m_mdc;
	CBitmap m_bitmap,*oldbmp;

	bool m_bShowVert;
	bool m_bShowImage;
	bool m_bDown;

	Quad m_quad;

	vector<Quad> m_quadextra;
	int m_nTopmost;

	CString m_strSrc;
	CString m_strBk1;
	CString m_strBk2;
	CString m_strBkTitle1;
	CString m_strBkTitle2;
	CString m_strMask;
	CString m_strMaskTitle;

	CString m_strCfg;
	CString m_strCfgBin;
	CString m_strWorkingPath;
	CString m_strTemplName;
	float m_fEdge;
	float m_fAlpha;
	BOOL m_bGray;
	BOOL m_bPackage;

	GUID m_guid;

// Operations
public:
	void DispImage(CDC* pDC, IplImage* image, CPoint ptBase);
	void Draw();
	void DrawLine(CvPoint pt1,CvPoint pt2,CvScalar color);
	void DrawQuad(Quad& quad);
	void VertexProcess(Quad& quad);
	void Reform(Vec2 vertin[4], Vec3 vertout[4]);
	void Init();
	CString FormatGuid(GUID& guid);
	void SaveLocalData();
	void AddQuad();
	void InitQuad(Quad& quad);
	void ResetQuad();
	bool HitTest(Quad& quad, POINT point);
	bool TestQuad(Quad& quad, POINT pt);
	float GetAsp(Quad& quad);
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
	afx_msg void OnFileOpen();
	afx_msg void OnMenuSettings();
	afx_msg LRESULT OnGenerateTempl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnExportTempl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnApplySettings(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnExportPakage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFileAddquad();
	afx_msg void OnFileResetquad();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};

