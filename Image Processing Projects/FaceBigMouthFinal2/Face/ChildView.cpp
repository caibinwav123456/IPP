
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "Face.h"
#include "ChildView.h"
#include "shlobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define FACE_MORPH

// CChildView
Vec3 Pos(0,0,-3);
Mat rot(1,0,0,0,1,0,0,0,1);
Mat rotx,roty;
CString g_strTempl("Koala");
float FrameFunc(float x, float edge)
{
	if(x>=edge && x<1-edge)
	{
		return 1;
	}
	else
	{
		if(x<edge)
		{
			x-=edge;
		}
		else
		{
			x-=1-edge;
		}
		return (cos(x/edge*PI)+1)/2;
	}
}

CChildView::CChildView():m_oldPt(0,0)
{
	m_bShowVert = false;
	m_bShowImage = true;
	m_bDown = false;
	m_bMove = false;
	m_bAlign = false;
	m_bAnimate = false;
	m_bShowDetRegion = false;

	m_nPostureSel = 0;

	m_Image = NULL;
	m_imgObj = NULL;
	m_ImgBk = NULL;
	oldbmp = NULL;

	m_strTempl = "Koala.jpg";

	m_fProp = DEFAULT_FACE_DOME_ANGLE;
	m_fEdge = 0.2;

	TCHAR buf[MAX_PATH];
	SHGetSpecialFolderPath(NULL, buf, CSIDL_DESKTOP, TRUE);
	m_strWorkingPath = buf;

	m_fScale = 1.0;
	m_fWidth = 1.0;
	m_fHeight = 1.0;
	ZeroMemory(&m_guid, sizeof(GUID));
}

CChildView::~CChildView()
{
	cvReleaseImage(&m_ImgBk);
	cvReleaseImage(&m_Image);
	cvReleaseImage(&m_imgObj);
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_TIMER()
	ON_COMMAND(ID_FILE_OPEN, &CChildView::OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN_BK_IMG, &CChildView::OnFileOpenBkImg)
	ON_COMMAND(ID_FILE_SETTINGS, &CChildView::OnFileSettings)
	ON_MESSAGE(WM_NOTIFY_APPLY_SETTINGS, OnApplySettings)
	ON_MESSAGE(WM_NOTIFY_GENERATE_CONFIG_FILE, OnGenerateTempl)
	ON_MESSAGE(WM_NOTIFY_EXPORT_CONFIG_FILE, OnExportTempl)
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
	Draw();
	// Do not call CWnd::OnPaint() for painting messages
}


