#include "stdafx.h"
#include "ClassifyMain.h"
#include <assert.h>
#include <algorithm>

#define CONFIG_FILE_ALPHA "cls_alpha"
#define CONFIG_FILE_NUM   "cls_num"
#define CONFIG_FILE_SUBSTITUTE "substitution"
#define FIRST_REPLACE     "First"
#define TEXT_FILE_SUFFIX  ".txt"
#define INFO_FILE_SUFFIX  "_info.txt"
#define CLASS_TITLE       "Title"
#define CLASS_UNKNOWN     "Unkonwn"
#define CLASS_JUNK        "Junk"
#define CLASS_CAREER      "Career"
#define CLASS_MAIL        "Mail"
#define CLASS_URL         "Url"

#define ROUGH_CLASS_ALPHA 0
#define ROUGH_CLASS_NUM   1
#define ROUGH_CLASS_JUNK  2

struct RepPair
{
	char ch[10];
	char rep[10];
};

const unsigned char kExtraCharInTitle[]={' ','(',')'};

bool GetCharEnable(char c)
{
	static bool b=true;
	static bool TitleCharTable[256];
	if(b)
	{
		b=false;
		for(int i=0;i<256;i++)
			TitleCharTable[i]=false;
		for(unsigned char c='A';c<='Z';c++)
			TitleCharTable[c]=true;
		for(unsigned char c='a';c<='z';c++)
			TitleCharTable[c]=true;
		for(unsigned char c='0';c<='9';c++)
			TitleCharTable[c]=true;
		for(int i=0;i<sizeof(kExtraCharInTitle)/sizeof(char);i++)
			TitleCharTable[kExtraCharInTitle[i]]=true;
	}
	return TitleCharTable[(unsigned char)c];
}

ClassifyInterface::ClassifyInterface() : m_bExcludeTag(true), m_curItem(NULL)
{
	
}

ClassifyInterface::~ClassifyInterface()
{

}

void ClassifyInterface::SetBasePath(const char* path)
{
	m_strBase=path;
	if(m_strBase.size()>0&&m_strBase[m_strBase.size()-1]!='/'
		&&m_strBase[m_strBase.size()-1]!='\\')
		m_strBase+="//";
}

void ClassifyInterface::SetExcludeTag(bool b)
{
	m_bExcludeTag=b;
}

bool ClassifyInterface::LoadConfig()
{
	string strConfigAlpha=m_strBase+CONFIG_FILE_ALPHA;
	string strConfigNum=m_strBase+CONFIG_FILE_NUM;
	string strConfigReplace=m_strBase+CONFIG_FILE_SUBSTITUTE;
	return m_clsalpha.Load(strConfigAlpha.c_str()) && m_clsnum.Load(strConfigNum.c_str())
		&& LoadReplaceTable(strConfigReplace.c_str());
}

