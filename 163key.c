#include "163key.h"

#include <string.h>
#include <stdlib.h>

#include "NCMTool.h"
#include "metadata.h"
#include "base64.h"
#include "aes.h"

#ifdef WIN32
#include "utf8ToGbk.h"
#endif

// Function WIP
struct Metadata Parse163Key(unsigned char* dat, int len){
    int l;
    char* data = base64_decode(dat, len, &l);

    struct AES_ctx ctx;
    AES_init_ctx(&ctx, meta_key);
    int packSize = len / 16; 
    for (int i = 0; i < packSize; i++) AES_ECB_decrypt(&ctx, &data[i * 16]);

    // Parse into Metadata
#ifdef WIN32
    unsigned char * data_ = data;
	data = utf8ToGbk(data_, strlen(data_));
    free(data_);
#endif
    struct Metadata metadata;
    metadata = GetMetadata(cJSON_Parse(data + 6));
    LOG printf("Successfully Parsed into Metadata \n");
    free(data);

    return metadata;
}