void CChildView::Draw()
{
	CClientDC dc(this);

	CRect rcClt;
	GetClientRect(rcClt);

#ifdef FACE_MORPH

	CString str,str2;
	str.Format("posture selected: %d", m_nPostureSel+1);
	if(m_Detector.GetPosture(m_nPostureSel)==0)
		str2.Format("posture state: disabled");
	else if(m_Detector.GetPosture(m_nPostureSel)==1)
		str2.Format("posture state: positive");
	else if(m_Detector.GetPosture(m_nPostureSel)==-1)
		str2.Format("posture state: negative");

	DispImage(&m_mdc, m_Detector.AnimateFace(), CPoint(0,0));

	if(m_bShowDetRegion)
	{
		CPen rPen;
		rPen.CreatePen(PS_SOLID, 1, RGB(255,0,0));
		CPen* oldPen = m_mdc.SelectObject(&rPen);
		DetectType DetType[3] = {Det_LeftEye, Det_RightEye, Det_Mouth};
		for(int i=0;i<3;i++)
		{
			FaceRegion faceregion = m_Detector.GetDetRegion(DetType[i]);
			m_mdc.MoveTo(faceregion.topleft.x, faceregion.topleft.y);
			m_mdc.LineTo(faceregion.topright.x, faceregion.topright.y);
			m_mdc.LineTo(faceregion.bottomright.x, faceregion.bottomright.y);
			m_mdc.LineTo(faceregion.bottomleft.x, faceregion.bottomleft.y);
			m_mdc.LineTo(faceregion.topleft.x, faceregion.topleft.y);
		}
		m_mdc.SelectObject(oldPen);
	}
	m_mdc.SetBkMode(TRANSPARENT);
	m_mdc.SetTextColor(RGB(255,255,255));

	m_mdc.TextOut(0, 0, str);
	m_mdc.TextOut(0, 20, str2);

#else
	m_Detector.SetMeshData(m_fWidth, m_fHeight, m_fProp);
	m_Detector.SetDetRectScale(m_fScale);
	m_Detector.SetBlendEdge(m_fEdge);
	DispImage(&m_mdc, m_Detector.DrawFace(m_bShowVert),CPoint(0,0));
	if(m_bAlign)
	{
		m_mdc.SetBkMode(TRANSPARENT);
		m_mdc.SetTextColor(RGB(255,255,0));
		m_mdc.TextOut(0,0,"Face Aligned");
	}
#endif

	m_mdc.SelectStockObject(NULL_BRUSH);
	dc.BitBlt(0,0,rcClt.Width(),rcClt.Height(),&m_mdc,0,0,SRCCOPY);
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
	USES_CONVERSION;
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	IplImage* image = cvLoadImage("1.jpg");
	//m_ImgBk = cvLoadImage("Koala.jpg");
/*	float hs=800./image->width;
	float vs=600./image->height;
	float wobj=min(hs,vs)*image->width;
	float hobj=min(hs,vs)*image->height;
	m_imgObj = cvCreateImage(cvSize(wobj, hobj),IPL_DEPTH_8U,3);
	cvResize(image, m_imgObj);
*/	m_Detector.Init();
	m_Detector.LoadTemplate((LPSTR)(LPCTSTR)(m_strTempl), (LPSTR)(LPCTSTR)(m_strTempl.Left(m_strTempl.ReverseFind('.'))+".cfg"));
	m_Detector.LoadSourceImage(image);
	m_Detector.DetectFace();

	cvReleaseImage(&image);

	//int wImage = m_ImgBk->width;
	//int hImage = m_ImgBk->height;
	//m_Image = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,3);


	CClientDC dc(this);
	m_mdc.CreateCompatibleDC(&dc);
#ifdef FACE_MORPH
	m_bitmap.CreateCompatibleBitmap(&dc, DIM_FACE_IMG,DIM_FACE_IMG/*wImage, hImage*/);
#else
	CvSize szImage = m_Detector.GetDestSize(DEST_TYPE_CHGFACE);
	m_bitmap.CreateCompatibleBitmap(&dc, szImage.width, szImage.height);
#endif
	oldbmp = m_mdc.SelectObject(&m_bitmap);

	CRect rcWnd;
	AfxGetMainWnd()->GetWindowRect(rcWnd);
	//rcWnd.right=rcWnd.left+wImage;
	//rcWnd.bottom=rcWnd.top+hImage;
	AfxGetMainWnd()->MoveWindow(rcWnd);

#ifdef FACE_MORPH
#if 0
	m_WndDet = new CDetectWnd;
	m_WndDet->m_Image = m_Det;
	m_WndDet->Create(NULL, "Detect");
	m_WndDet->ShowWindow(SW_SHOW);
	m_WndDet->UpdateWindow();
	m_WndDet->Invalidate();
#endif
#endif
	CFile file;

	if(file.Open("config", CFile::modeRead))
	{
		int len = file.GetLength();
		char* buf = new char[len+1];
		file.Read(buf, len);
		buf[len] = 0;
		m_strWorkingPath = A2T(buf);
		file.Close();
	}
	m_fWidth = m_Detector.GetFaceWidth();
	m_fHeight = m_Detector.GetFaceHeight();
	m_fProp = m_Detector.GetFaceOcc();

	return 0;
}

