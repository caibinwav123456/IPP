#include "stdafx.h"
#include "FaceDetector.h"

CFaceDetector* CFaceDetector::s_pObj = NULL;

CFaceDetector::CFaceDetector():m_vOrgLEyel(-0.05,-0.15), 
							   m_vOrgREyer(0.05, -0.15), 
							   m_vOrgLEyer(0.03, -0.15), 
							   m_vOrgREyel(-0.03,-0.15), 
							   m_vOrgMouthl(-0.1,-0.3),
							   m_vOrgMouthr(0.1, -0.3)
{
	m_Image = NULL;
	m_ImgOrg = NULL;
	m_ImgBk = NULL;
	m_imgObj = NULL;
	m_face = NULL;
	m_fgray = NULL;
	m_deBuf = NULL;
	m_ImgAni = NULL;

	m_rcFace = cvRect(0,0,0,0);

	m_bDown = false;
	m_bMove = false;
	m_bFit = false;
	m_bDet = false;
	m_bAnimation = false;
	m_bEnd = false;
	m_bAlign = false;

	m_nPosture[0] = 1;
	m_nPosture[1] = 0;
	m_nPosture[2] = 1;

	m_EyeX = X_INIT;
	m_EyeY = Y_INIT;
	m_EyeW = W_INIT;//, 20, 0.004,0.0;
	m_EyeH = H_INIT;
	m_EyeSlp = SLP_INIT;
	m_EyeCrv = CRV_INIT;

	m_fFacew = 1;
	m_fFaceh = 1;
	m_fFaceoc = 0.2;

	m_vMoveLEyel = m_vOrgLEyel; 
	m_vMoveREyer = m_vOrgREyer; 
	m_vMoveLEyer = m_vOrgLEyer; 
	m_vMoveREyel = m_vOrgREyel; 
	m_vMoveMouthl =m_vOrgMouthl;
	m_vMoveMouthr =m_vOrgMouthr;

	m_fMorph = 0;
	m_fSpeed = 0;

	for(int i=0;i<2;i++)
	{
		m_shiftAlign[i] = 0;
		for(int j=0;j<2;j++)
		{
			m_matAlign[i][j] = 0;
		}
	}
}

CFaceDetector::~CFaceDetector()
{
	cvReleaseImage(&m_Image);
	cvReleaseImage(&m_ImgOrg);
	cvReleaseImage(&m_ImgBk);
	cvReleaseImage(&m_imgObj);
	cvReleaseImage(&m_face);
	cvReleaseImage(&m_fgray);
	cvReleaseImage(&m_deBuf);
	cvReleaseImage(&m_ImgAni);
}

int CFaceDetector::Init(char* strDetectFile)
{
	m_texture = m_face = cvCreateImage(cvSize(DIM_FACE_IMG,DIM_FACE_IMG),IPL_DEPTH_8U,3);
	m_fgray = cvCreateImage(cvGetSize(m_face), IPL_DEPTH_8U, 1);
	m_ImgAni = cvCreateImage(cvGetSize(m_face), IPL_DEPTH_8U, 3);

	cvZero(m_face);
	cvZero(m_fgray);
	cvZero(m_ImgAni);

	RestoreEyeData(Grid);
	return InitFaceDetect(strDetectFile);
}

void CFaceDetector::LoadTemplate(char* strimgTempl, char* strCfgFile)
{
	IplImage* image = cvLoadImage(strimgTempl);
	if(image->width<=1024&&image->height<=768)
	{
		m_ImgBk = image;
	}
	else
	{
		float xscale=1024./image->width;
		float yscale=768./image->height;
		float scale = min(xscale, yscale);
		int w=scale*image->width;
		int h=scale*image->height;
		cvReleaseImage(&m_ImgBk);
		m_ImgBk = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 3);
		cvResize(image, m_ImgBk);
		cvReleaseImage(&image);
	}
	int wImage = m_ImgBk->width;
	int hImage = m_ImgBk->height;
	ViewPort = Rect2D(0,0, wImage, hImage);
	cvReleaseImage(&m_Image);
	cvReleaseImage(&m_deBuf);
	m_Image = cvCreateImage(cvSize(wImage,hImage),IPL_DEPTH_8U,3);
	m_zBuf = m_deBuf = cvCreateImage(cvSize(wImage, hImage), IPL_DEPTH_32F, 1);
	CFile file;
	if(file.Open(strCfgFile,CFile::modeRead))
	{
		file.Read(&rot, sizeof(rot));
		file.Read(&Pos, sizeof(Pos));
		CPoint pt;
		file.Read(&pt, sizeof(pt));
		ViewPort.x=pt.x;
		ViewPort.y=pt.y;
		file.Read(&m_fFacew, sizeof(m_fFacew));
		file.Read(&m_fFaceh, sizeof(m_fFaceh));
		file.Read(&m_fFaceoc, sizeof(m_fFaceoc));
		file.Close();
	}
	CreateDome(&m_mesh,m_fFacew,m_fFaceh,m_fFaceoc,20);
}

void CFaceDetector::SaveTemplate(char* strCfgFile)
{
	CFile file;
	file.Open(strCfgFile,CFile::modeCreate|CFile::modeWrite);
	file.Write(&rot, sizeof(rot));
	file.Write(&Pos, sizeof(Pos));
	CPoint pt(ViewPort.x,ViewPort.y);
	file.Write(&pt, sizeof(pt));
	file.Write(&m_fFacew, sizeof(m_fFacew));
	file.Write(&m_fFaceh, sizeof(m_fFaceh));
	file.Write(&m_fFaceoc, sizeof(m_fFaceoc));
	file.Close();
}

