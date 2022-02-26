#ifndef _FFT_H
#define _FFT_H
#include <math.h>
class Complex
{
public:
	float r,i;
	Complex(){r=i=0;}
	Complex(double x,double y){r=x,i=y;}
	Complex operator*(Complex& val)
	{
		return Complex(r*val.r-i*val.i,r*val.i+i*val.r);
	}
	Complex operator+(Complex& val)
	{
		return Complex(r+val.r,i+val.i);
	}
	Complex operator-(Complex& val)
	{
		return Complex(r-val.r,i-val.i);
	}
	Complex operator-()
	{
		return Complex(-r,-i);
	}
	double length()
	{
		return r*r+i*i;
	}
	static Complex Exp(double theta)
	{
		return Complex(cos(theta),sin(theta));
	}
};

void FFT(Complex*input,Complex*output,int s);
void FFT2(Complex*input,Complex*output,int s);
void FFT2D(IplImage* input,IplImage* output);
#endif