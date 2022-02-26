
// ChildView.h : interface of the CChildView class
//


#pragma once

#include "Mesh.h"
// CChildView window
struct Rect2D32f
{
	float x;
	float y;
	float cx;
	float cy;
	Rect2D32f(float _x = 0, float _y = 0, float _cx = 0, float _cy = 0)
	{
		x = _x;
		y = _y;
		cx = _cx;
		cy = _cy;
	}
};

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	CMesh mesh;

	IplImage* m_Image;
	IplImage* m_ImgBk1;
	IplImage* m_ImgBk2;
	IplImage* m_debuf;
	IplImage* m_ImgTex;

	RawImage  m_depImg;
	RawImage  m_texture;

	CPoint m_oldPt;

	bool m_bShowVert;
	bool m_bShowImage;
	bool m_bDown;
	bool m_bSpcDown;
	bool m_bFDown;
	bool m_bGDown;
	float m_fProj;
	
	Mat m_rot;
	Vec3 m_pos;
	Rect2D32f m_Vport;
	CString m_strMeshName;
	bool m_bSpec;
	Vec3 m_specl;
// Operations
public:
	void DispImage(CDC* pDC, IplImage* image, CPoint ptBase);
	void Draw();
	static void VertexShader(Vec3* posin, Vec3* colorin, Vec3* normalin, Vec2* texin, float* vuserdatain, Vec3* posout, Vec3* colorout, Vec2* texout, float* vuserdataout, void* usrptr);
	static void PixelShader(Point2D pos, Vec3* color, Vec2* tex, float* userdata, Vec3* colorout, float* depth, int iface, void* usrptr);
	void SaveConfig();
	void LoadConfig();
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
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnFileOpenmesh();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnFileOpenimage();
};

