#ifndef	AES_H_
#define	AES_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define Bits128	16  //��Կ����
#define Bits192	24
#define Bits256	32
typedef unsigned char byte;



extern void InitAes(int keySize, unsigned char* keyBytes);
extern void Cipher(unsigned char* input, unsigned char* output);  // encipher 16-bit input
void InvCipher(unsigned char * input, unsigned char * output);  // decipher 16-bit input
extern void RandString(unsigned char* output);

void SetNbNkNr(int keyS);
void AddRoundKey(int round);
void SubBytes();
void InvSubBytes();
void ShiftRows();
void InvShiftRows();
void MixColumns();
void InvMixColumns();
unsigned char gfmultby01(unsigned char b);
unsigned char gfmultby02(unsigned char b);
unsigned char gfmultby03(unsigned char b);
unsigned char gfmultby09(unsigned char b);
unsigned char gfmultby0b(unsigned char b);
unsigned char gfmultby0d(unsigned char b);
unsigned char gfmultby0e(unsigned char b);
void KeyExpansion();
void SubWord(unsigned char * word,unsigned char* result);
void RotWord(unsigned char * word,unsigned char* result);

#endif
