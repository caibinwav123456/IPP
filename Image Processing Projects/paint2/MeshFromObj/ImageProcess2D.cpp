#include "stdafx.h"
#include "Process.h"

int m_nFilterScale=3;
int m_nThresh=10;

bool EdgeProcess(IplImage* &pSrc)
{
	CvMat* kernelx = cvCreateMat(2 * m_nFilterScale + 1, 2 * m_nFilterScale + 1, CV_32FC1);
	CvMat* kernely = cvCreateMat(2 * m_nFilterScale + 1, 2 * m_nFilterScale + 1, CV_32FC1);
	CvMat* kernelm = cvCreateMat(3, 3, CV_32FC1);


	uchar c[3][3] =
	{
		{20, 60, 20},
		{60, 255, 60},
		{20, 60, 20}
	};

	int i;
	for(i=0; i<3; i++)
		for(int j=0; j<3; j++)
		{
			cvSet2D(kernelm, i, j, cvScalar( ((float)c[i][j]) / 255) );
		}


	float sum = 0;

	for(i = -m_nFilterScale; i <= m_nFilterScale; i++)
		for(int j = -m_nFilterScale; j <= m_nFilterScale; j++)
		{
			float sqrk = m_nFilterScale*m_nFilterScale;
			float coeff = exp( -(float)(i*i + j*j) / sqrk );
			sum += coeff;
			cvSet2D(kernelx, i + m_nFilterScale, j + m_nFilterScale, cvScalar(-2*j * coeff / sqrk));
			cvSet2D(kernely, i + m_nFilterScale, j + m_nFilterScale, cvScalar(-2*i * coeff / sqrk));
		}

	cvScale(kernelx, kernelx, 1/sum);
	cvScale(kernely, kernely, 1/sum);

	IplImage* gray = cvCreateImage(cvSize(pSrc->width, pSrc->height), IPL_DEPTH_32F, 1);
	IplImage* tmp = cvCreateImage(cvSize(pSrc->width, pSrc->height), IPL_DEPTH_8U, 1);
	IplImage* tmpf = cvCreateImage(cvSize(pSrc->width, pSrc->height), IPL_DEPTH_32F, 1);

	cvSplit(pSrc, tmp, NULL, NULL, NULL);
	cvConvertScale(tmp, gray);
	cvSplit(pSrc, NULL, tmp, NULL, NULL);
	cvConvertScale(tmp, tmpf);
	cvAdd(gray, tmpf, gray);
	cvSplit(pSrc, NULL, NULL, tmp, NULL);
	cvConvertScale(tmp, tmpf);
	cvAdd(gray, tmpf, gray);
	cvReleaseImage( &tmp );
	cvReleaseImage( &tmpf );

	IplImage* dx = cvCreateImage(cvSize(pSrc->width, pSrc->height), IPL_DEPTH_32F, 1);
	IplImage* dy = cvCreateImage(cvSize(pSrc->width, pSrc->height), IPL_DEPTH_32F, 1);
	IplImage* result = cvCreateImage(cvSize(pSrc->width, pSrc->height), IPL_DEPTH_8U, 1);

	cvFilter2D(gray, dx, kernelx, cvPoint(m_nFilterScale, m_nFilterScale));
	cvFilter2D(gray, dy, kernely, cvPoint(m_nFilterScale, m_nFilterScale));

	//index allocation
	//7 0 1
	//6 M 2
	//5 4 3

	float param[4] = {-sqrt(2.)-1, -sqrt(2.)+1, sqrt(2.)-1, sqrt(2.)+1};
	int dir[8][2] = {{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}};
	for(i=0;i<pSrc->width;i++)
		for(int j=0;j<pSrc->height;j++)
		{
			float x=cvGet2D(dx,j,i).val[0];
			float y=cvGet2D(dy,j,i).val[0];
			int index;
			if(x!=0)
			{
				index = 4;
				for(int k=0;k<4;k++)
				{
					if(y/x<param[k])
					{
						index=k;
						break;
					}
				}
				if(x<0)
					index=(index+4)%8;
			}
			else
			{
				if(y>0)
					index=4;
				else if(y<0)
					index=0;
				else
					continue;
			}
			int ni=i+dir[index][0];
			int nj=j+dir[index][1];
			int pi=i-dir[index][0];
			int pj=j-dir[index][1];
			ni<0?ni=0:(ni>=pSrc->width?ni=pSrc->width-1:ni=ni);
			nj<0?nj=0:(nj>=pSrc->height?nj=pSrc->height-1:nj=nj);
			pi<0?pi=0:(pi>=pSrc->width?pi=pSrc->width-1:pi=pi);
			pj<0?pj=0:(pj>=pSrc->height?pj=pSrc->height-1:pj=pj);
			float ddir=x*dir[index][0]+y*dir[index][1];
			float pddir=cvGet2D(dx,pj,pi).val[0]*dir[index][0]+cvGet2D(dy,pj,pi).val[0]*dir[index][1];
			float nddir=cvGet2D(dx,nj,ni).val[0]*dir[index][0]+cvGet2D(dy,nj,ni).val[0]*dir[index][1];
			if(ddir>pddir && ddir>nddir && sqrt(x*x + y*y) >= m_nThresh)
				cvSet2D(result,j,i,cvScalar(0));
			else
				cvSet2D(result,j,i,cvScalar(255));

		}

	cvNot(result, result);
	cvFilter2D(result, result, kernelm, cvPoint(1,1));
	cvNot(result, result);

	IplImage* mask = cvCreateImage(cvSize(pSrc->width, pSrc->height), IPL_DEPTH_8U, 3);
	cvMerge(result, result, result, NULL, mask);
	cvMul(pSrc, mask, pSrc, 1./255);

	cvReleaseImage(&result);
	cvReleaseImage(&mask);
	cvReleaseImage(&gray);
	cvReleaseImage(&dx);
	cvReleaseImage(&dy);

	cvReleaseMat(&kernelx);
	cvReleaseMat(&kernely);
	cvReleaseMat(&kernelm);

	return true;
}

