#ifndef _TEXT_UTILITY_H_
#define _TEXT_UTILITY_H_

#include <string>
using namespace std;

#define IsSpace(p) (*(p)=='\t'||*(p)==' ')
#define IsLineEnd(p) (*(p)=='\r'||*(p)=='\n'||*(p)=='\0')

char* NextUtf8(char* txt);

int Utf8Len(char* txt);

bool GetNextToken(char* &p, char* token);

wstring CharToWchar(string s);

string WcharToChar(wstring ws);

#endif