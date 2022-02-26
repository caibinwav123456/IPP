#ifndef _LICENSE_RECOG_H_
#define _LICENSE_RECOG_H_
#include <vector>
using namespace std;

#define FIND_COMPS_ALGORITHM1   1
#define FIND_COMPS_ALGORITHM2   2
#define FIND_LINES_ALGORITHM1   4
#define FIND_LINES_ALGORITHM2   8

struct Line
{
	float theta; //angle between the distance line and x-axis;
	float dist;  //distance from the origin point 
	float start;
	float end;

	float conf;

	Line(float _theta = 0, float _dist = 0, float _start = 0, float _end = 0)
	{
		theta = _theta;
		dist = _dist;
		start = _start;
		end = _end;
		conf = 0;
	}
	void GetEndPoint(Vec2* ptstart, Vec2* ptend)
	{
		Vec2 norm(cos(theta), sin(theta));
		Vec2 tang(-norm.y, norm.x);
		if(ptstart)
			*ptstart = norm*dist+tang*start;
		if(ptend)
			*ptend = norm*dist+tang*end;
	}
};

struct Quad
{
	Vec2 _00,_01,_10,_11;
	Quad(Vec2 __00 = Vec2(), Vec2 __01 = Vec2(), Vec2 __10 = Vec2(), Vec2 __11 = Vec2())
	{
		_00 = __00, _01 = __01, _10 = __10, _11 = __11;
	}
};

struct BOX
{
	float left;
	float top;
	float right;
	float bot;
	BOX(float _left = 0, float _top = 0, float _right = 0, float _bot = 0)
	{
		left = _left, top = _top, right = _right, bot = _bot;
	}
	BOX(CvRect& rc)
	{
		left = rc.x, top = rc.y, right = rc.x+rc.width, bot = rc.y+rc.height;
	}
	BOX(CvSize& sz)
	{
		left = top = 0, right = sz.width, bot = sz.height;
	}
};

struct LineListNode
{
	Line line;
	vector<int> linepts;
	LineListNode* next;
	LineListNode* prev;
	LineListNode(){next = prev = NULL;}
};

#define REGRESSION_DEBUG
#define REGRESSION_RELEASE
class CLicenseRecog
{
public:
	CLicenseRecog();
	~CLicenseRecog();

	void Release();
	void Reset();
	void LoadImage(char* name);
	void LoadImage(IplImage* image);
	void Threshold();
	void Segment();
	float ComputeThreshValue(IplImage* image);
	void Refine();
	void IdentifyRegion();
	void FindComps(int nAlg = FIND_COMPS_ALGORITHM1);
	void DrawComps(CDC* pDC, CPoint ptBase = CPoint(0,0));
	void DrawComps(CDC* pDC, int index, CPoint ptBase = CPoint(0,0), bool bPlot = false);
	int IndexFromPoint(CPoint pt);
	CvPoint2D32f FindContourCenter(int index, float* maxdiff = NULL);
	IplImage* HoughImage(int index, CvPoint2D32f* ptCenter = NULL, IplImage** imgstartend = NULL);
	void FindLines(int index, Line** buffer, int* nLine, float thresh1 = 0.08, float thresh2 = 20);
	void FindLines(int nAlg, float thresh1 = 0.08, float thresh2 = 20);
	void RefineLines();
	int ProcessAllLines();
	void OverAllRefineLines();
	int  OverAllConnect();
	void FindLicenseArea();
	void IterAllLines(void (*pIterFunc)(int, int, void*), void* param);
#ifdef REGRESSION_DEBUG
	void StartRegression(int index);
	void LineRegression();
	void EndLineRegression();
	void ClearLineBuf();
	void DrawSpots(CDC* pDC, int index);
#endif
#ifdef REGRESSION_RELEASE
	void LineRegression(CvPoint* ptlist, int npt, Line** lines, int* nline);
#endif
	void Process(int nAlg = FIND_COMPS_ALGORITHM1);
private:
public:
	IplImage* m_src;
	IplImage* m_bin;
	IplImage* m_seg;
	IplImage* m_refine;
	IplImage* m_imgid;
	vector<vector<CvPoint>*> m_Comps;
	vector<Line*> m_lines;
	vector<int> m_nlines;
	vector<Line*> m_reflines;
	vector<int> m_nreflines;
	CvMemStorage* m_storage;
	CvSeq*    m_Contours;
	CvSeq*    m_Segments;
	int  m_nContour;
	int m_nComp;
	vector<Quad> m_QuadList;
	vector<Line> m_AllLines;
	vector<int>* m_LineCells;
	int          m_xcellline;
	int          m_ycellline;
#ifdef REGRESSION_DEBUG
	CvPoint* m_ptlist;
	int m_npt;
	BOX m_box;
	int m_xcell;
	int m_ycell;
	vector<int>* m_cells;
	LineListNode** m_Assign;
	LineListNode* m_curline;
	LineListNode* m_linelist;
	int m_ptpos;
#endif
private:
	void SetupCells(CvPoint* ptlist, int npt, const BOX& box, int xcell, int ycell, vector<int>* &cells);
	bool Sorting(CvPoint* ptlist, LineListNode* &linelist, LineListNode* &curline,LineListNode** assign, int ptpos, vector<int>& chopcandts, vector<float>& dists, float& meandist, bool& bincorp);
	bool Chop(CvPoint* ptlist, LineListNode* &linelist, LineListNode* &curline, LineListNode** assign, vector<int> chopcandts, vector<float>& dists, float meandist);
	bool Connect(CvPoint* ptlist, LineListNode* &linelist, LineListNode* &curline, LineListNode** assign);
	void ComputeLineDist(const Line& a, const Line& b, float& dist, float& theta);
	float PointLineDist(const Line& line, CvPoint pt);
	void BeginNewLine(CvPoint* ptlist, LineListNode* &linelist, LineListNode* &curline, LineListNode** assign, int ptpos);
	void FindNearestPoint(CvPoint* ptlist, int& ptpos, int npt, const BOX& box, const vector<int>* cells, int xcell, int ycell);

	static int LineCompare(const void* a, const void* b);
	void CategorizeLine(const Line& line, int ndata, vector<CvPoint>* cate = NULL);
	void CategorizeLines();
	static void _OverAllConnect(int lindex1, int lindex2, void* param);
	static void _FindLicenseArea(int lindex1, int lindex2, void* param);
	bool FindRectArea(const Line& linea, const Line& lineb);
};

#endif