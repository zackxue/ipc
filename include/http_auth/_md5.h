#ifndef __MD5_H__
#define __MD5_H__

typedef unsigned int u32;
typedef unsigned char u8;

typedef struct MD5Context {
	u32 buf[4];
	u32 bits[2];
	u8 in[64];
}MD5_CTX;

void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, unsigned char const *buf,
	       unsigned len);
void MD5Final(unsigned char digest[16], struct MD5Context *context);
void MD5Transform(u32 buf[4], u32 const in[16]);

#endif 

