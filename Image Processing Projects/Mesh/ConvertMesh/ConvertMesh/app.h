#ifndef _app_h
#define _app_h
#include "d3d9.h"
#include "d3dx9.h"
#include "d3dfont.h"

extern IDirect3DDevice9* device;

bool InitD3D(int w,int h,HWND hwnd);
bool Setup();
void Cleanup();
bool Display(float tdelta);
void onmdown(HWND hwnd);
void onmup(HWND hwnd);
void onrdown(HWND hwnd);
void onrup(HWND hwnd);
void onmove(HWND hwnd);
void onkey(int key,bool down,int type);

#endif
