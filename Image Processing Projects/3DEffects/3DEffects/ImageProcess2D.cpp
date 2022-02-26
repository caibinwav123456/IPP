#include "stdafx.h"
#include "Process.h"
#include "ImageProcess2D.h"
#include "FFT.H"
#include <vector>
using namespace std;
bool g_bEnd = false;

void EndProcess()
{
	g_bEnd = true;
}

void PostEndProcess()
{
	g_bEnd = false;
}

bool EdgeProcess(IplImage* &pSrc, float nFilterScale, int nThresh, float satscale)//7,0.5
{
	int KernelSize = 5;//max(1,ceil(nFilterScale));
	CvMat* kernelx = cvCreateMat(2 * KernelSize + 1, 2 * KernelSize + 1, CV_32FC1);
	CvMat* kernely = cvCreateMat(2 * KernelSize + 1, 2 * KernelSize + 1, CV_32FC1);
	CvMat* kernelm = cvCreateMat(3, 3, CV_32FC1);
	IplImage* hsv=cvCreateImage(cvGetSize(pSrc), IPL_DEPTH_8U, 3);
	cvCvtColor(pSrc, hsv, CV_BGR2HSV);
	IplImage* s=cvCreateImage(cvGetSize(pSrc),IPL_DEPTH_8U,1);
	IplImage* v=cvCreateImage(cvGetSize(pSrc), IPL_DEPTH_8U,1);
	cvSplit(hsv, NULL, s, NULL, NULL);
	cvSplit(hsv, NULL, NULL, v, NULL);
	cvScale(s, s, satscale);
	cvCvtScale(v, v, 0.8);
	cvMerge(NULL, s, NULL, NULL, hsv);
	cvMerge(NULL, NULL, v, NULL, hsv);
	cvCvtColor(hsv,hsv,CV_HSV2BGR);
	cvReleaseImage(&s);
	cvReleaseImage(&v);

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

	for(i = -KernelSize; i <= KernelSize; i++)
		for(int j = -KernelSize; j <= KernelSize; j++)
		{
			float sqrk = nFilterScale*nFilterScale;
			float coeff = exp( -(float)(i*i + j*j) / sqrk );
			sum += coeff;
			cvSet2D(kernelx, i + KernelSize, j + KernelSize, cvScalar(-2*j * coeff / sqrk));
			cvSet2D(kernely, i + KernelSize, j + KernelSize, cvScalar(-2*i * coeff / sqrk));
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

	cvFilter2D(gray, dx, kernelx, cvPoint(KernelSize, KernelSize));
	cvFilter2D(gray, dy, kernely, cvPoint(KernelSize, KernelSize));

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
			if(ddir>pddir && ddir>nddir && sqrt(x*x + y*y) >= nThresh)
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
	cvNot(mask,mask);
	cvMul(hsv,mask,hsv, 1./255);
	//cvAdd(hsv, pSrc, pSrc);

	cvReleaseImage(&hsv);
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

void DimEffect(IplImage* src, IplImage* dest, Rect2D32f* roi)
{
	cvZero(dest);
	if(src==NULL)
		return;
	CvPoint2D32f center=cvPoint2D32f(0.5, 0.5);
	float roiWidth=1;
	float roiHeight=1;
	if(roi&&!(roi->x==0&&roi->y==0&&roi->cx==0&&roi->cy==0))
	{
		center.x=(float)(roi->x+roi->cx/2);
		center.y=(float)(roi->y+roi->cy/2);
		
		roiWidth=(float)roi->cx*4;
		roiHeight=(float)roi->cy*4;
	}
	IplImage* intg=cvCreateImage(cvGetSize(src), IPL_DEPTH_32S, 3);
	cvZero(intg);
	for(int i=0;i<src->width;i++)
	{
		for(int j=0;j<src->height;j++)
		{
			int pi=max(i-1, 0);
			int* pix=(int*)PTR_PIX(*intg,i,j);
			int* ppix=(int*)PTR_PIX(*intg,pi,j);
			uchar* psrc=PTR_PIX(*src,i,j);
			*pix=*ppix+*psrc;
			*(pix+1)=*(ppix+1)+*(psrc+1);
			*(pix+2)=*(ppix+2)+*(psrc+2);
			if(g_bEnd)goto end;
		}
	}
	for(int i=0;i<src->width;i++)
	{
		for(int j=0;j<src->height;j++)
		{
			int pj=max(j-1, 0);
			int* pix=(int*)PTR_PIX(*intg,i,j);
			int* ppix=(int*)PTR_PIX(*intg,i,pj);
			*pix+=*ppix;
			*(pix+1)+=*(ppix+1);
			*(pix+2)+=*(ppix+2);
			if(g_bEnd)goto end;
		}
	}
	for(int i=0;i<dest->height;i++)
	{
		float y=(float)i/dest->height;
		for(int j=0;j<dest->width;j++)
		{
			float x=(float)j/dest->width;
			float lk=sqrt(powf((x-center.x)/roiWidth,2.)+powf((y-center.y)/roiHeight,2.))*0.02;

			int lkx=lk*intg->width;
			int lky=lk*intg->height;
			//if(lkx<1)lkx=1;
			//if(lky<1)lky=1;
			Point2D vert[4]={Point2D(j-lkx-1,i-lky-1), Point2D(j+lkx,i-lky-1), Point2D(j-lkx-1,i+lky), Point2D(j+lkx,i+lky)};
			int sign[4]={1, -1, -1, 1};
			Vec3 fill(0,0,0);
			float np=0;
			int k;
			for(k=0;k<4;k++)
			{
				Point2D v=vert[k];
				if(v.x<0)v.x=0;
				else if(v.x>=dest->width)v.x=dest->width-1;
				if(v.y<0)v.y=0;
				else if(v.y>=dest->height)v.y=dest->height-1;
				int* pix=(int*)PTR_PIX(*intg, v.x, v.y);
				fill+=sign[k]*Vec3(*pix, *(pix+1), *(pix+2));
				np+=sign[k]*v.x*v.y;
			}
			fill/=np;

			for(k=0;k<3;k++)
			{
				if(fill.elem[k]<0)
					fill.elem[k]=0;
				else if(fill.elem[k]>255)
					fill.elem[k]=255;
			}

			uchar* pix=PTR_PIX(*dest, j,i);
			*pix=fill.x;
			*(pix+1)=fill.y;
			*(pix+2)=fill.z;
			if(g_bEnd)goto end;
		}
	}

end:
	//cvReleaseImage(&tmp);
	cvReleaseImage(&intg);
}

#define DIM_GRID 25

void Mist(IplImage* src, IplImage* dest, int dens)
{
	cvZero(dest);
	if(src==NULL)
		return;
	IplImage* himg;
	himg=cvCreateImage(cvGetSize(dest),IPL_DEPTH_32F, 1);

	Perlin(himg, dens*himg->width/himg->height, dens);
	int i;

	for(i=0;i<dest->height;i++)
	{
		float y=(float)i/dest->height;
		for(int j=0;j<dest->width;j++)
		{
			float x=(float)j/dest->width;
			float h=*(float*)PTR_PIX(*himg, j, i);
			float hx=*(float*)PTR_PIX(*himg, min(j+1,himg->width-1),i);
			float hy=*(float*)PTR_PIX(*himg, j, min(i+1,himg->height-1));

			Vec2  norm=Vec2(h-hx,h-hy);
			if(h<0.8)
				norm = Vec2(0,0);
			else
			{
				norm/=2*sqrt((h-0.8)*200);
			}
			Vec3 norm3=Vec3(norm.x,norm.y,1./himg->height);
			norm3=norm3.normalize();
			float fac=dot(norm3, normalize(Vec3(-0.5,-0.5,1)));
			fac=fabs(1.5*fac);
			if(fac>1)
				fac=1;
			if(h<=0.8)
				fac=1;

			Vec2 offset(0,0);
			float ffrac=0.75;
			Vec3 lin(0,0,1);
			Vec3 offin=lin-dot(lin, norm3)*norm3;
			Vec3 offout=offin*ffrac;
			float frac=sqrt(1-dot(offout,offout));
			Vec3 refrac=frac*norm3+offout;
			if(h>0.8)
				offset=Vec2(refrac.x,refrac.y)/refrac.z*sqrt(h-0.8);

			int ni=i+offset.x*dest->height;
			int nj=j+offset.y*dest->height;
			if(ni<0)ni=0;
			else if(ni>=dest->height)ni=dest->height-1;
			if(nj<0)nj=0;
			else if(nj>=dest->width)nj=dest->width-1;
			
			uchar* pix=PTR_PIX(*src, nj, ni);

			*PTR_PIX(*dest, j, i)=max(0,min(*pix*fac,255));
			*(PTR_PIX(*dest, j, i)+1)=max(0,min(*(pix+1)*fac,255));
			*(PTR_PIX(*dest, j, i)+2)=max(0,min(*(pix+2)*fac,255));
		}
	}
	cvReleaseImage(&himg);
}

void _Snow(IplImage* src, IplImage* dest, int nSnow, float rSnow)
{
	int kl=25*min(dest->width,dest->height)/767;
	IplImage* mask=cvCreateImage(cvSize(2*kl+1,2*kl+1),IPL_DEPTH_32F, 1);
	for(int i=0;i<2*kl+1;i++)
	{
		for(int j=0;j<2*kl+1;j++)
		{
			int r2=(i-kl)*(i-kl)+(j-kl)*(j-kl);
			float fac=exp(-(float)(r2)/(kl*kl)/rSnow/rSnow);
			if(r2>kl*kl)
			{
				fac=0;
			}
			*(float*)PTR_PIX(*mask,i,j)=fac;
		}
	}

	for(int k=0;k<nSnow;k++)
	{
		int x=rand()*dest->width/(RAND_MAX+1);
		int y=rand()*dest->height/(RAND_MAX+1);
		for(int i=-kl;i<=kl;i++)
		{
			for(int j=-kl;j<=kl;j++)
			{
				int nx=x+j;
				int ny=y+i;
				if(nx>=0&&nx<dest->width&&ny>=0&&ny<dest->height)
				{
					uchar* pdest=PTR_PIX(*dest, nx, ny);
					//uchar* psrc=PTR_PIX(*src, nx, ny);
					float* pmask=(float*)PTR_PIX(*mask, j+kl, i+kl);
					*pdest=255-(255-*pdest)*(1-*pmask);
					*(pdest+1)=255-(255-*(pdest+1))*(1-*pmask);
					*(pdest+2)=255-(255-*(pdest+2))*(1-*pmask);
				}
			}
		}
	}
	cvReleaseImage(&mask);
}

void Snow(IplImage* src, IplImage* dest, int nSnowBig, int nSnowMid, int nSnowSmall)
{
	//srand(time(NULL));
	cvResize(src, dest);
	_Snow(src, dest,nSnowBig/*300*/, 1/sqrt(5.));
	_Snow(src, dest,nSnowMid/*150*/,1/sqrt(10.));
	_Snow(src, dest,nSnowSmall/*600*/,1/sqrt(50.));
}

void WaterPaint(IplImage* src, IplImage* dest, float size)
{
	if(!src)
		return;
	CvSize szTmp;
	if(dest->width<=400&&dest->height<=400)
	{
		szTmp=cvGetSize(dest);
	}
	else
	{
		float xscale=600./dest->width;
		float yscale=600./dest->height;
		float scale=min(xscale,yscale);
		szTmp.width=cvRound(dest->width*scale*size);
		szTmp.height=cvRound(dest->height*scale*size);
		if(szTmp.width<1)szTmp.width=1;
		if(szTmp.height<1)szTmp.height=1;
	}
	IplImage* tmp1=cvCreateImage(szTmp,IPL_DEPTH_32F,3);
	IplImage* tmp2=cvCreateImage(szTmp,IPL_DEPTH_32F,3);
	IplImage* resize=cvCreateImage(szTmp,IPL_DEPTH_8U, 3);
	cvResize(src, resize);
	cvConvert(resize, tmp1);
	cvConvert(resize, tmp2);
	IplImage* tmp[2]={tmp1,tmp2};
	int index=0;
	int ptimes=0;
	bool bEnd = false;
	while(!bEnd)
	{
		IplImage* src=tmp[index];
		IplImage* dest=tmp[1-index];

		float criterion=0;
		for(int i=0;i<src->height;i++)
		{
			for(int j=0;j<src->width;j++)
			{
				float* p=(float*)PTR_PIX(*src, j, i);
				Vec3 d(0,0,0);
				POINT off[4]={{1,0},{0,1},{-1,0},{0,-1}};
				for(int k=0;k<4;k++)
				{
					int nx=j+off[k].x,ny=i+off[k].y;
					if(nx<0)nx=0;
					if(nx>src->width-1)nx=src->width-1;
					if(ny<0)ny=0;
					if(ny>src->height-1)ny=src->height-1;
					float* pd=(float*)PTR_PIX(*src, nx, ny);
					for(int l=0;l<3;l++)
					{
						float ds=*(pd+l)-*(p+l);
						float dd;//=ds*(ds-dmax.elem[l]*0.3)*(ds-dmax.elem[l]*0.7)/0.21/dmax.elem[l]/dmax.elem[l]/dmax.elem[l];
						if(ds>0)dd=1;
						else if(ds<0)dd=-1;
						else dd=0;
						d.elem[l]+=dd;
					}
				}
				Vec3 force;

				for(int k=0;k<3;k++)
				{
					force.elem[k]=(*(PTR_PIX(*resize,j,i)+k)-*(p+k))/20;
				}

				float* pdest=(float*)PTR_PIX(*dest, j,i);
				for(int k=0;k<3;k++)
				{
					if(criterion<fabs(force.elem[k]+d.elem[k]))
						criterion=fabs(force.elem[k]+d.elem[k]);
					*(pdest+k)=*(p+k)+force.elem[k]+d.elem[k];
				}
				if(g_bEnd)goto end;
			}
		}
		if(criterion<1)bEnd=true;
		ptimes++;
		if(ptimes>20)break;
		index=1-index;
	}
	cvConvert(tmp1, resize);
	cvResize(resize, dest);
end:
	cvReleaseImage(&tmp1);
	cvReleaseImage(&tmp2);
	cvReleaseImage(&resize);
}

void WaxPen(IplImage* src,IplImage* dest, int szStroke, float hardness)
{
	//srand(time(NULL));
	cvSet(dest, cvScalarAll(255));

	IplImage* tmp=cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 3);
	cvResize(src, tmp);

	vector<CvPoint> ptlist;
	//srand(time(NULL));
	for(int i=-szStroke;i<szStroke;i++)//stroke=10, hardness=0.5
	{
		for(int j=-szStroke;j<szStroke;j++)
		{
			if(i+j>=-szStroke/2&&i+j<szStroke/2)
			{
				ptlist.push_back(cvPoint(i,j));
			}
		}
	}
	int size=(int)ptlist.size()*(1-hardness);
	for(int i=0;i<size;i++)
	{
		int pos=rand()*ptlist.size()/(RAND_MAX+1);
		ptlist.erase(ptlist.begin()+pos);
	}

	int nP=dest->width*dest->height;
	if(ptlist.size()==0)
		goto next;
	while(nP>500)
	{
		int x=rand()*dest->width/(RAND_MAX+1);
		int y=rand()*dest->height/(RAND_MAX+1);
		int nPix=0;
		uchar* psrc=PTR_PIX(*tmp, x, y);
		for(int i=0;i<(int)ptlist.size();i++)
		{
			CvPoint pt=ptlist.at(i);
			CvPoint ptn=cvPoint(pt.x+x, pt.y+y);
			if(ptn.x>=0&&ptn.x<dest->width&&ptn.y>=0&&ptn.y<dest->height)
			{
				uchar* pdest=PTR_PIX(*dest, ptn.x, ptn.y);
				if(*pdest==255&&*(pdest+1)==255&&*(pdest+2)==255)
				{
					*pdest=*psrc;
					*(pdest+1)=*(psrc+1);
					*(pdest+2)=*(psrc+2);
					nPix++;
				}
			}
		}
		if(g_bEnd)goto end;
		nPix=max(1,nPix);
		nP-=nPix;
	}
next:
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			uchar* pix=PTR_PIX(*dest, j, i);
			if(*pix==255&&*(pix+1)==255&&*(pix+2)==255)
			{
				uchar* ps=PTR_PIX(*tmp, j, i);
				*pix=*ps;
				*(pix+1)=*(ps+1);
				*(pix+2)=*(ps+2);
			}
		}
	}
