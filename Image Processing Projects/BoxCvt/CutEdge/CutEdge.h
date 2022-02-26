#ifndef _CUTEDGE_H_
#define _CUTEDGE_H_

#ifndef STATIC_LIB
#ifdef CUTEDGE_EXPORTS
#define CUTEDGE_API __declspec(dllexport)
#else
#define CUTEDGE_API __declspec(dllimport)
#endif
#else
#define CUTEDGE_API
#endif

#include "cxcore.h"
CUTEDGE_API CvRect CutEdge(IplImage* src);
CUTEDGE_API CvRect CutEdge(unsigned char* data, int width, int height, int step);

#endif