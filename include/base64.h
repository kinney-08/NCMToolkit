#ifndef _BASE64_H_
#define _BASE64_H_

#include "NCMTool.h"

extern int Base64Table[];

unsigned char* base64_decode(unsigned char* code, int len, int* actLen);

#endif