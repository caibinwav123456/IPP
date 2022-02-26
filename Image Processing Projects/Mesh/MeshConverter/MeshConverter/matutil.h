#ifndef _MATUTIL_H_
#define _MATUTIL_H_

#include <math.h>
#ifdef USE_OPENCV
#include <cv.h>
#include <cxcore.h>
#endif

class Vec2;
class Vec3;

Vec2 operator*(Vec2 v1,Vec2 v2);
Vec3 operator*(Vec3 v1,Vec3 v2);
Vec2 operator*(Vec2 v,float a);
Vec2 operator*(float a,Vec2 v);
Vec2 operator/(Vec2 v,float a);
float dot(Vec2 a,Vec2 b);
Vec2 cross(Vec2 a,Vec2 b);
float length(Vec2 v);
Vec2 normalize(Vec2 v);
Vec2 operator+(Vec2 a,Vec2 b);
Vec2 operator-(Vec2 a,Vec2 b);

Vec3 operator*(Vec3 v,float a);
Vec3 operator*(float a,Vec3 v);
Vec3 operator/(Vec3 v,float a);
float dot(Vec3 a,Vec3 b);
Vec3 cross(Vec3 a,Vec3 b);
float length(Vec3 v);
Vec3 normalize(Vec3 v);
Vec3 operator+(Vec3 a,Vec3 b);
Vec3 operator-(Vec3 a,Vec3 b);

class Vec2
{
public:
	union
	{
		float elem[2];
		struct 
		{
			float x,y;
		};
	};
	Vec2()
	{
		x=y=0;
	}
	Vec2(float _x,float _y)
	{
		x=_x;y=_y;
	}
#ifdef USE_OPENCV
	Vec2(CvPoint2D32f& v)
	{
		x=v.x;y=v.y;
	}
	operator CvPoint2D32f()
	{
		return cvPoint2D32f(x,y);
	}
#endif
	Vec2& operator*=(float a)
	{
		for(int i=0;i<2;i++)
			elem[i]*=a;
		return *this;
	}
	Vec2& operator*=(Vec2 v)
	{
		for(int i=0;i<2;i++)
			elem[i]*=v.elem[i];
		return *this;
	}
	Vec2& operator/=(float a)
	{
		(*this)*=(1/a);
		return *this;
	}
	Vec2& operator+=(Vec2 a)
	{
		x+=a.x;y+=a.y;
		return *this;
	}
	Vec2& operator-=(Vec2 a)
	{
		x-=a.x;y-=a.y;
		return *this;
	}
	bool operator==(Vec2 a)
	{
		return x==a.x&&y==a.y;
	}
	bool operator!=(Vec2 a)
	{
		return x!=a.x||y!=a.y;
	}
	Vec2 operator-()
	{
		return Vec2(-x,-y);
	}
	float length()
	{
		return sqrt(x*x+y*y);
	}
	Vec2 normalize()
	{
		return *this/length();
	}
};


class Vec3
{
	friend float dot(Vec3 a,Vec3 b);
	friend Vec3 cross(Vec3 a,Vec3 b);
public:
	union
	{
		float elem[3];
		struct 
		{
			float x,y,z;
		};
	};
	Vec3()
	{
		x=y=z=0;
	}
	Vec3(float _x,float _y,float _z)
	{
		x=_x;y=_y;z=_z;
	}
#ifdef USE_OPENCV
	Vec3(CvPoint3D32f& v)
	{
		x=v.x;y=v.y;z=v.z;
	}
	operator CvPoint3D32f()
	{
		return cvPoint3D32f(x,y,z);
	}
#endif
	Vec3& operator*=(float a)
	{
		for(int i=0;i<3;i++)
			elem[i]*=a;
		return *this;
	}
	Vec3& operator*=(Vec3 v)
	{
		for(int i=0;i<3;i++)
			elem[i]*=v.elem[i];
		return *this;
	}
	Vec3& operator/=(float a)
	{
		(*this)*=(1/a);
		return *this;
	}
	Vec3& operator+=(Vec3 a)
	{
		x+=a.x;y+=a.y;z+=a.z;
		return *this;
	}
	Vec3& operator-=(Vec3 a)
	{
		x-=a.x;y-=a.y;z-=a.z;
		return *this;
	}
	bool operator==(Vec3 a)
	{
		return x==a.x&&y==a.y&&z==a.z;
	}
	bool operator!=(Vec3 a)
	{
		return x!=a.x||y!=a.y||z!=a.z;
	}
	Vec3 operator-()
	{
		return Vec3(-x,-y,-z);
	}
	float length()
	{
		return sqrt(x*x+y*y+z*z);
	}
	Vec3 normalize()
	{
		return *this/length();
	}
};

