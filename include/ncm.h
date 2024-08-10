#ifndef __NCM_H
#define __NCM_H

#include <stdio.h>

#include "NCMTool.h"
#include "cJSON.h"
#include "metadata.h"

struct NCM{
    struct Metadata metadata;
    cJSON* meta_json;
    unsigned char * img;
    unsigned char * music;
    int len_img;
    int len_music;
    int errlevel;
};

void Del(struct NCM ncm);
void swap(unsigned char * a, unsigned char * b);
void rc4Init(unsigned char * s, const unsigned char * key, int len);
void rc4PRGA(unsigned char * s, unsigned char  * data, int len);
struct NCM DecryptNcm(FILE* f);
void AuthorStr(struct NCM ncm, char* author);
void FileName(struct NCM ncm, char* filename);
void WriteMusic(struct NCM ncm, char* filename);
void WriteCover(struct NCM ncm, char* filename);
void WriteMetadata(struct NCM ncm, char* filename);
#endif