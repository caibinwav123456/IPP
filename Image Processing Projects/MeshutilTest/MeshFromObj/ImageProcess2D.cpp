#include "stdafx.h"
#include "Process.h"
#include "ImageProcess3D.h"
#include "ImageProcess2D.h"
#include <vector>
using namespace std;

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

bool bEnd;

void DimEffect(IplImage* src, IplImage* dest)
{
	bEnd=false;
	cvZero(dest);
	if(src==NULL)
		return;
/*	IplImage* tmp=cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 3);
	cvZero(tmp);

	for(int i=0;i<tmp->height;i++)
	{
		float y=(float)i/tmp->height;
		for(int j=0;j<tmp->width;j++)
		{
			float x=(float)j/tmp->width;
			float lk=sqrt(pow((x-0.5),2.)+pow((y-0.5),2.))*0.02;
			float acc=0;
			Vec3  accp(0,0,0);
			for(float k=-lk;k<lk;k+=1./src->width)
			{
				float fac=exp(-k*k/lk/lk*3);
				acc+=fac;
				accp+=fac*Sample(src, Vec2(x+k,y));
			}
			accp/=acc;
			*PTR_PIX(*tmp,j,i)=accp.x;
			*(PTR_PIX(*tmp,j,i)+1)=accp.y;
			*(PTR_PIX(*tmp,j,i)+2)=accp.z;
			if(bEnd)goto end;
		}
	}
	for(int i=0;i<dest->height;i++)
	{
		float y=(float)i/dest->height;
		for(int j=0;j<dest->width;j++)
		{
			float x=(float)j/dest->width;
			float lk=sqrt(pow((x-0.5),2.)+pow((y-0.5),2.))*0.02;
			float acc=0;
			Vec3  accp(0,0,0);
			for(float k=-lk;k<lk;k+=1./tmp->height)
			{
				float fac=exp(-(k*k)/(lk*lk)*3);
				acc+=fac;
				accp+=fac*Sample(tmp, Vec2(x,y+k));
			}
			accp/=acc;
			*PTR_PIX(*dest,j,i)=accp.x;
			*(PTR_PIX(*dest,j,i)+1)=accp.y;
			*(PTR_PIX(*dest,j,i)+2)=accp.z;
			if(bEnd)goto end;
		}
	}*/
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
			if(bEnd)goto end;
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
			if(bEnd)goto end;
		}
	}
	for(int i=0;i<dest->height;i++)
	{
		float y=(float)i/dest->height;
		for(int j=0;j<dest->width;j++)
		{
			float x=(float)j/dest->width;
			float lk=sqrt(pow((x-0.5),2.)+pow((y-0.5),2.))*0.02;
			if(lk<1./intg->height)
				lk=1./intg->height;
			Vec3 ul=Sample(intg, Vec2(x-lk,y-lk));
			Vec3 ur=Sample(intg, Vec2(x+lk,y-lk));
			Vec3 ll=Sample(intg, Vec2(x-lk,y+lk));
			Vec3 lr=Sample(intg, Vec2(x+lk,y+lk));
			Vec3 fill=lr-ll-ur+ul;
			fill/=intg->width*intg->height*lk*lk*4;
			for(int k=0;k<3;k++)
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
			if(bEnd)goto end;
		}
	}

end:
	//cvReleaseImage(&tmp);
	cvReleaseImage(&intg);
}

#define DIM_GRID 25

