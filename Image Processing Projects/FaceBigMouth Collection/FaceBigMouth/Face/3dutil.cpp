#include "stdafx.h"
#include "3dutil.h"

void Reform(Vec2 vertin[4], Vec3 vertout[4])
{
	float mat1[2][2]={{vertin[1].y-vertin[0].y, -(vertin[1].x-vertin[0].x)}, {vertin[3].y-vertin[2].y, -(vertin[3].x-vertin[2].x)}};
	float vec1[2]   ={vertin[0].x*(vertin[1].y-vertin[0].y)-vertin[0].y*(vertin[1].x-vertin[0].x), vertin[2].x*(vertin[3].y-vertin[2].y)-vertin[2].y*(vertin[3].x-vertin[2].x)};

	float mat2[2][2]={{vertin[2].y-vertin[0].y, -(vertin[2].x-vertin[0].x)}, {vertin[3].y-vertin[1].y, -(vertin[3].x-vertin[1].x)}};
	float vec2[2]   ={vertin[0].x*(vertin[2].y-vertin[0].y)-vertin[0].y*(vertin[2].x-vertin[0].x), vertin[1].x*(vertin[3].y-vertin[1].y)-vertin[1].y*(vertin[3].x-vertin[1].x)};

	float det1=mat1[0][0]*mat1[1][1]-mat1[0][1]*mat1[1][0];
	float x1=vec1[0]*mat1[1][1]-vec1[1]*mat1[0][1];
	float y1=mat1[0][0]*vec1[1]-mat1[1][0]*vec1[0];

	float det2=mat2[0][0]*mat2[1][1]-mat2[0][1]*mat2[1][0];
	float x2=vec2[0]*mat2[1][1]-vec2[1]*mat2[0][1];
	float y2=mat2[0][0]*vec2[1]-mat2[1][0]*vec2[0];

	Vec3 pole1,pole2;
	if(det1 != 0)
		pole1 = Vec3(x1/det1, y1/det1, 1);
	else
		pole1 = Vec3(x1,y1,0);
	if(det2 != 0)
		pole2 = Vec3(x2/det2, y2/det2, 1);
	else
		pole2 = Vec3(x2,y2,0);

	Vec3  v00(vertin[0].x, vertin[0].y, 1);
	Vec3  normal=cross(pole1, pole2);

	for(int i=0;i<4;i++)
	{
		Vec3 tv=Vec3(vertin[i].x, vertin[i].y, 1);
		float zv=dot(tv,normal);
		float zn=dot(v00,normal);
		float nv1=dot(-v00, pole1);
		float nv2=dot(-v00, pole2);
		float v1=dot(tv, pole1)*zn/zv;
		float v2=dot(tv, pole2)*zn/zv;
		v1+=nv1;
		v2+=nv2;
		Vec3 num(v1,v2,0);
		Mat  den(pole1,pole2,normal);
		Vec3 s=num*den.trans().inv();
		Vec3 v=s+v00;
		vertout[i]=v;
	}
}

void CreateSphere(CMesh* pMesh, float r, int nSeg)
{
	pMesh->Release();
	pMesh->CreateMesh(2*nSeg*(nSeg-1)+2, 4*nSeg*(nSeg-1),VF_XYZ|VF_NORMAL);
	struct vertex
	{
		Vec3 pos;
		Vec3 normal;
	};
	vertex* pv=(vertex*)pMesh->m_vbuf;
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
	DWORD* ind=pMesh->m_ibuf;
	for(int i=0;i<2*nSeg;i++)
	{
		ind[3*i]=0;
		ind[3*i+1]=1+i;
		ind[3*i+2]=1+(i+1)%(2*nSeg);
	}
	for(int i=1;i<nSeg-1;i++)
	{
		for(int j=0;j<2*nSeg;j++)
		{
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6  ]=1+(i-1)*2*nSeg+j;
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+1]=1+(i  )*2*nSeg+j;
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+2]=1+(i  )*2*nSeg+(j+1)%(2*nSeg);
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+3]=1+(i  )*2*nSeg+(j+1)%(2*nSeg);
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+4]=1+(i-1)*2*nSeg+(j+1)%(2*nSeg);
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+5]=1+(i-1)*2*nSeg+j;
		}
	}
	for(int i=0;i<2*nSeg;i++)
	{
		ind[nSeg*6+(nSeg-2)*nSeg*12+i*3  ]=1+(nSeg-2)*nSeg*2+i;
		ind[nSeg*6+(nSeg-2)*nSeg*12+i*3+1]=1+(nSeg-2)*nSeg*2+(i+1)%(2*nSeg);
		ind[nSeg*6+(nSeg-2)*nSeg*12+i*3+2]=1+(nSeg-1)*nSeg*2;
	}
}

