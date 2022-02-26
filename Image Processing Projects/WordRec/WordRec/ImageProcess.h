#ifndef _IMAGE_PROCESS_H_
#define _IMAGE_PROCESS_H_
#include "matutil.h"
#include "Image.h"

#define WRAP_TYPE_CLAMP   0
#define WRAP_TYPE_BORDER  1
#define WRAP_TYPE_REPEAT  2
#define WRAP_TYPE_MIRROR  3

#define TEX_FILTER_NONE   0
#define TEX_FILTER_LINEAR 1

#define PTR_PIX(IMG,x,y) ((uchar*)((IMG).imageData)+(y)*(IMG).widthStep+(x)*(IMG).nChannels*(((IMG).depth&0x7fffffff)>>3))
Vec3 Sample(RawImage image, Vec2 tex, int wrapType = WRAP_TYPE_CLAMP, int filterMode=TEX_FILTER_LINEAR, bool bUniformCoord = true);

#endif