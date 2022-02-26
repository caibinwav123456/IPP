
// 3DEffects.h : main header file for the 3DEffects application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include "Gdiplus.h"
using namespace Gdiplus;
// CMy3DEffectsApp:
// See 3DEffects.cpp for the implementation of this class
//

class CMy3DEffectsApp : public CWinAppEx
{
public:
	CMy3DEffectsApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

public:
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
	ULONG_PTR m_token;
	GdiplusStartupInput m_input;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMy3DEffectsApp theApp;