void Mist(IplImage* src, IplImage* dest)
{
	cvZero(dest);
	if(src==NULL)
		return;
	float hmap[DIM_GRID][DIM_GRID];
	IplImage* himg=cvCreateImage(cvGetSize(dest),IPL_DEPTH_32F, 1);
	cvZero(himg);
	srand(time(NULL));
	for(int i=0;i<DIM_GRID;i++)
		for(int j=0;j<DIM_GRID;j++)
			hmap[i][j]=(float)rand()/RAND_MAX;
	bEnd=false;
	for(int i=0;i<DIM_GRID;i++)
	{
		for(int j=0;j<DIM_GRID;j++)
		{
			for(int k=0;k<dest->height;k++)
			{
				float y=(float)k/dest->height;
				for(int l=0;l<dest->width;l++)
				{
					float x=(float)l/dest->width;

					float hx=sin((x*(DIM_GRID-1)-i)*PI)/((x*(DIM_GRID-1)-i)*PI);
					if(x*(DIM_GRID-1)-i==0)
						hx=1;
					float hy=sin((y*(DIM_GRID-1)-j)*PI)/((y*(DIM_GRID-1)-j)*PI);
					if(y*(DIM_GRID-1)-j==0)
						hy=1;

					float h=hx*hy;
					*(float*)PTR_PIX(*himg, l, k)+=h*hmap[i][j];
				}
			}
			if(bEnd)goto end;
		}
	}

	for(int i=0;i<dest->height;i++)
	{
		float y=(float)i/dest->height;
		for(int j=0;j<dest->width;j++)
		{
			float x=(float)j/dest->width;
			float h=*(float*)PTR_PIX(*himg, j, i);
			float hx=*(float*)PTR_PIX(*himg, min(j+1,dest->width-1),i);
			float hy=*(float*)PTR_PIX(*himg, j, min(i+1,dest->height-1));

			Vec2  norm=Vec2(h-hx,h-hy);
			if(h<0.8)
				norm = Vec2(0,0);
			else
			{
				norm/=2*sqrt((h-0.8)*200);
			}
			Vec3 norm3=Vec3(norm.x,norm.y,1./dest->height);
			norm3=norm3.normalize();
			float fac=dot(norm3, normalize(Vec3(-0.5,-0.5,1)));
			fac=fabs(1.5*fac);
			if(fac>1)
				fac=1;
			if(h<=0.8)
				fac=1;
			Vec3 pix=Sample(src,Vec2(x,y))*fac;
			*PTR_PIX(*dest, j, i)=pix.x;
			*(PTR_PIX(*dest, j, i)+1)=pix.y;
			*(PTR_PIX(*dest, j, i)+2)=pix.z;
		}
	}
end:
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

void Snow(IplImage* src, IplImage* dest)
{
	srand(time(NULL));
	cvResize(src, dest);
	_Snow(src, dest,300, 1/sqrt(5.));
	_Snow(src, dest,150,1/sqrt(10.));
	_Snow(src, dest,600,1/sqrt(50.));
}

void WaterPaint(IplImage* src, IplImage* dest)
{
	bEnd=false;
	if(!src)
		return;
	CvSize szTmp;
	if(dest->width<=800&&dest->height<=800)
	{
		szTmp=cvGetSize(dest);
	}
	else
	{
		float xscale=800./dest->width;
		float yscale=800./dest->height;
		float scale=min(xscale,yscale);
		szTmp.width=dest->width*scale;
		szTmp.height=dest->height*scale;
	}
	IplImage* tmp1=cvCreateImage(szTmp,IPL_DEPTH_32F,3);
	IplImage* tmp2=cvCreateImage(szTmp,IPL_DEPTH_32F,3);
	IplImage* resize=cvCreateImage(szTmp,IPL_DEPTH_8U, 3);
	cvResize(src, resize);
	cvConvert(resize, tmp1);
	cvConvert(resize, tmp2);
	IplImage* tmp[2]={tmp1,tmp2};
	int index=0;
	bool bEnd=false;
	int ptimes=0;
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
				if(bEnd)goto end;
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

void WaxPen(IplImage* src,IplImage* dest)
{
	bEnd=false;
	srand(time(NULL));
	cvSet(dest, cvScalarAll(255));

	IplImage* tmp=cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 3);
	cvResize(src, tmp);

	vector<CvPoint> ptlist;
	srand(time(NULL));
	for(int i=-10;i<10;i++)
	{
		for(int j=-10;j<10;j++)
		{
			if(i+j>=-5&&i+j<5)
			{
				ptlist.push_back(cvPoint(i,j));
			}
		}
	}
	int size=(int)ptlist.size()/2;
	for(int i=0;i<size;i++)
	{
		int pos=rand()*ptlist.size()/(RAND_MAX+1);
		ptlist.erase(ptlist.begin()+pos);
	}

	int nP=dest->width*dest->height;
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
		if(bEnd)goto end;
		nP-=nPix;
	}
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

void WoodPrint(IplImage* src, IplImage* dest, IplImage* foregnd, IplImage* bkgnd)
{
	IplImage* tmp=cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 3);
	cvResize(src, tmp);

	IplImage* mask=cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 1);

	for(int i=0;i<mask->height;i++)
	{
		for(int j=0;j<mask->width;j++)
		{
			float y=(float)i/mask->height;
			float dx=sin(y*5*PI);
			float x=(float)j/mask->width;
			x+=dx*0.05;
			float h=fabs(fmod(x*300,2)-1);
			*PTR_PIX(*mask, j,i)=255*h;
		}
	}

	IplImage* gray=cvCreateImage(cvGetSize(dest),IPL_DEPTH_8U, 1);
	IplImage* carve=cvCreateImage(cvGetSize(dest),IPL_DEPTH_8U, 1);
	IplImage* hsv=cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 3);
	IplImage* tfore=cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 3);
	IplImage* tbk=cvCreateImage(cvGetSize(dest), IPL_DEPTH_8U, 3);
	cvResize(foregnd, tfore);
	cvResize(bkgnd, tbk);
	cvCvtColor(tmp, hsv, CV_RGB2HSV);
	cvSplit(hsv, NULL, NULL, gray, NULL);
	cvCmp(gray,mask,carve, CV_CMP_LT);
	//cvSet(dest, cvScalar(30,240,240), carve);
	cvCopy(tfore, dest, carve);
	cvNot(carve,carve);
	cvCopy(tbk, dest, carve);
	cvReleaseImage(&gray);
	cvReleaseImage(&carve);
	cvReleaseImage(&hsv);
	cvReleaseImage(&mask);
	cvReleaseImage(&tmp);
	cvReleaseImage(&tfore);
	cvReleaseImage(&tbk);
}