void CreateDome(CMesh* pMesh, float w, float h, float z, int nSeg)
{
	pMesh->Release();
	pMesh->CreateMesh(2*nSeg*nSeg+1, 4*nSeg*(nSeg-1)+2*nSeg,VF_XYZ|VF_NORMAL|VF_TEXCOORD);
	float sumab=(h/2)*(h/2)/z;
	float r=(sumab+z)/2;
	float zbias=(sumab-z)/2;
	float angle=atan((h/2)/zbias);
	struct vertex
	{
		Vec3 pos;
		Vec3 normal;
		Vec2 tex;
	};
	vertex* pv=(vertex*)pMesh->m_vbuf;
	pv[0].pos=Vec3(0,0,-z);
	pv[0].normal=Vec3(0,0,-1);
	pv[0].tex=Vec2(0.5,0.5);
	for(int i=1;i<nSeg+1;i++)
	{
		for(int j=0;j<2*nSeg;j++)
		{
			float vangle=angle*i/nSeg;
			float hangle=PI/nSeg*j;
			Vec3 vec(sin(vangle)*sin(hangle),sin(vangle)*cos(hangle),-cos(vangle));
			pv[1+(i-1)*2*nSeg+j].pos=Vec3(0,0,zbias)+vec*r;
			pv[1+(i-1)*2*nSeg+j].pos.x*=w/h;
			pv[1+(i-1)*2*nSeg+j].normal=vec;
			pv[1+(i-1)*2*nSeg+j].normal.x*=h/w;
			pv[1+(i-1)*2*nSeg+j].normal=normalize(pv[1+(i-1)*2*nSeg+j].normal);
			pv[1+(i-1)*2*nSeg+j].tex=Vec2(pv[1+(i-1)*2*nSeg+j].pos.x/w+0.5,-pv[1+(i-1)*2*nSeg+j].pos.y/h+0.5);
		}
	}
	DWORD* ind=pMesh->m_ibuf;
	for(int i=0;i<2*nSeg;i++)
	{
		ind[3*i]=0;
		ind[3*i+1]=1+i;
		ind[3*i+2]=1+(i+1)%(2*nSeg);
	}
	for(int i=1;i<nSeg;i++)
	{
		for(int j=0;j<2*nSeg;j++)
		{
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6  ]=1+(i-1)*2*nSeg+j;
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+1]=1+(i  )*2*nSeg+j;
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+2]=1+(i  )*2*nSeg+(j+1)%(2*nSeg);
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+3]=1+(i  )*2*nSeg+(j+1)%(2*nSeg);
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+4]=1+(i-1)*2*nSeg+(j+1)%(2*nSeg);
			ind[nSeg*6+(i-1)*2*nSeg*6+j*6+5]=1+(i-1)*2*nSeg+j;
		}
	}
}

