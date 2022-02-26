
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "3DCollageModel.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#pragma warning(disable:4996)
// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
}

void CChildView::GenCube(int nSeg, char* name)
{
	CMesh mesh;
	mesh.CreateMesh(24*nSeg*nSeg, 12*nSeg*nSeg, VF_XYZ|VF_NORMAL|VF_TEXCOORD);
	struct vertex
	{
		Vec3 pos;
		Vec3 normal;
		Vec2 tex;
	};
	vertex* pv = (vertex*)mesh.m_vbuf;
	Mat mat[6] = {Mat(1,0,0,0,1,0,0,0,1),Mat(0,0,1,0,1,0,-1,0,0),Mat(-1,0,0,0,1,0,0,0,-1),
		Mat(0,0,-1,0,1,0,1,0,0),Mat(1,0,0,0,0,1,0,-1,0),Mat(1,0,0,0,0,-1,0,1,0)};
	for(int i=0;i<6;i++)
	{
		for(int m=0;m<nSeg;m++)
		for(int n=0;n<nSeg;n++)
		for(int j=0;j<2;j++)
		{
			for(int k=0;k<2;k++)
			{
				vertex v;
				v.pos.z = -1;
				v.pos.x = -1+(float)(m+j)*2/nSeg;
				v.pos.y = -1+(float)(n+k)*2/nSeg;
				v.normal.x = 0;
				v.normal.y = 0;
				v.normal.z = -1;
				v.tex.x = j;
				v.tex.y = 1-k;
				v.pos=v.pos*mat[i];
				v.normal=v.normal*mat[i];
				pv[i*4*nSeg*nSeg+m*nSeg*4+n*4+j*2+k] = v;
			}
		}
	}
	DWORD* ind = mesh.m_ibuf;
	for(int i=0;i<6*nSeg*nSeg;i++)
	{
		ind[i*6] = i*4;
		ind[i*6+1] = i*4+1;
		ind[i*6+2] = i*4+3;
		ind[i*6+3] = i*4+3;
		ind[i*6+4] = i*4+2;
		ind[i*6+5] = i*4;
	}
	strcpy(mesh.m_szFaceAttrDesc, "groupindex:int");
	mesh.m_nFaceAttrLen = 1;
	mesh.m_FaceAttr = new float[mesh.m_nFace*mesh.m_nFaceAttrLen];
	for(int i=0;i<6*nSeg*nSeg;i++)
	{
		*(int*)(mesh.m_FaceAttr+i*2) = i;
		*(int*)(mesh.m_FaceAttr+i*2+1) = i;
	}
	mesh.SaveMesh(name);
	float sqr2=sqrtf(2),sqr3=sqrtf(3),sqr6=sqrtf(6);
	Mat m(sqr2/2,0,-sqr2/2,sqr3/3,sqr3/3,sqr3/3,sqr6/6,-sqr6/3,sqr6/6);
	for(int i=0;i<mesh.m_nVert;i++)
	{
		vertex* pv = ((vertex*)mesh.m_vbuf)+i;
		pv->pos = pv->pos*m.trans();
		pv->normal = pv->normal*m.trans();
	}
	char name2[50];
	strcpy(name2,name);
	char* p;
	for(p=name2;*p!=0&&*p!='.';p++);
	if(*p=='.')
	{
		char ext[50];
		strcpy(ext, p);
		*(p++)='2';
		strcpy(p, ext);
	}
	else
	strcat(name2, "2");
	mesh.SaveMesh(name2);
}