void CFaceDetector::LoadSourceImage(IplImage* image)
{
	cvReleaseImage(&m_ImgOrg);
	cvReleaseImage(&m_imgObj);
	m_ImgOrg = cvCloneImage(image);
	float hs=800./image->width;
	float vs=600./image->height;
	float wobj=min(hs,vs)*image->width;
	float hobj=min(hs,vs)*image->height;
	m_imgObj = cvCreateImage(cvSize(wobj, hobj),IPL_DEPTH_8U,3);
	cvResize(image, m_imgObj);
	//m_bDet = false;
}

void CFaceDetector::ClearBuf()
{
	::ClearBuf();
	m_rcFace.x=m_rcFace.y=m_rcFace.width=m_rcFace.height=0;
	m_bDet = false;
}

void CFaceDetector::DetectFace()
{
	if(m_imgObj)
	{
		m_rcFace = Detect(m_imgObj);
		Process();
	}
	m_bDet = false;
}

void CFaceDetector::FitFace()
{
	if(!m_imgObj)
		return;
	RestoreEyeData(Grid);
	m_bEnd = false;
	for(int i=0;i<EYE_GRID_NX*EYE_GRID_NY;i++)
	{
		while(Fit(&lEGrid[i]) && !m_bEnd);
		while(Fit(&rEGrid[i]) && !m_bEnd);
	}
	for(int i=0;i<MOUTH_GRID_N;i++)
	{
		while(Fit(&MGrid[i]) && !m_bEnd);
	}

	if(!m_bEnd)
	{
		AdjustDetRegion();
		m_bDet = true;
	}
}

IplImage* CFaceDetector::DrawFace()
{
	cvCopyImage(m_ImgBk, m_Image);
	cvZero(m_deBuf);

	if(m_bAlign)
	{
		Vec2 vlEye(X_ALIGN_LEYE, Y_ALIGN_EYE);
		Vec2 vrEye(X_ALIGN_REYE, Y_ALIGN_EYE);
		Vec2 vMouth(X_ALIGN_MOUTH, Y_ALIGN_MOUTH);

		Vec2 vlEyeT((m_rgnlEadj.topleft.x+m_rgnlEadj.topright.x+m_rgnlEadj.bottomleft.x+m_rgnlEadj.bottomright.x) / 4,
			        (m_rgnlEadj.topleft.y+m_rgnlEadj.topright.y+m_rgnlEadj.bottomleft.y+m_rgnlEadj.bottomright.y) / 4);
		Vec2 vrEyeT((m_rgnrEadj.topleft.x+m_rgnrEadj.topright.x+m_rgnrEadj.bottomleft.x+m_rgnrEadj.bottomright.x) / 4,
			        (m_rgnrEadj.topleft.y+m_rgnrEadj.topright.y+m_rgnrEadj.bottomleft.y+m_rgnrEadj.bottomright.y) / 4);
		Vec2 vMouthT((m_rgnMadj.topleft.x+m_rgnMadj.topright.x+m_rgnMadj.bottomleft.x+m_rgnMadj.bottomright.x) / 4,
			         (m_rgnMadj.topleft.y+m_rgnMadj.topright.y+m_rgnMadj.bottomleft.y+m_rgnMadj.bottomright.y) / 4);
		
		vlEye/=DIM_FACE_IMG;
		vrEye/=DIM_FACE_IMG;
		vMouth/=DIM_FACE_IMG;
		vlEyeT/=DIM_FACE_IMG;
		vrEyeT/=DIM_FACE_IMG;
		vMouthT/=DIM_FACE_IMG;

		Mat mat(vlEye.x,vrEye.x,vMouth.x,vlEye.y,vrEye.y,vMouth.y,1,1,1);
		mat=mat.inv();

		Vec3 v1 = Vec3(vlEyeT.x,vrEyeT.x,vMouthT.x)*mat;
		Vec3 v2 = Vec3(vlEyeT.y,vrEyeT.y,vMouthT.y)*mat;

		m_matAlign[0][0] = v1.x;
		m_matAlign[0][1] = v1.y;
		m_matAlign[1][0] = v2.x;
		m_matAlign[1][1] = v2.y;

		m_shiftAlign[0] = v1.z;
		m_shiftAlign[1] = v2.z;
	}

	s_pObj=this;
	m_mesh.SetViewPort(&ViewPort);
	m_mesh.SetPixelShader(PixelShader);
	m_mesh.SetMatrix(rot, -Pos);
	m_mesh.Render(m_Image, &m_zBuf);
	s_pObj=NULL;

	return m_Image;
}

