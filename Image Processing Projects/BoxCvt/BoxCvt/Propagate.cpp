#include "stdafx.h"
#include "Propagate.h"
#include "Image.h"

template<class T>
Comp_Container<T>::Comp_Container(int initl, int grow)
{
	m_initl=max(1,initl);
	m_ngrow=max(GROW_LINEAR,min(GROW_EXPONENTIAL,grow));
	m_nMax=0;
	first=(CompNode<T>*)malloc(sizeof(CompNode<T>)+(m_initl-1)*sizeof(T));
	first->ndata=m_initl;
	first->next=NULL;
}

template<class T>
Comp_Container<T>::~Comp_Container()
{
	CompNode<T>* node=first;
	while(node!=NULL)
	{
		CompNode<T>* tmp=node;
		node=node->next;
		free(tmp);
	}
}

template<class T>
T& Comp_Container<T>::operator[](int n)
{
	int i=0;
	CompNode<T>* node=first;
	while(node->next!=NULL&&i+node->ndata<=n)
	{
		i+=node->ndata;
		node=node->next;
	}
	if(i+node->ndata>n)
	{
		return node->data[n-i];
	}
	else
	{
		//Grow array
		int size;
		if(m_ngrow==GROW_LINEAR)
		{
			size = node->ndata;
		}
		else if(m_ngrow==GROW_EXPONENTIAL)
		{
			size = 2*node->ndata;
		}
		if(m_nMax&&i+node->ndata+size>m_nMax)
			size=m_nMax-i-node->ndata;
		size=max(size,n+1-i-node->ndata);
		node->next = (CompNode<T>*)malloc(sizeof(CompNode<T>)+(size-1)*sizeof(T));
		i+=node->ndata;
		node=node->next;
		node->ndata=size;
		node->next=NULL;
		return node->data[n-i];
	}
}

template<class T>
void Comp_Container<T>::Reset()
{
	CompNode<T>* node = first->next;
	while(node!=NULL)
	{
		CompNode<T>* tmp=node;
		node=node->next;
		free(tmp);
	}
	first->next=NULL;
}

template<class T>
void Comp_Container<T>::Swap(Comp_Container<T>& inst2)
{
	swap(first, inst2.first);
	swap(m_initl, inst2.m_initl);
	swap(m_ngrow, inst2.m_ngrow);
	swap(m_nMax, inst2.m_nMax);
}

CPropagate::CPropagate():m_buf(100,GROW_EXPONENTIAL),m_buf2(100,GROW_EXPONENTIAL),m_bufacc(100,GROW_EXPONENTIAL)
{
	m_mask = NULL;
	m_ptacc = 0;
	m_nBuf = 0;
	m_pts = 0;
	m_pFunc = NULL;
	m_pt = cvPoint(-1,-1);
	m_bMask=false;
	m_width=0;
	m_height=0;
}

CPropagate::~CPropagate()
{
	Clear();
}

void CPropagate::Clear()
{
	cvReleaseImage(&m_mask);
	m_nBuf = 0;
	m_pts = 0;
	m_ptacc = 0;
}

void CPropagate::Init(int width, int height, bool usemask)
{
	Clear();

	m_bMask=usemask;
	m_width=width;
	m_height=height;
	if(m_bMask)
	{
		m_mask = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
		cvZero(m_mask);
	}

	m_nBuf = width*height;
	m_pts = 0;
	m_ptacc = 0;
	m_buf.SetMaxSize(m_nBuf);
	m_buf2.SetMaxSize(m_nBuf);
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
	m_buf.Reset();
	m_buf2.Reset();
	m_bufacc.Reset();
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
			if(ptadj.x<0||ptadj.x>=m_width
				|| ptadj.y<0||ptadj.y>=m_height)
				continue;
			if(!m_mask||*(PTR_PIX(*m_mask, ptadj.x, ptadj.y)) == 0)
			{
				if(m_pFunc && m_pFunc(param, ptadj, pt, m_mask, flags[j]))
				{
					m_buf2[pts2] = ptadj;
					m_bufacc[m_ptacc] = ptadj;
					if(bAutoMark&&m_mask)
						*(PTR_PIX(*m_mask, ptadj.x, ptadj.y)) = 255;
					pts2++;
					pts++;
					m_ptacc++;
				}
			}
		}
	}
	m_buf.Swap(m_buf2);
	m_pts = pts2;
	return pts;
}