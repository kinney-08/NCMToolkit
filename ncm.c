#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "aes.c"
#include "cJSON.c"
#include "base64.c"

#define STRINGLEN 100
#define MULTILEN 10

#define bool char

#ifdef WIN32
#include "utf8ToGbk.c"
#endif

/*
NCM
Netease Cloud Music Music File

I. File Structure

Data Type       Size            Description

Magic Header    10B             File Header
Key Length      4B              Length of AES128 encrypted RC4 Key (little-endian)
Key Data        Key Length      (See Below)
Metadata Length 4B              Length of Metadata  (little-endian)
Metadata        Metadata Length (See Below)
CRC             4B              Skip
Gap             4B              Skip
Cover Size      4B              Size of Cover Image
Cover           Cover Size      
Music           -               (See Below)

II. Decryption

1. RC4 Key:
    - XOR 0x64 by byte
    - AES decrypt
    - Remove filling area
    - Remove front 17 bytes ("neteasecloudmusic")

    AES key:
        0x68,0x7A,0x48,0x52,0x41,0x6D,0x73,0x6F,0x35,0x6B,0x49,0x6E,0x62,0x61,0x78,0x57

2. Metadata
    Data Type: JSON

    - XOR 0x63 by byte
    - Remove front 22 bytes
    - Base64 decode
    - AES decrypt
    - Remove front 6 bytes

    AES key:
        0x23,0x31,0x34,0x6C,0x6A,0x6B,0x5F,0x21,0x5C,0x5D,0x26,0x30,0x55,0x3C,0x27,0x28

3. Music
    
    - Generate S-Box by RC4-KSA using RC4 Key 
    - Decrypt by S-Box

III. Metadata
Example:
{
"musicId":          1991012773,
"musicName":        "ギターと孤独と蒼い惑星",
"artist":           [["結束バンド", 54103171]],
"albumId":          153542094,
"album":            "ギターと孤独と蒼い惑星",
"albumPicDocId":    "109951167983448236",
"albumPic":         "https://p4.music.126.net/rfstzrVK05hCPjU-4mzSFA==/109951167983448236.jpg",
"bitrate":          320000,
"mp3DocId":         "f481d20151f01d5d681d2768d753ad64",
"duration":         229015,
"mvId":             0,
"alias":            ["TV动画《孤独摇滚！》插曲"],
"transNames":       [],
"format":           "mp3",
"flag":             4
}
*/

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

bool logging = false;
#define LOG if(logging) printf("[Log] "), 

struct NCM{
    struct Metadata metadata;
    cJSON* meta_json;
    unsigned char * img;
    unsigned char * music;
    int len_img;
    int len_music;
    int errlevel;
};

void Del(struct NCM ncm){
    free(ncm.music);
    free(ncm.img);
}

void swap(unsigned char * a, unsigned char * b)
{
	unsigned char  t = *a;
	*a = *b;
	*b = t;
}

// S-Box by RC4-KSA using RC4 Key
// @param s S-Box
// @param key Key
// @param len Length of key
void rc4Init(unsigned char * s, const unsigned char * key, int len){
    unsigned char  T[256] = { 0 };

    for (int i = 0; i < 256; i++) {
        s[i] = i;
        T[i] = key[i % len];
    }

    for (int i = 0; i < 256; i++) {
        int j = (j + s[i] + T[i]) % 256;
        swap(s + i, s + j);
    }
}