bool DotProcess(IplImage* &pSrc)
{
	IplImage* gray = cvCreateImage(cvSize(pSrc->width,pSrc->height),IPL_DEPTH_8U,1);
	IplImage* tmp = cvCreateImage(cvSize(pSrc->width,pSrc->height),IPL_DEPTH_32F,1);
	cvCvtColor(pSrc,pSrc,CV_BGR2HSV);
	cvSplit(pSrc,NULL,NULL,gray,NULL);
	cvConvertScale(gray,tmp);
	for(int i=0; i<pSrc->height; i++)
	{
		for(int j=0;j<pSrc->width;j++)
		{
			float bright = cvGet2D(tmp, i, j).val[0];
			float diff;

			if(bright >= 128)
			{
				cvSet2D(pSrc, i, j, cvScalarAll(255));
				diff = bright-255;
			}
			else
			{
				cvSet2D(pSrc, i, j, cvScalarAll(0));
				diff = bright;
			}
			*(float*)cvPtr2D(tmp,i,j+1>=pSrc->width?pSrc->width-1:j+1)+=diff*7/16;
			*(float*)cvPtr2D(tmp,i+1>=pSrc->height?pSrc->height-1:i+1,j-1<0?0:j-1)+=diff*3/16;
			*(float*)cvPtr2D(tmp,i+1>=pSrc->height?pSrc->height-1:i+1,j)+=diff*5/16;
			*(float*)cvPtr2D(tmp,i+1>=pSrc->height?pSrc->height-1:i+1,j+1>=pSrc->width?pSrc->width-1:j+1)+=diff*1/16;
		}
	}
	cvReleaseImage(&gray);
	cvReleaseImage(&tmp);

	return true;
}

