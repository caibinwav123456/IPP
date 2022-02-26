#include "stdafx.h"
#include "ImageProcess3D.h"

#define L_SAMP(x) ((int)floor(x-0.5))
#define R_SAMP(x) (ceil(x-0.5)==(x-0.5)?(int)ceil(x-0.5)+1:(int)ceil(x-0.5))
void Triangle(RawImage image, CvPoint2D32f vert[3], int count, float (*vdata)[3], PIXELFUNC pFunc)
{
	CvPoint3D32f vert3d[3];
	for(int i=0;i<3;i++)
	{
		vert3d[i] = cvPoint3D32f(vert[i].x, vert[i].y, 1);
	}
	Triangle(image, vert3d, count, vdata, pFunc);
}

void Triangle(RawImage image, CvPoint3D32f vert[3], int count, float (*vdata)[3], PIXELFUNC pFunc)
{
	CvPoint3D32f tmp;
	CvPoint3D32f vbuf[3];
	int i;

	for(i=0;i<3;i++)
	{
		vbuf[i]=vert[i];
	}

	for(i=0;i<3;i++)
	{
		for(int j=i+1;j<3;j++)
		{
			if(vbuf[i].y>vbuf[j].y)
			{
				tmp=vbuf[i];
				vbuf[i]=vbuf[j];
				vbuf[j]=tmp;
			}
		}
	}

	if(vbuf[2].y == vbuf[0].y)
		return;
	float midx = (vbuf[2].x-vbuf[0].x) * (vbuf[1].y-vbuf[0].y) / (vbuf[2].y-vbuf[0].y) + vbuf[0].x;

	int order;

	if(midx > vbuf[1].x)
		order = 0;
	else if(midx < vbuf[1].x)
		order = 1;
	else
		return;

	int y;
	float xbuf[2];

	float* pdata=NULL;
	if(count > 0)
	{
		pdata = new float[count];
	}

	for(y=max(R_SAMP(vbuf[0].y),0); y<=min(L_SAMP(vbuf[2].y),image.height-1); y++)
	{
		if(y<=min(L_SAMP(vbuf[1].y),image.height-1))
		{
			xbuf[order] = (vbuf[1].x-vbuf[0].x)*(y-vbuf[0].y)/(vbuf[1].y-vbuf[0].y)+vbuf[0].x;
			xbuf[1-order] = (vbuf[2].x-vbuf[0].x)*(y-vbuf[0].y)/(vbuf[2].y-vbuf[0].y)+vbuf[0].x;
		}
		else
		{
			xbuf[order] = (vbuf[1].x-vbuf[2].x)*(y-vbuf[2].y)/(vbuf[1].y-vbuf[2].y)+vbuf[2].x;
			xbuf[1-order] = (vbuf[0].x-vbuf[2].x)*(y-vbuf[2].y)/(vbuf[0].y-vbuf[2].y)+vbuf[2].x;
		}

		for(int x=max(R_SAMP(xbuf[0]),0); x<=min(L_SAMP(xbuf[1]),image.width-1); x++)
		{
			if(pFunc == NULL)
			{
				for(int k=0;k<image.nChannels;k++)
				{
					switch(image.depth)
					{
					case IPL_DEPTH_8U:
						*((uchar*)(image.imgdata)+y*image.widthStep+x*image.nChannels+k)+=80;
						break;
					case IPL_DEPTH_8S:
						*((char*)(image.imgdata)+y*image.widthStep+x*image.nChannels+k)+=80;
						break;
					case IPL_DEPTH_16U:
						*((ushort*)((uchar*)(image.imgdata)+y*image.widthStep+x*image.nChannels*2+k))+=80;
						break;
					case IPL_DEPTH_16S:
						*((short*)((uchar*)(image.imgdata)+y*image.widthStep+x*image.nChannels*2+k))+=80;
						break;
					case IPL_DEPTH_32F:
						*((float*)((uchar*)(image.imgdata)+y*image.widthStep+x*image.nChannels*4+k))+=0.3f;
						break;
					case IPL_DEPTH_32S:
						*((long*)((uchar*)(image.imgdata)+y*image.widthStep+x*image.nChannels*4+k))+=80;
						break;
					case IPL_DEPTH_64F:
						*((double*)((uchar*)(image.imgdata)+y*image.widthStep+x*image.nChannels*8+k))+=0.3;
						break;
					}
				}
			}
			else
			{
				int i;
				float xparam[3], yparam[3], zparam[3];
				for(i=0;i<3;i++)
				{
					xparam[i]=vert[i].x/vert[i].z;
					yparam[i]=vert[i].y/vert[i].z;
					zparam[i]=1./vert[i].z;
				}
				CvPoint2D32f point = cvPoint2D32f(x+0.5, y+0.5);
				CvPoint2D32f vec   = cvPoint2D32f(point.x*zparam[0]-xparam[0], point.y*zparam[0]-yparam[0]);
				CvPoint2D32f vbase1= cvPoint2D32f(xparam[1]-xparam[0],yparam[1]-yparam[0]-point.x*(zparam[1]-zparam[0]));
				CvPoint2D32f vbase2= cvPoint2D32f(xparam[2]-xparam[0],yparam[2]-yparam[0]-point.y*(zparam[2]-zparam[0]));
				float det = vbase1.x*vbase2.y-vbase1.y*vbase2.x;
				float lerp1 = (vec.x*vbase2.y-vec.y*vbase2.x)/det;
				float lerp2 = (vbase1.x*vec.y-vbase1.y*vec.x)/det;
				for(i=0;i<count;i++)
				{
					pdata[i]=vdata[i][0]+lerp1*(vdata[i][1]-vdata[i][0])+lerp2*(vdata[i][2]-vdata[i][0]);
				}
				pFunc(&image, cvPoint(x,y), count, pdata);
			}
		}
	}
	if(pdata != NULL)
	{
		delete[] pdata;
		pdata = NULL;
	}
}

