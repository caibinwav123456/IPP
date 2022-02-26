#include "stdafx.h"
#include "TextClassify.h"
#include <math.h>
#include <assert.h>
#include <algorithm>

static const char trailingBytesForUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

char* NextUtf8(char* txt)
{
	int trail=trailingBytesForUTF8[*(unsigned char*)txt];
	return txt+1+trail;
}

int Utf8Len(char* txt)
{
	int trail=trailingBytesForUTF8[*(unsigned char*)txt];
	return 1+trail;
}

static const unsigned char Delimiters[] = {' ', ':', ';', '\t', ',', '.','\r', '\n'};

bool IsDelimiter(char* p)
{
	static bool _table[256];
	static bool b=true;
	if(b)
	{
		b=false;
		for(int i=0;i<256;i++)
			_table[i]=false;
		for(int i=0;i<sizeof(Delimiters)/sizeof(unsigned char);i++)
			_table[Delimiters[i]]=true;
	}
	return _table[*(unsigned char*)p];
}

void TrimString(char* &p)
{
	for(;IsSpace(p);p=NextUtf8(p));
}

bool GetNextToken(char* &p, char* token)
{
	char* org=p;
	for(;IsSpace(p);p=NextUtf8(p));
	char* p2;
	for(p2=p;!IsLineEnd(p2)&&!IsSpace(p2);p2=NextUtf8(p2));
	memcpy(token, p, p2-p);
	token[p2-p]=0;
	if(*token==0)
		p=org;
	else
		p=p2;
	return *token!=0;
}

CharMap::CharMap()
{
	for(int i=0;i<256;i++)
		submaps[i]=NULL;
}

CharMap::~CharMap()
{
	clear();
}

void CharMap::clear()
{
	ids.clear();
	for(int i=0;i<256;i++)
	{
		if(submaps[i]!=NULL)
		{
			delete submaps[i];
			submaps[i]=NULL;
		}
	}
}

void CharMap::Insert(char* ch, int id)
{
	CharMap* next=this;
	for(char* p=ch;*p!=0;p++)
	{
		int c=*(unsigned char*)p;
		if(next->submaps[c]==NULL)
			next->submaps[c]=new CharMap;
		next=next->submaps[c];
	}
	if(next->ids.size()==0||next->ids[next->ids.size()-1]!=id)
		next->ids.push_back(id);
}

vector<int>* CharMap::Find(char* ch)
{
	int id=-1;
	CharMap* next=this;
	for(char* p=ch;*p!=0;p++)
	{
		id=*(unsigned char*)p;
		next=next->submaps[id];
		if(next==NULL)
			return NULL;
	}
	return &next->ids;
}

const float kUnkownThreshold=0.5f;
const float kDefAdaptThresh=0.9f;
TextClassify::TextClassify()
{
	m_thresh=kUnkownThreshold;
	m_idDefault=-1;
	m_adapt_thresh=kDefAdaptThresh;
}

void TextClassify::SetUnkownThreshold(float th)
{
	m_thresh=th;
	if(m_thresh>1)m_thresh=1;
	if(m_thresh<0)m_thresh=0;
}

void TextClassify::RestoreAdapters()
{
	m_Adapts.clear();
}

void TextClassify::PreAdapters()
{
	ModifyClasses(true);
	ComputeMaxRating(true);
}

bool TextClassify::Load(const char* filename)
{
	RestoreAdapters();
	Reset();
	char line[max_line];
	if(FILE* file=fopen(filename, "r"))
	{
		while(fgets(line, max_line, file))
		{
			if(line[0]!='\n'&&line[0]!='#')
			{
				ParseLine(line);
			}
		}
		fclose(file);
		ModifyClasses();
		ComputeMaxRating();
		AddToCharMap();
		return true;
	}
	return false;
}

