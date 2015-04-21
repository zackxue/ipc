/*
 ============================================================================
 Name        : md5sum.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

typedef struct _md5_ctx_t_ {
	uint32_t A;
	uint32_t B;
	uint32_t C;
	uint32_t D;
	uint32_t total[2];
	uint32_t buflen;
	char buffer[128];
} md5_ctx_t;

# define MD5_SIZE_VS_SPEED (1)

# define FF(b, c, d) (d ^ (b & (c ^ d)))
# define FG(b, c, d) FF (d, b, c)
# define FH(b, c, d) (b ^ c ^ d)
# define FI(b, c, d) (c ^ (b | ~d))
# define SWAP(n) (n)

#define bb_perror_msg printf
//#define ENABLE_FEATURE_MD5_SHA1_SUM_CHECK 1
//
//
//#define FLAG_SILENT	1
//#define FLAG_CHECK	2
//#define FLAG_WARN	4

static void md5_hash_block(const void *buffer, size_t len, md5_ctx_t *ctx)
{
	uint32_t correct_words[16];
	const uint32_t *words = buffer;
	size_t nwords = len / sizeof(uint32_t);
	const uint32_t *endp = words + nwords;

# if MD5_SIZE_VS_SPEED > 0
	static const uint32_t C_array[] = {
		/* round 1 */
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		/* round 2 */
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x2441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		/* round 3 */
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x4881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		/* round 4 */
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
	};

	static const char P_array[] = {
#  if MD5_SIZE_VS_SPEED > 1
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,	/* 1 */
#  endif	/* MD5_SIZE_VS_SPEED > 1 */
		1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12,	/* 2 */
		5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2,	/* 3 */
		0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9	/* 4 */
	};

#  if MD5_SIZE_VS_SPEED > 1
	static const char S_array[] = {
		7, 12, 17, 22,
		5, 9, 14, 20,
		4, 11, 16, 23,
		6, 10, 15, 21
	};
