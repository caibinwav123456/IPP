#include "StdAfx.h"
#include "FFT.H"

//FFT transform

void FFT(Complex*input,Complex*output,int s)
{
	int exp2;
	Complex* tmp=new Complex[s];
	for(exp2=0;(s>>exp2)!=1;exp2++);
	int i;
	for(i=0;i<s;i++)
		tmp[i]=input[i];
	for(int n=exp2-1;n>=0;n--)
	{
		for(int i=0;i<s;i++)
		{
			int m=(i>>n);
			int c=(m&1);
			int v=(s>>n);
			if(c==0)
			{
				int tm=0;
				for(int j=0;j<exp2-1-n;j++)
				{
					tm|=((m&(1<<(j+1)))!=0);
					tm<<=1;
				}
				int ci=(i|(1<<n));
				Complex a=tmp[ci]*Complex::Exp(PI*tm/v);
				Complex b=tmp[ci]*Complex::Exp(PI*(tm+v)/v);
				Complex c=tmp[i];
				
				tmp[i]=c+a;
				tmp[ci]=c+b;
			}
		}
	}
	for(i=0;i<s;i++)
	{
		int t=i;
		int ci=0;
		for(int j=0;j<exp2;j++)
		{
			ci=((ci<<1)|(t&1));
			t>>=1;
		}
		output[i]=tmp[ci];
	}
	delete[] tmp;
}

//2D FFT transform

void FFT2D(IplImage* input, IplImage* output)
{
	ASSERT(input!=NULL&&output!=NULL);
	ASSERT(input->width==output->width && input->height==output->height);
	ASSERT(input->depth==IPL_DEPTH_32F&&output->depth==IPL_DEPTH_32F);
	ASSERT(input->nChannels==2&&output->nChannels==2);
	
	IplImage* tmp=cvCreateImage(cvGetSize(output), IPL_DEPTH_32F, 2);
	IplImage* tmp1=cvCreateImage(cvSize(output->height,output->width),IPL_DEPTH_32F,2);
	IplImage* tmp2=cvCreateImage(cvSize(output->height,output->width),IPL_DEPTH_32F,2);
	
	int i;
	for(i=0;i<output->height;i++)
	{
		FFT((Complex*)(input->imageData+input->widthStep*i), 
			(Complex*)(tmp->imageData+tmp->widthStep*i),input->width);
	}
	cvTranspose(tmp, tmp1);
	for(i=0;i<output->width;i++)
	{
		FFT((Complex*)(tmp1->imageData+tmp1->widthStep*i),
			(Complex*)(tmp2->imageData+tmp2->widthStep*i), tmp->width);
	}
	cvTranspose(tmp2, output);

	cvReleaseImage(&tmp);
	cvReleaseImage(&tmp1);
	cvReleaseImage(&tmp2);
}

//Generation of Perlin noise with FFT algorithm

