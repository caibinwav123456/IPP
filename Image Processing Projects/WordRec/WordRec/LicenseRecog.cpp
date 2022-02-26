#include "stdafx.h"
#include "LicenseRecog.h"
#include "stdlib.h"
#include "search.h"

struct IterParam
{
	CLicenseRecog* pInst;
	int* nUsageCnt;
};

CLicenseRecog::CLicenseRecog()
{
	m_src = NULL;
	m_bin = NULL;
	m_refine = NULL;
	m_imgid = NULL;
	m_seg = NULL;
	m_nComp = 0;
	m_storage = cvCreateMemStorage();
	m_LineCells = NULL;
	m_xcellline = 0;
	m_ycellline = 0;
#ifdef REGRESSION_DEBUG
	m_ptlist = NULL;
	m_npt = 0;
	m_xcell = 0;
	m_ycell = 0;
	m_cells = NULL;
	m_Assign = NULL;
	m_curline = NULL;
	m_linelist = NULL;
	m_ptpos = 0;
#endif
}

CLicenseRecog::~CLicenseRecog()
{
	Release();
	cvReleaseMemStorage(&m_storage);
}

void CLicenseRecog::Release()
{
	cvReleaseImage(&m_src);
	cvReleaseImage(&m_bin);
	cvReleaseImage(&m_refine);
	cvReleaseImage(&m_imgid);
	cvReleaseImage(&m_seg);
	Reset();
}

void CLicenseRecog::Reset()
{
	while((int)m_Comps.size()>0)
	{
		vector<CvPoint>* comp = m_Comps.back();
		m_Comps.pop_back();
		delete comp;
	}
	while((int)m_lines.size()>0)
	{
		Line* line = m_lines.back();
		int nline = m_nlines.back();

		m_lines.pop_back();
		m_nlines.pop_back();

		delete[] line;
	}

	while((int)m_reflines.size()>0)
	{
		Line* line = m_reflines.back();
		int nline = m_nreflines.back();

		m_reflines.pop_back();
		m_nreflines.pop_back();

		delete[] line;
	}
	m_AllLines.clear();
	if(m_LineCells)
	{
		delete[] m_LineCells;
		m_LineCells = NULL;
	}
	m_xcellline = 0;
	m_ycellline = 0;
#ifdef REGRESSION_DEBUG
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
	m_xcell = 0;
	m_ycell = 0;
	if(m_ptlist)
	{
		delete[] m_ptlist;
		m_ptlist = NULL;
	}
#endif
}

void CLicenseRecog::LoadImage(char* name)
{
	Release();
	m_src = cvLoadImage(name);
	m_bin = cvCreateImage(cvGetSize(m_src), IPL_DEPTH_8U, 1);
	m_refine = cvCreateImage(cvGetSize(m_src), IPL_DEPTH_8U, 1);
	m_imgid = cvCreateImage(cvGetSize(m_src), IPL_DEPTH_32F, 1);
	int width = (m_src->width | 15)+1;
	int height = (m_src->height | 15)+1;
	m_seg = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
}

void CLicenseRecog::LoadImage(IplImage* image)
{
	Release();
	m_src = cvCloneImage(image);
	m_bin = cvCreateImage(cvGetSize(m_src), IPL_DEPTH_8U, 1);
	m_refine = cvCreateImage(cvGetSize(m_src), IPL_DEPTH_8U, 1);
	m_imgid = cvCreateImage(cvGetSize(m_src), IPL_DEPTH_32F, 1);
	int width = (m_src->width | 15)+1;
	int height = (m_src->height | 15)+1;
	m_seg = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
}

void CLicenseRecog::Threshold()
{
	if(!m_src)return;
	IplImage* gray = cvCreateImage(cvGetSize(m_src), IPL_DEPTH_8U, 1);
	cvCvtColor(m_src, gray, CV_RGB2GRAY);
	float thresh = ComputeThreshValue(gray);
	cvThreshold(gray, m_bin, thresh, 0xff, CV_THRESH_BINARY);
	cvReleaseImage(&gray);
}

void CLicenseRecog::Segment()
{
	if(!m_src)
		return;

	IplImage* gray = cvCreateImage(cvGetSize(m_src), IPL_DEPTH_8U, 1);
	cvCvtColor(m_src, gray, CV_RGB2GRAY);

	IplImage* tmp = cvCreateImage(cvGetSize(m_seg), IPL_DEPTH_8U, 1);
	cvResize(gray, tmp);
	cvPyrSegmentation(tmp, m_seg, m_storage, &m_Segments, 4, 50, 20);
	cvReleaseImage(&gray);
	cvReleaseImage(&tmp);

	for(int i = 0; i < m_Segments->total; i++)
	{
		CvConnectedComp* comp = CV_GET_SEQ_ELEM(CvConnectedComp, m_Segments, i);
	}
}

float CLicenseRecog::ComputeThreshValue(IplImage* image)
{
	if(!image)return 0;
	int nHist[256];
	for(int i=0;i<256;i++)
	{
		nHist[i] = 0;
	}

	for(int i=0;i<image->height;i++)
	{
		for(int j=0;j<image->width;j++)
		{
			nHist[*PTR_PIX(*image, j, i)]++;
		}
	}
	float fSum = 0.0f;
	for(int i=0;i<256;i++)
	{
		fSum+=i*nHist[i];
	}

	float sum = fSum / image->width / image->height;
	return sum;
}

void CLicenseRecog::Refine()
{
	if(!m_src)return;		
	CvMat* kernel = cvCreateMat(3,3,CV_32FC1);
	cvSet(kernel, cvScalar(1));
	cvFilter2D(m_bin, m_refine, kernel);
	cvReleaseMat(&kernel);
}

void CLicenseRecog::IdentifyRegion()
{
	if(!m_src)return;
	cvZero(m_imgid);
	cvSet(m_imgid, cvScalar(-1), m_refine);
	int id = 0;
	for(int i=0;i<m_imgid->height;i++)
	{
		for(int j=0;j<m_imgid->width;j++)
		{
			if(*((float*)PTR_PIX(*m_imgid, j, i)) == -1)
			{
				cvFloodFill(m_imgid, cvPoint(j,i), cvScalar(id));
				id++;
			}
		}
	}
	m_nComp = id;
}

