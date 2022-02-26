#ifndef _IMAGE_PROCESS_2D_H_
#define _IMAGE_PROCESS_2D_H_

#include <cv.h>
#include <cxcore.h>

extern int m_nFilterScale;
extern int m_nThresh;
bool EdgeProcess(IplImage* &pSrc);
bool DotProcess(IplImage* &pSrc);

#endif