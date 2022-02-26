
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "giftest.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef unsigned char uchar;
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


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	GifFileType* infile = DGifOpenFileName("dogkid.gif");
	GifFileType* outfile = EGifOpenFileName("dogkid2.gif", FALSE);
	//char* strComment;
	//DGifGetComment(infile, strComment);
	EGifSetGifVersion("89a");
	DGifSlurp(infile);
	EGifPutScreenDesc(outfile, infile->SWidth, infile->SHeight, infile->SColorResolution, infile->SBackGroundColor, infile->SColorMap);

	for(int i=0;i<infile->ImageCount;i++)
	{
		SavedImage* image = infile->SavedImages+i;
		if(image->ExtensionBlockCount == 1)
		{}//	EGifPutExtension(outfile, image->ExtensionBlocks->Function, image->ExtensionBlocks->ByteCount, image->ExtensionBlocks->Bytes);
		else
		for(int j=0;j<image->ExtensionBlockCount;j++)
		{
			ExtensionBlock* block = image->ExtensionBlocks + j;
			if(j==0)
				EGifPutExtensionFirst(outfile, block->Function, block->ByteCount, block->Bytes);
			else if(j==image->ExtensionBlockCount-1)
			{
				//EGifPutExtension/*Next*/(outfile, block->Function, block->ByteCount, block->Bytes);
				//EGifPutExtensionLast(outfile, block->Function, 0, NULL);
			}
			else
				EGifPutExtensionLast(outfile, block->Function, block->ByteCount, block->Bytes);
		}
		EGifPutImageDesc(outfile, image->ImageDesc.Left, image->ImageDesc.Top, image->ImageDesc.Width, image->ImageDesc.Height, image->ImageDesc.Interlace, image->ImageDesc.ColorMap);
		EGifPutLine(outfile, image->RasterBits, image->ImageDesc.Width*image->ImageDesc.Height);
	}
	DGifCloseFile(infile);
	EGifCloseFile(outfile);
	GifColorType* colormap = new GifColorType[256];
	for(int i=0;i<256;i++)
	{
		colormap[i].Red = 0;
		colormap[i].Green = 0;
		colormap[i].Blue = i;
	}
	ColorMapObject* cmap = MakeMapObject(256, colormap);
	GifFileType* file = EGifOpenFileName("test.gif", FALSE);
	EGifSetGifVersion("89a");
	if(GIF_ERROR == EGifPutScreenDesc(file, 400, 400, 7, 0, cmap))
	{
		int o=0;
		o++;
	}
	uchar* pix = new uchar[400*400];
	for(int i=0;i<30;i++)
	{
		if(i==0)
		{
			char* appname = "NETSCAPE2.0";
			uchar appdata[3] = {1,0,0};
			EGifPutExtensionFirst(file, APPLICATION_EXT_FUNC_CODE, strlen(appname), appname);
			EGifPutExtensionLast(file, 0, 3, appdata);
		}
		if(GIF_ERROR == EGifPutImageDesc(file, 0,0,400,400,0,NULL))
		{
			int o=0;
			o++;
		}
		for(int j=0;j<400;j++)
		{
			for(int k=0;k<400;k++)
			{
				if(j>i*10&&j<i*10+100&&k>i*10&&k<i*10+100)
				{
					int dist[4];
					dist[0] = abs(j-i*10);
					dist[1] = abs(j-i*10-100);
					dist[2] = abs(k-i*10);
					dist[3] = abs(k-i*10-100);
					int dmin = 400;
					for(int m=0;m<4;m++)
					{
						if(dmin>dist[m])
							dmin = dist[m];
					}
					pix[j*400+k] = dmin*4;
				}
				else
				{
					pix[j*400+k] = 255;
				}
			}
		}
		if(GIF_ERROR == EGifPutLine(file, pix, 400*400))
		{
			int o=0;
			o++;
		}
	}
	EGifCloseFile(file);
	delete[] pix;
	delete[] colormap;

	return 0;
}
