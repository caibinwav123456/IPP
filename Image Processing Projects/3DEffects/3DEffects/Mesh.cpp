#include "stdafx.h"
#include "Mesh.h"

#define NUM_VERTEX_ATTR 5
#define NUM_PIXEL_ATTR  4

int g_VFMap[5]={3,3,3,2,0};
int g_VFMask[5]={1,2,4,8,16};

int g_PFMap[4]={3,3,2,0};
int g_PFMask[4]={1,6,8,16};

int g_tVMask[4]={1,2,4,8};

void DefaultVertexFunc(float* vdatain, int sizein, float* vdataout, int sizeout, void* pThis)
{
	CMesh* pMeshInRender = (CMesh*)pThis;
	if(pMeshInRender == NULL)
		return;
	Vec3* pPos = (Vec3*)(vdatain+pMeshInRender->VertAttrOffset(VF_XYZ));
	Vec3* ptPos= (Vec3*)(vdataout+pMeshInRender->PixAttrOffset(VFT_XYZ));
	Vec3* pClr    = NULL;
	Vec3* pNorm   = NULL;
	Vec3* ptClr   = NULL;
	Vec2* pTex    = NULL;
	Vec2* ptTex   = NULL;
	float* pUser  = NULL;
	float* ptUser = NULL;

	Vec3  color(1,1,1);
	float l=1;

	if(pMeshInRender->m_nVertexFormat & VF_COLOR)
	{
		pClr=(Vec3*)(vdatain+pMeshInRender->VertAttrOffset(VF_COLOR));
	}

	if(pMeshInRender->m_nVertexFormat & VF_NORMAL)
	{
		pNorm=(Vec3*)(vdatain+pMeshInRender->VertAttrOffset(VF_NORMAL));
	}

	if(pMeshInRender->m_ntVFormat & VFT_COLOR)
	{
		ptClr=(Vec3*)(vdataout+pMeshInRender->PixAttrOffset(VFT_COLOR));
	}

	if(pMeshInRender->m_nVertexFormat & VF_TEXCOORD)
	{
		pTex=(Vec2*)(vdatain+pMeshInRender->VertAttrOffset(VF_TEXCOORD));
	}

	if(pMeshInRender->m_ntVFormat & VFT_TEXCOORD)
	{
		ptTex=(Vec2*)(vdataout+pMeshInRender->PixAttrOffset(VFT_TEXCOORD));
	}

	if(pMeshInRender->m_nVertexFormat & VF_USERF)
	{
		pUser = vdatain+pMeshInRender->VertAttrOffset(VF_USERF);
	}

	if(pMeshInRender->m_ntVFormat & VFT_USERF)
	{
		ptUser = vdataout+pMeshInRender->PixAttrOffset(VFT_USERF);
	}

	if(pMeshInRender->m_pVShader != NULL)
	{
		pMeshInRender->m_pVShader(pPos, pClr, pNorm, pTex, pUser, ptPos, ptClr, ptTex, ptUser, pMeshInRender->m_pUserPtr);
	}
	else
	{
		Vec3 tPos=*pPos*pMeshInRender->m_rot;
		tPos+=pMeshInRender->m_trans;
		*ptPos=Vec3(tPos.x/tPos.z/pMeshInRender->m_fAngle,tPos.y/tPos.z/pMeshInRender->m_fAngle,1/tPos.z);

		if(pClr)color=*pClr;
		if(pNorm)
		{
			Vec3  tNorm=*pNorm*pMeshInRender->m_rot;
			if(pMeshInRender->m_bNormalize)
				tNorm=normalize(tNorm);
			l=abs(-dot(tNorm,tPos)/length(tPos));
		}
		if(ptClr)*ptClr = color*l;
		if(pTex && ptTex)*ptTex = *pTex;
		else if(ptTex)*ptTex = Vec2(0,0);
		if(pUser && ptUser && (pMeshInRender->m_nVertexFormat>>16) ==
			(pMeshInRender->m_ntVFormat>>16))
		{
			for(int i=0;i<pMeshInRender->m_ntVFormat>>16;i++)
			{
				ptUser[i]=pUser[i];
			}
		}
		else if(ptUser)
		{
			for(int i=0;i<pMeshInRender->m_ntVFormat>>16;i++)
			{
				ptUser[i]=0;
			}
		}
	}	
}