// NCM Music Decrypt Function
// @param s S-Box
// @param data Encrypt/Decrypt Data
// @param len Length of data
void rc4PRGA(unsigned char * s, unsigned char  * data, int len){
    for (int idx = 0; idx < len; idx++){
        int i = (idx + 1)       % 256;
        int j = (i + s[i])      % 256;
        int k = (s[i] + s[j])   % 256;
        data[idx] ^= s[k];
    }
}


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
void PrintMetadata(struct NCM ncm){
    struct Metadata dat = ncm.metadata;
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


// NCM File Decrypt Main Function
// @param f NCM File 
// @return NCM Data
struct NCM DecryptNcm(FILE* f){
    struct NCM ncm;

    unsigned char  buf[16];
    int len = 0;

    unsigned char  meta_key[] = { 0x23,0x31,0x34,0x6C,0x6A,0x6B,0x5F,0x21,0x5C,0x5D,0x26,0x30,0x55,0x3C,0x27,0x28 };
	unsigned char  core_key[] = { 0x68,0x7A,0x48,0x52,0x41,0x6D,0x73,0x6F,0x35,0x6B,0x49,0x6E,0x62,0x61,0x78,0x57 };
	
    /* RC4 Key */
    LOG printf("RC4 Key: \n");

    // Get Length
    fseek(f, 10, SEEK_CUR);
    fread(buf, 1, 4, f);
    len = (buf[3] << 8 | buf[2]) << 16 | (buf[1] << 8 | buf[0]);
    LOG printf("Length = %d\n", len);

    // Read Data
    unsigned char * rc4Key = (unsigned char *)malloc(sizeof(unsigned char ) * len);
    fread(rc4Key, 1, len, f);

    // Decrypt
    for (int i = 0; i < len; i++) rc4Key[i] ^= 0x64;

    // Applyed AES-ECB and Pkcs7padding filling method 
    struct AES_ctx ctx;
    AES_init_ctx(&ctx, core_key); // Decrypt with AES, corekey
    int packSize = len / 16; 
    for (int i = 0; i < packSize; i++) AES_ECB_decrypt(&ctx, &rc4Key[i * 16]);
    
    // Remove Filled Area
    int pad = rc4Key[len - 1];
    rc4Key[len - pad] = '\0';
    rc4Key += 17;
    LOG printf("Key= %s\n", rc4Key);

    /* Music Info (Meta Data) */
    LOG printf("Metadata Info: \n");
    // Get Length
    fread(buf, 1, 4, f);
    len = ((buf[3] << 8 | buf[2]) << 16) | (buf[1] << 8 | buf[0]);
    LOG printf("Length= %d \n", len);
    // Read data
    unsigned char * meta = (unsigned char *)malloc(sizeof(unsigned char ) * len);
    fread(meta, 1, len, f);

    // Get Raw JSON Data
    for (int i = 0; i < len; i++) meta[i] ^= 0x63;
    int act = 0;
    unsigned char * data = base64_decode(&meta[22], len - 22, &act);	// Decode with BASE64
	AES_init_ctx(&ctx, meta_key); // Decrypt with AES, metakey
    packSize = act / 16;
    for (int i = 0; i < packSize; i++) AES_ECB_decrypt(&ctx, &data[i * 16]);
    
    // Remove Filled Area
    pad = data[act - 1];
    data[act - pad] = '\0';

    // Parse into Metadata
#ifdef WIN32
    unsigned char * data_ = data;
	data = utf8ToGbk(data_, strlen(data_));
    free(data_);
    LOG printf("Turned into GBK\n");
#endif
    ncm.metadata = GetMetadata(cJSON_Parse(data + 6));
    LOG printf("Successfully Parsed into Metadata \n");

    /* Album Cover */
    LOG printf("Album Cover: \n");

    // Get Length
    fseek(f, 9, SEEK_CUR);
    fread(buf, 1, 4, f);
    len = (buf[3] << 8 | buf[2]) << 16 | (buf[1] << 8 | buf[0]);
    LOG printf("Length= %d \n", len);

    // Read Data
    ncm.img = (unsigned char *)malloc(sizeof(unsigned char ) * len);
    fread(ncm.img, 1, len, f);

    ncm.len_img = len;
    LOG printf("Successfully Loaded Music Cover. \n");

    /* Music */
    LOG printf("Music: \n");

    int offset= 1024 * 1024 * 10; // Read 10MB one time
	int total = 0;
	int reSize = offset;
	ncm.music = (unsigned char *)malloc(offset);

    while (!feof(f))
	{
        LOG printf("Load block %d \n", (int)(total / offset));
		len = fread(ncm.music + total, 1, offset, f);	// Read 10MB
		total += len;
		reSize += offset;
	    ncm.music = realloc(ncm.music, reSize);	    // Enlarge
	}

    LOG printf("Exactly %d Bytes \n", len);

    unsigned char  sBox[256] = {0};
    rc4Init(sBox, rc4Key, strlen(rc4Key)); // Use RC4 to Initialize
    rc4PRGA(sBox, ncm.music, total);

    ncm.len_music = total;

    LOG printf("Decrypted Music Data. \n", len);

    printf("[Info] Main Metadata\n");
    PrintMetadata(ncm);

    free(data);
	free(meta);

    fseek(f, 0, SEEK_SET);

    ncm.errlevel = 0;
    return ncm;
}

void AuthorStr(struct NCM ncm, char* author){
    strcat(author, ncm.metadata.artist[0].Name);
    for(int i = 1; i < MULTILEN; i++)
        if(ncm.metadata.artist[i].Id != 0){
            strcat(author, ",");
            strcat(author, ncm.metadata.artist[i].Name);
        }
}

void FileName(struct NCM ncm, char* filename){
    unsigned char author[STRINGLEN * 2];
    memset(author, 0, sizeof(author));
    AuthorStr(ncm, author);
	sprintf(filename, "%s - %s", author, ncm.metadata.musicName);
}

void WriteMusic(struct NCM ncm, char* filename){
    unsigned char saveFileName[STRINGLEN * 4]; 
    sprintf(saveFileName, "%s.%s", filename, ncm.metadata.format);
    FILE* of = fopen(saveFileName, "wb");
    fwrite(ncm.music, 1, ncm.len_music, of);
    fclose(of);
    LOG printf("Wrote Music to %s\n", saveFileName);
}

void WriteCover(struct NCM ncm, char* filename){
    unsigned char saveFileName[STRINGLEN * 4]; 
    sprintf(saveFileName, "%s.jpg", filename);
    FILE* of = fopen(saveFileName, "wb");
    fwrite(ncm.img, 1, ncm.len_img, of);
    fclose(of);
    LOG printf("Wrote Cover to %s\n", saveFileName);
}

