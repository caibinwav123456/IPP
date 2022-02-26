#include "stdafx.h"
#include "Propagate.h"
#include "Image.h"

CPropagate::CPropagate()
{
	m_mask = NULL;
	m_buf = NULL;
	m_buf2 = NULL;
	m_bufacc = NULL;
	m_flags = NULL;
	m_flags2 = NULL;
	m_ptacc = 0;
	m_nBuf = 0;
	m_pts = 0;
	m_pFunc = NULL;
	m_pt = cvPoint(-1,-1);
}

CPropagate::~CPropagate()
{
	Clear();
}

void CPropagate::Clear()
{
	cvReleaseImage(&m_mask);
	if(m_buf)
	{
		delete[] m_buf;
		m_buf = NULL;
	}
	if(m_buf2)
	{
		delete[] m_buf2;
		m_buf2 = NULL;
	}
	if(m_bufacc)
	{
		delete[] m_bufacc;
		m_bufacc = NULL;
	}
	if(m_flags)
	{
		delete[] m_flags;
		m_flags = NULL;
	}
	if(m_flags2)
	{
		delete[] m_flags2;
		m_flags2 = NULL;
	}
	m_nBuf = 0;
	m_pts = 0;
	m_ptacc = 0;
}

void CPropagate::Init(int width, int height)
{
	Clear();

	m_mask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	cvZero(m_mask);

	m_nBuf = width*height;
	m_pts = 0;
	m_ptacc = 0;

	m_buf = new CvPoint[width*height];
	ZeroMemory(m_buf, m_nBuf*sizeof(CvPoint));
	m_buf2 = new CvPoint[width*height];
	ZeroMemory(m_buf2, m_nBuf*sizeof(CvPoint));
	m_bufacc = new CvPoint[width*height];
	ZeroMemory(m_bufacc, m_nBuf*sizeof(CvPoint));
	m_flags = new UINT8[width*height];
	ZeroMemory(m_flags, m_nBuf*sizeof(UINT8));
	m_flags2 = new UINT8[width*height];
	ZeroMemory(m_flags2, m_nBuf*sizeof(UINT8));
}

void CPropagate::SetFunc(PropFunc func)
{
	m_pFunc = func;
}

void CPropagate::SetPoint(CvPoint pt)
{
	m_pts = 0;
	m_ptacc = 0;
	m_pt = pt;
}

int CPropagate::Propagate(int mode, bool bAutoMark, void* param, bool bmaintainoutliers)
{
	CvPoint adj[8] = {cvPoint(1,0),cvPoint(0,1),cvPoint(-1,0),cvPoint(0,-1),
					cvPoint(1,1),cvPoint(-1,1),cvPoint(-1,-1),cvPoint(1,-1)};
	UINT8 flags[8] = {PROP_FLAG_RIGHT, PROP_FLAG_BOTTOM, PROP_FLAG_LEFT, PROP_FLAG_TOP,
					PROP_FLAG_RIGHT|PROP_FLAG_BOTTOM, PROP_FLAG_LEFT|PROP_FLAG_BOTTOM, PROP_FLAG_LEFT|PROP_FLAG_TOP, PROP_FLAG_RIGHT|PROP_FLAG_TOP};
	int pts = 0;
	int pts2 = 0;
	int oldptacc = m_ptacc;
	for(int i=0;i<m_pts+1;i++)
	{
		CvPoint pt;
		if(i != m_pts)
			pt = m_buf[i];
		else if(m_pt.x>=0 && m_pt.y>=0)
		{
			pt = m_pt;
			m_pt = cvPoint(-1,-1);
		}
		else
			break;
		int nadj = 0;
		if(mode == PROP_MODE_QUAD)
			nadj = 4;
		else if(mode == PROP_MODE_RECT)
			nadj = 8;
		bool noprop = true;
		for(int j=0;j<nadj;j++)
		{
			CvPoint ptadj;
			if(i==m_pts && pt.x>=0 && pt.y>=0)
			{
				if(j>0)
					break;
				ptadj = pt;
			}
			else
				ptadj = cvPoint(pt.x+adj[j].x, pt.y+adj[j].y);
			if(ptadj.x<0||ptadj.x>=m_mask->width
				|| ptadj.y<0||ptadj.y>=m_mask->height)
				continue;
			if(*(PTR_PIX(*m_mask, ptadj.x, ptadj.y)) == 0)
			{
				if(m_pFunc && m_pFunc(param, ptadj, pt, m_mask, flags[j]))
				{
					m_buf2[pts2] = ptadj;
					m_flags2[pts2] = flags[j];
					m_bufacc[m_ptacc] = ptadj;
					if(bAutoMark)
						*(PTR_PIX(*m_mask, ptadj.x, ptadj.y)) = 255;
					pts2++;
					pts++;
					m_ptacc++;
					noprop = false;
				}
			}
		}
		if(noprop && i!=m_pts && bmaintainoutliers)
		{
			bool bInc = false;
			for(int j=0;j<oldptacc;j++)
			{
				if(m_bufacc[j].x == pt.x && m_bufacc[j].y == pt.y)
				{
					bInc = true;
					break;
				}
			}
			if(!bInc)
			{
				m_buf2[pts2] = pt;
				m_flags2[pts2] = m_flags[i];
				pts2++;
			}
		}
	}
	CvPoint* tmp = m_buf;
	m_buf = m_buf2;
	m_buf2 = tmp;
	UINT8* tmp2 = m_flags;
	m_flags = m_flags2;
	m_flags2 = tmp2;
	m_pts = pts2;
	return pts;
}