void CChildView::OnDestroy()
{
	CWnd::OnDestroy();
#ifdef FACE_MORPH
#if 0
	if(!m_WndDet->m_bDestroyed)
		m_WndDet->DestroyWindow();
#endif
#endif
	// TODO: Add your message handler code here
	if(oldbmp!=NULL)
		m_mdc.SelectObject(oldbmp);
	m_mdc.DeleteDC();
	m_bitmap.DeleteObject();
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SetCapture();
	m_bDown = true;
	m_oldPt = point;
#ifdef FACE_MORPH

#endif
	Draw();
	CWnd::OnLButtonDown(nFlags, point);
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	ReleaseCapture();
	m_bDown = false;

	CWnd::OnLButtonUp(nFlags, point);
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect rcClt;
	GetClientRect(rcClt);
#ifdef FACE_MORPH
	if(m_bDown)
	{
		Draw();
	}
#else
	if(m_bDown)
	{
		CPoint offset=point-m_oldPt;
		if(!m_bMove)
		{
			float HAngle = (float)offset.x/100;
			float VAngle = (float)offset.y/100;
			rotx = Mat(cos(HAngle),0,sin(HAngle),0,1,0,-sin(HAngle),0,cos(HAngle));
			roty = Mat(1,0,0,0,cos(VAngle),-sin(VAngle),0,sin(VAngle),cos(VAngle));
			m_Detector.GetRotationMatrix() = m_Detector.GetRotationMatrix()*rotx*roty;
		}
		else
		{
			m_Detector.GetViewPort().x+=offset.x;
			m_Detector.GetViewPort().y+=offset.y;
		}
		m_oldPt = point;
		Draw();
	}
#endif
	CWnd::OnMouseMove(nFlags, point);
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
#ifdef FACE_MORPH
#endif
	switch(nChar)
	{
	case VK_SPACE:
		m_bMove = true;
		Draw();
		break;
	case 'K':
#ifdef FACE_MORPH
#else
		//m_bShowImage = !m_bShowImage;
		m_bShowVert = !m_bShowVert;
#endif
		Draw();
		break;
#ifdef FACE_MORPH
	case 'A':
		m_bAnimate = !m_bAnimate;
		if(m_bAnimate)
		{
			m_bShowDetRegion = false;
			SetTimer(1,10,NULL);
		}
		else
		{
			KillTimer(1);
		}
		break;
#else
	case 'A':
		m_Detector.GetPos().x-=0.05;
		Draw();
		break;
	case 'D':
		m_Detector.GetPos().x+=0.05;
		Draw();
		break;
	case 'W':
		m_Detector.GetPos().z+=0.05;
		Draw();
		break;
	case 'S':
		m_Detector.GetPos().z-=0.05;
		Draw();
		break;
	case 'Q':
		m_Detector.GetPos().y-=0.05;
		Draw();
		break;
	case 'E':
		m_Detector.GetPos().y+=0.05;
		Draw();
		break;
	case 'G':
		m_Detector.SaveTemplate((LPSTR)(LPCTSTR)(m_strTempl.Left(m_strTempl.ReverseFind('.'))+".cfg"));
		MessageBox("Save Succeeded");
		break;
	case 'Z':
		m_fWidth += 0.01;
		Draw();
		break;
	case 'X':
		m_fWidth -= 0.01;
		Draw();
		break;
	case 'C':
		m_fHeight += 0.01;
		Draw();
		break;
	case 'V':
		m_fHeight -= 0.01;
		Draw();
		break;
#endif
	case 'I':
		KillTimer(1);
		m_Detector.DetectFace();
		Draw();
		break;
	case 'T':
		m_bAlign = !m_bAlign;
		m_Detector.EnableFaceAlign(m_bAlign);
		Draw();
		break;
#ifdef FACE_MORPH
		Draw();
		break;
	case VK_LEFT:
		m_nPostureSel--;
		if(m_nPostureSel < 0)
			m_nPostureSel = 0;
		Draw();
		break;
	case VK_RIGHT:
		m_nPostureSel++;
		if(m_nPostureSel >= 3)
			m_nPostureSel = 2;
		Draw();
		break;
	case VK_UP:
		m_Detector.SetPosture(m_nPostureSel,min(m_Detector.GetPosture(m_nPostureSel)+1,1));
		Draw();
		break;
	case VK_DOWN:
		m_Detector.SetPosture(m_nPostureSel,max(m_Detector.GetPosture(m_nPostureSel)-1,-1));
		Draw();
		break;
#endif
	case 'P':
		m_Detector.FitFace();
#ifdef FACE_MORPH
		m_bShowDetRegion = true;
		//SetTimer(1,10,NULL);
#endif
		Draw();
		break;
	case 'N':
#ifdef FACE_MORPH
		m_bShowDetRegion = !m_bShowDetRegion;
		Draw();
#endif
		break;
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChildView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	switch(nChar)
	{
	case VK_SPACE:
		m_bMove = false;
		break;
	}
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == 1)
	{
		Draw();
	}
	CWnd::OnTimer(nIDEvent);
}

