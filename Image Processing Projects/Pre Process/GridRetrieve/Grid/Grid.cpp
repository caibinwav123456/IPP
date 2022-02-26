// Grid.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "math.h"
#include <vector>
using namespace std;

#define WIDTH  800
#define HEIGHT 600
#define SEGX   40
#define SEGY   30
#define LINEW  4
#define MINV   110
#define MAXV   (255-MINV)
int  PROL = 30;
float CONN_LENGTH =20;
float CONN_WIDTH  =3;

vector<CvPoint> fldpt;
void connect(IplImage* image, IplImage* acc, bool bProl=false);
bool findtread(IplImage* image, int y, int x, IplImage* comp, IplImage* acc, bool bProl=false);
void floodfill(IplImage* image, int cx, int cy, IplImage* comp);
void findnodes();
int shift[8][2]={{1,0},{0,1},{-1,0},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
vector<vector<CvPoint*>> nodes;
IplImage* imgcon=NULL;
IplImage* horz = NULL;
IplImage* vert = NULL;
IplImage* horzo = NULL;
IplImage* verto = NULL;
IplImage* maskh = NULL;
IplImage* maskv = NULL;
IplImage* mask = NULL;
IplImage* acc = NULL;
IplImage* horzp = NULL;
IplImage* vertp = NULL;
CvFont font;

int radius=5;

void Mouse(int event, int x,int y,int flags, void* param)
{
	if(event==CV_EVENT_MOUSEMOVE)
	{
	}
	else if(event==CV_EVENT_LBUTTONDOWN)
	{
		//cvCircle(maskh, cvPoint(x,y), radius, cvScalar(CONN_LENGTH), -1);
		cvCircle(horz, cvPoint(x,y), radius, cvScalarAll(0), -1);
		mask=maskh;
		cvZero(acc);
		//cvCopyImage(horzp,horz);
		//connect(horz,acc);
	}
	else if(event==CV_EVENT_RBUTTONDOWN)
	{
		cvCircle(maskh, cvPoint(x,y), radius, cvScalar(PROL,0,  0), -1);
		mask=maskh;
		cvZero(acc);
		cvCopyImage(horzp,horz);
		connect(horz,acc,true);
	}
	cvCopyImage(horz,horzo);
	cvCircle(horzo, cvPoint(x,y),radius,cvScalar(0,0,255));
	cvShowImage("horz",horzo);
}

void Mouse2(int event, int x,int y,int flags, void* param)
{
	if(event==CV_EVENT_MOUSEMOVE)
	{
	}
	else if(event==CV_EVENT_LBUTTONDOWN)
	{
		cvCircle(vert, cvPoint(x,y), radius, cvScalarAll(0), -1);
		//cvCircle(maskv, cvPoint(x,y), radius, cvScalar(CONN_LENGTH), -1);
		mask=maskv;
		cvZero(acc);
		//cvCopyImage(vertp,vert);
		//connect(vert,acc);
	}
	else if(event==CV_EVENT_RBUTTONDOWN)
	{
		cvCircle(maskv, cvPoint(x,y), radius, cvScalar(PROL,0, 0), -1);
		mask=maskv;
		cvZero(acc);
		cvCopyImage(vertp,vert);
		connect(vert,acc,true);
	}
	cvCopyImage(vert,verto);
	cvCircle(verto, cvPoint(x,y),radius,cvScalar(0,0,255));
	cvShowImage("vert",verto);
}

int _tmain(int argc, _TCHAR* argv[])
{
	printf("Next Step:N\n");
	IplImage* image= cvLoadImage("flippagegrid.bmp");
	horz = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 3);
	vert = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 3);
	acc = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 1);
	IplImage* horzc = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U,3);
	IplImage* vertc = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U,3);
	horzo = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U,3);
	verto = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U,3);
	horzp = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U,3);
	vertp = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U,3);
	maskh = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U,1);
	maskv = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U,1);
	cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX, 0.5, 0.5);

	//cvZero(maskh);
	//cvZero(maskv);
	cvSet(maskh, cvScalar(PROL,0));//CONN_LENGTH,
	cvSet(maskv, cvScalar(PROL,0));//CONN_LENGTH,
	cvZero(horzc);
	cvZero(vertc);
	cvZero(horz);
	cvZero(vert);
	cvZero(horzp);
	cvZero(vertp);

	for(int i=0;i<image->height;i++)
	{
		for(int j=0;j<image->width;j++)
		{
			CvScalar pix=cvGet2D(image, i, j);
			if(pix.val[1]<MINV)
			{
				if(pix.val[2]>MAXV && (pix.val[0]<MINV || pix.val[0]>MAXV))
				{
					cvSet2D(horzp, i, j, cvScalarAll(255));
				}
				if(pix.val[0]>MAXV && (pix.val[2]<MINV || pix.val[2]>MAXV))
				{
					cvSet2D(vertp, i, j, cvScalarAll(255));
				}
			}
		}
	}
	//cvFilter2D(horz,horz,kernel);
	//cvFilter2D(vert,vert,kernel);

	cvCopyImage(horzp, horz);
	cvCopyImage(vertp, vert);

	cvZero(acc);
	imgcon = horzc;
	mask=maskh;
	connect(horz,acc);

	cvZero(acc);
	imgcon=vertc;
	mask=maskv;
	connect(vert,acc);

	cvCopyImage(horz,horzo);
	cvCopyImage(vert,verto);

	cvNamedWindow("horzc");
	cvShowImage("horzc", horzc);

	cvNamedWindow("vertc");
	cvShowImage("vertc", vertc);

	cvNamedWindow("image");
	cvShowImage("image", image);
	cvNamedWindow("horz");
	cvShowImage("horz", horzo);
	cvNamedWindow("vert");
	cvShowImage("vert",verto);
	cvSetMouseCallback("horz", Mouse);
	cvSetMouseCallback("vert", Mouse2);
	while(true)
	{
		char c=cvWaitKey();
		//printf("%c\n",c);
		if(c=='s')
		{
			cvSaveImage("horz.bmp",horz);
			cvSaveImage("vert.bmp",vert);
		}
		else if(c=='c')
		{
			cvCopyImage(horz, horzp);
			cvCopyImage(vert, vertp);
			cvSet(maskh,cvScalar(CONN_LENGTH));//cvZero(maskh);
			cvSet(maskv,cvScalar(CONN_LENGTH));//cvZero(maskv);
			imgcon=horzc;
			mask=maskh;
			cvZero(acc);
			connect(horz,acc);
			imgcon=vertc;
			mask=maskv;
			cvZero(acc);
			connect(vert,acc);
			cvCopyImage(horz, horzo);
			cvCopyImage(vert,verto);
			cvPutText(horzo, "commited", cvPoint(0,20),&font, cvScalar(0,255,255));
			cvPutText(verto, "commited", cvPoint(0,20),&font, cvScalar(0,255,255));
			cvShowImage("horz",horzo);
			cvShowImage("vert",verto);
			cvShowImage("horzc",horzc);
			cvShowImage("vertc",vertc);
		}
		else if(c=='z'||c=='x')
		{
			if(c=='z')
			{
				CONN_LENGTH++;
			}
			else
			{
				CONN_LENGTH--;
			}
			if(CONN_LENGTH<0)
			{
				CONN_LENGTH=0;
			}
			cvCopyImage(horzp,horz);
			cvCopyImage(vertp,vert);
			mask=maskh;
			cvZero(acc);
			connect(horz,acc);
			mask=maskv;
			cvZero(acc);
			connect(vert,acc);
			cvCopyImage(horz, horzo);
			cvCopyImage(vert,verto);
			char str[30];
			sprintf(str,"connect length:%d",(int)CONN_LENGTH);
			cvPutText(horzo, str, cvPoint(0,20),&font, cvScalar(0,255,255));
			cvPutText(verto, str, cvPoint(0,20),&font, cvScalar(0,255,255));
			cvShowImage("horz",horzo);
			cvShowImage("vert",verto);
		}
		else if(c=='q' || c=='w')
		{
			if(c=='q')
			{
				radius++;
			}
			else
			{
				radius--;
				if(radius<0)
					radius=0;
			}
		}
		else if(c=='n')
		{
			cvCopyImage(horz, horzp);
			cvCopyImage(vert, vertp);
			break;
		}
	}

	cvZero(acc);
	mask=maskh;
	connect(horz, acc, true);
	cvZero(acc);
	mask=maskv;
	connect(vert, acc, true);
	cvCopyImage(horz,horzo);
	cvCopyImage(vert,verto);
	cvShowImage("horz", horzo);
	cvShowImage("vert", verto);
	while(true)
	{
		char c=cvWaitKey();
		if(c=='z'||c=='x')
		{
			if(c=='z')
			{
				PROL++;
			}
			else
			{
				PROL--;
			}
			if(PROL<0)
			{
				PROL=0;
			}
			cvCopyImage(horzp,horz);
			cvCopyImage(vertp,vert);
			mask=maskh;
			cvZero(acc);
			connect(horz,acc,true);
			mask=maskv;
			cvZero(acc);
			connect(vert,acc,true);
			cvCopyImage(horz, horzo);
			cvCopyImage(vert,verto);
			char str[30];
			sprintf(str,"connect length:%d",(int)PROL);
			cvPutText(horzo, str, cvPoint(0,20),&font, cvScalar(0,255,255));
			cvPutText(verto, str, cvPoint(0,20),&font, cvScalar(0,255,255));
			cvShowImage("horz",horzo);
			cvShowImage("vert",verto);
		}
		else if(c=='q' || c=='w')
		{
			if(c=='q')
			{
				radius++;
			}
			else
			{
				radius--;
				if(radius<0)
					radius=0;
			}
		}
		else if(c=='n')
			break;
	}


	cvReleaseImage(&image);
	cvReleaseImage(&horz);
	cvReleaseImage(&vert);
	cvReleaseImage(&acc);
	cvReleaseImage(&horzc);
	cvReleaseImage(&vertc);
	cvReleaseImage(&horzo);
	cvReleaseImage(&verto);
	cvReleaseImage(&maskh);
	cvReleaseImage(&maskv);
	cvReleaseImage(&horzp);
	cvReleaseImage(&vertp);

	return 0;
}

