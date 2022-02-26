
// ChildView.h : interface of the CChildView class
//


#pragma once


// CChildView window
#include "matutil.h"
#include <vector>
using namespace std;
class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	int m_size;
	int m_seg;
	float m_overlap;
	IplImage* m_Image;
	IplImage* m_ImageConv;
	IplImage* m_ImageOrg;
	float m_scale;
	int m_scanl;
	int m_scanlx;
	bool m_bShow;
	bool m_bShowAdjust;
	Vec4 m_Compensation;
	Vec4 m_CompensationT;
	double m_CompExt[6];
	double m_CompExtT[6];
	Vec2 m_adj;
	int m_adjmode;
	float m_finhibit;
	bool m_bSupressRipple;
	bool m_bFillHoles;
	bool m_bShowRipple;
	bool m_bDbgMode;
	bool m_dbg;
	CString m_dbgPath;
	FILE* m_file;
	float* m_xIntense;
	float* m_yIntense;
	float* m_xDev;
	float* m_yDev;
	int m_nx;
	int m_ny;
	CPoint m_Pos;
	int m_topcut;
	int m_botcut;
	int m_leftcut;
	int m_rightcut;
	IplImage* m_cutimage;
	bool m_bShowCut;
	int m_ncore;
	IplImage* m_rTable;
	IplImage* m_CosineTable;
	IplImage* m_Grad;
	IplImage* m_RotImg;
	IplImage* m_RotConv;
	bool m_bShowGrad;
	bool m_bShowRotate;
	Vec2 m_vDir;
	float m_certainty;
	float* m_GradIntense;
	float* m_GradIntenseAdj;
	int m_nGradIntense;
	bool m_bShowGradIntense;
	float m_kBaseAmple;
	float m_kRippleAmple;
	float m_kBasePower;
	float m_kT;
	float m_kTheta;
	float m_kNoise;
	float m_kCenter;
	float m_kCenterAsc;
	float m_kAmpleAsc;
	vector<int> m_highs,m_lows;
	bool m_bAdjustMotion;
	int m_adjmotionmode;
	bool m_bSinglePass;
// Operations
public:
	void Draw();
	void Draw(CDC* pDC);
	void DrawSub(CDC* pDC);
	void DispImage(CDC* pDC, IplImage* image, CPoint ptBase);
	void Convert(IplImage*& image, CString strBox, CString strOut);
	void Process(IplImage* image);
	IplImage* ComputeAmp(IplImage* image,IplImage** ripple=NULL);
	Vec4 ComputeNormal(IplImage* image);
	void AdjustImage(IplImage* src, IplImage* dest, Vec4 comp);
	void AdjustMotion(IplImage* src, IplImage* dest);
	void RotateImage(IplImage* src, IplImage* dest, Vec2 rot, Vec2 shift);
	void RotateImage(IplImage* image, Vec2 dir, int d);
	void ProcessTotal(IplImage* src,IplImage* dest);
	void SupressRipple(IplImage* src, IplImage* dest);
	void SupressHigh(IplImage* src, IplImage* dest);
	void Denoise(IplImage* src, IplImage* dest);
	void ComputeAxisIntensity(IplImage* src);
	void ComputeDeviation();
	bool ComputeDev(float* intense,float* dev, int n, int dir, int* cut);
	void ComputeGrad(IplImage* src, IplImage* dest);
	void ComputeMotionDirection(IplImage* src, Vec2* dir, float* certainty);
	void ComputeGradIntense(IplImage* src, Vec2 dir, float** intense, int* n);
	void NormalizeGrad(IplImage* grad);
	void EstimateKernel(float* v, int n, float* a, float* k, float* ample, float* t, float* theta, float* noise, float* center, float* casc, float* aasc);
	bool Estimate1(float* v, int n, float* a,float* b, float* k);
	void Estimate2(float* v, int n, float* t, float* theta, float* a, float* center, float* casc, float* aasc);
	struct StepParam
	{
		int cellx;
		int celly;
		int blkx;
		int blky;
		int segx;
		int segy;
		int offx;
		int offy;
		IplImage*blk;
		IplImage*prefft;
		IplImage*fft;
		IplImage*fft2;
		IplImage*dest;
		Vec4* params;
		double (*paramext)[6];
	};
	typedef void (CChildView::*ProcessStep)(IplImage* image, int i, int j, StepParam* ptr, CvRect rc);
	void step1(IplImage* image, int i, int j, StepParam* ptr, CvRect rc);
	void step2(IplImage* image, int i, int j, StepParam* ptr, CvRect rc);
	void PerformStep(ProcessStep step, IplImage* image, StepParam* ptr);
	void DebugAdjust(IplImage* src);
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
	afx_msg void OnFileOpen2();
	afx_msg void OnFileSetsize();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnFileOpentex();
	afx_msg void OnFileProcesstotal();
	afx_msg void OnFileThreshold();
	afx_msg void OnFileDenoise();
	afx_msg void OnFileThoroughprocess();
	afx_msg void OnFileDll();
	afx_msg void OnFileShowaxisdistribution();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnFileDllcutedge();
	afx_msg void OnCore1();
	afx_msg void OnCore2();
	afx_msg void OnCore4();
	afx_msg void OnUpdateCore1(CCmdUI *pCmdUI);
	afx_msg void OnUpdateCore2(CCmdUI *pCmdUI);
	afx_msg void OnUpdateCore4(CCmdUI *pCmdUI);
	afx_msg void OnCoreSinglepass();
	afx_msg void OnUpdateCoreSinglepass(CCmdUI *pCmdUI);
	afx_msg void OnFileResizeimage();
};

