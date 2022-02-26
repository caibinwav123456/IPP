#ifndef _FACE_DETECTOR_H_
#define _FACE_DETECTOR_H_

#include "facedetect.h"
#include "ImageProcess2D.h"
#include "Mesh.h"
#include "3dutil.h"

#define sqr(x) ((x)*(x))

#define DIM_FACE_IMG   400
#define FACERECT_SCALE  1

#define MOUTH_CROSS 
#define CLASS_THRESH 1.2

#define EYE_GRID_X  0//70
#define EYE_GRID_Y  80
#define EYE_GRID_NX  1//5
#define EYE_GRID_NY  5
#define MOUTH_GRID_X 80
#define MOUTH_GRID_Y 50
#define MOUTH_GRID_NX 3
#define MOUTH_GRID_NY 3
#ifdef  MOUTH_CROSS
#define MOUTH_GRID_N  5
#else
#define MOUTH_GRID_N  9
#endif
#define MIN_EYE_WIDTH  100
#define MIN_EYE_HEIGHT 50
#define DEFAULT_EYE_WIDTH 50
#define DEFAULT_EYE_HEIGHT 60
#define DEFAULT_MOUTH_WIDTH 100
#define DEFAULT_MOUTH_HEIGHT 50
#define X_INIT 200
#define Y_INIT 200
#define W_INIT 100//, 20, 0.004,0.0;
#define H_INIT 20
#define SLP_INIT 0.0
#define CRV_INIT 0//0.004

#define X_INIT_LEYE  125
#define X_INIT_REYE  275
#define Y_INIT_EYE   153//157

#define X_INIT_MOUTH 200
#define Y_INIT_MOUTH 328//338

#define X_ALIGN_LEYE 100
#define X_ALIGN_REYE 300
#define Y_ALIGN_EYE  150

#define X_ALIGN_MOUTH 200
#define Y_ALIGN_MOUTH 328

#define W_INIT_MOUTH 122
#define H_INIT_MOUTH 33

#define W_MOUTH_MAX  140
#define W_MOUTH_MIN  100
#define H_MOUTH_MAX  38
#define H_MOUTH_MIN  25
#define SLP_MOUTH_MIN -0.05
#define SLP_MOUTH_MAX 0.05

#define POS_SHIFT 1
#define SZ_SHIFT  1
#define CRV_SHIFT 0.0002
#define SLP_SHIFT 0.01
#define MAX_CRV   0.002

#define MAX_EYE_OFFSET 5
#define MAX_MOUTH_TRPZ 20

#define FC_INNER_EYE  1
#define FC_OUTER_EYE  2
#define FC_MOUTH_EDGE 4

#define RANGE_INNER_EYE  0.07
#define RANGE_OUTER_EYE  0.07
#define RANGE_MOUTH_EDGE 0.07

#define ELA_SCALE   0.1
#define FIT_SCALE   80
#define ELA_SCALE2  0.03

#define MIN_ITER_CRIT   10

#define EDGE_PADDING   0.5

#define DEFAULT_FACE_DOME_ANGLE  PI/6

#define MORPH_EXTENT     0.3
#define ANIMATION_SPEED  0.03

#define DEST_TYPE_CHGFACE    1
#define DEST_TYPE_EXPRESSION 2

typedef enum{DefaultInit, LeftEye, RightEye, Mouth, Grid}InitType;
typedef enum{Det_LeftEye, Det_RightEye, Det_Mouth, Det_Face}DetectType;

struct FaceData
{
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
	float x,y;
	float w,h;
	float c,s;
	float wh,wl;
	float hh,hl;
	float ch,cl;
	float sh,sl;

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

class CFaceDetector
{
public:
	CFaceDetector();
	~CFaceDetector();