void WaxPen2(IplImage* src,IplImage* dest)
{
	vector<CvPoint> ptlist;
	vector<CvPoint> pthigh;
	cvSet(dest, cvScalarAll(255));
	int i;
	for(i=-5;i<=5;i++)
	{
		for(int j=-5;j<=5;j++)
		{
			if(i*i+j*j<=25)
			{
				ptlist.push_back(cvPoint(i,j));
			}
		}
	}
	int size=(int)ptlist.size()/2;
	for(i=0;i<size;i++)
	{
		int pos=rand()*ptlist.size()/(RAND_MAX+1);
		ptlist.erase(ptlist.begin()+pos);
	}
	size=3;//(int)m_ptlist.size()/10;
	for(i=0;i<size;i++)
	{
		int pos=rand()*ptlist.size()/(RAND_MAX+1);
		pthigh.push_back(ptlist.at(pos));
		ptlist.erase(ptlist.begin()+pos);
	}
	int nP=dest->width*dest->height;
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
		nP-=nPix;
	}

	for(i=0;i<dest->height;i++)
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

void OilPaint(IplImage* src, IplImage* dest)
{
	cvSet(dest, cvScalarAll(255));
	vector<CvPoint> ptlist;
	int bhairs[11];
	int bhaire[11];
	bhairs[0]=5+5*rand()/RAND_MAX;
	bhaire[0]=-10+5*rand()/RAND_MAX;
	IplImage* mask=cvCreateImage(cvGetSize(dest),IPL_DEPTH_32S,1);
	cvSet(mask, cvScalar(0));

	for(int i=1;i<11;i++)
	{
		bhairs[i]=bhairs[i-1]+rand()*3/(RAND_MAX+1)-1;
		bhaire[i]=bhaire[i-1]+rand()*3/(RAND_MAX+1)-1;
	}
	for(int i=-5;i<=5;i++)
	{
		for(int j=-5;j<=5;j++)
		{
			int a=i+j;
			int b=i-j;

			if(abs(a)<=5)
			{
				if(b<bhairs[a+5]&& b>bhaire[a+5])
					ptlist.push_back(cvPoint(i,j));
			}
		}
	}
	int nP=dest->width*dest->height;
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
		nP-=nPix;
	}

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

void Rain(IplImage* src, IplImage* dest, float rain_min_length, float rain_max_length)
{
	Vec2 rain_vec(-0.2, 1);
	for(int i=0;i<10000;i++)
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