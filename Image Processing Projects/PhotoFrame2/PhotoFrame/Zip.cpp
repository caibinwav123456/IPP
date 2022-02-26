#include "stdafx.h"
#include "Zip.h"
#include "xzip/XZip.h"
#include "shlwapi.h"

BOOL AddDir(CString strDirSrc, CString strDir, HZIP hzip)
{
	BOOL bRet = TRUE;
	WIN32_FIND_DATA file;
	HANDLE handle=FindFirstFile(strDirSrc + _T("*.*"),&file);
	BOOL bFind = TRUE;
	while(handle!=INVALID_HANDLE_VALUE && bFind)
	{
		CString strFileNameSrc = strDirSrc +file.cFileName;
		CString strFileNameZip = strDir + file.cFileName;
		if(CString(file.cFileName) != _T(".") && CString(file.cFileName) != _T(".."))
		{
			if(PathIsDirectory(strFileNameSrc))
			{
				strFileNameSrc+=_T("\\");
				strFileNameZip+=_T("\\");
				if(ZipAdd(hzip, (LPCTSTR)strFileNameZip, 0,0,ZIP_FOLDER) != ZR_OK)
					bRet = FALSE;
				if(!AddDir(strFileNameSrc, strFileNameZip, hzip))
					bRet = FALSE;
			}
			else
			{
				if(ZipAdd(hzip, (LPCTSTR)strFileNameZip, (void*)(LPCTSTR)strFileNameSrc, 0, ZIP_FILENAME) != ZR_OK)
					bRet = FALSE;
			}
		}
		bFind = FindNextFile(handle,&file);
	}
	FindClose(handle);
	return bRet;
}

void DeleteDir(CString strDir)
{
	if(strDir.Right(1) != _T("\\"))
		strDir+=_T("\\");
	WIN32_FIND_DATA file;
	HANDLE handle=FindFirstFile(strDir + _T("*.*"),&file);
	BOOL bFind = TRUE;
	while(handle!=INVALID_HANDLE_VALUE && bFind)
	{
		CString strFileName = strDir +file.cFileName;
		if(CString(file.cFileName) != _T(".") && CString(file.cFileName) != _T(".."))
		{
			if(PathIsDirectory(strFileName))
			{
				DeleteDir(strFileName);
			}
			else
			{
				DeleteFile(strFileName);
			}
		}
		bFind = FindNextFile(handle,&file);		
	}
	FindClose(handle);
	BOOL bRet = RemoveDirectory(strDir.Left(strDir.GetLength()-1));
	DWORD dw = GetLastError();
}

BOOL ZipToDir(CString strZipFile, CString strDir)
{
	BOOL ret = PathFileExists(strDir);
	if (! ret){
		return FALSE;
	}

	HZIP hz = CreateZip((void*)(LPCTSTR)strZipFile, 0, ZIP_FILENAME);
	ret = TRUE;
	if (hz)
	{
		ZRESULT zr = 0;
		int index = 0;

		if(strDir.Right(1) != _T("\\"))
			strDir+=_T("\\");

		BOOL bSuc = AddDir(strDir, _T(""), hz);


		_ASSERT(bSuc);

		CloseZip(hz);
	}

	return ret;
}