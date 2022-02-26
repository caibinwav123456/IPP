#include "stdafx.h"
#include "ProcessImage.h"
#include "Image.h"
#include "matutil.h"
#include "Propagate.h"

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

#define USE_STATIC_TABLE 1
#define NOISE_THRESH     8
#define NOISE_THRESH2    20
#define BIG_COMP_THRESH  40
#define CONN_RADIUS      10

struct FFTBuffer
{
	IplImage*blk;
	IplImage*prefft;
	IplImage*fft;
	IplImage*fft2;
	void Init(int w,int h)
	{
		blk = cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,1);
		prefft = cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,2);
		fft = cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,2);
		fft2 = cvCreateImage(cvSize(w,h),IPL_DEPTH_32F,2);
	}
	void Release()
	{
		cvReleaseImage(&blk);
		cvReleaseImage(&prefft);
		cvReleaseImage(&fft);
		cvReleaseImage(&fft2);
	}
};

struct ProgData
{
	int nblock;
	int nblock_done;
	int npass;
	int ipass;
};

struct StepParam
{
	int cellx;
	int celly;
	int blkx;
	int blky;
	float pitchx;
	float pitchy;
	int offx;
	int offy;
	IplImage*blk;
	IplImage*prefft;
	IplImage*fft;
	IplImage*fft2;
	IplImage*dest;
	IplImage*rTable;
	IplImage*CosineTable;
	Vec4* params;
	double (*paramext)[6];
	float overlap;
	float rv;
	float hi;
	float lo;
	ProgData* prog;
};
#ifdef STATIC_LIB
namespace Pre_process {
#endif
static const float sInhibit=0.35;
struct ThreadData;
struct InputParam;
const CvPoint shift[8]={{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}};
typedef void (*ProcessStep)(IplImage* image, int i, int j, StepParam* ptr, CvRect rc, ThreadData* th);
void ProcessTotal(IplImage* src,IplImage* dest, int seg, float overlap, InputParam* input, Preprocess_Callback* callback, bool singlepass);
void PerformStep(ProcessStep step, IplImage* image, StepParam* ptr, int start, int end, ThreadData* th);
void process_step1(IplImage* image, int i, int j, StepParam* ptr, CvRect rc, ThreadData* th);
void process_step2(IplImage* image, int i, int j, StepParam* ptr, CvRect rc, ThreadData* th);
void ComputeNormal(IplImage* image, double(*comp)[6]);
void SmoothEstimation(int cellx, int celly, double (*paramext)[6]);
void AdjustImage(IplImage* src,IplImage* dest, double(*Comp)[6]);
void Threshold(IplImage* src, IplImage* dest);
void Denoise(IplImage* src, IplImage* dest);
void Erosion(IplImage* src, IplImage* dest);
void Dilation(IplImage* src, IplImage* dest);
void Close(IplImage* src, IplImage* dest);
unsigned int ThreadFunc(void* param);
inline void ThreadStart(Preprocess_Callback* callback, IplImage* src, StepParam* param, ProcessStep step, vector<void*>& v);
IplImage* CreateRTable(CvSize size);
IplImage* CreateCosineTable(int dim);
void SupressHighFreq(IplImage* img, float hi, float lo, IplImage* rt, IplImage* ct);

struct ThreadData
{
	StepParam* param;
	IplImage* src;
	int index;
	ProcessStep step;
	Preprocess_Callback* callback;
	int lockid;
	int lockid2;
	int lockid_s;
	int lockid_e;
};

struct InputParam
{
	float inhibitL;
	float inhibitH;
};

unsigned int ThreadFunc(void* pparam)
{
	ThreadData* param=(ThreadData*)pparam;
	int index=param->index;
	StepParam* sparam=param->param;
	int sep[5];
	int core=param->callback->core;
	for(int i=0;i<=core;i++)
		sep[i]=sparam->cellx*i/core;
	int start=sep[index],end=sep[index+1];
	param->lockid_s=(index==0?-1:index);
	param->lockid_e=(index==core-1?-1:index+1);
	PerformStep(param->step, param->src, param->param, start, end, param);
	param->callback->signal(param->index);
	delete param;
	return 0;
}

inline void ThreadStart(Preprocess_Callback* callback, IplImage* src, StepParam* param, ProcessStep step, vector<void*>& v)
{
	ThreadData* th[4];
	int ncore;
	if(callback->core == 2)
		ncore=2;
	else if(callback->core == 4)
		ncore=4;
	else
		return;
	for(int i=0;i<ncore;i++)
	{
		th[i]=new ThreadData;
		th[i]->index=i;
		th[i]->param=param+i;
		th[i]->src=src;
		th[i]->step=step;
		th[i]->callback=callback;
		v.push_back(callback->threadstart(ThreadFunc, th[i]));
	}
}
//#define OVERLAP_RATIO 0.5
void ProcessTotal(IplImage* src,IplImage* dest, int seg, float overlap, InputParam* input, Preprocess_Callback* callback, bool singlepass)
{
	float r=overlap/(1-overlap);
	int min_blk_size=64;
	//int seg=19;
	int blk_size=MIN(src->width,src->height)/(seg+r);
	if(blk_size<min_blk_size)blk_size=min_blk_size;
	int segx=MAX(1,src->width/blk_size-r);
	int segy=MAX(1,src->height/blk_size-r);
	int blkx=src->width/((1-overlap)*segx+overlap)+1;
	int blky=src->height/((1-overlap)*segy+overlap)+1;
	if(blkx>src->width)blkx=src->width;
	if(blky>src->height)blky=src->height;
	int cellx=segx;
	int celly=segy;
	float pitchx=src->width/(segx+r);
	float pitchy=src->height/(segy+r);
	IplImage* gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	cvCvtColor(src,gray, CV_RGB2GRAY);
	IplImage* grayf = cvCreateImage(cvGetSize(gray), IPL_DEPTH_32F, 1);
	cvCvtScale(gray,grayf);
	cvReleaseImage(&gray);
	IplImage* blk = NULL;
	IplImage* prefft = NULL;
	IplImage* fft = NULL;
	IplImage* fft2 = NULL;
	IplImage* rTable=NULL;
	IplImage* CosTable=NULL;
	if(input&&!(input->inhibitH==0&&input->inhibitL==0))
	{
		rTable=CreateRTable(cvSize(100,100));
		CosTable=CreateCosineTable(100);
	}
	FFTBuffer grpfft[4];
	if(!callback)
	{
		blk = cvCreateImage(cvSize(blkx,blky),IPL_DEPTH_32F,1);
		prefft = cvCreateImage(cvSize(blkx,blky),IPL_DEPTH_32F,2);
		fft = cvCreateImage(cvSize(blkx,blky),IPL_DEPTH_32F,2);
		fft2 = cvCreateImage(cvSize(blkx,blky),IPL_DEPTH_32F,2);
		cvZero(prefft);
	}
	else
	{
		for(int i=0;i<callback->core;i++)
		{
			grpfft[i].Init(blkx,blky);
			cvZero(grpfft[i].prefft);
		}
	}
	Vec4* params = new Vec4[cellx*celly];
	double (*paramext)[6] = new double[cellx*celly][6];
	IplImage* destf=cvCreateImage(cvGetSize(dest), IPL_DEPTH_32F, 1);
	StepParam sparam,sparam2[4];
	ProgData prog;
	prog.nblock=cellx*celly;
	prog.nblock_done=0;
	prog.npass=(singlepass?1:2);
	prog.ipass=0;
	sparam.pitchx=pitchx;
	sparam.pitchy=pitchy;
	sparam.cellx=cellx;
	sparam.celly=celly;
	sparam.blkx=blkx;
	sparam.blky=blky;
	sparam.blk=blk;
	sparam.prefft=prefft;
	sparam.fft=fft;
	sparam.fft2=fft2;
	sparam.dest=destf;
	sparam.params=params;
	sparam.paramext=paramext;
	sparam.overlap=overlap;
	sparam.rv=1.0f/overlap;
	sparam.rTable=rTable;
	sparam.CosineTable=CosTable;
	sparam.hi=(input?input->inhibitH:0);
	sparam.lo=(input?input->inhibitL:0);
	sparam.prog=&prog;
	if(callback)
	{
		for(int i=0;i<callback->core;i++)
		{
			sparam2[i]=sparam;
			sparam2[i].blk=grpfft[i].blk;
			sparam2[i].prefft=grpfft[i].prefft;
			sparam2[i].fft=grpfft[i].fft;
			sparam2[i].fft2=grpfft[i].fft2;
		}
	}
	cvZero(destf);
	if(!callback)
		PerformStep(&process_step1,grayf,&sparam,0,sparam.cellx,NULL);
	else
	{
		vector<void*> v;
		ThreadStart(callback, grayf, sparam2, process_step1, v);
		callback->wait(v);
	}
	prog.ipass=1;
    if(!singlepass)
	{
		SmoothEstimation(cellx, celly, paramext);
		if(!callback)
			PerformStep(&process_step2,grayf,&sparam,0,sparam.cellx,NULL);
		else
		{
			vector<void*> v;
			ThreadStart(callback, grayf, sparam2, process_step2, v);
			callback->wait(v);
		}
	}
	for(int i=0;i<dest->width;i++)
	{
		for(int j=0;j<dest->height;j++)
		{
			uchar* pix=PTR_PIX(*dest,i,j);
			float* pixs=(float*)PTR_PIX(*destf,i,j);
			pix[0]=MAX(0,MIN(255,*pixs));
		}
	}
	//Clean up
	cvReleaseImage(&destf);
	cvReleaseImage(&grayf); 
	cvReleaseImage(&blk); 
	cvReleaseImage(&prefft);
	cvReleaseImage(&fft);
	cvReleaseImage(&fft2);
#if USE_STATIC_TABLE
	cvReleaseImageHeader(&rTable);
	cvReleaseImageHeader(&CosTable);
#else
	cvReleaseImage(&rTable);
	cvReleaseImage(&CosTable);
#endif
	if(callback)
	{
		for(int i=0;i<callback->core;i++)
			grpfft[i].Release();
	}
	delete[] params;
	delete[] paramext;
}

void PerformStep(ProcessStep step, IplImage* image, StepParam* ptr, int start, int end, ThreadData* th)
{
	Preprocess_Callback* callback=NULL;
	if(th)
		callback=th->callback;
	for(int i=start;i<end;i++)
	{
		for(int j=0;j<ptr->celly;j++)
		{
			CvRect rc = cvRect(i*ptr->pitchx,j*ptr->pitchy,ptr->blkx,ptr->blky);
			ptr->offx=ptr->offy=0;
			if(rc.x+rc.width>image->width)
			{
				ptr->offx=rc.x+rc.width-image->width;
				rc.x=image->width-rc.width;
			}
			if(rc.y+rc.height>image->height)
			{
				ptr->offy=rc.y+rc.height-image->height;
				rc.y=image->height-rc.height;
			}
			if(callback)
				callback->lock(0);
			cvSetImageROI(image, rc);
			cvCopyImage(image, ptr->blk);
			if(callback)
				callback->unlock(0);
			cvMerge(ptr->blk, NULL, NULL, NULL, ptr->prefft);
			cvDFT(ptr->prefft, ptr->fft, CV_DXT_FORWARD);
			if(th)
			{
				th->lockid2=-1;
				if(th->lockid_s>0&&i==start&&th->lockid_e>0&&i==end-1&&!callback->bsinglelock)
				{
					th->lockid=th->lockid_s;
					th->lockid2=th->lockid_e;
				}
				else if(th->lockid_s>0&&i==start)
					th->lockid=th->lockid_s;
				else if(th->lockid_e>0&&i==end-1)
					th->lockid=th->lockid_e;
				else
					th->lockid=-1;
			}
			step(image,i,j,ptr,rc, th);
			if(callback&&callback->prog)
			{
				callback->prog->interlock_inc(&ptr->prog->nblock_done);
				callback->prog->prog(((float)ptr->prog->nblock_done/ptr->prog->nblock+ptr->prog->ipass)*100/ptr->prog->npass);
			}
		}
	}
}

void process_step1(IplImage* image, int i, int j, StepParam* ptr, CvRect rc, ThreadData* th)
{
	ComputeNormal(ptr->fft,ptr->paramext+i*ptr->celly+j);
    if(ptr->prog->npass==1)
        process_step2(image, i, j, ptr, rc, th);
}

void process_step2(IplImage* image, int i, int j, StepParam* ptr, CvRect rc, ThreadData* th)
{
	AdjustImage(ptr->fft,ptr->fft2,ptr->paramext+i*ptr->celly+j);
	if(ptr->hi!=0.0f||ptr->lo!=0.0f)
		SupressHighFreq(ptr->fft2, ptr->hi, ptr->lo, ptr->rTable, ptr->CosineTable);
	cvDFT(ptr->fft2, ptr->fft, CV_DXT_INV_SCALE);
	float fxl=(i==0?1:0);
	float fxr=(i==ptr->cellx-1?1:0);
	float fyl=(j==0?1:0);
	float fyr=(j==ptr->celly-1?1:0);
	float rv=ptr->rv;
	if(th)
	{
		if(th->lockid>0)
			th->callback->lock(th->lockid);
		if(th->lockid2>0)
			th->callback->lock(th->lockid2);
	}
	for(int k=0;k<ptr->blkx;k++)
	for(int l=0;l<ptr->blky;l++)
	{
		int x=k+rc.x;
		int y=l+rc.y;
		float fx,fy;
		float factorx=(float)MAX(k-ptr->offx,0)/ptr->blkx;
		float factory=(float)MAX(l-ptr->offy,0)/ptr->blky;
		if(factorx<ptr->overlap)
			fx=(1-factorx*rv)*fxl+factorx*rv;
		else if(factorx>1-ptr->overlap)
			fx=(rv-factorx*rv)+(factorx*rv-rv+1)*fxr;
		else
			fx=1;
		if(factory<ptr->overlap)
			fy=(1-factory*rv)*fyl+factory*rv;
		else if(factory>1-ptr->overlap)
			fy=(rv-factory*rv)+(factory*rv-rv+1)*fyr;
		else
			fy=1;
		float f = fx*fy;
		float* pix = (float*)PTR_PIX(*ptr->fft, k, l);
		float* pixd = (float*)PTR_PIX(*ptr->dest, x, y);
		int dc=*pix*f;
		*pixd+=dc;
	}
	if(th)
	{
		if(th->lockid>0)
			th->callback->unlock(th->lockid);
		if(th->lockid2>0)
			th->callback->unlock(th->lockid2);
	}
}

IplImage* ComputeAmp(IplImage* image, IplImage** ripple)
{
	IplImage* amp2 = cvCreateImage(cvSize(image->width+10,image->height+10),IPL_DEPTH_32F,1);
	for(int i=0;i<amp2->width;i++)
	{
		for(int j=0;j<amp2->height;j++)
		{
			int x=(i-5+image->width)%image->width;
			int y=(j-5+image->height)%image->height;
			float* pix = (float*)PTR_PIX(*image, x, y);
			float* pixd = (float*)PTR_PIX(*amp2, i, j);
			*pixd = sqrt((pow(pix[0],2)+pow(pix[1],2))/2);
		}
	}
	IplImage* integ = cvCreateImage(cvSize(amp2->width+1,amp2->height+1),IPL_DEPTH_64F,1);
	cvIntegral(amp2,integ);
	cvReleaseImage(&amp2);

	IplImage* amp = cvCreateImage(cvGetSize(image), IPL_DEPTH_64F, 1);
	if(ripple)
		*ripple=cvCreateImage(cvGetSize(image), IPL_DEPTH_64F, 1);
	for(int x=0;x<image->width;x++)
	for(int y=0;y<image->height;y++)
	{
		double ac=0,ac2=0;
		int gaugex=MIN(20,MIN(x,image->width-x)*0.25);
		int gaugey=MIN(20,MIN(y,image->height-y)*0.25);
		double* tl=(double*)PTR_PIX(*integ,x,y);
		double* tr=(double*)PTR_PIX(*integ,x+11,y);
		double* bl=(double*)PTR_PIX(*integ,x,y+11);
		double* br=(double*)PTR_PIX(*integ,x+11,y+11);
		double* tl2=(double*)PTR_PIX(*integ,x-gaugex,y-gaugey);
		double* tr2=(double*)PTR_PIX(*integ,x+gaugex+11,y-gaugey);
		double* bl2=(double*)PTR_PIX(*integ,x-gaugex,y+gaugey+11);
		double* br2=(double*)PTR_PIX(*integ,x+gaugex+11,y+gaugey+11);
		ac=*br-*bl-*tr+*tl;
		ac2=*br2-*bl2-*tr2+*tl2;
		ac/=11*11;
		ac2/=(11+2*gaugex)*(11+2*gaugey);
		*(double*)PTR_PIX(*amp, x, y) = ac;
		if(ripple)
			*(double*)PTR_PIX(**ripple, x, y)=ac2/ac;
	}
	cvReleaseImage(&integ);
	return amp;
}

void ComputeNormal(IplImage* image, double(*comp)[6])
{
	IplImage* amp=ComputeAmp(image, NULL);
	int mincx = 0/*image->width/2/10*/,mincy = 0/*image->height/2/10*/;
	double coeff[6][6];
	double coeff2[6];
	ZeroMemory(coeff,sizeof(coeff));
	ZeroMemory(coeff2,sizeof(coeff2));
	for(int i=0;i<image->width/2;i++)
	{
		if(i==0)
			continue;
		for(int j=0;j<image->height/2;j++)
		{
			if(i<mincx&&j<mincy)
				continue;
			if(j==0)
				continue;
			//The i2,j2 and i,j region and the i,j2 and i2,j region are central symmetric, so ignore half.
			int i2=image->width-i;
			Vec2 v(i,j);
			float l=v.length();
			Vec2 vn=v.normalize();
			float co[7],co2[7];
			co[0]=co2[0]=log(l);
			co[1]=co2[1]=powf(vn.x,2)-powf(vn.y,2);
			co[2]=vn.x*vn.y;
			co2[2]=-co[2];
			co[3]=co2[3]=logf(i);
			co[4]=co2[4]=logf(j);
			co[5]=co2[5]=1.0f;
			co[6]=log(*(double*)PTR_PIX(*amp, i, j));
			co2[6]=log(*(double*)PTR_PIX(*amp, i2, j));
			if(*(double*)PTR_PIX(*amp, i, j)<=0.0
				||*(double*)PTR_PIX(*amp, i2, j)<=0.0)
				continue;
			for(int k=0;k<6;k++)
			for(int l=k;l<6;l++)
			{
				coeff[k][l]+=co[k]*co[l]+co2[k]*co2[l];
			}
			for(int k=0;k<6;k++)
			{
				coeff2[k]+=co[6]*co[k]+co2[6]*co2[k];
			}
		}
	}
	cvReleaseImage(&amp);
	int n=6;
	IplImage* mat = cvCreateImage(cvSize(n,n),IPL_DEPTH_64F,1);
	for(int i=0;i<n;i++)
	for(int j=0;j<n;j++)
	{
		*(double*)PTR_PIX(*mat,i,j)=coeff[MIN(i,j)][MAX(i,j)];
	}
	IplImage* invmat = cvCreateImage(cvSize(n,n),IPL_DEPTH_64F,1);
	cvInvert(mat,invmat,CV_SVD_SYM);
	double co[6];
	ZeroMemory(co,sizeof(co));
	for(int i=0;i<n;i++)
	for(int j=0;j<n;j++)
		co[i]+=*(double*)PTR_PIX(*invmat,i,j)*coeff2[j];
	cvReleaseImage(&mat);
	cvReleaseImage(&invmat);
	(*comp)[0]=co[0];
	double ea=tanh(co[1]*2/co[0]);
	(*comp)[1]=1+ea;
	(*comp)[2]=tanh(co[2]*2/co[0]);
	(*comp)[3]=1-ea;
	(*comp)[4]=co[3];
	(*comp)[5]=co[4];
}

const int kIteration=3;
void SmoothEstimation(int cellx, int celly, double (*paramext)[6])
{
	CvPoint pad[4]={cvPoint(1,0),cvPoint(0,1),cvPoint(-1,0),cvPoint(0,-1)};
	for(int n=0;n<kIteration;n++)
	for(int i=1;i<cellx-1;i++)
	{
		for(int j=1;j<celly-1;j++)
		{
			for(int k=0;k<6;k++)
			{
				double t=0;
				for(int l=0;l<4;l++)
				{
					t+=paramext[(i+pad[l].x)*celly+(j+pad[l].y)][k];
				}
				t=t/4-paramext[i*celly+j][k];
				paramext[i*celly+j][k]+=t*0.5;
			}
		}
	}
}

void AdjustImage(IplImage* src,IplImage* dest, double(*Comp)[6])
{
	Vec3 lvl(-0.45f,0.f,0.f);
	float max_adj=0.2f;
	float max_adj2=0.4f;
	float max_adj3=0.5f;
	float slopelvl=-0.45f;
	lvl = Vec3(-0.35f,1.0f,0.0f);
	double diffx=0;
	double diffy=0;
	double diffz=0;
	double diffk=0;
	double slopex=0;
	double slopey=0;
	double lvla,lvlb,lvlc,llvl;
	Vec4 comp;
	comp.x=(*Comp)[0];
	comp.y=(*Comp)[1];
	comp.z=(*Comp)[2];
	comp.t=(*Comp)[3];
	slopex=(*Comp)[4];
	slopey=(*Comp)[5];
	slopex=MAX(0,MIN(max_adj,slopelvl-slopex));
	slopey=MAX(0,MIN(max_adj,slopelvl-slopey));
	diffk = MAX(0,MIN(max_adj3,lvl.x-comp.x));
	diffx = MAX(-max_adj2,MIN(max_adj2,lvl.y-comp.y));
	diffy = MAX(-max_adj2,MIN(max_adj2,lvl.z-comp.z));
	diffz = MAX(-max_adj2,MIN(max_adj2,lvl.y-comp.t));
	lvla=lvl.y-diffx;
	lvlb=-diffy;
	lvlc=lvl.y-diffz;
	double thresh=0.9;
	if(lvlb*lvlb>4*lvla*lvlc*thresh*thresh)
	{
		lvlb=(lvlb>=0?1:-1)*sqrt(lvla*lvlc)*2*thresh;
	}
	llvl=(-sqrtf(powf(lvla-lvlc,2)+powf(lvlb,2))+lvla+lvlc)/2;
	//llvl*=3;
	int hw=(src->width+2)/2;
	int hh=(src->height+2)/2;
	for(int i=0;i<hw;i++)
	{
		for(int j=0;j<hh;j++)
		{
			double f,f2;
			f=powf(i*i*lvla+i*j*lvlb+j*j*lvlc,diffk/2)/llvl;
			f2=powf(i*i*lvla-i*j*lvlb+j*j*lvlc,diffk/2)/llvl;
			float f3=MAX(1,powf(i,slopex))*MAX(1,powf(j,slopey));
			f*=f3,f2*=f3;
			if(i==0&&j==0)
				f=f2=1;
			int x[2]={i,src->width-i};
			int y[2]={j,src->height-j};
			for(int k=0;k<2;k++)
			for(int l=0;l<2;l++)
			{
				if(x[k]==src->width||y[l]==src->height)
					continue;
				float* pixs = (float*)PTR_PIX(*src,x[k],y[l]);
				float* pix = (float*)PTR_PIX(*dest,x[k],y[l]);
				if((k+l)%2==0)
				{
					pix[0] = pixs[0]*f;
					pix[1] = pixs[1]*f;
				}
				else
				{
					pix[0] = pixs[0]*f2;
					pix[1] = pixs[1]*f2;
				}
			}
		}
	}
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

void Threshold(IplImage* src, IplImage* dest)
{
	IplImage* gray=src;
	CvScalar avg,sddv;
	cvAvgSdv(gray, &avg,&sddv);
	float step_thresh = 0.25f/0.46f*sddv.val[0]/avg.val[0];
	step_thresh = MIN(0.3,step_thresh);

	IplImage* integ = cvCreateImage(cvSize(src->width+1, src->height+1), IPL_DEPTH_64F, 1);
	cvIntegral(gray, integ);
	int span = 20, span2 = 25;
	float scale = 2.2f;
	for(int i=0;i<src->height;i++)
	{
		for(int j=0;j<src->width;j++)
		{
			int i0 = MAX(0,i-span),i1 = MIN(src->height,i+span+1);
			int j0 = MAX(0,j-span),j1 = MIN(src->width,j+span+1);
			int i02 = MAX(0,i-span2),i12 = MIN(src->height,i+span2+1);
			int j02 = MAX(0,j-span2),j12 = MIN(src->width,j+span2+1);

			int cnt = (i1-i0)*(j1-j0);
			int cnt2 = (i12-i02)*(j12-j02);

			double sump = *(double*)PTR_PIX(*integ, j1, i1)
				-*(double*)PTR_PIX(*integ, j0, i1)
				-*(double*)PTR_PIX(*integ, j1, i0)
				+*(double*)PTR_PIX(*integ, j0, i0);
			double sump2 =  *(double*)PTR_PIX(*integ, j12, i12)
				-*(double*)PTR_PIX(*integ, j02, i12)
				-*(double*)PTR_PIX(*integ, j12, i02)
				+*(double*)PTR_PIX(*integ, j02, i02);
			float mean = sump/cnt;
			float mean2 = sump2/cnt2;
			float l1 = mean;
			float l2 = *PTR_PIX(*gray, j, i);

			float diff = l2-l1;
			float level = 250;
			if(mean2<10)mean2=10;
			diff/=mean2/125;
			float re = 255*SmoothStep(level+diff*scale,160,60)*SmoothStep(l2,avg.val[0]*0.5,10);

			uchar* ptr2 = PTR_PIX(*dest, j, i);
			*ptr2 = MAX(0,MIN(re, 255));
		}
	}

	cvReleaseImage(&integ);
}

struct FillParam
{
	IplImage* src;
	IplImage* mask;
	CPropagate* prop;
	bool bw;
};

int wthresh=200;
int bthresh=100;

bool FillFunc(void* param, CvPoint pt, CvPoint ptorg, IplImage* pMask, UINT8 flags)
{
	FillParam* fparam=(FillParam*)param;
	if(fparam->prop->m_ptacc==0)
	{
		*PTR_PIX(*fparam->mask,pt.x,pt.y)=255;
		return true;
	}
	else
	{
		if(*PTR_PIX(*fparam->mask,pt.x,pt.y)!=0)
			return false;
		if(fparam->bw)
		{
			if(*PTR_PIX(*fparam->src,pt.x,pt.y)<wthresh)
				return false;
		}
		else
		{
			if(*PTR_PIX(*fparam->src,pt.x,pt.y)>bthresh)
				return false;
		}
	}
	*PTR_PIX(*fparam->mask,pt.x,pt.y)=255;
	return true;
}

void Denoise(IplImage* src, IplImage* dest)
{
	cvCopyImage(src,dest);
	CPropagate prop;
	prop.Init(src->width,src->height,false);
	prop.SetFunc(FillFunc);
	IplImage* mask = cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	cvZero(mask);
	FillParam param;
	param.src=src;
	param.mask=mask;
	param.prop=&prop;
	bool bFillHoles=false;
	CvPoint vecConn[(2*CONN_RADIUS+1)*(2*CONN_RADIUS+1)];
	int nconn=0;
	int last=CONN_RADIUS;
	for(int i=0;i<=CONN_RADIUS;i++)
	{
		int r=cvRound(sqrtf(CONN_RADIUS*CONN_RADIUS-i*i));
		for(int j=r;j<=last;j++)
		{
			vecConn[nconn++]=cvPoint(-j,-i);
			if(j!=0)
				vecConn[nconn++]=cvPoint(j,-i);
			if(i!=0)
			{
				vecConn[nconn++]=cvPoint(-j,i);
				if(j!=0)
					vecConn[nconn++]=cvPoint(j,i);
			}
		}
		last=r;
	}

	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			if(*PTR_PIX(*mask,j,i)!=0)
				continue;
			param.bw=false;
			prop.SetPoint(cvPoint(j,i));
			while(prop.Propagate(PROP_MODE_RECT, false,&param));
			if(prop.m_ptacc>=BIG_COMP_THRESH)
			{
				for(int k=0;k<prop.m_ptacc;k++)
				{
					CvPoint pt=prop.m_bufacc[k];
					uchar* pix=PTR_PIX(*mask, pt.x, pt.y);
					pix[0]=127;
				}
			}
		}
	}
	for(int i=0;i<mask->height;i++)
	for(int j=0;j<mask->width;j++)
	{
		if(*PTR_PIX(*mask, j, i)!=127)
			*PTR_PIX(*mask, j, i)=0;
	}
	for(int i=0;i<dest->height;i++)
	{
		for(int j=0;j<dest->width;j++)
		{
			if(*PTR_PIX(*mask,j,i)!=0)
				continue;
			bool bw;
			if(*PTR_PIX(*src,j,i)>wthresh && bFillHoles)
				bw=true;
			else if(*PTR_PIX(*src,j,i)<bthresh)
				bw=false;
			else
				continue;
			param.bw=bw;
			prop.SetPoint(cvPoint(j,i));
			while(prop.Propagate(PROP_MODE_RECT, false,&param));
			if(prop.m_ptacc<NOISE_THRESH2)
			{
				//Test Component
				bool bconn=false;
				for(int k=0;k<prop.m_ptacc;k++)
				{
					CvPoint pt=prop.m_bufacc[k];
					for(int l=0;l<nconn;l++)
					{
						CvPoint pt2={pt.x+vecConn[l].x,pt.y+vecConn[l].y};
						if(pt2.x>=0&&pt2.x<mask->width&&pt2.y>=0&&pt2.y<mask->height&&*PTR_PIX(*mask,pt2.x,pt2.y)==127)
						{
							bconn=true;
							break;
						}
					}
				}
				if(!bconn||prop.m_ptacc<NOISE_THRESH)
				{
					for(int k=0;k<prop.m_ptacc;k++)
					{
						CvPoint pt=prop.m_bufacc[k];
						uchar* pix=PTR_PIX(*dest, pt.x, pt.y);
						pix[0]=(bw?0:255);
					}
				}
			}
		}
	}
	cvReleaseImage(&mask);
}

