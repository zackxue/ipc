
#ifndef __BASE64_H__
#define __BASE64_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern int base64_encode(const void*in_str, void* out_str, ssize_t length);
extern int base64_decode(const void* in_str, void* out_str, ssize_t length);



//#define _TEST_BASE64
#ifdef _TEST_BASE64
int main(int argc,char ** argv)
{
    char data[]={0x01,0x02,0x03,0x04,0x00,0x00,0x35,0x00};
    char *encoded_buf;
    char *decoded_buf;

    // encode
    int encoded_buf_len = (sizeof(data) / 3 + 1) * 4;
    encoded_buf = (char *)malloc(encoded_buf_len);
    base64_encode(data, encoded_buf, sizeof(data));
    printf("encoded text:%s\n", encoded_buf);

    // decode
    decoded_buf = (char *)malloc(encoded_buf_len / 4 * 3);
    int dec_len = base64_decode((char *)encoded_buf, decoded_buf, encoded_buf_len);
    int i;
    printf("decoded data:");
    for (i = 0; i < dec_len; i++)
    {
        printf(" %02x",decoded_buf[i]);
    }
    printf("\n");
    

    // free
    free(encoded_buf);
    free(decoded_buf);

    return 0;
}
#endif

#ifdef __cplusplus
};
#endif
#endif //__BASE64_H__