void TextClassify::ParseLine(char* line)
{
	char buf[max_line];
	char* p=line;
	GetNextToken(p, buf);
	if(strcmp(buf, "class")==0)
	{
		int id=0;
		float damp=0.5;
		float corr_th=m_state.corrthresh;
		char *p1,*p2=p,*p3=p;
		bool b;
		int token_count=-1;
		do
		{
			p1=p2;
			p2=p3;
			b=GetNextToken(p3, buf);
			token_count++;
		}
		while(strcmp(buf, "str")!=0&&b);
		char buf2[max_line];
		if(token_count==3)
		{
			memcpy(buf2, p, p1-p);
			buf2[p1-p]=0;
			char buf3[max_line];
			memcpy(buf3, p1, p2-p1);
			buf3[p2-p1]=0;
			sscanf(buf3, "%f", &corr_th);
		}
		else
		{
			memcpy(buf2, p, p2-p);
			buf2[p2-p]=0;
		}
		sscanf(buf2, "%d%f", &id, &damp);
		AddClass(id, damp, corr_th);
		if(strcmp(buf, "str")==0)
		{
			TrimString(p3);
			char* p4=p3;
			while(GetNextToken(p4, buf2));
			memcpy(buf2, p3, p4-p3);
			buf2[p4-p3]=0;
			AddStr(buf2, 5);
		}
	}
	else if(strcmp(buf, "str")==0)
	{
		char buf2[max_line];
		int rating=0;
		TrimString(p);
		char*p2;
		char*p3=p;
		char*p4=p;
		do 
		{
			p2=p3;
			p3=p4;
		} while (GetNextToken(p4, buf2));
		memcpy(buf2, p, p2-p);
		buf2[p2-p]=0;
		sscanf(p2, "%d", &rating);
		AddStr(buf2, rating);
	}
	else if(strcmp(buf, "attrib")==0)
	{
		char buf2[max_line];
		while(GetNextToken(p, buf2))
		{
			AttributeItem item;
			item.attrib=string(buf2);
			item.thresh=0.0f;
			item.adapt_thresh=10.0f;
			m_Attributes.push_back(item);
		}
	}
	else if(strcmp(buf, "thresh")==0)
	{
		char buf2[max_line];
		GetNextToken(p, buf2);
		float arg1=0;
		sscanf(buf2, "%f", &arg1);
		if(GetNextToken(p, buf2))
		{
			//Format 1: thresh (id) (thresh)
			int id=(int)arg1;
			float thresh=0;
			sscanf(buf2, "%f", &thresh);
			if(thresh<0)thresh=0;
			if(thresh>1)thresh=1;
			if(id>=0&&id<(int)m_Attributes.size())
				m_Attributes[id].thresh=thresh;
		}
		else
		{
			//Format 2: thresh (total thresh)
			m_thresh=arg1;
			if(m_thresh>1)m_thresh=1;
			if(m_thresh<0)m_thresh=0;
		}
	}
	else if(strcmp(buf, "adapt_th")==0)
	{
		char buf2[max_line];
		GetNextToken(p, buf2);
		float arg1=0;
		sscanf(buf2, "%f", &arg1);
		if(GetNextToken(p, buf2))
		{
			//Format 1: adapt_thresh (id) (thresh)
			int id=(int)arg1;
			float thresh=0;
			sscanf(buf2, "%f", &thresh);
			if(thresh<0)thresh=0;
			if(id>=0&&id<(int)m_Attributes.size())
				m_Attributes[id].adapt_thresh=thresh;
		}
		else
		{
			//Format 2: adapt_thresh (total thresh)
			m_adapt_thresh=arg1;
			if(m_adapt_thresh<0)m_adapt_thresh=0;
		}
	}
	else if(strcmp(buf, "default")==0)
	{
		sscanf(p, "%d", &m_idDefault);
		if(m_idDefault<0||m_idDefault>=(int)m_Attributes.size())
			m_idDefault=-1;
	}
	else if(strcmp(buf, "bias")==0)
	{
		float bias;
		sscanf(p, "%f", &bias);
		if(m_Classes.size()>0)
			m_Classes[m_Classes.size()-1].bias=bias;
	}
	else if(strcmp(buf, "corr_text")==0)
	{
		char buf2[max_line];
		TrimString(p);
		char* p2=p;
		while(GetNextToken(p2, buf2));
		memcpy(buf2, p, p2-p);
		buf2[p2-p]=0;
		if(m_Classes.size()>0)
		{
			if(strcmp(buf2, "#")==0)
				m_Classes[m_Classes.size()-1].corr_text_empty=true;
			else
				m_Classes[m_Classes.size()-1].corr_text=buf2;
		}
	}
	else if(strcmp(buf, "corr_th")==0)
	{
		float corr_th=m_state.corrthresh;
		sscanf(p, "%f", &corr_th);
		m_state.corrthresh=corr_th;
	}
	else if(strcmp(buf, "ignore_case")==0)
	{
		char buf2[max_line];
		GetNextToken(p, buf2);
		if(strcmp(buf2, "+")==0)
			m_state.ignore_case=true;
		else if(strcmp(buf2, "-")==0)
			m_state.ignore_case=false;
		if(GetNextToken(p, buf2)&&strcmp(buf2, "c")==0)
			m_state.caption=true;
		else
			m_state.caption=false;
	}
	else if(strcmp(buf, "adapt_ignore_case")==0)
	{
		char buf2[max_line];
		GetNextToken(p, buf2);
		if(strcmp(buf2, "+")==0)
			m_adapter_state.adapt_ignore_case=true;
		else if(strcmp(buf2, "-")==0)
			m_adapter_state.adapt_ignore_case=false;
		if(GetNextToken(p, buf2)&&strcmp(buf2, "c")==0)
			m_adapter_state.adapt_caption=true;
		else
			m_adapter_state.adapt_caption=false;
	}
	else if(strcmp(buf, "whole_word")==0)
	{
		char buf2[max_line];
		GetNextToken(p, buf2);
		if(strcmp(buf2, "+")==0)
			m_state.whole_word=true;
		else if(strcmp(buf2, "-")==0)
			m_state.whole_word=false;
		if(GetNextToken(p, buf2))
		{
			float rating=0.0f;
			sscanf(buf2, "%f", &rating);
			if(rating<0.0f)rating=0.0f;
			m_state.margin_rating=rating;
		}
	}
	else if(strcmp(buf, "adapt_whole_word")==0)
	{
		char buf2[max_line];
		GetNextToken(p, buf2);
		if(strcmp(buf2, "+")==0)
			m_adapter_state.adapt_whole_word=true;
		else if(strcmp(buf2, "-")==0)
			m_adapter_state.adapt_whole_word=false;
		if(GetNextToken(p, buf2))
		{
			float rating=0.0f;
			sscanf(buf2, "%f", &rating);
			if(rating<0.0f)rating=0.0f;
			m_adapter_state.adapt_margin=rating;
		}
	}
}

