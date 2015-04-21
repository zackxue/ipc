
#include "esee_crypto.h"

#include "aes.h"
#include "base64.h"

static void esee_encrypt(uint8_t* key, void* input, void* output)
{
	uint8_t cipher[48] = {0};
	uint8_t First[16] = {0}, Second[16] = {0};
	uint8_t newKey[16] = {0};
	uint8_t randStr[16] = {0};
	char base64[128] = {0};
	RandString(randStr);
	InitAes(16, key);
	Cipher(randStr, newKey);
	InitAes(16, newKey);
	Cipher(input, First);
	Cipher(input + 16, Second);
	memcpy(cipher, First, sizeof(First));
	memcpy(cipher + sizeof(First), Second, sizeof(Second));
	memcpy(cipher + sizeof(First) + sizeof(Second), randStr, sizeof(randStr));
	base64_encode(cipher, base64, 48);
	strcpy(output, base64);
}
/*
static void esee_decrypt(uint8_t* key, void* input, void* output)
{
	uint8_t newKey[16] = {0};
	uint8_t original[32] = {0};
	uint8_t debase64[48] = {0};
	base64_decode(input,debase64,64);
	InitAes(16, key);
	Cipher(debase64 + 32, newKey);
	InitAes(16, newKey);
	InvCipher(debase64, original);
	InvCipher(debase64 + 16, original +16);
	memcpy(output, original, sizeof(original));
}
*/

#define ESEE_KEY_UCODE_ENCRYPT {0x55, 0x93, 0xe3, 0x7d, 0xa5, 0x64, 0x1a, 0x85, 0xb6, 0xc9, 0xac, 0x24, 0xc3, 0x2c,0xbf, 0x2c}
#define ESEE_KEY_ID_ENCRYPT {0x29, 0x3c, 0x7c, 0xcd, 0x9f, 0x6a, 0x33, 0x87, 0x55, 0x9c, 0x39, 0xa0, 0xee, 0x12,0x1c, 0x5d}

int ESEE_encrypt_ucode(void* origin, void* crypto)
{
	uint8_t ucode_key[] = ESEE_KEY_UCODE_ENCRYPT;
	esee_encrypt(ucode_key, origin, crypto);
	return 0;
}

int ESEE_encrypt_id(void* origin, void* crypto)
{
	uint8_t id_key[] = ESEE_KEY_ID_ENCRYPT;
	esee_encrypt(id_key, origin, crypto);
	return 0;
}


