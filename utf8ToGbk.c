#include<Windows.h>

unsigned char* utf8ToGbk(unsigned char* src, int len)
{
	wchar_t* tmp = (wchar_t*)malloc(sizeof(wchar_t) * (len + 1));
	unsigned char* newSrc = (unsigned char*)malloc(sizeof(unsigned char) * (len + 2));
	
	MultiByteToWideChar(CP_UTF8, 0, src, -1, tmp, len);
	WideCharToMultiByte(CP_ACP, 0, tmp, -1, newSrc, len+2, NULL,NULL); 
	
	return newSrc;
}