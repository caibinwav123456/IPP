#include<windows.h>
#include<math.h>
#include "app.h"
#include <mmsystem.h>
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE PreInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{
	HWND hwnd;
	MSG msg;
	char lpszClassName[]="´°¿Ú";//
	WNDCLASS wc;
	wc.style=0;//CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc=WndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=hInstance;
	wc.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName=NULL;
	wc.lpszClassName=lpszClassName;

	RegisterClass(&wc);

	hwnd=CreateWindow(lpszClassName,
		              "windows",
					  WS_OVERLAPPEDWINDOW,
					  200,100,640,480,//200,100,640,480,
					  NULL,
					  NULL,
					  hInstance,
					  NULL);

	ShowWindow(hwnd,nCmdShow);
	UpdateWindow(hwnd);

	if(!InitD3D(1024,768,hwnd))
	{
		MessageBox(0,"init failed",0,0);
		return 0;
	}
	if(!Setup())
	{
		MessageBox(0,"setup failed",0,0);
		return 0;
	}
	float lTime=(float)timeGetTime();
	ZeroMemory(&msg,sizeof(MSG));
	while(msg.message!=WM_QUIT)
	{
		if(PeekMessage(&msg,0,0,0,PM_REMOVE))
		{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		}
		else 
		{
			float cTime=(float)timeGetTime();
			float tdelta=(cTime-lTime)*0.001f;
			Display(tdelta);
			lTime=cTime;
		}
	}
/*	while(GetMessage(&msg,0,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
*/	Cleanup();
	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd,
						 UINT message,
						 WPARAM wParam,
						 LPARAM lParam)
{
	int k;
	int width,height;
	switch(message)
	{
	case WM_LBUTTONDOWN: onmdown(hwnd);break;
	case WM_LBUTTONUP:   onmup(hwnd);break;
	case WM_RBUTTONDOWN: onrdown(hwnd);break;
	case WM_RBUTTONUP: onrup(hwnd);break;
	case WM_MOUSEMOVE: onmove(hwnd);break;
	case WM_KEYDOWN: //if(wParam==VK_ESCAPE)::DestroyWindow(hwnd);
		switch(wParam)
		{
		case  VK_LEFT:k=1;break;
		case      'A':k=13;break;
		case VK_RIGHT:k=2;break;
		case      'D':k=14;break;
		case    VK_UP:k=3;break;
		case      'W':k=15;break;
		case  VK_DOWN:k=4;break;
		case      'S':k=16;break;
		case      'Q':k=5;break;
		case      'E':k=6;break;
		case      'Z':k=7;break;
		case      'X':k=8;break;
		case      'K':k=9;break;
		case      'L':k=10;break;
		case    VK_F1:k=17;break;
		case VK_SPACE:
		case VK_RCONTROL:k=18;break;
/*		case VK_SPACE:k=20;break;
		case      'N':k=21;break;
		case      'H':k=22;break;
		case VK_SHIFT:k=11;break;
		case VK_ESCAPE:k=12;break;
*/		default      :k=0;break;  
		}
		onkey(k,true,0);
		break;
	case WM_KEYUP:		
		switch(wParam)
		{
		case  VK_LEFT:k=1;break;
		case      'A':k=13;break;
		case VK_RIGHT:k=2;break;
		case      'D':k=14;break;
		case    VK_UP:k=3;break;
		case      'W':k=15;break;
		case  VK_DOWN:k=4;break;
		case      'S':k=16;break;
		case      'Q':k=5;break;
		case      'E':k=6;break;
		case      'Z':k=7;break;
		case      'X':k=8;break;
		case      'K':k=9;break;
		case      'L':k=10;break;
		case    VK_F1:k=17;break;
		case VK_SPACE:
		case VK_RCONTROL:k=18;break;
/*		case VK_SPACE:k=20;break;
		case      'N':k=21;break;
		case      'H':k=22;break;
		case VK_SHIFT:k=11;break;
		case VK_ESCAPE:k=12;break;
*/		default      :k=0;break;  
		}
		onkey(k,false,0);
		break;
	case WM_SIZE:
			width = LOWORD(lParam);
			height = HIWORD(lParam);
			break;
	case WM_DESTROY: PostQuitMessage(0);break;
	default: return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;
}