void CreateCylinder(CMesh* pMesh, float r, float h, int rSeg, int hSeg)
{
	pMesh->Release();
	pMesh->CreateMesh((rSeg+1)*(hSeg+1), rSeg*hSeg*2, VF_XYZ|VF_NORMAL|VF_TEXCOORD);

	struct vertex
	{
		Vec3 pos;
		Vec3 normal;
		Vec2 tex;
	};
	vertex* pv=(vertex*)pMesh->m_vbuf;

	for(int i=0;i<hSeg+1;i++)
	{
		for(int j=0;j<rSeg+1;j++)
		{
			float hangle=2*PI/rSeg*j;
			Vec3 vec(sin(hangle),0,cos(hangle));
			pv[i*(rSeg+1)+j].pos=Vec3(vec.x*r,-h/2+h/hSeg*i,vec.z*r);
			pv[i*(rSeg+1)+j].normal=vec;
			pv[i*(rSeg+1)+j].tex = Vec2(1./rSeg*j,1-1./hSeg*i);
		}
	}

	DWORD* ind=pMesh->m_ibuf;
	for(int i=0;i<hSeg;i++)
	{
		for(int j=0;j<rSeg;j++)
		{
			ind[(i*rSeg+j)*6  ]=i*(rSeg+1)+j;
			ind[(i*rSeg+j)*6+1]=(i+1)*(rSeg+1)+j;
			ind[(i*rSeg+j)*6+2]=(i+1)*(rSeg+1)+j+1;
			ind[(i*rSeg+j)*6+3]=(i+1)*(rSeg+1)+j+1;
			ind[(i*rSeg+j)*6+4]=i*(rSeg+1)+j+1;
			ind[(i*rSeg+j)*6+5]=i*(rSeg+1)+j;			
		}
	}
}

void CreateWave(CMesh* pMesh, float w, float h,float waveh, float wlen, int wSeg, int hSeg)
{
	pMesh->Release();
	pMesh->CreateMesh((wSeg+1)*(hSeg+1),wSeg*hSeg*2,VF_XYZ|VF_NORMAL);

	struct vertex
	{
		Vec3 pos;
		Vec3 normal;
	};

	vertex* pv=(vertex*)pMesh->m_vbuf;

	for(int i=0;i<wSeg+1;i++)
	{
		for(int j=0;j<hSeg+1;j++)
		{
			float fw=w/wSeg*i-w/2;
			float fh=h/hSeg*j-h/2;
			float fac=(fw*fw+fh*fh)/wlen/wlen;
			float wh=waveh*sin(fac)/fac;
			if(fac==0)
				wh=waveh;
			Vec2  dh(fw,fh);
			dh*=sin(fac)*waveh;
			pv[i*(hSeg+1)+j].pos=Vec3(fw,wh,fh);
			pv[i*(hSeg+1)+j].normal=normalize(Vec3(dh.x, 1, dh.y));
		}
	}
	
	DWORD* ind=pMesh->m_ibuf;
	for(int i=0;i<wSeg;i++)
	{
		for(int j=0;j<hSeg;j++)
		{
			ind[(i*hSeg+j)*6  ]=    i*(hSeg+1)+j;
			ind[(i*hSeg+j)*6+1]=    i*(hSeg+1)+j+1;
			ind[(i*hSeg+j)*6+2]=(i+1)*(hSeg+1)+j+1;
			ind[(i*hSeg+j)*6+3]=(i+1)*(hSeg+1)+j+1;
			ind[(i*hSeg+j)*6+4]=(i+1)*(hSeg+1)+j;
			ind[(i*hSeg+j)*6+5]=    i*(hSeg+1)+j;			
		}
	}
}