void ClassifyInterface::MasterClassify(const char* textbase, ClassifyRes& cres)
{
	cres.records.clear();
	cres.sorted_records.clear();
	LoadText(textbase, cres);
	m_clsalpha.RestoreAdapters();
	m_clsnum.RestoreAdapters();
	for(int pass=0;pass<2;pass++)
	{
		if(pass==1)
		{
			m_clsalpha.PreAdapters();
			m_clsnum.PreAdapters();
		}
		for(int i=0;i<(int)cres.records.size();i++)
		{
			TextRecord& rd=cres.records[i];
			char* txt=const_cast<char*>(rd.str.c_str());
			int rclass;
			if(pass==0)
			{
				char buf[max_line];
				ReplaceCharacters(txt, buf, string(FIRST_REPLACE));
				rd.str=buf;
				txt=const_cast<char*>(rd.str.c_str());
				rclass=RoughClassify(txt);
				rd.rclass=rclass;
			}
			else
				rclass=rd.rclass;
			if(pass==0)
				rd.cls_data.mode=CLSMODE_ADAPT;
			else
				rd.cls_data.mode=CLSMODE_ADAPT_CLASSIFY;
			if(rclass==ROUGH_CLASS_NUM)
			{
				char* attrib=m_clsnum.GetAttribute(txt, &rd.cls_data);
				if(pass==0)
					rd.str=rd.cls_data.corr;
				else
					ProcessNumRecords(rd.str);
				rd.tclass=(attrib?attrib:"");
			}
			else if(rclass==ROUGH_CLASS_ALPHA)
			{
				char* attrib=m_clsalpha.GetAttribute(txt, &rd.cls_data);
				if(pass==0)
					rd.str=rd.cls_data.corr;
				rd.tclass=(attrib?attrib:"");
			}
			else if(rclass==ROUGH_CLASS_JUNK)
			{
				rd.tclass=CLASS_JUNK;
			}
		}
	}
	PostClassify(cres);
	ComputeStats(cres);
	SortResult(cres);
	RefineCharacters(cres);
	int idUnkown=-1,idCareer=-1,idTitle=-1;
	for(int i=0;i<(int)cres.sorted_records.size();i++)
	{
		if(cres.sorted_records[i].tclass==CLASS_UNKNOWN)
			idUnkown=i;
		else if(cres.sorted_records[i].tclass==CLASS_CAREER)
			idCareer=i;
		else if(cres.sorted_records[i].tclass==CLASS_TITLE)
			idTitle=i;
	}
	if(idTitle>=0)
	{
		SortTitle(cres.sorted_records[idTitle].records,
			idCareer>=0?&cres.sorted_records[idCareer].records:NULL, cres);
	}
	else if(idUnkown>=0)
	{
		TextRecord* rd=SiftTitle(cres.sorted_records[idUnkown].records, 
			idCareer>=0?&cres.sorted_records[idCareer].records:NULL, cres);
		if(rd!=NULL)
		{
			ClassRecord cd;
			cd.tclass=CLASS_TITLE;
			cd.records.push_back(rd);
			cres.sorted_records.push_back(cd);
		}
	}
}

