// Grid.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define WIDTH  800
#define HEIGHT 600
#define SEGX   20
#define SEGY   15
#define LINEW  5
int _tmain(int argc, _TCHAR* argv[])
{
	IplImage* image=cvCreateImage(cvSize(WIDTH,HEIGHT), IPL_DEPTH_8U, 3);
	for(int i=0;i<SEGX;i++)
	{
		int sx=(WIDTH-LINEW)*i/(SEGX-1);
		for(int j=0;j<LINEW;j++)
		{
			for(int k=0;k<HEIGHT;k++)
			{
				*(image->imageData + image->widthStep*k + (sx+j)*3)=255;
			}
		}
	}
	for(int i=0;i<SEGY;i++)
	{
		int sy=(HEIGHT-LINEW)*i/(SEGY-1);
		for(int j=0;j<LINEW;j++)
		{
			for(int k=0;k<WIDTH;k++)
			{
				*(image->imageData + image->widthStep*(sy+j) + k*3 +2)=255;
			}
		}
	}
	cvNamedWindow("output");
	cvShowImage("output", image);
	while(true)
	{
		if(cvWaitKey() == 's')
		{
			cvSaveImage("grid.png",image);
		}
	}
	cvReleaseImage(&image);

	return 0;
}