void Close(IplImage* src, IplImage* dest)
{
	IplImage* tmp=cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
	Dilation(src, tmp);
	Erosion(tmp, dest);
	cvReleaseImage(&tmp);
}

void Erosion(IplImage* src, IplImage* dest)
{
	cvCopyImage(src, dest);
	for(int i=0;i<src->height;i++)
	for(int j=0;j<src->width;j++)
	{
		if(*PTR_PIX(*src, j, i)>wthresh)
		{
			for(int k=0;k<8;k++)
			{
				int x=j+shift[k].x;
				int y=i+shift[k].y;
				if(x>=0&&x<src->width&&y>=0&&y<src->height)
				{
					*PTR_PIX(*dest, x, y)=255;
				}
			}
		}
	}
}

void Dilation(IplImage* src, IplImage* dest)
{
	cvCopyImage(src, dest);
	for(int i=0;i<src->height;i++)
	for(int j=0;j<src->width;j++)
	{
		if(*PTR_PIX(*src, j, i)<bthresh)
		{
			for(int k=0;k<8;k++)
			{
				int x=j+shift[k].x;
				int y=i+shift[k].y;
				if(x>=0&&x<src->width&&y>=0&&y<src->height)
				{
					*PTR_PIX(*dest, x, y)=0;
				}
			}
		}
	}
}