const float kNumThresh=0.5f;
const float kNumThreshAbs=5;
const float kPuncThresh=0.5f;
int ClassifyInterface::RoughClassify(const char* text)
{
	int cc=0,nc=0,nn=0,nsp=0,npunc=0;
	for(char* p=const_cast<char*>(text);!IsLineEnd(p);p=NextUtf8(p))
	{
		cc++;
		if((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z'))
			nc++;
		else if(*p>='0'&&*p<='9')
			nn++;
		else if(IsSpace(p))
			nsp++;
		else
			npunc++;
	}
	if(nsp+npunc==cc||npunc>=kPuncThresh*cc)
		return ROUGH_CLASS_JUNK;
	else if(nn>kNumThresh*(nc+nn)&&nn>kNumThreshAbs)
		return ROUGH_CLASS_NUM;
	else
		return ROUGH_CLASS_ALPHA;
}

#define is_alpha(p) ((*(p)>='a'&&*(p)<='z')||(*(p)>='A'&&*(p)<='Z'))
#define is_tag_punct(p) (*(p)==':'||*(p)==';')
#define probably_is_alpha(p) (is_alpha(p) && *(p)!='o' && *(p)!='O')
void ClassifyInterface::ProcessNumRecords(string& text)
{
	const int max_buf=4096;
	char buf[max_buf];
	char* pbuf=buf;
	char* last=NULL;
	for(char* p=(char*)text.c_str();!IsLineEnd(p);p=NextUtf8(p))
	{
		int len=Utf8Len(p);
		char* next=p+len;
		if(IsLineEnd(next))
			next=NULL;
		if(*p=='o' || *p=='O')
		{
			bool probably_num=((last==NULL||!probably_is_alpha(last))
				&&(next==NULL||!probably_is_alpha(next)));
			if(probably_num)
			{
				*pbuf='0';
				pbuf++;
				last=p;
				continue;
			}
		}
		else if(*p=='_')
		{
			*pbuf='-';
			pbuf++;
			last=p;
			continue;
		}
		if((is_alpha(p)||is_tag_punct(p))&&m_bExcludeTag)
		{
			//We want to exclude tags(probably alpha letters), so do nothing;
		}
		else
		{
			//Do the normal thing;
			memcpy(pbuf, p, len);
			pbuf+=len;
		}
		last=p;
	}
	//End the string.
	*pbuf=0;
	//Exclude the empty brackets
	char* last_left_bracket=NULL;
	pbuf=buf;
	for(char* p=buf;!IsLineEnd(p);p=NextUtf8(p))
	{
		if(last_left_bracket&&*p==')')
		{
			pbuf-=p-last_left_bracket;
			last_left_bracket=NULL;
			continue;
		}
		if(last_left_bracket&&!IsSpace(p))
			last_left_bracket=NULL;
		if(*p=='(')
			last_left_bracket=p;
		int len=Utf8Len(p);
		if(pbuf!=p)
			memcpy(pbuf, p, len);
		pbuf+=len;
	}
	//End the string.
	*pbuf=0;

	text=buf;
}

const int kTitleCharCountThreshold=4;
void ClassifyInterface::PostClassify(ClassifyRes& cres)
{
	const float validate_threshold=0.66f;
	for(int i=0;i<(int)cres.records.size();i++)
	{
		if(cres.records[i].tclass=="")
		{
			GroupRecord* group=cres.records[i].group;
			map<string, int> measure;
			for(int j=0;j<(int)group->records.size();j++)
			{
				string& tclass=group->records[j]->tclass;
				if(tclass=="")
					continue;
				if(measure.find(tclass)!=measure.end())
					measure[tclass]++;
				else
					measure[tclass]=1;
			}
			int max_count=0;
			string max_class;
			for(map<string, int>::iterator it=measure.begin();it!=measure.end();it++)
			{
				if((*it).second>max_count)
				{
					max_count=(*it).second;
					max_class=(*it).first;
				}
			}
			if(max_count>0&&max_count>=group->records.size()*validate_threshold)
				cres.records[i].tclass=max_class;
		}
		else if(cres.records[i].tclass==CLASS_TITLE)
		{
			if(cres.records[i].str.size()<kTitleCharCountThreshold)
				cres.records[i].tclass=CLASS_UNKNOWN;
		}
	}
}

bool lessh(const TextRecord* rd1, const TextRecord* rd2)
{
	return rd1->rct.bottom-rd1->rct.top<rd2->rct.bottom-rd2->rct.top;
}

#define MAX_INT32 ((int)0x7fffffff)
#define MIN_INT32 ((int)0x80000000)
void ClassifyInterface::ComputeStats(ClassifyRes& cres)
{
	vector<TextRecord*> records;
	cres.bounding_box.left=cres.bounding_box.top=MAX_INT32;
	cres.bounding_box.right=cres.bounding_box.bottom=MIN_INT32;
	for(int i=0;i<(int)cres.records.size();i++)
	{
		if(cres.records[i].tclass!=CLASS_JUNK)
		{
			TRect rct=cres.records[i].rct;
			if(rct.left<cres.bounding_box.left)
				cres.bounding_box.left=rct.left;
			if(rct.right>cres.bounding_box.right)
				cres.bounding_box.right=rct.right;
			if(rct.top<cres.bounding_box.top)
				cres.bounding_box.top=rct.top;
			if(rct.bottom>cres.bounding_box.bottom)
				cres.bounding_box.bottom=rct.bottom;
			records.push_back(&cres.records[i]);
		}
	}
	sort(records.begin(),records.end(), lessh);
	if(records.size()==0)
		cres.mean_height=0;
	else if(records.size()%2==1)
	{
		TRect rect=records[(records.size()-1)/2]->rct;
		cres.mean_height=(float)(rect.bottom-rect.top);
	}
	else
	{
		TRect rect1=records[records.size()/2-1]->rct;
		TRect rect2=records[records.size()/2]->rct;
		cres.mean_height=(float)((rect1.bottom-rect1.top)+(rect2.bottom-rect2.top))/2;
	}
}

void ClassifyInterface::SortResult(ClassifyRes& cres)
{
	for(int i=0;i<(int)cres.records.size();i++)
	{
		TextRecord& rec=cres.records[i];
		if(rec.tclass==CLASS_JUNK)
			continue;
		if(rec.tclass=="")
			rec.tclass=CLASS_UNKNOWN;
		bool bfound=false;
		int ins=-1;
		for(int j=0;j<(int)cres.sorted_records.size();j++)
		{
			if(cres.sorted_records[j].tclass==rec.tclass)
			{
				bfound=true;
				cres.sorted_records[j].records.push_back(&rec);
				break;
			}
		}
		if(!bfound)
		{
			ClassRecord rd;
			rd.tclass=rec.tclass;
			rd.records.push_back(&rec);
			cres.sorted_records.push_back(rd);
		}
	}
}

void LowerCase(char* p, char*&p2)
{
	int offset=(int)'a'-(int)'A';
	int len=Utf8Len(p);
	if(*p>='A'&&*p<='Z')
	{
		assert(len==1);
		*p2=*p+offset;
	}
	else
		memcpy(p2, p, len);
	p2+=len;
}

void ClassifyInterface::RefineCharacters(ClassifyRes& cres)
{
	for(int i=0;i<(int)cres.sorted_records.size();i++)
	{
		ClassRecord& records=cres.sorted_records[i];
		for(int j=0;j<(int)records.records.size();j++)
		{
			TextRecord* record=records.records[j];
			char buf[max_line];
			ReplaceCharacters((char*)record->str.c_str(), buf, records.tclass);
			record->str=buf;
			if(records.tclass==string(CLASS_MAIL)||records.tclass==string(CLASS_URL))
			{
				char *pbuf=buf;
				for(char* p=(char*)record->str.c_str();!IsLineEnd(p);p=NextUtf8(p))
					LowerCase(p, pbuf);
				*pbuf=0;
				record->str=buf;
			}
		}
	}
}

bool RecordPred(const TextRecord* rd1, const TextRecord* rd2)
{
	if(rd1->rating>rd2->rating)
		return true;
	else
		return false;
}

void ClassifyInterface::SortTitle(vector<TextRecord*>& records, vector<TextRecord*>* relatives, ClassifyRes& cres)
{
	const float kWeightClassifyRate=0.6f;
	const float kWeightEvidence=0.4f;
	for(int i=0;i<(int)records.size();i++)
	{
		float evidence=TitleEvidence(*records[i], relatives, cres);
		records[i]->rating=records[i]->cls_data.rating*kWeightClassifyRate+evidence*kWeightEvidence;
	}
	sort(records.begin(), records.end(), RecordPred);
}

TextRecord* ClassifyInterface::SiftTitle(vector<TextRecord*>& records, vector<TextRecord*>* relatives, ClassifyRes& cres)
{
	if(records.size()==0)
		return NULL;
	int index=-1;
	float max_evidence=0.0f;
	for(int i=0;i<(int)records.size();i++)
	{
		if(records[i]->str.size()<kTitleCharCountThreshold)
			records[i]->rating=0;
		else
			records[i]->rating=TitleEvidence(*records[i], relatives, cres);
		if(records[i]->rating>max_evidence)
		{
			max_evidence=records[i]->rating;
			index=i;
		}
	}
	if(index>=0)
	{
		TextRecord* ret=records[index];
		records.erase(records.begin()+index);
		return ret;
	}
	else
		return NULL;
}

float ClassifyInterface::TitleEvidence(TextRecord& record, vector<TextRecord*>* relatives, ClassifyRes& cres)
{
	float pevi=PositionEvidence(record, relatives, cres.mean_height,
		(float)(cres.bounding_box.bottom-cres.bounding_box.top), (float)(cres.bounding_box.bottom-cres.bounding_box.top));
	float sevi=ScaleEvidence(record, cres.mean_height);
	float cevi=ContentEvidence(record);
	const float kWeightPosition=1.0f/3.0f;
	const float kWeightScale=1.0f/3.0f;
	const float kWeightContent=1.0f/3.0f;
	float evidence=kWeightPosition*pevi+kWeightScale*sevi+kWeightContent*cevi;
	return evidence;
}

float ClassifyInterface::PositionEvidence(TextRecord& record, vector<TextRecord*>* relatives, float mean, float width, float height)
{
	if(relatives==NULL||width<=0||height<=0)
		return 0;
	float max_evidence=0;
	const float kXMultiplier=0.5f;
	const float kYMultiplier=2.5f;
	const float base_evidence=-1.0f;
	float thresh=kYMultiplier*mean;
	float threshx=(record.rct.right-record.rct.left)*kXMultiplier;
	for(int i=0;i<(int)relatives->size();i++)
	{
		TextRecord& rela=*(*relatives)[i];
		float evidence=1.0f;
		if(record.rct.top-rela.rct.bottom>thresh)
			evidence+=(float)(record.rct.top-rela.rct.bottom-thresh)/height*base_evidence;
		else if(rela.rct.top-record.rct.bottom>thresh)
			evidence+=(float)(rela.rct.top-record.rct.bottom-thresh)/height*base_evidence;
		if(record.rct.left-rela.rct.right>threshx)
			evidence+=(float)(record.rct.left-rela.rct.right-threshx)/width*base_evidence;
		else if(rela.rct.left-record.rct.right>threshx)
			evidence+=(float)(rela.rct.left-record.rct.right-threshx)/width*base_evidence;
		evidence=MAX(0,MIN(1,evidence));
		evidence*=rela.cls_data.rating;
		if(evidence>max_evidence)
			max_evidence=evidence;
	}
	return max_evidence;
}

float ClassifyInterface::ScaleEvidence(TextRecord& record, float mean)
{
	const float kYMultiplierLower=1.0f;
	const float kYMultiplierUpper=2.2f;
	const float damp=1.0f/2;
	float evidence;
	int height=record.rct.bottom-record.rct.top;
	const float base_evidence=1.0f;
	float thresh1=mean*kYMultiplierLower;
	float thresh2=mean*kYMultiplierUpper;
	if(height<=thresh1)
		evidence=height/thresh1*base_evidence;
	else if(height>=thresh2)
		evidence=(1-(height/thresh2-1)*damp)*base_evidence;
	else
		evidence=1;
	evidence=MAX(0,MIN(1,evidence));
	return evidence;
}

float ClassifyInterface::ContentEvidence(TextRecord& record)
{
	const float base_evidence=1.0f;
	int regular=0;
	for(int i=0;i<(int)record.str.size();i++)
	{
		if(GetCharEnable(record.str[i]))
			regular++;
	}
	float evidence=(float)regular/(int)record.str.size()*base_evidence;
	return evidence;
}

bool ClassifyInterface::LoadText(const char* textbase, ClassifyRes& cres)
{
	string strText=string(textbase)+TEXT_FILE_SUFFIX;
	string strInfo=string(textbase)+INFO_FILE_SUFFIX;
	string strPara;
	FILE* fpt=fopen(strText.c_str(), "r");
	if(!fpt)
		return false;
	FILE* fpi=fopen(strInfo.c_str(), "r");
	if(!fpi)
	{
#ifndef TEXTCLASSIFY_TEST
		fclose(fpt);
		return false;
#endif
	}
	cres.groups.push_back(GroupRecord());
	char buf[4096];
	char ibuf[4096]={""};
	int s=0,e=0;
	TRect rc={0,0,0,0};
	float r=0,c=0;
	int cur=0;
	ParseInfo(fpi, s,e,rc,r,c);
	int groupid=0;
	vector<int> group;
	while(fgets(buf, 4096, fpt))
	{
		if(buf[0]==0||buf[0]==0x0A)
		{
			cres.groups.push_back(GroupRecord());
			groupid++;
			continue;
		}
		cur=strlen(buf);
		char* p;
		for(p=buf;!IsLineEnd(p);p=NextUtf8(p));
		if(*p=='\n'||*p=='\r')
			*p=0;
		TRect _rc=rc;
		int _s=s,_e=0;
		float _r=r,_c=c;
		do
		{
			_e+=e+1;
			if(_e>cur)
				break;
			if(rc.left<_rc.left)
				_rc.left=rc.left;
			if(rc.right>_rc.right)
				_rc.right=rc.right;
			if(rc.top<_rc.top)
				_rc.top=rc.top;
			if(rc.bottom>_rc.bottom)
				_rc.bottom=rc.bottom;
			if(r>_r)_r=r;
			if(c>_c)_c=c;
		}while(!ParseInfo(fpi, s,e,rc,r,c));
		TextRecord rd;
		rd.rct=_rc;
		rd.str=buf;
		rd.rating=0.0f;
		rd.cls_data.rating=0.0f;
		cres.records.push_back(rd);
		group.push_back(groupid);
	}
	fclose(fpt);
	if(fpi)
		fclose(fpi);

	for(int i=0;i<(int)cres.records.size();i++)
	{
		TextRecord& record = cres.records[i];
		record.group=&cres.groups[group[i]];
		cres.groups[group[i]].records.push_back(&record);
	}

	return true;
}

bool ClassifyInterface::ParseInfo(FILE* fpi, int& s, int& d, TRect& rc, float& r, float& c)
{
	if(!fpi)
		return true;
	char ibuf[4096]={""};
	if(!fgets(ibuf, 4096, fpi))
		return true;
	char* p;
	for(p=ibuf;*p!='#'&&!IsLineEnd(p);p=NextUtf8(p));
	//0,4,156,146,198,187,24.662197,-8.438055
	sscanf(p, "%*s%d%d%d%d%d%d%f%f", &s,&d,&rc.left,&rc.top,&rc.right,&rc.bottom,&r,&c);
	return s==0;
}

bool ClassifyInterface::LoadReplaceTable(const char* path)
{
	m_mapReplace.maps.clear();
	char line[max_line];
	m_curItem=NULL;
	if(FILE* file=fopen(path, "r"))
	{
		while(fgets(line, max_line, file))
		{
			if(line[0]!='\n'&&line[0]!='#')
			{
				ParseLine(line);
			}
		}
		fclose(file);
		m_curItem=NULL;
		return true;
	}
	return false;
}

void ClassifyInterface::ParseLine(char* line)
{
	char buf[max_line];
	char* p=line;
	GetNextToken(p, buf);
	if(buf[0]==':')
	{
		string item=buf+1;
		map<string, ReplaceMapItem>::iterator it=m_mapReplace.maps.find(item);
		if(it==m_mapReplace.maps.end())
			it=m_mapReplace.maps.insert(pair<string, ReplaceMapItem>(item,ReplaceMapItem())).first;
		m_curItem=&(*it).second;
	}
	else
	{
		string first=buf;
		GetNextToken(p, buf);
		string replace=buf;
		if(replace=="#")
			replace="";
		if(m_curItem!=NULL)
			m_curItem->substitutes[first]=replace;
	}
}

void ClassifyInterface::ReplaceCharacters(char* src, char* dest, const string& item)
{
	map<string,ReplaceMapItem>::iterator it=m_mapReplace.maps.find(item);
	if(it!=m_mapReplace.maps.end())
	{
		ReplaceMapItem& mapitem=(*it).second;
		char* pdest=dest;
		for(char* p=src;!IsLineEnd(p);p=NextUtf8(p))
		{
			char ch[10];
			int len=Utf8Len(p);
			memcpy(ch, p, len);
			ch[len]=0;
			string sch=ch;
			map<string, string>::iterator its=mapitem.substitutes.find(sch);
			if(its!=mapitem.substitutes.end())
				sch=(*its).second;
			strcpy(pdest, sch.c_str());
			pdest+=sch.size();
		}
	}
	else
		strcpy(dest, src);
}