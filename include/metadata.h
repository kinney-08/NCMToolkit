#ifndef __METADATA_H_
#define __METADATA_H_

#include "NCMTool.h"
#include "cJSON.h"

struct Metadata{
    int musicId;
    unsigned char musicName[STRINGLEN];
    struct{
        unsigned char Name[STRINGLEN];
        int Id;
    } artist[MULTILEN];
    int albumId;
    unsigned char album[STRINGLEN];
    unsigned char albumPicDocId[STRINGLEN];
    unsigned char albumPic[STRINGLEN];
    int bitrate;
    unsigned char mp3DocId[STRINGLEN];
    int duration;
    int mvId;
    unsigned char alias[MULTILEN][STRINGLEN];
    unsigned char transNames[MULTILEN][STRINGLEN];
    unsigned char format[STRINGLEN];
    int flag;
};

struct Metadata GetMetadata(struct cJSON* json);
void PrintMetadata(struct Metadata dat);

#endif