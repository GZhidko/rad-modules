#include <string.h>
#include <stdio.h>
#include "b64.h"


/********************************************************
 This poetry borrows words from Apache 2.0.59 source code
 ********************************************************/


#define XX 64


static const unsigned char pr2six[256] =
{
  XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
  XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
  XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 62, XX, XX, XX, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, XX, XX, XX, XX, XX, XX,
  XX,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, XX, XX, XX, XX, XX,
  XX, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, XX, XX, XX, XX, XX,
  XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
  XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
  XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
  XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
  XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
  XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
  XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
  XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX
};


int b64_dec(char *bufcoded, char *bufplain)
{
  unsigned char *ibuf;
  unsigned char *obuf;
  int ilen;

  ibuf = (unsigned char *) bufcoded;
  while (pr2six[*(ibuf++)] != XX); /* find EOL */
  ilen = (ibuf - (const unsigned char *) bufcoded) - 1;
  /* out_boffrer_len = ((ilen + 3) / 4) * 3 + 1;
     out buffer is never overflow */

  obuf = (unsigned char *) bufplain;
  ibuf = (unsigned char *) bufcoded;

  while (ilen > 4) {
    *(obuf++) = (unsigned char) (pr2six[*ibuf] << 2 | pr2six[ibuf[1]] >> 4);
    *(obuf++) = (unsigned char) (pr2six[ibuf[1]] << 4 | pr2six[ibuf[2]] >> 2);
    *(obuf++) = (unsigned char) (pr2six[ibuf[2]] << 6 | pr2six[ibuf[3]]);
    ibuf += 4;
    ilen -= 4;
  }

  /* Note: (ilen == 1) would be an error, so just ingore that case */
  if (ilen > 1)
    *(obuf++) = (unsigned char) (pr2six[*ibuf] << 2 | pr2six[ibuf[1]] >> 4);
  if (ilen > 2)
    *(obuf++) = (unsigned char) (pr2six[ibuf[1]] << 4 | pr2six[ibuf[2]] >> 2);
  if (ilen > 3)
    *(obuf++) = (unsigned char) (pr2six[ibuf[2]] << 6 | pr2six[ibuf[3]]);
  *obuf = '\0';
  return 0; /* 0 is OK */
}


static const char basis_64[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


int b64_enc(char *string, char *encoded)
{
  int i;
  char *p;
  int len = strlen(string);

  if ( ((len + 2) / 3 * 4) + 1 > BUFFSIZE ) {
    *encoded = '\0';
    return 1; /* ERROR: buffer overflow */
  }

  p = encoded;
  for (i = 0; i < len - 2; i += 3) {
    *p++ = basis_64[(string[i] >> 2) & 0x3F];
    *p++ = basis_64[((string[i] & 0x3) << 4) |
                    ((int) (string[i + 1] & 0xF0) >> 4)];
    *p++ = basis_64[((string[i + 1] & 0xF) << 2) |
                    ((int) (string[i + 2] & 0xC0) >> 6)];
    *p++ = basis_64[string[i + 2] & 0x3F];
  }
  if (i < len) {
    *p++ = basis_64[(string[i] >> 2) & 0x3F];
    if (i == (len - 1)) {
      *p++ = basis_64[((string[i] & 0x3) << 4)];
      *p++ = '=';
    }
    else {
      *p++ = basis_64[((string[i] & 0x3) << 4) |
                      ((int) (string[i + 1] & 0xF0) >> 4)];
      *p++ = basis_64[((string[i + 1] & 0xF) << 2)];
    }
    *p++ = '=';
  }

  *p++ = '\0';
  return 0; /* 0 is OK */
}