void connect(IplImage* image, IplImage* acc, bool bProl)
{
	IplImage* comp=cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 1);

	for(int i=0;i<image->height;i++)
	{
		for(int j=0;j<image->width;j++)
		{
			CvScalar pix=cvGet2D(image, i, j);
			CvScalar pa=cvGet2D(acc, i, j);
			if(pix.val[0]==255 && pa.val[0]==0)
			{//158,162,232,227
				if(!bProl)
				while(findtread(image, i, j, comp, acc))
				{
				/*	cvNamedWindow("mid");
					cvShowImage("mid",image);
					while(cvWaitKey()!='o');*/
				}
				else
				{
					findtread(image, i, j, comp, acc, true);
				/*	cvNamedWindow("mid");
					cvShowImage("mid",image);
					while(cvWaitKey()!='o');*/
				}
				//while(cvWaitKey()!='i');
			}
		}
	}

	cvReleaseImage(&comp);
}

bool findtread(IplImage* image, int y, int x, IplImage* comp, IplImage* acc, bool bProl)
{
	cvZero(comp);
	bool bRet=false;
	int cx=x,cy=y;

	floodfill(image,x,y,comp);
	cvOr(comp, acc, acc);

/*	cvNamedWindow("mid");
	cvShowImage("mid", image);
	while(cvWaitKey()!='p');
*/
	int n=0,accx=0,accy=0;
	float centerx,centery;
	for(int i=0;i<fldpt.size();i++)
	{
		n++;
		accx+=fldpt.at(i).x;
		accy+=fldpt.at(i).y;
	}
	centerx=(float)accx/n;
	centery=(float)accy/n;
	float xx=0,yy=0,xy=0;
	for(int i=0;i<fldpt.size();i++)
	{
		xx+=(fldpt.at(i).x-centerx)*(fldpt.at(i).x-centerx);
		yy+=(fldpt.at(i).y-centery)*(fldpt.at(i).y-centery);
		xy+=(fldpt.at(i).x-centerx)*(fldpt.at(i).y-centery);
	}
	xx/=n;
	yy/=n;
	xy/=n;
	CvPoint2D32f vEigen;
	vEigen.x=-xy;
	vEigen.y=(xx-yy-sqrt((xx+yy)*(xx+yy)-4*(xx*yy-xy*xy)))/2;
	if(vEigen.x==0&&vEigen.y==0)
	{
		vEigen.x=(yy-xx-sqrt((xx+yy)*(xx+yy)-4*(xx*yy-xy*xy)))/2;
		vEigen.y=-xy;
	}
	float l=sqrt(vEigen.x*vEigen.x+vEigen.y*vEigen.y);

	if(l>0 && n>4)
	{
		vEigen.x/=l;
		vEigen.y/=l;
	}
	else
	{
		vEigen.x=vEigen.y=0;
	}
	//printf("%f,%f\n",vEigen.x,vEigen.y);
	int length=CONN_LENGTH;
/*	if(mask)
	{
		CvScalar sc=cvGet2D(mask, centery, centerx);
		length = sc.val[0];
		//if(sc.val[0]==255)
		//	vEigen.x=vEigen.y=0;
	}
*/	//	cvLine(image, cvPoint(centerx-vEigen.x*10,centery-vEigen.y*10),cvPoint(centerx+vEigen.x*10,centery+vEigen.y*10), cvScalar(0,0,255), 3);

	int search=length;//=max(fabs(vEigen.x)*length,fabs(vEigen.y)*length);
	int dx=(floor(fabs(vEigen.x))+1)*length;
	int dy=(floor(fabs(vEigen.y))+1)*length;
	CvPoint ptl=cvPoint(0,0),ptr=cvPoint(0,0);
	float dmin=0,dmax=0;
	float scl=PROL;
	for(int i=0;i<fldpt.size();i++)
	{
		float dot=(fldpt.at(i).y-centery)*vEigen.y+(fldpt.at(i).x-centerx)*vEigen.x;
		if(dot<dmin)
		{
			ptl=cvPoint(fldpt.at(i).x,fldpt.at(i).y);
			dmin=dot;
		}
		if(dot>dmax)
		{
			ptr=cvPoint(fldpt.at(i).x,fldpt.at(i).y);
			dmax=dot;
		}
	}
	if(bProl)
	{
		centerx=centery=0;
		xx=yy=xy=0;
		n=0;
		for(int i=0;i<fldpt.size();i++)
		{
			CvPoint pt=fldpt.at(i);
			if((pt.x-ptl.x)*(pt.x-ptl.x)+(pt.y-ptl.y)*(pt.y-ptl.y)<300)
			{
				centerx+=pt.x;
				centery+=pt.y;
				n++;
			}
		}
		centerx/=n;
		centery/=n;
		for(int i=0;i<fldpt.size();i++)
		{
			CvPoint pt=fldpt.at(i);
			if((pt.x-ptl.x)*(pt.x-ptl.x)+(pt.y-ptl.y)*(pt.y-ptl.y)<300)
			{
				xx+=(pt.x-centerx)*(pt.x-centerx);
				yy+=(pt.y-centery)*(pt.y-centery);
				xy+=(pt.x-centerx)*(pt.y-centery);
			}
		}
		vEigen.x=-xy;
		vEigen.y=(xx-yy-sqrt((xx+yy)*(xx+yy)-4*(xx*yy-xy*xy)))/2;
		if(vEigen.x==0&&vEigen.y==0)
		{
			vEigen.x=(yy-xx-sqrt((xx+yy)*(xx+yy)-4*(xx*yy-xy*xy)))/2;
			vEigen.y=-xy;
		}
		if(vEigen.x*(centerx-ptl.x)+vEigen.y*(centery-ptl.y)>0)
		{
			vEigen.x=-vEigen.x;
			vEigen.y=-vEigen.y;
		}
		l=sqrt(vEigen.x*vEigen.x+vEigen.y*vEigen.y);

		if(l>0 && n>4)
		{
			vEigen.x/=l;
			vEigen.y/=l;
		}
		else
		{
			vEigen.x=vEigen.y=0;
		}

		if(mask)
		{
			CvScalar deb=cvGet2D(mask, ptl.y, ptl.x);
			scl=deb.val[0];
		}
		CvPoint dptl = cvPoint(ptl.x+vEigen.x*scl, ptl.y+vEigen.y*scl);

		centerx=centery=0;
		xx=yy=xy=0;
		n=0;
		for(int i=0;i<fldpt.size();i++)
		{
			CvPoint pt=fldpt.at(i);
			if((pt.x-ptr.x)*(pt.x-ptr.x)+(pt.y-ptr.y)*(pt.y-ptr.y)<300)
			{
				centerx+=pt.x;
				centery+=pt.y;
				n++;
			}
		}
		centerx/=n;
		centery/=n;
		for(int i=0;i<fldpt.size();i++)
		{
			CvPoint pt=fldpt.at(i);
			if((pt.x-ptr.x)*(pt.x-ptr.x)+(pt.y-ptr.y)*(pt.y-ptr.y)<300)
			{
				xx+=(pt.x-centerx)*(pt.x-centerx);
				yy+=(pt.y-centery)*(pt.y-centery);
				xy+=(pt.x-centerx)*(pt.y-centery);
			}
		}
		vEigen.x=-xy;
		vEigen.y=(xx-yy-sqrt((xx+yy)*(xx+yy)-4*(xx*yy-xy*xy)))/2;
		if(vEigen.x==0&&vEigen.y==0)
		{
			vEigen.x=(yy-xx-sqrt((xx+yy)*(xx+yy)-4*(xx*yy-xy*xy)))/2;
			vEigen.y=-xy;
		}
		if(vEigen.x*(centerx-ptr.x)+vEigen.y*(centery-ptr.y)>0)
		{
			vEigen.x=-vEigen.x;
			vEigen.y=-vEigen.y;
		}
		l=sqrt(vEigen.x*vEigen.x+vEigen.y*vEigen.y);

		if(l>0 && n>4)
		{
			vEigen.x/=l;
			vEigen.y/=l;
		}
		else
		{
			vEigen.x=vEigen.y=0;
		}

		if(mask)
		{
			scl=cvGet2D(mask, ptr.y, ptr.x).val[0];
		}
		CvPoint dptr = cvPoint(ptr.x+vEigen.x*scl, ptr.y+vEigen.y*scl);
		cvLine(image, ptl, dptl, cvScalar(0,255,255));
		cvLine(image, ptr, dptr, cvScalar(0,255,255));
	//	cvLine(acc, ptl, dptl, cvScalar(255,255,0));
	//	cvLine(acc, ptr, dptr, cvScalar(255,255,0));
		return true;
	}
	for(int i=max(ptl.y-dy,0);i<min(ptl.y+dy,image->height);i++)
	for(int j=max(ptl.x-dx,0);j<min(ptl.x+dx,image->width);j++)
	{
		float dx=j-ptl.x;
		float dy=i-ptl.y;
		float dot=dx*vEigen.x+dy*vEigen.y;
		float dotn=dx*vEigen.y-dy*vEigen.x;
		if(dot<0 && dot>-length && fabs(dotn)<=CONN_WIDTH)
		{
			if(imgcon)
				*(imgcon->imageData+imgcon->widthStep*i+3*j)=255;
			//cvSet2D(image, i,j,cvScalar(255,0,0));
			CvScalar sc=cvGet2D(image, i, j);
			CvScalar scm=cvGet2D(acc, i, j);
			if(sc.val[0]==255 && scm.val[0]==0)
			{
				int deb=fldpt.size();
				cvLine(image, ptl, cvPoint(j,i), cvScalar(255,0,255), 1);
				cvLine(acc, ptl, cvPoint(j,i), cvScalarAll(255), 1);
				bRet = true;
				break;
			}
		}
	}
	for(int i=max(ptr.y-dy,0);i<min(ptr.y+dy,image->height);i++)
	for(int j=max(ptr.x-dx,0);j<min(ptr.x+dx,image->width);j++)
	{
		float dx=j-ptr.x;
		float dy=i-ptr.y;
		float dot=dx*vEigen.x+dy*vEigen.y;
		float dotn=dx*vEigen.y-dy*vEigen.x;
		if(dot>0 && dot<length && fabs(dotn)<=CONN_WIDTH)
		{
			if(imgcon)
				*(imgcon->imageData+imgcon->widthStep*i+3*j+2)=255;
			//cvSet2D(image, i,j,cvScalar(0,0,255));
			CvScalar sc=cvGet2D(image, i, j);
			CvScalar scm=cvGet2D(acc, i, j);
			if(sc.val[0]==255 && scm.val[0]==0)
			{
				cvLine(image, ptr, cvPoint(j,i), cvScalar(255,0,255), 1);
				cvLine(acc, ptr, cvPoint(j,i), cvScalarAll(255), 1);
				bRet = true;
				break;
			}
		}
	}
	return bRet;
}


