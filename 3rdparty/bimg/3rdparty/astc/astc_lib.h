/*----------------------------------------------------------------------------*/
/**
 *	@author Andrew Willmott
 *
 *	@brief	Library api for astc codec, to be used as an alternative to astc_toplevel.cpp
 */
/*----------------------------------------------------------------------------*/

#ifndef ASTC_LIB_H
#define ASTC_LIB_H

#include <stdint.h>
#include <stdlib.h>

enum ASTC_COMPRESS_MODE     // Trade-off compression quality for speed
{
	ASTC_COMPRESS_VERY_FAST,
	ASTC_COMPRESS_FAST,
	ASTC_COMPRESS_MEDIUM,
	ASTC_COMPRESS_THOROUGH,
	ASTC_COMPRESS_EXHAUSTIVE,
};

enum ASTC_DECODE_MODE
{
	ASTC_DECODE_LDR_SRGB,   // texture will be decompressed to 8-bit SRGB
	ASTC_DECODE_LDR_LINEAR, // texture will be decompressed to 8-bit linear
	ASTC_DECODE_HDR         // texture will be decompressed to 16-bit linear
};

enum ASTC_CHANNELS
{
    ASTC_RGBA,
    ASTC_BGRA
};


size_t astc_compressed_size(int block_width, int block_height, int width, int height);
//!< Returns size of the compressed data for a width x height source image, assuming the given block size

void astc_compress
(
    int                src_width,
    int                src_height,
    const uint8_t*     src_data,
    ASTC_CHANNELS      src_channels,
    int                src_stride,

    int                block_width,
    int                block_height,
    ASTC_COMPRESS_MODE compress_mode,
    ASTC_DECODE_MODE   decode_mode,
    uint8_t*           dst_data
);
//!< Compress 8-bit rgba source image into dst_data (expected to be of size astc_compressed_size(...))

void astc_decompress
(
    const uint8_t*     src_data,
    int                block_width,
    int                block_height,
    ASTC_DECODE_MODE   decode_mode,

    int                dst_width,
    int                dst_height,
    uint8_t*           dst_data,
    ASTC_CHANNELS      dst_channels,
    int                dst_stride
);
//!< Decompress astc source image into 8-bit rgba destination image.

#endif

