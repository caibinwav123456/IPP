
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "TSGenerator.h"
#include "ChildView.h"
#include "Image.h"
#include "allheaders.h"
#include "DlgParams.h"
#include "DlgProgress.h"
#include FT_GLYPH_H

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView():m_library(NULL),m_face(NULL),m_fontsize(100),m_org(20,20),m_buf(NULL),m_bOrigin(false),m_LineSpace(20),m_CharSpace(20),m_Index(0),m_ptr(NULL),m_bShowRowStats(false),m_bSaveRowStats(false)
{
	m_szImg=cvSize(2048,2048);
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_LOADFACE, &CChildView::OnFileLoadface)
	ON_COMMAND(ID_FILE_LOADTEXT, &CChildView::OnFileLoadtext)
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_FILE_SAVE2, &CChildView::OnFileSave2)
	ON_COMMAND(ID_FILE_SETPARAMS, &CChildView::OnFileSetparams)
	ON_COMMAND(ID_FILE_BATCHPROCESS, &CChildView::OnFileBatchprocess)
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
	Draw();
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
}

void CChildView::Draw()
{
	CRect rcClient;
	GetClientRect(rcClient);
	CClientDC dc(this);
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
	CBitmap* oldbmp = dcMem.SelectObject(&bmp);
	Draw(&dcMem);
	dc.BitBlt(0,0,rcClient.Width(),rcClient.Height(),&dcMem,0,0,SRCCOPY);
	dcMem.SelectObject(oldbmp);
	dcMem.DeleteDC();
	bmp.DeleteObject();
}

void CChildView::Draw(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(rcClient);
	pDC->FillSolidRect(rcClient, RGB(255,255,255));
	if(m_Image.size()==0)
		return;
	IplImage* image;
	ASSERT(m_Image.size()==m_vBox.size());
	if(m_Index>=0&&m_Index<(int)m_Image.size())
		image=m_Image[m_Index];
	if(!image)
		return;

	float scale=1;
	IplImage* disp=NULL;
	if(!m_bOrigin)
	{
		float xscale=(float)rcClient.Width()/image->width;
		float yscale=(float)rcClient.Height()/image->height;
		scale=min(xscale, yscale);
		int width=cvRound(image->width*scale);
		int height=cvRound(image->height*scale);
		width=max(1,width);
		height=max(1,height);
		disp=cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
		cvResize(image, disp);
	}
	DispImage(pDC, m_bOrigin?image:disp, CPoint(0,0));
	cvReleaseImage(&disp);
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(255,0,0));
	CPen* oldpen=pDC->SelectObject(&pen);
	pDC->SelectStockObject(NULL_BRUSH);
	for(int i=0;i<(int)m_vBox[m_Index].size();i++)
	{
		CRect rc=m_vBox[m_Index][i];
		rc.left*=scale;
		rc.top*=scale;
		rc.right*=scale;
		rc.bottom*=scale;
		pDC->Rectangle(rc);
	}
	if(m_bShowRowStats)
	{
		CPen bpen;
		bpen.CreatePen(PS_SOLID, 1, RGB(0,0,255));
		pDC->SelectObject(&bpen);
		for(int i=0;i<(int)m_RowStats[m_Index].size();i++)
		{
			RowStats stats=m_RowStats[m_Index][i];
			pDC->MoveTo(m_org.x*scale, stats.baseliney*scale);
			pDC->LineTo((m_szImg.width-m_org.x)*scale, stats.baseliney*scale);
		}
		pDC->SelectObject(oldpen);
		bpen.DeleteObject();
	}
	pDC->SelectObject(oldpen);
	pen.DeleteObject();
}

void CChildView::DispImage(CDC* pDC, IplImage* image, CPoint ptBase)
{
	ASSERT(image->depth==IPL_DEPTH_8U && image->nChannels == 3);

	BITMAPINFO bi;
	ZeroMemory(&bi,sizeof(bi));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = image->width;
	bi.bmiHeader.biHeight = -image->height;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 24;
	bi.bmiHeader.biCompression = BI_RGB;

	SetDIBitsToDevice(pDC->m_hDC, ptBase.x, ptBase.y, image->width, image->height,
		0, 0, 0, image->height, (void*)(image->imageData),&bi, DIB_RGB_COLORS);
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	FT_Error error=FT_Init_FreeType(&m_library);
	if(error)
	{
		MessageBox(_T("Init failed"));
	}
	return 0;
}

