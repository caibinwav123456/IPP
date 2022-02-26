#ifndef _IMAGE_PROCESS_2D_H_
#define _IMAGE_PROCESS_2D_H_

#define PI 3.1415926535897932384626433832795028841971

#include <cv.h>
#include <cxcore.h>

extern int m_nFilterScale;
extern int m_nThresh;

bool EdgeProcess(IplImage* &pSrc);
bool DotProcess(IplImage* &pSrc);
void DimEffect(IplImage* src, IplImage* dest);
void Mist(IplImage* src, IplImage* dest);
void Snow(IplImage* src, IplImage* dest);
void WaterPaint(IplImage* src, IplImage* dest);
void WaxPen(IplImage* src,IplImage* dest);
void WoodPrint(IplImage* src, IplImage* dest, IplImage* foregnd, IplImage* bkgnd);
void WaxPen2(IplImage* src,IplImage* dest);
void OilPaint(IplImage* src, IplImage* dest);
void Rain(IplImage* src, IplImage* dest, float rain_min_length, float rain_max_length);

#endif