#  endif	/* MD5_SIZE_VS_SPEED > 1 */
# endif

	uint32_t A = ctx->A;
	uint32_t B = ctx->B;
	uint32_t C = ctx->C;
	uint32_t D = ctx->D;

	/* First increment the byte count.  RFC 1321 specifies the possible
	   length of the file up to 2^64 bits.  Here we only compute the
	   number of bytes.  Do a double word increment.  */
	ctx->total[0] += len;
	if (ctx->total[0] < len)
		++ctx->total[1];

	/* Process all bytes in the buffer with 64 bytes in each round of
	   the loop.  */
	while (words < endp) {
		uint32_t *cwp = correct_words;
		uint32_t A_save = A;
		uint32_t B_save = B;
		uint32_t C_save = C;
		uint32_t D_save = D;

# if MD5_SIZE_VS_SPEED > 1
#  define CYCLIC(w, s) (w = (w << s) | (w >> (32 - s)))

		const uint32_t *pc;
		const char *pp;
		const char *ps;
		int i;
		uint32_t temp;

		for (i = 0; i < 16; i++) {
			cwp[i] = SWAP(words[i]);
		}
		words += 16;

#  if MD5_SIZE_VS_SPEED > 2
		pc = C_array;
		pp = P_array;
		ps = S_array - 4;

		for (i = 0; i < 64; i++) {
			if ((i & 0x0f) == 0)
				ps += 4;
			temp = A;
			switch (i >> 4) {
			case 0:
				temp += FF(B, C, D);
				break;
			case 1:
				temp += FG(B, C, D);
				break;
			case 2:
				temp += FH(B, C, D);
				break;
			case 3:
				temp += FI(B, C, D);
			}
			temp += cwp[(int) (*pp++)] + *pc++;
			CYCLIC(temp, ps[i & 3]);
			temp += B;
			A = D;
			D = C;
			C = B;
			B = temp;
		}
#  else
		pc = C_array;
		pp = P_array;
		ps = S_array;

		for (i = 0; i < 16; i++) {
			temp = A + FF(B, C, D) + cwp[(int) (*pp++)] + *pc++;
			CYCLIC(temp, ps[i & 3]);
			temp += B;
			A = D;
			D = C;
			C = B;
			B = temp;
		}

		ps += 4;
		for (i = 0; i < 16; i++) {
			temp = A + FG(B, C, D) + cwp[(int) (*pp++)] + *pc++;
			CYCLIC(temp, ps[i & 3]);
			temp += B;
			A = D;
			D = C;
			C = B;
			B = temp;
		}
		ps += 4;
		for (i = 0; i < 16; i++) {
			temp = A + FH(B, C, D) + cwp[(int) (*pp++)] + *pc++;
			CYCLIC(temp, ps[i & 3]);
			temp += B;
			A = D;
			D = C;
			C = B;
			B = temp;
		}
		ps += 4;
		for (i = 0; i < 16; i++) {
			temp = A + FI(B, C, D) + cwp[(int) (*pp++)] + *pc++;
			CYCLIC(temp, ps[i & 3]);
			temp += B;
			A = D;
			D = C;
			C = B;
			B = temp;
		}

#  endif	/* MD5_SIZE_VS_SPEED > 2 */
# else
		/* First round: using the given function, the context and a constant
		   the next context is computed.  Because the algorithms processing
		   unit is a 32-bit word and it is determined to work on words in
		   little endian byte order we perhaps have to change the byte order
		   before the computation.  To reduce the work for the next steps
		   we store the swapped words in the array CORRECT_WORDS.  */

#  define OP(a, b, c, d, s, T)	\
      do	\
	{	\
	  a += FF (b, c, d) + (*cwp++ = SWAP (*words)) + T;	\
	  ++words;	\
	  CYCLIC (a, s);	\
	  a += b;	\
	}	\
      while (0)

		/* It is unfortunate that C does not provide an operator for
		   cyclic rotation.  Hope the C compiler is smart enough.  */
		/* gcc 2.95.4 seems to be --aaronl */
#  define CYCLIC(w, s) (w = (w << s) | (w >> (32 - s)))

		/* Before we start, one word to the strange constants.
		   They are defined in RFC 1321 as

		   T[i] = (int) (4294967296.0 * fabs (sin (i))), i=1..64
		 */

#  if MD5_SIZE_VS_SPEED == 1
		const uint32_t *pc;
		const char *pp;
		int i;
#  endif	/* MD5_SIZE_VS_SPEED */

		/* Round 1.  */
#  if MD5_SIZE_VS_SPEED == 1
		pc = C_array;
		for (i = 0; i < 4; i++) {
			OP(A, B, C, D, 7, *pc++);
			OP(D, A, B, C, 12, *pc++);
			OP(C, D, A, B, 17, *pc++);
			OP(B, C, D, A, 22, *pc++);
		}
#  else
		OP(A, B, C, D, 7, 0xd76aa478);
		OP(D, A, B, C, 12, 0xe8c7b756);
		OP(C, D, A, B, 17, 0x242070db);
		OP(B, C, D, A, 22, 0xc1bdceee);
		OP(A, B, C, D, 7, 0xf57c0faf);
		OP(D, A, B, C, 12, 0x4787c62a);
		OP(C, D, A, B, 17, 0xa8304613);
		OP(B, C, D, A, 22, 0xfd469501);
		OP(A, B, C, D, 7, 0x698098d8);
		OP(D, A, B, C, 12, 0x8b44f7af);
		OP(C, D, A, B, 17, 0xffff5bb1);
		OP(B, C, D, A, 22, 0x895cd7be);
		OP(A, B, C, D, 7, 0x6b901122);
		OP(D, A, B, C, 12, 0xfd987193);
		OP(C, D, A, B, 17, 0xa679438e);
		OP(B, C, D, A, 22, 0x49b40821);
#  endif	/* MD5_SIZE_VS_SPEED == 1 */

		/* For the second to fourth round we have the possibly swapped words
		   in CORRECT_WORDS.  Redefine the macro to take an additional first
		   argument specifying the function to use.  */
#  undef OP
#  define OP(f, a, b, c, d, k, s, T)	\
      do	\
	{	\
	  a += f (b, c, d) + correct_words[k] + T;	\
	  CYCLIC (a, s);	\
	  a += b;	\
	}	\
      while (0)

		/* Round 2.  */
#  if MD5_SIZE_VS_SPEED == 1
		pp = P_array;
		for (i = 0; i < 4; i++) {
			OP(FG, A, B, C, D, (int) (*pp++), 5, *pc++);
			OP(FG, D, A, B, C, (int) (*pp++), 9, *pc++);
			OP(FG, C, D, A, B, (int) (*pp++), 14, *pc++);
			OP(FG, B, C, D, A, (int) (*pp++), 20, *pc++);
		}
#  else
		OP(FG, A, B, C, D, 1, 5, 0xf61e2562);
		OP(FG, D, A, B, C, 6, 9, 0xc040b340);
		OP(FG, C, D, A, B, 11, 14, 0x265e5a51);
		OP(FG, B, C, D, A, 0, 20, 0xe9b6c7aa);
		OP(FG, A, B, C, D, 5, 5, 0xd62f105d);
		OP(FG, D, A, B, C, 10, 9, 0x02441453);
		OP(FG, C, D, A, B, 15, 14, 0xd8a1e681);
		OP(FG, B, C, D, A, 4, 20, 0xe7d3fbc8);
		OP(FG, A, B, C, D, 9, 5, 0x21e1cde6);
		OP(FG, D, A, B, C, 14, 9, 0xc33707d6);
		OP(FG, C, D, A, B, 3, 14, 0xf4d50d87);
		OP(FG, B, C, D, A, 8, 20, 0x455a14ed);
		OP(FG, A, B, C, D, 13, 5, 0xa9e3e905);
		OP(FG, D, A, B, C, 2, 9, 0xfcefa3f8);
		OP(FG, C, D, A, B, 7, 14, 0x676f02d9);
		OP(FG, B, C, D, A, 12, 20, 0x8d2a4c8a);
#  endif	/* MD5_SIZE_VS_SPEED == 1 */

		/* Round 3.  */
#  if MD5_SIZE_VS_SPEED == 1
		for (i = 0; i < 4; i++) {
			OP(FH, A, B, C, D, (int) (*pp++), 4, *pc++);
			OP(FH, D, A, B, C, (int) (*pp++), 11, *pc++);
			OP(FH, C, D, A, B, (int) (*pp++), 16, *pc++);
			OP(FH, B, C, D, A, (int) (*pp++), 23, *pc++);
		}
#  else
		OP(FH, A, B, C, D, 5, 4, 0xfffa3942);
		OP(FH, D, A, B, C, 8, 11, 0x8771f681);
		OP(FH, C, D, A, B, 11, 16, 0x6d9d6122);
		OP(FH, B, C, D, A, 14, 23, 0xfde5380c);
		OP(FH, A, B, C, D, 1, 4, 0xa4beea44);
		OP(FH, D, A, B, C, 4, 11, 0x4bdecfa9);
		OP(FH, C, D, A, B, 7, 16, 0xf6bb4b60);
		OP(FH, B, C, D, A, 10, 23, 0xbebfbc70);
		OP(FH, A, B, C, D, 13, 4, 0x289b7ec6);
		OP(FH, D, A, B, C, 0, 11, 0xeaa127fa);
		OP(FH, C, D, A, B, 3, 16, 0xd4ef3085);
		OP(FH, B, C, D, A, 6, 23, 0x04881d05);
		OP(FH, A, B, C, D, 9, 4, 0xd9d4d039);
		OP(FH, D, A, B, C, 12, 11, 0xe6db99e5);
		OP(FH, C, D, A, B, 15, 16, 0x1fa27cf8);
		OP(FH, B, C, D, A, 2, 23, 0xc4ac5665);
#  endif	/* MD5_SIZE_VS_SPEED == 1 */

		/* Round 4.  */
#  if MD5_SIZE_VS_SPEED == 1
		for (i = 0; i < 4; i++) {
			OP(FI, A, B, C, D, (int) (*pp++), 6, *pc++);
			OP(FI, D, A, B, C, (int) (*pp++), 10, *pc++);
			OP(FI, C, D, A, B, (int) (*pp++), 15, *pc++);
			OP(FI, B, C, D, A, (int) (*pp++), 21, *pc++);
		}
#  else
		OP(FI, A, B, C, D, 0, 6, 0xf4292244);
		OP(FI, D, A, B, C, 7, 10, 0x432aff97);
		OP(FI, C, D, A, B, 14, 15, 0xab9423a7);
		OP(FI, B, C, D, A, 5, 21, 0xfc93a039);
		OP(FI, A, B, C, D, 12, 6, 0x655b59c3);
		OP(FI, D, A, B, C, 3, 10, 0x8f0ccc92);
		OP(FI, C, D, A, B, 10, 15, 0xffeff47d);
		OP(FI, B, C, D, A, 1, 21, 0x85845dd1);
		OP(FI, A, B, C, D, 8, 6, 0x6fa87e4f);
		OP(FI, D, A, B, C, 15, 10, 0xfe2ce6e0);
		OP(FI, C, D, A, B, 6, 15, 0xa3014314);
		OP(FI, B, C, D, A, 13, 21, 0x4e0811a1);
		OP(FI, A, B, C, D, 4, 6, 0xf7537e82);
		OP(FI, D, A, B, C, 11, 10, 0xbd3af235);
		OP(FI, C, D, A, B, 2, 15, 0x2ad7d2bb);
		OP(FI, B, C, D, A, 9, 21, 0xeb86d391);
#  endif	/* MD5_SIZE_VS_SPEED == 1 */
# endif	/* MD5_SIZE_VS_SPEED > 1 */

		/* Add the starting values of the context.  */
		A += A_save;
		B += B_save;
		C += C_save;
		D += D_save;
	}

	/* Put checksum in context given as argument.  */
	ctx->A = A;
	ctx->B = B;
	ctx->C = C;
	ctx->D = D;
}

