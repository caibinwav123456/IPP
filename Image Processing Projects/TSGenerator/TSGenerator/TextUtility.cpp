#include "stdafx.h"
#include "TextUtility.h"

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

char* NextUtf8( char* txt )
{
	int trail=trailingBytesForUTF8[*(unsigned char*)txt];
	return txt+1+trail;
}

int Utf8Len( char* txt )
{
	int trail=trailingBytesForUTF8[*(unsigned char*)txt];
	return 1+trail;
}

bool GetNextToken( char* &p, char* token )
{
	char* org=p;
	for(;IsSpace(p);p=NextUtf8(p));
	char* p2;
	if(*p=='\"')
	{
		p++;
		for(p2=p;!IsLineEnd(p2)&&*p2!='\"';p2=NextUtf8(p2));
		if(*p2=='\"')
		{
			memcpy(token, p, p2-p);
			token[p2-p]=0;
			if(*token==0)
				p=org;
			else
				p=p2+1;
		}
		else
		{
			*token=0;
			p=org;
		}
	}
	else
	{
		for(p2=p;!IsLineEnd(p2)&&!IsSpace(p2);p2=NextUtf8(p2));
		memcpy(token, p, p2-p);
		token[p2-p]=0;
		if(*token==0)
			p=org;
		else
			p=p2;
	}
	return *token!=0;
}

wstring CharToWchar(string s)
{
	UINT CodePage = CP_UTF8;
	int lstr = lstrlenA(s.c_str())+1;
	WCHAR* buf = (WCHAR*)alloca(lstr*sizeof(WCHAR));
	*buf = 0;
	int nRet = MultiByteToWideChar(CodePage, 0, s.c_str(), -1, buf, lstr);
	if(nRet == 0)
	{
		ASSERT(FALSE);
		return wstring();
	}
	wstring strw = buf;
	return strw;
}

string WcharToChar(wstring ws)
{
	UINT CodePage = CP_UTF8;
	int lstr = lstrlenW(ws.c_str())+1;
	char* buf = (char*)alloca(lstr*sizeof(WCHAR));
	*buf = 0;
	int nRet = WideCharToMultiByte(CodePage, 0, ws.c_str(), -1, buf, lstr*sizeof(WCHAR), NULL, FALSE);
	if(nRet == 0)
	{
		ASSERT(FALSE);
		return string();
	}
	string str = buf;
	return str;
}