void TextClassify::AddClass(int id, float damp, float corr, bool bAdapt, float adapt_base)
{
	TextClass tclass;
	tclass.idattr=id;
	tclass.damp=damp;
	tclass.bias=0;
	tclass.corr_thresh=corr;
	tclass.ignore_case=m_state.ignore_case;
	tclass.caption=m_state.caption;
	tclass.whole_word=m_state.whole_word;
	tclass.margin_rating=m_state.margin_rating;
	tclass.corr_text_empty=false;
	tclass.adapt_base=adapt_base;
	if(bAdapt)
		m_Adapts.push_back(tclass);
	else
		m_Classes.push_back(tclass);
}

void TextClassify::AddStr(char* str, int rating, bool bAdapt)
{
	if((!bAdapt&&m_Classes.size()==0)||(bAdapt&&m_Adapts.size()==0))
		return;
	TextClass& tclass=(bAdapt?m_Adapts[m_Adapts.size()-1]:m_Classes[m_Classes.size()-1]);
	if(tclass.chars.size()==0)
	{
		int last=0;
		for(char* p=str;*p!=0;p=NextUtf8(p))
		{
			if(*p!='$')
			{
				RChar rchar;
				memcpy(rchar.txt, p, Utf8Len(p));
				rchar.txt[Utf8Len(p)]=0;
				rchar.ratings=rating;
				rchar.duration=1;
				CharChoice choice;
				choice.choice.push_back(rchar);
				tclass.chars.push_back(choice);
				last++;
			}
			else
			{
				assert(last>0);
				tclass.chars[last-1].choice[0].duration++;
				tclass.chars.push_back(CharChoice());
			}
		}
	}
	else
	{
		int i=0;
		RChar* lastchar=NULL;
		for(char* p=str;*p!=0;p=NextUtf8(p),i++)
		{
			RChar rchar;
			memcpy(rchar.txt, p, Utf8Len(p));
			rchar.txt[Utf8Len(p)]=0;
			rchar.ratings=rating;
			rchar.duration=1;
			if(i>=(int)tclass.chars.size())
				tclass.chars.push_back(CharChoice());
			if(strcmp(rchar.txt, "$")==0)
			{
				if(lastchar)
					lastchar->duration++;
				continue;
			}
			CharChoice& choice=tclass.chars[i];
			bool bfound=false;
			for(int j=0;j<(int)choice.choice.size();j++)
			{
				if(strcmp(choice.choice[j].txt, rchar.txt)==0)
				{
					bfound=true;
					lastchar=NULL;
					break;
				}
			}
			if(!bfound)
			{
				choice.choice.push_back(rchar);
				lastchar=&choice.choice[choice.choice.size()-1];
			}
		}
	}
	if(tclass.corr_text.empty())
	{
		for(int i=0;i<(int)tclass.chars.size();i++)
		{
			if(tclass.chars[i].choice.size()==0)
				continue;
			assert(tclass.chars[i].choice.size()==1);
			tclass.corr_text+=tclass.chars[i].choice[0].txt;
		}
	}
}