void CChildView::OnFileOpen()
{
	// TODO: Add your command handler code here
	char* strFilter = "All Image Files|*.bmp;*.jpg;*.png|Bitmap Files|*.bmp|JPEG Files|*.jpg|Potable Net Graph|*.png|All Files|*.*||";
	CFileDialog dlg(true, NULL, NULL, 0, strFilter, this);
	if(dlg.DoModal()==IDOK)
	{
		CString name = dlg.GetPathName();
		IplImage* image = cvLoadImage((LPCTSTR)name);
		KillTimer(1);
		//m_fMorph = 0;
		//m_rcFace = Detect(m_imgObj);
		m_Detector.LoadSourceImage(image);
		m_Detector.DetectFace();
		Draw();
	}
}

void CChildView::OnFileOpenBkImg()
{
	// TODO: Add your command handler code here
#ifndef FACE_MORPH
	char* strFilter = "All Image Files|*.bmp;*.jpg;*.png|Bitmap Files|*.bmp|JPEG Files|*.jpg|Potable Net Graph|*.png|All Files|*.*||";
	CFileDialog dlg(true, NULL, NULL, 0, strFilter, this);
	if(dlg.DoModal()==IDOK)
	{
		m_strTempl = dlg.GetPathName();
		if(m_strTempl != "")
		{
			IplImage* image = cvLoadImage((LPSTR)(LPCTSTR)m_strTempl);
			m_Detector.LoadTemplate((LPSTR)(LPCTSTR)m_strTempl, (LPSTR)(LPCTSTR)(m_strTempl.Left(m_strTempl.ReverseFind('.'))+".cfg"));
			//cvReleaseImage(&m_Image);
			//m_Image = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 3);
			if(oldbmp)
			{
				CClientDC dc(this);
				m_mdc.SelectObject(oldbmp);
				m_bitmap.DeleteObject();
				CvSize szImage = m_Detector.GetDestSize(DEST_TYPE_CHGFACE);
				m_bitmap.CreateCompatibleBitmap(&dc, szImage.width, szImage.height);
			}
			CRect rcClient;
			GetClientRect(rcClient);
			CClientDC dc(this);
			m_mdc.FillSolidRect(rcClient, RGB(255,255,255));
			dc.FillSolidRect(rcClient, RGB(255,255,255));
			m_fWidth = m_Detector.GetFaceWidth();
			m_fHeight = m_Detector.GetFaceHeight();
			m_fProp = m_Detector.GetFaceOcc();
			m_strBkTitle = m_strTempl.Right(m_strTempl.GetLength() - 1 - m_strTempl.ReverseFind(_T('\\')));

			Draw();
		}
	}
#endif
	m_strTemplName = _T("");
}

void CChildView::OnFileSettings()
{
	// TODO: Add your command handler code here
	USES_CONVERSION;
	CDialogSettings dlg(this);
	dlg.m_strFaceWidth.Format(_T("%4.2f"), m_fWidth);
	dlg.m_strFaceHeight.Format(_T("%4.2f"), m_fHeight);
	dlg.m_strProp.Format(_T("%4.2f"), m_fProp);
	dlg.m_strEdge.Format(_T("%4.2f"), m_fEdge);
	dlg.m_strScale.Format(_T("%4.2f"), m_fScale);
	dlg.m_strWorkingPath = m_strWorkingPath;
	dlg.m_strTemplName = m_strTemplName;
	if(IDOK == dlg.DoModal())
	{
		m_fWidth = atof(T2A((LPTSTR)(LPCTSTR)dlg.m_strFaceWidth));
		m_fHeight = atof(T2A((LPTSTR)(LPCTSTR)dlg.m_strFaceHeight));
		m_fProp = atof(T2A((LPTSTR)(LPCTSTR)dlg.m_strProp));
		m_fEdge = atof(T2A((LPTSTR)(LPCTSTR)dlg.m_strEdge));
		m_fScale = atof(T2A((LPTSTR)(LPCTSTR)dlg.m_strScale));
		m_strWorkingPath = dlg.m_strWorkingPath;
		m_strTemplName = dlg.m_strTemplName;
		CFile file;
		if(file.Open("config", CFile::modeCreate|CFile::modeWrite))
		{
			char* str = T2A((LPTSTR)(LPCTSTR)m_strWorkingPath);
			file.Write(str, strlen(str));
			file.Close();
		}
	}
}

