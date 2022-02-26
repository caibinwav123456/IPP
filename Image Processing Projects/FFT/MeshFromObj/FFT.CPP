#include "stdafx.h"
#include "fft.h"
#include <windows.h>
#define PI 3.1415926535897932384626
#define BOX  char car[100];
#define BOX1 MessageBox(0,car,"jmhgmj",MB_OK);
#define BOX2 MessageBox(0,"fgbsdg","jmhgmj",MB_OK);
int tspan=8192;
void fft(Complex*in,Complex**out,int s);

void FFT2(Complex*input,Complex*output,int s)
{
	Complex*in=new Complex[s];
	int i;
	for(i=0;i<s;i++)in[i]=input[i];
	Complex*out;
	fft(in,&out,s);
	for(i=0;i<s;i++)output[i]=out[i];
	delete[] out;
}

void fft(Complex*in,Complex**out,int s)
{char car[50];
//MessageBox(0,"hg","njgnj",MB_OK);
	if(s>1){
	Complex *in1=new Complex[s/2];
	Complex *in2=new Complex[s/2];
	int i;
	for(i=0;i<s/2;i++)
	{
		in1[i]=(in[i*2]);
		in2[i]=(in[i*2+1]);
	}
	delete[] in;
	Complex *out1,*out2;
	fft(in1,&out1,s/2);
	fft(in2,&out2,s/2);
	for(i=0;i<s/2;i++)
	{
		Complex deb1=out1[i];
		Complex deb2=out2[i];
		int o=0;
	}
	*out=new Complex[s];
	for(i=0;i<s/2;i++)
	{
		(*out)[i]=out1[i]+out2[i]*Complex::Exp(2*PI*i/s);
	}
	for(i=s/2;i<s;i++)
	{
		(*out)[i]=out1[i-s/2]+out2[i-s/2]*Complex::Exp(2*PI*i/s);
	}
	delete[] out1;delete[] out2;
	}
	else
	{*out=new Complex[1];(*out)[0]=in[0];delete[] in;}
}

void FFT(Complex*input,Complex*output,int s)
{
	int exp2;
	Complex* tmp=new Complex[s];
	for(exp2=0;(s>>exp2)!=1;exp2++);
	for(int i=0;i<s;i++)
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
	for(int i=0;i<s;i++)
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

void FFT2D(IplImage* input, IplImage* output)
{
	ASSERT(input!=NULL&&output!=NULL);
	ASSERT(input->width==output->width && input->height==output->height);
	ASSERT(input->depth==IPL_DEPTH_32F&&output->depth==IPL_DEPTH_32F);
	ASSERT(input->nChannels==2&&output->nChannels==2);

	IplImage* tmp=cvCreateImage(cvGetSize(output), IPL_DEPTH_32F, 2);
	IplImage* tmp1=cvCreateImage(cvSize(output->height,output->width),IPL_DEPTH_32F,2);
	IplImage* tmp2=cvCreateImage(cvSize(output->height,output->width),IPL_DEPTH_32F,2);
	
	for(int i=0;i<output->height;i++)
	{
		FFT((Complex*)(input->imageData+input->widthStep*i), 
			(Complex*)(tmp->imageData+tmp->widthStep*i),input->width);
	}
	cvTranspose(tmp, tmp1);
	for(int i=0;i<output->width;i++)
	{
		FFT((Complex*)(tmp1->imageData+tmp1->widthStep*i),
			(Complex*)(tmp2->imageData+tmp2->widthStep*i), tmp->width);
	}
	cvTranspose(tmp2, output);

	cvReleaseImage(&tmp);
	cvReleaseImage(&tmp1);
	cvReleaseImage(&tmp2);
}