
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "SphereInversion.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "complex"
#include "math.h"
using namespace std;
// CChildView
#pragma warning(disable:4244)
#pragma warning(disable:4305)
#define PI 3.1415926535897932384626
float _X=0,_Y=0;
struct RotParam
{
	Vec2 p;
	Vec2 v;
};
complex<float> ZFunc(complex<float> z, float p);
float SegFunc(float t, float l, float h);
class Rotator
{
public:
	Rotator(float _a,float _b)
	{
		a=_a;b=_b;
	}
	RotParam GetRot(Vec2 v, float seg)
	{
		float i=1.4f;
		complex<float> z(i,seg+PI/2);
		complex<float> zr=ZFunc(z, 7.3);
		Vec2 c(zr.real(), zr.imag()/4);
		zr.real(-c.y/_Y*_X);zr.imag(c.x/_X*_Y);
		c=Vec2(zr.real(),zr.imag());
		RotParam param;
		param.v=(v-c)/2;
		param.p=(v+c)/2;
		return param;
	}

	float a,b;
};

class Rotator1
{
public:
	Rotator1()
	{
	}
	RotParam GetRot(Vec2 v, int seg)
	{
		Vec2 s;
		switch(seg)
		{
		case 0:
			s=Vec2(v.x,0);
			break;
		case 1:
			s=Vec2(0,v.y);
			break;
		}
		Vec2 pv=v-s;
		RotParam param;
		param.v=s;
		param.p=pv;
		return param;
	}
};

Vec2 Rotate(Vec2 v, float a, float k, float th)
{
	float mat[3][4];
	mat[0][0]=mat[0][3]=mat[2][0]=mat[2][3]=cosf(th);
	mat[0][1]=mat[2][2]=-sinf(th);
	mat[0][2]=mat[2][1]=-mat[0][1];
	mat[1][0]=mat[1][3]=0;
	mat[1][1]=1.f/k;
	mat[1][2]=-k;
	Vec2 v2=v;
	for(int i=0;i<3;i++)
	{
		Vec2 _v2;
		_v2.x=v2.x*mat[i][0]+v2.y*mat[i][2];
		_v2.y=v2.x*mat[i][1]+v2.y*mat[i][3];
		v2=_v2;
	}
	return v*cosf(a)+v2*sinf(a);
	//return Vec2(v.x*cosf(a)-2*v.y*sinf(a),v.x*sinf(a)+v.y*cosf(a));
}
Vec3 Rotate3(Vec3 v, float a, Vec3 axis)
{
	Vec3 vc=cross(v,axis);
	if(vc.length()==0)
		return v;
	Vec3 vr = vc.normalize()*v.length();
	Vec3 vret = v*cosf(a)+vr*sinf(a);
	return vret;
}

class Rotatorl
{
public:
	RotParam GetRot(Vec2 v, int t)
	{
		const float angle = PI/6;
		float x=t*cosf(angle);
		RotParam ret;
		ret.p = Vec2(x,0);
		ret.v = v-ret.p;
		return ret;
	}
};

class Rotatorc
{
public:
	RotParam GetRot(Vec2 v, int t)
	{
		const float angle = PI/8;
		Vec2 p(cosf(t*angle),sinf(t*angle));
		RotParam ret;
		ret.p = p;
		ret.v = v-p;
		return ret;
	}
};

CChildView::CChildView():m_rot(1,0,0,0,1,0,0,0,1),m_pt(0,0)
{
	m_extent = 0;
	m_bDown = false;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}
//#define ORIGIN
void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	Draw();
}
void CChildView::Draw()
{
	CClientDC dc(this);
	CRect rcClient;
	GetClientRect(rcClient);
	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rcClient.Width(),rcClient.Height());
	CBitmap* oldbmp = dcMem.SelectObject(&bmp);
	Draw(&dcMem);
	DrawWave(&dcMem);
	DrawWavyCurve(&dcMem);
	dc.BitBlt(0,0,rcClient.Width(),rcClient.Height(),&dcMem,0,0,SRCCOPY);
	dcMem.SelectObject(oldbmp);
	dcMem.DeleteDC();
	bmp.DeleteObject();
	// Do not call CWnd::OnPaint() for painting messages
}