bool XUpper(char* src, char* dest)
{
	int offset=(int)'A'-(int)'a';
	if(*src>='a'&&*src<='z')
	{
		dest[0]=*src+offset;
		dest[1]=0;
		return true;
	}
	else if(*src>='A'&&*src<='Z')
	{
		dest[0]=*src-offset;
		dest[1]=0;
		return true;
	}
	return false;
}

void TextClassify::ModifyClasses(bool bAdapt)
{
	vector<TextClass>& classes=(bAdapt?m_Adapts:m_Classes);
	for(int i=0;i<(int)classes.size();i++)
	{
		TextClass& tclass=classes[i];
		if(tclass.ignore_case)
		{
			for(int j=0;j<(int)tclass.chars.size();j++)
			{
				if((int)tclass.chars[j].choice.size()>0)
				{
					RChar& rchar=tclass.chars[j].choice[0];
					RChar rchar2=rchar;
					if(!(tclass.caption&&j==0))
					{
						if(XUpper(rchar.txt, rchar2.txt))
							tclass.chars[j].choice.push_back(rchar2);
					}
					j+=tclass.chars[j].choice[0].duration-1;
					assert(j<(int)tclass.chars.size());
				}
			}
		}
		if(tclass.corr_text_empty)
			tclass.corr_text="";
	}
}

void TextClassify::ComputeMaxRating(bool bAdapt)
{
	vector<TextClass>& classes=(bAdapt?m_Adapts:m_Classes);
	for(int i=0;i<(int)classes.size();i++)
	{
		TextClass& tclass=classes[i];
		vector<CharChoice>& chars=tclass.chars;
		float margin=0;
		if(tclass.whole_word)
			margin=tclass.margin_rating;
		tclass.max_rating_left.push_back(margin);
		for(int j=(int)chars.size()-1;j>=0;j--)
		{
			vector<RChar>& choice = chars[j].choice;
			float max_rating_left=0;
			for(int k=0;k<(int)choice.size();k++)
			{
				float max_rating=tclass.max_rating_left[tclass.max_rating_left.size()-choice[k].duration];
				max_rating+=choice[k].ratings*choice[k].duration;
				if(max_rating>max_rating_left)
					max_rating_left=max_rating;
			}
			if(j!=0)
				tclass.max_rating_left.push_back(max_rating_left);
			else
			{
				if(tclass.whole_word)
					max_rating_left+=tclass.margin_rating;
				tclass.max_rating=max_rating_left;
			}
		}
		tclass.max_rating_left[0]=0;
	}
}

void TextClassify::AddToCharMap()
{
	for(int i=0;i<(int)m_Classes.size();i++)
	{
		TextClass& tclass=m_Classes[i];
		for(int j=0;j<(int)tclass.chars.size();j++)
		{
			for(int k=0;k<(int)tclass.chars[j].choice.size();k++)
			{
				m_CharMap.Insert(tclass.chars[j].choice[k].txt, i);
			}
		}
	}
}

bool CorrectsPred(const CorrStr& str1, const CorrStr& str2)
{
	if(str1.start<str2.start)
		return true;
	else if(str1.start>str2.start)
		return false;
	else
	{
		int l1=str1.end-str1.start;
		int l2=str2.end-str2.start;
		if(l1>l2)
			return true;
		else
			return false;
	}
}