IplImage* CFaceDetector::AnimateFace()
{
	cvZero(m_ImgAni);
	if(m_imgObj == NULL || m_ImgOrg == NULL)
		return m_ImgAni;
	if(m_rcFace.x==0 && m_rcFace.y==0 && m_rcFace.width==0 && m_rcFace.height==0)
		return m_ImgAni;
	if(m_bDet)
	{
		m_fMorph+=m_fSpeed;
		if(m_fSpeed == 0)
		{
			m_fSpeed = 0.03;
		}
		if(m_fSpeed>0&&m_fMorph>0.3)
		{
			m_fSpeed=-0.03;
		}
		if(m_fSpeed<0&&m_fMorph<0)
		{
			m_fSpeed=0.03;
		}

		int posture=0;
		if(m_nPosture[0])posture|=FC_OUTER_EYE;
		if(m_nPosture[1])posture|=FC_INNER_EYE;
		if(m_nPosture[2])posture|=FC_MOUTH_EDGE;
		for(int i=0;i<m_ImgAni->height;i++)
		{
			for(int j=0;j<m_ImgAni->width;j++)
			{
				float x=(float)j/m_ImgAni->width;
				float y=(float)i/m_ImgAni->height;

				Vec2 shift=ExpressionFunc(x, y, m_fMorph, posture);
				Vec2 tex = Vec2(x,y)+shift;
				Vec2 texOrg = tex*Vec2((float)m_rcFace.width/m_imgObj->width,(float)m_rcFace.height/m_imgObj->height)+
					Vec2((float)m_rcFace.x/m_imgObj->width,(float)m_rcFace.y/m_imgObj->height);
				Vec3 smp=Sample(m_ImgOrg, texOrg/*Vec2(x,y)+shift*/);
				uchar* pix=PTR_PIX(*m_ImgAni, j, i);
				*pix = smp.x;
				*(pix+1) = smp.y;
				*(pix+2) = smp.z;
			}
		}
	}
	else
	{
		CvRect rcOrg;
		rcOrg.x = m_rcFace.x*m_ImgOrg->width/m_imgObj->width;
		rcOrg.y = m_rcFace.y*m_ImgOrg->height/m_imgObj->height;
		rcOrg.width = m_rcFace.width*m_ImgOrg->width/m_imgObj->width;
		rcOrg.height = m_rcFace.height*m_ImgOrg->height/m_imgObj->height;

		cvSetImageROI(m_ImgOrg, rcOrg);
		cvResize(m_ImgOrg, m_ImgAni);
		cvResetImageROI(m_ImgOrg);
	}
	return m_ImgAni;
}

void CFaceDetector::SetPosture(int iPosture, int posture)
{
	if(posture>0)posture=1;
	else if(posture == 0)posture=0;
	else posture=-1;
	m_nPosture[iPosture] = posture;

	Vec2* posv[6] = {&m_vMoveLEyel, &m_vMoveREyer, &m_vMoveLEyer, &m_vMoveREyel, &m_vMoveMouthl, &m_vMoveMouthr};
	Vec2* gposv[6] = {&m_vOrgLEyel, &m_vOrgREyer, &m_vOrgLEyer, &m_vOrgREyel, &m_vOrgMouthl, &m_vOrgMouthr};

	for(int i=0;i<6;i++)
	{
		*posv[i] = *gposv[i]*m_nPosture[i/2];
	}
}

int CFaceDetector::GetPosture(int iPosture)
{
	return m_nPosture[iPosture];
}

void CFaceDetector::EnableFaceAlign(bool bEnable)
{
	m_bAlign = bEnable;
}

void CFaceDetector::SetMeshData(float w,float h,float occ)
{
	m_fFacew = w;
	m_fFaceh = h;
	m_fFaceoc = occ;
	CreateDome(&m_mesh, m_fFacew, m_fFaceh, m_fFaceoc, 20);
}

CvSize CFaceDetector::GetDestSize()
{
	if(m_Image)
		return cvGetSize(m_Image);
	else
		return cvSize(0,0);
}

void CFaceDetector::Process()
{
	int cx=m_rcFace.x+m_rcFace.width/2;
	int cy=m_rcFace.y+m_rcFace.height/2;
	m_rcFace.x=cx-m_rcFace.width/2*FACERECT_SCALE;
	m_rcFace.y=cy-m_rcFace.height/2*FACERECT_SCALE;
	m_rcFace.width*=FACERECT_SCALE;
	m_rcFace.height*=FACERECT_SCALE;
	//float hs=DIM_FACE_IMG./rcFace.width;
	//float vs=300./rcFace.height;

	if(m_rcFace.width>0&&m_rcFace.height>0)
	{
		cvSetImageROI(m_imgObj, m_rcFace);
		cvResize(m_imgObj, m_face);
		cvResetImageROI(m_imgObj);
	}
	else
	{
		cvZero(m_face);
	}

	IplImage* integ=cvCreateImage(cvSize(m_fgray->width+1,m_fgray->height+1), IPL_DEPTH_32S, 1);
	IplImage* integ2=cvCreateImage(cvSize(m_fgray->width+1,m_fgray->height+1), IPL_DEPTH_64F, 1);

	cvCvtColor(m_face, m_fgray, CV_RGB2GRAY);
	cvIntegral(m_fgray, integ, integ2);
	for(int i=0;i<m_fgray->height;i++)
	{
		for(int j=0;j<m_fgray->width;j++)
		{
			int ks=10;
			int pi=i-ks;
			int ni=i+ks;
			int pj=j-ks;
			int nj=j+ks;
			if(pi<0)pi=0;
			if(ni>=m_fgray->width)ni=m_fgray->width-1;
			if(pj<0)pj=0;
			if(nj>=m_fgray->height)nj=m_fgray->height-1;
			int a=*(int*)PTR_PIX(*integ, pj, pi);
			int b=*(int*)PTR_PIX(*integ, nj, pi);
			int c=*(int*)PTR_PIX(*integ, pj, ni);
			int d=*(int*)PTR_PIX(*integ, nj, ni);
			double a2=*(double*)PTR_PIX(*integ2, pj, pi);
			double b2=*(double*)PTR_PIX(*integ2, nj, pi);
			double c2=*(double*)PTR_PIX(*integ2, pj, ni);
			double d2=*(double*)PTR_PIX(*integ2, nj, ni);
			int output=0;
			if((ni-pi)*(nj-pj)!=0)
			{
				output = sqrt((d2-b2-c2+a2)/((ni-pi)*(nj-pj))-((float)(d-b-c+a)/((ni-pi)*(nj-pj)))*((float)(d-b-c+a)/((ni-pi)*(nj-pj))));
			}
			*(uchar*)PTR_PIX(*m_fgray, j, i)=output;
		}
	}
	cvReleaseImage(&integ);
	cvReleaseImage(&integ2);
}