end:
	cvReleaseImage(&tmp);
}

void WoodPrint(IplImage* src, IplImage* dest, IplImage* foregnd, IplImage* bkgnd, float treadlen, float treadrag)
{
	IplImage* tmp=cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 3);
	IplImage* gray=cvCreateImage(cvGetSize(dest),IPL_DEPTH_8U, 1);
	IplImage* carve=cvCreateImage(cvGetSize(dest),IPL_DEPTH_8U, 3);
	IplImage* tfore=cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 3);
	IplImage* tbk=cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 3);
	cvResize(src, tmp);
	cvResize(foregnd, tfore);
	cvResize(bkgnd, tbk);
	cvCvtColor(tmp, gray, CV_RGB2GRAY);

	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			float y=(float)i/dest->height;
			float dx=sin(y*PI/treadlen);//*5
			float x=(float)j/dest->width;
			x+=dx*treadrag;//.05;
			float h=fabs(fmod(x*100,2)-1);
			uchar pixgray = *PTR_PIX(*gray, j, i);
			uchar* pcarve =  PTR_PIX(*carve, j, i);
			float diff = (pixgray-h*255)*1+128;
			if(diff>255)diff=255;
			else if(diff<0)diff=0;
			*pcarve = diff;
			*(pcarve+1) = diff;
			*(pcarve+2) = diff;
			//*PTR_PIX(*mask, j,i)=255*h;
		}
	}

	cvMul(tfore, carve, tfore, 1./255);
	cvNot(carve,carve);
	cvMul(tbk, carve, tbk, 1./255);
	cvAdd(tfore, tbk, dest);
	cvReleaseImage(&gray);
	cvReleaseImage(&carve);
	cvReleaseImage(&tmp);
	cvReleaseImage(&tfore);
	cvReleaseImage(&tbk);
}

