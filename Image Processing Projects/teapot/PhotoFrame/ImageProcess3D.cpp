#include "stdafx.h"
#include "ImageProcess3D.h"

#define L_SAMP(x) ((int)floor(x-0.5))
#define R_SAMP(x) (L_SAMP(x)+1)//(ceil(x-0.5)==(x-0.5)?(int)ceil(x-0.5)+1:(int)ceil(x-0.5))

void Triangle2D(RawImage image, Vec2 vert[3], int count, float (*vdata)[3], PIXELFUNC pFunc, Rect2D* pVp)
{
	Vec3 vert3d[3];
	for(int i=0;i<3;i++)
	{
		vert3d[i] = Vec3(vert[i].x, vert[i].y, 1);
	}
	Rect2D Vp3d;
	if(pVp == NULL)
	{
		Vp3d=Rect2D(-1,1,2,-2);
	}
	else
	{
		Vp3d=*pVp;
	}
	Triangle(image, vert3d, count, vdata, pFunc, &Vp3d);
}

void Triangle(RawImage image, Vec3 vert[3], int count, float (*vdata)[3], PIXELFUNC pFunc, Rect2D* pVp)
{
	Vec3 vbuf[3];
	int i;
	float vpx=(float)image.width/2;
	float vpy=(float)image.height/2;
	float scale=(float)image.height/2;
	if(pVp != NULL)
	{
		vpx = (float)pVp->width/2+pVp->x;
		vpy = (float)pVp->height/2+pVp->y;
		scale = (float)pVp->height/2;
	}

	for(i=0;i<3;i++)
	{
		vbuf[i]=vert[i];
	}

	float py1=-vbuf[0].y*scale+vpy;
	float py2=-vbuf[1].y*scale+vpy;
	float py3=-vbuf[2].y*scale+vpy;

	float* py[3]={&py1,&py2,&py3};
	for(i=0;i<3;i++)
	{
		for(int j=i+1;j<3;j++)
		{
			if(*py[i]>*py[j])
			{
				Vec3 tmp=vbuf[i];
				vbuf[i]=vbuf[j];
				vbuf[j]=tmp;
				float tmpf=*py[i];
				*py[i]=*py[j];
				*py[j]=tmpf;
			}
		}
	}

	if(vbuf[0].z<0 && vbuf[1].z<0 && vbuf[2].z<0)
		return;
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
	int xbuf[2];

	float* pdata=NULL;
	if(count > 0)
	{
		pdata = new float[count];
	}

	int ymin,ymax,ymid,ymidex,xbd;
	int status;
	int StatusMatrix[7][2][2]={{{1,0},{1,0}},{{1,2},{1,2}},{{0,3},{2,3}},
							   {{2,1},{2,0}},{{0,2},{0,1}},{{0,1},{2,1}},{{3,2},{3,0}}};
	if(vbuf[1].z<0)
	{
		if(vbuf[0].z>=0 && vbuf[2].z<0)
		{
			ymin=0;
			ymax=R_SAMP(py1);
			ymid=R_SAMP(py2);
			status=0;
		}
		else if(vbuf[0].z<0 && vbuf[2].z>=0)
		{
			ymin=R_SAMP(py3);
			ymax=image.height;
			ymid=R_SAMP(py2);
			status=1;
		}
		else if(vbuf[0].z>=0 && vbuf[2].z>=0)
		{
			ymin=0;
			ymax=image.height;
			ymid=R_SAMP(py1);
			ymidex=R_SAMP(py3);
			order==1?xbd=0:xbd=image.width;
			status=2;
		}
	}
	else
	{
		if(vbuf[0].z>=0 && vbuf[2].z<0)
		{
			ymin=0;
			ymax=R_SAMP(py2);
			ymid=R_SAMP(py1);
			status=3;
		}
		else if(vbuf[0].z<0 && vbuf[2].z>=0)
		{
			ymin=R_SAMP(py2);
			ymax=image.height;
			ymid=R_SAMP(py3);
			status=4;
		}
		else if(vbuf[0].z>=0 && vbuf[2].z>=0)
		{
			ymin=R_SAMP(py1);
			ymax=R_SAMP(py3);
			ymid=R_SAMP(py2);
			status=5;
		}
		else if(vbuf[0].z<0 && vbuf[2].z<0)
		{
			ymin=0;
			ymax=image.height;
			ymid=R_SAMP(py2);
			order==1?xbd=image.width:xbd=0;
			status=6;
		}
	}
	for(y=max(ymin,0); y<min(ymax,image.height); y++)
	{
		float e12 = (vbuf[1].x-vbuf[0].x)*(-(y+0.5-vpy)/scale-vbuf[0].y)/(vbuf[1].y-vbuf[0].y)+vbuf[0].x;
		float e13 = (vbuf[2].x-vbuf[0].x)*(-(y+0.5-vpy)/scale-vbuf[0].y)/(vbuf[2].y-vbuf[0].y)+vbuf[0].x;
		float e23 = (vbuf[1].x-vbuf[2].x)*(-(y+0.5-vpy)/scale-vbuf[2].y)/(vbuf[1].y-vbuf[2].y)+vbuf[2].x;
		float edge[4]={e12,e13,e23,(xbd-vpx)/fabs(scale)};
		if(y<min(ymid,image.height))
		{
			xbuf[0] = R_SAMP(edge[StatusMatrix[status][0][  order]]*fabs(scale)+vpx);
			xbuf[1] = R_SAMP(edge[StatusMatrix[status][0][1-order]]*fabs(scale)+vpx);
		}
		else if(status==2 && y<min(ymidex,image.height))
		{
			xbuf[order] = R_SAMP(e13*fabs(scale)+vpx);
			xbuf[1-order] = xbd;
		}
		else
		{
			xbuf[0] = R_SAMP(edge[StatusMatrix[status][1][order]]*fabs(scale)+vpx);
			xbuf[1] = R_SAMP(edge[StatusMatrix[status][1][1-order]]*fabs(scale)+vpx);
		}

		for(int x=max(xbuf[0],0); x<min(xbuf[1],image.width); x++)
		{
			if(pFunc == NULL)
			{
				for(int k=0;k<image.nChannels;k++)
				{
					uchar* ptr=PTR_PIX(image,x,y)+k;
					switch(image.depth)
					{
					case IPL_DEPTH_8U:
						*ptr+=80;
						break;
					case IPL_DEPTH_8S:
						*((char*)ptr)+=80;
						break;
					case IPL_DEPTH_16U:
						*((ushort*)ptr)+=80;
						break;
					case IPL_DEPTH_16S:
						*((short*)ptr)+=80;
						break;
					case IPL_DEPTH_32F:
						*((float*)ptr)+=0.3f;
						break;
					case IPL_DEPTH_32S:
						*((long*)ptr)+=80;
						break;
					case IPL_DEPTH_64F:
						*((double*)ptr)+=0.3;
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
				Vec2 point((x+0.5-vpx)/fabs(scale), -(y+0.5-vpy)/fabs(scale));

				Vec2 vec(point.x*zparam[0]-xparam[0], point.y*zparam[0]-yparam[0]);
				Vec2 vbase1(xparam[1]-xparam[0]-point.x*(zparam[1]-zparam[0]),yparam[1]-yparam[0]-point.y*(zparam[1]-zparam[0]));
				Vec2 vbase2(xparam[2]-xparam[0]-point.x*(zparam[2]-zparam[0]),yparam[2]-yparam[0]-point.y*(zparam[2]-zparam[0]));

				float det = vbase1.x*vbase2.y-vbase1.y*vbase2.x;
				float lerp1 = (vec.x*vbase2.y-vec.y*vbase2.x)/det;
				float lerp2 = (vbase1.x*vec.y-vbase1.y*vec.x)/det;

				for(i=0;i<count;i++)
				{
					pdata[i]=vdata[i][0]+lerp1*(vdata[i][1]-vdata[i][0])+lerp2*(vdata[i][2]-vdata[i][0]);
				}
				pFunc(&image, Point2D(x,y), count, pdata);
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

CvScalar Sample(RawImage image, Vec2 tex, int wrapType, int filterMode, bool bUniformCoord)
{
	if(bUniformCoord)
	{
		tex.x*=image.width;
		tex.y*=image.height;
	}

	int   tx=(int)floor(tex.x-0.5);
	int   ty=(int)floor(tex.y-0.5);
	float dx=tex.x-0.5-tx;
	float dy=tex.y-0.5-ty;

	CvScalar color[2][2];
	CvScalar dstc=cvScalarAll(0);

	int szKernel=1;
	if(filterMode == TEX_FILTER_LINEAR)
		szKernel=2;
	for(int k=0;k<szKernel;k++)
	{
		for(int l=0;l<szKernel;l++)
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
				else if(wrapType == WRAP_TYPE_REPEAT)
				{
					ntx = ntx-(int)floorf((float)ntx/image.width)*image.width;
					nty = nty-(int)floorf((float)nty/image.height)*image.height;
				}
				else if(wrapType == WRAP_TYPE_MIRROR)
				{
					ntx=abs(abs(fmodf(ntx,(2*image.width-1)))-(image.width-1));
					nty=abs(abs(fmodf(nty,(2*image.height-1)))-(image.height-1));
				}
			}
			for(int p=0;p<min(3,image.nChannels);p++)
			{
				uchar* ptr = PTR_PIX(image,ntx,nty)+p*((image.depth&0x7fffffff)>>3);
				switch(image.depth)
				{
				case IPL_DEPTH_8U:
					color[k][l].val[p] = *ptr;
					break;
				case IPL_DEPTH_8S:
					color[k][l].val[p] = *((char*)ptr);
					break;
				case IPL_DEPTH_16U:
					color[k][l].val[p] = *((ushort*)ptr);
					break;
				case IPL_DEPTH_16S:
					color[k][l].val[p] = *((short*)ptr);
					break;
				case IPL_DEPTH_32F:
					color[k][l].val[p] = *((float*)ptr);
					break;
				case IPL_DEPTH_32S:
					color[k][l].val[p] = *((long*)ptr);
					break;
				case IPL_DEPTH_64F:
					color[k][l].val[p] = *((double*)ptr);
					break;
				}
			}
		}
	}	

	if(filterMode == TEX_FILTER_NONE)
	{
		dstc=color[0][0];
	}
	else
	{
		if(dx+dy<=1.0)
		{
			dstc=color[0][0]+dx*(color[1][0]-color[0][0])+dy*(color[0][1]-color[0][0]);
		}
		else
		{
			dstc=color[1][1]+(1-dx)*(color[0][1]-color[1][1])+(1-dy)*(color[1][0]-color[1][1]);
		}
	}

	return dstc;
}