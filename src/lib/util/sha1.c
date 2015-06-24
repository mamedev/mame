// license:LGPL-2.1+
// copyright-holders:Peter Gutmann, Andrew Kuchling, Niels Moeller
/* sha1.h
 *
 * The sha1 hash function.
 */

/* nettle, low-level cryptographics library
 *
 * Copyright 2001 Peter Gutmann, Andrew Kuchling, Niels Moeller
 *
 * The nettle library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * The nettle library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the nettle library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "sha1.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static unsigned int READ_UINT32(const UINT8* data)
{
	return ((UINT32)data[0] << 24) |
		((UINT32)data[1] << 16) |
		((UINT32)data[2] << 8) |
		((UINT32)data[3]);
}

static void WRITE_UINT32(unsigned char* data, UINT32 val)
{
	data[0] = (val >> 24) & 0xFF;
	data[1] = (val >> 16) & 0xFF;
	data[2] = (val >> 8) & 0xFF;
	data[3] = (val >> 0) & 0xFF;
}


/* A block, treated as a sequence of 32-bit words. */
#define SHA1_DATA_LENGTH 16

/* The SHA f()-functions.  The f1 and f3 functions can be optimized to
   save one boolean operation each - thanks to Rich Schroeppel,
   rcs@cs.arizona.edu for discovering this */

/* #define f1(x,y,z) ( ( x & y ) | ( ~x & z ) )            Rounds  0-19 */
#define f1(x,y,z)   ( z ^ ( x & ( y ^ z ) ) )           /* Rounds  0-19 */
#define f2(x,y,z)   ( x ^ y ^ z )                       /* Rounds 20-39 */
/* #define f3(x,y,z) ( ( x & y ) | ( x & z ) | ( y & z ) ) Rounds 40-59 */
#define f3(x,y,z)   ( ( x & y ) | ( z & ( x | y ) ) )   /* Rounds 40-59 */
#define f4(x,y,z)   ( x ^ y ^ z )                       /* Rounds 60-79 */

/* The SHA Mysterious Constants */

#define K1  0x5A827999L                                 /* Rounds  0-19 */
#define K2  0x6ED9EBA1L                                 /* Rounds 20-39 */
#define K3  0x8F1BBCDCL                                 /* Rounds 40-59 */
#define K4  0xCA62C1D6L                                 /* Rounds 60-79 */

/* SHA initial values */

#define h0init  0x67452301L
#define h1init  0xEFCDAB89L
#define h2init  0x98BADCFEL
#define h3init  0x10325476L
#define h4init  0xC3D2E1F0L

/* 32-bit rotate left - kludged with shifts */
#ifdef _MSC_VER
#define ROTL(n,X)  _lrotl(X, n)
#else
#define ROTL(n,X)  ( ( (X) << (n) ) | ( (X) >> ( 32 - (n) ) ) )
#endif

/* The initial expanding function.  The hash function is defined over an
   80-word expanded input array W, where the first 16 are copies of the input
   data, and the remaining 64 are defined by

        W[ i ] = W[ i - 16 ] ^ W[ i - 14 ] ^ W[ i - 8 ] ^ W[ i - 3 ]

   This implementation generates these values on the fly in a circular
   buffer - thanks to Colin Plumb, colin@nyx10.cs.du.edu for this
   optimization.

   The updated SHA changes the expanding function by adding a rotate of 1
   bit.  Thanks to Jim Gillogly, jim@rand.org, and an anonymous contributor
   for this information */

#define expand(W,i) ( W[ i & 15 ] = \
				ROTL( 1, ( W[ i & 15 ] ^ W[ (i - 14) & 15 ] ^ \
					W[ (i - 8) & 15 ] ^ W[ (i - 3) & 15 ] ) ) )


/* The prototype SHA sub-round.  The fundamental sub-round is:

        a' = e + ROTL( 5, a ) + f( b, c, d ) + k + data;
        b' = a;
        c' = ROTL( 30, b );
        d' = c;
        e' = d;

   but this is implemented by unrolling the loop 5 times and renaming the
   variables ( e, a, b, c, d ) = ( a', b', c', d', e' ) each iteration.
   This code is then replicated 20 times for each of the 4 functions, using
   the next 20 values from the W[] array each time */

#define subRound(a, b, c, d, e, f, k, data) \
	( e += ROTL( 5, a ) + f( b, c, d ) + k + data, b = ROTL( 30, b ) )

