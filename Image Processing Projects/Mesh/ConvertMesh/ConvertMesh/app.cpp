#include "stdafx.h"
#include "app.h"
#include "CreateMesh.h"

IDirect3DDevice9* device = NULL;
bool Setup()
{
	CreateMeshFile(device, "skullocc.x", "skull.mesh");
	return true;
}
void Cleanup()
{

}
bool Display(float tdelta)
{
	return true;
}
void onmdown(HWND hwnd)
{

}
void onmup(HWND hwnd)
{

}
void onrdown(HWND hwnd)
{

}
void onrup(HWND hwnd)
{

}
void onmove(HWND hwnd)
{

}
void onkey(int key,bool down,int type)
{

}