static void md5_begin(md5_ctx_t *ctx)
{
	ctx->A = 0x67452301;
	ctx->B = 0xefcdab89;
	ctx->C = 0x98badcfe;
	ctx->D = 0x10325476;

	ctx->total[0] = ctx->total[1] = 0;
	ctx->buflen = 0;
}

static void md5_hash_bytes(const void *buffer, size_t len, md5_ctx_t *ctx)
{
	/* When we already have some bits in our internal buffer concatenate
	   both inputs first.  */
	if (ctx->buflen != 0) {
		size_t left_over = ctx->buflen;
		size_t add = 128 - left_over > len ? len : 128 - left_over;

		memcpy(&ctx->buffer[left_over], buffer, add);
		ctx->buflen += add;

		if (left_over + add > 64) {
			md5_hash_block(ctx->buffer, (left_over + add) & ~63, ctx);
			/* The regions in the following copy operation cannot overlap.  */
			memcpy(ctx->buffer, &ctx->buffer[(left_over + add) & ~63],
				   (left_over + add) & 63);
			ctx->buflen = (left_over + add) & 63;
		}

		buffer = (const char *) buffer + add;
		len -= add;
	}

	/* Process available complete blocks.  */
	if (len > 64) {
		md5_hash_block(buffer, len & ~63, ctx);
		buffer = (const char *) buffer + (len & ~63);
		len &= 63;
	}

	/* Move remaining bytes in internal buffer.  */
	if (len > 0) {
		memcpy(ctx->buffer, buffer, len);
		ctx->buflen = len;
	}
}