int TextClassify::Classify(char* text, ClassifyData* data)
{
	vector<Score> scores;
	ClassifyDataInternal cdata;
	cdata.mode=(data?data->mode:CLSMODE_NORMAL);
	Correction& corr_text=cdata.correction;
	corr_text.fix_point=corr_text.fix_point_corr=0;
	vector<TextClass>& classes=(cdata.mode==CLSMODE_ADAPT_CLASSIFY?m_Adapts:m_Classes);
	for(int i=0;i<(int)classes.size();i++)
		scores.push_back(Score());
	int index=0;
	char* last=NULL;
	char* end;
	for(end=text;*end!=0;end++);
	bool end_of_line=false;
	for(char* p=text;!end_of_line;p=NextUtf8(p),index++)
	{
		end_of_line=!(*p!=0&&p<end);
		if(IsSpace(p) && last && IsSpace(last))
		{
			index--;
			continue;
		}
		char utfch[10];
		if(end_of_line)
			utfch[0]=0;
		else
		{
			int len=Utf8Len(p);
			memcpy(utfch, p, len);
			utfch[len]=0;
		}
		char* next=(end_of_line?NULL:NextUtf8(p));
		cdata.start_of_word=(!last||IsDelimiter(last));
		cdata.end_of_word=(end_of_line||!*next||IsDelimiter(next));
		corr_text.candts.clear();
		vector<int>* Classes=(cdata.mode==CLSMODE_ADAPT_CLASSIFY?NULL:m_CharMap.Find(utfch));
		if(end_of_line||cdata.mode==CLSMODE_ADAPT_CLASSIFY)
			for(int i=0;i<(int)classes.size();i++)
				AddPiece(utfch, index, corr_text.org.size(), classes[i], scores[i], cdata);
		else if(Classes!=NULL)
			for(int i=0;i<(int)Classes->size();i++)
				AddPiece(utfch, index, corr_text.org.size(), m_Classes[(*Classes)[i]], scores[(*Classes)[i]], cdata);
		corr_text.corr+=utfch;
		corr_text.org+=utfch;
		CorrStr* candt=NULL;
		float corr_max_score=0.0f;
		for(int i=0;i<(int)corr_text.candts.size();i++)
		{
			if(corr_text.candts[i].score>corr_max_score)
			{
				corr_max_score=corr_text.candts[i].score;
				candt=&corr_text.candts[i];
			}
			else if(corr_text.candts[i].score=corr_max_score)
			{
				if(CorrectsPred(corr_text.candts[i], *candt))
					candt=&corr_text.candts[i];
			}
		}
		if(candt!=NULL)
			corr_text.corrects.push_back(*candt);
		last=p;
	}
	if(cdata.mode==CLSMODE_NORMAL||cdata.mode==CLSMODE_ADAPT)
		DelayedCorrectText(corr_text);
	int ibest=-1;
	float max_rating=0;
	vector<int> complete_matches;
	for(int i=0;i<(int)m_Attributes.size();i++)
		complete_matches.push_back(0);
	for(int i=0;i<(int)scores.size();i++)
	{
		scores[i].rating/=classes[i].max_rating;//normalize ratings
		//Sift out the classes with higher ratings than the
		//corresponding attribute threshold.
		if(scores[i].rating>=m_Attributes[classes[i].idattr].thresh)
		{
			if(cdata.mode==CLSMODE_ADAPT_CLASSIFY&&data)
			{
				scores[i].rating*=classes[i].adapt_base;
			}
			if(scores[i].rating==1.0f)
			{
				scores[i].rating+=classes[i].bias;
				complete_matches[classes[i].idattr]++;
			}
			if(scores[i].rating>max_rating)
			{
				max_rating=scores[i].rating;
				ibest=i;
			}
		}
	}

	const float min_rating=0.2f;
	if(cdata.mode==CLSMODE_ADAPT_CLASSIFY&&data&&max_rating<=data->rating)
	{
		ibest=data->idattr;
		max_rating=data->rating;
	}
	else if(ibest>=0&&ibest<(int)classes.size()&&max_rating>m_thresh)
	{
		if(max_rating==1.0f)
		{
			//If there are complete matches(except for biased),
			//sift out the category with the maximum times of match.
			ibest=m_idDefault;
			int max_count=0;
			for(int i=0;i<(int)m_Attributes.size();i++)
			{
				if(complete_matches[i]>max_count)
				{
					max_count=complete_matches[i];
					ibest=i;
				}
			}
		}
		else
			ibest=classes[ibest].idattr;
		if(cdata.mode==CLSMODE_ADAPT&&max_rating>=m_adapt_thresh&&max_rating>=m_Attributes[ibest].adapt_thresh)
			AdaptToText(text, ibest, max_rating, cdata);
	}
	else
	{
		ibest=m_idDefault;
		max_rating=min_rating;
	}
	if(data)
	{
		data->idattr=ibest;
		data->rating=max_rating;
		if(cdata.mode!=CLSMODE_ADAPT_CLASSIFY)
			data->corr=corr_text.corr;
	}
	return ibest;
}