void CChildView::OnDestroy()
{
	CWnd::OnDestroy();
	for(int i=0;i<(int)m_Image.size();i++)
		cvReleaseImage(&m_Image[i]);
	m_Image.clear();
	if(m_face)
	{
		FT_Done_Face(m_face);
		m_face=NULL;
	}
	FT_Done_FreeType(m_library);
	if(m_buf)
	{
		delete[] m_buf;
		m_buf = NULL;
	}
	// TODO: Add your message handler code here
}

void CChildView::OnFileLoadface()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	if(!m_library)
		return;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("All Files|*.*||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strPath=dlg.GetPathName();
		if(!LoadFace(strPath))
			MessageBox(_T("Load Face Failed"));
		Draw();
	}
}

bool CChildView::LoadFace(CString strPath, bool bFullRender)
{
	USES_CONVERSION;
	if(m_face)
	{
		FT_Done_Face(m_face);
		m_face=NULL;
	}
	FT_Error error=FT_New_Face(m_library, T2A(strPath), 0, &m_face);
	if(error)
	{
		return false;
	}
	Reset();
	if(!bFullRender)
		Render(0);
	else
	{
		int index=0;
		while(Render(index))
			index++;
	}
	return true;
}

bool CChildView::Render(int n)
{
	if(n>(int)m_Image.size())
		return false;
	else if(n<(int)m_Image.size())
		return true;
	if(!m_ptr||!*m_ptr)
		return false;
	if(!m_buf)
		return false;
	if(!m_library||!m_face)
		return false;
	FT_Error error=FT_Set_Pixel_Sizes(m_face, 0, m_fontsize);
	if(error)
	{
		MessageBox(_T("Set size failed"));
		return false;
	}
	if(m_face->charmap==NULL)
	{
		MessageBox(_T("Font does not contain unicode charmaps"));
		return false;
	}
	IplImage* image=cvCreateImage(m_szImg, IPL_DEPTH_8U, 3);
	m_Image.push_back(image);
	m_vBox.push_back(vector<CRect>());
	vector<CRect>& vbox=m_vBox[m_vBox.size()-1];
	m_vChar.push_back(vector<WCHAR>());
	vector<WCHAR>& vchar=m_vChar[m_vChar.size()-1];
	m_RowStats.push_back(vector<RowStats>());
	vector<RowStats>& rowstats=m_RowStats[m_RowStats.size()-1];
	cvSet(image, cvScalarAll(255));
	FT_GlyphSlot slot=m_face->glyph;
	CPoint pen(m_org.x,m_org.y+m_fontsize);
	RowStats stats;
	stats.baseliney=pen.y;
	rowstats.push_back(stats);
	for(;*m_ptr!=0;m_ptr++)
	{
		FT_UInt index=FT_Get_Char_Index(m_face, *m_ptr);
		error=FT_Load_Glyph(m_face, index, FT_LOAD_DEFAULT);
		if(error)
			continue;
		FT_Glyph glyph;
		FT_Get_Glyph(slot, &glyph);
		FT_BBox box;
		FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_GRIDFIT, &box);
		FT_Done_Glyph(glyph);
		//if(slot->format==FT_GLYPH_FORMAT_OUTLINE)
		//	slot->outline.flags = FT_OUTLINE_EVEN_ODD_FILL;
		error=FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL);
		if(error)
			continue;
		if(DrawBitmap(&slot->bitmap, pen.x+slot->bitmap_left, pen.y-slot->bitmap_top))
		{
			CRect rect;
			rect.left=(box.xMin>>6)+pen.x;
			rect.right=(box.xMax>>6)+pen.x;
			rect.top=pen.y-(box.yMax>>6);
			rect.bottom=pen.y-(box.yMin>>6);
			vbox.push_back(rect);
			vchar.push_back(*m_ptr);
		}
		pen.x+=slot->advance.x>>6;
		pen.y+=slot->advance.y>>6;
		pen.x+=m_CharSpace;
		if(pen.x>m_szImg.width-m_org.x-m_fontsize)
		{
			pen.x=m_org.x;
			pen.y+=m_fontsize+m_LineSpace;
			if(pen.y>m_szImg.height-m_org.y)
				break;
			RowStats stats;
			stats.baseliney=pen.y;
			rowstats.push_back(stats);
		}
	}
	return true;
}

