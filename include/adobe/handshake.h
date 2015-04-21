#include <openssl/sha.h>
#include <openssl/hmac.h>
#if OPENSSL_VERSION_NUMBER < 0x0090800 || !defined(SHA256_DIGEST_LENGTH)
#error Your OpenSSL is too old, need 0.9.8 or newer with SHA256
#endif
#define HMAC_setup(ctx, key, len)	HMAC_CTX_init(&ctx); HMAC_Init_ex(&ctx, key, len, EVP_sha256(), 0)
#define HMAC_crunch(ctx, buf, len)	HMAC_Update(&ctx, buf, len)
#define HMAC_finish(ctx, dig, dlen)	HMAC_Final(&ctx, dig, &dlen); HMAC_CTX_cleanup(&ctx)

#define FP10

#include <string.h>
#include <assert.h>
#include "rtmpsock.h"
#include "rtmplog.h"

static const uint8_t GenuineFMSKey[] = {
    0x47, 0x65, 0x6e, 0x75, 0x69, 0x6e, 0x65, 0x20, 0x41, 0x64, 0x6f, 0x62,
    0x65, 0x20, 0x46, 0x6c,
    0x61, 0x73, 0x68, 0x20, 0x4d, 0x65, 0x64, 0x69, 0x61, 0x20, 0x53, 0x65,
    0x72, 0x76, 0x65, 0x72,
    0x20, 0x30, 0x30, 0x31,	/* Genuine Adobe Flash Media Server 001 */

    0xf0, 0xee, 0xc2, 0x4a, 0x80, 0x68, 0xbe, 0xe8, 0x2e, 0x00, 0xd0, 0xd1,
    0x02, 0x9e, 0x7e, 0x57, 0x6e, 0xec, 0x5d, 0x2d, 0x29, 0x80, 0x6f, 0xab,
    0x93, 0xb8, 0xe6, 0x36,
    0xcf, 0xeb, 0x31, 0xae
};				/* 68 */

static const uint8_t GenuineFPKey[] = {
    0x47, 0x65, 0x6E, 0x75, 0x69, 0x6E, 0x65, 0x20, 0x41, 0x64, 0x6F, 0x62,
    0x65, 0x20, 0x46, 0x6C,
    0x61, 0x73, 0x68, 0x20, 0x50, 0x6C, 0x61, 0x79, 0x65, 0x72, 0x20, 0x30,
    0x30, 0x31,			/* Genuine Adobe Flash Player 001 */
    0xF0, 0xEE,
    0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1, 0x02, 0x9E,
    0x7E, 0x57, 0x6E, 0xEC,
    0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB, 0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB,
    0x31, 0xAE
};				/* 62 */


typedef unsigned int (getoff)(uint8_t *buf, unsigned int len);

static unsigned int GetDHOffset2(uint8_t *handshake, unsigned int len) {
    unsigned int offset = 0;
    uint8_t *ptr = handshake + 768;
    unsigned int res;

    assert(RTMP_SIG_SIZE <= len);

    offset += (*ptr);
    ptr++;
    offset += (*ptr);
    ptr++;
    offset += (*ptr);
    ptr++;
    offset += (*ptr);

    res = (offset % 632) + 8;

    if (res + 128 > 767) {
        RTMP_Log(RTMP_LOGERROR,
                 "%s: Couldn't calculate correct DH offset (got %d), exiting!",
                 __FUNCTION__, res);
        exit(1);
    }
    return res;
}

static unsigned int GetDigestOffset2(uint8_t *handshake, unsigned int len) {
    unsigned int offset = 0;
    uint8_t *ptr = handshake + 772;
    unsigned int res;

    offset += (*ptr);
    ptr++;
    offset += (*ptr);
    ptr++;
    offset += (*ptr);
    ptr++;
    offset += (*ptr);

    res = (offset % 728) + 776;

    if (res + 32 > 1535) {
        RTMP_Log(RTMP_LOGERROR,
                 "%s: Couldn't calculate correct digest offset (got %d), exiting",
                 __FUNCTION__, res);
        exit(1);
    }
    return res;
}

