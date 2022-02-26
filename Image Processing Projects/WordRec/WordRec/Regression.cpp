#include "stdafx.h"
#include "LicenseRecog.h"

#define CLUSTER_THRESH  5
#define CHOP_THRESH     4

#define LINE_CONN_THRESH_DIST  5
#define LINE_CONN_THRESH_THETA PI/16
#define MIN_LINE_POINTS        10
#define CHOP_SCALE             0.3
#define CHOP_MEAN_THRESH       3

Line LinearRegression(const CvPoint* ptlist, const vector<int>& index)
{
	double xx=0,yy=0,xy=0;
	double x=0,y=0;
	for(int i=0;i<(int)index.size();i++)
	{
		CvPoint pt = ptlist[index[i]];
		xx += pt.x*pt.x;
		yy += pt.y*pt.y;
		xy += pt.x*pt.y;

		x += pt.x;
		y += pt.y;
	}
	double det = xx*yy-xy*xy;
	Line line;
	if(det == 0)
	{
		line.dist = 0;
		line.theta = atan(y/x)+PI/2;
		if(line.theta>PI/2)line.theta -= PI;
		else if(line.theta<-PI/2)line.theta += PI;
		if(x == 0)
		{
			line.theta = PI/2;
		}
	}
	else
	{
		double xxinv = yy/det;
		double yyinv = xx/det;
		double xyinv = -xy/det;

		double vx = xxinv*x+xyinv*y;
		double vy = xyinv*x+yyinv*y;
		Vec2  v(vx, vy);
		line.dist = 1/v.length();
		line.theta = atan(vy/vx);
		if(vx == 0)
		{
			if(vy > 0)
				line.theta = PI/2;
			else if(vy < 0)
				line.theta = -PI/2;
			else
			{
				line.theta = line.dist = 0;
			}
		}
		if(vx < 0)
		{
			line.theta += PI;
			if(line.theta > PI)
				line.theta -= 2*PI;
			else if(line.theta < -PI)
				line.theta += 2*PI;
		}
	}
	line.start = 1.0e10;
	line.end = -1.0e10;

	for(int i=0;i<(int)index.size();i++)
	{
		CvPoint pt = ptlist[index[i]];
		float coord = -pt.x*sin(line.theta)+pt.y*cos(line.theta);
		if(coord<line.start)line.start = coord;
		if(coord>line.end)line.end = coord;
	}
	if((int)index.size() == 0)
	{
		line.start = line.end = 0;
	}

	return line;
}

BOX ComputeBoundingBox(const CvPoint* ptlist, int npt)
{
	BOX box(1.0e10,1.0e10,-1.0e10,-1.0e10);
	for(int i=0;i<npt;i++)
	{
		if(box.left>ptlist[i].x)
			box.left = ptlist[i].x;
		if(box.right<ptlist[i].x)
			box.right = ptlist[i].x;
		if(box.top>ptlist[i].y)
			box.top = ptlist[i].y;
		if(box.bot<ptlist[i].y)
			box.bot = ptlist[i].y;
	}
	return box;
}

#ifdef REGRESSION_DEBUG
void CLicenseRecog::ClearLineBuf()
{
	if(m_linelist)
	{
		for(LineListNode* iter = m_linelist->next; iter != m_linelist;)
		{
			LineListNode* node = iter;
			iter = iter->next;
			delete node;
		}
		delete m_linelist;
		m_linelist = NULL;
	}
	m_curline = NULL;
}

void CLicenseRecog::StartRegression(int index)
{
	ClearLineBuf();
	vector<CvPoint>* contour = m_Comps[index];
	m_npt = (int)contour->size();
	if(m_npt == 0)
		return;
	m_ptlist = new CvPoint[m_npt];
	for(int i=0;i<m_npt;i++)
	{
		m_ptlist[i] = contour->at(i);
	}
	m_box = ComputeBoundingBox(m_ptlist, m_npt);
	float nAdj = 4*CLUSTER_THRESH;
	m_xcell = (int)floor((m_box.right-m_box.left)/nAdj);
	m_ycell = (int)floor((m_box.bot-m_box.top)/nAdj);
	if(m_xcell<1)m_xcell = 1;
	if(m_ycell<1)m_ycell = 1;

	SetupCells(m_ptlist, m_npt, m_box, m_xcell, m_ycell, m_cells);
	m_Assign = new LineListNode*[m_npt];
	for(int i=0;i<m_npt;i++)
	{
		m_Assign[i] = NULL;
	}
	m_ptpos = 0;
	m_curline = NULL;
}

