#ifndef _IMAGE_PROCESS_2D_H_
#define _IMAGE_PROCESS_2D_H_

#define PI 3.1415926535897932384626433832795028841971

#include <cv.h>
#include <cxcore.h>
#include "ImageProcess3D.h"

//extern bool g_bEnd;

struct Rect2D32f
{
	float x;
	float y;
	float cx;
	float cy;
};

bool EdgeProcess(IplImage* &pSrc, float nFilterScale = 7, int nThresh = 0, float satscale = 0);
bool DotProcess(IplImage* &pSrc);
void DimEffect(IplImage* src, IplImage* dest, Rect2D32f* roi=NULL);
void Mist(IplImage* src, IplImage* dest, int dens = 25);
void Snow(IplImage* src, IplImage* dest, int nSnowBig = 300, int nSnowMid = 150, int nSnowSmall = 600);
void WaterPaint(IplImage* src, IplImage* dest, float size = 1);
void WaxPen(IplImage* src,IplImage* dest, int szStroke = 10, float hardness = 0.5);
void WoodPrint(IplImage* src, IplImage* dest, IplImage* foregnd, IplImage* bkgnd, float treadlen = 0.2, float treadrag = 0.05);
void WaxPen2(IplImage* src,IplImage* dest, int szStroke = 5, float hardness = 0.5, int nGrain = 3);
void OilPaint(IplImage* src, IplImage* dest, int szStroke = 5);
void Rain(IplImage* src, IplImage* dest, float rain_min_length = 15, float rain_max_length = 30, int nDrops = 10000);
void Wave(IplImage* src, IplImage* dest, float wavelen = 0.146, float wavh = 0.02, float refrac = 0.75, Vec3 ldir = Vec3(0.25,0.25,1));
void EndProcess();
void PostEndProcess();

#endif