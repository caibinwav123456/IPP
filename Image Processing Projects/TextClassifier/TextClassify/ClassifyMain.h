#ifndef _CLASSIFY_MAIN_H_
#define _CLASSIFY_MAIN_H_

#include "TextClassify.h"
#include <string>
#include <vector>
#include <map>
using namespace std;

struct TRect
{
	int left,top,right,bottom;
};

struct GroupRecord;
struct TextRecord
{
	string str;
	TRect rct;
	string tclass;
	int rclass;
	GroupRecord* group;
	ClassifyData cls_data;
	float rating;
};

struct GroupRecord
{
	vector<TextRecord*> records;
};

struct ClassRecord
{
	string tclass;
	vector<TextRecord*> records;
};

struct ClassifyRes
{
	vector<TextRecord> records;
	vector<ClassRecord> sorted_records;
	vector<GroupRecord> groups;
	float mean_height;
	TRect bounding_box;
};

struct ReplaceMapItem
{
	map<string, string> substitutes;
};

struct ReplaceMap
{
	map<string, ReplaceMapItem> maps;
};

class TEXTCLASSIFY_API ClassifyInterface
{
public:
	ClassifyInterface();
	~ClassifyInterface();
	void SetBasePath(const char* path);
	void SetExcludeTag(bool b);
	bool LoadConfig();
	void MasterClassify(const char* textbase, ClassifyRes& cres);
private:
	bool LoadText(const char* textbase, ClassifyRes& cres);
	bool LoadReplaceTable(const char* path);
	bool ParseInfo(FILE* fpi, int& s, int& d, TRect& rc, float& r, float& c);
	void ParseLine(char* line);
	int RoughClassify(const char* text);
	void PostClassify(ClassifyRes& cres);
	void ComputeStats(ClassifyRes& cres);
	void SortResult(ClassifyRes& cres);
	float ComputeEvidence(TextRecord& record);
	float PositionEvidence(TextRecord& record, vector<TextRecord*>* relatives, float mean, float width, float height);
	float ScaleEvidence(TextRecord& record, float mean);
	float ContentEvidence(TextRecord& record);
	float TitleEvidence(TextRecord& record, vector<TextRecord*>* relatives, ClassifyRes& cres);
	void SortTitle(vector<TextRecord*>& records, vector<TextRecord*>* relatives, ClassifyRes& cres);
	TextRecord* SiftTitle(vector<TextRecord*>& records, vector<TextRecord*>* relatives, ClassifyRes& cres);
	void ProcessNumRecords(string& text);
	void RefineCharacters(ClassifyRes& res);
	void ReplaceCharacters(char* src, char* dest, const string& item);
	string m_strBase;
	TextClassify m_clsalpha;
	TextClassify m_clsnum;
	bool m_bExcludeTag;
	ReplaceMap m_mapReplace;
	ReplaceMapItem* m_curItem;
};

#endif //_CLASSIFY_MAIN_H_