void DefaultPixelFunc(RawImage* image, Point2D pos, int count, float* vdata, void* p)
{
	CMesh* pMeshInRender = (CMesh*)p;
	if(image == NULL || pMeshInRender == NULL)
		return;
	if(count==0)
		return;
	Vec3 color(1,1,1);
	Vec3 tex(1,1,1);

	Vec3 colorout(0,0,0);

	int idep=pMeshInRender->m_idepData;
	int iclr=pMeshInRender->m_iclrData;
	int itex=pMeshInRender->m_itexData;
	int iuser = pMeshInRender->m_iuserData;
	float depth=0;
	float* pDep  = NULL;
	Vec3* pClr   = NULL;
	Vec2* pTex   = NULL;
	float* pUser = NULL;
	if(idep!=-1)
	{
		pDep=&depth;//vdata+idep;
		depth=1./(*(vdata+idep));
	}
	if(iclr!=-1)
	{
		pClr = (Vec3*)(vdata+iclr);
	}
	if(itex!=-1)
	{
		pTex = (Vec2*)(vdata+itex);
	}
	if(iuser!=-1)
	{
		pUser=vdata+iuser;
	}

	if(pMeshInRender->m_pPShader != NULL)
	{
		pMeshInRender->m_pPShader(pos, pClr, pTex, pUser, &colorout, pDep, pMeshInRender->m_pUserPtr);
		colorout*=255;
	}

	RawImage* depbuf=pMeshInRender->m_pDepBuf;
	if(depbuf !=NULL && pDep != NULL)
	{
		if(*(float*)PTR_PIX(*depbuf, pos.x, pos.y)<depth)
		{
			*(float*)PTR_PIX(*depbuf, pos.x, pos.y)=depth;
		}
		else
			return;
	}

	if(pMeshInRender->m_pPShader == NULL)
	{
		if(pClr)color=*pClr;
		if(color.x<0)color.x=0;
		if(color.y<0)color.y=0;
		if(color.z<0)color.z=0;
		if(pMeshInRender->m_pTex != NULL && pTex != NULL)
		{
			Vec2 texcoord=*pTex;
			Vec3 stex=Sample(*pMeshInRender->m_pTex, texcoord);
			tex = stex/255;
		}
		colorout = color*tex*255;
	}

	uchar* ptr=PTR_PIX(*image, pos.x, pos.y);
	* ptr   = max(0,min(255,colorout.x));
	*(ptr+1)= max(0,min(255,colorout.y));
	*(ptr+2)= max(0,min(255,colorout.z));
}

CMesh::CMesh()
{
	m_nFace=0;
	m_nVert=0;

	m_nVertexFormat=VF_XYZ|VF_TEXCOORD;
	m_ntVFormat=VFT_XYZ|VFT_TEXCOORD;
	m_vbuf=NULL;
	m_ibuf=NULL;
	m_pVp=NULL;
	m_rot=Mat(1,0,0,0,1,0,0,0,1);
	m_trans=Vec3(0,0,0);
	m_bNormalize=false;
	m_fAngle=1;
	m_nCullMode = CULL_MODE_NONE;
	m_pUserPtr = NULL;

	m_pVFunc = DefaultVertexFunc;
	m_pPFunc = DefaultPixelFunc;
	m_pVShader = NULL;
	m_pPShader = NULL;
	m_pDepBuf = NULL;
	m_pTex   = NULL;
	m_tvbuf = NULL;

	m_idepData=-1;
	m_iclrData=-1;
	m_itexData=-1;
	m_iuserData=-1;
}

CMesh::~CMesh()
{
	Release();
}

void CMesh::Release()
{
	if(m_vbuf)
	{
		delete[] m_vbuf;
		m_vbuf = NULL;
	}
	if(m_ibuf)
	{
		delete[] m_ibuf;
		m_ibuf = NULL;
	}
	if(m_pVp)
	{
		delete m_pVp;
		m_pVp = NULL;
	}
	if(m_tvbuf)
	{
		delete[] m_tvbuf;
		m_tvbuf = NULL;
	}
}

int CMesh::ComputeVertLength()
{
	int vertlen=0;
	for(int i=0;i<NUM_VERTEX_ATTR;i++)
	{
		if(m_nVertexFormat&g_VFMask[i])
		{
			vertlen+=g_VFMap[i];
		}
	}
	if(m_nVertexFormat&VF_USERF)
	{
		vertlen+=(m_nVertexFormat>>16);
	}
	return vertlen;
}

int CMesh::ComputePixLength()
{
	//m_ntVFormat=ComputetVFormat(m_nVertexFormat);
	int pixlen=0;
	for(int i=0;i<NUM_PIXEL_ATTR;i++)
	{
		if(m_ntVFormat & g_tVMask[i])
		{
			pixlen+=g_PFMap[i];
		}
	}
	if(m_ntVFormat & VFT_USERF)
	{
		pixlen+=(m_ntVFormat>>16);
	}
	return pixlen;
}

