#ifndef _TEXT_CLASSIFY_H_
#define _TEXT_CLASSIFY_H_

#ifndef STATIC_LIB
#ifdef TEXTCLASSIFY_EXPORTS
#define TEXTCLASSIFY_API __declspec(dllexport)
#else
#define TEXTCLASSIFY_API __declspec(dllimport)
#endif
#else
#define TEXTCLASSIFY_API
#endif

#include <vector>
#include <string>
using namespace std;

#define IsSpace(p) (*(p)=='\t'||*(p)==' ')
#define IsLineEnd(p) (*(p)=='\r'||*(p)=='\n'||*(p)=='\0')

#define MAX_CHARSIZE 10
#define max_line 4096

#define CLSMODE_NORMAL 0
#define CLSMODE_ADAPT  1
#define CLSMODE_ADAPT_CLASSIFY 2

struct RChar
{
	char txt[MAX_CHARSIZE];
	int ratings;
	int duration;
};

struct CharChoice
{
	vector<RChar> choice;
};

struct TextClass
{
	vector<CharChoice> chars;
	int idattr;
	float damp;
	float max_rating;
	float bias;
	float corr_thresh;
	bool ignore_case;
	bool caption;
	bool whole_word;
	float margin_rating;
	float adapt_base;
	string corr_text;
	bool corr_text_empty;
	vector<float> max_rating_left;
};

struct SubStr
{
	int left;
	int index;
	float score;
	bool delflag;
	bool modified;
	float damp;
	int start;
	int end;
};

struct Score
{
public:
	float rating;
	vector<SubStr> pieces;
	Score():rating(0.0f){}
};

struct CorrStr
{
	int start;
	int end;
	float score;
	TextClass* tclass;
};

struct Correction
{
	string org;
	string corr;
	int fix_point;
	int fix_point_corr;
	vector<CorrStr> candts;
	vector<CorrStr> corrects;
};

struct ClassifyDataInternal
{
	bool start_of_word;
	bool end_of_word;
	Correction correction;
	int mode;
};

struct ClassifyData
{
	string corr;
	float rating;
	int idattr;
	int mode;
	ClassifyData()
	{
		rating=0.0f;
		idattr=-1;
		mode=CLSMODE_NORMAL;
	}
};

class CharMap
{
public:
	CharMap();
	~CharMap();
	void Insert(char* ch, int id);
	vector<int>* Find(char* ch);
	void clear();
private:
	vector<int> ids;
	CharMap* submaps[256];
};

struct AttributeItem
{
	string attrib;
	float thresh;
	float adapt_thresh;
};

struct ClassifierState
{
	float corrthresh;
	float margin_rating;
	bool ignore_case;
	bool whole_word;
	bool caption;
	ClassifierState()
	{
		clear();
	}
	void clear()
	{
		corrthresh=0.0f;
		ignore_case=false;
		whole_word=false;
		margin_rating=0.0f;
		caption=false;
	}
};

struct AdapterState
{
	float adapt_margin;
	bool adapt_ignore_case;
	bool adapt_whole_word;
	bool adapt_caption;
	AdapterState()
	{
		clear();
	}
	void clear()
	{
		adapt_ignore_case=false;
		adapt_whole_word=false;
		adapt_margin=0.0f;
		adapt_caption=false;
	}
};

char* NextUtf8(char* txt);
int Utf8Len(char* txt);
bool GetNextToken(char* &p, char* token);

class TEXTCLASSIFY_API TextClassify
{
public:
	TextClassify();
	char* GetAttribute(char* text, ClassifyData* data=NULL);
	char* GetAttribute(int id);
	int Classify(char* text, ClassifyData* data=NULL);
	bool Load(const char* filename);
	void Reset();
	void SetUnkownThreshold(float th);
	void RestoreAdapters();
	void PreAdapters();
private:
	void ParseLine(char* line);
	void AddClass(int id, float damp, float corr, bool bAdapt=false, float adapt_base=0.0f);
	void AddStr(char* str, int rating, bool bAdapt=false);
	void ModifyClasses(bool bAdapt=false);
	void AddToCharMap();
	void ComputeMaxRating(bool bAdapt=false);
	void AddPiece(char* p, int index, int pos, TextClass& tclass, Score& score, ClassifyDataInternal& cdata);
	void AdaptToText(char* p, int idattr, float adapt_base, ClassifyDataInternal& cdata);
	void FindScore(TextClass& tclass, char* p, int* index, float* score, int* duration);
	void DelayedCorrectText(Correction& corr);
	string ProcessAdapts(char* text);
	vector<TextClass> m_Classes;
	vector<TextClass> m_Adapts;
	vector<AttributeItem> m_Attributes;
	float m_thresh;
	int m_idDefault;
	CharMap m_CharMap;
	ClassifierState m_state;
	float m_adapt_thresh;
	AdapterState m_adapter_state;
};

#endif //_TEXT_CLASSIFY_H_