void CFaceDetector::RestoreEyeData(InitType type)
{
	switch(type)
	{
	case DefaultInit:
		m_EyeW = W_INIT;//, 20, 0.004,0.0;
		m_EyeH = H_INIT;
		m_EyeSlp = SLP_INIT;
		m_EyeCrv = CRV_INIT;
		break;
	case LeftEye:
		m_EyeX=X_INIT_LEYE;
		m_EyeY=Y_INIT_EYE;
		m_EyeW=W_INIT;
		m_EyeH=H_INIT;
		m_EyeCrv=CRV_INIT;
		m_EyeSlp=SLP_INIT;
		break;
	case RightEye:
		m_EyeX=X_INIT_REYE;
		m_EyeY=Y_INIT_EYE;
		m_EyeW=W_INIT;
		m_EyeH=H_INIT;
		m_EyeCrv=CRV_INIT;
		m_EyeSlp=SLP_INIT;
		break;
	case Mouth:
		m_EyeX=X_INIT_MOUTH;
		m_EyeY=Y_INIT_MOUTH;
		m_EyeW=W_INIT_MOUTH;
		m_EyeH=H_INIT_MOUTH;
		m_EyeCrv=CRV_INIT;
		m_EyeSlp=SLP_INIT;
		break;
	case Grid:
		for(int i=0;i<EYE_GRID_NX;i++)
		{
			for(int j=0;j<EYE_GRID_NY;j++)
			{
				lEGrid[i*EYE_GRID_NY+j].x=X_INIT_LEYE-EYE_GRID_X/2+EYE_GRID_X*i/max(1,(EYE_GRID_NX-1));
				lEGrid[i*EYE_GRID_NY+j].y=Y_INIT_EYE-EYE_GRID_Y/2+EYE_GRID_Y*j/max(1,(EYE_GRID_NY-1));
				rEGrid[i*EYE_GRID_NY+j].x=X_INIT_REYE-EYE_GRID_X/2+EYE_GRID_X*i/max(1,(EYE_GRID_NX-1));
				rEGrid[i*EYE_GRID_NY+j].y=Y_INIT_EYE-EYE_GRID_Y/2+EYE_GRID_Y*j/max(1,(EYE_GRID_NY-1));

				lEGrid[i*EYE_GRID_NY+j].w=W_INIT;
				lEGrid[i*EYE_GRID_NY+j].h=H_INIT;
				rEGrid[i*EYE_GRID_NY+j].w=W_INIT;
				rEGrid[i*EYE_GRID_NY+j].h=H_INIT;

				lEGrid[i*EYE_GRID_NY+j].c=CRV_INIT;
				lEGrid[i*EYE_GRID_NY+j].s=SLP_INIT;
				rEGrid[i*EYE_GRID_NY+j].c=CRV_INIT;
				rEGrid[i*EYE_GRID_NY+j].s=SLP_INIT;
			}
		}
		int im=0;
		for(int i=0;i<MOUTH_GRID_NX;i++)
		{
			for(int j=0;j<MOUTH_GRID_NY;j++)
			{
#ifdef MOUTH_CROSS
				if((i==0||i==2)&&(j==0||j==2))
					continue;
#endif
				MGrid[/*i*MOUTH_GRID_NY+j*/im].x=X_INIT_MOUTH-MOUTH_GRID_X/2+MOUTH_GRID_X*i/max(1,(MOUTH_GRID_NX-1));
				MGrid[/*i*MOUTH_GRID_NY+j*/im].y=Y_INIT_MOUTH-MOUTH_GRID_Y/2+MOUTH_GRID_Y*j/max(1,(MOUTH_GRID_NY-1));

				MGrid[/*i*MOUTH_GRID_NY+j*/im].w=W_INIT_MOUTH;
				MGrid[/*i*MOUTH_GRID_NY+j*/im].h=H_INIT_MOUTH;

				MGrid[/*i*MOUTH_GRID_NY+j*/im].c=CRV_INIT;
				MGrid[/*i*MOUTH_GRID_NY+j*/im].s=SLP_INIT;
				im++;
			}
		}
		m_bDet = false;
		break;
	}
}