void CreateSwirl(CMesh* pMesh, float w, float h, float waveh, float nw, float spiral, int wSeg, int hSeg)
{
	pMesh->Release();
	pMesh->CreateMesh((wSeg+1)*(hSeg+1), 2*wSeg*hSeg, VF_XYZ|VF_NORMAL);

	struct vertex
	{
		Vec3 pos;
		Vec3 normal;
	};

	vertex* pv=(vertex*)pMesh->m_vbuf;

	for(int i=0;i<wSeg+1;i++)
	{
		for(int j=0;j<hSeg+1;j++)
		{
			float fw=(float)i/wSeg*w-w/2;
			float fh=(float)j/hSeg*h-h/2;

			float r=sqrt(fw*fw+fh*fh);
			Vec2 tangle(-fh,fw);
			Vec2 pole(fw,fh);
			float angle=acos(fw/r);
			if(fh<0)
				angle=-angle;
			angle+=spiral*r;
			float h=cos(nw*angle);
			float hp=(1-1./(r*r/64+1))*waveh;;
			tangle=normalize(tangle)*sin(nw*angle);
			tangle*=hp;
			pole=-normalize(pole)/(r*r/64+1)/(r*r/64+1)*r/4;
			pole*=h;
			Vec2 norm=tangle+pole;
			if(r==0)
			{
				norm=Vec2(0,0);
				h=hp=0;
			}
			Vec3 normal=Vec3(norm.x, 1, norm.y);
			normal=normal.normalize();
			//h*=(1-1./(r*r/64+1))*waveh;
			pv[i*(hSeg+1)+j].pos=Vec3(fw,h*hp,fh);
			pv[i*(hSeg+1)+j].normal=normal;
		}
	}

	DWORD* ind=pMesh->m_ibuf;
	for(int i=0;i<wSeg;i++)
	{
		for(int j=0;j<hSeg;j++)
		{
			ind[(i*hSeg+j)*6  ]=    i*(hSeg+1)+j;
			ind[(i*hSeg+j)*6+1]=    i*(hSeg+1)+j+1;
			ind[(i*hSeg+j)*6+2]=(i+1)*(hSeg+1)+j+1;
			ind[(i*hSeg+j)*6+3]=(i+1)*(hSeg+1)+j+1;
			ind[(i*hSeg+j)*6+4]=(i+1)*(hSeg+1)+j;
			ind[(i*hSeg+j)*6+5]=    i*(hSeg+1)+j;			
		}
	}
}

void CreateTurus(CMesh* pMesh, float rin, float rout, int rSeg, int cSeg)
{
	pMesh->Release();
	pMesh->CreateMesh((rSeg+1)*(cSeg+1), rSeg*cSeg*2, VF_XYZ|VF_NORMAL|VF_TEXCOORD);

	float rc=(rin+rout)/2;
	float ri=(rout-rin)/2;
	struct vertex
	{
		Vec3 pos;
		Vec3 normal;
		Vec2 tex;
	};

	vertex* pv=(vertex*)pMesh->m_vbuf;

	for(int i=0;i<cSeg+1;i++)
	{
		float c=i*2*PI/cSeg;
		for(int j=0;j<rSeg+1;j++)
		{
			float angle=j*2*PI/rSeg;
			Vec3 center(rc*cos(c), 0, rc*sin(c));
			Vec3 dr(cos(angle)*cos(c), sin(angle), cos(angle)*sin(c));
			pv[i*(rSeg+1)+j].pos=center+ri*dr;
			pv[i*(rSeg+1)+j].normal=dr;
			pv[i*(rSeg+1)+j].tex=Vec2((float)i/cSeg, (float)j/rSeg);
		}
	}
	DWORD* ind=pMesh->m_ibuf;
	for(int i=0;i<cSeg;i++)
	{
		for(int j=0;j<rSeg;j++)
		{
			ind[(i*rSeg+j)*6  ]=    i*(rSeg+1)+j;
			ind[(i*rSeg+j)*6+1]=    i*(rSeg+1)+j+1;
			ind[(i*rSeg+j)*6+2]=(i+1)*(rSeg+1)+j+1;
			ind[(i*rSeg+j)*6+3]=(i+1)*(rSeg+1)+j+1;
			ind[(i*rSeg+j)*6+4]=(i+1)*(rSeg+1)+j;
			ind[(i*rSeg+j)*6+5]=    i*(rSeg+1)+j;			
		}
	}
}