void WaxPen2(IplImage* src,IplImage* dest, int szStroke, float hardness, int nGrain)
{
	vector<CvPoint> ptlist;
	vector<CvPoint> pthigh;
	cvSet(dest, cvScalarAll(255));

	for(int i=-szStroke;i<=szStroke;i++)//5
	{
		for(int j=-szStroke;j<=szStroke;j++)
		{
			if(i*i+j*j<=szStroke*szStroke)
			{
				ptlist.push_back(cvPoint(i,j));
			}
		}
	}
	int size=(int)ptlist.size()*(1-hardness);//0.5
	for(int i=0;i<size;i++)
	{
		int pos=rand()*ptlist.size()/(RAND_MAX+1);
		ptlist.erase(ptlist.begin()+pos);
	}
	size=nGrain;//3//(int)m_ptlist.size()/10;
	for(int i=0;i<size;i++)
	{
		if(ptlist.size()==0)break;
		int pos=rand()*ptlist.size()/(RAND_MAX+1);
		pthigh.push_back(ptlist.at(pos));
		ptlist.erase(ptlist.begin()+pos);
	}
	int nP=dest->width*dest->height;
	if(ptlist.size()==0&&pthigh.size()==0)
		goto next;
	while(nP>500)
	{
		int x=rand()*dest->width/(RAND_MAX+1);
		int y=rand()*dest->height/(RAND_MAX+1);
		int nPix=0;
		uchar* psrc=PTR_PIX(*src, x, y);
		for(int i=0;i<(int)ptlist.size();i++)
		{
			CvPoint pt=ptlist.at(i);
			CvPoint ptn=cvPoint(pt.x+x, pt.y+y);
			if(ptn.x>=0&&ptn.x<dest->width&&ptn.y>=0&&ptn.y<dest->height)
			{
				uchar* pdest=PTR_PIX(*dest, ptn.x, ptn.y);
				if(*pdest==255&&*(pdest+1)==255&&*(pdest+2)==255)
				{
					*pdest=*psrc;
					*(pdest+1)=*(psrc+1);
					*(pdest+2)=*(psrc+2);
					nPix++;
				}
			}
		}
		for(int i=0;i<(int)pthigh.size();i++)
		{
			CvPoint pt=pthigh.at(i);
			CvPoint ptn=cvPoint(pt.x+x, pt.y+y);
			if(ptn.x>=0&&ptn.x<dest->width&&ptn.y>=0&&ptn.y<dest->height)
			{
				uchar* pdest=PTR_PIX(*dest, ptn.x, ptn.y);
				if(*pdest==255&&*(pdest+1)==255&&*(pdest+2)==255)
				{
					uchar b=*psrc;
					uchar g=*(psrc+1);
					uchar r=*(psrc+2);
					int mean=((int)r+g+b)/3;
					{
						*pdest=max(0,min(255,(int)(b-mean)*2+mean));//*255/(maxc-minc);
						*(pdest+1)=max(0,min(255,(int)(g-mean)*2+mean));//*255/(maxc-minc);
						*(pdest+2)=max(0,min(255,(int)(r-mean)*2+mean));//*255/(maxc-minc);
					}
					nPix++;
				}
			}
		}
		nPix=max(1,nPix);
		nP-=nPix;
	}
next:
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			uchar* pix=PTR_PIX(*dest, j, i);
			if(*pix==255&&*(pix+1)==255&&*(pix+2)==255)
			{
				uchar* ps=PTR_PIX(*src, j, i);
				*pix=*ps;
				*(pix+1)=*(ps+1);
				*(pix+2)=*(ps+2);
			}
		}
	}
}

