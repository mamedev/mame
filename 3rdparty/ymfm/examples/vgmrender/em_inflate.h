/*
 * em_inflate.h - fast in-memory inflate (gzip/zlib decompressor) definitions
 *
 * Copyright (C) 2019 Emmanuel Marty
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef _EM_INFLATE_H
#define _EM_INFLATE_H

#include <sys/types.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Inflate gzip or zlib data
 *
 * @param pCompressedData pointer to start of zlib data
 * @param nCompressedDataSize size of zlib data, in bytes
 * @param pOutData pointer to start of decompression buffer
 * @param nMaxOutDataSize maximum size of decompression buffer, in bytes
 *
 * @return number of bytes decompressed, or -1 in case of an error
 */
size_t em_inflate(const void *pCompressedData, size_t nCompressedDataSize, unsigned char *pOutData, size_t nMaxOutDataSize);

#ifdef __cplusplus
}
#endif

#endif /* _EM_INFLATE_H */
