
// ChildView.h : interface of the CChildView class
//


#pragma once


// CChildView window
#include "facedetect.h"
#include "ImageProcess2D.h"
#include "Mesh.h"
#include "3dutil.h"
#include "DetectWnd.h"

struct FaceData
{
	float x,y;
	float w,h;
	float c,s;
	float wh,wl;
	float hh,hl;
	float ch,cl;
	float sh,sl;
	FaceData()
	{
		x=y=0;
		w=h=0;
		c=s=0;
		wh=wl=0;
		hh=hl=0;
		ch=cl=0;
		sh=sl=0;
	}
};
struct FaceRegion
{
	CvPoint2D32f topleft;
	CvPoint2D32f topright;
	CvPoint2D32f bottomright;
	CvPoint2D32f bottomleft;
	FaceRegion()
	{
		topleft = cvPoint2D32f(0,0);
		topright = cvPoint2D32f(0,0);
		bottomright = cvPoint2D32f(0,0);
		bottomleft = cvPoint2D32f(0,0);
	}
};

class CChildView : public CWnd
{
// Construction
public:
	CChildView();
	typedef enum{Default, LeftEye, RightEye, Mouth, Grid}InitType;
	typedef enum{Det_LeftEye, Det_RightEye, Det_Mouth}DetectType;
// Attributes
public:
	IplImage* m_Image;
	IplImage* m_ImgBk;
	IplImage* m_imgObj;
	IplImage* m_face;
	IplImage* m_fgray;
	IplImage** m_facepyr;
	int m_nPyr;

	CvRect    m_rcFace;

	IplImage* m_Det;
	CDetectWnd* m_WndDet;

	IplImage* m_deBuf;
	RawImage  m_texture;
	RawImage  m_zBuf;
	CMesh     m_mesh;

	CDC m_mdc;
	CBitmap m_bitmap,*oldbmp;
	CPoint  m_oldPt;

	bool m_bShowVert;
	bool m_bShowImage;
	bool m_bDown;
	bool m_bMove;
	bool m_bFit;
	bool m_bAuto;
	bool m_bShowGrid;
	bool m_bShowDetBox;
	bool m_bShowAdj;
	bool m_bShowClrImg;
	bool m_bDet;
	bool m_bAnimation;
	bool m_bUsePyr;
	bool m_bMFit;

	bool m_bFitMove;

	int  m_nPostureEnable[3];
	int  m_nPostureSel;

	float m_EyeX;
	float m_EyeY;
	float m_EyeW;
	float m_EyeH;
	float m_EyeSlp;
	float m_EyeCrv;

	FaceRegion m_rgnlEye,m_rgnrEye,m_rgnMouth;
	FaceRegion m_rgnlEadj,m_rgnrEadj,m_rgnMadj;

	Vec2 m_vMoveLEyel;
	Vec2 m_vMoveLEyer;
	Vec2 m_vMoveREyel;
	Vec2 m_vMoveREyer;
	Vec2 m_vMoveMouthl;
	Vec2 m_vMoveMouthr;

	CvRect m_rcML, m_rcMR;

	CvPoint2D32f m_lcorner[10];
	CvPoint2D32f m_rcorner[10];
	int m_lcount;
	int m_rcount;

	Vec2 m_vFitSt;
	float m_fFitEnrgy;
	float m_fMorph;
	float m_fEnergy;

	bool m_bShowCornerBox;

	int  m_nTime;

	int m_nDetLevel;
	int m_nIterMax;
// Operations
public:
	void DispImage(CDC* pDC, IplImage* image, CPoint ptBase);
	void Draw();
	void Process();
	void SaveFile();
	void LoadFile();
	void RestoreEyeData(InitType type);
	void BuildPyramid();
	bool Fit();
	bool Fit(FaceData* fitdata, int level = 0);
	void StartDetect();
	void EndDetect();
	FaceRegion GetDetectedRegion(DetectType type);
	void AdjustDetRegion();
	void Classify(float* val,bool* cls, int n);
	void Classify(FaceData* val, bool* cls, int n);
	void DrawDetectRegion(CDC* pDC);
	void OneTimeDetect();
	Vec2 ExpressionFunc(float x, float y,float ext, int corner);
	void FindCorner();
	void FindMouthCorner(CvRect rcCorner, CvPoint2D32f* pt, int* count);
	void DrawCornerBox(CDC* pDC);
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
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnFileOpen();
};