static void md5_hash(const void *data, size_t length, md5_ctx_t *ctx)
{
	if (length % 64 == 0) {
		md5_hash_block(data, length, ctx);
	} else {
		md5_hash_bytes(data, length, ctx);
	}
}

static void *md5_end(void *resbuf, md5_ctx_t *ctx)
{
	/* Take yet unprocessed bytes into account.  */
	uint32_t bytes = ctx->buflen;
	size_t pad;

	/* Now count remaining bytes.  */
	ctx->total[0] += bytes;
	if (ctx->total[0] < bytes)
		++ctx->total[1];

	pad = bytes >= 56 ? 64 + 56 - bytes : 56 - bytes;
# if MD5_SIZE_VS_SPEED > 0
	memset(&ctx->buffer[bytes], 0, pad);
	ctx->buffer[bytes] = 0x80;
# else
	memcpy(&ctx->buffer[bytes], fillbuf, pad);
# endif	/* MD5_SIZE_VS_SPEED > 0 */

	/* Put the 64-bit file length in *bits* at the end of the buffer.  */
	*(uint32_t *) & ctx->buffer[bytes + pad] = SWAP(ctx->total[0] << 3);
	*(uint32_t *) & ctx->buffer[bytes + pad + 4] =
		SWAP(((ctx->total[1] << 3) | (ctx->total[0] >> 29)));

	/* Process last bytes.  */
	md5_hash_block(ctx->buffer, bytes + pad + 8, ctx);

	/* Put result from CTX in first 16 bytes following RESBUF.  The result is
	 * always in little endian byte order, so that a byte-wise output yields
	 * to the wanted ASCII representation of the message digest.
	 *
	 * IMPORTANT: On some systems it is required that RESBUF is correctly
	 * aligned for a 32 bits value.
	 */
	((uint32_t *) resbuf)[0] = SWAP(ctx->A);
	((uint32_t *) resbuf)[1] = SWAP(ctx->B);
	((uint32_t *) resbuf)[2] = SWAP(ctx->C);
	((uint32_t *) resbuf)[3] = SWAP(ctx->D);

	return resbuf;
}