IplImage* CreateRTable(CvSize size)
{
#if USE_STATIC_TABLE
	const int kDim=100;
	static float _table[kDim*kDim];
	static bool binit=true;
	IplImage* rTable=cvCreateImageHeader(cvSize(kDim,kDim),IPL_DEPTH_32F,1);
	cvSetData(rTable, _table, kDim*sizeof(float));
#else
	IplImage* rTable=cvCreateImage(size,IPL_DEPTH_32F,1);
#endif
#if USE_STATIC_TABLE
	if(binit)
	{
		binit=false;
#endif
	for(int i=0;i<rTable->width;i++)
	{
		for(int j=0;j<rTable->height;j++)
		{
			float fx=(float)i/rTable->width;
			float fy=(float)j/rTable->height;
			float r=sqrtf(fx*fx+fy*fy);
			*(float*)PTR_PIX(*rTable, i, j)=r;
		}
	}
#if USE_STATIC_TABLE
	}
#endif
	return rTable;
}

IplImage* CreateCosineTable(int dim)
{
#if USE_STATIC_TABLE
	const int kDim=100;
	static float _table[kDim];
	static bool binit=true;
	IplImage* CosineTable=cvCreateImageHeader(cvSize(kDim,1),IPL_DEPTH_32F,1);
	cvSetData(CosineTable, _table, kDim*sizeof(float));
#else
	IplImage* CosineTable=cvCreateImage(cvSize(dim,1),IPL_DEPTH_32F,1);
#endif
#if USE_STATIC_TABLE
	if(binit)
	{
		binit=false;
#endif
	for(int i=0;i<CosineTable->width;i++)
	{
		float x=(float)i/CosineTable->width;
		float cosine=(cosf(x*CV_PI)+1)/2;
		*(float*)PTR_PIX(*CosineTable, i, 0)=cosine;
	}
#if USE_STATIC_TABLE
	}
#endif
	return CosineTable;
}