bool CFaceDetector::Fit()
{
	float cval=FitEyeLid(m_fgray, cvPoint2D32f(m_EyeX,m_EyeY),m_EyeW,m_EyeH,m_EyeCrv,m_EyeSlp);
	float fitval[12];
	int sign[2]={-1,1};
	float X,Y,W,H,Crv,Slp;
	float* vparam[6]={&m_EyeX,&m_EyeY,&m_EyeW,&m_EyeH,&m_EyeCrv,&m_EyeSlp};
	float* vp[6]={&X,&Y,&W,&H,&Crv,&Slp};
	float paramlist[6]={POS_SHIFT, POS_SHIFT, SZ_SHIFT, SZ_SHIFT, CRV_SHIFT, SLP_SHIFT};
	int poslist[6]={0,1,2,3,4,5};
	for(int i=0;i<12;i++)
	{
		int s=sign[i%2];
		int nparam=i/2;
		for(int j=0;j<6;j++)
		{
			*vp[j]=*vparam[j];
		}
		*vp[poslist[nparam]]+=s*paramlist[nparam];
		if(Crv>MAX_CRV)Crv=MAX_CRV;
		else if(Crv<-MAX_CRV)Crv=-MAX_CRV;
		fitval[i]=FitEyeLid(m_fgray, cvPoint2D32f(X,Y),W,H,Crv,Slp);
	}
	float vmax=0;
	int index=-1;
	for(int i=0;i<12;i++)
	{
		if(vmax<fitval[i])
		{
			vmax = fitval[i];
			index = i;
		}
	}

	if(vmax<=cval)
	{
		return false;
	}

	if(index!=-1)
	{
		int s=sign[index%2];
		int pos=index/2;
		*vparam[poslist[pos]]+=s*paramlist[pos];
		if(m_EyeCrv>MAX_CRV)m_EyeCrv=MAX_CRV;
		else if(m_EyeCrv<-MAX_CRV)m_EyeCrv=-MAX_CRV;
		return true;
	}

	return false;
}

bool CFaceDetector::Fit(FaceData* fitdata)
{
	float cval=FitEyeLid(m_fgray, cvPoint2D32f(fitdata->x,fitdata->y),fitdata->w,fitdata->h,fitdata->c,fitdata->s);
	float fitval[12];
	int sign[2]={-1,1};
	float X,Y,W,H,Crv,Slp;
	float* vparam[6]={&(fitdata->x),&(fitdata->y),&(fitdata->w),&(fitdata->h),&(fitdata->c),&(fitdata->s)};
	float* vp[6]={&X,&Y,&W,&H,&Crv,&Slp};
	float paramlist[6]={POS_SHIFT, POS_SHIFT, SZ_SHIFT, SZ_SHIFT, CRV_SHIFT, SLP_SHIFT};
	int poslist[6]={0,1,2,3,4,5};
	for(int i=0;i<12;i++)
	{
		int s=sign[i%2];
		int nparam=i/2;
		for(int j=0;j<6;j++)
		{
			*vp[j]=*vparam[j];
		}
		*vp[poslist[nparam]]+=s*paramlist[nparam];
		if(Crv>MAX_CRV)Crv=MAX_CRV;
		else if(Crv<-MAX_CRV)Crv=-MAX_CRV;
		fitval[i]=FitEyeLid(m_fgray, cvPoint2D32f(X,Y),W,H,Crv,Slp);
	}
	float vmax=0;
	int index=-1;
	for(int i=0;i<12;i++)
	{
		if(vmax<fitval[i])
		{
			vmax = fitval[i];
			index = i;
		}
	}

	if(vmax<=cval)
	{
		return false;
	}

	if(index!=-1)
	{
		int s=sign[index%2];
		int pos=index/2;
		*vparam[poslist[pos]]+=s*paramlist[pos];
		if(fitdata->c>MAX_CRV)fitdata->c=MAX_CRV;
		else if(fitdata->c<-MAX_CRV)fitdata->c=-MAX_CRV;
		return true;
	}

	return false;
}