bool CChildView::DrawBitmap(FT_Bitmap* bmp, int left, int top)
{
	if(m_Image.size()==0||!bmp||!bmp->width||!bmp->rows||!bmp->pitch||!bmp->buffer)
		return false;
	ASSERT(bmp->pixel_mode==FT_PIXEL_MODE_GRAY);
	if(bmp->pixel_mode!=FT_PIXEL_MODE_GRAY)
		return false;
	IplImage* image=m_Image[m_Image.size()-1];
	for(int i=0;i<bmp->rows;i++)
	{
		int y=i+top;
		if(y<0||y>=image->height)
			continue;
		for(int j=0;j<bmp->width;j++)
		{
			int x=j+left;
			if(x<0||x>=image->width)
				continue;
			uchar* pixd=PTR_PIX(*image, x,y);
			uchar* pixs=(uchar*)bmp->buffer+i*bmp->pitch+j;
			float sc=(float)*pixs;
			float c=(float)*pixd*(255-sc)/255+(255-sc)*sc/255;
			*pixd=max(0,min(255,c));
			*(pixd+1)=max(0,min(255,c));
			*(pixd+2)=max(0,min(255,c));
		}
	}
	return true;
}

void CChildView::OnFileLoadtext()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	if(!m_library)
		return;
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("All Files|*.*||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strPath=dlg.GetPathName();
		CFile file;
		if(file.Open(strPath, CFile::modeRead))
		{
			int n=file.GetLength();
			WCHAR prefix=0;
			file.Read(&prefix, 2);
			if(prefix!=0xFEFF)
			{
				MessageBox(_T("txt file is not of unicode format"));
				file.Close();
				return;
			}
			n-=2;
			if(m_buf)
			{
				delete[] m_buf;
				m_buf = NULL;
			}
			m_buf = new WCHAR[n/2+1];
			file.Read(m_buf, n);
			m_buf[n/2]=0;
			file.Close();
			Reset();
			Render(0);
			Draw();
		}
	}
}

