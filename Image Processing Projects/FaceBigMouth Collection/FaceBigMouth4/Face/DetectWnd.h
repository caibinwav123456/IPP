#pragma once


// CDetectWnd frame

class CDetectWnd : public CFrameWnd
{
	DECLARE_DYNCREATE(CDetectWnd)
public:
	CDetectWnd();           // protected constructor used by dynamic creation
	virtual ~CDetectWnd();

	bool m_bDestroyed;
	IplImage* m_Image;
	void DispImage(CDC* pDC, IplImage* image, CPoint ptBase);
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
};


