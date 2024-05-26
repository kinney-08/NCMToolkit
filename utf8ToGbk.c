#ifdef WIN32
#include<Windows.h>
//返回转换好的字符串指针
unsigned char* utf8ToGbk(unsigned char*src,int len)
{
	wchar_t* tmp = (wchar_t*)malloc(sizeof(wchar_t) * len+2);
	unsigned char* newSrc = (unsigned char*)malloc(sizeof(unsigned char) * len + 2);
	
	MultiByteToWideChar(CP_UTF8, 0, src, -1, tmp, len);	//转为unicode
	WideCharToMultiByte(CP_ACP, 0, tmp, -1, newSrc, len+2, NULL,NULL); //转gbk
	
	return newSrc;
}
#endif