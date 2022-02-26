#ifndef _IMAGE_H_
#define _IMAGE_H_

#ifdef USE_OPENCV
#include <cv.h>
#include <cxcore.h>
#include <deque>
#endif
using namespace std;

#define PTR_PIX(IMG,x,y) ((uchar*)((IMG).imageData)+(y)*(IMG).widthStep+(x)*(IMG).nChannels*(((IMG).depth&0x7fffffff)>>3))

/******************************************************************************************************************************
Class Name: RawImage
Author    : CaiBin
Desc      : This class provide a common structure for image processing which is compatible with OpenCV data structures,
            but can be transplanted to more platforms,it can be converted to/from the IplImage structure.The IplImage converted
			from this structure can not be released by cvReleaseImage,you should use cvReleaseImageHeader instead. Similarly,
			a RawImage struct converted from an IplImage cannot call Release method.			

Date      : 2011-10-10
*******************************************************************************************************************************/
class RawImage
{
public:
	int width;
	int height;
	int depth;
	int widthStep;
	int nChannels;
	uchar* imageData;
private:
#ifdef USE_OPENCV
	static deque<IplImage*> tempIpl;
#endif
public:
	RawImage()
	{
		width = height = 0;
		widthStep = 0;
		depth = 8;
		nChannels = 3;
		imageData = NULL;
	}

	~RawImage(){}

	void Release()
	{
		if(imageData != NULL)
		{
			delete[] imageData;
			imageData = NULL;
		}
	}

#ifdef USE_OPENCV
	RawImage(IplImage* image)
	{
		width = image->width;
		height = image->height;
		depth = image->depth;
		nChannels = image->nChannels;
		widthStep = image->widthStep;
		imageData = (uchar*)(image->imageData);
	}

	operator IplImage* ()
	{
		IplImage* imghdr = cvCreateImageHeader(cvSize(width,height), depth, nChannels);
		cvSetData(imghdr, imageData, widthStep);
		tempIpl.push_back(imghdr);
		if(tempIpl.size()>100)
		{
			while(tempIpl.size()>50)
			{
				IplImage* img = tempIpl.front();
				tempIpl.pop_front();
				if(img != NULL)
				{
					cvReleaseImageHeader(&img);
				}
			}
		}
		return imghdr;
	}

	static void ReleaseInternalHdr()
	{
		while(tempIpl.size()!=0)
		{
			IplImage* img = *(tempIpl.end()-1);
			tempIpl.pop_back();
			if(img != NULL)
			{
				cvReleaseImageHeader(&img);
			}
		}
	}
#endif
};

#endif