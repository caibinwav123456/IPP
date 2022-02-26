#include "stdafx.h"
#include "matutil.h"

Vec2 operator/(Vec2 v,float a)
{
	Vec2 result=v;
	result/=a;
	return result;
}

Vec3 operator+(Vec3 a,Vec3 b)
{
	return Vec3(a.x+b.x,a.y+b.y,a.z+b.z);
}

Vec3 operator-(Vec3 a,Vec3 b)
{
	return Vec3(a.x-b.x,a.y-b.y,a.z-b.z);
}

Vec3 operator*(Vec3 v,float a)
{
	Vec3 result=v;
	result*=a;
	return result;
}

Vec3 operator*(float a,Vec3 v)
{
	Vec3 result=v;
	result*=a;
	return result;
}

Vec3 operator/(Vec3 v,float a)
{
	Vec3 result=v;
	result/=a;
	return result;
}

float dot(Vec3 a,Vec3 b)
{
	return a.x*b.x+a.y*b.y+a.z*b.z;
}

Vec3 cross(Vec3 a,Vec3 b)
{
	return Vec3(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x);
}

float length(Vec3 v)
{
	return v.length();
}

Vec3 normalize(Vec3 v)
{
	return v.normalize();
}

Mat transpose(Mat m)
{
	return m.trans();
}

Mat inv(Mat m)
{
	return m.inv();
}

Mat operator+(Mat a,Mat b)
{
	Mat result;
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			result.elem[i][j]=a.elem[i][j]+b.elem[i][j];
	return result;
}

Mat operator-(Mat a,Mat b)
{
	Mat result;
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			result.elem[i][j]=a.elem[i][j]-b.elem[i][j];
	return result;
}

Mat operator*(Mat m,float a)
{
	Mat result=m;
	result*=a;
	return result;
}

Mat operator*(float a,Mat m)
{
	Mat result=m;
	result*=a;
	return result;
}

Mat operator/(Mat m,float a)
{
	Mat result=m;
	result/=a;
	return result;
}

Vec3 operator*(Vec3 v,Mat m)
{
	Vec3 result(0,0,0);
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			result.elem[i]+=m.elem[j][i]*v.elem[j];
	return result;
}
