#ifndef _IMAGE_PROCESS_3D_H_
#define _IMAGE_PROCESS_3D_H_

#include <math.h>
#include "Image.h"
#include "matutil.h"

#define WRAP_TYPE_CLAMP   0
#define WRAP_TYPE_BORDER  1
#define WRAP_TYPE_REPEAT  2
#define WRAP_TYPE_MIRROR  3

#define TEX_FILTER_NONE   0
#define TEX_FILTER_LINEAR 1

class Point2D
{
public:
	int x;
	int y;

	Point2D(){x=y=0;}
	Point2D(int _x,int _y){x=_x;y=_y;}
#ifdef USE_OPENCV
	Point2D(CvPoint& point){x=point.x;y=point.y;}
	operator CvPoint(){return cvPoint(x,y);}
#endif
};

class Size2D
{
public:
	int width;
	int height;

	Size2D(){width=height=0;}
	Size2D(int w,int h){width=w;height=h;}
#ifdef USE_OPENCV
	Size2D(CvSize& size){width=size.width;height=size.height;}
	operator CvSize(){return cvSize(width,height);}
#endif
};

class Rect2D
{
public:
	int x;
	int y;
	int width;
	int height;

	Rect2D(){x=y=width=height=0;}
	Rect2D(int l,int t,int w,int h){x=l;y=t;width=w;height=h;}
#ifdef USE_OPENCV
	Rect2D(CvRect& rc){x=rc.x;y=rc.y;width=rc.width;height=rc.height;}
	operator CvRect(){return cvRect(x,y,width,height);}
#endif
};

#define Triangle3D Triangle

typedef void(*PIXELFUNC)(RawImage* target,Point2D pos, int count, float* pdata);
void Triangle2D(RawImage image, Vec2 vert[3], int count = 0, float (*vdata)[3] = NULL, PIXELFUNC pFunc = NULL, Rect2D* pVp = NULL);
void Triangle(RawImage image, Vec3 vert[3], int count = 0, float (*vdata)[3] = NULL, PIXELFUNC pFunc = NULL, Rect2D* pVp = NULL);
CvScalar Sample(RawImage image, Vec2 tex, int wrapType = WRAP_TYPE_CLAMP, int filterMode=TEX_FILTER_LINEAR, bool bUniformCoord = true);


#endif