void floodfill(IplImage* image, int cx, int cy, IplImage* comp)
{
	fldpt.clear();
	cvZero(comp);
	bool bCon=true;
	cvSet2D(comp, cy, cx, cvScalar(255));
	fldpt.push_back(cvPoint(cx,cy));
	vector<CvPoint> tmp1,tmp2;
	vector<CvPoint>* pc[2]={&tmp1,&tmp2};
	int n=0;
	tmp1.push_back(cvPoint(cx,cy));
	while(bCon)
	{
		pc[1-n]->clear();
		while(pc[n]->size()!=0)
		{
			CvPoint pt=pc[n]->back();
			pc[n]->pop_back();
			bool bf=false;
			for(int i=0;i<8;i++)
			{
				CvPoint ptn=cvPoint(pt.x+shift[i][0], pt.y+shift[i][1]);
				if(ptn.x<0)ptn.x=0;
				else if(ptn.x>=image->width)ptn.x=image->width-1;
				if(ptn.y<0)ptn.y=0;
				else if(ptn.y>=image->height)ptn.y=image->height-1;
				CvScalar pix=cvGet2D(image, ptn.y, ptn.x);
				CvScalar pcomp=cvGet2D(comp, ptn.y, ptn.x);
				if(pix.val[0]==255 && pcomp.val[0]==0)
				{
					bf=true;
					cvSet2D(comp, ptn.y, ptn.x, cvScalar(255));
					cvSet2D(image, ptn.y, ptn.x, cvScalar(255,255,0));
					pc[1-n]->push_back(ptn);
				}
			}
		}
		bCon=(pc[1-n]->size()!=0);
		for(int i=0;i<pc[1-n]->size();i++)
		{
			fldpt.push_back(pc[1-n]->at(i));
		}
		n=1-n;
	}
}

void findnodes()
{

}