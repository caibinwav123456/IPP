#ifndef _3D_UTIL_H_
#define _3D_UTIL_H_

#include "matutil.h"
#include "Mesh.h"

#define PI 3.1415926535897932384626433832795028841971

void Reform(Vec2 vertin[4], Vec3 vertout[4]);
void CreateSphere(CMesh* pMesh, float r, int nSeg);
void CreateCylinder(CMesh* pMesh, float r, float h, int rSeg, int hSeg);
void CreateWave(CMesh* pMesh, float w, float h,float waveh, float wlen, int wSeg, int hSeg);
void CreateSwirl(CMesh* pMesh, float w, float h,float waveh, float nw, float spiral, int wSeg, int hSeg);
void CreateTurus(CMesh* pMesh, float rin, float rout, int rSeg, int cSeg);
void CreateDome(CMesh* pMesh, float w, float h, float angle, int nSeg);

#endif