void CLicenseRecog::FindComps(int nAlg)
{
	if(nAlg == FIND_COMPS_ALGORITHM1)
	{
		Refine();
		if(!m_refine)return;
		IplImage* image = cvCloneImage(m_refine);
		m_nContour = cvFindContours(image, m_storage, &m_Contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
		m_nContour = 0;
		for(CvSeq* pos = m_Contours; pos != NULL; pos = pos->h_next)
		{
			if(pos->total>=30)
			{
				vector<CvPoint>* contour = new vector<CvPoint>;
				for(int j=0;j<pos->total;j++)
					contour->push_back(*CV_GET_SEQ_ELEM(CvPoint, pos, j));
				m_Comps.push_back(contour);
				m_nContour++;
			}
		}
		cvReleaseImage(&image);
	}
	else if(nAlg == FIND_COMPS_ALGORITHM2)
	{
		Segment();
		if(!m_seg)return;
		IplImage* mask = cvCreateImage(cvGetSize(m_seg), IPL_DEPTH_8U, 1);
		IplImage* flag = cvCreateImage(cvGetSize(m_seg), IPL_DEPTH_8U, 1);
		cvZero(mask);
		cvZero(flag);
		IplImage* tmp = cvCloneImage(mask);
		for(int i=0;i<m_seg->height;i++)
		{
			for(int j=0;j<m_seg->width;j++)
			{
				uchar* pix = PTR_PIX(*m_seg, j, i);
				uchar* pflag = PTR_PIX(*flag, j, i);
				if(*pflag == 0)
				{
					cvCmpS(m_seg, *pix, mask, CV_CMP_EQ);
					cvCopyImage(mask, tmp);
					m_nContour = cvFindContours(tmp, m_storage, &m_Contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
					CvSeq* pos = m_Contours;
					bool bIterBack = false;
					while(true)
					{
						if(pos->total >= 30)
						{
							vector<CvPoint>* contour = new vector<CvPoint>;
							for(int k=0;k<pos->total;k++)
							{
								CvPoint* point = CV_GET_SEQ_ELEM(CvPoint, pos, k);
								CvPoint  pttrans = cvPoint(point->x*m_src->width/m_seg->width, point->y*m_src->height/m_seg->height);
								contour->push_back(pttrans);
							}
							m_Comps.push_back(contour);
						}

						bool bBack = bIterBack;
						bIterBack = false;
						if(pos->v_next && !bBack)
							pos = pos->v_next;
						else if(pos->h_next)
							pos = pos->h_next;
						else if(pos->v_prev)
						{
							pos = pos->v_prev;
							bIterBack = true;
						}
						else
							break;
					}
					cvOr(flag, mask, flag);
				}
			}
		}
		cvReleaseImage(&tmp);
		cvReleaseImage(&mask);
		cvReleaseImage(&flag);
	}
}

void CLicenseRecog::DrawComps(CDC* pDC, CPoint ptBase)
{
	for(int i=0;i<(int)m_Comps.size();i++)
	{
		vector<CvPoint>* comp = m_Comps[i];
		if((int)(comp->size())>0)
		{
			pDC->MoveTo(comp->at(comp->size()-1).x+ptBase.x,comp->at(comp->size()-1).y+ptBase.y);
			for(int j=0;j<(int)(comp->size());j++)
			{
				CvPoint point = comp->at(j);
				pDC->LineTo(point.x+ptBase.x, point.y+ptBase.y);
			}
		}
	}
}

void CLicenseRecog::DrawComps(CDC* pDC, int index, CPoint ptBase, bool bPlot)
{
	if(index<0 || index>=(int)m_Comps.size())
		return;

	vector<CvPoint>* contour = m_Comps[index];
	if((int)contour->size()>0)
	{
		CPoint pt(contour->at(contour->size()-1).x, contour->at(contour->size()-1).y);
		pDC->MoveTo(pt);
		for(int i=0;i<(int)contour->size();i++)
		{
			CPoint pt2(contour->at(i).x, contour->at(i).y);
			pDC->LineTo(pt2);
		}
		if(bPlot)
		{
			CRect rcPlotBase(-3,-3,3,3);
			for(int i=0;i<(int)contour->size();i++)
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
}

int CLicenseRecog::IndexFromPoint(CPoint pt)
{
	for(int i=0;i<(int)m_Comps.size();i++)
	{
		vector<CvPoint>* contour = m_Comps[i];
		for(int j=0;j<(int)contour->size();j++)
		{
			CvPoint ptHit = contour->at(j);
			if(sqrt(pow((float)(pt.x-ptHit.x),2)+pow((float)(pt.y-ptHit.y),2))<3)
			{
				return i;
			}
		}
	}
	return -1;
}

CvPoint2D32f CLicenseRecog::FindContourCenter(int index, float* maxdiff)
{
	if(index<-1 || index >= (int)m_Comps.size())
		return cvPoint2D32f(0,0);

	CvPoint2D32f acc = cvPoint2D32f(0,0);
	if(index!=-1)
	{
		vector<CvPoint>* contour = m_Comps[index];

		for(int i=0;i<(int)contour->size();i++)
		{
			acc.x += contour->at(i).x;
			acc.y += contour->at(i).y;
		}
		acc.x/=(int)contour->size();
		acc.y/=(int)contour->size();

		if(maxdiff)
		{
			*maxdiff = 0;
			for(int i=0;i<(int)contour->size();i++)
			{
				float dist = sqrt(pow(contour->at(i).x-acc.x, 2)+pow(contour->at(i).y-acc.y, 2));
				if(dist>*maxdiff)
					*maxdiff = dist;
			}
		}
	}
	else
	{
		int count = 0;
		for(int n=0;n<(int)m_Comps.size();n++)
		{
			vector<CvPoint>* contour = m_Comps[n];

			for(int i=0;i<(int)contour->size();i++)
			{
				acc.x += contour->at(i).x;
				acc.y += contour->at(i).y;
			}
			count+=(int)contour->size();
		}
		acc.x/=count;
		acc.y/=count;

		if(maxdiff)
		{
			*maxdiff = 0;
			for(int n=0; n<(int)m_Comps.size(); n++)
			{
				vector<CvPoint>* contour = m_Comps[n];
				for(int i=0;i<(int)contour->size();i++)
				{
					float dist = sqrt(pow(contour->at(i).x-acc.x, 2)+pow(contour->at(i).y-acc.y, 2));
					if(dist>*maxdiff)
						*maxdiff = dist;
				}
			}
		}
	}

	return acc;
}

IplImage* CLicenseRecog::HoughImage(int index, CvPoint2D32f* ptCenter, IplImage** imgstartend)
{
	if(index<-1 || index>=(int)m_Comps.size())
		return NULL;

	float maxdist = 0;
	CvPoint2D32f center = FindContourCenter(index, &maxdist);

	if(ptCenter)
		*ptCenter = center;

	IplImage* imgHough = cvCreateImage(cvSize(128, 2*((int)floor(maxdist)+1)), IPL_DEPTH_32F, 1);

	if(imgstartend)
	{
		*imgstartend = cvCreateImage(cvGetSize(imgHough), IPL_DEPTH_32F, 2);
		cvSet(*imgstartend, cvScalar(1.0e10, -1.0e10));
	}

	cvZero(imgHough);

	if(index!=-1)
	{
		vector<CvPoint>* contour = m_Comps[index];

		for(int i=0;i<(int)contour->size();i++)
		{
			CvPoint pt = contour->at(i);
			pt.x-=center.x;
			pt.y-=center.y;
			for(int j=0;j<128;j++)
			{
				float theta = PI*j/128;
				float dist = pt.x*cos(theta)+pt.y*sin(theta);
				int deb = (int)floor(dist)+imgHough->height/2;
				if(deb<0||deb>=imgHough->height)
				{
					int o=0;
					o++;
				}
				(*(float*)PTR_PIX(*imgHough, j, (int)floor(dist)+imgHough->height/2))++;
				if(imgstartend)
				{
					float coord = -pt.x*sin(theta)+pt.y*cos(theta);
					float& mincoord = *(float*)PTR_PIX(**imgstartend, j, (int)floor(dist)+imgHough->height/2);
					float& maxcoord = *((float*)PTR_PIX(**imgstartend, j, (int)floor(dist)+imgHough->height/2)+1);
					if(coord<mincoord)
						mincoord = coord;
					if(coord>maxcoord)
						maxcoord = coord;
				}
			}
		}
	}
	else
	{
		for(int n=0; n<(int)m_Comps.size(); n++)
		{
			vector<CvPoint>* contour = m_Comps[n];

			for(int i=0;i<(int)contour->size();i++)
			{
				CvPoint pt = contour->at(i);
				pt.x-=center.x;
				pt.y-=center.y;
				for(int j=0;j<128;j++)
				{
					float theta = PI*j/128;
					float dist = pt.x*cos(theta)+pt.y*sin(theta);
					int deb = (int)floor(dist)+imgHough->height/2;
					if(deb<0||deb>=imgHough->height)
					{
						int o=0;
						o++;
					}
					(*(float*)PTR_PIX(*imgHough, j, (int)floor(dist)+imgHough->height/2))++;
					if(imgstartend)
					{
						float coord = -pt.x*sin(theta)+pt.y*cos(theta);
						float& mincoord = *(float*)PTR_PIX(**imgstartend, j, (int)floor(dist)+imgHough->height/2);
						float& maxcoord = *((float*)PTR_PIX(**imgstartend, j, (int)floor(dist)+imgHough->height/2)+1);
						if(coord<mincoord)
							mincoord = coord;
						if(coord>maxcoord)
							maxcoord = coord;
					}
				}
			}
		}
	}
	return imgHough;
}
#define RADIUS 0
void CLicenseRecog::FindLines(int index, Line** buffer, int* nLine, float thresh1, float thresh2)
{
	if(index<0 || index>=(int)m_Comps.size())
		return;

	CvPoint2D32f center;

	IplImage* imgends = NULL;
	IplImage* imgHough = HoughImage(index, &center, &imgends);

	IplImage* integ = cvCreateImage(cvSize(imgHough->width+1, imgHough->height+1), IPL_DEPTH_64F, 1);
	cvIntegral(imgHough, integ);

	vector<CvPoint> candidates;
	double accmax = 0;
	for(int i=0;i<imgHough->width;i++)
	{
		for(int j=0;j<imgHough->height;j++)
		{
			int lj = j-RADIUS;
			int hj = j+RADIUS+1;
			if(lj<0)lj=0;
			if(hj>imgHough->height)
				hj = imgHough->height;

			double acc=0;
			if(i>=RADIUS && i<imgHough->width-RADIUS)
			{
				int li = i-RADIUS;
				int hi = i+RADIUS+1;

				double tl = *(double*)PTR_PIX(*integ, li, lj);
				double tr = *(double*)PTR_PIX(*integ, hi, lj);
				double bl = *(double*)PTR_PIX(*integ, li, hj);
				double br = *(double*)PTR_PIX(*integ, hi, hj);

				acc = br-tr-bl+tl;
			}
			else
			{
				int li = i-RADIUS;
				if(li<0)li = imgHough->width+li;
				int hi = i+RADIUS+1;
				if (hi>imgHough->width)hi = hi-imgHough->width;

				double tl1 = *(double*)PTR_PIX(*integ, 0, lj);
				double tr1 = *(double*)PTR_PIX(*integ, hi,lj);
				double bl1 = *(double*)PTR_PIX(*integ, 0, hj);
				double br1 = *(double*)PTR_PIX(*integ, hi,hj);

				double tl2 = *(double*)PTR_PIX(*integ, li, lj);
				double tr2 = *(double*)PTR_PIX(*integ, imgHough->width, lj);
				double bl2 = *(double*)PTR_PIX(*integ, li, hj);
				double br2 = *(double*)PTR_PIX(*integ, imgHough->width, hj);

				acc = (br1-tr1-bl1+tl1)+(br2-tr2-bl2+tl2);
			}

			double threshold;
			if(index!=-1)
				threshold = min(thresh1*(int)m_Comps[index]->size(), thresh2);
			else
				threshold = thresh1;
			if(acc>threshold)// 0.08 20
			{
				candidates.push_back(cvPoint(i,j-imgHough->height/2));
			}
		}
	}
	vector<CvPoint> lines;
	vector<float> starts;
	vector<float> ends;
	vector<float> confs;

	for(int i=0;i<(int)candidates.size();i++)
	{
		CvPoint pt = candidates[i];
		CvPoint ptChg = pt;
		do 
		{
			pt = ptChg;

			CvPoint ptlow = cvPoint(pt.x-RADIUS,pt.y-RADIUS);
			CvPoint pthigh = cvPoint(pt.x+RADIUS+1,pt.y+RADIUS+1);

			float n = 0;
			float accx = 0;
			float accy = 0;
			for(int j=ptlow.x;j<pthigh.x;j++)
			{
				for(int k=ptlow.y;k<pthigh.y;k++)
				{
					int tj = j;
					if(tj<0)tj+=imgHough->width;
					if(tj>=imgHough->width)tj-=imgHough->width;

					int tk = k;
					if(tk<-imgHough->height/2)continue;
					if(tk>=imgHough->height/2)continue;

					float v = *(float*)PTR_PIX(*imgHough, tj, tk+imgHough->height/2);
					n+=v;
					accx+=j*v;
					accy+=k*v;
				}
			}
			float meanx = accx/n;
			float meany = accy/n;

			ptChg = cvPoint(cvRound(meanx), cvRound(meany));

			if(ptChg.x<0)ptChg.x+=imgHough->width;
			if(ptChg.x>=imgHough->width)ptChg.x-=imgHough->width;
			if(ptChg.y<-imgHough->height/2)ptChg.y = -imgHough->height/2;
			if(ptChg.y>=imgHough->height/2)ptChg.y=imgHough->height/2-1;
		}
		while(ptChg.x != pt.x || ptChg.y != pt.y);

		CvPoint ptlow = cvPoint(pt.x-RADIUS,pt.y-RADIUS);
		CvPoint pthigh = cvPoint(pt.x+RADIUS+1,pt.y+RADIUS+1);

		float minpro = 1.0e10, maxpro = -1.0e10;
		float confidence = 0;
		for(int j=ptlow.x;j<pthigh.x;j++)
		{
			for(int k=ptlow.y;k<pthigh.y;k++)
			{
				int tj = j;
				if(tj<0)tj+=imgHough->width;
				if(tj>=imgHough->width)tj-=imgHough->width;

				int tk = k;
				if(tk<-imgHough->height/2)continue;
				if(tk>=imgHough->height/2)continue;

				float mincoord = *(float*)PTR_PIX(*imgends, tj, tk+imgHough->height/2);
				float maxcoord = *((float*)PTR_PIX(*imgends, tj, tk+imgHough->height/2)+1);
				float v = *(float*)PTR_PIX(*imgHough, tj, tk+imgHough->height/2);
				confidence+=v;

				if(mincoord < minpro)
					minpro = mincoord;
				if(maxcoord > maxpro)
					maxpro = maxcoord;
			}
		}

		lines.push_back(pt);
		starts.push_back(minpro);
		ends.push_back(maxpro);
		confs.push_back(confidence);
	}
	*buffer = new Line[lines.size()];
	for(int i=0;i<(int)lines.size();i++)
	{
		Line line(lines[i].x*PI/128, lines[i].y, starts[i], ends[i]);
		line.dist+=center.x*cos(line.theta)+center.y*sin(line.theta);
		line.start+=-center.x*sin(line.theta)+center.y*cos(line.theta);
		line.end+=-center.x*sin(line.theta)+center.y*cos(line.theta);
		line.conf = confs[i];
		(*buffer)[i] = line;
	}
	*nLine = (int)lines.size();

	cvReleaseImage(&imgHough);
	cvReleaseImage(&imgends);
	cvReleaseImage(&integ);
}

void CLicenseRecog::FindLines(int nAlg, float thresh1, float thresh2)
{
	if(nAlg == FIND_LINES_ALGORITHM1)
	{
		while((int)m_lines.size()>0)
		{
			Line* line = m_lines.back();
			int nline = m_nlines.back();

			m_lines.pop_back();
			m_nlines.pop_back();

			delete[] line;
		}

		if((int)m_Comps.size() == 0)
			return;

		for(int i=0;i<(int)m_Comps.size();i++)
		{
			Line* line = NULL;
			int nline = 0;
			FindLines(i, &line, &nline, thresh1, thresh2);
			m_lines.push_back(line);
			m_nlines.push_back(nline);
		}
		RefineLines();
	}
	else if(nAlg == FIND_LINES_ALGORITHM2)
	{
		for(int i=0;i<(int)m_Comps.size();i++)
		{
			vector<CvPoint>* comp = m_Comps[i];
			int npt = (int)comp->size();
			CvPoint* ptlist = new CvPoint[npt];
			for(int j=0;j<npt;j++)
			{
				ptlist[j] = comp->at(j);
			}
			Line* line = NULL;
			int nline = 0;
			LineRegression(ptlist, npt, &line, &nline);
			m_reflines.push_back(line);
			m_nreflines.push_back(nline);
			delete[] ptlist;
		}
	}
}

void CLicenseRecog::RefineLines()
{
	for(int i=0;i<(int)m_lines.size();i++)
	{
		Line* line = m_lines[i];
		int nline = m_nlines[i];
		qsort(line, nline, sizeof(Line), LineCompare);

		int nlimit = min(nline, nline);//40

		int* flags = new int[nlimit];
		for(int j=0;j<nlimit;j++)
		{
			flags[j] = 0;
		}
		vector<Line> reflines;
		while(true)
		{
			Line refline;
			int  ncounted = 0;
			for(int j=0;j<nlimit;j++)
			{
				if(!(flags[j]&(1|4)))
				{
					ncounted = 1;
					refline = line[j];
					flags[j]|=4;
					break;
				}
			}
			if(ncounted == 0)
				break;

			Line newrefline = refline;
			ncounted = 0;
			do 
			{
				refline = newrefline;
				newrefline = Line();
				ncounted = 0;
				float minstart = 1.0e10;
				float maxend = -1.0e10;
				for(int j=0;j<nlimit;j++)
				{
					if(!(flags[j]&1))
					{
						Line linepick = line[j];
						float tdiff = linepick.theta-refline.theta;
						float ddiff = linepick.dist-refline.dist;

						float diff = sqrt(pow(tdiff*128,2) + pow(ddiff,2));

						if(diff<30)
						{
							flags[j]|=2;
							newrefline.theta += linepick.theta;
							newrefline.dist += linepick.dist;
							ncounted++;
						}
						else
						{
							flags[j]&=(~2);
						}
					}
				}
				newrefline.theta/=ncounted;
				newrefline.dist/=ncounted;

				for(int j=0;j<nlimit;j++)
				{
					if(!(flags[j]&1) && (flags[j]&2))
					{
						CvPoint2D32f ptstart = cvPoint2D32f(line[j].dist*cos(line[j].theta) - line[j].start*sin(line[j].theta),
							                                line[j].dist*sin(line[j].theta) + line[j].start*cos(line[j].theta));
						CvPoint2D32f ptend = cvPoint2D32f(line[j].dist*cos(line[j].theta) - line[j].end*sin(line[j].theta),
							                              line[j].dist*sin(line[j].theta) + line[j].end*cos(line[j].theta));
						float start = -ptstart.x*sin(newrefline.theta)+ptstart.y*cos(newrefline.theta);
						float end = -ptend.x*sin(newrefline.theta)+ptend.y*cos(newrefline.theta);
						if(start<minstart)minstart = start;
						if(end>maxend)maxend = end;
					}
				}
				newrefline.start = minstart;
				newrefline.end = maxend;
			} while (newrefline.theta != refline.theta || 
				     newrefline.dist != refline.dist ||
					 newrefline.start != refline.start ||
					 newrefline.end != refline.end);
			for(int j=0;j<nlimit;j++)
			{
				if(flags[j]&2)
				{
					flags[j]&=(~2);
					flags[j]|=1;
				}
			}
			reflines.push_back(refline);
		}
		int nrefline = (int)reflines.size();
		Line* refline_arr = new Line[nrefline];
		for(int j=0;j<nrefline;j++)
		{
			refline_arr[j] = reflines[j];
		}
		m_reflines.push_back(refline_arr);
		m_nreflines.push_back(nrefline);
		delete[] flags;
	}
}

int CLicenseRecog::LineCompare(const void* a, const void* b)
{
	Line* linea = (Line*)a;
	Line* lineb = (Line*)b;

	if(linea->conf<lineb->conf)
		return 1;
	else if(linea->conf>lineb->conf)
		return -1;
	else
		return 0;
}


#define CELL_SIZE_OVERALL  150
int CLicenseRecog::ProcessAllLines()
{
	m_AllLines.clear();
	if(m_LineCells)
	{
		delete[] m_LineCells;
		m_LineCells = NULL;
	}
	m_xcellline = m_ycellline = 0;
	for(int i=0;i<(int)m_reflines.size();i++)
	{
		Line* line = m_reflines[i];
		int   nline = m_nreflines[i];
		for(int j=0;j<nline;j++)
		{
			m_AllLines.push_back(line[j]);
		}
	}
	int nAdj = CELL_SIZE_OVERALL;
	BOX box = cvGetSize(m_src);
	m_xcellline = (int)floor((box.right-box.left)/nAdj);
	m_ycellline = (int)floor((box.bot-box.top)/nAdj);
	if(m_xcellline < 1)m_xcellline = 1;
	if(m_ycellline < 1)m_ycellline = 1;
	m_LineCells = new vector<int>[m_xcellline*m_ycellline];

	CategorizeLines();

	OverAllRefineLines();

	return OverAllConnect();
}

void CLicenseRecog::OverAllRefineLines()
{
	CvPoint Adj[9] = {{0,0}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1}};
	bool* occupy = new bool[m_xcellline*m_ycellline];
	for(int i=0;i<m_xcellline*m_ycellline;i++)
	{
		occupy[i] = false;
	}
	vector<CvPoint> category;
	int szOrg = m_AllLines.size();
	bool* bEliminate = new bool[szOrg];
	for(int i=0; i<szOrg; i++)
	{
		bEliminate[i] = false;
	}
	for(int i=0; i<szOrg; i++)
	{
		Line line = m_AllLines[i];
		CategorizeLine(line, 0, &category);
		for(int j=0;j<(int)category.size();j++)
		{
			CvPoint cellcoord = category[j];
			for(int k=0;k<9;k++)
			{
				CvPoint adj = cvPoint(cellcoord.x+Adj[k].x, cellcoord.y+Adj[k].y);
				if(adj.x<0 || adj.x>=m_xcellline ||
					adj.y<0 || adj.y>=m_ycellline)
					continue;
				if(occupy[adj.y*m_xcellline+adj.x])
					continue;
				vector<int>* adjline = m_LineCells + adj.y*m_xcellline+adj.x;
				for(int l=0; l<(int)adjline->size(); l++)
				{
					int comple = adjline->at(l);

					if(comple <= i)
						continue;

					Line cmpline = m_AllLines[comple];
					
					Vec2 ls, le, cls, cle;
					line.GetEndPoint(&ls, &le);
					cmpline.GetEndPoint(&cls, &cle);
					if((fabs(ls.x-cls.x)<=1 && fabs(ls.y-cls.y)<=1
						&& fabs(le.x-cle.x)<=1 && fabs(le.y-cle.y)<=1)
						|| (fabs(ls.x-cle.x)<=1 && fabs(ls.y-cle.y)<=1
						&& fabs(le.x-cls.x)<=1 && fabs(le.y-cls.y)<=1))
					{
						//m_AllLines.erase(m_AllLines.begin()+comple);
						bEliminate[comple] = true;
					}
				}
				occupy[adj.y*m_xcellline+adj.x] = true;
			}
		}
		for(int j=0;j<(int)category.size();j++)
		{
			CvPoint cellcoord = category[j];
			for(int k=0;k<9;k++)
			{
				CvPoint adj = cvPoint(cellcoord.x+Adj[k].x, cellcoord.y+Adj[k].y);
				if(adj.x<0 || adj.x>=m_xcellline ||
					adj.y<0 || adj.y>=m_ycellline)
					continue;
				occupy[adj.y*m_xcellline+adj.x] = false;
			}
		}
	}
	for(int i1=0,i2=0; i1<(int)m_AllLines.size() && i2<szOrg; i2++)
	{
		if(bEliminate[i2])
		{
			m_AllLines.erase(m_AllLines.begin()+i1);
		}
		else
		{
			i1++;
		}
	}
	CategorizeLines();
	delete[] occupy;
	delete[] bEliminate;
}

void CLicenseRecog::CategorizeLines()
{
	for(int i=0;i<m_xcellline*m_ycellline;i++)
	{
		m_LineCells[i].clear();
	}
	for(int i=0;i<(int)m_AllLines.size();i++)
	{
		Line line = m_AllLines[i];
		CategorizeLine(line, i);
	}
}

void CLicenseRecog::CategorizeLine(const Line& line, int ndata, vector<CvPoint>* cate)
{
	Vec2 ptstart(line.dist*cos(line.theta)-line.start*sin(line.theta),
				 line.dist*sin(line.theta)+line.start*cos(line.theta));
	Vec2 ptend(line.dist*cos(line.theta)-line.end*sin(line.theta),
			   line.dist*sin(line.theta)+line.end*cos(line.theta));

	BOX box = cvGetSize(m_src);
	if(ptstart.x>ptend.x)
	{
		Vec2 tmp = ptstart;
		ptstart = ptend;
		ptend = tmp;
	}

	float sx = (ptstart.x-box.left)/(box.right-box.left)*m_xcellline;
	float sy = (ptstart.y-box.top)/(box.bot-box.top)*m_ycellline;
	float ex = (ptend.x-box.left)/(box.right-box.left)*m_xcellline;
	float ey = (ptend.y-box.top)/(box.right-box.left)*m_ycellline;

	int minx = (int)floor(sx);
	int maxx = (int)floor(ex);
	if(minx<0)minx=0;
	if(maxx>=m_xcellline)maxx = m_xcellline-1;
	if(cate)
		cate->clear();
	for(int i=minx;i<=maxx;i++)
	{
		float lcross, rcross;
		lcross = sy+(ey-sy)/(ex-sx)*(i-sx);
		rcross = sy+(ey-sy)/(ex-sx)*(i+1-sx);
		if(i == minx)
		{
			lcross = sy;
		}
		if(i == maxx)
		{
			rcross = ey;
		}
		int miny = (int)floor(lcross);
		int maxy = (int)floor(rcross);
		if(miny>maxy)
		{
			int tmp = miny;
			miny = maxy;
			maxy = tmp;
		}
		if(miny<0)miny=0;
		if(maxy>=m_ycellline)maxy = m_ycellline-1;
		for(int j=miny;j<=maxy;j++)
		{
			if(cate)
			{
				cate->push_back(cvPoint(i,j));
			}
			else
			{
				m_LineCells[j*m_xcellline+i].push_back(ndata);
			}
		}
	}
}

void CLicenseRecog::IterAllLines(void (*pIterFunc)(int, int, void*), void* param)
{
	CvPoint Adj[9] = {{0,0}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1}};
	bool* occupy = new bool[m_xcellline*m_ycellline];
	for(int i=0;i<m_xcellline*m_ycellline;i++)
	{
		occupy[i] = false;
	}
	vector<CvPoint> category;
	int szOrigin = (int)m_AllLines.size();

	for(int i=0;i<szOrigin;i++)
	{
		Line line = m_AllLines[i];
		CategorizeLine(line, 0, &category);
		for(int j=0;j<(int)category.size();j++)
		{
			CvPoint cellcoord = category[j];
			for(int k=0;k<9;k++)
			{
				CvPoint adj = cvPoint(cellcoord.x+Adj[k].x, cellcoord.y+Adj[k].y);
				if(adj.x<0 || adj.x>=m_xcellline ||
					adj.y<0 || adj.y>=m_ycellline)
					continue;
				if(occupy[adj.y*m_xcellline+adj.x])
					continue;
				vector<int>* adjline = m_LineCells + adj.y*m_xcellline+adj.x;
				for(int l=0; l<(int)adjline->size(); l++)
				{
					int comple = adjline->at(l);

					if(comple < i)
						continue;

					pIterFunc(i, comple, param);
				}
				occupy[adj.y*m_xcellline+adj.x] = true;
			}
		}
		for(int j=0;j<(int)category.size();j++)
		{
			CvPoint cellcoord = category[j];
			for(int k=0;k<9;k++)
			{
				CvPoint adj = cvPoint(cellcoord.x+Adj[k].x, cellcoord.y+Adj[k].y);
				if(adj.x<0 || adj.x>=m_xcellline ||
					adj.y<0 || adj.y>=m_ycellline)
					continue;
				occupy[adj.y*m_xcellline+adj.x] = false;
			}
		}
	}
	delete[] occupy;
}

int CLicenseRecog::OverAllConnect()
{
	int szOrg = (int)m_AllLines.size();
	int* nUsageCnt = new int[szOrg];
	for(int i=0;i<szOrg;i++)
	{
		nUsageCnt[i] = 0;
	}

	IterParam param;

	param.pInst = this;
	param.nUsageCnt = nUsageCnt;
	IterAllLines(_OverAllConnect, &param);

	delete[] nUsageCnt;

	for(int i=szOrg;i<(int)m_AllLines.size();i++)
	{
		Line line = m_AllLines[i];
		CategorizeLine(line, i);
	}
	return (int)m_AllLines.size() - szOrg;
}

#define MAX_ANGLE_DIFF_OVERALL  (PI/16)
#define MAX_DIST_DIFF_OVERALL   3.5
#define MAX_ENDS_DIFF_OVERALL   20
#define MIN_LENGTH_CONN         50
#define MAX_USAGE_CONNECT       2
void CLicenseRecog::_OverAllConnect(int lindex1, int lindex2, void* param)
{
	IterParam* iparam = (IterParam*)param;
	CLicenseRecog* pInst = iparam->pInst;
	int* nUsageCnt = iparam->nUsageCnt;

	if(nUsageCnt[lindex1]>=MAX_USAGE_CONNECT
		|| nUsageCnt[lindex2]>=MAX_USAGE_CONNECT)
	{
		return;
	}

	Line line1 = pInst->m_AllLines[lindex1];
	Line line2 = pInst->m_AllLines[lindex2];

	float thetadiff = line1.theta - line2.theta;
	if(fabs(fmod(thetadiff/PI+0.5, 1.0)-0.5) < MAX_ANGLE_DIFF_OVERALL)
	{
		Vec2 v1(-sin(line1.theta), cos(line1.theta));
		Vec2 v2(-sin(line2.theta), cos(line2.theta));
		Vec2 n1(cos(line1.theta), sin(line1.theta));
		Vec2 n2(cos(line1.theta), sin(line2.theta));

		Vec2 pts1 = n1*line1.dist+v1*line1.start;
		Vec2 pte1 = n1*line1.dist+v1*line1.end;
		Vec2 pts2 = n2*line2.dist+v2*line2.start;
		Vec2 pte2 = n2*line2.dist+v2*line2.end;

		bool bInvert = false;
		if(fmod(thetadiff/PI/2+0.25, 1.0) > 0.5)
		{
			bInvert = true;
		}

		Vec2 v = (v1 + (bInvert?-v2:v2)).normalize();
		Vec2 normal(v.y, -v.x);
		Line lcomb;
		lcomb.theta = atan(normal.y/normal.x);
		if(normal.x == 0.0)
		{
			if(normal.y > 0)
			{
				lcomb.theta = PI/2;				
			}
			else if(normal.y < 0)
			{
				lcomb.theta = -PI/2;
			}
			else
			{
				lcomb.theta = 0;
			}
		}
		else if(normal.x < 0.0)
		{
			lcomb.theta += PI;
			if(lcomb.theta > PI)
				lcomb.theta -= 2*PI;
			else if(lcomb.theta < -PI)
				lcomb.theta += 2*PI;
		}
		lcomb.dist = 0;
		lcomb.start = 1.0e10;
		lcomb.end = -1.0e10;
		Vec2 pts[4] = {pts1, pte1, pts2, pte2};
		float dists[4];
		float coords[4];
		for(int i=0;i<4;i++)
		{
			dists[i] = dot(pts[i], normal);
			lcomb.dist += dot(pts[i], normal);
			float coord = dot(pts[i], v);
			coords[i] = coord;
			if(coord<lcomb.start)
				lcomb.start = coord;
			if(coord>lcomb.end)
				lcomb.end = coord;
		}
		lcomb.dist/=4;

		float dist1 = (dists[0] + dists[1]) / 2;
		float dist2 = (dists[2] + dists[3]) / 2;
		float mincoord1 = min(coords[0], coords[1]);
		float maxcoord1 = max(coords[0], coords[1]);
		float mincoord2 = min(coords[2], coords[3]);
		float maxcoord2 = max(coords[2], coords[3]);

		bool bCapable = false;

		if(fabs(dist1-dist2)<=MAX_DIST_DIFF_OVERALL)
		{
			bCapable = true;
		}
		for(int i=0;i<4;i++)
		{
			if(fabs(dists[i]-lcomb.dist) > MAX_DIST_DIFF_OVERALL)
				bCapable = false;
		}

		if((maxcoord1<mincoord2 && mincoord2 - maxcoord1 > MAX_ENDS_DIFF_OVERALL)
			|| (maxcoord2<mincoord1 && mincoord1 - maxcoord2 > MAX_ENDS_DIFF_OVERALL))
			bCapable = false;

		if(lcomb.end - lcomb.start < MIN_LENGTH_CONN)
			bCapable = false;

		if(bCapable)
		{
			pInst->m_AllLines.push_back(lcomb);
			nUsageCnt[lindex1]++;
			nUsageCnt[lindex2]++;
		}
	}
}

//#define FIND_LICENSE_AREA_ALGOR1
#define MAX_USAGE_FIND_RECT  20
void CLicenseRecog::FindLicenseArea()
{
	m_QuadList.clear();
#ifdef FIND_LICENSE_AREA_ALGOR1
	for(int i=0;i<(int)m_reflines.size();i++)
	{
		Line* line = m_reflines[i];
		int nline = m_nreflines[i];

		for(int j=0;j<nline;j++)
		{
			Line linea = line[j];
			for(int k=0;k<nline;k++)
			{
				if(j == k)
					continue;
				Line lineb = line[k];

				FindRectArea(linea, lineb);
			}
		}
	}
#else
	int* nUsageCnt = new int[m_AllLines.size()];
	for(int i=0;i<(int)m_AllLines.size();i++)
	{
		nUsageCnt[i] = 0;
	}
	IterParam param;
	param.pInst = this;
	param.nUsageCnt = nUsageCnt;
	IterAllLines(_FindLicenseArea, &param);
	delete[] nUsageCnt;
#endif
}

void CLicenseRecog::_FindLicenseArea(int lindex1, int lindex2, void* param)
{
	IterParam* iparam = (IterParam*)param;
	CLicenseRecog* pInst = iparam->pInst;
	int* nUsageCnt = iparam->nUsageCnt;
	if(nUsageCnt[lindex1] >= MAX_USAGE_FIND_RECT
		|| nUsageCnt[lindex2] >= MAX_USAGE_FIND_RECT)
	{
		return;
	}
	Line linea = pInst->m_AllLines[lindex1];
	Line lineb = pInst->m_AllLines[lindex2];
	if(pInst->FindRectArea(linea, lineb))
	{
		nUsageCnt[lindex1]++;
		nUsageCnt[lindex2]++;
	}
}

#define MAX_ANGLE_DIFF_H  (PI/32)
#define MAX_ANGLE_DIFF_V  (PI/8)
#define MAX_LENGTH_DIFF_H 15
#define MAX_LENGTH_DIFF_V 15
#define MIN_LENGTH_H      50
#define MIN_LENGTH_V      30
#define MAX_RATIO         5
#define MIN_RATIO         2
#define HORZ_LINE_RANGE   (PI/8)
#define MAX_COORD_DIFF_H    0.3
#define MAX_COORD_DIFF_V    1//0.5

bool CLicenseRecog::FindRectArea(const Line& linea, const Line& lineb)
{
	float anglediff = fabs(linea.theta-lineb.theta);

	float lengtha = linea.end - linea.start;
	float lengthb = lineb.end - lineb.start;

	float lengthdiff = fabs(lengtha - lengthb);

	float mlength = (lengtha + lengthb)/2;
	float midb = (lineb.start+lineb.end)/2;
	float mida = (linea.start+linea.end)/2;
	Vec2  anorm(cos(linea.theta), sin(linea.theta));
	Vec2  bnorm(cos(lineb.theta), sin(lineb.theta));
	Vec2  tana(-anorm.y, anorm.x);
	Vec2  tanb(-bnorm.y, bnorm.x);
	Vec2  ptmid = bnorm*lineb.dist+tanb*midb;
	float dist = dot(ptmid, anorm);
	bool Order = (dist - linea.dist > 0);
	float distab = fabs(linea.dist - dist);
	float coorda = dot(ptmid, tana);
	float coorddiff = fabs(mida - coorda);

	float ratio = mlength/distab;

	if(fabs(fmod(linea.theta/PI, 1.0)-0.5)<HORZ_LINE_RANGE/PI
		&& fabs(fmod(lineb.theta/PI, 1.0)-0.5)<HORZ_LINE_RANGE/PI
		&& anglediff<MAX_ANGLE_DIFF_H
		&& coorddiff/mlength < MAX_COORD_DIFF_H
		&& mlength>MIN_LENGTH_H
		&& lengthdiff < MAX_LENGTH_DIFF_H
		&& ratio<MAX_RATIO && ratio>MIN_RATIO)
	{
		Vec2 as = linea.dist*anorm+linea.start*tana;
		Vec2 ae = linea.dist*anorm+linea.end*tana;
		Vec2 bs = lineb.dist*bnorm+lineb.start*tanb;
		Vec2 be = lineb.dist*bnorm+lineb.end*tanb;

		if(fmod(linea.theta/PI/2, 1.0)>0.5)
		{
			Order = !Order;
		}

		bool xOrdera = (fmod(linea.theta/PI/2, 1.0)<0.5);
		bool xOrderb = (fmod(lineb.theta/PI/2, 1.0)<0.5);

		if(Order)
			m_QuadList.push_back(Quad(xOrdera?ae:as, xOrdera?as:ae,
			xOrderb?be:bs, xOrderb?bs:be));
		else
			m_QuadList.push_back(Quad(xOrderb?be:bs, xOrderb?bs:be,
			xOrdera?ae:as, xOrdera?as:ae));

		return true;
	}
	else if(fabs(fmod(linea.theta/PI+0.5, 1.0)-0.5)<HORZ_LINE_RANGE/PI
		&& fabs(fmod(lineb.theta/PI+0.5, 1.0)-0.5)<HORZ_LINE_RANGE/PI
		&& anglediff<MAX_ANGLE_DIFF_V
		&& coorddiff/mlength < MAX_COORD_DIFF_V
		&& mlength>MIN_LENGTH_V
		&& lengthdiff < MAX_LENGTH_DIFF_V
		&& ratio<1./MIN_RATIO && ratio>1./MAX_RATIO)
	{
		if(fmod(linea.theta/2/PI+0.25, 1.0)>0.5)
		{
			Order = !Order;
		}
		bool yOrdera = (fmod(linea.theta/2/PI+0.25, 1.0)<0.5);
		bool yOrderb = (fmod(lineb.theta/2/PI+0.25, 1.0)<0.5);

		Vec2 as = linea.dist*anorm+linea.start*tana;
		Vec2 ae = linea.dist*anorm+linea.end*tana;
		Vec2 bs = lineb.dist*bnorm+lineb.start*tanb;
		Vec2 be = lineb.dist*bnorm+lineb.end*tanb;

		if(Order)
			m_QuadList.push_back(Quad(yOrdera?as:ae, yOrderb?bs:be, 
			yOrdera?ae:as, yOrderb?be:bs));
		else
			m_QuadList.push_back(Quad(yOrderb?bs:be, yOrdera?as:ae,
			yOrderb?be:bs, yOrdera?ae:as));

		return true;
	}
	return false;
}

extern int g_deb;
void CLicenseRecog::Process(int nAlg)
{
	Threshold();
	if((nAlg & FIND_COMPS_ALGORITHM1) || !(nAlg & (FIND_COMPS_ALGORITHM1 | FIND_COMPS_ALGORITHM2)))
		FindComps(FIND_COMPS_ALGORITHM1);
	if(nAlg & FIND_COMPS_ALGORITHM2)
		FindComps(FIND_COMPS_ALGORITHM2);
	if((nAlg & FIND_LINES_ALGORITHM1) || !(nAlg & (FIND_LINES_ALGORITHM1 | FIND_LINES_ALGORITHM2)))
		FindLines(FIND_LINES_ALGORITHM1, 0.1, 40);
	if(nAlg & FIND_LINES_ALGORITHM2)
		FindLines(FIND_LINES_ALGORITHM2);
	g_deb = ProcessAllLines();
	FindLicenseArea();
}