class Mat;
Mat transpose(Mat m);
Mat inv(Mat m);
Mat operator+(Mat a,Mat b);
Mat operator-(Mat a,Mat b);
Mat operator*(Mat m,float a);
Mat operator*(float a,Mat m);
Mat operator/(Mat m,float a);
Vec3 operator*(Vec3 v,Mat m);
class Mat
{
	friend Vec3 operator*(Vec3 v,Mat m);
public:
	union
	{
		float elem[3][3];
		struct
		{
			Vec3   v[3];
		};
		struct
		{
			float _00,_01,_02;
			float _10,_11,_12;
			float _20,_21,_22;
		};
		struct
		{
			Vec3 _0;
			Vec3 _1;
			Vec3 _2;
		};
	};

	Mat()
	{
		for(int i=0;i<3;i++)
			for (int j=0;j<3;j++)
				elem[i][j]=0;
	}

	Mat(Vec3 v1,Vec3 v2,Vec3 v3)
	{
		_0=v1;_1=v2;_2=v3;
	}

	Mat(Vec3 v1,Vec3 v2,Vec3 v3,int d)
	{
		Vec3* pv[3]={&v1,&v2,&v3};
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++)
				elem[i][j]=pv[j]->elem[i];
	}

	Mat(float xx,float xy,float xz,float yx,float yy,float yz,float zx,float zy,float zz)
	{
		float pv[3][3]={{xx,xy,xz},{yx,yy,yz},{zx,zy,zz}};
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++)
				elem[i][j]=pv[i][j];
	}

	Mat& operator+=(Mat a)
	{
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++)
				elem[i][j]+=a.elem[i][j];
		return *this;
	}

	Mat& operator-=(Mat a)
	{
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++)
				elem[i][j]-=a.elem[i][j];
		return *this;
	}

	Mat& operator*=(float a)
	{
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++)
				elem[i][j]*=a;
		return *this;
	}

	Mat& operator/=(float a)
	{
		(*this)*=(1/a);
		return *this;
	}

	Mat operator*(Mat mat)
	{
		Mat result;
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++)
				for(int k=0;k<3;k++)
					result.elem[i][j]+=elem[i][k]*mat.elem[k][j];
		return result;
	}
	
	Mat& operator*=(Mat m)
	{
		*this=*this*m;
		return *this;
	}

	Mat operator-()
	{
		Mat result;
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++)
				result.elem[i][j]=-elem[i][j];
		return result;
	}

	Mat trans()
	{
		Mat result;
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++)
				result.elem[i][j]=elem[j][i];
		return result;
	}

	float det()
	{
		return _00*_11*_22-_00*_12*_21-_01*_10*_22+_01*_12*_20+_02*_10*_21-_02*_11*_20;
	}

	Mat inv()
	{
		Mat result(_11*_22-_21*_12, _02*_21-_01*_22, _01*_12-_02*_11, 
			_12*_20-_10*_22, _00*_22-_02*_20, _02*_10-_00*_12,
			_10*_21-_11*_20, _01*_20-_00*_21, _00*_11-_01*_10);
		result/=det();
		return result;
	}
};



#endif