void CChildView::GenSpere()
{
	CMesh mesh;
	float r = 1;
	int nSeg = 100;
	mesh.CreateMesh(2*nSeg*(nSeg-1)+2, 4*nSeg*(nSeg-1),VF_XYZ|VF_NORMAL);
	struct vertex
	{
		Vec3 pos;
		Vec3 normal;
	};
	vertex* pv=(vertex*)mesh.m_vbuf;
	pv[0].pos=Vec3(0,-r,0);
	pv[0].normal=Vec3(0,-1,0);
	for(int i=1;i<nSeg;i++)
	{
		for(int j=0;j<2*nSeg;j++)
		{
			float vangle=-PI/2+PI/nSeg*i;
			float hangle=PI/nSeg*j;
			Vec3 vec(cos(vangle)*sin(hangle),sin(vangle),cos(vangle)*cos(hangle));
			pv[1+(i-1)*2*nSeg+j].pos=vec*r;
			pv[1+(i-1)*2*nSeg+j].normal=vec;
		}
	}
	pv[2*nSeg*(nSeg-1)+1].pos=Vec3(0,r,0);
	pv[2*nSeg*(nSeg-1)+1].normal=Vec3(0,1,0);
	DWORD* ind=mesh.m_ibuf;
	for(int i=0;i<2*nSeg;i++)
	{
		ind[3*i]=0;
		ind[3*i+1]=1+(i+1)%(2*nSeg);
		ind[3*i+2]=1+i;
	}
	for(int i=1;i<nSeg-1;i++)
	{
		for(int j=0;j<2*nSeg;j++)
		{
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6  ]=1+(i-1)*2*nSeg+(j+1)%(2*nSeg);
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+1]=1+(i  )*2*nSeg+(j+1)%(2*nSeg);
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+2]=1+(i  )*2*nSeg+j;
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+3]=1+(i  )*2*nSeg+j;
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+4]=1+(i-1)*2*nSeg+j;
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+5]=1+(i-1)*2*nSeg+(j+1)%(2*nSeg);
		}
	}
	for(int i=0;i<2*nSeg;i++)
	{
		ind[nSeg*6+(nSeg-2)*nSeg*12+i*3  ]=1+(nSeg-2)*nSeg*2+i;
		ind[nSeg*6+(nSeg-2)*nSeg*12+i*3+1]=1+(nSeg-2)*nSeg*2+(i+1)%(2*nSeg);
		ind[nSeg*6+(nSeg-2)*nSeg*12+i*3+2]=1+(nSeg-1)*nSeg*2;
	}
	strcpy(mesh.m_szFaceAttrDesc, "groupindex:int");
	mesh.m_nFaceAttrLen = 1;
	mesh.m_FaceAttr = new float[mesh.m_nFace*mesh.m_nFaceAttrLen];
	int ngrp = 10;
	for(int i=0;i<2*nSeg;i++)
	{
		*(int*)(mesh.m_FaceAttr+i) = i/ngrp;
	}
	for(int i=1;i<nSeg-1;i++)
	{
		for(int j=0;j<2*nSeg;j++)
		{
			*(int*)(mesh.m_FaceAttr+2*nSeg+(i-1)*2*nSeg*2+j*2) = (i/ngrp)*(nSeg/ngrp*2)+j/ngrp;
			*(int*)(mesh.m_FaceAttr+2*nSeg+(i-1)*2*nSeg*2+j*2+1) = (i/ngrp)*(nSeg/ngrp*2)+j/ngrp;
		}
	}
	for(int i=0;i<2*nSeg;i++)
	{
		*(int*)(mesh.m_FaceAttr+2*nSeg+(nSeg-2)*nSeg*4+i) = (nSeg/ngrp-1)*(nSeg/ngrp*2)+i/ngrp;
	}
	strcpy(mesh.m_szGrpAttrDesc, "center:float3,tanu:float3,tanv:float3");
	mesh.m_nGrpAttrLen = 9;
	mesh.m_nGrpAttr = (nSeg/ngrp)*(nSeg/ngrp)*2;
	mesh.m_GrpAttr = new float[mesh.m_nGrpAttr*mesh.m_nGrpAttrLen];
	for(int i=0;i<nSeg/ngrp;i++)
	{
		for(int j=0;j<nSeg/ngrp*2;j++)
		{
			float vangle=-PI/2+PI/(nSeg/ngrp)*i+PI/(nSeg/ngrp)/2;
			float hangle=PI/(nSeg/ngrp)*j+PI/(nSeg/ngrp)/2;
			Vec3 vec(cos(vangle)*sin(hangle),sin(vangle),cos(vangle)*cos(hangle));
			Vec3 center = vec*r;
			Vec3 tanu(vec.z,0,-vec.x);
			tanu = tanu/tanu.length();
			Vec3 tanv;
			tanv.y = sqrt(vec.x*vec.x+vec.z*vec.z);
			Vec2 tanvxy = -Vec2(vec.x, vec.z).normalize()*vec.y;
			tanv.x = tanvxy.x;
			tanv.z = tanvxy.y;
			tanu = -tanu*10/CV_PI;
			tanv = -tanv*10/CV_PI;
			*(Vec3*)(mesh.m_GrpAttr+(i*nSeg/ngrp*2+j)*mesh.m_nGrpAttrLen) = center;
			*(Vec3*)(mesh.m_GrpAttr+(i*nSeg/ngrp*2+j)*mesh.m_nGrpAttrLen+3) = tanu;
			*(Vec3*)(mesh.m_GrpAttr+(i*nSeg/ngrp*2+j)*mesh.m_nGrpAttrLen+6) = tanv;
		}
	}
	mesh.SaveMesh("sphere.mesh");
}