LRESULT CChildView::OnApplySettings(WPARAM wParam, LPARAM lParam)
{
	USES_CONVERSION;
	CDialogSettings* dlg = (CDialogSettings*)wParam;
	m_strWorkingPath = dlg->m_strWorkingPath;
	if(m_strWorkingPath.Right(1) == _T("\\"))
		m_strWorkingPath = m_strWorkingPath.Left(m_strWorkingPath.GetLength()-1);
	m_fWidth = atof(T2A((LPTSTR)(LPCTSTR)dlg->m_strFaceWidth));
	m_fHeight = atof(T2A((LPTSTR)(LPCTSTR)dlg->m_strFaceHeight));
	m_fProp = atof(T2A((LPTSTR)(LPCTSTR)dlg->m_strProp));
	m_fEdge = atof(T2A((LPTSTR)(LPCTSTR)dlg->m_strEdge));
	m_fScale = atof(T2A((LPTSTR)(LPCTSTR)dlg->m_strScale));
	m_strTemplName = dlg->m_strTemplName;
	CFile file;
	if(file.Open("config", CFile::modeCreate|CFile::modeWrite))
	{
		char* str = T2A((LPTSTR)(LPCTSTR)m_strWorkingPath);
		file.Write(str, strlen(str));
		file.Close();
	}
	Draw();
	return 0;
}

LRESULT CChildView::OnGenerateTempl(WPARAM wParam, LPARAM lParam)
{
	USES_CONVERSION;
	if (((CDialogSettings*)wParam)->m_strTemplName.IsEmpty())
	{
		return 3;
	}
	m_strTemplName = ((CDialogSettings*)wParam)->m_strTemplName;
	CoCreateGuid(&m_guid);
	TiXmlDocument DocTempl(T2A((LPTSTR)(LPCTSTR)(m_strTemplName+_T(".tmpl"))));

	TiXmlDocument* eleRoot = &DocTempl;
	TiXmlElement* eletmpls = new TiXmlElement("templates");
	TiXmlElement* eletmpl = new TiXmlElement("template");
	eletmpl->SetAttribute("name", T2A((LPTSTR)(LPCTSTR)m_strTemplName));
	eletmpl->SetAttribute("category", "face");
	eletmpl->SetAttribute("guid", T2A((LPTSTR)(LPCTSTR)FormatGuid(m_guid)));


	TiXmlElement* eleFilter = new TiXmlElement("filter");
	eleFilter->SetAttribute("name", "FaceTemplate");
	eleFilter->SetAttribute("type", "change face");
	TiXmlElement* eleParam = new TiXmlElement("param");
	eleParam->SetDoubleAttribute("scale", m_fScale);
	eleParam->SetDoubleAttribute("blend_edge", m_fEdge);

	TiXmlElement* eleFilter2 = new TiXmlElement("filter");
	eleFilter2->SetAttribute("name", "FaceFilter");
	TiXmlElement* eleParam2 = new TiXmlElement("param");
	CString strBk = CString(_T(".\\media\\Images\\")) + m_strBkTitle;
	eleParam2->SetAttribute("image", T2A((LPTSTR)(LPCTSTR)strBk));
	CString strCfgTitle = m_strBkTitle.Left(m_strBkTitle.Find(_T('.'))) + _T(".cfg");
	CString strCfg = CString(_T(".\\media\\Configurations\\")) + strCfgTitle;
	eleParam2->SetAttribute("config", T2A((LPTSTR)(LPCTSTR)strCfg));
	eleFilter2->LinkEndChild(eleParam2);
	eleFilter->LinkEndChild(eleParam);
	eleFilter->LinkEndChild(eleFilter2);

	eletmpls->LinkEndChild(eletmpl);
	eletmpls->LinkEndChild(eleFilter);

	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "UTF-8", "");

	eleRoot->LinkEndChild(decl);
	eleRoot->LinkEndChild(eletmpls);

	m_Detector.SaveTemplate((LPSTR)(LPCTSTR)(m_strTempl.Left(m_strTempl.ReverseFind('.'))+".cfg"));

	if(DocTempl.SaveFile(T2A((LPTSTR)(LPCTSTR)(m_strTemplName+_T(".tmpl")))))
		return S_OK;
	else
		return S_FALSE;
}

