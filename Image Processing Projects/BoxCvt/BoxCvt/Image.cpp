#include "stdafx.h"
#include "Image.h"

#ifdef USE_OPENCV
deque<IplImage*> RawImage::tempIpl;
#endif