void SupressHighFreq(IplImage* src, float hi, float lo, IplImage* rt, IplImage* ct)
{
	float sqrt2=sqrtf(2.0);
	float inhibitup=hi*sqrt2;
	float inhibitdown=lo*sqrt2;
	int hw=(src->width+2)/2;
	int hh=(src->height+2)/2;
	for(int i=0;i<hw;i++)
	for(int j=0;j<hh;j++)
	{
		float fx=(float)i/hw;
		float fy=(float)j/hh;
		int sx=rt->width*fx;
		int sy=rt->height*fy;
		sx=max(0,min(rt->width-1, sx));
		sy=max(0,min(rt->height-1, sy));
		float r=*(float*)PTR_PIX(*rt, sx, sy);
		float factor=1;
		if(r>sqrt2-inhibitup)
		{
			factor=(inhibitup==inhibitdown?0:(r-sqrt2+inhibitup)/(inhibitup-inhibitdown));
			if(factor>1)
				factor=0;
			else
			{
				int sf=factor*ct->width;
				sf=max(0,min(ct->width-1,sf));
				factor=*(float*)PTR_PIX(*ct, sf, 0);
			}
		}
		int x[2]={i,src->width-i};
		int y[2]={j,src->height-j};
		for(int k=0;k<2;k++)
		for(int l=0;l<2;l++)
		{
			if(x[k]==src->width||y[l]==src->height)
				continue;
			float* pix = (float*)PTR_PIX(*src,x[k],y[l]);
			pix[0]*=factor;
			pix[1]*=factor;
		}
	}
}

