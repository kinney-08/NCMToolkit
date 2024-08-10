#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ncm.h"

#include "NCMTool.h"
#include "aes.h"
#include "cJSON.h"
#include "base64.h"
#include "metadata.h"

#ifdef WIN32
#include "utf8ToGbk.h"
#endif

unsigned char meta_key[] = { 0x23,0x31,0x34,0x6C,0x6A,0x6B,0x5F,0x21,0x5C,0x5D,0x26,0x30,0x55,0x3C,0x27,0x28 };
unsigned char core_key[] = { 0x68,0x7A,0x48,0x52,0x41,0x6D,0x73,0x6F,0x35,0x6B,0x49,0x6E,0x62,0x61,0x78,0x57 };

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


// NCM File Decrypt Main Function
// @param f NCM File 
// @return NCM Data
struct NCM DecryptNcm(FILE* f){
    struct NCM ncm;

    unsigned char  buf[16];
    int len = 0;

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
    ncm.meta_json = cJSON_Parse(data + 6);
    ncm.metadata = GetMetadata(ncm.meta_json);
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

    // LOG printf("Exactly %d Bytes \n", len); // BUG

    unsigned char  sBox[256] = {0};
    rc4Init(sBox, rc4Key, strlen(rc4Key)); // Use RC4 to Initialize
    rc4PRGA(sBox, ncm.music, total);

    ncm.len_music = total;

    LOG printf("Decrypted Music Data. \n");

    printf("[Info] Main Metadata\n");
    PrintMetadata(ncm.metadata);

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

void WriteMetadata(struct NCM ncm, char* filename){
    unsigned char saveFileName[STRINGLEN * 4]; 
    sprintf(saveFileName, "%s.json", filename);
    FILE* of = fopen(saveFileName, "wb");
    char* data = cJSON_Print(ncm.meta_json);
    fwrite(data, 1, strlen(data), of);
    fclose(of);
    LOG printf("Wrote Metadata to %s\n", saveFileName);
}

