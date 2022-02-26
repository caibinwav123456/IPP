// CutEdge.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "CutEdge.h"
#include "cv.h"
#include "Image.h"

#define DEV_THRESH 3.5f
#define DEV_THRESH2 3.0f
#define LOW_THRESH (3.0f/3.5f)
#define DIST_THRESH 0.8f
#define SEARCH_RANGE 0.66f

CvRect ComputeDeviation(float* xIntense, float* yIntense, int nx, int ny);
bool ComputeDev(float* intense, int n, int dir, int* cut);

CUTEDGE_API CvRect CutEdge(unsigned char* data, int width, int height, int step)
{
	IplImage* header=cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, 4);
	cvSetData(header, data, step);
	IplImage* image=cvCreateImage(cvGetSize(header), IPL_DEPTH_8U, 3);
	for(int i=0;i<image->height;i++)
	for(int j=0;j<image->width;j++)
	{
		uchar* pix=PTR_PIX(*image, j, i);
		uchar* pixs=PTR_PIX(*header, j, i);
		memcpy(pix, pixs, 3);
	}
	CvRect rc=CutEdge(image);
	cvReleaseImage(&image);
	cvReleaseImageHeader(&header);
	return rc;
}

CUTEDGE_API CvRect CutEdge(IplImage* src)
{
	float* xIntense = new float[src->width];
	float* yIntense = new float[src->height];
	int nx = src->width;
	int ny = src->height;
	memset(xIntense,0,nx*sizeof(float));
	memset(yIntense,0,ny*sizeof(float));
	for(int i=0;i<nx;i++)
	for(int j=0;j<ny;j++)
	{
		xIntense[i]+=*PTR_PIX(*src,i,j);
		yIntense[j]+=*PTR_PIX(*src,i,j);
	}
	for(int i=0;i<nx;i++)
		xIntense[i]/=ny;
	for(int i=0;i<ny;i++)
		yIntense[i]/=nx;
	CvRect rc = ComputeDeviation(xIntense, yIntense, nx, ny);
	delete[] xIntense;
	delete[] yIntense;
	return rc;
}

CvRect ComputeDeviation(float* xIntense, float* yIntense, int nx, int ny)
{
	if(xIntense == NULL || yIntense == NULL)
		return cvRect(0,0,nx,ny);
	int topcut=0;
	int botcut=ny;
	int leftcut=0;
	int rightcut=nx;
	int cut;
	if(ComputeDev(xIntense, (nx+1)/2, 1, &cut))
		leftcut = cut;
	if(ComputeDev(xIntense+nx-1, (nx+1)/2, -1, &cut))
		rightcut = nx-1+cut;
	if(ComputeDev(yIntense, (ny+1)/2, 1, &cut))
		topcut = cut;
	if(ComputeDev(yIntense+ny-1, (ny+1)/2, -1, &cut))
		botcut = ny-1+cut;
	return cvRect(leftcut,topcut,rightcut-leftcut,botcut-topcut);
}

bool ComputeDev(float* intense, int n, int dir, int* cut)
{
	float X=0,Y=0,YY=0,XY=0,XX=0,N=0;
	float deviation=0;
	float predict=0;
	bool bpredict=false;
	int lowpos=0;
	int lowpos2=0;
	bool blow=false;
	bool blow2=0;
	bool bcut2=false;
	int cutpos2=-1;
	for(int i=0;dir>0?i<n:i>-n;i+=dir)
	{
		float x=(float)i;
		float y=intense[i];
		X+=x;
		Y+=y;
		YY+=y*y;
		XY+=x*y;
		XX+=x*x;
		N+=1;
		if(bpredict)
		{
			float dev=(y-predict)/max(1,deviation);
			if(!blow && fabs(dev)>DEV_THRESH*LOW_THRESH)
			{
				blow=true;
				lowpos=i;
			}
			if(!blow2 && fabs(dev)>DEV_THRESH2*LOW_THRESH)
			{
				blow2=true;
				lowpos2=i;
			}
			if(abs(i)>=n*SEARCH_RANGE)
				break;
			if(fabs(dev)>DEV_THRESH)
			{
				if(blow&&abs(lowpos)>=DIST_THRESH*abs(i))
				{
					*cut = i;
					return true;
				}
			}
			if(!bcut2&&fabs(dev)>DEV_THRESH2)
			{
				if(blow2&&abs(lowpos2)>=DIST_THRESH*abs(i))
				{
					cutpos2 = i;
					bcut2=true;
				}
			}
		}
		if(N>1)
		{
			float denum = XX*N-X*X;
			float k=(XY*N-Y*X)/denum;
			float b=(XX*Y-XY*X)/denum;
			deviation = (YY+k*k*XX+N*b*b-2*k*XY-2*b*Y+2*k*b*X)/N;
			predict=(i+dir)*k+b;
			bpredict=true;
		}
	}
	if(cutpos2>0)
	{
		*cut=cutpos2;
		return true;
	}
	return false;
}