void CChildView::GenOctahedron()
{
	CMesh mesh;
	mesh.CreateMesh(24, 8, VF_XYZ|VF_NORMAL|VF_TEXCOORD);
	struct vertex
	{
		Vec3 pos;
		Vec3 normal;
		Vec2 tex;
	};
	vertex* pv = (vertex*)mesh.m_vbuf;
	Mat mat[4] = {Mat(1,0,0,0,1,0,0,0,1),Mat(0,0,1,0,1,0,-1,0,0),Mat(-1,0,0,0,1,0,0,0,-1),
		Mat(0,0,-1,0,1,0,1,0,0)};
	for(int i=0;i<4;i++)
	{
		vertex vert[3];
		vert[0].pos = Vec3(0,1,0);
		vert[1].pos = Vec3(1,0,0);
		vert[2].pos = Vec3(0,0,-1);
		vert[0].normal = vert[1].normal = vert[2].normal = Vec3(1,1,-1).normalize();
		vert[0].tex = Vec2(0.5,0);
		vert[1].tex = Vec2(1,1);
		vert[2].tex = Vec2(0,1);
		for(int j=0;j<3;j++)
		{
			vert[j].pos = vert[j].pos*mat[i];
			vert[j].normal = vert[j].normal*mat[i];
			pv[i*3+j] = vert[j];
		}
	}
	for(int i=0;i<4;i++)
	{
		vertex vert[3];
		vert[0].pos = Vec3(0,-1,0);
		vert[1].pos = Vec3(0,0,-1);
		vert[2].pos = Vec3(1,0,0);
		vert[0].normal = vert[1].normal = vert[2].normal = Vec3(1,-1,-1).normalize();
		vert[0].tex = Vec2(0.5,1);
		vert[1].tex = Vec2(0,0);
		vert[2].tex = Vec2(1,0);
		for(int j=0;j<3;j++)
		{
			vert[j].pos = vert[j].pos*mat[i];
			vert[j].normal = vert[j].normal*mat[i];
			pv[12+i*3+j] = vert[j];
		}
	}
	DWORD* ind = mesh.m_ibuf;
	for(int i=0;i<24;i++)
		ind[i] = i;
	strcpy(mesh.m_szFaceAttrDesc, "groupindex:int");
	mesh.m_nFaceAttrLen = 1;
	mesh.m_FaceAttr = new float[mesh.m_nFace*mesh.m_nFaceAttrLen];
	for(int i=0;i<8;i++)
		*(int*)(mesh.m_FaceAttr+i) = i;
	mesh.SaveMesh("octahedron.mesh");
}

