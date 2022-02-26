
// stdafx.cpp : source file that includes just the standard includes
// MeshConverter.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
bool InitD3D(int w,int h,HWND hwnd, IDirect3DDevice9** ppDevice)
{
	IDirect3D9* d3d9;
	d3d9=Direct3DCreate9(D3D_SDK_VERSION);
	if(!d3d9)return 0;
	DWORD dwBehaviorFlags;

	dwBehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;


	DWORD dw;
	d3d9->CheckDeviceMultiSampleType(
					D3DADAPTER_DEFAULT,
					D3DDEVTYPE_HAL,
					D3DFMT_A8R8G8B8,
					TRUE,
					D3DMULTISAMPLE_4_SAMPLES,
					&dw
					);


	D3DPRESENT_PARAMETERS d3dpp;
	d3dpp.BackBufferWidth=w;
	d3dpp.BackBufferHeight=h;
	d3dpp.BackBufferFormat=D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount=1;
	d3dpp.MultiSampleType=D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality=0;
	d3dpp.SwapEffect=D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow=hwnd;
	d3dpp.Windowed=true;
	d3dpp.EnableAutoDepthStencil=true;
	d3dpp.AutoDepthStencilFormat=D3DFMT_D24S8;
	d3dpp.Flags=0;
	d3dpp.FullScreen_RefreshRateInHz=D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_DEFAULT;//D3DPRESENT_INTERVAL_IMMEDIATE;
//////////////////////////////////
/*	HRESULT hr=d3d9->CheckDepthStencilMatch(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		D3DFMT_X8R8G8B8,
		D3DFMT_A32B32G32R32F,
		D3DFMT_D24S8);
	if(FAILED(hr))
	{
		MessageBox(0,"not compatible",0,0);
		return 0;
	}
	else
	{
		MessageBox(0,"is compatible!","success",0);
	}
//////////////////////////////////*/
	//IDirect3DDevice9* device=0;
	HRESULT hr=d3d9->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hwnd,
		dwBehaviorFlags,//D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp,
		ppDevice);
	if(FAILED(hr))
	{
		MessageBox(0,"device failed",0,0);
		return 0;
	}
	return 1;
}


