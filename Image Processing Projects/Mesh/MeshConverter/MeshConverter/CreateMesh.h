#ifndef _CREATE_MESH_H_
#define _CREATE_MESH_H_
#include "Mesh.h"

void CreateMeshFile(IDirect3DDevice9* device, char* xfile, char* meshfile, int indexFormat = IF_INDEX16);

#endif