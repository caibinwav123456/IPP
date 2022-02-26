// DialogMat.cpp : implementation file
//

#include "stdafx.h"
#include "ImagePropagate.h"
#include "DialogMat.h"
#include "afxdialogex.h"

int range_index[3][4] = 
{
	{0,0,0,1},
	{0,0,0,1},
	{0,0,0,1}
};

struct range_param
{
	int low;
	int high;
	int step;
	float flow;
	float fhigh;
};

range_param g_param[2] =
{
	{-500, 500, 1, -5.0f, 5.0f},
	{-255, 255, 1, -255.0f, 255.0f}
};
// CDialogMat dialog

IMPLEMENT_DYNAMIC(CDialogMat, CDialog)

CDialogMat::CDialogMat(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogMat::IDD, pParent)
	, m_fRR(1)
	, m_fRG(0)
	, m_fRB(0)
	, m_nRC(0)
	, m_fGR(0)
	, m_fGG(1)
	, m_fGB(0)
	, m_nGC(0)
	, m_fBR(0)
	, m_fBG(0)
	, m_fBB(1)
	, m_nBC(0)
	, m_pMat(NULL)
	, m_pVec(NULL)
	, m_pWndParent(pParent)
	, m_nVal(0)
	, m_nIndex1(0)
	, m_nIndex2(0)
{
	m_pVal = &m_fRR;
}

CDialogMat::~CDialogMat()
{
}

void CDialogMat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_RR, m_fRR);
	DDX_Text(pDX, IDC_EDIT_RG, m_fRG);
	DDX_Text(pDX, IDC_EDIT_RB, m_fRB);
	DDX_Text(pDX, IDC_EDIT_RC, m_nRC);
	DDX_Text(pDX, IDC_EDIT_GR, m_fGR);
	DDX_Text(pDX, IDC_EDIT_GG, m_fGG);
	DDX_Text(pDX, IDC_EDIT_GB, m_fGB);
	DDX_Text(pDX, IDC_EDIT_GC, m_nGC);
	DDX_Text(pDX, IDC_EDIT_BR, m_fBR);
	DDX_Text(pDX, IDC_EDIT_BG, m_fBG);
	DDX_Text(pDX, IDC_EDIT_BB, m_fBB);
	DDX_Text(pDX, IDC_EDIT_BC, m_nBC);
	DDX_Slider(pDX, IDC_SLIDER_VAL, m_nVal);
	DDX_Control(pDX, IDC_SLIDER_VAL, m_Slider);
}

#define ON_MSG_SPIN(M) 	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_##M, &CDialogMat::OnDeltaposSpin##M)
#define ON_MSG_EDIT(M)  ON_EN_SETFOCUS(IDC_EDIT_##M, &CDialogMat::OnEnSetfocusEdit##M)

BEGIN_MESSAGE_MAP(CDialogMat, CDialog)
	ON_MSG_SPIN(RR)
	ON_MSG_SPIN(RG)
	ON_MSG_SPIN(RB)
	ON_MSG_SPIN(GR)
	ON_MSG_SPIN(GG)
	ON_MSG_SPIN(GB)
	ON_MSG_SPIN(BR)
	ON_MSG_SPIN(BG)
	ON_MSG_SPIN(BB)
	ON_MSG_SPIN(RC)
	ON_MSG_SPIN(GC)
	ON_MSG_SPIN(BC)
	ON_MSG_EDIT(RR)
	ON_MSG_EDIT(RG)
	ON_MSG_EDIT(RB)
	ON_MSG_EDIT(GR)
	ON_MSG_EDIT(GG)
	ON_MSG_EDIT(GB)
	ON_MSG_EDIT(BR)
	ON_MSG_EDIT(BG)
	ON_MSG_EDIT(BB)
	ON_MSG_EDIT(RC)
	ON_MSG_EDIT(GC)
	ON_MSG_EDIT(BC)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CDialogMat message handlers


BOOL CDialogMat::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_RR))->SetBuddy(GetDlgItem(IDC_EDIT_RR));
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_RG))->SetBuddy(GetDlgItem(IDC_EDIT_RG));
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_RB))->SetBuddy(GetDlgItem(IDC_EDIT_RB));
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_GR))->SetBuddy(GetDlgItem(IDC_EDIT_GR));
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_GG))->SetBuddy(GetDlgItem(IDC_EDIT_GG));
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_GB))->SetBuddy(GetDlgItem(IDC_EDIT_GB));
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_BR))->SetBuddy(GetDlgItem(IDC_EDIT_BR));
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_BG))->SetBuddy(GetDlgItem(IDC_EDIT_BG));
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_BB))->SetBuddy(GetDlgItem(IDC_EDIT_BB));
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_RC))->SetBuddy(GetDlgItem(IDC_EDIT_RC));
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_GC))->SetBuddy(GetDlgItem(IDC_EDIT_GC));
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN_BC))->SetBuddy(GetDlgItem(IDC_EDIT_BC));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDialogMat::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialog::OnOK();
	HandleSlider();
	Process();
}