void CChildView::Draw(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(rcClient);
	float w=(float)rcClient.Width(),h=(float)rcClient.Height();
	pDC->FillSolidRect(rcClient, RGB(255,255,255));
	// TODO: Add your message handler code here
	float i=1.4f;
	{
		complex<float> f[4];
		for(int j=0;j<4;j++)
		{
			complex<float> z(i,j*PI/2);
			complex<float> zr=ZFunc(z, 7.3);
			f[j]=zr;
			float x=zr.real()/6*2*h+w/2,y=zr.imag()/3*4*h+h/2;
			CPoint pt((int)x,(int)y);
			pDC->SelectStockObject(NULL_BRUSH);
			pDC->Ellipse(pt.x-5,pt.y-5,pt.x+5,pt.y+5);
		}

		float X=_X=fabs(f[0].real());
		float Y=_Y=fabs(f[1].imag());
		bool bfirst=true;
		Rotator rot(X,Y);
		for(float j=0;j<=2*PI;j+=0.01f)
		{
			complex<float> z(i,j);
			complex<float> zr=ZFunc(z, 7.3);
			Vec2 c(zr.real()/4, zr.imag());
			RotParam r=rot.GetRot(c, j);
			Vec3 c3(c.x,c.y,0);
			Vec3 p3(r.p.x,r.p.y,0),v3(r.v.x,r.v.y,0);
			int sign=((int)floorf(j/PI*2)%2)*2-1;
			Vec3 rt=p3+Rotate3(v3, m_extent*PI, Vec3(-v3.y*sign,v3.x*sign,0));
			rt=rt*m_rot;
			Vec3 rtp = rt+Vec3(0,0,3);
			Vec3 rtpr = rtp/rtp.z;
			Vec2 rtv = Vec2(rtpr.x,rtpr.y)*3;
			float x=rtv.x/3*4*h+w/2,y=-rtv.y/3*4*h+h/2;
			CPoint pt((int)x,(int)y);
			if(bfirst)
				pDC->MoveTo(pt);
			else
				pDC->LineTo(pt);
			bfirst=false;
		}
		bfirst=true;
		for(int j=0;j<=50;j++)
		{
			float x=(1-cosf((float)j/50*PI/2))*X;
			float y=(1-sinf((float)j/50*PI/2))*Y;
			x=x/6*2*h+w/2,y=y/3*4*h+h/2;
			CPoint pt((int)x,(int)y);
			if(bfirst)
				pDC->MoveTo(pt);
			else
				pDC->LineTo(pt);
			bfirst=false;
		}
	}
}

void CChildView::Draw2(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(rcClient);
	float w=(float)rcClient.Width(),h=(float)rcClient.Height();
	pDC->FillSolidRect(rcClient, RGB(255,255,255));
	// TODO: Add your message handler code here
	for(float i=0.1f;i<2.f;i+=0.1f)
	{
		bool bfirst=true;
		for(float j=-PI;j<=PI;j+=0.01f)
		{
			complex<float> z(i,j);
			complex<float> zr=ZFunc(z,3);
			float x=zr.real()/3*h+w/2,y=zr.imag()/3*h+h/2;
			CPoint pt((int)x,(int)y);
			if(bfirst)
				pDC->MoveTo(pt);
			else
				pDC->LineTo(pt);
			bfirst=false;
		}
	}
	for(float j=-PI;j<PI;j+=0.1f)
	{
		bool bfirst=true;
		for(float i=0.1f;i<2.f;i+=0.1f)
		{
			complex<float> z(i,j);
			complex<float> zr=ZFunc(z,3);
			float x=zr.real()/3*h+w/2,y=zr.imag()/3*h+h/2;
			CPoint pt((int)x,(int)y);
			if(bfirst)
				pDC->MoveTo(pt);
			else
				pDC->LineTo(pt);
			bfirst=false;
		}
	}
}

void CChildView::DrawWave(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(rcClient);
	float w=(float)rcClient.Width(),h=(float)rcClient.Height();
	bool bfirst=true;
	Rotatorl rot;
	for(float f=-5.f;f<5.f;f+=0.01f)
	{
		Vec2 r=WaveFunc(f);
		float r1=floorf(f*2);
		float r2=r1+1;
		RotParam rot1=rot.GetRot(r, (int)r1);
		RotParam rot2=rot.GetRot(r, (int)r2);
		Vec2 rr1 = Rotate(rot1.v, m_extent*PI, 2, 0)+rot1.p;
		Vec2 rr2 = Rotate(rot2.v, m_extent*PI, 2, 0)+rot2.p;
		float ext = 1-(f*2-floorf(f*2));
		Vec2 re = rr1*ext+rr2*(1-ext);
		CPoint pt(re.x*h/10+w/2,re.y*h/10+h/8*7);
		if(bfirst)
			pDC->MoveTo(pt);
		else
			pDC->LineTo(pt);
		bfirst=false;
	}
}

