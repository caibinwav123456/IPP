#ifndef _MATUTIL_H_
#define _MATUTIL_H_

#include <math.h>
#ifdef USE_OPENCV
#include <cv.h>
#include <cxcore.h>
#endif

class Vec2;
class Vec3;
class Vec4;

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

Vec3 operator*(Vec3 v1,Vec3 v2);
Vec3 operator*(Vec3 v,float a);
Vec3 operator*(float a,Vec3 v);
Vec3 operator/(Vec3 v,float a);
float dot(Vec3 a,Vec3 b);
Vec3 cross(Vec3 a,Vec3 b);
float length(Vec3 v);
Vec3 normalize(Vec3 v);
Vec3 operator+(Vec3 a,Vec3 b);
Vec3 operator-(Vec3 a,Vec3 b);

Vec4 operator*(Vec4 v1,Vec4 v2);
Vec4 operator*(Vec4 v,float a);
Vec4 operator*(float a,Vec4 v);
Vec4 operator/(Vec4 v,float a);
float dot(Vec4 a,Vec4 b);
float length(Vec4 v);
Vec4 normalize(Vec4 v);
Vec4 operator+(Vec4 a,Vec4 b);
Vec4 operator-(Vec4 a,Vec4 b);

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
		double elem[3];
		struct 
		{
			double x,y,z;
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

class Vec4
{
public:
	union
	{
		double elem[4];
		struct 
		{
			double x,y,z,t;
		};
	};
	Vec4()
	{
		x=y=z=t=0;
	}
	Vec4(float _x,float _y,float _z,float _t)
	{
		x=_x;y=_y;z=_z;t=_t;
	}
	Vec4& operator*=(float a)
	{
		for(int i=0;i<4;i++)
			elem[i]*=a;
		return *this;
	}
	Vec4& operator*=(Vec4 v)
	{
		for(int i=0;i<4;i++)
			elem[i]*=v.elem[i];
		return *this;
	}
	Vec4& operator/=(float a)
	{
		(*this)*=(1/a);
		return *this;
	}
	Vec4& operator+=(Vec4 a)
	{
		x+=a.x;y+=a.y;z+=a.z;t+=a.t;
		return *this;
	}
	Vec4& operator-=(Vec4 a)
	{
		x-=a.x;y-=a.y;z-=a.z;t-=a.t;
		return *this;
	}
	bool operator==(Vec4 a)
	{
		return x==a.x&&y==a.y&&z==a.z&&t==a.t;
	}
	bool operator!=(Vec4 a)
	{
		return x!=a.x||y!=a.y||z!=a.z||t!=a.t;
	}
	Vec4 operator-()
	{
		return Vec4(-x,-y,-z,-t);
	}
	float length()
	{
		return sqrt(x*x+y*y+z*z+t*t);
	}
	Vec4 normalize()
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
		double elem[3][3];
		struct
		{
			Vec3   v[3];
		};
		struct
		{
			double _00,_01,_02;
			double _10,_11,_12;
			double _20,_21,_22;
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

	Mat& operator*=(double a)
	{
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++)
				elem[i][j]*=a;
		return *this;
	}

	Mat& operator/=(double a)
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

	double det()
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
	Mat ortho()
	{
		Mat result = *this;
		result._0 = result._0.normalize();
		result._2 = cross(result._0,result._1).normalize();
		result._1 = cross(result._2,result._0).normalize();
		return result;
	}
};

class Mat4;
Mat4 transpose(Mat4 m);
Mat4 inv(Mat4 m);
Mat4 operator+(Mat4 a,Mat4 b);
Mat4 operator-(Mat4 a,Mat4 b);
Mat4 operator*(Mat4 m,float a);
Mat4 operator*(float a,Mat4 m);
Mat4 operator/(Mat4 m,float a);
Vec4 operator*(Vec4 v,Mat4 m);

class Mat4
{
	friend Vec4 operator*(Vec4 v,Mat4 m);
public:
	union
	{
		double elem[4][4];
		struct
		{
			Vec4   v[4];
		};
		struct
		{
			double _00,_01,_02,_03;
			double _10,_11,_12,_13;
			double _20,_21,_22,_23;
			double _30,_31,_32,_33;
		};
		struct
		{
			Vec4 _0;
			Vec4 _1;
			Vec4 _2;
			Vec4 _3;
		};
	};

	Mat4()
	{
		for(int i=0;i<4;i++)
			for (int j=0;j<4;j++)
				elem[i][j]=0;
	}

	Mat4(Vec4 v1,Vec4 v2,Vec4 v3,Vec4 v4)
	{
		_0=v1;_1=v2;_2=v3;
	}

	Mat4(Vec4 v1,Vec4 v2,Vec4 v3,Vec4 v4,int d)
	{
		Vec4* pv[4]={&v1,&v2,&v3,&v4};
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				elem[i][j]=pv[j]->elem[i];
	}

	Mat4(float xx,float xy,float xz,float xt,float yx,float yy,float yz,float yt,float zx,float zy,float zz,float zt,float tx,float ty,float tz,float tt)
	{
		float pv[4][4]={{xx,xy,xz,xt},{yx,yy,yz,yt},{zx,zy,zz,zt},{tx,ty,tz,tt}};
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				elem[i][j]=pv[i][j];
	}

	Mat4& operator+=(Mat4 a)
	{
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				elem[i][j]+=a.elem[i][j];
		return *this;
	}

	Mat4& operator-=(Mat4 a)
	{
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				elem[i][j]-=a.elem[i][j];
		return *this;
	}

	Mat4& operator*=(double a)
	{
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				elem[i][j]*=a;
		return *this;
	}

	Mat4& operator/=(double a)
	{
		(*this)*=(1/a);
		return *this;
	}

	Mat4 operator*(Mat4 mat)
	{
		Mat4 result;
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				for(int k=0;k<4;k++)
					result.elem[i][j]+=elem[i][k]*mat.elem[k][j];
		return result;
	}

	Mat4& operator*=(Mat4 m)
	{
		*this=*this*m;
		return *this;
	}

	Mat4 operator-()
	{
		Mat4 result;
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				result.elem[i][j]=-elem[i][j];
		return result;
	}

	Mat4 trans()
	{
		Mat4 result;
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				result.elem[i][j]=elem[j][i];
		return result;
	}

	double det()
	{
		Mat m[4];
		for(int i=0;i<4;i++)
			for(int j=0;j<3;j++)
				for(int k=0;k<3;k++)
					m[i].elem[j][k]=elem[j+1][k>=i?k+1:k];
		return elem[0][0]*m[0].det()-elem[0][1]*m[1].det()+elem[0][2]*m[2].det()-elem[0][3]*m[3].det();
	}

	Mat4 inv()
	{
		Mat4 result;
		for(int i=0;i<4;i++)
		for(int j=0;j<4;j++)
		{
			Mat m;
			for(int k=0;k<3;k++)
				for(int l=0;l<3;l++)
					m.elem[k][l] = elem[k>=j?k+1:k][l>=i?l+1:l];
			result.elem[i][j]=m.det()*pow(-1.0,i+j);
		}
		result/=det();
		return result;
	}
};


#endif