/* Initialize the SHA values */

/**
 * @fn  void sha1_init(struct sha1_ctx *ctx)
 *
 * @brief   Sha 1 initialise.
 *
 * @param [in,out]  ctx If non-null, the context.
 */

void
sha1_init(struct sha1_ctx *ctx)
{
	/* Set the h-vars to their initial values */
	ctx->digest[ 0 ] = h0init;
	ctx->digest[ 1 ] = h1init;
	ctx->digest[ 2 ] = h2init;
	ctx->digest[ 3 ] = h3init;
	ctx->digest[ 4 ] = h4init;

	/* Initialize bit count */
	ctx->count_low = ctx->count_high = 0;

	/* Initialize buffer */
	ctx->index = 0;
}

/* Perform the SHA transformation.  Note that this code, like MD5, seems to
   break some optimizing compilers due to the complexity of the expressions
   and the size of the basic block.  It may be necessary to split it into
   sections, e.g. based on the four subrounds

   Note that this function destroys the data area */

/**
 * @fn  static void sha1_transform(UINT32 *state, UINT32 *data)
 *
 * @brief   Sha 1 transform.
 *
 * @param [in,out]  state   If non-null, the state.
 * @param [in,out]  data    If non-null, the data.
 */

static void
sha1_transform(UINT32 *state, UINT32 *data)
{
	UINT32 A, B, C, D, E;     /* Local vars */

	/* Set up first buffer and local data buffer */
	A = state[0];
	B = state[1];
	C = state[2];
	D = state[3];
	E = state[4];

	/* Heavy mangling, in 4 sub-rounds of 20 interations each. */
	subRound( A, B, C, D, E, f1, K1, data[ 0] );
	subRound( E, A, B, C, D, f1, K1, data[ 1] );
	subRound( D, E, A, B, C, f1, K1, data[ 2] );
	subRound( C, D, E, A, B, f1, K1, data[ 3] );
	subRound( B, C, D, E, A, f1, K1, data[ 4] );
	subRound( A, B, C, D, E, f1, K1, data[ 5] );
	subRound( E, A, B, C, D, f1, K1, data[ 6] );
	subRound( D, E, A, B, C, f1, K1, data[ 7] );
	subRound( C, D, E, A, B, f1, K1, data[ 8] );
	subRound( B, C, D, E, A, f1, K1, data[ 9] );
	subRound( A, B, C, D, E, f1, K1, data[10] );
	subRound( E, A, B, C, D, f1, K1, data[11] );
	subRound( D, E, A, B, C, f1, K1, data[12] );
	subRound( C, D, E, A, B, f1, K1, data[13] );
	subRound( B, C, D, E, A, f1, K1, data[14] );
	subRound( A, B, C, D, E, f1, K1, data[15] );
	subRound( E, A, B, C, D, f1, K1, expand( data, 16 ) );
	subRound( D, E, A, B, C, f1, K1, expand( data, 17 ) );
	subRound( C, D, E, A, B, f1, K1, expand( data, 18 ) );
	subRound( B, C, D, E, A, f1, K1, expand( data, 19 ) );

	subRound( A, B, C, D, E, f2, K2, expand( data, 20 ) );
	subRound( E, A, B, C, D, f2, K2, expand( data, 21 ) );
	subRound( D, E, A, B, C, f2, K2, expand( data, 22 ) );
	subRound( C, D, E, A, B, f2, K2, expand( data, 23 ) );
	subRound( B, C, D, E, A, f2, K2, expand( data, 24 ) );
	subRound( A, B, C, D, E, f2, K2, expand( data, 25 ) );
	subRound( E, A, B, C, D, f2, K2, expand( data, 26 ) );
	subRound( D, E, A, B, C, f2, K2, expand( data, 27 ) );
	subRound( C, D, E, A, B, f2, K2, expand( data, 28 ) );
	subRound( B, C, D, E, A, f2, K2, expand( data, 29 ) );
	subRound( A, B, C, D, E, f2, K2, expand( data, 30 ) );
	subRound( E, A, B, C, D, f2, K2, expand( data, 31 ) );
	subRound( D, E, A, B, C, f2, K2, expand( data, 32 ) );
	subRound( C, D, E, A, B, f2, K2, expand( data, 33 ) );
	subRound( B, C, D, E, A, f2, K2, expand( data, 34 ) );
	subRound( A, B, C, D, E, f2, K2, expand( data, 35 ) );
	subRound( E, A, B, C, D, f2, K2, expand( data, 36 ) );
	subRound( D, E, A, B, C, f2, K2, expand( data, 37 ) );
	subRound( C, D, E, A, B, f2, K2, expand( data, 38 ) );
	subRound( B, C, D, E, A, f2, K2, expand( data, 39 ) );

	subRound( A, B, C, D, E, f3, K3, expand( data, 40 ) );
	subRound( E, A, B, C, D, f3, K3, expand( data, 41 ) );
	subRound( D, E, A, B, C, f3, K3, expand( data, 42 ) );
	subRound( C, D, E, A, B, f3, K3, expand( data, 43 ) );
	subRound( B, C, D, E, A, f3, K3, expand( data, 44 ) );
	subRound( A, B, C, D, E, f3, K3, expand( data, 45 ) );
	subRound( E, A, B, C, D, f3, K3, expand( data, 46 ) );
	subRound( D, E, A, B, C, f3, K3, expand( data, 47 ) );
	subRound( C, D, E, A, B, f3, K3, expand( data, 48 ) );
	subRound( B, C, D, E, A, f3, K3, expand( data, 49 ) );
	subRound( A, B, C, D, E, f3, K3, expand( data, 50 ) );
	subRound( E, A, B, C, D, f3, K3, expand( data, 51 ) );
	subRound( D, E, A, B, C, f3, K3, expand( data, 52 ) );
	subRound( C, D, E, A, B, f3, K3, expand( data, 53 ) );
	subRound( B, C, D, E, A, f3, K3, expand( data, 54 ) );
	subRound( A, B, C, D, E, f3, K3, expand( data, 55 ) );
	subRound( E, A, B, C, D, f3, K3, expand( data, 56 ) );
	subRound( D, E, A, B, C, f3, K3, expand( data, 57 ) );
	subRound( C, D, E, A, B, f3, K3, expand( data, 58 ) );
	subRound( B, C, D, E, A, f3, K3, expand( data, 59 ) );

	subRound( A, B, C, D, E, f4, K4, expand( data, 60 ) );
	subRound( E, A, B, C, D, f4, K4, expand( data, 61 ) );
	subRound( D, E, A, B, C, f4, K4, expand( data, 62 ) );
	subRound( C, D, E, A, B, f4, K4, expand( data, 63 ) );
	subRound( B, C, D, E, A, f4, K4, expand( data, 64 ) );
	subRound( A, B, C, D, E, f4, K4, expand( data, 65 ) );
	subRound( E, A, B, C, D, f4, K4, expand( data, 66 ) );
	subRound( D, E, A, B, C, f4, K4, expand( data, 67 ) );
	subRound( C, D, E, A, B, f4, K4, expand( data, 68 ) );
	subRound( B, C, D, E, A, f4, K4, expand( data, 69 ) );
	subRound( A, B, C, D, E, f4, K4, expand( data, 70 ) );
	subRound( E, A, B, C, D, f4, K4, expand( data, 71 ) );
	subRound( D, E, A, B, C, f4, K4, expand( data, 72 ) );
	subRound( C, D, E, A, B, f4, K4, expand( data, 73 ) );
	subRound( B, C, D, E, A, f4, K4, expand( data, 74 ) );
	subRound( A, B, C, D, E, f4, K4, expand( data, 75 ) );
	subRound( E, A, B, C, D, f4, K4, expand( data, 76 ) );
	subRound( D, E, A, B, C, f4, K4, expand( data, 77 ) );
	subRound( C, D, E, A, B, f4, K4, expand( data, 78 ) );
	subRound( B, C, D, E, A, f4, K4, expand( data, 79 ) );

	/* Build message digest */
	state[0] += A;
	state[1] += B;
	state[2] += C;
	state[3] += D;
	state[4] += E;
}