void CChildView::DrawWavyCurve(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(rcClient);
	float w=(float)rcClient.Width(),h=(float)rcClient.Height();
	bool bfirst=true;
	Rotatorc rot;
	for(float f=0;f<2*PI;f+=0.01)
	{
		Vec2 curve(cos(f),sin(f));
		Vec2 wave=_WaveFunc(f/2/PI*16);
		Vec2 tw(-wave.x*curve.y-wave.y*curve.x, wave.x*curve.x-wave.y*curve.y);
		Vec2 c;
		if(m_extent<0.5)
			c=curve+tw*0.4*m_extent*2;
		else
		{
			Vec2 c1=curve+tw*0.4;
			float f1=f/(PI/8);
			float f2=f1+1;
			RotParam rot1=rot.GetRot(c1,(int)floorf(f1));
			RotParam rot2=rot.GetRot(c1,(int)floorf(f2));
			Vec2 r1=rot1.p+Rotate(rot1.v, (m_extent*2-1)*PI, 1.5, f+PI/2);
			Vec2 r2=rot2.p+Rotate(rot2.v, (m_extent*2-1)*PI, 1.5, f+PI/2);
			float ext=1-(f1-floorf(f1));
			c=r1*ext+r2*(1-ext);
		}
		CPoint pt(c.x*h/5+w/5,c.y*h/5+h/2);
		if(bfirst)
			pDC->MoveTo(pt);
		else
			pDC->LineTo(pt);
		bfirst=false;
	}
}

Vec2 CChildView::WaveFunc(float t)
{
	float tn=floorf(t);
	float parity=t-floorf(t/2)*2;
	int bp=(parity>1?1:-1);
	float frac=t-tn;
	const float angle = PI/6;
	float th=angle+PI-frac*(2*angle+PI);
	Vec2 r(cos(th),bp*0.5*(sin(th)+sin(angle)));
	r+=Vec2(tn*2*cos(angle)+cos(angle),0);
	return r;
}

Vec2 CChildView::_WaveFunc(float t)
{
	float tn=floorf(t);
	float parity=t-floorf(t/2)*2;
	int bp=(parity>1?1:-1);
	float frac=t-tn;
	const float angle = PI/6;
	float th=angle+PI-frac*(2*angle+PI);
	Vec2 r(cos(th),bp*0.5*(sin(th)+sin(angle)));
	r+=Vec2(tn*2*cos(angle),0);
	r-=Vec2(t*2*cos(angle)-cos(angle),0);
	return r;
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return 0;//CWnd::OnEraseBkgnd(pDC);
}

complex<float> ZFunc(complex<float> z, float p)
{
	complex<float> k(p,0);
	complex<float> z1(1,0),z2(2,0),z3(3,0);
	complex<float> ze=exp(z);
	complex<float> za=pow(ze+z1,k),zb=pow(ze-z1,k);
	complex<float> zr=(za+zb)/(za-zb);//(exp(z3*z)+z3*exp(z))/(z3*exp(z2*z)+z1);
	return zr;
}

float SegFunc(float t, float l, float h)
{
	float frac=t-floorf(t/2)*2;
	float f=fabs(frac-1);
	//const float l=0.8,h=0.95;
	if(f<l)return 0;
	else if(f>h)return 1;
	else
	{
		float th = (f-l)/(h-l);
		float re = (1-cosf(th*PI))*0.5;
		return re;
	}
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	switch(nChar)
	{
	case 'W':
		m_extent+=0.01f;
		if(m_extent>1)m_extent=1;
		Draw();
		break;
	case 'S':
		m_extent-=0.01f;
		if(m_extent<0)m_extent=0;
		Draw();
		break;
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}


void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_bDown = true;
	m_pt = point;
	SetCapture();
	CWnd::OnLButtonDown(nFlags, point);
}


void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_bDown = false;
	m_pt = CPoint(0,0);
	ReleaseCapture();
	CWnd::OnLButtonUp(nFlags, point);
}


void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if(m_bDown)
	{
		CPoint off = point-m_pt;
		Vec2 vo((float)off.x/100,(float)off.y/100);
		Mat rotx(cosf(vo.x),0,-sinf(vo.x),0,1,0,sinf(vo.x),0,cosf(vo.x));
		Mat roty(1,0,0,0,cosf(vo.y),sinf(vo.y),0,-sinf(vo.y),cosf(vo.y));
		m_rot = m_rot*rotx*roty;
		m_rot = m_rot.ortho();
		m_pt=point;
		Draw();
	}
	CWnd::OnMouseMove(nFlags, point);
}