void Perlin2(IplImage* img, int nx, int ny)
{
	ASSERT(img!=NULL);
	ASSERT(img->depth==IPL_DEPTH_32F);
	ASSERT(img->nChannels==1);
	
	//srand(time(NULL));
	IplImage* grid=cvCreateImage(cvSize(nx,ny), IPL_DEPTH_32F, 1);
	IplImage* imgfft=cvCreateImage(cvGetSize(grid),IPL_DEPTH_32F, 2);
	IplImage* output=cvCreateImage(cvGetSize(grid), IPL_DEPTH_32F, 2);
	IplImage* imgout=cvCreateImage(cvGetSize(grid),IPL_DEPTH_32F, 2);
	
	cvZero(imgfft);
	
	int i;
	for(i=0;i<nx;i++)
	{
		for(int j=0;j<ny;j++)
		{
			*(float*)(grid->imageData+grid->widthStep*j+i*4)=(float)rand()/RAND_MAX;
		}
	}
	cvMerge(grid, NULL, NULL, NULL, imgfft);
	FFT2D(imgfft,imgout);
	
	for(i=0;i<nx;i++)
	{
		for(int j=0;j<ny;j++)
		{
			Complex* comp=(Complex*)(imgfft->imageData+j*imgfft->widthStep+i*4*2);
			Complex* compd=(Complex*)(imgout->imageData+((j+ny/2)%ny)*imgout->widthStep+((i+nx/2)%nx)*4*2);
			*comp=*compd;
		}
	}
	cvCopyImage(imgfft, imgout);
	cvZero(img);
	for(i=0;i<img->width/nx;i++)
	{
		for(int j=0;j<img->height/ny;j++)
		{
			cvCopyImage(imgout,imgfft);
			int k;
			for(k=0;k<nx;k++)
			{
				for(int l=0;l<ny;l++)
				{
					Complex& comp=*(Complex*)(imgfft->imageData+l*imgfft->widthStep+k*4*2);
					comp=comp*Complex::Exp(2*PI*(i*(k-nx/2))/img->width)*Complex::Exp(2*PI*(j*(l-ny/2))/img->height);
				}
			}
			FFT2D(imgfft, imgfft);
			for(k=0;k<nx;k++)
			{
				for(int l=0;l<ny;l++)
				{
					Complex* comp=(Complex*)(imgfft->imageData+l*imgfft->widthStep+k*4*2);
					*(float*)(img->imageData+(j+l*(img->height/ny))*img->widthStep+(i+k*(img->width/nx))*4)=
						(*comp*Complex::Exp(2*PI*k/2)*Complex::Exp(2*PI*l/2)).r/nx/ny;
				}
			}
		}
	}
	cvReleaseImage(&grid);
	cvReleaseImage(&imgfft);
	cvReleaseImage(&output);
	cvReleaseImage(&imgout);
}

//Wow! this function runs much faster.It uses the internal DFT function in OpenCV library and performs faster FFT computation with SSE acceleration.

void Perlin(IplImage* img, float nx, float ny)
{
	ASSERT(img!=NULL);
	ASSERT(img->depth==IPL_DEPTH_32F);
	ASSERT(img->nChannels==1);
	
	int adjx=(int)pow(2,ceil(logf(img->width)/logf(2)));
	int adjy=(int)pow(2,ceil(logf(img->height)/logf(2)));
	int nw=(int)pow(2,ceil(logf(nx)/logf(2)));
	int nh=(int)pow(2,ceil(logf(ny)/logf(2)));
	if(nw<2)nw=2;
	if(nh<2)nh=2;
	if(nw>=adjx/2)nw=adjx/2;
	if(nh>=adjy/2)nh=adjy/2;
	IplImage* noise=cvCreateImage(cvSize(adjx,adjy),IPL_DEPTH_32F,1);

	//srand(time(NULL));
	IplImage* grid=cvCreateImage(cvSize(nw,nh), IPL_DEPTH_32F, 1);
	IplImage* imgfft=cvCreateImage(cvGetSize(grid),IPL_DEPTH_32F, 2);
	IplImage* imgout=cvCreateImage(cvGetSize(noise), IPL_DEPTH_32F, 2);
	
	cvZero(imgfft);
	cvZero(imgout);
	cvZero(noise);
	
	int i;
	for(i=0;i<nw;i++)
	{
		for(int j=0;j<nh;j++)
		{
			*(float*)(grid->imageData+grid->widthStep*j+i*4)=(float)rand()/RAND_MAX;
		}
	}
	cvMerge(grid, NULL, NULL, NULL, imgfft);
	cvDFT(imgfft, imgfft, CV_DXT_FORWARD);
	//FFT2D(imgfft,imgfft);
	
	for(i=0;i<2;i++)
	{
		for(int j=0;j<2;j++)
		{
			cvSetImageROI(imgout,cvRect(i*(noise->width-nw/2),j*(noise->height-nh/2), nw/2, nh/2));
			cvSetImageROI(imgfft, cvRect(i*nw/2,j*nh/2,nw/2,nh/2));
			cvCopyImage(imgfft, imgout);
		}
	}
	cvResetImageROI(imgout);
	cvResetImageROI(imgfft);
	
	//FFT2D(imgout, imgout);
	cvDFT(imgout, imgout, CV_DXT_INVERSE);
	cvSplit(imgout, noise, NULL, NULL, NULL);
	cvCvtScale(noise, noise, 1./nw/nh);
	
	cvReleaseImage(&grid);
	cvReleaseImage(&imgfft);
	cvReleaseImage(&imgout);

	cvSetImageROI(noise, cvRect(0,0,noise->width*min(1,(float)nx/nw),noise->height*min(1,(float)ny/nh)));
	cvResize(noise, img);
	cvReleaseImage(&noise);
}