/**
 * @fn  static void sha1_block(struct sha1_ctx *ctx, const UINT8 *block)
 *
 * @brief   Sha 1 block.
 *
 * @param [in,out]  ctx If non-null, the context.
 * @param   block       The block.
 */

static void
sha1_block(struct sha1_ctx *ctx, const UINT8 *block)
{
	UINT32 data[SHA1_DATA_LENGTH];
	int i;

	/* Update block count */
	if (!++ctx->count_low)
	++ctx->count_high;

	/* Endian independent conversion */
	for (i = 0; i<SHA1_DATA_LENGTH; i++, block += 4)
	data[i] = READ_UINT32(block);

	sha1_transform(ctx->digest, data);
}

/**
 * @fn  void sha1_update(struct sha1_ctx *ctx, unsigned length, const UINT8 *buffer)
 *
 * @brief   Sha 1 update.
 *
 * @param [in,out]  ctx If non-null, the context.
 * @param   length      The length.
 * @param   buffer      The buffer.
 */

void
sha1_update(struct sha1_ctx *ctx,
		unsigned length, const UINT8 *buffer)
{
	if (ctx->index)
	{ /* Try to fill partial block */
		unsigned left = SHA1_DATA_SIZE - ctx->index;
		if (length < left)
	{
		memcpy(ctx->block + ctx->index, buffer, length);
		ctx->index += length;
		return; /* Finished */
	}
		else
	{
		memcpy(ctx->block + ctx->index, buffer, left);
		sha1_block(ctx, ctx->block);
		buffer += left;
		length -= left;
	}
	}
	while (length >= SHA1_DATA_SIZE)
	{
		sha1_block(ctx, buffer);
		buffer += SHA1_DATA_SIZE;
		length -= SHA1_DATA_SIZE;
	}
	ctx->index = length;
	if (length)
	/* Buffer leftovers */
	memcpy(ctx->block, buffer, length);
}

