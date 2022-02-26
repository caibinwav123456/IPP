#pragma once
#ifndef STATIC_LIB
#ifdef PREPROCESS_EXPORTS
#define PREPROCESS_API _declspec(dllexport)
#else
#define PREPROCESS_API _declspec(dllimport)
#endif
#else
#define PREPROCESS_API
#endif

#include <vector>
using namespace std;

#define PRE_NO_ADJUST  1
#define PRE_NO_DENOISE 2
#define PRE_NO_CLOSE   4
#define PRE_ADJ_SINGLEPASS 32
struct Progress_Callback
{
	void (*prog)(int pcnt);
	void (*interlock_inc)(int* n);
};

struct Preprocess_Callback
{
	void* (*threadstart)(unsigned int (*threadfunc)(void*),void* param);
	void (*lock)(int i);
	void (*unlock)(int i);
	void (*wait)(vector<void*>& v);
	void (*signal)(int nthread);
	Progress_Callback* prog;
	int core;
	bool bsinglelock;
};
//default save to source path
PREPROCESS_API bool Preprocess(char* filepath, char* outpath=NULL, int seg=19, float overlap=0.5, Preprocess_Callback* callback=NULL, int flags=PRE_NO_CLOSE);