void OilPaint(IplImage* src, IplImage* dest, int szStroke)
{
	cvSet(dest, cvScalarAll(255));
	vector<CvPoint> ptlist;
	int* bhairs=new int[szStroke*2+1];
	int* bhaire=new int[szStroke*2+1];
	bhairs[0]=5+5*rand()/RAND_MAX;
	bhaire[0]=-10+5*rand()/RAND_MAX;
	IplImage* mask=cvCreateImage(cvGetSize(dest),IPL_DEPTH_32S,1);
	cvSet(mask, cvScalar(0));

	for(int i=1;i<szStroke*2+1;i++)
	{
		bhairs[i]=bhairs[i-1]+rand()*3/(RAND_MAX+1)-1;
		bhaire[i]=bhaire[i-1]+rand()*3/(RAND_MAX+1)-1;
	}
	for(int i=-szStroke;i<=szStroke;i++)//5
	{
		for(int j=-szStroke;j<=szStroke;j++)
		{
			int a=i+j;
			int b=i-j;

			if(abs(a)<=szStroke)
			{
				if(b<bhairs[a+szStroke]&& b>bhaire[a+szStroke])
					ptlist.push_back(cvPoint(i,j));
			}
		}
	}
	delete[] bhaire;
	delete[] bhairs;
	int nP=dest->width*dest->height;
	if(ptlist.size()==0)
		goto end;
	while(nP>500)
	{
		int x=rand()*dest->width/(RAND_MAX+1);
		int y=rand()*dest->height/(RAND_MAX+1);
		int nPix=0;
		uchar* psrc=PTR_PIX(*src, x, y);
		for(int i=0;i<(int)ptlist.size();i++)
		{
			CvPoint pt=ptlist.at(i);
			CvPoint ptn=cvPoint(pt.x+x, pt.y+y);
			if(ptn.x>=0&&ptn.x<dest->width&&ptn.y>=0&&ptn.y<dest->height)
			{
				uchar* pdest=PTR_PIX(*dest, ptn.x, ptn.y);
				int* pmask=(int*)PTR_PIX(*mask, ptn.x, ptn.y);
				if(*pdest==255&&*(pdest+1)==255&&*(pdest+2)==255)
				{
					*pdest=*psrc;
					*(pdest+1)=*(psrc+1);
					*(pdest+2)=*(psrc+2);
					nPix++;
				}
				else
				{
					*pdest=(uchar)(((int)*pdest**pmask+*psrc)/(*pmask+1));
					*(pdest+1)=(uchar)(((int)*(pdest+1)**pmask+*(psrc+1))/(*pmask+1));
					*(pdest+2)=(uchar)(((int)*(pdest+2)**pmask+*(psrc+2))/(*pmask+1));
				}
				(*pmask)++;
			}
		}
		nPix=max(1,nPix);
		nP-=nPix;
	}
end:
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			uchar* pix=PTR_PIX(*dest, j, i);
			if(*pix==255&&*(pix+1)==255&&*(pix+2)==255)
			{
				uchar* ps=PTR_PIX(*src, j, i);
				*pix=*ps;
				*(pix+1)=*(ps+1);
				*(pix+2)=*(ps+2);
			}
		}
	}
	cvReleaseImage(&mask);
}

