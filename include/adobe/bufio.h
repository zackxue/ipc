#ifndef __BUFIO__
#define __BUFIO__

typedef signed char int8_t;
typedef unsigned char   uint8_t;
typedef short  int16_t;
typedef unsigned short  uint16_t;
typedef int  int32_t;
typedef unsigned   uint32_t;
typedef long long  int64_t;
typedef unsigned long long   uint64_t;


typedef struct {
    unsigned char *buffer;
    int buffer_size;
    unsigned char *buf_ptr, *buf_end;
    uint32_t pos;
	uint32_t actlen;
} BufIO;

void bufio_init(BufIO* bufio, void* buf, size_t buf_size);

int init_buffer(BufIO *s,int size);
void bufio_put_byte(BufIO *s, int b);
void bufio_put_buffer(BufIO *s, const unsigned char *buf, int size);
void bufio_put_le64(BufIO *s, uint64_t val);
void bufio_put_be64(BufIO *s, uint64_t val);
void bufio_put_le32(BufIO *s, unsigned int val);
void bufio_put_be32(BufIO *s, unsigned int val);
void bufio_put_le24(BufIO *s, unsigned int val);
void bufio_put_be24(BufIO *s, unsigned int val);
void bufio_put_le16(BufIO *s, unsigned int val);
void bufio_put_be16(BufIO *s, unsigned int val);
void bufio_put_tag(BufIO *s, const char *tag);
void bufio_put_strz(BufIO *s, const char *buf);
int64_t bufio_url_fseek(BufIO *s, int64_t offset, int whence);
void bufio_url_fskip(BufIO *s, int64_t offset);
int64_t bufio_url_ftell(BufIO *s);
int64_t bufio_url_fsize(BufIO *s);
int bufio_url_feof(BufIO *s);
int bufio_url_ferror(BufIO *s);
void bufio_put_amf_double(BufIO *pb, double d);
void bufio_put_amf_string(BufIO *pb, const char *str);
void bufio_put_amf_bool(BufIO *pb, int b);

void bufio_print(BufIO *pb);

#endif
