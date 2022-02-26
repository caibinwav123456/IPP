
// MeshFromObj.h : main header file for the MeshFromObj application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CMeshFromObjApp:
// See MeshFromObj.cpp for the implementation of this class
//

class CMeshFromObjApp : public CWinAppEx
{
public:
	CMeshFromObjApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

public:
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMeshFromObjApp theApp;