void Rain(IplImage* src, IplImage* dest, float rain_min_length, float rain_max_length, int nDrops)
{
	cvResize(src, dest);
	Vec2 rain_vec(-0.2, 1);
	for(int i=0;i<nDrops;i++)//10000
	{
		int x=(float)rand()/(RAND_MAX+1)*dest->width;
		int y=(float)rand()/(RAND_MAX+1)*dest->height;
		
		int length=rain_min_length+(float)rand()/(RAND_MAX)*(rain_max_length-rain_min_length);
		for(int j=-length;j<length;j++)
		{
			int startx=x+j/rain_vec.length()*rain_vec.x;
			int starty=y+j/rain_vec.length()*rain_vec.y;
			if(startx<0||startx>=dest->width||starty<0||starty>=dest->height)
				continue;
			uchar* pix=PTR_PIX(*dest, startx, starty);
			*pix=255-(255-*pix)/1.1;
			*(pix+1)=255-(255-*(pix+1))/1.1;
			*(pix+2)=255-(255-*(pix+2))/1.1;
		}
	}
}

void Wave(IplImage* src, IplImage* dest, float wavelen, float wavh, float refrac, Vec3 ldir)
{
	ldir=ldir.normalize();
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			float x=(float)(j-dest->width/2)/min(dest->width,dest->height);
			float y=(float)(i-dest->height/2)/min(dest->width,dest->height);
			
			float sqr=(x*x+y*y)/wavelen/wavelen;
			float h=wavh*cos(sqr);
			Vec2 dh(2*x,2*y);
			dh*=-wavh*sin(sqr)/wavelen/wavelen;
			Vec3 normal(dh.x, dh.y, 1);
			normal=normal.normalize();

			Vec3 lin(0,0,1);
			Vec3 offin=lin-dot(lin, normal)*normal;
			Vec3 offout=offin*refrac;
			float frac=sqrt(1-dot(offout,offout));
			Vec3 refr=frac*normal+offout;
			Vec2 offset(0,0);
			offset=Vec2(refr.x,refr.y)/refr.z*h;
			
			float l=dot(normal, normalize(ldir));
			if(l<0.2)l=0.2;
			float lh=pow(l,100);

			int ni=i+offset.x*dest->height;
			int nj=j+offset.y*dest->height;
			if(ni<0)ni=0;
			else if(ni>=dest->height)ni=dest->height-1;
			if(nj<0)nj=0;
			else if(nj>=dest->width)nj=dest->width-1;

			uchar* pix=PTR_PIX(*src, nj, ni);
			uchar* opix=PTR_PIX(*dest, j, i);

			*opix=max(0,min(*pix*l+lh*100,255));
			*(opix+1)=max(0,min(*(pix+1)*l+lh*100,255));
			*(opix+2)=max(0,min(*(pix+2)*l+lh*100,255));
		}
	}
}