int CMesh::ComputetVFormat(int vertexFormat)
{
	int pixFormat=0;
	for(int i=0;i<NUM_PIXEL_ATTR;i++)
	{
		if(vertexFormat&g_PFMask[i])
		{
			pixFormat |= g_tVMask[i];
		}
	}
	if(vertexFormat&VF_USERF)
	{
		pixFormat|=(vertexFormat&0xffff0000);
	}
	return pixFormat;
}

int CMesh::VertAttrOffset(int nAttr)
{
	int offset=0;
	for(int i=0;i<NUM_VERTEX_ATTR;i++)
	{
		if(nAttr == g_VFMask[i])
			break;
		if(m_nVertexFormat&g_VFMask[i])
		{
			offset+=g_VFMap[i];
		}
	}
	return offset;
}

int CMesh::PixAttrOffset(int nAttr)
{
//	m_ntVFormat=ComputetVFormat(m_nVertexFormat);
	int offset=0;
	for(int i=0;i<NUM_PIXEL_ATTR;i++)
	{
		if(nAttr == g_tVMask[i])
			break;
		if(m_ntVFormat&g_tVMask[i])
		{
			offset+=g_PFMap[i];
		}
	}
	return offset;
}

void CMesh::CreateMesh(int nVert, int nFace, int vertexFormat)
{
	m_nFace=nFace;
	m_nVert=nVert;

	m_nVertexFormat = vertexFormat;
	m_ntVFormat = ComputetVFormat(m_nVertexFormat);

	int vertlen = ComputeVertLength();

	m_vbuf=new float[vertlen*m_nVert];
	m_ibuf=new DWORD[m_nFace*3];
	ZeroMemory(m_vbuf, vertlen*m_nVert*4);
	ZeroMemory(m_ibuf, m_nFace*3*4);
}

bool CMesh::LoadMesh(char* filename)
{
	USES_CONVERSION;
	HANDLE hfile = CreateFile(A2T(filename), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,0);
	if(hfile == INVALID_HANDLE_VALUE || hfile == 0)
	{
		return false;
	}
	DWORD nRd;
	int indexFormat;
	ReadFile(hfile, &m_nVertexFormat, 4, &nRd, 0);
	ReadFile(hfile, &indexFormat, 4, &nRd, 0);

	m_ntVFormat = ComputetVFormat(m_nVertexFormat);
	int vertlen = ComputeVertLength();

	ReadFile(hfile,&m_nVert,4,&nRd,0);
	if(m_vbuf!=NULL)
	{
		delete[] m_vbuf;
		m_vbuf = NULL;
	}
	m_vbuf = new float[m_nVert*vertlen];
	ReadFile(hfile, m_vbuf, m_nVert*vertlen*4, &nRd, 0);
	ReadFile(hfile, &m_nFace, 4, &nRd, 0);
	m_ibuf = new DWORD[m_nFace*3];
	if(indexFormat == IF_INDEX32)
	{
		ReadFile(hfile, m_ibuf, m_nFace*3*4, &nRd, 0);
	}
	else if(indexFormat == IF_INDEX16)
	{
		WORD* tmp=new WORD[m_nFace*3];
		ReadFile(hfile, tmp, m_nFace*3*2, &nRd, 0);
		for(int i=0;i<m_nFace*3;i++)
		{
			m_ibuf[i]=tmp[i];
		}
		delete[] tmp;
	}
	CloseHandle(hfile);
	return true;
}

