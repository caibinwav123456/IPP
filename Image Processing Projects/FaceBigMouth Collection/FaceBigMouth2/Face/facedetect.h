#ifndef _FACEDETECT_H_
#define _FACEDETECT_H_

#include "cv.h"
#include "highgui.h"
int InitFaceDetect();
CvRect Detect( IplImage* image );
void ClearBuf();
#endif