void CDialogMat::HandleSlider()
{
	UpdateData(TRUE);
	int index = range_index[m_nIndex1][m_nIndex2];
	range_param param = g_param[index];
	int pos;
	if(index == 0)
		pos = (*(float*)m_pVal - param.flow)/(param.fhigh-param.flow)*(param.high-param.low)+param.low;
	else
		pos = *(int*)m_pVal;
	m_nVal = pos;
	UpdateData(FALSE);
}

void CDialogMat::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	ShowWindow(SW_HIDE);
	//CDialog::OnCancel();
}

#define IMPL_MSG_SPIN(M,N) \
void CDialogMat::OnDeltaposSpin##M(NMHDR *pNMHDR, LRESULT *pResult) \
{ \
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR); \
	UpdateData(TRUE); \
	if(pNMUpDown->iDelta > 0) \
	{ \
		N-=0.01; \
	} \
	else \
	{ \
		N+=0.01; \
	} \
	if(fabs(N)<0.000001) \
		N=0; \
	UpdateData(FALSE); \
	HandleSlider(); \
	Process(); \
	*pResult = 0; \
}

#define IMPL_MSG_SPIN2(M,N) \
void CDialogMat::OnDeltaposSpin##M(NMHDR *pNMHDR, LRESULT *pResult) \
{ \
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR); \
	UpdateData(TRUE); \
	if(pNMUpDown->iDelta > 0) \
	{ \
		N-=1; \
	} \
	else \
	{ \
		N+=1; \
	} \
	UpdateData(FALSE); \
	HandleSlider(); \
	Process(); \
	*pResult = 0; \
}

IMPL_MSG_SPIN(RR, m_fRR)
IMPL_MSG_SPIN(RG, m_fRG)
IMPL_MSG_SPIN(RB, m_fRB)
IMPL_MSG_SPIN(GR, m_fGR)
IMPL_MSG_SPIN(GG, m_fGG)
IMPL_MSG_SPIN(GB, m_fGB)
IMPL_MSG_SPIN(BR, m_fBR)
IMPL_MSG_SPIN(BG, m_fBG)
IMPL_MSG_SPIN(BB, m_fBB)
IMPL_MSG_SPIN2(RC, m_nRC)
IMPL_MSG_SPIN2(GC, m_nGC)
IMPL_MSG_SPIN2(BC, m_nBC)

void CDialogMat::Process()
{
	if(m_pMat && m_pVec)
	{
		m_pMat->_00 = m_fRR;
		m_pMat->_01 = m_fRG;
		m_pMat->_02 = m_fRB;
		m_pMat->_10 = m_fGR;
		m_pMat->_11 = m_fGG;
		m_pMat->_12 = m_fGB;
		m_pMat->_20 = m_fBR;
		m_pMat->_21 = m_fBG;
		m_pMat->_22 = m_fBB;

		m_pVec->x = m_nRC;
		m_pVec->y = m_nGC;
		m_pVec->z = m_nBC;
	}
	if(m_pWndParent)
		m_pWndParent->SendMessage(WM_PROCESS_COLOR);
}

#define IMPL_MSG_EDIT(M, N, I, J) \
void CDialogMat::OnEnSetfocusEdit##M() \
{ \
	m_Slider.EnableWindow(TRUE); \
	UpdateData(TRUE); \
	m_nIndex1 = I; \
	m_nIndex2 = J; \
	m_pVal = &N; \
	range_param param = g_param[range_index[m_nIndex1][m_nIndex2]]; \
	m_Slider.SetRange(param.low, param.high); \
	int pos = (N - param.flow)/(param.fhigh-param.flow)*(param.high-param.low)+param.low; \
	m_nVal = pos; \
	UpdateData(FALSE); \
}

IMPL_MSG_EDIT(RR, m_fRR, 0, 0)
IMPL_MSG_EDIT(RG, m_fRG, 0, 1)
IMPL_MSG_EDIT(RB, m_fRB, 0, 2)
IMPL_MSG_EDIT(GR, m_fGR, 1, 0)
IMPL_MSG_EDIT(GG, m_fGG, 1, 1)
IMPL_MSG_EDIT(GB, m_fGB, 1, 2)
IMPL_MSG_EDIT(BR, m_fBR, 2, 0)
IMPL_MSG_EDIT(BG, m_fBG, 2, 1)
IMPL_MSG_EDIT(BB, m_fBB, 2, 2)
IMPL_MSG_EDIT(RC, m_nRC, 0, 3)
IMPL_MSG_EDIT(GC, m_nGC, 1, 3)
IMPL_MSG_EDIT(BC, m_nBC, 2, 3)

void CDialogMat::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	UpdateData(TRUE);
	int index = range_index[m_nIndex1][m_nIndex2];
	range_param param = g_param[index];
	if(index == 0)
	{
		float fVal = param.flow+(m_nVal-param.low)*(param.fhigh-param.flow)/(param.high-param.low);
		*(float*)m_pVal = fVal;
	}
	else
	{
		*(int*)m_pVal = m_nVal;
	}
	UpdateData(FALSE);
	Process();
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
