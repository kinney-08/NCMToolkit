cmake_minimum_required(VERSION 3.5.0)
project(NCMTool VERSION 0.1.0 LANGUAGES C)

include_directories(include)

add_executable(NCMTool 163key.c aes.c base64.c cJSON.c main.c metadata.c ncm.c utf8ToGbk.c)