void Perlin(IplImage* img, float nx, float ny, float xy, float yx)
{
	ASSERT(img!=NULL);
	ASSERT(img->depth==IPL_DEPTH_32F);
	ASSERT(img->nChannels==1);

	int adjx=(int)pow(2,ceil(logf(img->width)/logf(2)));
	int adjy=(int)pow(2,ceil(logf(img->height)/logf(2)));
	int nw=(int)pow(2,ceil(logf(nx)/logf(2)));
	int nh=(int)pow(2,ceil(logf(ny)/logf(2)));
	if(nw<2)nw=2;
	if(nh<2)nh=2;
	if(nw>=adjx/2)nw=adjx/2;
	if(nh>=adjy/2)nh=adjy/2;

	float wh = xy;
	float hw = yx;

	IplImage* noise=cvCreateImage(cvSize(adjx,adjy),IPL_DEPTH_32F,1);

	//srand(time(NULL));
	IplImage* grid=cvCreateImage(cvSize(nw,nh), IPL_DEPTH_32F, 1);
	IplImage* imgfft=cvCreateImage(cvGetSize(grid),IPL_DEPTH_32F, 2);
	IplImage* imgout=cvCreateImage(cvGetSize(noise), IPL_DEPTH_32F, 2);

	cvZero(imgfft);
	cvZero(imgout);
	cvZero(noise);

	int i;
	for(i=0;i<nw;i++)
	{
		for(int j=0;j<nh;j++)
		{
			*(float*)(grid->imageData+grid->widthStep*j+i*4)=(float)rand()/RAND_MAX;
		}
	}
	cvMerge(grid, NULL, NULL, NULL, imgfft);
	cvDFT(imgfft, imgfft, CV_DXT_FORWARD);


	IplImage* tmp = imgfft;
	imgfft = cvCreateImage(cvGetSize(grid),IPL_DEPTH_32F, 2);
	for(i=0;i<2;i++)
	{
		for(int j=0;j<2;j++)
		{
			cvSetImageROI(tmp,cvRect((1-i)*nw/2,(1-j)*nh/2,nw/2,nh/2));
			cvSetImageROI(imgfft, cvRect(i*nw/2,j*nh/2,nw/2,nh/2));
			cvCopyImage(tmp, imgfft);
		}
	}
	cvResetImageROI(imgfft);
	cvReleaseImage(&tmp);

	CvMat* mat = NULL;
	for(int i=0;i<4;i++)
	{
		mat = cvCreateMat(2, 3, CV_32FC1);
		cvZero(mat);
		mat->data.fl[0] = 1;
		mat->data.fl[1] = wh;
		mat->data.fl[3] = hw;
		mat->data.fl[4] = 1;
		mat->data.fl[2] = (i/2)*(noise->width)-(1+wh)/2*nw;
		mat->data.fl[5] = (i%2)*(noise->height)-(1+hw)/2*nh;

		cvWarpAffine(imgfft, imgout, mat, CV_INTER_NN);
		cvReleaseMat(&mat);
	}

	cvDFT(imgout, imgout, CV_DXT_INVERSE);
	cvSplit(imgout, noise, NULL, NULL, NULL);
	cvCvtScale(noise, noise, 1./(nw*nh*(1-wh*hw)));

	cvReleaseImage(&grid);
	cvReleaseImage(&imgfft);
	cvReleaseImage(&imgout);

	cvSetImageROI(noise, cvRect(0,0,noise->width*min(1,(float)nx/nw),noise->height*min(1,(float)ny/nh)));
	cvResize(noise, img);
	cvReleaseImage(&noise);
}