void Mosaic(IplImage* dst, float nSize, Rect2D32f* roi)
{
	if (NULL == dst)
		return ;

	if (nSize <= 0)
		return ;

	int sizeMosaic = nSize*min(roi->cx*dst->width, roi->cy*dst->height);

	if (sizeMosaic < 1)
	{
		sizeMosaic = 1;
	}

	int x, y, z;
	int r=0, g=0, b=0;
	vector<uchar*> vec;

	int tl_x = static_cast<int>(roi->x * dst->width);
	int tl_y = static_cast<int>(roi->y * dst->height);
	int br_x = static_cast<int>(roi->cx *dst->width) + tl_x;
	int br_y = static_cast<int>(roi->cy * dst->height) + tl_y;

	if (roi->x < 0 || roi->y < 0 || roi->cx < 0 || roi->cy < 0.0f)
	{
		tl_x = 0;
		tl_y = 0;
		br_x = dst->width;
		br_y = dst->height;
	}

	if (br_x > dst->width)
		br_x = dst->width;

	if (br_y > dst->height)
		br_y = dst->height;

	for (y = tl_y; y < br_y; y += sizeMosaic)
	{
		for (x = tl_x; x < br_x; x++)
		{
			if (x % sizeMosaic == 0)
			{
				r = 0;
				g = 0;
				b = 0;
				vec.clear();
			}

			for (z = y; z < y + sizeMosaic; z++)
			{
				if (z >= br_y)
				{
					break;
				}

				uchar* ptr = (uchar*)(dst->imageData + z * dst->widthStep);
				r += ptr[3*x + 2];
				g += ptr[3*x + 1];
				b += ptr[3*x];
				vec.push_back(ptr + 3*x);
			}

			if (x % sizeMosaic == sizeMosaic - 1 || x == br_x - 1)
			{
				if (vec.size() <= 0)
					continue;

				r = r / vec.size(); 
				if (r > 255) r = 255;
				if (r < 0) r = 0;

				g = g / vec.size();
				if (g > 255) g = 255;
				if (g < 0) g = 0;

				b = b / vec.size();
				if (b > 255) b = 255;
				if (b < 0) b = 0;

				while (! vec.empty())
				{
					uchar *ptr = vec.back();
					*ptr = b;
					*(ptr + 1) = g;
					*(ptr + 2) = r;
					vec.pop_back();
				}
			}
		}
	}
}