void CChildView::GenIcosahedron()
{
	CMesh mesh;
	mesh.CreateMesh(60, 20, VF_XYZ|VF_NORMAL|VF_TEXCOORD);
	struct vertex
	{
		Vec3 pos;
		Vec3 normal;
		Vec2 tex;
	};
	vertex* pv = (vertex*)mesh.m_vbuf;
	float sqrt5 = sqrtf(5.f);
	Vec3 vert[5] = {Vec3(0,0.5,1), Vec3(sqrt(10+2*sqrt5)/4,0.5,(sqrt5-1)/4), Vec3(sqrt(10-2*sqrt5)/4,0.5,-(1+sqrt5)/4),Vec3(-sqrt(10-2*sqrt5)/4,0.5,-(1+sqrt5)/4),Vec3(-sqrt(10+2*sqrt5)/4,0.5,(sqrt5-1)/4)};
	for(int i=0;i<5;i++)
		vert[i]/=sqrt5/2;
	for(int i=0;i<5;i++)
	{
		vertex v[3];
		v[0].pos = Vec3(0,1,0);
		v[1].pos = vert[i];
		v[2].pos = vert[(i+1)%5];
		v[0].normal = v[1].normal = v[2].normal = (v[0].pos+v[1].pos+v[2].pos).normalize();
		v[0].tex = Vec2(0.5,0);
		v[1].tex = Vec2(1,1);
		v[2].tex = Vec2(0,1);
		for(int j=0;j<3;j++)
		{
			pv[i*3+j] = v[j];
		}
	}
	for(int i=0;i<5;i++)
	{
		vertex v[6];
		v[0].pos = vert[(i+1)%5];
		v[1].pos = vert[i];
		v[2].pos = -vert[(i+3)%5];
		v[0].normal = v[1].normal = v[2].normal = (v[0].pos+v[1].pos+v[2].pos).normalize();
		v[0].tex = Vec2(0,0);
		v[1].tex = Vec2(1,0);
		v[2].tex = Vec2(0.5,1);
		v[3].pos = vert[i];
		v[4].pos = -vert[(i+2)%5];
		v[5].pos = -vert[(i+3)%5];
		v[3].normal = v[4].normal = v[5].normal = (v[3].pos+v[4].pos+v[5].pos).normalize();
		v[3].tex = Vec2(0.5,0);
		v[4].tex = Vec2(1,1);
		v[5].tex = Vec2(0,1);
		
		for(int j=0;j<6;j++)
		{
			pv[15+i*6+j] = v[j];
		}
	}
	for(int i=0;i<5;i++)
	{
		vertex v[3];
		v[0].pos = Vec3(0,-1,0);
		v[1].pos = -vert[(i+1)%5];
		v[2].pos = -vert[i];
		v[0].normal = v[1].normal = v[2].normal = (v[0].pos+v[1].pos+v[2].pos).normalize();
		v[0].tex = Vec2(0.5,1);
		v[1].tex = Vec2(0,0);
		v[2].tex = Vec2(1,0);
		for(int j=0;j<3;j++)
		{
			pv[45+i*3+j] = v[j];
		}
	}
	DWORD* ind = mesh.m_ibuf;
	for(int i=0;i<60;i++)
		ind[i]=i;
	strcpy(mesh.m_szFaceAttrDesc, "groupindex:int");
	mesh.m_nFaceAttrLen = 1;
	mesh.m_FaceAttr = new float[mesh.m_nFace*mesh.m_nFaceAttrLen];
	for(int i=0;i<20;i++)
		*(int*)(mesh.m_FaceAttr+i) = i;
	mesh.SaveMesh("icosahedron.mesh");
}

