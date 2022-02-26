
// ChildView.h : interface of the CChildView class
//


#pragma once


// CChildView window
#include "ft2build.h"
#include FT_FREETYPE_H
#include <vector>
using namespace std;
struct RowStats
{
	int baseliney;
};
class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	FT_Library m_library;
	FT_Face m_face;
	vector<IplImage*> m_Image;
	int m_fontsize;
	CPoint m_org;
	vector<vector<CRect>> m_vBox;
	vector<vector<WCHAR>> m_vChar;
	vector<vector<RowStats>> m_RowStats;
	WCHAR* m_buf;
	WCHAR* m_ptr;
	CvSize m_szImg;
	bool m_bOrigin;
	bool m_bShowRowStats;
	bool m_bSaveRowStats;
	int m_LineSpace;
	int m_CharSpace;
	int m_Index;
// Operations
public:
	void Draw();
	void Draw(CDC* pDC);
	bool Render(int n);
	bool DrawBitmap(FT_Bitmap* bmp, int left, int top);
	void DispImage(CDC* pDC, IplImage* image, CPoint ptBase);
	void PageShift(int dir);
	void Reset();
	bool SaveOutput(CString strImgFile, CString strBoxFile);
	bool LoadFace(CString strPath, bool bFullRender=false);
	bool Save(CString strPath);
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
	afx_msg void OnFileLoadface();
	afx_msg void OnFileLoadtext();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnFileSave2();
	afx_msg void OnFileSetparams();
	afx_msg void OnFileBatchprocess();
};