void CChildView::Reset()
{
	for(int i=0;i<(int)m_Image.size();i++)
		cvReleaseImage(&m_Image[i]);
	m_Image.clear();
	m_vBox.clear();
	m_vChar.clear();
	m_RowStats.clear();
	m_ptr=m_buf;
	m_Index=0;
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	switch(nChar)
	{
	case 'Z':
		m_bOrigin=!m_bOrigin;
		Draw();
		break;
	case 'X':
		m_bShowRowStats=!m_bShowRowStats;
		Draw();
		break;
	case VK_UP:
		PageShift(-1);
		Draw();
		break;
	case VK_DOWN:
		PageShift(1);
		Draw();
		break;
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChildView::PageShift(int dir)
{
	if(dir>0)
	{
		m_Index++;
	}
	else if(dir<0)
	{
		m_Index--;
		if(m_Index<0)
			m_Index=0;
	}
	if(!Render(m_Index))
	{
		m_Index--;
		if(m_Index<0)
			m_Index=0;
	}
}

BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	PageShift(-zDelta);
	Draw();
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CChildView::OnFileSave2()
{
	// TODO: Add your command handler code here
	if(m_library==NULL||m_face==NULL||m_buf==NULL)
	{
		MessageBox(_T("No saving data available"));
		return;
	}
	CFileDialog dlg(FALSE, _T(".tiff"), _T("output"), 0, _T("Tiff Files|*.tiff||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strPath=dlg.GetPathName();
		if(Save(strPath))
			MessageBox(_T("Save succeeded"));
		else
			MessageBox(_T("Save Failed"));
	}
}

bool CChildView::Save(CString strPath)
{
	int index=0;
	while(Render(index))
		index++;
	CString strBox=strPath;
	int iDot=strPath.ReverseFind(_T('.'));
	if(iDot!=-1)
		strBox=strPath.Left(iDot);
	strBox+=_T(".box");
	return SaveOutput(strPath, strBox);
}

#pragma warning(disable:4996)
bool CChildView::SaveOutput(CString strImgFile, CString strBoxFile)
{
	USES_CONVERSION;
	ASSERT(m_Image.size()==m_vBox.size()&&m_Image.size()==m_vChar.size());
	bool bfirst=true;
	for(int n=0;n<(int)m_Image.size();n++)
	{
		IplImage* image=m_Image[n];
		Pix* pix=pixCreate(image->width, image->height, 24);
		for(int i=0;i<image->height;i++)
		for(int j=0;j<image->width;j++)
		{
			uchar* pixs=PTR_PIX(*image, j, i);
			uchar* pixd=(uchar*)pix->data+pix->wpl*i*4+j*3;
			memcpy(pixd, pixs, 3);
		}
		if(pixWriteTiff(T2A(strImgFile), pix, IFF_TIFF_ZIP, bfirst ? "w" : "a"))
		{
			pixDestroy(&pix);
			return false;
		}
		pixDestroy(&pix);
		bfirst=false;
	}
	CFile file;
	if(!file.Open(strBoxFile, CFile::modeCreate|CFile::modeWrite))
		return false;
	WCHAR buf[1024];
	for(int i=0;i<(int)m_vBox.size();i++)
	{
		ASSERT(m_vBox[i].size()==m_vChar[i].size());
		for(int j=0;j<(int)m_vBox[i].size();j++)
		{
			CRect rc=m_vBox[i][j];
			wsprintf(buf, L"%c %d %d %d %d %d", m_vChar[i][j], rc.left, m_szImg.height-rc.bottom, rc.right, m_szImg.height-rc.top, i);
			int lstr = lstrlenW(buf)+2;
			char* sbuf = (char*)alloca(lstr*sizeof(WCHAR));
			*sbuf = 0;
			int nRet = WideCharToMultiByte(CP_UTF8, 0, buf, -1, sbuf, lstr*sizeof(WCHAR), NULL, FALSE);
			strcat(sbuf, "\12");
			ASSERT(nRet>0);
			file.Write(sbuf, strlen(sbuf));
		}
	}
	file.Close();
	if(m_bSaveRowStats)
	{
		CFile file;
		CString strRowFile=strBoxFile+_T(".baseline");
		if(!file.Open(strRowFile, CFile::modeCreate|CFile::modeWrite))
			return false;
		for(int i=0;i<(int)m_RowStats.size();i++)
		{
			for(int j=0;j<(int)m_RowStats[i].size();j++)
			{
				wsprintf(buf, L"%d %d", m_szImg.height-m_RowStats[i][j].baseliney, i);
				int lstr = lstrlenW(buf)+2;
				char* sbuf = (char*)alloca(lstr*sizeof(WCHAR));
				*sbuf = 0;
				int nRet = WideCharToMultiByte(CP_UTF8, 0, buf, -1, sbuf, lstr*sizeof(WCHAR), NULL, FALSE);
				strcat(sbuf, "\12");
				ASSERT(nRet>0);
				file.Write(sbuf, strlen(sbuf));
			}
		}
		file.Close();
	}
	return true;
}

void CChildView::OnFileSetparams()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CDlgParams dlg;
	dlg.m_strCharSpace.Format(_T("%d"), m_CharSpace);
	dlg.m_strLineSpace.Format(_T("%d"), m_LineSpace);
	dlg.m_strHMargin.Format(_T("%d"), m_org.x);
	dlg.m_strVMargin.Format(_T("%d"), m_org.y);
	dlg.m_bOutputRowStats=m_bSaveRowStats;
	if(dlg.DoModal()==IDOK)
	{
		m_CharSpace=atoi(T2A(dlg.m_strCharSpace));
		m_LineSpace=atoi(T2A(dlg.m_strLineSpace));
		m_org.x=atoi(T2A(dlg.m_strHMargin));
		m_org.y=atoi(T2A(dlg.m_strVMargin));
		m_bSaveRowStats=!!dlg.m_bOutputRowStats;
		if(m_CharSpace<0)
			m_CharSpace=0;
		if(m_LineSpace<0)
			m_LineSpace=0;
		if(m_org.x<0)
			m_org.x=0;
		if(m_org.y<0)
			m_org.y=0;
		Reset();
		Render(0);
		Draw();
	}
}

void CChildView::OnFileBatchprocess()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	if(!m_buf||!*m_buf)
	{
		MessageBox(_T("No text loaded!"));
		return;
	}
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Text Files|*.txt||"));
	if(dlg.DoModal() == IDOK)
	{
		CString strPath=dlg.GetPathName();
		DlgProgress prog;
		prog.m_View=this;
		prog.m_BatchFile=strPath;
		prog.DoModal();
	}
}