void CChildView::GenFootball()
{
	CMesh mesh;
	mesh.CreateMesh(92, 180, VF_XYZ|VF_NORMAL);
	struct vertex
	{
		Vec3 pos;
		Vec3 normal;
	};
	vertex* pv = (vertex*)mesh.m_vbuf;
	float sqrt5 = sqrtf(5.f);
	Vec3 vert[5] = {Vec3(0,0.5,1), Vec3(sqrt(10+2*sqrt5)/4,0.5,(sqrt5-1)/4), Vec3(sqrt(10-2*sqrt5)/4,0.5,-(1+sqrt5)/4),Vec3(-sqrt(10-2*sqrt5)/4,0.5,-(1+sqrt5)/4),Vec3(-sqrt(10+2*sqrt5)/4,0.5,(sqrt5-1)/4)};
	for(int i=0;i<5;i++)
		vert[i]/=sqrt5/2;
	Vec3 vertp[12];
	vertp[0] = Vec3(0,1,0);
	for(int i=0;i<5;i++)
		vertp[1+i] = vert[i];
	for(int i=0;i<5;i++)
		vertp[6+i] = -vert[i];
	vertp[11] = Vec3(0,-1,0);
	for(int i=0;i<12;i++)
		vertp[i] = vertp[i].normalize();
	int index[60];
	for(int i=0;i<5;i++)
	{
		index[i*3] = 0;
		index[i*3+1] = 1+i;
		index[i*3+2] = 1+(1+i)%5;
	}
	for(int i=0;i<5;i++)
	{
		index[15+i*6] = 1+i;
		index[15+i*6+1] = 6+(i+3)%5;
		index[15+i*6+2] = 1+(1+i)%5;
		index[15+i*6+3] = 1+i;
		index[15+i*6+4] = 6+(i+2)%5;
		index[15+i*6+5] = 6+(i+3)%5;
	}
	for(int i=0;i<5;i++)
	{
		index[45+i*3] = 6+(i+1)%5;
		index[45+i*3+1] = 6+i;
		index[45+i*3+2] = 11;
	}
	int conn[12][5][3];
	int face[12][5];
	int nconn[12];
	int nface[12];
	ZeroMemory(nconn, sizeof(nconn));
	ZeroMemory(nface, sizeof(nface));
	for(int i=0;i<20;i++)
	{
		int n[3] = {index[3*i], index[3*i+1], index[3*i+2]};
		for(int j=0;j<3;j++)
		{
			int n1 = n[j];
			face[n1][nface[n1]] = i;
			nface[n1]++;
			ASSERT(nface[n1]<=5);
			for(int k=0;k<3;k++)
			{
				if(k==j)continue;
				int n2 = n[k];
				int p;
				for(p=0;p<nconn[n1];p++)
				{
					if(conn[n1][p][0] == n2)
						break;
				}
				if(p == nconn[n1])
				{
					conn[n1][nconn[n1]][0] = n2;
					nconn[n1]++;
					ASSERT(nconn[n1]<=5);
				}
			}
		}
	}
	for(int i=0;i<12;i++)
	{
		ASSERT(nconn[i]==5);
		ASSERT(nface[i]==5);
	}
	
	for(int i=0;i<12;i++)
	{
		pv[i].pos = vertp[i];
		pv[i].normal = vertp[i];
	}
	for(int i=0;i<12;i++)
	{
		for(int j=0;j<5;j++)
		{
			Vec3 v1 = vertp[i];
			Vec3 v2 = vertp[conn[i][j][0]];
			Vec3 vp = (2*v1+v2)/3;
			vertex v;
			v.pos = vp.normalize();
			v.normal = vp.normalize();
			pv[12+i*5+j] = v;
			conn[i][j][1] = 12+i*5+j;
			int k;
			for(k=0;k<5;k++)
			{
				if(conn[conn[i][j][0]][k][0] == i)
					break;
			}
			ASSERT(k<5);
			conn[conn[i][j][0]][k][2] = 12+i*5+j;
		}
	}
	DWORD* ind = mesh.m_ibuf;
	for(int i=0;i<12;i++)
	{
		for(int j=0;j<5;j++)
		{
			int iface = face[i][j];
			int iv[3] = {index[3*iface],index[3*iface+1],index[3*iface+2]};
			int iev[2];
			int k;
			for(k=0;k<3;k++)
			{
				if(iv[k]==i)
					break;
			}
			ASSERT(k<3);
			for(int n=0;n<2;n++)
				iev[n] = iv[(k+n+1)%3];
			ind[(i*5+j)*3] = i;
			for(int n=0;n<2;n++)
			{
				for(k=0;k<5;k++)
				{
					if(conn[i][k][0] == iev[n])
						break;
				}
				ASSERT(k<5);
				ind[(i*5+j)*3+n+1] = conn[i][k][1];
			}
		}
	}
	for(int i=0;i<20;i++)
	{
		int n[3] = {index[3*i],index[3*i+1],index[3*i+2]};
		vertex v;
		v.pos = (vertp[n[0]]+vertp[n[1]]+vertp[n[2]]).normalize();
		v.normal = v.pos;
		pv[72+i] = v;
		int iv[6];
		for(int j=0;j<3;j++)
		{
			int n1=n[j];
			int n2=n[(j+1)%3];
			int k;
			for(k=0;k<5;k++)
			{
				if(conn[n1][k][0] == n2)
					break;
			}
			ASSERT(k<5);
			iv[j*2] = conn[n1][k][1];
			iv[j*2+1] = conn[n1][k][2];
		}
		for(int j=0;j<6;j++)
		{
			ind[180+(i*6+j)*3] = 72+i;
			ind[180+(i*6+j)*3+1] = iv[j];
			ind[180+(i*6+j)*3+2] = iv[(j+1)%6];
		}
	}
	strcpy(mesh.m_szFaceAttrDesc, "groupindex:int");
	mesh.m_nFaceAttrLen = 1;
	mesh.m_FaceAttr = new float[mesh.m_nFace*mesh.m_nFaceAttrLen];
	for(int i=0;i<12;i++)
	{
		for(int j=0;j<5;j++)
		{
			*(int*)(mesh.m_FaceAttr+i*5+j) = i;
		}
	}
	for(int i=0;i<20;i++)
	{
		for(int j=0;j<6;j++)
		{
			*(int*)(mesh.m_FaceAttr+60+i*6+j) = i+12;
		}
	}
	strcpy(mesh.m_szGrpAttrDesc, "center:float3,tanu:float3,tanv:float3");
	mesh.m_nGrpAttrLen = 9;
	mesh.m_nGrpAttr = 32;
	mesh.m_GrpAttr = new float[mesh.m_nGrpAttr*mesh.m_nGrpAttrLen];
	float uvscale = (vertp[0]-vertp[1]).length()*0.75;
	for(int i=0;i<12;i++)
	{
		Vec3 center = vertp[i];
		Vec3 tanu = cross(center,Vec3(0,1,0));
		if(tanu.length()>0)
			tanu = tanu.normalize();
		else
			tanu = Vec3(1,0,0);
		Vec3 tanv = -cross(tanu,center);
		tanu/=uvscale;
		tanv/=uvscale;
		*(Vec3*)(mesh.m_GrpAttr+i*9) = center;
		*(Vec3*)(mesh.m_GrpAttr+i*9+3) = tanu;
		*(Vec3*)(mesh.m_GrpAttr+i*9+6) = tanv;
	}
	for(int i=0;i<20;i++)
	{
		int n[3] = {index[3*i],index[3*i+1],index[3*i+2]};
		Vec3 center = (vertp[n[0]]+vertp[n[1]]+vertp[n[2]]).normalize();
		Vec3 tanu = -cross(center,Vec3(0,1,0));
		if(tanu.length()>0)
			tanu = tanu.normalize();
		else
			tanu = Vec3(1,0,0);
		Vec3 tanv = -cross(tanu,center);
		tanu/=uvscale;
		tanv/=uvscale;
		*(Vec3*)(mesh.m_GrpAttr+(i+12)*9) = center;
		*(Vec3*)(mesh.m_GrpAttr+(i+12)*9+3) = tanu;
		*(Vec3*)(mesh.m_GrpAttr+(i+12)*9+6) = tanv;
	}
	mesh.SaveMesh("football0.mesh");
	Tesselation(mesh);
	mesh.SaveMesh("football.mesh");
}

