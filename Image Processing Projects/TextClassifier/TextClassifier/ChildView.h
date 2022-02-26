
// ChildView.h : interface of the CChildView class
//


#pragma once
#include "TextClassify.h"
// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	TextClassify m_Classify;
	bool m_bExcludeTags;
// Operations
public:
	void TestOCR(char* basepath);
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
	afx_msg void OnFileClassify();
	afx_msg void OnFileLoad();
	afx_msg void OnFileTestocr();
	afx_msg void OnOptionsExcludetags();
	afx_msg void OnUpdateOptionsExcludetags(CCmdUI *pCmdUI);
};

