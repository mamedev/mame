// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

    aes192ecb.h

    AES192ECB helpers.

***************************************************************************/
#ifndef MAME_LIB_UTIL_AES192ECB_H
#define MAME_LIB_UTIL_AES192ECB_H

namespace aes192ecb {

#define AES_BLOCK_SIZE 16

typedef struct {
	unsigned int roundkey[52];
} AES_CTX;

// The key is 24 bytes. AES_Encrypt/AES_Decrypt process one 16-byte block per call.
// In-place operation is supported; no padding is added or removed.
void AES_EncryptInit(AES_CTX *ctx, const unsigned char *key);
void AES_Encrypt(const AES_CTX *ctx, const unsigned char in_data[AES_BLOCK_SIZE], unsigned char out_data[AES_BLOCK_SIZE]);
void AES_DecryptInit(AES_CTX *ctx, const unsigned char *key);
void AES_Decrypt(const AES_CTX *ctx, const unsigned char in_data[AES_BLOCK_SIZE], unsigned char out_data[AES_BLOCK_SIZE]);
void AES_CTX_Free(AES_CTX *ctx);

} // namespace aes192ecb

#endif // MAME_LIB_UTIL_AES192ECB_H