void Emboss(IplImage* dst, int nDepth, int nBrightness)
{
	int i;
	int x, y;
	int tmp;
	int x_bottomRight;

	if (NULL == dst)
		return ;

	// 	IplImage *src = cvCloneImage(dst);
	// 	if (NULL == src)
	// 		return false;

	for (i = 0; i < dst->nChannels; i++)
	{
		for (y = 0; y < dst->height; y++)
		{
			uchar*ptr_src;
			if (y + nDepth >= dst->height)
			{
				ptr_src = (uchar*)(dst->imageData + dst->widthStep * (dst->height - 1));	
			}
			else
			{
				ptr_src = (uchar*)(dst->imageData + dst->widthStep * (y + nDepth));
			}

			uchar*ptr_dst = (uchar*)(dst->imageData + dst->widthStep * y);

			for (x = 0; x < dst->width; x++)
			{
				x_bottomRight = x + nDepth;
				if (x_bottomRight >= dst->width) x_bottomRight = dst->width - 1;

				tmp = ptr_dst[3 * x + i];
				tmp = tmp - ptr_src[3 * x_bottomRight + i] + nBrightness;
				if (tmp > 255)
					tmp = 255;
				else if (tmp < 0)
					tmp = 0;

				ptr_dst[3 * x + i] = (uchar)tmp;
			}
		}
	}
}

void Grid3D(IplImage* dst, int nSize, int nDepth)
{
	if (NULL == dst)
		return;

	bool bScaled = false;
	CvSize szGridMap;
	if(dst->width<=1024 && dst->height<=1024)
	{
		szGridMap = cvGetSize(dst);
	}
	else
	{
		float xscale = 1024./dst->width;
		float yscale = 1024./dst->height;
		float scale = min(xscale, yscale);
		szGridMap = cvSize(cvRound(dst->width*scale), cvRound(dst->height*scale));
		bScaled = true;
	}

	IplImage* imggrid = cvCreateImage(szGridMap, IPL_DEPTH_32F, 1);
	cvZero(imggrid);
	int x, y;
	const int nWidth = nSize;
	for (y = 0; y < imggrid->height; y++)
	{
		uchar* ptr = (uchar*)(imggrid->imageData + y * imggrid->widthStep);
		int delta = y % nWidth;
		bool bAllLine = false;
		if (delta > nWidth - 4)
		{
			bAllLine = true;
		}
		for (x = 0; x < imggrid->width; x++)
		{
			int nAmount = 0;
			if (bAllLine)
			{
				//
			}
			else
			{
				delta = x % nWidth;
				if (delta <= nWidth - 4)
				{
					continue;
				}
			}

			if (nWidth -3 == delta)
			{
				nAmount = -nDepth;
			}
			else if (nWidth - 1 == delta)
			{
				nAmount = nDepth;
			}
			else
			{
				nAmount = 0;
			}


			*(float*)(ptr+4*x) = nAmount;
		}
	}

	for(int i=0;i<dst->height;i++)
	{
		for(int j=0;j<dst->width;j++)
		{
			uchar* pix = PTR_PIX(*dst, j, i);
			Vec3 smp = Sample(imggrid, Vec2((float)j/dst->width,(float)i/dst->height));
			int val=*pix+smp.x;
			if(val<0)val=0;
			else if(val>255)val=255;
			*pix=val;
			val=*(pix+1)+smp.x;
			if(val<0)val=0;
			else if(val>255)val=255;
			*(pix+1)=val;
			val=*(pix+2)+smp.x;
			if(val<0)val=0;
			else if(val>255)val=255;
			*(pix+2)=val;
		}
	}
	cvReleaseImage(&imggrid);
}