FaceRegion CFaceDetector::GetDetectedRegion(DetectType type)
{
	float xmin=DIM_FACE_IMG,xmax=0;
	float yminl=DIM_FACE_IMG,ymaxl=0;
	float yminr=DIM_FACE_IMG,ymaxr=0;

	int nxmin=0;
	int nxmax=0;
	int nyminl=0;
	int nyminr=0;
	int nymaxl=0;
	int nymaxr=0;
	float cx,cy;
	float w,h;
	FaceData* grid=NULL;
	int dim=0;
	switch(type)
	{
	case Det_LeftEye:
		grid = (FaceData*)lEGrid;
		dim = EYE_GRID_NX*EYE_GRID_NY;
		cx = X_INIT_LEYE;
		cy = Y_INIT_EYE;
		w = DEFAULT_EYE_WIDTH;
		h = DEFAULT_EYE_HEIGHT;

		break;
	case Det_RightEye:
		grid = (FaceData*)rEGrid;
		dim = EYE_GRID_NX*EYE_GRID_NY;
		cx = X_INIT_REYE;
		cy = Y_INIT_EYE;
		w = DEFAULT_EYE_WIDTH;
		h = DEFAULT_EYE_HEIGHT;
		break;
	case Det_Mouth:
		grid = (FaceData*)MGrid;
		dim = MOUTH_GRID_N;
		cx = X_INIT_MOUTH;
		cy = Y_INIT_MOUTH;
		w = DEFAULT_MOUTH_WIDTH;
		h = DEFAULT_MOUTH_HEIGHT;
		break;
	}
	if(grid == NULL)
		return FaceRegion();
	bool* blist=new bool[dim];
	Classify(grid, blist, dim);
	for(int i=0;i<dim;i++)
	{
		FaceData* data=grid+i;
		if(data->x>cx-w/2&&data->x<cx+w/2&&data->y>cy-h/2&&data->y<cy+h/2)
		{
			blist[i]=true;
		}
	}
	for(int i=0;i<dim;i++)
	{
		if(!blist[i])continue;
		FaceData facedata=grid[i];
		if(facedata.x-facedata.w/2<xmin)
			xmin=facedata.x-facedata.w/2;
		if(facedata.x+facedata.w/2>xmax)
			xmax=facedata.x+facedata.w/2;
		if(facedata.y-facedata.h/2-facedata.s*facedata.w/2<yminl)
			yminl=facedata.y-facedata.h/2-facedata.s*facedata.w/2;
		if(facedata.y+facedata.h/2-facedata.s*facedata.w/2>ymaxl)
			ymaxl=facedata.y+facedata.h/2-facedata.s*facedata.w/2;
		if(facedata.y-facedata.h/2+facedata.s*facedata.w/2<yminr)
			yminr=facedata.y-facedata.h/2+facedata.s*facedata.w/2;
		if(facedata.y+facedata.h/2+facedata.s*facedata.w/2>ymaxr)
			ymaxr=facedata.y+facedata.h/2+facedata.s*facedata.w/2;
	}

	delete[] blist;
	FaceRegion detectregion;
	detectregion.topleft = cvPoint2D32f(xmin,yminl);
	detectregion.topright = cvPoint2D32f(xmax,yminr);
	detectregion.bottomright = cvPoint2D32f(xmax,ymaxr);
	detectregion.bottomleft = cvPoint2D32f(xmin,ymaxl);
	if(detectregion.topright.x-detectregion.topleft.x<MIN_EYE_WIDTH)
	{
		float width=detectregion.topright.x-detectregion.topleft.x;
		detectregion.topleft.x-=(MIN_EYE_WIDTH-width)/2;
		detectregion.topright.x+=(MIN_EYE_WIDTH-width)/2;
	}
	if(detectregion.bottomright.x-detectregion.bottomleft.x<MIN_EYE_WIDTH)
	{
		float width=detectregion.bottomright.x-detectregion.bottomleft.x;
		detectregion.bottomleft.x-=(MIN_EYE_WIDTH-width)/2;
		detectregion.bottomright.x+=(MIN_EYE_WIDTH-width)/2;
	}
	if(detectregion.bottomleft.y-detectregion.topleft.y<MIN_EYE_HEIGHT)
	{
		float height=detectregion.bottomleft.y-detectregion.topleft.y;
		detectregion.topleft.y-=(MIN_EYE_HEIGHT-height)/2;
		detectregion.bottomleft.y+=(MIN_EYE_HEIGHT-height)/2;
	}
	if(detectregion.bottomright.y-detectregion.topright.y<MIN_EYE_HEIGHT)
	{
		float height=detectregion.bottomright.y-detectregion.topright.y;
		detectregion.topright.y-=(MIN_EYE_HEIGHT-height)/2;
		detectregion.bottomright.y+=(MIN_EYE_HEIGHT-height)/2;
	}
	return detectregion;
}

void CFaceDetector::AdjustDetRegion()
{
	m_rgnlEadj = m_rgnlEye = GetDetectedRegion(Det_LeftEye);
	m_rgnrEadj = m_rgnrEye = GetDetectedRegion(Det_RightEye);
	m_rgnMadj = m_rgnMouth = GetDetectedRegion(Det_Mouth);

	CvPoint2D32f* left[2][2] = {{&m_rgnlEadj.topleft, &m_rgnlEadj.bottomleft}, {&m_rgnrEadj.topleft, &m_rgnrEadj.bottomleft}};
	CvPoint2D32f* right[2][2] = {{&m_rgnlEadj.topright, &m_rgnlEadj.bottomright}, {&m_rgnrEadj.topright, &m_rgnrEadj.bottomright}};
	for(int i=0;i<2;i++)
	{
		float CenterlE = (left[0][i]->y+right[0][i]->y)/2;
		float CenterrE = (left[1][i]->y+right[1][i]->y)/2;
		float offsetE = CenterlE-CenterrE;
		float adjE=0;
		if(offsetE<-MAX_EYE_OFFSET)
		{
			adjE = (-MAX_EYE_OFFSET-offsetE)/2;
		}
		else if(offsetE>MAX_EYE_OFFSET)
		{
			adjE = (MAX_EYE_OFFSET-offsetE)/2;
		}
		left[0][i]->y+=adjE;
		right[0][i]->y+=adjE;
		left[1][i]->y-=adjE;
		right[1][i]->y-=adjE;
	}

	float lhM = m_rgnMadj.bottomleft.y-m_rgnMadj.topleft.y;
	float rhM = m_rgnMadj.bottomright.y-m_rgnMadj.topright.y;
	float offsetM=lhM-rhM;
	float adjM=0;
	if(offsetM < -MAX_MOUTH_TRPZ)
	{
		adjM = (-MAX_MOUTH_TRPZ-offsetM)/2;
	}
	else if(offsetM > MAX_MOUTH_TRPZ)
	{
		adjM = (MAX_MOUTH_TRPZ-offsetM)/2;
	}
	m_rgnMadj.topleft.y-=adjM;
	m_rgnMadj.bottomleft.y+=adjM;
	m_rgnMadj.topright.y+=adjM;
	m_rgnMadj.bottomright.y-=adjM;
}