	int Init(char* strDetectFile = NULL);
	void LoadTemplate(char* strimgTempl, char* strCfgFile);
	void SaveTemplate(char* strCfgFile);
	void LoadSourceImage(IplImage* image);
	void ClearBuf();
	IplImage* DrawFace(bool bMesh = false);
	IplImage* AnimateFace();
	IplImage* ShowFace();
	int GetFrameCount();
	IplImage* GetFrame(int index);
	IplImage* GetFrame(float fMorph);
	void SetPosture(int iPosture, int posture);
	int GetPosture(int iPosture);
	Mat& GetRotationMatrix(){return rot;}
	Vec3& GetPos(){return Pos;}
	Rect2D& GetViewPort(){return ViewPort;}
	float GetFaceWidth(){return m_fFacew;}
	float GetFaceHeight(){return m_fFaceh;}
	float GetFaceOcc(){return m_fFaceoc;}
	CvSize GetDestSize(int type);
	void SetMeshData(float w,float h,float occ);
	void DetectFace();
	void FitFace();
	void EnableFaceAlign(bool bEnable = true);
	void SetDetRectScale(float scale);
	void SetBlendEdge(float fEdge);
	FaceRegion GetDetRegion(DetectType type);
	void SetDetRegion(DetectType type, FaceRegion rgn);
	float GetBlendRadius(DetectType type);
	void SetBlendRadius(DetectType type, float radius);
	void EnableEnhColorBlending(bool bEnable);

private:
	void Process();
	void RestoreEyeData();
	bool Fit(FaceData* fitdata);
	FaceRegion GetDetectedRegion(DetectType type);
	void AdjustDetRegion();
	void Classify(FaceData* val, bool* cls, int n);
	Vec2 ExpressionFunc(float x, float y,float ext, int corner);
	float FitEyeLid(IplImage* image, CvPoint2D32f center, float w, float h, float curvation, float slope);

	static float FrameFunc(float x, float edge, float left = 0, float right = 1);
	static float HoleFunc(float x, float y, FaceRegion region, float radius = 1.25);
	static void PixelShader(Point2D pos, Vec3* color, Vec2* tex, float* userdata, Vec3* colorout, float* depth);

private:
	IplImage* m_Image;
	IplImage* m_ImgOrg;
	IplImage* m_ImgBk;
	IplImage* m_imgObj;
	IplImage* m_face;
	IplImage* m_fgray;
	IplImage* m_fhsv;
	IplImage* m_deBuf;
	IplImage* m_ImgAni;

	CvRect    m_rcFace;

	RawImage  m_texture;
	RawImage  m_zBuf;
	CMesh     m_mesh;
	Vec3      Pos;
	Mat       rot;
	Rect2D    ViewPort;

	bool m_bDown;
	bool m_bMove;
	bool m_bDet;
	bool m_bAnimation;
	bool m_bAlign;
	bool m_bFitMove;
	bool m_bColorBlend;

	int  m_nPosture[3];

	float m_fFacew;
	float m_fFaceh;
	float m_fFaceoc;
	float m_fDetScale;
	float m_fBlendEdge;
	float m_fRBlendLE;
	float m_fRBlendRE;
	float m_fRBlendM;

	FaceRegion m_rgnlEye,m_rgnrEye,m_rgnMouth;
	FaceRegion m_rgnlEadj,m_rgnrEadj,m_rgnMadj;

	Vec2 m_vMoveLEyel;
	Vec2 m_vMoveLEyer;
	Vec2 m_vMoveREyel;
	Vec2 m_vMoveREyer;
	Vec2 m_vMoveMouthl;
	Vec2 m_vMoveMouthr;

	Vec2 m_vOrgLEyel;
	Vec2 m_vOrgREyer;
	Vec2 m_vOrgLEyer;
	Vec2 m_vOrgREyel;
	Vec2 m_vOrgMouthl;
	Vec2 m_vOrgMouthr;

	Vec2 m_vFitSt;

	float m_fMorph;
	float m_fSpeed;

	FaceData lEGrid[EYE_GRID_NX*EYE_GRID_NY];
	FaceData rEGrid[EYE_GRID_NX*EYE_GRID_NY];
	FaceData MGrid[MOUTH_GRID_N];
	FaceData MGridS[MOUTH_GRID_N];

	float m_matAlign[2][2];
	float m_shiftAlign[2];

	static CFaceDetector* s_pObj; 
};

#endif