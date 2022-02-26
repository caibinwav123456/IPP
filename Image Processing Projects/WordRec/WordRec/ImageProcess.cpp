#include "stdafx.h"
#include "ImageProcess.h"

Vec3 Sample(RawImage image, Vec2 tex, int wrapType, int filterMode, bool bUniformCoord)
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

	Vec3 color[2][2];
	Vec3 dstc(0,0,0);

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
					color[k][l]=Vec3(128,128,128);
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
					ntx=ntx%image.width;
					nty=nty%image.height;
				}
				else if(wrapType == WRAP_TYPE_MIRROR)
				{
					ntx=abs(ntx%(2*image.width-1)-(image.width-1));
					nty=abs(nty%(2*image.height-1)-(image.height-1));
				}
			}
			for(int p=0;p<image.nChannels;p++)
			{
				uchar* ptr = PTR_PIX(image,ntx,nty)+p*((image.depth&0x7fffffff)>>3);
				switch(image.depth)
				{
				case IPL_DEPTH_8U:
					color[k][l].elem[p] = *ptr;
					break;
				case IPL_DEPTH_8S:
					color[k][l].elem[p] = *((char*)ptr);
					break;
				case IPL_DEPTH_16U:
					color[k][l].elem[p] = *((ushort*)ptr);
					break;
				case IPL_DEPTH_16S:
					color[k][l].elem[p] = *((short*)ptr);
					break;
				case IPL_DEPTH_32F:
					color[k][l].elem[p] = *((float*)ptr);
					break;
				case IPL_DEPTH_32S:
					color[k][l].elem[p] = *((long*)ptr);
					break;
				case IPL_DEPTH_64F:
					color[k][l].elem[p] = *((double*)ptr);
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