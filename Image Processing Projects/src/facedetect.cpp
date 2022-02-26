// facedetect.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "facedetect.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>


static CvMemStorage* storage = 0;
static CvHaarClassifierCascade* cascade = 0;
static CvSeq* faces = NULL;
static int nface = 0;
static int iface = 0;

const char* cascade_name =
    "haarcascade_frontalface_alt2.xml";
/*    "haarcascade_profileface.xml";*/
IplImage* imgbuf=NULL;
int InitFaceDetect(char* name)
{
	cascade = (CvHaarClassifierCascade*)cvLoad( name==NULL?cascade_name:name, 0, 0, 0 );
    
    if( !cascade )
    {
        fprintf( stderr, "ERROR: Could not load classifier cascade\n" );
        fprintf( stderr,
        "Usage: facedetect --cascade=\"<cascade_path>\" [filename|camera_index]\n" );
        return -1;
    }
    storage = cvCreateMemStorage(0);
    
    return 0;
}

void ClearBuf()
{
	iface = nface = 0;
	faces = NULL;
}

CvRect Detect( IplImage* img )
{
    static CvScalar colors[] = 
    {
        {{0,0,255}},
        {{0,128,255}},
        {{0,255,255}},
        {{0,255,0}},
        {{255,128,0}},
        {{255,255,0}},
        {{255,0,0}},
        {{255,0,255}}
    };

	CvRect rcResult=cvRect(0,0,0,0);
    double scale = 1.3;
    IplImage* gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
    IplImage* small_img = cvCreateImage( cvSize( cvRound (img->width/scale),
                         cvRound (img->height/scale)),
                     8, 1 );
    //int i;

    cvCvtColor( img, gray, CV_BGR2GRAY );
    cvResize( gray, small_img, CV_INTER_LINEAR );
    cvEqualizeHist( small_img, small_img );
    cvClearMemStorage( storage );

    if( cascade )
    {
        double t = (double)cvGetTickCount();
		if(img!=imgbuf)
		{
			iface = nface = 0;
			faces = NULL;
			imgbuf = img;
		}
		if(!faces)
		{
			faces = cvHaarDetectObjects( small_img, cascade, storage,
                                            1.1, 2, 0/*CV_HAAR_DO_CANNY_PRUNING*/,
                                            cvSize(30, 30) );
			if(faces)nface=faces->total;
			if(nface>0)
			{
				iface=0;
			}
			else
			{
				iface=nface=0;
				faces=0;
			}
		}
        t = (double)cvGetTickCount() - t;
        printf( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );
		if(faces)
		{
			CvRect* r = (CvRect*)cvGetSeqElem( faces, iface );
			rcResult = cvRect(r->x*scale,r->y*scale,r->width*scale,r->height*scale);
		}
		else
		{
			rcResult = cvRect(0,0,0,0);
		}
		iface++;
		if(iface>=nface)
		{
			iface=nface=0;
			faces = NULL;
		}
		//CvPoint center;
        //int radius;
        //center.x = cvRound((r->x + r->width*0.5)*scale);
        //center.y = cvRound((r->y + r->height*0.5)*scale);
        //radius = cvRound((r->width + r->height)*0.25*scale);
        //cvRectangle(img, cvPoint(r->x*scale,r->y*scale), cvPoint((r->x+r->width)*scale,(r->y+r->height)*scale), cvScalar(0,255,255), 2);//cvCircle( img, center, radius, colors[i%8], 3, 8, 0 );
    }

    cvReleaseImage( &gray );
    cvReleaseImage( &small_img );
	return rcResult;
}
