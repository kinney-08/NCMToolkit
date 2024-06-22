<h1 align="center">
   NCM Toolkit
</h1>
<p align="center">
   Solutions for analysing music acquired from NetEase Cloud Music.
</p>

> [!Tip]
> NCM stands for NetEase Cloud Music.

## Tools (*Publicly Available*)
* NCM file (*.ncm) decryptor
* 163 key reader (Not implemented)
* Lyrics downloader (Not implemented)

## Usage

As of the submission of this commit, I haven't made a proper GUI yet, as all the planned features of this application are not yet implemented.

So you can run this program from the command line.

Usage:
```shell
./NCMTool.exe NCMFile [-n output_path] [-d] 
```

Parameter:
- ``-n``: Name the output file with a specific name;
- ``-d``: Enable logging.

If nothing goes wrong, you may see the following text on your terminal:

```shell
> .\NCMTool.exe 'YOASOBI - Halzion.ncm' -d
Log Start...
[Log] RC4 Key:
[Log] Length = 128
[Log] Key= 273581793719377E7fT49x7dof9OKCgg9cdvhEuezy3iZCL1nFvBFd1T4uSktAJKmwZXsijPbijliionVUXXg9plTbXEclAE9Lb
[Log] Metadata Info:
[Log] Length= 598
[Log] Turned into GBK
[Log] Successfully Parsed into Metadata 
[Log] Album Cover:
[Log] Length= 606725
[Log] Successfully Loaded Music Cover.
[Log] Music:
[Log] Load block 0
[Log] Exactly 7933796 Bytes
[Log] Decrypted Music Data. 
[Info] Main Metadata
Name:
             Halzion
Author:
             YOASOBI            33927412

Album:
                      E-SIDE 2
Alias:
             ハルジオン
TransNames:
                   春紫菀
ID:             1994998091
Album ID:       154291740
[Log] Wrote Music to YOASOBI - Halzion.mp3
[Log] Wrote Cover to YOASOBI - Halzion.jpg
```

It will export the decrypted music files and album art to the current directory under the name "Author - Name".

There is currently no code implemented to add album art to music files. But as you can see, I've added id3v3lib to the project and the relevant code is being written ~~(why I don't use Python)~~. Before adding this feature, if you have ffmpeg installed on your computer, you can add the album art into the file with the following command.
```bash
ffmpeg -i [sourcefile].mp3 -i [coverfile] -map 0:a -map 1:v -map_metadata 0 -id3v2_version 3 -c:a copy -c:v copy [outputfile].mp3
```

Hopefully this project will save your Netease Cloud Music.


## Installation

You can get an executeable file by using the following commands:

```bash
mkdir ./build && cd ./build
cmake ..
make
```

If all goes well, you'll see NCMTool.exe in the ./build directory.

## About NCM File

NCM File = Netease Cloud Music Music File

### I. File Structure

| Data Type       | Size            | Description                                       |
|:---------------:|----------------:|:--------------------------------------------------|
|Magic Header     |10B              |File Header                                        |
|Key Length       |4B               |Length of AES128 encrypted RC4 Key (little-endian) |
|Key Data         |Key Length       |(See Below)                                        |
|Metadata Length  |4B               |Length of Metadata  (little-endian)                |
|Metadata         |Metadata Length  |(See Below)                                        |
|CRC              |4B               |Skip                                               |
|Gap              |4B               |Skip                                               |
|Cover Size       |4B               |Size of Cover Image                                |
|Cover            |Cover Size       |JFIF Picture                                       |
|Music            |-                |(See Below)                                        |

### II. Decryption

1. RC4 Key:
   * Decrypt Method:
     1. XOR ``0x64`` by byte
     2. AES decrypt
     3. Remove filling area
     4. Remove front 17 bytes:``neteasecloudmusic``
   * AES key: ``0x68,0x7A,0x48,0x52,0x41,0x6D,0x73，0x6F,0x35,0x6B,0x49,0x6E,0x62,0x61,0x78,0x57``
2. Metadata
   - Data Type: JSON
   - Decrypt Method:
     1. XOR 0x63 by byte
     2. Remove front 22 bytes
     3. Base64 decode
     4. AES decrypt
     5. Remove front 6 bytes
   - AES key: ``0x23,0x31,0x34,0x6C,0x6A,0x6B,0x5F,0x21,0x5C,0x5D,0x26,0x30,0x55,0x3C,0x27,0x28``
3. Music
   - Generate S-Box by RC4-KSA using RC4 Key
   - Decrypt by S-Box

### III. Metadata

Metadata is stored as compressed JSON text, typically has 15 key-value pairs.

|Key            |Value                      |Description                            |
|:-------------:|:--------------------------|:--------------------------------------|
|musicId        |Integer                    |Music ID in Netease Cloud Music        |
|musicName      |String                     |Music Name                             |
|artist         |List(List(String, Integer))|Name & ID of the authors               |
|albumId        |Integer                    |Album ID                               |
|album          |String                     |Album Name                             |
|albumPicDocId  |String                     |Album cover ID                         |
|albumPic       |String                     |URL pointing to the album cover        |
|bitrate        |Integer                    |Bitrate                                |
|mp3DocId       |String                     |~~I don't get it.~~                    |
|duration       |Integer                    |Music length, calculated by bitrate    |
|mvID           |Integer                    |~~I don't get it.~~                    |
|alias          |List(String)               |Music alias, often used as a subtitle  |
|transNames     |List(String)               |Music name in different languages      |
|format         |String                     |File extension of the decrypted music  |
|flag           |Integer                    |~~I don't get it.~~                    |

Example:
```json
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
```