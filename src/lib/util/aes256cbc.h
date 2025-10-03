// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

    aes256cbc.h

    AES256CBC helpers.

***************************************************************************/
#ifndef MAME_LIB_UTIL_AES256CBC_H
#define MAME_LIB_UTIL_AES256CBC_H

namespace aes256cbc {

#define AES_BLOCK_SIZE 16

typedef struct {
	unsigned int roundkey[60];
	unsigned int iv[4];
} AES_CTX;

void AES_DecryptInit(AES_CTX *ctx, const unsigned char *key, const unsigned char *iv);
void AES_Decrypt(AES_CTX* ctx, const unsigned char in_data[AES_BLOCK_SIZE], unsigned char out_data[AES_BLOCK_SIZE]);
void AES_CTX_Free(AES_CTX *ctx);

} // namespace aes256cbc

#endif // MAME_LIB_UTIL_AES256CBC_H