static unsigned int GetDHOffset1(uint8_t *handshake, unsigned int len) {
    unsigned int offset = 0;
    uint8_t *ptr = handshake + 1532;
    unsigned int res;

    assert(RTMP_SIG_SIZE <= len);

    offset += (*ptr);
    ptr++;
    offset += (*ptr);
    ptr++;
    offset += (*ptr);
    ptr++;
    offset += (*ptr);

    res = (offset % 632) + 772;

    if (res + 128 > 1531) {
        RTMP_Log(RTMP_LOGERROR, "%s: Couldn't calculate DH offset (got %d), exiting!",
                 __FUNCTION__, res);
        exit(1);
    }

    return res;
}

static unsigned int GetDigestOffset1(uint8_t *handshake, unsigned int len) {
    unsigned int offset = 0;
    uint8_t *ptr = handshake + 8;
    unsigned int res;

    assert(12 <= len);

    offset += (*ptr);
    ptr++;
    offset += (*ptr);
    ptr++;
    offset += (*ptr);
    ptr++;
    offset += (*ptr);

    res = (offset % 728) + 12;

    if (res + 32 > 771) {
        RTMP_Log(RTMP_LOGERROR,
                 "%s: Couldn't calculate digest offset (got %d), exiting!",
                 __FUNCTION__, res);
        exit(1);
    }

    return res;
}

static getoff *digoff[] = {GetDigestOffset1, GetDigestOffset2};
static getoff *dhoff[] = {GetDHOffset1, GetDHOffset2};

#if 1
static void HMACsha256(const uint8_t *message, size_t messageLen, const uint8_t *key,
           size_t keylen, uint8_t *digest) {
    unsigned int digestLen;
    HMAC_CTX ctx;

    HMAC_setup(ctx, key, keylen);
    HMAC_crunch(ctx, message, messageLen);
    HMAC_finish(ctx, digest, digestLen);

    assert(digestLen == 32);
}
#endif

static void CalculateDigest(unsigned int digestPos, uint8_t *handshakeMessage,
                const uint8_t *key, size_t keyLen, uint8_t *digest) {
    const int messageLen = RTMP_SIG_SIZE - SHA256_DIGEST_LENGTH;
    uint8_t message[RTMP_SIG_SIZE - SHA256_DIGEST_LENGTH];

    memcpy(message, handshakeMessage, digestPos);
    memcpy(message + digestPos,
           &handshakeMessage[digestPos + SHA256_DIGEST_LENGTH],
           messageLen - digestPos);

    HMACsha256(message, messageLen, key, keyLen, digest);
}

static int VerifyDigest(unsigned int digestPos, uint8_t *handshakeMessage, const uint8_t *key,
             size_t keyLen) {
    uint8_t calcDigest[SHA256_DIGEST_LENGTH];

    CalculateDigest(digestPos, handshakeMessage, key, keyLen, calcDigest);

    return memcmp(&handshakeMessage[digestPos], calcDigest,
                  SHA256_DIGEST_LENGTH) == 0;
}

