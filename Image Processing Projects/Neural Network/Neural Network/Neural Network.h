
// Neural Network.h : main header file for the Neural Network application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CNeuralNetworkApp:
// See Neural Network.cpp for the implementation of this class
//

class CNeuralNetworkApp : public CWinAppEx
{
public:
	CNeuralNetworkApp();


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

extern CNeuralNetworkApp theApp;
