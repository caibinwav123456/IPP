#include "stdafx.h"
#include "CreateMesh.h"
/////convert a mesh from xfile//////
void CreateMeshFile(IDirect3DDevice9* device, char* xfile, char* meshfile, int indexFormat)
{
	struct tvert
	{
		float x,y,z;
		float nx,ny,nz;
	};
	struct tvert2
	{
		float x,y,z;
		float nx,ny,nz;
		float u,v;
	};
	ID3DXMesh* Mesh=NULL;
	D3DXLoadMeshFromX(
		xfile,
		D3DXMESH_SYSTEMMEM | D3DXMESH_32BIT,
		device,
		NULL,
		NULL,
		NULL,
		NULL,
		&Mesh
		);

	ID3DXMesh* CnvMesh = NULL;
	DWORD FVF = Mesh->GetFVF();
	DWORD tFVF = 0;
	if(FVF & D3DFVF_TEX1)
	{
		tFVF |= D3DFVF_TEX1;
	}
	tFVF |= D3DFVF_XYZ;
	tFVF |= D3DFVF_NORMAL;
	Mesh->CloneMeshFVF(D3DXMESH_SYSTEMMEM | D3DXMESH_32BIT, tFVF, device, &CnvMesh);
	//D3DXCreateTeapot(device, &teapot, NULL);
	Mesh->Release();
	if(!(FVF & D3DFVF_NORMAL))
		D3DXComputeNormals(CnvMesh, NULL);

	DWORD vf = VF_XYZ | VF_NORMAL;
	if(tFVF & D3DFVF_TEX1)
		vf |= VF_TEXCOORD;

	IDirect3DVertexBuffer9* vbt;
	IDirect3DIndexBuffer9* ibt;
	CnvMesh->GetVertexBuffer(&vbt);
	CnvMesh->GetIndexBuffer(&ibt);

	DWORD nf=CnvMesh->GetNumFaces();
	DWORD nv=CnvMesh->GetNumVertices();
	CMesh mymesh;
	mymesh.CreateMesh(nv, nf, vf);
	tvert* tv;
	CnvMesh->LockVertexBuffer(0, (void**)&tv);
	if(tFVF & D3DFVF_TEX1)
		memcpy(mymesh.m_vbuf, tv, nv*sizeof(tvert2));
	else
		memcpy(mymesh.m_vbuf, tv, nv*sizeof(tvert));
	CnvMesh->UnlockVertexBuffer();
	DWORD* ind;
	HRESULT hr=CnvMesh->LockIndexBuffer(0, (void**)&ind);
	memcpy(mymesh.m_ibuf, ind, nf*3*sizeof(DWORD));
	CnvMesh->UnlockIndexBuffer();
	if(!mymesh.SaveMesh(meshfile, indexFormat))
	{
		MessageBox(0, "Save Failed","error", MB_OK);
	}
	CnvMesh->Release();
/*
	DWORD l=nv*sizeof(tvert)+nf*3*sizeof(WORD)+8;
	DWORD l1=nv*sizeof(tvert)+4;
	DWORD l2=nf*3*sizeof(WORD)+8;
*/
}
////////////////////////////
