// GetMask.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

int hl=0,sl=0,vl=0;
int hh=255,sh=255,vh=255;
CvPoint pt=cvPoint(0,0);
CvPoint ptorg=cvPoint(0,0);
CvScalar low=cvScalar(0,0,0);
CvScalar high=cvScalar(255,255,255);
IplImage* image=NULL;
IplImage* output=NULL;
IplImage* mask=NULL;
void hlow(int pos);
void hhigh(int pos);
void slow(int pos);
void shigh(int pos);
void vlow(int pos);
void vhigh(int pos);
void Process();
void Mouse(int event, int x,int y,int flags, void* param);
int _tmain(int argc, _TCHAR* argv[])
{
	image=cvLoadImage("mask.jpg");
	output=cvCreateImage(cvSize(image->width,image->height),IPL_DEPTH_8U,3);
	mask=cvCreateImage(cvSize(image->width,image->height),IPL_DEPTH_8U,1);
	cvNamedWindow("output");
	cvCreateTrackbar("h low","output", &hl, 256, hlow);
	cvCreateTrackbar("h high","output", &hh, 256, hhigh);
	cvCreateTrackbar("s low","output", &sl, 256, slow);
	cvCreateTrackbar("s high","output", &sh, 256, shigh);
	cvCreateTrackbar("v low","output", &vl, 256, vlow);
	cvCreateTrackbar("v high","output", &vh, 256, vhigh);
	cvSetMouseCallback("output", Mouse);
	Process();
	cvWaitKey();
	cvReleaseImage(&output);
	cvReleaseImage(&mask);
	cvReleaseImage(&image);
	return 0;
}

void Process()
{
	low=cvScalar(hl,sl,vl);
	high=cvScalar(hh,sh,vh);
	IplImage* hsv=cvCreateImage(cvSize(image->width, image->height),IPL_DEPTH_8U, 3);
	cvCvtColor(image,hsv,CV_RGB2HSV);
	cvInRangeS(hsv, low, high, mask);
	cvMerge(mask,mask,mask,NULL,output);
	cvReleaseImage(&hsv);
	cvShowImage("output",output);
}

void Mouse(int event, int x, int y, int flags, void* param)
{
	if(event==CV_EVENT_LBUTTONDOWN)
	{
		cvFloodFill(mask,cvPoint(x,y),cvScalar(128));
		cvCmpS(mask,128,mask,CV_CMP_EQ);
		cvMerge(mask,mask,mask,NULL,output);
		cvShowImage("output",output);
	}
	else if((event == CV_EVENT_MOUSEMOVE) && (flags & CV_EVENT_FLAG_RBUTTON))
	{
		cvMerge(mask, mask, mask, NULL, output);
		cvLine(output,pt,cvPoint(x-ptorg.x,y-ptorg.y),cvScalarAll(0));
		cvShowImage("output",output);
	}
	else if(event == CV_EVENT_RBUTTONDOWN)
	{
		pt=cvPoint(x-ptorg.x,y-ptorg.y);
	}
	else if(event == CV_EVENT_MBUTTONDOWN)
	{
		cvSaveImage("mask.bmp",output);//ptorg=cvPoint(x,y);
	}
}

void hlow(int pos)
{
	hl=pos;
	Process();
}

void hhigh(int pos)
{
	hh=pos;
	Process();
}

void slow(int pos)
{
	sl=pos;
	Process();
}

void shigh(int pos)
{
	sh=pos;
	Process();
}

void vlow(int pos)
{
	vl=pos;
	Process();
}

void vhigh(int pos)
{
	vh=pos;
	Process();
}