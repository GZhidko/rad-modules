#ifndef __MS_MPPE_H
#define __MS_MPPE_H

void ms_mppe_encode(
           unsigned char * const c,
           unsigned char const * const p, int p_len,
           unsigned char const * const a,            /* 2 octers */
           unsigned char const * const s, int s_len,
           unsigned char const * const r             /* 16 octets (128-bit) */
          );

void ms_mppe_decode(
           unsigned char * const p,
           unsigned char const * const c, int c_len,
           unsigned char const * const a,            /* 2 octers */
           unsigned char const * const s, int s_len,
           unsigned char const * const r             /* 16 octets (128-bit) */
          );

void ms_mppe_encrypt_value(
           unsigned char * const value, int value_len,
           unsigned char const * const secret, int secret_len,
           unsigned char const * const req_authen
          );

void ms_mppe_decrypt_value(
           unsigned char * const value, int value_len,
           unsigned char const * const secret, int secret_len,
           unsigned char const * const req_authen
          );

#endif
