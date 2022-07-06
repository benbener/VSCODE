/**
 * @file base64.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-07-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef unsigned char uint8;
typedef unsigned long uint32;

static uint8 alphabet_map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static uint8 reverse_map[] = 
{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62,  255, 255, 255, 63,
    52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  255, 255, 255, 255, 255, 255,
    255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  255, 255, 255, 255, 255,
    255, 26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
    41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  255, 255, 255, 255, 255
};

/**
 * @brief 
 * 
 * @param data 
 * @param len 
 * @param base64 
 * @return uint32 
 */
uint32 encode(const uint8* data,uint32 len,uint8* base64)
{
    uint32 base64_len = 0;
    for (uint32 i = 0; i+3 <= len; i+=3)
    {
        base64[base64_len++] = alphabet_map[data[i] >> 2];                             //取出第一个字符的前6位并找出对应的结果字符
        base64[base64_len++] = alphabet_map[((data[i] << 4) & 0x30) | (data[i+1] >> 4)];     //将第一个字符的后2位与第二个字符的前4位进行组合并找到对应的结果字符
        base64[base64_len++] = alphabet_map[((data[i+1] << 2) & 0x3c) | (data[i+2] >> 6)];   //将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符
        base64[base64_len++] = alphabet_map[data[i+2] & 0x3f]; 
    }
    
    int tail = len % 3;
    if (tail == 1)
    {
        base64[base64_len++] = alphabet_map[data[len-1] >> 2];
        base64[base64_len++] = alphabet_map[(data[len-1] << 4) & 0x30];
        base64[base64_len++] = '=';
        base64[base64_len++] = '=';
    }
    else if (tail == 2)
    {
        base64[base64_len++] = alphabet_map[data[len-2] >> 2];
        base64[base64_len++] = alphabet_map[(data[len-2] << 4) & 0x30 | (data[len-1] >> 4)];
        base64[base64_len++] = alphabet_map[(data[len-1] << 2) & 0x3c];
        base64[base64_len++] = '=';
    }

    return base64_len;
}

uint32 decode(const uint8* base64,uint32 base64_len,uint8 *plain)
{
    assert((base64_len & 0x03) == 0);

    uint32 plain_len = 0;
    uint8 quad[4];
    for (uint32 i = 0; i < base64_len; i+=4)
    {
        for (uint32 k = 0; k < 4; k++)
        {
            quad[k] = reverse_map[base64[i+k]];
        }

        assert(quad[0] < 64 && quad[1] < 64);

        plain[plain_len++] = (quad[0] << 2) | (quad[1] >> 4);

        if (quad[2] >= 64)
        {
            break;
        }
        else if (quad[3] >= 64)
        {
            plain[plain_len++] = (quad[1] << 4) | (quad[2] >> 2);
            break;
        }
        else
        {
            plain[plain_len++] = (quad[1] << 4) | (quad[2] >> 2);
            plain[plain_len++] = (quad[2] << 6) | quad[3];
        }
    }
    
    return plain_len;
}

int main()
{
    /// encode test
    char input[256];
    do
    {
        strset(input,0);
        printf("please input (q to exit): ");
        scanf("%s",input);
        uint8 *data = (uint8*)input;
        uint32 len = (uint32)strlen(input);

        uint8 base64_buffer[1024],decode_base64_buffer[4096];
        uint32 size = encode(data,len,base64_buffer);
        base64_buffer[size] = 0;
        printf("toBase64: %s len = %d\n",base64_buffer,size);

        size = decode(base64_buffer,size,decode_base64_buffer);
        decode_base64_buffer[size] = 0;
        printf("decode base64: %s\n",decode_base64_buffer);
    } while (strcmp(input,"q") != 0); 

    /// decode test

    return 0;
}
