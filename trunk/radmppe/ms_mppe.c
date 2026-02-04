#include <stdlib.h>
#include <sys/types.h>
#include "md5.h"
#include "parse.h"
#include <string.h>
#include "ms_mppe.h"
#include "log.h"

#define MD5Init md5_init
#define MD5Update md5_append
#define MD5Final(X, Y) md5_finish((Y), (X))
#define MD5_CTX md5_state_t

/****** UTIL ******/

void xor16octets(
           unsigned char * tgt,
           unsigned char const * s1,
           unsigned char const * s2)
{
    int i;
    for (i=0; i<16; i++) {
        *tgt++ = *s1++ ^ *s2++;
    }
}

void ms_mppe_encode(
           unsigned char * const c,
           unsigned char const * const p, int p_len,
           unsigned char const * const a,            /* 2 octers */
           unsigned char const * const s, int s_len,
           unsigned char const * const r             /* 16 octets (128-bit) */
          )
{
    MD5_CTX md5ctx;
    unsigned char ci[18];           /* c[i] */
    int ci_len;
    unsigned char bi[16];           /* b[i] */
    int shift;

    memcpy(ci, r, 16);              /* c[0] = R + A */
    memcpy(ci+16, a, 2);
    ci_len = 18;

    for (shift=0; shift < p_len; shift+=16) {
         MD5Init(&md5ctx);
         MD5Update(&md5ctx, s, s_len);
         MD5Update(&md5ctx, ci, ci_len);
         MD5Final(bi, &md5ctx);   /* b[i] = md5(s + c[i]) */
         xor16octets(ci, p + shift, bi); /* c[i] = p[i] xor b[i] */
         memcpy(c + shift, ci, 16); /* c += ci */
         ci_len = 16;
    }
}

void ms_mppe_decode(
           unsigned char * const p,
           unsigned char const * const c, int c_len,
           unsigned char const * const a,            /* 2 octers */
           unsigned char const * const s, int s_len,
           unsigned char const * const r             /* 16 octets (128-bit) */
          )
{
    MD5_CTX md5ctx;
    unsigned char ci[18];           /* c[i] */
    int ci_len;
    unsigned char bi[16];           /* b[i] */
    int shift;

    memcpy(ci, r, 16);              /* c[0] = R + A */
    memcpy(ci+16, a, 2);
    ci_len = 18;

    for (shift=0; shift < c_len; shift+=16) {
         MD5Init(&md5ctx);
         MD5Update(&md5ctx, s, s_len);
         MD5Update(&md5ctx, ci, ci_len);
         MD5Final(bi, &md5ctx);   /* b[i] = md5(s + c[i]) */
         memcpy(ci, c + shift, 16); /* c[i] = chank */
         ci_len = 16;
         xor16octets(p + shift, ci, bi); /* p[i] = c[i] xor b[i] */
    }
}

/****** CONVERTION ******/

void ms_mppe_encrypt_value(
           unsigned char * const value, int value_len,
           unsigned char const * const secret, int secret_len,
           unsigned char const * const req_authen
          )
{
    int p_len = value_len - 2;
    if (p_len > VAL_SIZE) {
        write_error("FATAL: Value too long (on encrypt)");
        exit(1);
    }
    unsigned char p[VAL_SIZE];
    unsigned char * v2;
    v2 = value + 2;
    memcpy(p, v2, p_len);
    ms_mppe_encode(v2, p, p_len, value, secret, secret_len, req_authen);
}

void ms_mppe_decrypt_value(
           unsigned char * const value, int value_len,
           unsigned char const * const secret, int secret_len,
           unsigned char const * const req_authen
          )
{
    int p_len = value_len - 2;
    if (p_len > VAL_SIZE) {
        write_error("FATAL: Value too long (on decrypt)");
        exit(1);
    }
    unsigned char p[VAL_SIZE];
    unsigned char * v2;
    v2 = value + 2;
    memcpy(p, v2, p_len);
    ms_mppe_decode(v2, p, p_len, value, secret, secret_len, req_authen);
}