void CLicenseRecog::LineRegression()
{
	if(m_ptlist == NULL)
		return;
	if(m_linelist == NULL)
	{
		BeginNewLine(m_ptlist, m_linelist, m_curline, m_Assign, m_ptpos);
	}
	else
	{
		if(m_Assign[m_ptpos] != NULL)
		{
			m_curline = m_Assign[m_ptpos];
			m_ptpos = m_curline->linepts.back();
			Connect(m_ptlist, m_linelist, m_curline, m_Assign);
		}
		else
		{
			vector<int> chopcandts;
			vector<float>dists;
			float meandist = 0;
			bool bStartNewLine = false;
			Sorting(m_ptlist, m_linelist, m_curline, m_Assign, m_ptpos, chopcandts, dists, meandist, bStartNewLine);
			bStartNewLine = !bStartNewLine;

			if(m_curline && m_linelist)
			{
				Chop(m_ptlist, m_linelist, m_curline, m_Assign, chopcandts, dists, meandist);
			}

			if(bStartNewLine) // dist>CLUSTER_THRESH
			{
				BeginNewLine(m_ptlist, m_linelist, m_curline, m_Assign, m_ptpos);
			}
		}
	}
	FindNearestPoint(m_ptlist, m_ptpos, m_npt, m_box, m_cells, m_xcell, m_ycell);
}

void CLicenseRecog::EndLineRegression()
{
	if(m_ptlist)
	{
		delete[] m_ptlist;
		m_ptlist = NULL;
	}
	m_npt = 0;
	if(m_Assign)
	{
		delete[] m_Assign;
		m_Assign = NULL;
	}
	if(m_cells)
	{
		delete[] m_cells;
		m_cells = NULL;
	}
	m_ptpos = 0;
	if(m_linelist)
	{
		for(LineListNode* iter = m_linelist->next; iter != m_linelist;)
		{
			LineListNode* node = iter;
			iter = iter->next;
			delete node;
		}
		delete m_linelist;
		m_linelist = NULL;
	}
	m_curline = NULL;
	m_xcell = 0;
	m_ycell = 0;
	m_box = BOX();
}
#endif
#ifdef REGRESSION_RELEASE
void CLicenseRecog::LineRegression(CvPoint* ptlist, int npt, Line** lines, int* nline)
{
	BOX box = ComputeBoundingBox(ptlist, npt);
	float nAdj = 4*CLUSTER_THRESH;
	int xcell = (int)floor((box.right-box.left)/nAdj);
	int ycell = (int)floor((box.bot-box.top)/nAdj);
	if(xcell <= 0)xcell = 1;
	if(ycell <= 0)ycell = 1;

	vector<int> *cells = NULL;
	SetupCells(ptlist, npt, box, xcell, ycell, cells);

	LineListNode** Assign = new LineListNode*[npt];
	for(int i=0;i<npt;i++)
	{
		Assign[i] = NULL;
	}
	LineListNode* linelist = NULL;
	LineListNode* curline = NULL;
	int ptpos = 0;
	int itertime = 0;
	while(true)
	{
		bool bChg = false;
		if(linelist == NULL)
		{
			BeginNewLine(ptlist, linelist, curline, Assign, ptpos);
			bChg = true;
		}
		else
		{
			if(Assign[ptpos] != NULL)
			{
				int oldpos = ptpos;
				curline = Assign[ptpos];
				ptpos = curline->linepts.back();

				if(oldpos <= ptpos)
				{
					itertime += ptpos - oldpos;
				}
				else
				{
					itertime += ptpos + npt - oldpos;
				}

				if(Connect(ptlist, linelist, curline, Assign))
					bChg = true;
			}
			else
			{
				vector<int> chopcandts;
				vector<float> dists;
				float meandist = 0;
				bool bStartNewLine = false;
				if(Sorting(ptlist, linelist, curline, Assign, ptpos, chopcandts, dists, meandist, bStartNewLine))
					bChg = true;
				bStartNewLine = !bStartNewLine;

				if(curline && linelist)
				{
					if(Chop(ptlist, linelist, curline, Assign, chopcandts, dists, meandist))
						bChg = true;
				}

				if(bStartNewLine) // dist>CLUSTER_THRESH
				{
					BeginNewLine(ptlist, linelist, curline, Assign, ptpos);
					bChg = true;
				}
			}
		}
		int oldpos = ptpos;
		FindNearestPoint(ptlist, ptpos, npt, box, cells, xcell, ycell);
		if(oldpos <= ptpos)
		{
			itertime += ptpos - oldpos;
		}
		else
		{
			itertime += ptpos + npt - oldpos;
		}
		if(itertime>6*npt)
			break;
		//if(!bChg)
		//	break;
	}
	delete[] cells;
	delete[] Assign;
	int lcnt = 0;

	if(linelist)
	{
		for(LineListNode* iter = linelist->next; iter != linelist; iter = iter->next)
		{
			lcnt++;
		}
		lcnt++;
		if(nline)
			*nline = lcnt;
		if(lines)
		{
			*lines = new Line[lcnt];
			(*lines)[0] = linelist->line;
			int i = 1;
			for(LineListNode* iter = linelist->next; iter != linelist; iter = iter->next, i++)
			{
				(*lines)[i] = iter->line;
			}
		}
		for(LineListNode* iter = linelist->next; iter != linelist;)
		{
			LineListNode* node = iter;
			iter = iter->next;
			delete node;
		}
		delete linelist;
	}
	else
	{
		if(nline)
			*nline = 0;
		if(lines)
			*lines = 0;
	}
}
#endif
void CLicenseRecog::BeginNewLine(CvPoint* ptlist, LineListNode* &linelist, LineListNode* &curline, LineListNode** assign, int ptpos)
{
	LineListNode* linenode = new LineListNode;
	linenode->linepts.push_back(ptpos);
	linenode->line = LinearRegression(ptlist, linenode->linepts);
	if(curline == NULL)
	{
		linenode->next = linenode;
		linenode->prev = linenode;
		curline = linenode;
		linelist = linenode;
	}
	else
	{
		LineListNode* nextnode = curline->next;
		curline->next = linenode;
		linenode->next = nextnode;
		nextnode->prev = linenode;
		linenode->prev = curline;
		curline = linenode;
	}
	assign[ptpos] = linenode;
}

