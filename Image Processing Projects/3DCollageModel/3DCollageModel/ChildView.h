
// ChildView.h : interface of the CChildView class
//


#pragma once

#include "Mesh.h"

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
	void GenCube(int nSeg, char* name);
	void GenSpere();
	void GenOctahedron();
	void GenIcosahedron();
	void GenDodecahedron();
	void GenHexahedral();
	void GenFootball();
	void Tesselation(CMesh& mesh);
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
};

