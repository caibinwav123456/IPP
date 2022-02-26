#ifndef _MESH_H_
#define _MESH_H_

#include "ImageProcess3D.h"

#define VF_XYZ      1
#define VF_COLOR    2
#define VF_NORMAL   4
#define VF_TEXCOORD 8
#define VF_USER(n)  (16|(n<<16))
#define VF_USERF    16

#define VFT_XYZ      1
#define VFT_COLOR    2
#define VFT_TEXCOORD 4
#define VFT_USER(n)  (8|(n<<16))
#define VFT_USERF    8

#define IF_INDEX16   1
#define IF_INDEX32   2

typedef void (*VERTEXFUNC)(float* vdatain, int sizein, float* vdataout, int sizeout);
typedef void (*VShader)(Vec3* posin, Vec3* colorin, Vec3* normalin, Vec2* texin, float* vuserdatain, Vec3* posout, Vec3* colorout, Vec2* texout, float* vuserdataout);
typedef void (*PShader)(Point2D pos, Vec3* color, Vec2* tex, float* userdata, Vec3* colorout, float* depth);

class CMesh
{
	friend void DefaultPixelFunc(RawImage* image, Point2D pos, int count, float* vdata);
	friend void DefaultVertexFunc(float* vdatain, int sizein, float* vdataout, int sizeout);
public:
	CMesh();
	~CMesh();

	void CreateMesh(int nVert, int nFace, int vertexFormat);
	void Release();
	bool LoadMesh(char* filename);
	bool SaveMesh(char* filename, int indexFormat = IF_INDEX16);

	void Render(RawImage image,RawImage* depthbuf);
	void DrawWireFrame(RawImage image, int iface = -1);
	void SetViewPort(Rect2D* vp);
	void SetMatrix(Mat rot, Vec3 trans);
	void SetProjAngle(float fAngle);
	void NormalizeNormals(bool bNorm);
	void SetTexture(RawImage* texture);
	void SetPixelFunc(PIXELFUNC pFunc);
	void SetVertexFunc(VERTEXFUNC pFunc);
	void SetPixelShader(PShader pShaderFunc);
	void SetVertexShader(VShader pShaderFunc);
	void SetVSOutFormat(int tvFormat);
	void SetDefaultVSOut();
	void SetDefaultFunc();
	void SetCullMode(int nCullMode);
	bool Pick(RawImage image, Point2D pt, int* iFace, Vec3* color, Vec2* tex, float* pUser, int nUser);
public:
	int m_nFace;
	int m_nVert;
	int m_nVertexFormat;
	int m_ntVFormat;
	float* m_vbuf;
	DWORD* m_ibuf;
	Rect2D* m_pVp;
	Mat  m_rot;
	Vec3 m_trans;
	bool m_bNormalize;
	
private:
	VERTEXFUNC m_pVFunc;
	PIXELFUNC  m_pPFunc;
	VShader    m_pVShader;
	PShader    m_pPShader;
	RawImage*  m_pDepBuf;
	RawImage*  m_pTex;
	int        m_nCullMode;
	int m_idepData;
	int m_iclrData;
	int m_itexData;
	int m_iuserData;
	float* m_tvbuf;
	float  m_fAngle;

	int ComputetVFormat(int vertexFormat);
	int ComputeVertLength();
	int ComputePixLength();
	int VertAttrOffset(int nAttr);
	int PixAttrOffset(int nAttr);
	void VertexProcess();
	void DrawLine(RawImage image, Vec2 pt1, Vec2 pt2, Vec3 color);
};

#endif