void CLicenseRecog::ComputeLineDist(const Line& a, const Line& b, float& dist, float& theta)
{
	float xext = (a.start + a.end)/2;
	CvPoint2D32f ptcenter = cvPoint2D32f(a.dist*cos(a.theta)-xext*sin(a.theta),
		a.dist*sin(a.theta)+xext*cos(a.theta));
	float distdiff = fabs(ptcenter.x*cos(b.theta)+ptcenter.y*sin(b.theta)-b.dist);
	float thetadiff = fabs(a.theta - b.theta);

	dist = distdiff;
	theta = thetadiff;
}

float CLicenseRecog::PointLineDist(const Line& line, CvPoint pt)
{
	return fabs(pt.x*cos(line.theta)+pt.y*sin(line.theta)-line.dist);
}

bool CLicenseRecog::Sorting(CvPoint* ptlist, LineListNode* &linelist, LineListNode* &curline, LineListNode** assign, int ptpos, vector<int>& chopcandts, vector<float>& dists, float& meandist, bool& bincorp)
{
	bool bRet = false;
	meandist = 0;
	vector<int>& curlinept = curline->linepts;
	float dist = PointLineDist(curline->line, ptlist[ptpos]);
	if(dist<=CLUSTER_THRESH)
	{
		curlinept.push_back(ptpos);
		assign[ptpos] = curline;
		bincorp = true;
		bRet = true;
	}
	else
		bincorp = false;

	curline->line = LinearRegression(ptlist, curlinept);

	int n = (int)curlinept.size();

	for(int i=0;i<(int)curlinept.size();)
	{
		int index = curlinept[i];
		CvPoint pt = ptlist[index];
		float dist2 = PointLineDist(curline->line, pt);
		meandist+=dist2;
		if(dist2>CHOP_THRESH)
		{
			chopcandts.push_back(i);
			dists.push_back(dist2);
		}
		if(dist2>CLUSTER_THRESH)
		{
			curlinept.erase(curlinept.begin()+i);
			assign[index] = NULL;
			if(index == ptpos)
				bincorp = false;
			bRet = true;
		}
		else
		{
			i++;
		}
	}
	if((int)curlinept.size() <= 1)
	{
		LineListNode* prevline = curline->prev;
		LineListNode* nextline = curline->next;
		if(prevline == curline)
		{
			delete curline;
			curline = linelist = NULL;
		}
		else
		{
			prevline->next = nextline;
			nextline->prev = prevline;
			if(linelist == curline)
				linelist = prevline;
			delete curline;
			curline = prevline;
		}
	}
	meandist/=n;
	return bRet;
}

