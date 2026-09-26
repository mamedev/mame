// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

    aes192cbc.h

    AES192CBC helpers.

***************************************************************************/
#ifndef MAME_LIB_UTIL_AES192CBC_H
#define MAME_LIB_UTIL_AES192CBC_H

namespace aes192cbc {

#define AES_BLOCK_SIZE 16

typedef struct {
	unsigned int roundkey[52];
	unsigned int iv[4];
} AES_CTX;

// The key is 24 bytes; the IV and each data block are 16 bytes.
// Encryption/decryption update the CBC IV and support in-place operation.
void AES_EncryptInit(AES_CTX *ctx, const unsigned char *key, const unsigned char *iv);
void AES_Encrypt(AES_CTX *ctx, const unsigned char in_data[AES_BLOCK_SIZE], unsigned char out_data[AES_BLOCK_SIZE]);
void AES_DecryptInit(AES_CTX *ctx, const unsigned char *key, const unsigned char *iv);
void AES_Decrypt(AES_CTX* ctx, const unsigned char in_data[AES_BLOCK_SIZE], unsigned char out_data[AES_BLOCK_SIZE]);
void AES_CTX_Free(AES_CTX *ctx);

} // namespace aes192cbc

#endif // MAME_LIB_UTIL_AES192CBC_H