LRESULT CChildView::OnExportTempl(WPARAM wParam, LPARAM lParam)
{
	USES_CONVERSION;
	CString strTemplFileName = m_strTemplName+_T(".tmpl");
	CString strCfgFileName = (LPSTR)(LPCTSTR)(m_strTempl.Left(m_strTempl.ReverseFind('.'))+".cfg");
	CString strCfgTitle = m_strBkTitle.Left(m_strBkTitle.Find(_T('.'))) + _T(".cfg"); 
	if(m_strTemplName.IsEmpty()||!PathFileExists(strTemplFileName)
		|| !PathFileExists(strCfgFileName))
	{
		LRESULT lRet = OnGenerateTempl(wParam, lParam);
		if(lRet != S_OK)
		{
			return lRet;
		}
		strTemplFileName = m_strTemplName+_T(".tmpl");
	}

	CString strTemplFileNameDest = m_strWorkingPath + _T("\\Templates\\") + m_strTemplName+_T(".tmpl");
	if(PathFileExists(strTemplFileNameDest))
	{
		CString strErr;
		strErr.Format(_T("The file \"%s\" has already existed, overwrite it?"), strTemplFileNameDest);
		if(IDCANCEL == MessageBox(strErr, _T("Overwrite files"), MB_OKCANCEL))
			return 2;
	}
	if(!CopyFile(strTemplFileName, strTemplFileNameDest, FALSE))
		return 2;;

	CString strBk = m_strWorkingPath  + _T("\\Images\\") + m_strBkTitle;

	if(PathFileExists(strBk))
	{
		CString strErr;
		strErr.Format(_T("The file \"%s\" has already existed, overwrite it?"), strBk);
		if(IDCANCEL == MessageBox(strErr, _T("Overwrite files"), MB_OKCANCEL))
			return 2;
	}
	if(!CopyFile(m_strTempl, strBk, FALSE))
		return 2;;

	CString strCfgDest = m_strWorkingPath + _T("\\Configurations\\") + strCfgTitle;
	if(PathFileExists(strCfgDest))
	{
		CString strErr;
		strErr.Format(_T("The file \"%s\" has already existed, overwrite it?"), strCfgDest);
		if(IDCANCEL == MessageBox(strErr, _T("Overwrite files"), MB_OKCANCEL))
			return 2;
	}
	if(!CopyFile(strCfgFileName, strCfgDest, FALSE))
		return 2;

	CString CfgFileName = m_strWorkingPath + _T("\\config.cfg");
	TiXmlDocument docCfg(T2A((LPSTR)(LPCTSTR)CfgFileName));

	if(!docCfg.LoadFile(T2A((LPSTR)(LPCTSTR)CfgFileName)))
	{
		if(PathFileExists(CfgFileName))
		{
			MessageBox(_T("Read template database failed"));
			return ERROR_UNKNOWN;
		}
		docCfg.Clear();
	}
	TiXmlElement* eleTempl = docCfg.FirstChildElement("templates");
	if(eleTempl == NULL)
	{
		eleTempl = new TiXmlElement("templates");
		docCfg.LinkEndChild(eleTempl);
	}
	CString strCfgName = m_strTemplName;
	strCfgName.Replace(_T(' '), _T('_'));
	TiXmlElement* eleThis = eleTempl->FirstChildElement(T2A((LPTSTR)(LPCTSTR)strCfgName));
	if(eleThis != NULL)
	{
		CString strErr;
		strErr.Format(_T("The template \"%s\" already exists in the program database, overwrite it?"), m_strTemplName);
		if(IDOK == MessageBox(strErr, _T("Overwrite template"), MB_OKCANCEL))
		{
			eleTempl->RemoveChild(eleThis);
			eleThis = NULL;
		}
	}
	eleThis = new TiXmlElement(T2A((LPTSTR)(LPCTSTR)strCfgName));
	eleThis->SetAttribute("guid", T2A((LPTSTR)(LPCTSTR)FormatGuid(m_guid)));
	eleTempl->LinkEndChild(eleThis);
	if(!docCfg.SaveFile(T2A((LPSTR)(LPCTSTR)CfgFileName)))
		return 4;

	return S_OK;
}

CString CChildView::FormatGuid(GUID& guid)
{
	CString strGuid;
	strGuid.Format(_T("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),

		guid.Data1, guid.Data2, guid.Data3,

		guid.Data4[0], guid.Data4[1],

		guid.Data4[2], guid.Data4[3],

		guid.Data4[4], guid.Data4[5],

		guid.Data4[6], guid.Data4[7]);

	return strGuid;
}