void CChildView::Tesselation(CMesh& mesh)
{
	ASSERT(mesh.m_nVertexFormat == (VF_XYZ|VF_NORMAL));
	struct vertex
	{
		Vec3 pos;
		Vec3 normal;
	};
	int buflen = 65536;//1024*1024
	vertex* pvbuf = new vertex[buflen];
	DWORD* ibuf = new DWORD[buflen*3];
	int* idface = new int[buflen];
	DWORD* adj = new DWORD[buflen*3];
	bool* divtag = new bool[buflen];
	memcpy(pvbuf, mesh.m_vbuf, mesh.m_nVert*sizeof(vertex));
	memcpy(ibuf, mesh.m_ibuf, mesh.m_nFace*3*sizeof(DWORD));
	memcpy(idface, mesh.m_FaceAttr, mesh.m_nFace*sizeof(int));
	int nv = mesh.m_nVert;
	int nf = mesh.m_nFace;
	for(int i=0;i<mesh.m_nFace*3;i++)
		adj[i]=-1;
	int (*face)[10] = new int[nv][10];
	int *nface = new int[nv];
	ZeroMemory(nface, nv*sizeof(int));
	for(int i=0;i<nf;i++)
	{
		for(int j=0;j<3;j++)
		{
			int iv = ibuf[i*3+j];
			face[iv][nface[iv]] = i;
			nface[iv]++;
		}
	}
	for(int i=0;i<nv;i++)
	{
		for(int j=0;j<nface[i];j++)
		{
			int iv[3] = {ibuf[face[i][j]*3],ibuf[face[i][j]*3+1],ibuf[face[i][j]*3+2]};
			for(int k=0;k<3;k++)
			{
				if(iv[k]==i)
					continue;
				for(int n=0;n<nface[i];n++)
				{
					if(n==j)
						continue;
					int m;
					for(m=0;m<3;m++)
					{
						if(ibuf[face[i][n]*3+m]==iv[k])
							break;
					}
					if(m<3)
					{
						int f[2] = {face[i][j],face[i][n]};
						for(int p=0;p<2;p++)
						{
							for(int q=0;q<3;q++)
							{
								if(ibuf[f[p]*3+q] != i && ibuf[f[p]*3+q] != iv[k])
								{
									adj[f[p]*3+q] = f[1-p];
								}
							}
						}
					}
				}
			}
		}
	}
	for(int i=0;i<nf*3;i++)
	{
		ASSERT(adj[i]!=-1);
	}
	for(int i=0;i<nf;i++)
	{
		int ifc[3] = {adj[i*3],adj[i*3+1],adj[i*3+2]};
		for(int j=0;j<3;j++)
		{
			int iv[3] = {ibuf[i*3],ibuf[i*3+1],ibuf[i*3+2]};
			int ivadj[3] = {ibuf[ifc[j]*3], ibuf[ifc[j]*3+1],ibuf[ifc[j]*3+2]};
			for(int k=0;k<3;k++)
			{
				if(k==j)continue;
				int l;
				for(l=0;l<3;l++)
					if(ivadj[l]==iv[k])
						break;
				ASSERT(l<3);
			}
		}
	}

	delete[] face;
	delete[] nface;
	float thresh=0.4;
	bool bEnd = false;
	while(true)
	{
		ZeroMemory(divtag, nf*sizeof(bool));
		float md=0;
		int nvnew = nv, nfnew = nf;
		bool bDivide = false;
		for(int i=0;i<nf;i++)
		{
			int iv[3] = {ibuf[i*3],ibuf[i*3+1],ibuf[i*3+2]};
			float maxdist = 0;
			int jmax=0;
			for(int j=0;j<3;j++)
			{
				float dist = (pvbuf[iv[j]].pos-pvbuf[iv[(j+1)%3]].pos).length();
				if(maxdist<dist)
				{
					maxdist = dist;
					jmax = j;
				}
			}
			if(md<maxdist)
				md=maxdist;

			if(maxdist>thresh && !divtag[i])
			{
				if(nfnew+2>buflen || nvnew+1>buflen)
				{
					bEnd = true;
					break;
				}
				bDivide = true;

				vertex vnew;
				vnew.pos = vnew.normal = (pvbuf[iv[jmax]].pos+pvbuf[iv[(jmax+1)%3]].pos).normalize();
				pvbuf[nvnew] = vnew;

				ibuf[i*3+(jmax+1)%3] = nvnew;
				int adjc = adj[i*3+jmax];
				int jc;
				for(jc=0;jc<3;jc++)
				{
					if(ibuf[adjc*3+jc] != iv[(jmax+1)%3] && ibuf[adjc*3+jc] != iv[(jmax+2)%3])
						break;
				}
				ASSERT(jc<3);
				ASSERT(adj[adjc*3+jc]==i);
				adj[adjc*3+jc] = nfnew;
				adj[nfnew*3] = adjc;
				adj[nfnew*3+1] = i;
				adj[nfnew*3+2] = nfnew+1;
				adj[i*3+jmax] = nfnew;
				ibuf[nfnew*3] = nvnew;
				ibuf[nfnew*3+1] = iv[(jmax+1)%3];
				ibuf[nfnew*3+2] = iv[(jmax+2)%3];
				idface[nfnew] = idface[i];
				int iv2[3] = {ibuf[i*3],ibuf[i*3+1],ibuf[i*3+2]};
				int iv20[3] = {ibuf[nfnew*3], ibuf[nfnew*3+1], ibuf[nfnew*3+2]};
				float sd[3] = {(pvbuf[iv2[0]].pos-pvbuf[iv2[1]].pos).length(),
					(pvbuf[iv2[1]].pos-pvbuf[iv2[2]].pos).length(),
					(pvbuf[iv2[2]].pos-pvbuf[iv2[0]].pos).length()};
				float sd2[3] = {(pvbuf[iv20[0]].pos-pvbuf[iv20[1]].pos).length(),
					(pvbuf[iv20[1]].pos-pvbuf[iv20[2]].pos).length(),
					(pvbuf[iv20[2]].pos-pvbuf[iv20[0]].pos).length()};
				nfnew++;

				int fadj = adj[i*3+(jmax+2)%3];
				int ivadj[3] = {ibuf[fadj*3],ibuf[fadj*3+1],ibuf[fadj*3+2]};
				int j;
				for(j=0;j<3;j++)
				{
					if(ivadj[j] != iv[jmax] && ivadj[j] != iv[(jmax+1)%3])
						break;
				}
				ASSERT(j<3);
				ibuf[fadj*3+(j+1)%3] = nvnew;
				int adjc2 = adj[fadj*3+(j+2)%3];
				for(jc=0;jc<3;jc++)
				{
					if(ibuf[adjc2*3+jc] != ivadj[j] && ibuf[adjc2*3+jc] != ivadj[(j+1)%3])
						break;
				}
				ASSERT(jc<3);
				ASSERT(adj[adjc2*3+jc]==fadj);
				adj[adjc2*3+jc] = nfnew;
				adj[nfnew*3] = nfnew-1;
				adj[nfnew*3+1] = fadj;
				adj[nfnew*3+2] = adjc2;
				adj[fadj*3+(j+2)%3] = nfnew;
				ibuf[nfnew*3] = ivadj[j];
				ibuf[nfnew*3+1] = ivadj[(j+1)%3];
				ibuf[nfnew*3+2] = nvnew;
				idface[nfnew] = idface[fadj];
				int iv2_[3] = {ibuf[fadj*3],ibuf[fadj*3+1],ibuf[fadj*3+2]};
				int iv20_[3] = {ibuf[nfnew*3], ibuf[nfnew*3+1], ibuf[nfnew*3+2]};
				float sd_[3] = {(pvbuf[iv2[0]].pos-pvbuf[iv2[1]].pos).length(),
					(pvbuf[iv2[1]].pos-pvbuf[iv2[2]].pos).length(),
					(pvbuf[iv2[2]].pos-pvbuf[iv2[0]].pos).length()};
				float sd2_[3] = {(pvbuf[iv20[0]].pos-pvbuf[iv20[1]].pos).length(),
					(pvbuf[iv20[1]].pos-pvbuf[iv20[2]].pos).length(),
					(pvbuf[iv20[2]].pos-pvbuf[iv20[0]].pos).length()};

				divtag[i] = true;
				divtag[fadj] = true;
				nvnew++;
				nfnew++;
			}
		}
		if(bDivide)
		{
			nv=nvnew;
			nf=nfnew;
			if(nv>=buflen||nf>=buflen)
				break;
			if(bEnd)
				break;
		}
		else
		{
			thresh/=2;
		}
		for(int i=0;i<nf;i++)
		{
			int ifc[3] = {adj[i*3],adj[i*3+1],adj[i*3+2]};
			for(int j=0;j<3;j++)
			{
				int iv[3] = {ibuf[i*3],ibuf[i*3+1],ibuf[i*3+2]};
				int ivadj[3] = {ibuf[ifc[j]*3], ibuf[ifc[j]*3+1],ibuf[ifc[j]*3+2]};
				for(int k=0;k<3;k++)
				{
					if(k==j)continue;
					int l;
					for(l=0;l<3;l++)
						if(ivadj[l]==iv[k])
							break;
					ASSERT(l<3);
				}
			}
		}
	}
	mesh.m_nFace = nf;
	mesh.m_nVert = nv;
	delete[] mesh.m_vbuf;
	delete[] mesh.m_ibuf;
	delete[] mesh.m_FaceAttr;
	mesh.m_vbuf = new float[mesh.m_nVert*6];
	mesh.m_ibuf = new DWORD[mesh.m_nFace*3];
	mesh.m_FaceAttr = new float[mesh.m_nFace];

	memcpy(mesh.m_vbuf, pvbuf, mesh.m_nVert*sizeof(vertex));
	memcpy(mesh.m_ibuf, ibuf, mesh.m_nFace*3*sizeof(DWORD));
	memcpy(mesh.m_FaceAttr, idface, mesh.m_nFace*sizeof(int));

	delete[] pvbuf;
	delete[] ibuf;
	delete[] idface;
	delete[] adj;
	delete[] divtag;
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	GenCube(1,"cube.mesh");
	GenCube(3,"magic cube.mesh");
	GenOctahedron();
	GenSpere();
	GenIcosahedron();
	GenFootball();
	return 0;
}