bool CLicenseRecog::Chop(CvPoint* ptlist, LineListNode* &linelist, LineListNode* &curline, LineListNode** assign, vector<int> chopcandts, vector<float>& dists, float meandist)
{
	float maxdist = 0.0;
	int imax = -1;
	vector<int>& curlinept = curline->linepts;
	if(meandist >= CHOP_MEAN_THRESH)
	{
		for(int i=0;i<(int)chopcandts.size();i++)
		{
			int choplimit = (int)curlinept.size()*CHOP_SCALE;
			if(choplimit>=MIN_LINE_POINTS)
			{
				if(chopcandts[i]>=choplimit && chopcandts[i]<(int)curlinept.size() - choplimit)
				{
					if(dists[i]>maxdist)
					{
						maxdist = dists[i];
						imax = i;
					}
				}
			}
		}
	}
	if(imax != -1)
	{
		//chop
		LineListNode* linenodebefore = new LineListNode;
		LineListNode* linenodeafter = new LineListNode;
		for(int i=0;i<(int)curlinept.size();i++)
		{
			if(i<imax)
			{
				linenodebefore->linepts.push_back(curlinept[i]);
				assign[curlinept[i]] = linenodebefore;
			}
			else
			{
				linenodeafter->linepts.push_back(curlinept[i]);
				assign[curlinept[i]] = linenodeafter;
			}
		}
		linenodebefore->line = LinearRegression(ptlist, linenodebefore->linepts);
		linenodeafter->line = LinearRegression(ptlist, linenodeafter->linepts);
		LineListNode* node2del = curline;
		if(curline->prev == curline)
		{
			linenodebefore->prev = linenodeafter;
			linenodeafter->prev = linenodebefore;
			linenodebefore->next = linenodeafter;
			linenodeafter->next = linenodebefore;
			curline = linelist = linenodeafter;
		}
		else
		{
			LineListNode* prevnode = curline->prev;
			LineListNode* nextnode = curline->next;
			if(linelist == curline)
				linelist = linenodebefore;
			prevnode->next = linenodebefore;
			linenodebefore->next = linenodeafter;
			linenodeafter->next = nextnode;
			nextnode->prev = linenodeafter;
			linenodeafter->prev = linenodebefore;
			linenodebefore->prev = prevnode;
			curline = linenodeafter;
		}
		delete node2del;
		return true;
	}
	return false;
}

bool CLicenseRecog::Connect(CvPoint* ptlist, LineListNode* &linelist, LineListNode* &curline, LineListNode** assign)
{
	if(curline->prev != curline)
	{
		LineListNode* prevline = curline->prev;
		float xext = (prevline->line.start + prevline->line.end)/2;
		float distdiff = 0; 
		float thetadiff = 0;
		ComputeLineDist(prevline->line, curline->line, distdiff, thetadiff);
		if(distdiff <= LINE_CONN_THRESH_DIST && thetadiff <= LINE_CONN_THRESH_THETA)
		{
			// connect
			LineListNode* connline = new LineListNode;
			for(int i=0;i<(int)prevline->linepts.size();i++)
			{
				connline->linepts.push_back(prevline->linepts[i]);
				assign[prevline->linepts[i]] = connline;
			}
			for(int i=0;i<(int)curline->linepts.size();i++)
			{
				connline->linepts.push_back(curline->linepts[i]);
				assign[curline->linepts[i]] = connline;
			}
			connline->line = LinearRegression(ptlist, connline->linepts);
			if(prevline->prev == curline)
			{
				connline->prev = connline->next = connline;
				delete prevline;
				delete curline;
				curline = linelist = connline;
			}
			else
			{
				prevline->prev->next = connline;
				connline->next = curline->next;
				curline->next->prev = connline;
				connline->prev = prevline->prev;
				if(linelist == curline || linelist == prevline)
					linelist = connline;
				delete prevline;
				delete curline;
				curline = connline;
			}
			return true;
		}
	}
	return false;
}

