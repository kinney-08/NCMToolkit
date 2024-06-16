<h1 align="center">
   NCM Toolkit
</h1>
<p align="center">
   Solutions for analysing music acquired from NetEase Cloud Music.
</p>

## Tools (*Publicly Available*)

- NCM file (*.ncm) decryptor
- 163 key reader
- Lyrics downloader

> [!Tip]
> NCM stands for NetEase Cloud Music.


## I. File Structure

| Data Type    | Size        | Description                                        |
|--------------|-------------|----------------------------------------------------|
| Magic Header | 10B         | File Header                                        |
| Key Length   | 4B          | Length of AES128 encrypted RC4 Key (little-endian) |
| Key Data     | Key Length  | (See Below)                                        |
| Metadata     | 4B          | Length of Metadata (little-endian)                 |
| CRC          | 4B          |                                                    |
| Gap          | 4B          |                                                    |
| Cover Size   | 4B          | Size of Cover Image                                |
| Music        | (See Below) |                                                    |


## II. Decryption

**1. RC4 Key:**

   - Decrypt Method:
     - XOR ``0x64`` by byte
     - AES decrypt
     - Remove filling area
     - Remove front 17 bytes:``neteasecloudmusic``
   - AES key: ``0x68,0x7A,0x48,0x52,0x41,0x6D,0x73，0x6F,0x35,0x6B,0x49,0x6E,0x62,0x61,0x78,0x57``


**2. Metadata:**
   
   - Data Type: JSON
   - Decrypt Method:
     - XOR 0x63 by byte
     - Remove front 22 bytes
     - Base64 decode
     - AES decrypt
     - Remove front 6 bytes
   - AES key: ``0x23,0x31,0x34,0x6C,0x6A,0x6B,0x5F,0x21,0x5C,0x5D,0x26,0x30,0x55,0x3C,0x27,0x28``


**3. Music:**

   - Generate S-Box by RC4-KSA using RC4 Key
   - Decrypt by S-Box


## III. Metadata

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
