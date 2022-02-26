#ifndef _PROPAGATE_H_
#define _PROPAGATE_H_

#define PROP_MODE_QUAD  1
#define PROP_MODE_RECT  2

#define PROP_FLAG_LEFT   1
#define PROP_FLAG_RIGHT  2
#define PROP_FLAG_TOP    4
#define PROP_FLAG_BOTTOM 8

#pragma pack(push)
#pragma pack(1)

//This design can only be applied to those structures with no constructors

template<class T>
struct CompNode
{
	int ndata;
	CompNode*next;
	T data[1];
};
#pragma pack(pop)

#define GROW_LINEAR       0
#define GROW_EXPONENTIAL  1
template<class T>
class Comp_Container
{
public:
	Comp_Container(int initl=10, int grow=GROW_LINEAR);
	~Comp_Container();
	T& operator[](int n);
	void SetMaxSize(int nMax){m_nMax=max(0,nMax);}
	void Reset();
	void Swap(Comp_Container<T>& inst2);
private:
	CompNode<T>* first;
	int m_initl;
	int m_ngrow;
	int m_nMax;
};

typedef bool (*PropFunc)(void* param, CvPoint pt, CvPoint ptorg, IplImage* pMask, UINT8 flags);

class CPropagate
{
public: 
	CPropagate();
	~CPropagate();
	Comp_Container<CvPoint> m_bufacc;
	int m_ptacc;
public:
	IplImage* m_mask;
	Comp_Container<CvPoint> m_buf;
	Comp_Container<CvPoint> m_buf2;
	int m_nBuf;
	int m_pts;
	CvPoint m_pt;
	bool m_bMask;
	int m_width;
	int m_height;
	PropFunc m_pFunc;
public:
	void Clear();
	void Init(int width, int height, bool usemask=true);
	void SetFunc(PropFunc func);
	int Propagate(int mode = PROP_MODE_QUAD, bool bAutoMark = true, void* param = NULL, bool bmaintainoutliers = false);
	void SetPoint(CvPoint pt);
};

#endif