void CFaceDetector::Classify(float* val,bool* cls, int n)
{
	for(int i=0;i<n;i++)
	{
		cls[i]=true;
	}
	bool bChanged=false;

	do 
	{
		bChanged=false;
		float sum=0,sqsum=0;
		int cnt=0;
		for(int i=0;i<n;i++)
		{
			if(cls[i])
			{
				sum+=val[i];
				sqsum+=val[i]*val[i];
				cnt++;
			}
		}
		float mean=sum/cnt;
		float stddev=sqrt(sqsum/cnt-mean*mean);
		for(int i=0;i<n;i++)
		{
			if(fabs(val[i]-mean)>max(CLASS_THRESH*stddev,10))
			{
				if(cls[i])
				{
					cls[i]=false;
					bChanged=true;
				}
			}
			else
			{
				if(!cls[i])
				{
					cls[i]=true;
					bChanged=true;
				}
			}
		}
	}
	while(bChanged);
}

void CFaceDetector::Classify(FaceData* val, bool* cls, int n)
{
	for(int i=0;i<n;i++)
	{
		cls[i]=true;
	}
	bool bChanged=false;

	do 
	{
		bChanged=false;
		CvPoint2D32f sum=cvPoint2D32f(0,0);
		float sqsum=0;
		int cnt=0;
		for(int i=0;i<n;i++)
		{
			if(cls[i])
			{
				sum.x+=val[i].x;
				sum.y+=val[i].y;
				sqsum+=val[i].x*val[i].x+val[i].y*val[i].y;
				cnt++;
			}
		}
		float meanx=sum.x/cnt;
		float meany=sum.y/cnt;
		float stddev=sqrt(sqsum/cnt-meanx*meanx-meany*meany);
		for(int i=0;i<n;i++)
		{
			if(sqrt(pow(val[i].x-meanx,2)+pow(val[i].y-meany,2))>max(CLASS_THRESH*stddev,10))
			{
				if(cls[i])
				{
					cls[i]=false;
					bChanged=true;
				}
			}
			else
			{
				if(!cls[i])
				{
					cls[i]=true;
					bChanged=true;
				}
			}
		}
	}
	while(bChanged);
}

Vec2 CFaceDetector::ExpressionFunc(float x, float y,float ext, int corner)
{
	Vec2 shift(0,0);
	if(corner&FC_OUTER_EYE)
	{
		CvPoint2D32f lc = cvPoint2D32f((m_rgnlEadj.topleft.x+m_rgnlEadj.bottomleft.x)/2/DIM_FACE_IMG,
			(m_rgnlEadj.topleft.y+m_rgnlEadj.bottomleft.y)/2/DIM_FACE_IMG);
		shift-=m_vMoveLEyel*ext*exp(-(sqr((x-lc.x)/RANGE_OUTER_EYE)+sqr((y-lc.y)/RANGE_OUTER_EYE/2)));

		CvPoint2D32f rc = cvPoint2D32f((m_rgnrEadj.topright.x+m_rgnrEadj.bottomright.x)/2/DIM_FACE_IMG,
			(m_rgnrEadj.topright.y+m_rgnrEadj.bottomright.y)/2/DIM_FACE_IMG);
		shift-=m_vMoveREyer*ext*exp(-(sqr((x-rc.x)/RANGE_OUTER_EYE)+sqr((y-rc.y)/RANGE_OUTER_EYE/2)));
	}
	if(corner&FC_INNER_EYE)
	{
		CvPoint2D32f lc = cvPoint2D32f((m_rgnlEadj.topright.x+m_rgnlEadj.bottomright.x)/2/DIM_FACE_IMG,
			(m_rgnlEadj.topright.y+m_rgnlEadj.bottomright.y)/2/DIM_FACE_IMG);
		shift-=m_vMoveLEyer*ext*exp(-(sqr((x-lc.x)/RANGE_INNER_EYE)+sqr((y-lc.y)/RANGE_INNER_EYE/2)));

		CvPoint2D32f rc = cvPoint2D32f((m_rgnrEadj.topleft.x+m_rgnrEadj.bottomleft.x)/2/DIM_FACE_IMG,
			(m_rgnrEadj.topleft.y+m_rgnrEadj.bottomleft.y)/2/DIM_FACE_IMG);
		shift-=m_vMoveREyel*ext*exp(-(sqr((x-rc.x)/RANGE_INNER_EYE)+sqr((y-rc.y)/RANGE_INNER_EYE/2)));
	}
	if(corner&FC_MOUTH_EDGE)
	{
		CvPoint2D32f lc = cvPoint2D32f((m_rgnMadj.topleft.x+m_rgnMadj.bottomleft.x)/2/DIM_FACE_IMG-RANGE_MOUTH_EDGE/2,
			(m_rgnMadj.topleft.y+m_rgnMadj.bottomleft.y)/2/DIM_FACE_IMG);
		shift-=m_vMoveMouthl*ext*exp(-(sqr((x-lc.x)/RANGE_MOUTH_EDGE)+sqr((y-lc.y)/RANGE_MOUTH_EDGE/2)));
		CvPoint2D32f rc = cvPoint2D32f((m_rgnMadj.topright.x+m_rgnMadj.bottomright.x)/2/DIM_FACE_IMG+RANGE_MOUTH_EDGE/2,
			(m_rgnMadj.topright.y+m_rgnMadj.bottomright.y)/2/DIM_FACE_IMG);
		shift-=m_vMoveMouthr*ext*exp(-(sqr((x-rc.x)/RANGE_MOUTH_EDGE)+sqr((y-rc.y)/RANGE_MOUTH_EDGE/2)));
	}
	return shift;
}