void TextClassify::FindScore(TextClass& tclass, char* p, int* index, float* score, int* duration)
{
	int start=*index;
	for(int i=start+1;i<(int)tclass.chars.size();i++)
	{
		for(int j=0;j<(int)tclass.chars[i].choice.size();j++)
		{
			if(strcmp(tclass.chars[i].choice[j].txt, p)==0)
			{
				*index=i;
				*score=(float)tclass.chars[i].choice[j].ratings;
				*duration=tclass.chars[i].choice[j].duration;
				return;
			}
		}
	}
	*index=-1;
	*score=0;
	*duration=0;
	return;
}

void TextClassify::AddPiece(char* p, int ip, int pos, TextClass& tclass, Score& score, ClassifyDataInternal& cdata)
{
	int index=-1;
	float sc;
	int np=(int)score.pieces.size();
	vector<SubStr> complete_str;
	bool bcorr = (tclass.corr_thresh>0.0f);
	bool eliminate_pass=true;
	for(int i=0;i<(int)score.pieces.size();i++)
		score.pieces[i].modified=false;
	while(true)
	{
		int duration;
		if(*p!=0)
		{
			FindScore(tclass, p, &index, &sc, &duration);
			if(index==-1)
				break;
		}
		bool bstart=true;
		int left=tclass.chars.size()-index-1;
		for(int i=0;i<np;i++)
		{
			SubStr& str=score.pieces[i];
			assert(str.damp<=1.0f);
			if(str.modified)
				continue;
			if(eliminate_pass)
			{
				float damp=str.damp*powf(tclass.damp, (float)(ip-str.index-1));
				assert(damp<=1.0f);
				if(*p==0||damp*(str.score+tclass.max_rating_left[str.left])<=score.rating)
				{
					str.delflag=true;
					if(bcorr)
						complete_str.push_back(str);
					continue;
				}
			}
			if(str.left<=left)
				continue;
			SubStr str2;
			str2.index=ip;
			str2.left=left+1-duration;
			str2.score=str.score+sc*duration;
			str2.delflag=false;
			str2.damp=str.damp*powf(tclass.damp, (float)((ip-str.index-1)+(str.left-left-1)));
			assert(str2.damp<=1.0f);
			str2.start=str.start;
			str2.end=pos;
			str2.modified=true;
			if(str2.damp==1.0f)
				bstart=false;
			if(str2.left==0)
			{
				str2.delflag=true;
				if(tclass.whole_word&&cdata.end_of_word)
					str2.score+=tclass.margin_rating;
				if(bcorr)
					complete_str.push_back(str2);
			}
			if(str2.score*str2.damp>score.rating)
				score.rating=str2.score*str2.damp;
			if((str2.score+tclass.max_rating_left[str2.left])*str2.damp<=score.rating)
				str2.delflag=true;
			if(str.left-left>1)
				score.pieces.push_back(str2);
			else
				score.pieces[i]=str2;
		}
		if(*p==0)
			break;
		eliminate_pass=false;
		if(bstart)
		{
			SubStr str;
			str.index=ip;
			str.left=left+1-duration;
			str.score=sc*duration;
			str.delflag=false;
			str.damp=1;
			str.start=pos;
			str.end=pos;
			if(tclass.whole_word&&cdata.start_of_word&&index==0)
				str.score+=tclass.margin_rating;
			if(str.score*str.damp>score.rating)
				score.rating=str.score*str.damp;
			if((str.score+tclass.max_rating_left[str.left])*str.damp>score.rating&&str.left!=0)
				score.pieces.push_back(str);
		}
		for(int i=0;i<(int)score.pieces.size();i++)
		{
			if(score.pieces[i].delflag)
			{
				score.pieces.erase(score.pieces.begin()+i);
				if(i<np)np--;
				i--;
			}
		}
	}
	if(bcorr)
	{
		for(int i=0;i<(int)complete_str.size();i++)
		{
			SubStr& str=complete_str[i];
			float tscore=str.score*str.damp;
			float ntscore=tscore/tclass.max_rating;//normalize ratings
			assert(ntscore<=1.0f);
			if(tscore==score.rating && ntscore>=tclass.corr_thresh)
			{
				//Ready to correct the word
				CorrStr corr_str={str.start, str.end, ntscore, &tclass};
				cdata.correction.candts.push_back(corr_str);
			}
		}
	}
}