#ifdef STATIC_LIB
}//namespace Pre_process
#endif

#ifdef STATIC_LIB
using namespace Pre_process;
#endif

PREPROCESS_API bool Preprocess(char* filepath, char* outpath, int seg, float overlap, Preprocess_Callback* callback, int flags)
{
	IplImage* image = cvLoadImage(filepath);
	if(!image)
		return false;
	IplImage* dest = cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,1);
	InputParam input;
	input.inhibitL=0.0f;
	input.inhibitH=0.7f;
	if(!(flags&PRE_NO_ADJUST))
		ProcessTotal(image, dest, seg, overlap, &input, callback, !!(flags&PRE_ADJ_SINGLEPASS));
	else
		cvCvtColor(image, dest, CV_RGB2GRAY);
	swap(image,dest);
	cvReleaseImage(&dest);
	dest=cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,1);
	Threshold(image, dest);
	swap(image,dest);
	if(!(flags&PRE_NO_DENOISE))
	{
		Denoise(image, dest);
		swap(image, dest);
	}
	if(!(flags&PRE_NO_CLOSE))
	{
		Close(image, dest);
		swap(image, dest);
	}
	cvReleaseImage(&dest);
	dest=cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,3);
	cvCvtColor(image,dest,CV_GRAY2RGB);
	cvReleaseImage(&image);
	if(outpath==NULL)
		outpath=filepath;
	if(!cvSaveImage(outpath,dest))
	{
		cvReleaseImage(&dest);
		return false;
	}
	cvReleaseImage(&dest);

	return true;
}