static uint8_t *hash_file(const char *filename)
{
#define HASH_LEN (16)
	static uint8_t hash_value[HASH_LEN * 2 + 2] = {""};
	int src_fd, count;
	unsigned char* in_buf = NULL;
	int const in_buf_size = 4096;
	md5_ctx_t md5;

	in_buf = alloca(in_buf_size);
	if((src_fd = open(filename, O_RDONLY)) < 0){
		bb_perror_msg("%s", filename);
		goto hash_file_err1;
	}

	md5_begin(&md5);
	while((count = read(src_fd, in_buf, in_buf_size)) > 0) {
		md5_hash(in_buf, count, &md5);
	}
	if(count < 0){
		goto hash_file_err2;
	}

	md5_end(in_buf, &md5);
	{
		int x, len;
		for (x = len = 0; x < HASH_LEN; x++) {
			len += snprintf((char*)(hash_value + len), sizeof(hash_value) - len, "%02x", in_buf[x]);
		}
	}

	close(src_fd);
	return hash_value;

hash_file_err2:
	close(src_fd);

hash_file_err1:
	return NULL;
}

static uint8_t* hash_buffer(const void* buffer, ssize_t size)
{
#define HASH_LEN (16)
	static uint8_t hash_value[HASH_LEN * 2 + 2] = {""};
	unsigned char* in_buf = NULL;
	int const in_buf_size = 4096;
	md5_ctx_t md5;

	in_buf = alloca(in_buf_size);

	md5_begin(&md5);
	md5_hash(buffer, size, &md5);

	md5_end(in_buf, &md5);
	{
		int x, len;
		for (x = len = 0; x < HASH_LEN; x++) {
			len += snprintf((char*)(hash_value + len), sizeof(hash_value) - len, "%02x", in_buf[x]);
		}
	}

	return hash_value;
}

char* md5sum_file(const char* filename)
{
	return (char*)hash_file(filename);
}

char* md5sum_buffer(const void* buffer, int size)
{
	return (char*)hash_buffer(buffer, size);
}

char* md5sum_to_upper(void* buffer, int size)
{
	int i;
	char* p = (char*)buffer;
	for(i = 0; i < size; i++)
	{
		if(p[i] >= 'a' && p[i] <= 'z')
		{
			p[i] -= 32;
		}
	}
	return buffer;
}

//#define TEST_MD5
#ifdef TEST_MD5
int main(int argc, char **argv)
{
	uint8_t* ret = 0;
	if(argc == 2)
	{
		ret = md5sum_file((const char*)argv[1]);
		if(ret != NULL)
		{
			printf("%s\n", ret);
		}
	}
	return 0;
}
#endif