bool IsLegalAdaptChar(char* p)
{
	if((*p>='a'&&*p<='z')||(*p>='A'&&*p<='Z')||(*p>='0'&&*p<='9'))
		return true;
	else
		return false;
}

bool GetNextAdaptPiece(char* &p, char* piece)
{
	for(;*p!=0&&!IsDelimiter(p);p=NextUtf8(p));
	char* p2=p;
	for(;*p2!=0&&IsDelimiter(p2);p2=NextUtf8(p2));
	memcpy(piece, p, p2-p);
	piece[p2-p]=0;
	p=p2;
	return *piece!=0;
}

void TextClassify::AdaptToText(char* p, int idattr, float adapt_base, ClassifyDataInternal& cdata)
{
	const float kAdaptDamp=0.7f;
	char buf[max_line];
	m_state.clear();
	m_state.caption=m_adapter_state.adapt_caption;
	m_state.ignore_case=m_adapter_state.adapt_ignore_case;
	m_state.whole_word=m_adapter_state.adapt_whole_word;
	m_state.margin_rating=m_adapter_state.adapt_margin;
	while(GetNextAdaptPiece(p, buf))
	{
		string piece=ProcessAdapts(buf);
		if(!piece.empty())
		{
			AddClass(idattr, kAdaptDamp, 0.0f, true, adapt_base>1.0f?1.0f:adapt_base);
			AddStr((char*)piece.c_str(), 5, true);
		}
	}
}

string TextClassify::ProcessAdapts(char* text)
{
	float kPieceThresh=0.9f;
	int kPieceThreshAbs=5;
	int cnt=0,nlegal=0;
	for(char* p=text;*p!=0;p=NextUtf8(p))
	{
		cnt++;
		if(IsLegalAdaptChar(p))
			nlegal++;
	}
	if(cnt==0)
		return string();
	if(nlegal>=cnt*kPieceThresh && nlegal>=kPieceThreshAbs)
	{
		char* pbuf=text;
		for(char* p=text;*p!=0;p=NextUtf8(p))
		{
			if(IsLegalAdaptChar(p))
			{
				int len=Utf8Len(p);
				memcpy(pbuf, p, len);
				pbuf+=len;
			}
		}
		*pbuf=0;
		return string(text);
	}
	return string();
}

void TextClassify::DelayedCorrectText(Correction& corr)
{
	sort(corr.corrects.begin(), corr.corrects.end(), CorrectsPred);
	for(int i=0;i<(int)corr.corrects.size();i++)
	{
		if(corr.corrects[i].start<corr.fix_point)
			continue;
		string tail="";
		int offset=corr.fix_point_corr-corr.fix_point;
		int replace_start=corr.corrects[i].start+offset;
		int tail_start_org=corr.corrects[i].end;
		if((int)corr.org.size()>tail_start_org)
			tail_start_org+=Utf8Len(const_cast<char*>(corr.org.c_str())+tail_start_org);
		int tail_start=tail_start_org+offset;
		if((int)corr.corr.size()>tail_start)
			tail=corr.corr.substr(tail_start,corr.corr.size()-tail_start);
		corr.corr=corr.corr.substr(0, replace_start)+corr.corrects[i].tclass->corr_text+tail;
		corr.fix_point=tail_start_org;
		offset=corr.corr.size()-corr.org.size();
		corr.fix_point_corr=corr.fix_point+offset;
	}
}

void TextClassify::Reset()
{
	m_Classes.clear();
	m_CharMap.clear();
	m_Attributes.clear();
	m_state.clear();
	m_adapter_state.clear();
	m_thresh=kUnkownThreshold;
	m_idDefault=-1;
}

char* TextClassify::GetAttribute(int id)
{
	if(id>=0&&id<(int)m_Attributes.size())
		return const_cast<char*>(m_Attributes[id].attrib.c_str());
	else
		return NULL;
}

char* TextClassify::GetAttribute(char* text, ClassifyData* data)
{
	return GetAttribute(Classify(text, data));
}