CvScalar operator+(CvScalar a, CvScalar b)
{
	return cvScalar(a.val[0]+b.val[0], a.val[1]+b.val[1], a.val[2]+b.val[2], a.val[3]+b.val[3]);
}

CvScalar operator-(CvScalar a, CvScalar b)
{
	return cvScalar(a.val[0]-b.val[0], a.val[1]-b.val[1], a.val[2]-b.val[2], a.val[3]-b.val[3]);
}

CvScalar operator*(CvScalar a, float b)
{
	return cvScalar(a.val[0]*b, a.val[1]*b, a.val[2]*b, a.val[3]*b);
}

CvScalar operator*(float b, CvScalar a)
{
	return cvScalar(a.val[0]*b, a.val[1]*b, a.val[2]*b, a.val[3]*b);
}

CvScalar Sample(RawImage image, CvPoint2D32f tex, int wrapType)
{
	int   tx=(int)floor(tex.x-0.5);
	int   ty=(int)floor(tex.y-0.5);
	float dx=tex.x-0.5-tx;
	float dy=tex.y-0.5-ty;

	CvScalar color[2][2];
	CvScalar dstc=cvScalarAll(0);


	for(int k=0;k<2;k++)
	{
		for(int l=0;l<2;l++)
		{
			int ntx = tx+k;
			int nty = ty+l;
			if(ntx<0||ntx>=image.width||nty<0||nty>=image.height)
			{
				if(wrapType == WRAP_TYPE_BORDER)
				{
					color[k][l]=cvScalarAll(128);
					continue;
				}
				else if(wrapType == WRAP_TYPE_CLAMP)
				{
					if(ntx<0)ntx=0;
					else if(ntx>=image.width)ntx=image.width-1;
					if(nty<0)nty=0;
					else if(nty>=image.height)nty=image.height-1;
				}
			}
			for(int p=0;p<image.nChannels;p++)
			{
				switch(image.depth)
				{
				case IPL_DEPTH_8U:
					color[k][l].val[p] = *((uchar*)(image.imgdata)+nty*image.widthStep+ntx*image.nChannels+p);
					break;
				case IPL_DEPTH_8S:
					color[k][l].val[p] = *((char*)((uchar*)(image.imgdata)+nty*image.widthStep+ntx*image.nChannels+p));
					break;
				case IPL_DEPTH_16U:
					color[k][l].val[p] = *((ushort*)((uchar*)(image.imgdata)+nty*image.widthStep+ntx*image.nChannels*2+p));
					break;
				case IPL_DEPTH_16S:
					color[k][l].val[p] = *((short*)((uchar*)(image.imgdata)+nty*image.widthStep+ntx*image.nChannels*2+p));
					break;
				case IPL_DEPTH_32F:
					color[k][l].val[p] = *((float*)((uchar*)(image.imgdata)+nty*image.widthStep+ntx*image.nChannels*4+p));
					break;
				case IPL_DEPTH_32S:
					color[k][l].val[p] = *((long*)((uchar*)(image.imgdata)+nty*image.widthStep+ntx*image.nChannels*4+p));
					break;
				case IPL_DEPTH_64F:
					color[k][l].val[p] = *((double*)((uchar*)(image.imgdata)+nty*image.widthStep+ntx*image.nChannels*8+p));
					break;
				}
			}
		}
	}	
//	color[k][l].val[0]=cvGet2D(image, ty+l, tx+k);

	if(dx+dy<=1.0)
	{
		dstc=color[0][0]+dx*(color[1][0]-color[0][0])+dy*(color[0][1]-color[0][0]);
	}
	else
	{
		dstc=color[1][1]+(1-dx)*(color[0][1]-color[1][1])+(1-dy)*(color[1][0]-color[1][1]);
	}

	return dstc;
}