void Sketch(IplImage* src, IplImage* dest)
{
	IplImage* gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src, gray, CV_RGB2GRAY);
	CvScalar sum = cvSum(gray);
	float brightness = sum.val[0]/(gray->width*gray->height);
	IplImage* grayf = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	cvCvtScale(gray, grayf);
	cvReleaseImage(&gray);
	CvMat* kernel = cvCreateMat(1, 1, CV_32FC1);
	CvMat* kernel2 = cvCreateMat(41,41, CV_32FC1);
	CvMat* kernel3 = cvCreateMat(51,51, CV_32FC1);
	cvSet(kernel, cvScalar(1.0f/1));
	cvSet(kernel2, cvScalar(1.0f/41/41));
	cvSet(kernel3, cvScalar(1.0f/51/51));
	IplImage* smooth = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	IplImage* smooth2 = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	IplImage* smooth3 = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	cvFilter2D(grayf, smooth, kernel,cvPoint(0,0));
	cvFilter2D(grayf, smooth2, kernel2, cvPoint(20,20));
	cvFilter2D(grayf, smooth3, kernel3, cvPoint(25,25));
	IplImage* diff = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	cvSub(smooth, smooth2, diff);
	cvReleaseImage(&grayf);
	cvReleaseImage(&smooth);
	cvReleaseImage(&smooth2);
	IplImage* destg = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	float ext = (1-expf(-(brightness-93)/(188-93)*20))/(1-expf(-20));
	if(ext<0)ext=0;
	if(ext>1)ext=1;
	int level = 235+ext*(250-235);//250;//188,93|250,235
	float nmin = level;
	float nmax = 255-level;
	float scale = 1.8+ext*(2.5-1.8);//1.8;//2.5;
	float scale2 = nmax*(1.5/25);
	for(int i=0;i<diff->height;i++)
	{
		for(int j=0;j<diff->width;j++)
		{
			float* ptr = (float*)PTR_PIX(*diff, j, i);
			uchar* ptr2 = PTR_PIX(*destg, j, i);
			float* ptr3 = (float*)PTR_PIX(*smooth3, j, i);
			float re;
			float input = *ptr;//+1
			float s = *ptr3;
			if(s<10)s=10;
			input/=s/125;
			if(input<0)
			{
				re = (exp(input*scale/nmin)-1)*nmin;
			}
			else
			{
				re = (1-exp(-input*scale2/nmax))*nmax;
			}
			re+=level;
			//re+=*ptr3-220;
			*ptr2 = max(0,min(re, 255));
		}
	}
	//cvScale(diff, destg, 3.5, level);
	cvCvtColor(destg, dest, CV_GRAY2RGB);
	cvReleaseImage(&diff);
	cvReleaseImage(&destg);
	cvReleaseImage(&smooth3);
	cvReleaseMat(&kernel);
	cvReleaseMat(&kernel2);
	cvReleaseMat(&kernel3);
}


float SmoothStep(float x, float thresh, float width)
{
	if(x<=thresh-width/2)
		return 0;
	else if(x>=thresh+width/2)
		return 1;
	else
	{
		float scale = (x-thresh)/(width/2);
		float ret = (scale*3-powf(scale,3))/2;
		if(ret<0)ret=0;
		if(ret>1)ret=1;
		return ret;
	}
}

float SmoothSlope(float x, float top, float edge)
{
	if(x<top-edge)
		return x;
	else if(x>top+edge)
		return top;
	else
		return top-powf((x-top-edge)/2/edge, 2)*edge;
}

float SmoothSlopeB(float x, float bottom, float edge)
{
	if(x>bottom+edge)
		return x;
	else if(x<bottom-edge)
		return bottom;
	else
		return bottom+powf((x-bottom+edge)/2/edge, 2)*edge;
}

float FrameFunc(float x, float edge, float left, float right)
{
	if(x >= left+edge && x < right-edge)
	{
		return 1;
	}
	else if(x < left || x > right)
	{
		return 0;
	}
	else
	{
		if(x<edge)
		{
			x-=edge;
		}
		else
		{
			x-=1-edge;
		}
		return (cos(x/edge*PI)+1)/2;
	}
}
