#include <string.h>

#include "metadata.h"

#include "NCMTool.h"
#include "ncm.h"
#include "cJSON.h"

struct Metadata GetMetadata(struct cJSON* json){
    struct Metadata metadata;
    memset(&metadata, 0, sizeof(metadata));

    metadata.musicId    =           cJSON_GetObjectItem(json, "musicId")        ->valueint;
    metadata.albumId    =           cJSON_GetObjectItem(json, "albumId")        ->valueint;
    metadata.bitrate    =           cJSON_GetObjectItem(json, "bitrate")        ->valueint;
    metadata.duration   =           cJSON_GetObjectItem(json, "duration")       ->valueint;
    metadata.mvId       =           cJSON_GetObjectItem(json, "mvId")           ->valueint;
    metadata.flag       =           cJSON_GetObjectItem(json, "flag")           ->valueint;

    strcpy(metadata.musicName,      cJSON_GetObjectItem(json, "musicName")     ->valuestring);
    strcpy(metadata.album,          cJSON_GetObjectItem(json, "album")         ->valuestring);
    strcpy(metadata.albumPicDocId,  cJSON_GetObjectItem(json, "albumPicDocId") ->valuestring);
    strcpy(metadata.albumPic,       cJSON_GetObjectItem(json, "albumPic")      ->valuestring);
    strcpy(metadata.mp3DocId,       cJSON_GetObjectItem(json, "mp3DocId")      ->valuestring);
    strcpy(metadata.format,         cJSON_GetObjectItem(json, "format")        ->valuestring);

    struct cJSON* artist, *alias, *transNames;

    artist      = cJSON_GetObjectItem(json, "artist");
    alias       = cJSON_GetObjectItem(json, "alias");
    transNames  = cJSON_GetObjectItem(json, "transNames");

    for(int i = 0; i < cJSON_GetArraySize(artist); i++){
        strcpy(
        metadata.artist[i].Name, cJSON_GetArrayItem(cJSON_GetArrayItem(artist, i), 0)->valuestring);
        metadata.artist[i].Id =  cJSON_GetArrayItem(cJSON_GetArrayItem(artist, i), 1)->valueint;
    }
    for(int i = 0; i < cJSON_GetArraySize(alias); i++)
        strcpy(metadata.alias[i],       cJSON_GetArrayItem(alias, i)->valuestring);
    for(int i = 0; i < cJSON_GetArraySize(transNames); i++)
        strcpy(metadata.transNames[i],  cJSON_GetArrayItem(transNames, i)->valuestring);

    return metadata;
}

// Print main infomation for NCM
void PrintMetadata(struct Metadata dat){
    printf("Name:\n%20s\n", dat.musicName);
    printf("Author:\n");
    for(int i = 0; i < MULTILEN; i++)
        if(dat.artist[i].Id != 0)
            printf("%20s\t\t%d\n", dat.artist[i].Name, dat.artist[i].Id);
    printf("\n");
    printf("Album:\n%30s\n", dat.album);
    printf("Alias:\n");
    for(int i = 0; i < MULTILEN; i++)
        if(dat.alias[i][0] != 0)
            printf("\t%20s\n", dat.alias[i]);
    printf("TransNames:\n");
    for(int i = 0; i < MULTILEN; i++)
        if(dat.transNames[i][0] != 0)
            printf("\t%20s\n", dat.transNames[i]);
    printf("ID:\t\t%d\n", dat.musicId);
    printf("Album ID:\t%d\n", dat.albumId);
}    

