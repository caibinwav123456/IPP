#pragma once
#include "afxcmn.h"
#include "matutil.h"

// CDialogMat dialog
#define DECL_MSG_SPIN(M) 	afx_msg void OnDeltaposSpin##M(NMHDR *pNMHDR, LRESULT *pResult);
#define DECL_MSG_EDIT(M)    afx_msg void OnEnSetfocusEdit##M();
class CDialogMat : public CDialog
{
	DECLARE_DYNAMIC(CDialogMat)

public:
	CDialogMat(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogMat();

// Dialog Data
	enum { IDD = IDD_DIALOG_MAT };
	Mat* m_pMat;
	Vec3* m_pVec;
	CWnd* m_pWndParent;
	void Process();
	int m_nIndex1;
	int m_nIndex2;
	void* m_pVal;
	void HandleSlider();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	float m_fRR;
	float m_fRG;
	float m_fRB;
	int m_nRC;
	float m_fGR;
	float m_fGG;
	float m_fGB;
	int m_nGC;
	float m_fBR;
	float m_fBG;
	float m_fBB;
	int m_nBC;
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	DECL_MSG_SPIN(RR)
	DECL_MSG_SPIN(RG)
	DECL_MSG_SPIN(RB)
	DECL_MSG_SPIN(GR)
	DECL_MSG_SPIN(GG)
	DECL_MSG_SPIN(GB)
	DECL_MSG_SPIN(BR)
	DECL_MSG_SPIN(BG)
	DECL_MSG_SPIN(BB)
	DECL_MSG_SPIN(RC)
	DECL_MSG_SPIN(GC)
	DECL_MSG_SPIN(BC)
	DECL_MSG_EDIT(RR)
	DECL_MSG_EDIT(RG)
	DECL_MSG_EDIT(RB)
	DECL_MSG_EDIT(GR)
	DECL_MSG_EDIT(GG)
	DECL_MSG_EDIT(GB)
	DECL_MSG_EDIT(BR)
	DECL_MSG_EDIT(BG)
	DECL_MSG_EDIT(BB)
	DECL_MSG_EDIT(RC)
	DECL_MSG_EDIT(GC)
	DECL_MSG_EDIT(BC)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	int m_nVal;
	CSliderCtrl m_Slider;
};
