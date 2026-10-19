// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

    aes128ecb.h

    AES128ECB helpers.

***************************************************************************/
#ifndef MAME_LIB_UTIL_AES128ECB_H
#define MAME_LIB_UTIL_AES128ECB_H

namespace aes128ecb {

#define AES_BLOCK_SIZE 16

typedef struct {
	unsigned int roundkey[44];
} AES_CTX;

// The key is 16 bytes. AES_Encrypt/AES_Decrypt take a byte count that must be a multiple of 16.
// In-place operation is supported; no padding is added or removed.
void AES_EncryptInit(AES_CTX *ctx, const unsigned char *key);
void AES_Encrypt(AES_CTX *ctx, const unsigned char *in_data, unsigned int in_size, unsigned char *out_data);
void AES_DecryptInit(AES_CTX *ctx, const unsigned char *key);
void AES_Decrypt(AES_CTX *ctx, const unsigned char *in_data, unsigned int in_size, unsigned char *out_data);
void AES_CTX_Free(AES_CTX *ctx);

} // namespace aes128ecb

#endif // MAME_LIB_UTIL_AES128ECB_H