static int SHandShake(Rtmp_t * r) {
    int i, offalg = 0;
    int digestPosServer = 0;
    int FP9HandShake = FALSE;
    int32_t *ip;

    uint8_t clientsig[RTMP_SIG_SIZE];
    uint8_t serverbuf[RTMP_SIG_SIZE + 4], *serversig = serverbuf+4;
    uint8_t type;
    uint32_t uptime;
    getoff *getdh = NULL, *getdig = NULL;

    if (RTMP_SOCK_read(r, (char *)&type, 1) != 1)	/* 0x03 or 0x06 */
        return FALSE;

    if (RTMP_SOCK_read(r, (char *)clientsig, RTMP_SIG_SIZE) != RTMP_SIG_SIZE)
        return FALSE;

    RTMP_Log(RTMP_LOGINFO, "%s: Type Requested : %02X", __FUNCTION__, type);
    RTMP_LogHex(RTMP_LOGDEBUG2, clientsig, RTMP_SIG_SIZE);
    if (type == 3) {
        //RTMP_Log(RTMP_LOGDEBUG,"version code is 3.");
    } else if (type == 6 || type == 8) {
        RTMP_Log(RTMP_LOGERROR,"unsupport encry mode");
        return FALSE;
    } else {
        RTMP_Log(RTMP_LOGERROR,"unknown version");
        return FALSE;
    }

    if (!FP9HandShake && clientsig[4])
        FP9HandShake = TRUE;

    serversig[-1] = type;

    uptime = htonl(0);
    memcpy(serversig, &uptime, 4);

    if (FP9HandShake) {
        /* Server version */
        serversig[4] = 3;
        serversig[5] = 5;
        serversig[6] = 1;
        serversig[7] = 1;

        getdig = digoff[offalg];
        getdh  = dhoff[offalg];
    } else {
        memset(&serversig[4], 0, 4);
    }


    ip = (int32_t *)(serversig+8);
    for (i = 2; i < RTMP_SIG_SIZE/4; i++)
        *ip++ = rand();


    /* set handshake digest */
    if (FP9HandShake) {
        digestPosServer = getdig(serversig, RTMP_SIG_SIZE);	/* reuse this value in verification */
        RTMP_Log(RTMP_LOGDEBUG, "%s: Server digest offset: %d", __FUNCTION__,
                 digestPosServer);

        CalculateDigest(digestPosServer, serversig, GenuineFMSKey, 36,
                        &serversig[digestPosServer]);

        RTMP_Log(RTMP_LOGDEBUG, "%s: Initial server digest: ", __FUNCTION__);
        RTMP_LogHex(RTMP_LOGDEBUG, serversig + digestPosServer,
                    SHA256_DIGEST_LENGTH);
    }

    RTMP_Log(RTMP_LOGDEBUG2, "Serversig: ");
    RTMP_LogHex(RTMP_LOGDEBUG2, serversig, RTMP_SIG_SIZE);

    if (RTMP_SOCK_write(r, (char *)serversig-1, RTMP_SIG_SIZE + 1)==-1)
        return FALSE;

    /* decode client response */
    memcpy(&uptime, clientsig, 4);
    uptime = ntohl(uptime);

    RTMP_Log(RTMP_LOGDEBUG, "%s: Client Uptime : %d", __FUNCTION__, uptime);
    RTMP_Log(RTMP_LOGDEBUG, "%s: Player Version: %d.%d.%d.%d", __FUNCTION__, clientsig[4],
             clientsig[5], clientsig[6], clientsig[7]);

    if (FP9HandShake) {
        uint8_t digestResp[SHA256_DIGEST_LENGTH];
        uint8_t *signatureResp = NULL;

        /* we have to use this signature now to find the correct algorithms for getting the digest and DH positions */
        int digestPosClient = getdig(clientsig, RTMP_SIG_SIZE);

        if (!VerifyDigest(digestPosClient, clientsig, GenuineFPKey, 30)) {
            RTMP_Log(RTMP_LOGWARNING, "Trying different position for client digest!");
            offalg ^= 1;
            getdig = digoff[offalg];
            getdh  = dhoff[offalg];

            digestPosClient = getdig(clientsig, RTMP_SIG_SIZE);

            if (!VerifyDigest(digestPosClient, clientsig, GenuineFPKey, 30)) {
                RTMP_Log(RTMP_LOGERROR, "Couldn't verify the client digest");	/* continuing anyway will probably fail */
                return FALSE;
            }
        }

        /* calculate response now */
        signatureResp = clientsig+RTMP_SIG_SIZE-SHA256_DIGEST_LENGTH;

        HMACsha256(&clientsig[digestPosClient], SHA256_DIGEST_LENGTH,
                   GenuineFMSKey, sizeof(GenuineFMSKey), digestResp);
        HMACsha256(clientsig, RTMP_SIG_SIZE - SHA256_DIGEST_LENGTH, digestResp,
                   SHA256_DIGEST_LENGTH, signatureResp);

        /* some info output */
        RTMP_Log(RTMP_LOGDEBUG,
                 "%s: Calculated digest key from secure key and server digest: ",
                 __FUNCTION__);
        RTMP_LogHex(RTMP_LOGDEBUG, digestResp, SHA256_DIGEST_LENGTH);

        RTMP_Log(RTMP_LOGDEBUG, "%s: Server signature calculated:", __FUNCTION__);
        RTMP_LogHex(RTMP_LOGDEBUG, signatureResp, SHA256_DIGEST_LENGTH);
    }

    RTMP_Log(RTMP_LOGDEBUG2, "%s: Sending handshake response: ",
             __FUNCTION__);
    RTMP_LogHex(RTMP_LOGDEBUG2, clientsig, RTMP_SIG_SIZE);

    if (-1==RTMP_SOCK_write(r, (char *)clientsig, RTMP_SIG_SIZE))
        return FALSE;

    /* 2nd part of handshake */
    if (RTMP_SOCK_read(r, (char *)clientsig, RTMP_SIG_SIZE) != RTMP_SIG_SIZE)
        return FALSE;

    RTMP_Log(RTMP_LOGDEBUG2, "%s: 2nd handshake: ", __FUNCTION__);
    RTMP_LogHex(RTMP_LOGDEBUG2, clientsig, RTMP_SIG_SIZE);

    if (FP9HandShake) {
        uint8_t signature[SHA256_DIGEST_LENGTH];
        uint8_t digest[SHA256_DIGEST_LENGTH];

        RTMP_Log(RTMP_LOGDEBUG, "%s: Client sent signature:", __FUNCTION__);
        RTMP_LogHex(RTMP_LOGDEBUG, &clientsig[RTMP_SIG_SIZE - SHA256_DIGEST_LENGTH],
                    SHA256_DIGEST_LENGTH);

        /* verify client response */
        HMACsha256(&serversig[digestPosServer], SHA256_DIGEST_LENGTH,
                   GenuineFPKey, sizeof(GenuineFPKey), digest);
        HMACsha256(clientsig, RTMP_SIG_SIZE - SHA256_DIGEST_LENGTH, digest,
                   SHA256_DIGEST_LENGTH, signature);

        /* show some information */
        RTMP_Log(RTMP_LOGDEBUG, "%s: Digest key: ", __FUNCTION__);
        RTMP_LogHex(RTMP_LOGDEBUG, digest, SHA256_DIGEST_LENGTH);

        RTMP_Log(RTMP_LOGDEBUG, "%s: Signature calculated:", __FUNCTION__);
        RTMP_LogHex(RTMP_LOGDEBUG, signature, SHA256_DIGEST_LENGTH);
        if (memcmp
                (signature, &clientsig[RTMP_SIG_SIZE - SHA256_DIGEST_LENGTH],
                 SHA256_DIGEST_LENGTH) != 0) {
            RTMP_Log(RTMP_LOGWARNING, "%s: Client not genuine Adobe!", __FUNCTION__);
            return FALSE;
        } else {
            RTMP_Log(RTMP_LOGDEBUG, "%s: Genuine Adobe Flash Player", __FUNCTION__);
        }

    } else {
        if (memcmp(serversig, clientsig, RTMP_SIG_SIZE) != 0) {
            RTMP_Log(RTMP_LOGWARNING, "%s: client signature does not match!",
                     __FUNCTION__);
        }
    }

    RTMP_Log(RTMP_LOGINFO, "%s: Handshaking finished....", __FUNCTION__);
    return TRUE;
}
