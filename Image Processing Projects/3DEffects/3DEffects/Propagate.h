#ifndef _PROPAGATE_H_
#define _PROPAGATE_H_

#define PROP_MODE_QUAD  1
#define PROP_MODE_RECT  2

#define PROP_FLAG_LEFT   1
#define PROP_FLAG_RIGHT  2
#define PROP_FLAG_TOP    4
#define PROP_FLAG_BOTTOM 8

typedef bool (*PropFunc)(void* param, CvPoint pt, CvPoint ptorg, IplImage* pMask, UINT8 flags);

class CPropagate
{
public: 
	CPropagate();
	~CPropagate();
	CvPoint* m_bufacc;
	int m_ptacc;
public:
	IplImage* m_mask;
	CvPoint* m_buf;
	CvPoint* m_buf2;
	UINT8*     m_flags;
	UINT8*     m_flags2;
	int m_nBuf;
	int m_pts;
	CvPoint m_pt;
	PropFunc m_pFunc;
public:
	void Clear();
	void Init(int width, int height);
	void SetFunc(PropFunc func);
	int Propagate(int mode = PROP_MODE_QUAD, bool bAutoMark = true, void* param = NULL, bool bmaintainoutliers = false);
	void SetPoint(CvPoint pt);
};

#endif