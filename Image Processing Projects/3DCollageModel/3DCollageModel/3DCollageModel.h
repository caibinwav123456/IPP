
// 3DCollageModel.h : main header file for the 3DCollageModel application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CMy3DCollageModelApp:
// See 3DCollageModel.cpp for the implementation of this class
//

class CMy3DCollageModelApp : public CWinAppEx
{
public:
	CMy3DCollageModelApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

public:
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMy3DCollageModelApp theApp;