void CLicenseRecog::FindNearestPoint(CvPoint* ptlist, int& ptpos, int npt, const BOX& box, const vector<int>* cells, int xcell, int ycell)
{
	int oldpos = ptpos;
	ptpos++;//need to expand
	if(ptpos>=npt)
		ptpos = 0;
	float distpt = sqrt(powf(ptlist[ptpos].x-ptlist[oldpos].x, 2) + powf(ptlist[ptpos].y-ptlist[oldpos].y, 2));
	float mindistpt = 1.0e10;
	int imindist = -1;
	int icellmatch = -1;
	int jcellmatch = -1;
	if(distpt > 3*CLUSTER_THRESH)
	{
		int icell = (int)floor((ptlist[oldpos].x-box.left)/(box.right - box.left)*xcell);
		int jcell = (int)floor((ptlist[oldpos].y-box.top)/(box.bot-box.top)*ycell);
		if(icell<0)icell = 0;
		else if(icell>=xcell)icell = xcell-1;
		if(jcell<0)jcell = 0;
		else if(jcell>=ycell)jcell = ycell-1;
		for(int i=icell-1;i<=icell+1;i++)
		{
			for(int j=jcell-1;j<=jcell+1;j++)
			{
				if(i<0 || i>=xcell)continue;
				if(j<0 || j>=ycell)continue;
				const vector<int>& cell = cells[j*xcell+i];
				for(int k=0;k<(int)cell.size();k++)
				{
					int indpt = cell[k];
					float distpt2 = sqrt(powf(ptlist[indpt].x - ptlist[oldpos].x, 2) + powf(ptlist[indpt].y - ptlist[oldpos].y, 2));
					if(distpt2<distpt && distpt2<mindistpt)
					{
						mindistpt = distpt2;
						imindist = k;
						icellmatch = i;
						jcellmatch = j;
					}
				}
			}
		}
		if(imindist != -1)
		{
			ptpos = cells[icellmatch + jcellmatch*xcell][imindist];
		}
	}
}

void CLicenseRecog::SetupCells(CvPoint* ptlist, int npt, const BOX& box, int xcell, int ycell, vector<int>* &cells)
{
	cells = new vector<int>[xcell*ycell];
	for(int i=0;i<npt;i++)
	{
		CvPoint pt = ptlist[i];
		int icell = (int)floor((pt.x-box.left)/(box.right-box.left)*xcell);
		int jcell = (int)floor((pt.y-box.top)/(box.bot-box.top)*ycell);
		if(icell<0)icell = 0;
		else if(icell>=xcell)icell = xcell-1;
		if(jcell<0)jcell = 0;
		else if(jcell>=ycell)jcell = ycell-1;
		cells[jcell*xcell+icell].push_back(i);
	}
}

#ifdef REGRESSION_DEBUG 
void CLicenseRecog::DrawSpots(CDC* pDC, int index)
{
	if(m_Assign == NULL)
		return;
	if(index<0 || index>=(int)m_Comps.size())
		return;
	vector<CvPoint>* contour = m_Comps[index];
	CRect rcPlotBase(-3,-3,3,3);

	for(int i=0;i<(int)contour->size();i++)
	{
		if(m_Assign[i] != NULL)
		{
			CPoint pt2(contour->at(i).x, contour->at(i).y);
			CRect rcPlot2 = rcPlotBase+pt2;
			pDC->MoveTo(rcPlot2.TopLeft());
			pDC->LineTo(rcPlot2.BottomRight());
			pDC->MoveTo(rcPlot2.right, rcPlot2.top);
			pDC->LineTo(rcPlot2.left, rcPlot2.bottom);
		}
	}
}
#endif
