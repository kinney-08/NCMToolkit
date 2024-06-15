#include <stdio.h>

#include "NCMTool.h"
#include "ncm.h"

#define true 1

unsigned char* MusicName[100];
unsigned char name[1000];

bool logging;

int main(int argc, char** argv)
{	
    if (argc == 1){
        printf("NCM Decryptor\n");
        return 0;
    }

    FILE* f = fopen(argv[1], "rb");
    if (!f) {  
        printf("[Error] No such file: %s\n", argv[1]);
        return -1;
    }    

    for (int i = 2; i < argc; i++){
        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--name") == 0) {
            if (i + 1 >= argc || argv[i + 1][0] == '-')
                printf("[Warning] Invalid Filename\n");
            else
                strcpy(name, argv[i += 1]);
        }
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--detail") == 0){
            logging = true;
            printf("Log Start...\n");
        }
    }
    
    struct NCM music = DecryptNcm(f);
    
    if (strlen(name) == 0)
        FileName(music, name);
    WriteMusic(music, name);
    WriteCover(music, name);
    fclose(f);
    Del(music);
    return 0;
}
