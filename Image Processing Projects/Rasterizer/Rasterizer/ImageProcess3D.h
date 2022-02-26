#ifndef _IMAGE_PROCESS_3D_H_
#define _IMAGE_PROCESS_3D_H_

#include <math.h>
#include <cxcore.h>
#include <cv.h>
#include "Image.h"
#define WRAP_TYPE_CLAMP   0
#define WRAP_TYPE_BORDER  1

typedef void(*PIXELFUNC)(RawImage* target,CvPoint pos, int count, float* pdata);
void Triangle(RawImage image, CvPoint2D32f vert[3], int count = 0, float (*vdata)[3] = NULL, PIXELFUNC pFunc = NULL);
void Triangle(RawImage image, CvPoint3D32f vert[3], int count = 0, float (*vdata)[3] = NULL, PIXELFUNC pFunc = NULL);
CvScalar Sample(RawImage image, CvPoint2D32f tex, int wrapType = WRAP_TYPE_CLAMP);

#endif