float CFaceDetector::HoleFunc(float x, float y, FaceRegion region)
{
	float cxl=(region.topleft.x+region.bottomleft.x)/2;
	float cxr=(region.topright.x+region.bottomright.x)/2;
	float cyl=(region.topleft.y+region.bottomleft.y)/2;
	float cyr=(region.topright.y+region.bottomright.y)/2;
	float hl=(region.bottomleft.y-region.topleft.y)/2;
	float hr=(region.bottomright.y-region.topright.y)/2;
	float cx=(cxl+cxr)/2;
	float cy=(cyl+cyr)/2;
	float w=(cxr-cxl)/2;
	float h=(hl+hr)/2;

	cx/=DIM_FACE_IMG;
	cy/=DIM_FACE_IMG;
	w/=DIM_FACE_IMG;
	h/=DIM_FACE_IMG;

	float slope=(cyl-cyr)/(cxl-cxr);
	float expt=-(pow((x-cx)/w*0.8,2)+pow(((y-cy)-slope*(x-cx))/h*0.8,2));
	return exp(expt);
}

float CFaceDetector::FitEyeLid(IplImage* image, CvPoint2D32f center, float w, float h, float curvation, float slope)
{
	float acc=0, accp=0, acco=0;
	int accn=0;
	for(int i=floor(-w/2+center.x);i<=ceil(w/2+center.x);i++)
	{
		float cy=(i-center.x)*slope+center.y;
		for(int j=floor(-h/2+cy+min(0,curvation*w*w/4));j<=ceil(h/2+cy+max(0,curvation*w*w/4));j++)
		{
			CvPoint ptDraw=cvPoint(i, j);
			if(ptDraw.x>=0&&ptDraw.x<image->width&&ptDraw.y>=0&&ptDraw.y<image->height)
			{
				float x=i-center.x;
				float y=j-cy-curvation*x*x;
				float outpix=exp(-(x*x/w/w*16+y*y/h/h*16));
				if(outpix>4./255)
				{
					accp+=*(uchar*)PTR_PIX(*image, ptDraw.x, ptDraw.y)*outpix;
					acc+=pow((float)*(uchar*)PTR_PIX(*image, ptDraw.x, ptDraw.y),2.0f);
					acco+=pow(outpix, 2.0f);
					accn++;
				}
			}
		}
	}
	float result=accp*accp/acc/acco;//accn;
	float rela=acc/accn;
	float rela2=1./(1+exp(-0.1*(rela-100)));
	float rela3=result*rela2;
	return rela3;
}

void CFaceDetector::PixelShader(Point2D pos, Vec3* color, Vec2* tex, float* userdata, Vec3* colorout, float* depth)
{
	Vec2 stex=*tex-Vec2(0.5,0.5);
	uchar* pix=PTR_PIX(*(s_pObj->m_ImgBk), pos.x, pos.y);
	Vec3 cbk(*pix,*(pix+1),*(pix+2));
	cbk/=255;
	float radius=sqrt(stex.x*stex.x+stex.y*stex.y);
	float alpha=FrameFunc(0.5+radius, 0.2);
	Vec2 text = *tex;
	if(s_pObj->m_bDet)
	{
		float alphahole=0.5;
		alphahole+=HoleFunc(tex->x,tex->y,s_pObj->m_rgnlEadj);
		alphahole+=HoleFunc(tex->x,tex->y,s_pObj->m_rgnrEadj);
		alphahole+=HoleFunc(tex->x,tex->y,s_pObj->m_rgnMadj);
		if(alphahole>1)alphahole=1;
		alpha*=alphahole;

		if(s_pObj->m_bAlign)
		{
			text.x=tex->x*s_pObj->m_matAlign[0][0]+tex->y*s_pObj->m_matAlign[0][1]+s_pObj->m_shiftAlign[0];
			text.y=tex->y*s_pObj->m_matAlign[1][0]+tex->y*s_pObj->m_matAlign[1][1]+s_pObj->m_shiftAlign[1];
		}
	}
	Vec3 smp(0,0,0);
	if(s_pObj->m_imgObj&&s_pObj->m_ImgOrg)
	{
		Vec2 texOrg = (text*Vec2((float)s_pObj->m_rcFace.width/s_pObj->m_imgObj->width,(float)s_pObj->m_rcFace.height/s_pObj->m_imgObj->height)+
			Vec2((float)s_pObj->m_rcFace.x/s_pObj->m_imgObj->width, (float)s_pObj->m_rcFace.y/s_pObj->m_imgObj->height));

		smp=Sample(s_pObj->m_ImgOrg, texOrg)/255**color;
	}

	*colorout=smp*alpha+cbk*(1-alpha);
}

float CFaceDetector::FrameFunc(float x, float edge)
{
	if(x>=edge && x<1-edge)
	{
		return 1;
	}
	else
	{
		if(x<edge)
		{
			x-=edge;
		}
		else
		{
			x-=1-edge;
		}
		return (cos(x/edge*PI)+1)/2;
	}
}