/* Final wrapup - pad to SHA1_DATA_SIZE-byte boundary with the bit pattern
   1 0* (64-bit count of bits processed, MSB-first) */

/**
 * @fn  void sha1_final(struct sha1_ctx *ctx)
 *
 * @brief   Sha 1 final.
 *
 * @param [in,out]  ctx If non-null, the context.
 */

void
sha1_final(struct sha1_ctx *ctx)
{
	UINT32 data[SHA1_DATA_LENGTH];
	int i;
	int words;

	i = ctx->index;

	/* Set the first char of padding to 0x80.  This is safe since there is
	 always at least one byte free */

	assert(i < SHA1_DATA_SIZE);
	ctx->block[i++] = 0x80;

	/* Fill rest of word */
	for( ; i & 3; i++)
	ctx->block[i] = 0;

	/* i is now a multiple of the word size 4 */
	words = i >> 2;
	for (i = 0; i < words; i++)
	data[i] = READ_UINT32(ctx->block + 4*i);

	if (words > (SHA1_DATA_LENGTH-2))
	{ /* No room for length in this block. Process it and
       * pad with another one */
		for (i = words ; i < SHA1_DATA_LENGTH; i++)
	data[i] = 0;
		sha1_transform(ctx->digest, data);
		for (i = 0; i < (SHA1_DATA_LENGTH-2); i++)
	data[i] = 0;
	}
	else
	for (i = words ; i < SHA1_DATA_LENGTH - 2; i++)
		data[i] = 0;

	/* There are 512 = 2^9 bits in one block */
	data[SHA1_DATA_LENGTH-2] = (ctx->count_high << 9) | (ctx->count_low >> 23);
	data[SHA1_DATA_LENGTH-1] = (ctx->count_low << 9) | (ctx->index << 3);
	sha1_transform(ctx->digest, data);
}

/**
 * @fn  void sha1_digest(const struct sha1_ctx *ctx, unsigned length, UINT8 *digest)
 *
 * @brief   Sha 1 digest.
 *
 * @param   ctx             The context.
 * @param   length          The length.
 * @param [in,out]  digest  If non-null, the digest.
 */

void
sha1_digest(const struct sha1_ctx *ctx,
		unsigned length,
		UINT8 *digest)
{
	unsigned i;
	unsigned words;
	unsigned leftover;

	assert(length <= SHA1_DIGEST_SIZE);

	words = length / 4;
	leftover = length % 4;

	for (i = 0; i < words; i++, digest += 4)
	WRITE_UINT32(digest, ctx->digest[i]);

	if (leftover)
	{
		UINT32 word;
		unsigned j = leftover;

		assert(i < _SHA1_DIGEST_LENGTH);

		word = ctx->digest[i];

		switch (leftover)
	{
	default:
		/* this is just here to keep the compiler happy; it can never happen */
	case 3:
		digest[--j] = (word >> 8) & 0xff;
		/* Fall through */
	case 2:
		digest[--j] = (word >> 16) & 0xff;
		/* Fall through */
	case 1:
		digest[--j] = (word >> 24) & 0xff;
	}
	}
}
