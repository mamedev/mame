// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

    aes256ecb.h

    AES256ECB helpers.

***************************************************************************/
#ifndef MAME_LIB_UTIL_AES256ECB_H
#define MAME_LIB_UTIL_AES256ECB_H

namespace aes256ecb {

#define AES_BLOCK_SIZE 16

typedef struct {
	unsigned int roundkey[60];
} AES_CTX;

// The key is 32 bytes. AES_Encrypt/AES_Decrypt process one 16-byte block per call.
// In-place operation is supported; no padding is added or removed.
void AES_EncryptInit(AES_CTX *ctx, const unsigned char *key);
void AES_Encrypt(AES_CTX *ctx, const unsigned char in_data[AES_BLOCK_SIZE], unsigned char out_data[AES_BLOCK_SIZE]);
void AES_DecryptInit(AES_CTX *ctx, const unsigned char *key);
void AES_Decrypt(AES_CTX *ctx, const unsigned char in_data[AES_BLOCK_SIZE], unsigned char out_data[AES_BLOCK_SIZE]);
void AES_CTX_Free(AES_CTX *ctx);

} // namespace aes256ecb

#endif // MAME_LIB_UTIL_AES256ECB_H