bool CMesh::SaveMesh(char* filename, int indexFormat)
{
	USES_CONVERSION;
	HANDLE hfile = CreateFile(A2T(filename), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(hfile == INVALID_HANDLE_VALUE || hfile == 0)
	{
		return false;
	}

	DWORD nWt;

	WriteFile(hfile, &m_nVertexFormat, 4, &nWt, 0);
	WriteFile(hfile, &indexFormat, 4, &nWt, 0);

	WriteFile(hfile, &m_nVert, 4, &nWt, 0);
	int vertlen = ComputeVertLength();
	WriteFile(hfile, m_vbuf, vertlen*m_nVert*4, &nWt, 0);

	WriteFile(hfile, &m_nFace, 4, &nWt, 0);
	if(indexFormat == IF_INDEX32)
	{
		WriteFile(hfile, m_ibuf, m_nFace*3*4, &nWt, 0);
	}
	else if(indexFormat == IF_INDEX16)
	{
		WORD* tmp=new WORD[m_nFace*3];
		for(int i=0;i<m_nFace*3;i++)
		{
			tmp[i]=m_ibuf[i];
		}
		WriteFile(hfile, tmp, m_nFace*3*2, &nWt, 0);
		delete[] tmp;
	}
	CloseHandle(hfile);
	return true;
}

void CMesh::Render(RawImage image, RawImage* depthbuf)
{
	m_idepData=-1;
	m_iclrData=-1;
	m_itexData=-1;
	m_iuserData=-1;

	m_pDepBuf = depthbuf;

	int pixlen = ComputePixLength();
	if(m_tvbuf == NULL)
	{
		m_tvbuf=new float[pixlen*m_nVert];
	}
	VertexProcess();

	float (*vdata)[3]=new float[pixlen+1][3];
	Vec3 tPos[3];

	for(int i=0;i<m_nFace;i++)
	{
		int nvdata=0;
		for(int j=0;j<3;j++)
		{
			int count=0;
			float* pv=m_tvbuf+m_ibuf[i*3+j]*pixlen; 
			Vec3* pPos=(Vec3*)(pv+PixAttrOffset(VFT_XYZ));
			tPos[j]=*pPos;
			if(m_pDepBuf != NULL)
			{
				vdata[count][j]=1/pPos->z;
				m_idepData=count;
				count++;
			}
			if(m_ntVFormat & VFT_COLOR)
			{
				Vec3* pClr=(Vec3*)(pv+PixAttrOffset(VFT_COLOR));
				vdata[count][j]=pClr->x;
				vdata[count+1][j]=pClr->y;
				vdata[count+2][j]=pClr->z;
				m_iclrData=count;
				count+=3;
			}
			if(m_ntVFormat & VFT_TEXCOORD)
			{
				Vec2* pTex=(Vec2*)(pv+PixAttrOffset(VFT_TEXCOORD));
				vdata[count][j] = pTex->x;
				vdata[count+1][j] = pTex->y;
				m_itexData = count;
				count+=2;
			}
			if(m_ntVFormat & VFT_USERF)
			{
				float* pUser=pv+PixAttrOffset(VFT_USERF);
				int nudata=(m_ntVFormat>>16);
				for(int k=0;k<nudata;k++)
					vdata[count+k][j]=pUser[k];
				m_iuserData = count;
				count+=nudata;
			}
			nvdata = count;
		}
		Triangle3D(image, tPos, nvdata, vdata, m_pPFunc, this, m_pVp, m_nCullMode);
	}
	delete[] vdata;
	m_pDepBuf = NULL;
}

void CMesh::VertexProcess()
{
	if(m_tvbuf == NULL)
		return;
	int vertlen = ComputeVertLength();
	int pixlen = ComputePixLength();
	for(int i=0;i<m_nVert;i++)
	{
		float* pv=m_vbuf+i*vertlen;
		float* ptv=m_tvbuf+i*pixlen;

		if(m_pVFunc != NULL)
		{
			m_pVFunc(pv, vertlen, ptv, pixlen, this);
		}
	}
}

void CMesh::SetViewPort(Rect2D* vp)
{
	if(m_pVp != NULL)
	{
		delete m_pVp;
		m_pVp = NULL;
	}
	if(vp != NULL)
	{
		m_pVp=new Rect2D(*vp);
	}
}

void CMesh::SetMatrix(Mat rot, Vec3 trans)
{
	m_rot = rot;
	m_trans = trans;
}

void CMesh::SetProjAngle(float fAngle)
{
	m_fAngle=tan(fAngle/2);
}

void CMesh::NormalizeNormals(bool bNorm)
{
	m_bNormalize=bNorm;
}

void CMesh::SetTexture(RawImage* texture)
{
	m_pTex = texture;
}

void CMesh::SetPixelFunc(PIXELFUNC pFunc)
{
	m_pPFunc = pFunc;
}

void CMesh::SetVertexFunc(VERTEXFUNC pFunc)
{
	m_pVFunc = pFunc;
}

void CMesh::SetDefaultFunc()
{
	m_pPFunc = DefaultPixelFunc;
	m_pVFunc = DefaultVertexFunc;
}

void CMesh::SetCullMode(int nCullMode)
{
	m_nCullMode = nCullMode;
}

void CMesh::SetPixelShader(PShader pShaderFunc)
{
	m_pPShader = pShaderFunc;
}

void CMesh::SetVertexShader(VShader pShaderFunc)
{
	m_pVShader = pShaderFunc;
}

void CMesh::SetVSOutFormat(int tvFormat)
{
	if(m_tvbuf != NULL)
	{
		delete[] m_tvbuf;
		m_tvbuf = NULL;
	}

	m_ntVFormat = tvFormat;
}

void CMesh::SetDefaultVSOut()
{
	m_ntVFormat = ComputetVFormat(m_nVertexFormat);
}

void CMesh::SetUserParam(void* p)
{
	m_pUserPtr = p;
}