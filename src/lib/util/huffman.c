/***************************************************************************

    huffman.c

    Video compression and decompression helpers.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************

    Maximum codelength is officially (alphabetsize - 1). This would be 255 bits
    (since we use 1 byte values). However, it is also dependent upon the number
    of samples used, as follows:

         2 bits -> 3..4 samples
         3 bits -> 5..7 samples
         4 bits -> 8..12 samples
         5 bits -> 13..20 samples
         6 bits -> 21..33 samples
         7 bits -> 34..54 samples
         8 bits -> 55..88 samples
         9 bits -> 89..143 samples
        10 bits -> 144..232 samples
        11 bits -> 233..376 samples
        12 bits -> 377..609 samples
        13 bits -> 610..986 samples
        14 bits -> 987..1596 samples
        15 bits -> 1597..2583 samples
        16 bits -> 2584..4180 samples   -> note that a 4k data size guarantees codelength <= 16 bits
        17 bits -> 4181..6764 samples
        18 bits -> 6765..10945 samples
        19 bits -> 10946..17710 samples
        20 bits -> 17711..28656 samples
        21 bits -> 28657..46367 samples
        22 bits -> 46368..75024 samples
        23 bits -> 75025..121392 samples
        24 bits -> 121393..196417 samples
        25 bits -> 196418..317810 samples
        26 bits -> 317811..514228 samples
        27 bits -> 514229..832039 samples
        28 bits -> 832040..1346268 samples
        29 bits -> 1346269..2178308 samples
        30 bits -> 2178309..3524577 samples
        31 bits -> 3524578..5702886 samples
        32 bits -> 5702887..9227464 samples

    Looking at it differently, here is where powers of 2 fall into these buckets:

          256 samples -> 11 bits max
          512 samples -> 12 bits max
           1k samples -> 14 bits max
           2k samples -> 15 bits max
           4k samples -> 16 bits max
           8k samples -> 18 bits max
          16k samples -> 19 bits max
          32k samples -> 21 bits max
          64k samples -> 22 bits max
         128k samples -> 24 bits max
         256k samples -> 25 bits max
         512k samples -> 27 bits max
           1M samples -> 28 bits max
           2M samples -> 29 bits max
           4M samples -> 31 bits max
           8M samples -> 32 bits max

****************************************************************************

    Delta-RLE encoding works as follows:

    Starting value is assumed to be 0. All data is encoded as a delta
    from the previous value, such that final[i] = final[i - 1] + delta.
    Long runs of 0s are RLE-encoded as follows:

        0x100 = repeat count of 8
        0x101 = repeat count of 9
        0x102 = repeat count of 10
        0x103 = repeat count of 11
        0x104 = repeat count of 12
        0x105 = repeat count of 13
        0x106 = repeat count of 14
        0x107 = repeat count of 15
        0x108 = repeat count of 16
        0x109 = repeat count of 32
        0x10a = repeat count of 64
        0x10b = repeat count of 128
        0x10c = repeat count of 256
        0x10d = repeat count of 512
        0x10e = repeat count of 1024
        0x10f = repeat count of 2048

    Note that repeat counts are reset at the end of a row, so if a 0 run
    extends to the end of a row, a large repeat count may be used.

    The reason for starting the run counts at 8 is that 0 is expected to
    be the most common symbol, and is typically encoded in 1 or 2 bits.

***************************************************************************/

#include "huffman.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define HUFFMAN_CODES			256
#define HUFFMAN_DELTARLE_CODES	(HUFFMAN_CODES + 16)

#define MAX_HUFFMAN_CODES		(HUFFMAN_DELTARLE_CODES)
#define MAX_HUFFMAN_NODES		(MAX_HUFFMAN_CODES + MAX_HUFFMAN_CODES)



/***************************************************************************
    MACROS
***************************************************************************/

#define MAKE_LOOKUP(code,bits)	(((code) << 6) | ((bits) & 0x1f))
#define LOOKUP_CODE(val)		((val) >> 6)
#define LOOKUP_BITS(val)		((val) & 0x1f)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _bit_buffer bit_buffer;
struct _bit_buffer
{
	UINT32					buffer;							/* current bit accumulator */
	int						bits;							/* number of bits in the accumulator */
	union
	{
		const UINT8 *		read;							/* read pointer */
		UINT8 *				write;							/* write pointer */
	} data;
	UINT32					doffset;						/* byte offset within the data */
	UINT32					dlength;						/* length of the data */
	int						overflow;						/* flag: true if we read/wrote past the end */
};


typedef struct _huffman_node huffman_node;
struct _huffman_node
{
	huffman_node *			parent;							/* pointer to parent node */
	UINT32					count;							/* number of hits on this node */
	UINT32					weight;							/* assigned weight of this node */
	UINT32					bits;							/* bits used to encode the node */
	UINT8					numbits;						/* number of bits needed for this node */
};


struct _huffman_context
{
	UINT8					maxbits;						/* maximum bits per code */
	UINT8					lookupdirty;					/* TRUE if the lookup table is dirty */
	UINT8					prevdata;						/* value of the previous data (for delta-RLE encoding) */
	UINT32					datahisto[MAX_HUFFMAN_CODES];	/* histogram of data values */
	int						rleremaining;					/* number of RLE bytes remaining (for delta-RLE encoding) */
	huffman_node 			huffnode[MAX_HUFFMAN_NODES];	/* array of nodes */
	huffman_lookup_value *	lookup;							/* pointer to the lookup table */
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/

static huffman_error huffman_deltarle_decode_data_interleaved_0102(huffman_context **contexts, const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dwidth, UINT32 dheight, UINT32 dstride, UINT32 dxor, UINT32 *actlength);

static huffman_error import_tree(huffman_context *context, const UINT8 *source, UINT32 slength, UINT32 *actlength, UINT32 numcodes);
static huffman_error export_tree(huffman_context *context, UINT8 *dest, UINT32 dlength, UINT32 *actlength, UINT32 numcodes);
static void write_rle_tree_bits(bit_buffer *bitbuf, int value, int repcount, int numbits);
static int CLIB_DECL tree_node_compare(const void *item1, const void *item2);
static huffman_error compute_optimal_tree(huffman_context *context, const UINT32 *datahisto, UINT32 numcodes);
static int huffman_build_tree(huffman_context *context, const UINT32 *datahisto, UINT32 totaldata, UINT32 totalweight, UINT32 numcodes);
static huffman_error assign_canonical_codes(huffman_context *context, UINT32 numcodes);
static huffman_error build_lookup_table(huffman_context *context, UINT32 numcodes);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    bit_buffer_write_init - initialize a bit
    buffer for writing
-------------------------------------------------*/

INLINE void bit_buffer_write_init(bit_buffer *bitbuf, UINT8 *data, UINT32 dlength)
{
	/* fill in the basic data structure */
	bitbuf->buffer = 0;
	bitbuf->bits = 0;
	bitbuf->data.write = data;
	bitbuf->doffset = 0;
	bitbuf->dlength = dlength;
	bitbuf->overflow = FALSE;
}


/*-------------------------------------------------
    bit_buffer_write - write 'numbits' to the
    bit buffer, assuming that 'newbits' is right-
    justified
-------------------------------------------------*/

INLINE void bit_buffer_write(bit_buffer *bitbuf, UINT32 newbits, int numbits)
{
	/* flush the buffer if we're going to overflow it */
	if (bitbuf->bits + numbits > 32)
		while (bitbuf->bits >= 8)
		{
			if (bitbuf->doffset < bitbuf->dlength)
				bitbuf->data.write[bitbuf->doffset] = bitbuf->buffer >> 24;
			else
				bitbuf->overflow = TRUE;
			bitbuf->doffset++;
			bitbuf->buffer <<= 8;
			bitbuf->bits -= 8;
		}

	/* shift the bits to the top */
	newbits <<= 32 - numbits;

	/* now shift it down to account for the number of bits we already have and OR them in */
	bitbuf->buffer |= newbits >> bitbuf->bits;
	bitbuf->bits += numbits;
}


/*-------------------------------------------------
    bit_buffer_flush - flush any bits in the write
    buffer and return the final data offset
-------------------------------------------------*/

INLINE UINT32 bit_buffer_flush(bit_buffer *bitbuf)
{
	while (bitbuf->bits > 0)
	{
		if (bitbuf->doffset < bitbuf->dlength)
			bitbuf->data.write[bitbuf->doffset] = bitbuf->buffer >> 24;
		else
			bitbuf->overflow = TRUE;
		bitbuf->doffset++;
		bitbuf->buffer <<= 8;
		bitbuf->bits -= 8;
	}
	return bitbuf->doffset;
}


/*-------------------------------------------------
    bit_buffer_read_init - initialize a bit
    buffer for reading
-------------------------------------------------*/

INLINE void bit_buffer_read_init(bit_buffer *bitbuf, const UINT8 *data, UINT32 dlength)
{
	/* fill in the basic data structure */
	bitbuf->buffer = 0;
	bitbuf->bits = 0;
	bitbuf->data.read = data;
	bitbuf->doffset = 0;
	bitbuf->dlength = dlength;
	bitbuf->overflow = FALSE;
}


/*-------------------------------------------------
    bit_buffer_read - read 'numbits' bits from
    the buffer, returning them right-justified
-------------------------------------------------*/

INLINE UINT32 bit_buffer_read(bit_buffer *bitbuf, int numbits)
{
	UINT32 result;

	/* fetch data if we need more */
	if (numbits > bitbuf->bits)
	{
		while (bitbuf->bits <= 24)
		{
			if (bitbuf->doffset < bitbuf->dlength)
				bitbuf->buffer |= bitbuf->data.read[bitbuf->doffset] << (24 - bitbuf->bits);
			bitbuf->doffset++;
			bitbuf->bits += 8;
		}
		if (numbits > bitbuf->bits)
			bitbuf->overflow = TRUE;
	}

	/* return the data */
	result = bitbuf->buffer >> (32 - numbits);
	bitbuf->buffer <<= numbits;
	bitbuf->bits -= numbits;
	return result;
}


/*-------------------------------------------------
    bit_buffer_peek - peek ahead and return
    'numbits' bits from the buffer, returning
    them right-justified
-------------------------------------------------*/

INLINE UINT32 bit_buffer_peek(bit_buffer *bitbuf, int numbits)
{
	/* fetch data if we need more */
	if (numbits > bitbuf->bits)
	{
		while (bitbuf->bits <= 24)
		{
			if (bitbuf->doffset < bitbuf->dlength)
				bitbuf->buffer |= bitbuf->data.read[bitbuf->doffset] << (24 - bitbuf->bits);
			bitbuf->doffset++;
			bitbuf->bits += 8;
		}
		if (numbits > bitbuf->bits)
			bitbuf->overflow = TRUE;
	}

	/* return the data */
	return bitbuf->buffer >> (32 - numbits);
}


/*-------------------------------------------------
    bit_buffer_remove - remove 'numbits' bits
    from the bit buffer; this presupposes that
    at least 'numbits' are present
-------------------------------------------------*/

INLINE void bit_buffer_remove(bit_buffer *bitbuf, int numbits)
{
	bitbuf->buffer <<= numbits;
	bitbuf->bits -= numbits;
}


/*-------------------------------------------------
    bit_buffer_read_offset - return the current
    rounded byte reading offset
-------------------------------------------------*/

INLINE UINT32 bit_buffer_read_offset(bit_buffer *bitbuf)
{
	UINT32 result = bitbuf->doffset;
	int bits = bitbuf->bits;
	while (bits >= 8)
	{
		result--;
		bits -= 8;
	}
	return result;
}


/*-------------------------------------------------
    code_to_rlecount - number of RLE repetitions
    encoded in a given byte
-------------------------------------------------*/

INLINE int code_to_rlecount(int code)
{
	if (code == 0x00)
		return 1;
	if (code <= 0x107)
		return 8 + (code - 0x100);
	return 16 << (code - 0x108);
}


/*-------------------------------------------------
    rlecount_to_byte - return a byte encoding
    the maximum RLE count less than or equal to
    the provided amount
-------------------------------------------------*/

INLINE int rlecount_to_code(int rlecount)
{
	if (rlecount >= 2048)
		return 0x10f;
	if (rlecount >= 1024)
		return 0x10e;
	if (rlecount >= 512)
		return 0x10d;
	if (rlecount >= 256)
		return 0x10c;
	if (rlecount >= 128)
		return 0x10b;
	if (rlecount >= 64)
		return 0x10a;
	if (rlecount >= 32)
		return 0x109;
	if (rlecount >= 16)
		return 0x108;
	if (rlecount >= 8)
		return 0x100 + (rlecount - 8);
	return 0x00;
}



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    huffman_create_context - create an encoding/
    decoding context
-------------------------------------------------*/

huffman_error huffman_create_context(huffman_context **context, int maxbits)
{
	/* limit to 24 bits */
	if (maxbits > 24)
		return HUFFERR_TOO_MANY_BITS;

	/* allocate a context */
	*context = (huffman_context *)malloc(sizeof(**context));
	if (*context == NULL)
		return HUFFERR_OUT_OF_MEMORY;

	/* set the info */
	memset(*context, 0, sizeof(**context));
	(*context)->maxbits = maxbits;
	(*context)->lookupdirty = TRUE;

	return HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_free_context - free an encoding/
    decoding context
-------------------------------------------------*/

void huffman_free_context(huffman_context *context)
{
	if (context->lookup != NULL)
		free(context->lookup);
	free(context);
}


/*-------------------------------------------------
    huffman_import_tree - import a huffman tree
    from a source data stream
-------------------------------------------------*/

huffman_error huffman_import_tree(huffman_context *context, const UINT8 *source, UINT32 slength, UINT32 *actlength)
{
	return import_tree(context, source, slength, actlength, HUFFMAN_CODES);
}


/*-------------------------------------------------
    huffman_export_tree - export a huffman tree
    to a target data stream
-------------------------------------------------*/

huffman_error huffman_export_tree(huffman_context *context, UINT8 *dest, UINT32 dlength, UINT32 *actlength)
{
	return export_tree(context, dest, dlength, actlength, HUFFMAN_CODES);
}


/*-------------------------------------------------
    huffman_deltarle_import_tree - import a
    huffman tree from a source data stream for
    delta-RLE encoded data
-------------------------------------------------*/

huffman_error huffman_deltarle_import_tree(huffman_context *context, const UINT8 *source, UINT32 slength, UINT32 *actlength)
{
	return import_tree(context, source, slength, actlength, HUFFMAN_DELTARLE_CODES);
}


/*-------------------------------------------------
    huffman__deltarle_export_tree - export a
    huffman tree to a target data stream for
    delta-RLE encoded data
-------------------------------------------------*/

huffman_error huffman_deltarle_export_tree(huffman_context *context, UINT8 *dest, UINT32 dlength, UINT32 *actlength)
{
	return export_tree(context, dest, dlength, actlength, HUFFMAN_DELTARLE_CODES);
}


/*-------------------------------------------------
    huffman_compute_tree - compute an optimal
    huffman tree for the given source data
-------------------------------------------------*/

huffman_error huffman_compute_tree(huffman_context *context, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor)
{
	return huffman_compute_tree_interleaved(1, &context, source, swidth, sheight, sstride, sxor);
}

huffman_error huffman_compute_tree_interleaved(int numcontexts, huffman_context **contexts, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor)
{
	UINT32 sx, sy, ctxnum;
	huffman_error error;

	/* initialize all nodes */
	for (ctxnum = 0; ctxnum < numcontexts; ctxnum++)
	{
		huffman_context *context = contexts[ctxnum];
		memset(context->datahisto, 0, sizeof(context->datahisto));
	}

	/* iterate over "height" */
	for (sy = 0; sy < sheight; sy++)
	{
		/* iterate over "width" */
		for (sx = 0; sx < swidth; )
		{
			/* iterate over contexts */
			for (ctxnum = 0; ctxnum < numcontexts; ctxnum++, sx++)
			{
				huffman_context *context = contexts[ctxnum];
				context->datahisto[source[sx ^ sxor]]++;
			}
		}

		/* advance to the next row */
		source += sstride;
	}

	/* compute optimal trees for each */
	for (ctxnum = 0; ctxnum < numcontexts; ctxnum++)
	{
		huffman_context *context = contexts[ctxnum];
		error = compute_optimal_tree(context, context->datahisto, HUFFMAN_CODES);
		if (error != HUFFERR_NONE)
			return error;
	}
	return HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_deltarle_compute_tree - compute an
    optimal huffman tree for the given source
    data, with pre-encoding as delta-RLE
-------------------------------------------------*/

huffman_error huffman_deltarle_compute_tree(huffman_context *context, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor)
{
	return huffman_deltarle_compute_tree_interleaved(1, &context, source, swidth, sheight, sstride, sxor);
}

huffman_error huffman_deltarle_compute_tree_interleaved(int numcontexts, huffman_context **contexts, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor)
{
	UINT32 sx, sy, ctxnum;
	huffman_error error;

	/* initialize all nodes */
	for (ctxnum = 0; ctxnum < numcontexts; ctxnum++)
	{
		huffman_context *context = contexts[ctxnum];
		memset(context->datahisto, 0, sizeof(context->datahisto));
		context->prevdata = 0;
	}

	/* iterate over "height" */
	for (sy = 0; sy < sheight; sy++)
	{
		/* reset RLE counts */
		for (ctxnum = 0; ctxnum < numcontexts; ctxnum++)
		{
			huffman_context *context = contexts[ctxnum];
			context->rleremaining = 0;
		}

		/* iterate over "width" */
		for (sx = 0; sx < swidth; )
		{
			/* iterate over contexts */
			for (ctxnum = 0; ctxnum < numcontexts; ctxnum++, sx++)
			{
				huffman_context *context = contexts[ctxnum];
				UINT8 newdata, delta;

				/* if still counting RLE, do nothing */
				if (context->rleremaining != 0)
				{
					context->rleremaining--;
					continue;
				}

				/* fetch new data and compute the delta */
				newdata = source[sx ^ sxor];
				delta = newdata - context->prevdata;
				context->prevdata = newdata;

				/* 0 deltas scan forward for a count */
				if (delta == 0)
				{
					int zerocount = 1;
					int rlecode;
					UINT32 scan;

					/* count the number of consecutive values */
					for (scan = sx + 1; scan < swidth; scan++)
						if (contexts[scan % numcontexts] == context)
						{
							if (newdata == source[scan ^ sxor])
								zerocount++;
							else
								break;
						}

					/* if we hit the end of row, maximize the count */
					if (scan >= swidth && zerocount >= 8)
						zerocount = 100000;

					/* encode the maximal count we can */
					rlecode = rlecount_to_code(zerocount);
					context->datahisto[rlecode]++;

					/* set up the remaining count */
					context->rleremaining = code_to_rlecount(rlecode) - 1;
				}
				else
				{
					/* encode the actual delta */
					context->datahisto[delta]++;
				}
			}
		}

		/* advance to the next row */
		source += sstride;
	}

	/* compute optimal trees for each */
	for (ctxnum = 0; ctxnum < numcontexts; ctxnum++)
	{
		huffman_context *context = contexts[ctxnum];
		error = compute_optimal_tree(context, context->datahisto, HUFFMAN_DELTARLE_CODES);
		if (error != HUFFERR_NONE)
			return error;
	}
	return HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_encode_data - encode data using the
    given tree
-------------------------------------------------*/

huffman_error huffman_encode_data(huffman_context *context, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor, UINT8 *dest, UINT32 dlength, UINT32 *actlength)
{
	return huffman_encode_data_interleaved(1, &context, source, swidth, sheight, sstride, sxor, dest, dlength, actlength);
}

huffman_error huffman_encode_data_interleaved(int numcontexts, huffman_context **contexts, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor, UINT8 *dest, UINT32 dlength, UINT32 *actlength)
{
	UINT32 sx, sy, ctxnum;
	bit_buffer bitbuf;

	/* initialize the output buffer */
	bit_buffer_write_init(&bitbuf, dest, dlength);

	/* iterate over "height" */
	for (sy = 0; sy < sheight; sy++)
	{
		/* iterate over "width" */
		for (sx = 0; sx < swidth; )
		{
			/* iterate over contexts */
			for (ctxnum = 0; ctxnum < numcontexts; ctxnum++, sx++)
			{
				huffman_context *context = contexts[ctxnum];
				huffman_node *node = &context->huffnode[source[sx ^ sxor]];
				bit_buffer_write(&bitbuf, node->bits, node->numbits);
			}
		}

		/* advance to the next row */
		source += sstride;
	}

	/* flush and return a status */
	*actlength = bit_buffer_flush(&bitbuf);
	return bitbuf.overflow ? HUFFERR_OUTPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_deltarle_encode_data - encode data
    using the given tree with delta-RLE
    pre-encoding
-------------------------------------------------*/

huffman_error huffman_deltarle_encode_data(huffman_context *context, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor, UINT8 *dest, UINT32 dlength, UINT32 *actlength)
{
	return huffman_deltarle_encode_data_interleaved(1, &context, source, swidth, sheight, sstride, sxor, dest, dlength, actlength);
}

huffman_error huffman_deltarle_encode_data_interleaved(int numcontexts, huffman_context **contexts, const UINT8 *source, UINT32 swidth, UINT32 sheight, UINT32 sstride, UINT32 sxor, UINT8 *dest, UINT32 dlength, UINT32 *actlength)
{
	UINT32 sx, sy, ctxnum;
	bit_buffer bitbuf;

	/* initialize the output buffer */
	bit_buffer_write_init(&bitbuf, dest, dlength);

	/* initialize the contexts */
	for (ctxnum = 0; ctxnum < numcontexts; ctxnum++)
	{
		huffman_context *context = contexts[ctxnum];
		context->prevdata = 0;
	}

	/* iterate over "height" */
	for (sy = 0; sy < sheight; sy++)
	{
		/* reset RLE counts */
		for (ctxnum = 0; ctxnum < numcontexts; ctxnum++)
		{
			huffman_context *context = contexts[ctxnum];
			context->rleremaining = 0;
		}

		/* iterate over "width" */
		for (sx = 0; sx < swidth; )
		{
			/* iterate over contexts */
			for (ctxnum = 0; ctxnum < numcontexts; ctxnum++, sx++)
			{
				huffman_context *context = contexts[ctxnum];
				UINT8 newdata, delta;
				huffman_node *node;

				/* if still counting RLE, do nothing */
				if (context->rleremaining != 0)
				{
					context->rleremaining--;
					continue;
				}

				/* fetch new data and compute the delta */
				newdata = source[sx ^ sxor];
				delta = newdata - context->prevdata;
				context->prevdata = newdata;

				/* 0 deltas scan forward for a count */
				if (delta == 0)
				{
					int zerocount = 1;
					int rlecode;
					UINT32 scan;

					/* count the number of consecutive values */
					for (scan = sx + 1; scan < swidth; scan++)
						if (contexts[scan % numcontexts] == context)
						{
							if (newdata == source[scan ^ sxor])
								zerocount++;
							else
								break;
						}

					/* if we hit the end of row, maximize the count */
					if (scan >= swidth && zerocount >= 8)
						zerocount = 100000;

					/* encode the maximal count we can */
					rlecode = rlecount_to_code(zerocount);
					node = &context->huffnode[rlecode];
					bit_buffer_write(&bitbuf, node->bits, node->numbits);

					/* set up the remaining count */
					context->rleremaining = code_to_rlecount(rlecode) - 1;
				}
				else
				{
					/* encode the actual delta */
					node = &context->huffnode[delta];
					bit_buffer_write(&bitbuf, node->bits, node->numbits);
				}
			}
		}

		/* advance to the next row */
		source += sstride;
	}

	/* flush and return a status */
	*actlength = bit_buffer_flush(&bitbuf);
	return bitbuf.overflow ? HUFFERR_OUTPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_decode_data - decode data using the
    given tree
-------------------------------------------------*/

huffman_error huffman_decode_data(huffman_context *context, const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dwidth, UINT32 dheight, UINT32 dstride, UINT32 dxor, UINT32 *actlength)
{
	const huffman_lookup_value *table;
	int maxbits = context->maxbits;
	huffman_error error;
	bit_buffer bitbuf;
	UINT32 dx, dy;

	/* regenerate the lookup table if necessary */
	if (context->lookupdirty)
	{
		error = build_lookup_table(context, HUFFMAN_CODES);
		if (error != HUFFERR_NONE)
			return error;
	}
	table = context->lookup;

	/* initialize our bit buffer */
	bit_buffer_read_init(&bitbuf, source, slength);

	/* iterate over "height" */
	for (dy = 0; dy < dheight; dy++)
	{
		/* iterate over "width" */
		for (dx = 0; dx < dwidth; dx++)
		{
			huffman_lookup_value lookup;
			UINT32 bits;

			/* peek ahead to get maxbits worth of data */
			bits = bit_buffer_peek(&bitbuf, maxbits);

			/* look it up, then remove the actual number of bits for this code */
			lookup = table[bits];
			bit_buffer_remove(&bitbuf, LOOKUP_BITS(lookup));

			/* store the upper byte */
			dest[dx ^ dxor] = LOOKUP_CODE(lookup);
		}

		/* advance to the next row */
		dest += dstride;
	}

	/* determine the actual length and indicate overflow */
	*actlength = bit_buffer_read_offset(&bitbuf);
	return bitbuf.overflow ? HUFFERR_INPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_decode_data_interleaved - decode
    interleaved data using multiple contexts
-------------------------------------------------*/

huffman_error huffman_decode_data_interleaved(int numcontexts, huffman_context **contexts, const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dwidth, UINT32 dheight, UINT32 dstride, UINT32 dxor, UINT32 *actlength)
{
	UINT32 dx, dy, ctxnum;
	huffman_error error;
	bit_buffer bitbuf;

	/* regenerate the lookup tables if necessary */
	for (ctxnum = 0; ctxnum < numcontexts; ctxnum++)
	{
		huffman_context *context = contexts[ctxnum];
		if (context->lookupdirty)
		{
			error = build_lookup_table(context, HUFFMAN_CODES);
			if (error != HUFFERR_NONE)
				return error;
		}
	}

	/* initialize our bit buffer */
	bit_buffer_read_init(&bitbuf, source, slength);

	/* iterate over "height" */
	for (dy = 0; dy < dheight; dy++)
	{
		/* iterate over "width" */
		for (dx = 0; dx < dwidth; )
		{
			/* iterate over contexts */
			for (ctxnum = 0; ctxnum < numcontexts; ctxnum++, dx++)
			{
				huffman_context *context = contexts[ctxnum];
				huffman_lookup_value lookup;
				UINT32 bits;

				/* peek ahead to get maxbits worth of data */
				bits = bit_buffer_peek(&bitbuf, context->maxbits);

				/* look it up, then remove the actual number of bits for this code */
				lookup = context->lookup[bits];
				bit_buffer_remove(&bitbuf, LOOKUP_BITS(lookup));

				/* store the upper byte */
				dest[dx ^ dxor] = LOOKUP_CODE(lookup);
			}
		}

		/* advance to the next row */
		dest += dstride;
	}

	/* determine the actual length and indicate overflow */
	*actlength = bit_buffer_read_offset(&bitbuf);
	return bitbuf.overflow ? HUFFERR_INPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_deltarle_decode_data - decode data
    using the given tree with delta-RLE
    post-decoding
-------------------------------------------------*/

huffman_error huffman_deltarle_decode_data(huffman_context *context, const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dwidth, UINT32 dheight, UINT32 dstride, UINT32 dxor, UINT32 *actlength)
{
	const huffman_lookup_value *table;
	int maxbits = context->maxbits;
	UINT32 rleremaining = 0;
	huffman_error error;
	UINT8 prevdata = 0;
	bit_buffer bitbuf;
	UINT32 dx, dy;

	/* regenerate the lookup table if necessary */
	if (context->lookupdirty)
	{
		error = build_lookup_table(context, HUFFMAN_DELTARLE_CODES);
		if (error != HUFFERR_NONE)
			return error;
	}
	table = context->lookup;

	/* initialize our bit buffer */
	bit_buffer_read_init(&bitbuf, source, slength);

	/* iterate over "height" */
	for (dy = 0; dy < dheight; dy++)
	{
		/* reset RLE counts */
		rleremaining = 0;

		/* iterate over "width" */
		for (dx = 0; dx < dwidth; dx++)
		{
			huffman_lookup_value lookup;
			UINT32 bits;
			int data;

			/* if we have RLE remaining, just store that */
			if (rleremaining != 0)
			{
				rleremaining--;
				dest[dx ^ dxor] = prevdata;
				continue;
			}

			/* peek ahead to get maxbits worth of data */
			bits = bit_buffer_peek(&bitbuf, maxbits);

			/* look it up, then remove the actual number of bits for this code */
			lookup = table[bits];
			bit_buffer_remove(&bitbuf, LOOKUP_BITS(lookup));

			/* compute the data and handle RLE decoding */
			data = LOOKUP_CODE(lookup);

			/* if not an RLE special, just add to the previous; otherwise, start counting RLE */
			if (data < 0x100)
				prevdata += (UINT8)data;
			else
				rleremaining = code_to_rlecount(data) - 1;

			/* store the updated data value */
			dest[dx ^ dxor] = prevdata;
		}

		/* advance to the next row */
		dest += dstride;
	}

	/* determine the actual length and indicate overflow */
	*actlength = bit_buffer_read_offset(&bitbuf);
	return bitbuf.overflow ? HUFFERR_INPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_deltarle_decode_data_interleaved -
    decode data using multiple contexts and
    delta-RLE post-decoding
-------------------------------------------------*/

huffman_error huffman_deltarle_decode_data_interleaved(int numcontexts, huffman_context **contexts, const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dwidth, UINT32 dheight, UINT32 dstride, UINT32 dxor, UINT32 *actlength)
{
	UINT32 dx, dy, ctxnum;
	huffman_error error;
	bit_buffer bitbuf;

	/* fast case the A/V Y/Cb/Y/Cr case */
	if (numcontexts == 4 && contexts[0] == contexts[2] && contexts[0] != contexts[1] && contexts[1] != contexts[3] &&
		contexts[0]->maxbits == contexts[1]->maxbits && contexts[0]->maxbits == contexts[3]->maxbits)
		return huffman_deltarle_decode_data_interleaved_0102(contexts, source, slength, dest, dwidth, dheight, dstride, dxor, actlength);

	/* regenerate the lookup tables if necessary */
	for (ctxnum = 0; ctxnum < numcontexts; ctxnum++)
	{
		huffman_context *context = contexts[ctxnum];
		if (context->lookupdirty)
		{
			error = build_lookup_table(context, HUFFMAN_DELTARLE_CODES);
			if (error != HUFFERR_NONE)
				return error;
		}
		context->prevdata = 0;
	}

	/* initialize our bit buffer */
	bit_buffer_read_init(&bitbuf, source, slength);

	/* iterate over "height" */
	for (dy = 0; dy < dheight; dy++)
	{
		/* reset RLE counts */
		for (ctxnum = 0; ctxnum < numcontexts; ctxnum++)
		{
			huffman_context *context = contexts[ctxnum];
			context->rleremaining = 0;
		}

		/* iterate over "width" */
		for (dx = 0; dx < dwidth; )
		{
			/* iterate over contexts */
			for (ctxnum = 0; ctxnum < numcontexts; ctxnum++, dx++)
			{
				huffman_context *context = contexts[ctxnum];
				huffman_lookup_value lookup;
				UINT32 bits;
				int data;

				/* if we have RLE remaining, just store that */
				if (context->rleremaining != 0)
				{
					context->rleremaining--;
					dest[dx ^ dxor] = context->prevdata;
					continue;
				}

				/* peek ahead to get maxbits worth of data */
				bits = bit_buffer_peek(&bitbuf, context->maxbits);

				/* look it up, then remove the actual number of bits for this code */
				lookup = context->lookup[bits];
				bit_buffer_remove(&bitbuf, LOOKUP_BITS(lookup));

				/* compute the data and handle RLE decoding */
				data = LOOKUP_CODE(lookup);

				/* if not an RLE special, just add to the previous; otherwise, start counting RLE */
				if (data < 0x100)
					context->prevdata += (UINT8)data;
				else
					context->rleremaining = code_to_rlecount(data) - 1;

				/* store the updated data value */
				dest[dx ^ dxor] = context->prevdata;
			}
		}

		/* advance to the next row */
		dest += dstride;
	}

	/* determine the actual length and indicate overflow */
	*actlength = bit_buffer_read_offset(&bitbuf);
	return bitbuf.overflow ? HUFFERR_INPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_deltarle_decode_data_interleaved_0102 -
    decode data using 3 unique contexts in
    0/1/0/2 order (used for Y/Cb/Y/Cr encoding)
-------------------------------------------------*/

static huffman_error huffman_deltarle_decode_data_interleaved_0102(huffman_context **contexts, const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dwidth, UINT32 dheight, UINT32 dstride, UINT32 dxor, UINT32 *actlength)
{
	const huffman_lookup_value *table02, *table1, *table3;
	int rleremaining02, rleremaining1, rleremaining3;
	UINT8 prevdata02 = 0, prevdata1 = 0, prevdata3 = 0;
	int maxbits = contexts[0]->maxbits;
	huffman_error error;
	bit_buffer bitbuf;
	UINT32 dx, dy;

	/* regenerate the lookup tables if necessary */
	if (contexts[0]->lookupdirty)
	{
		error = build_lookup_table(contexts[0], HUFFMAN_DELTARLE_CODES);
		if (error != HUFFERR_NONE)
			return error;
	}
	if (contexts[1]->lookupdirty)
	{
		error = build_lookup_table(contexts[1], HUFFMAN_DELTARLE_CODES);
		if (error != HUFFERR_NONE)
			return error;
	}
	if (contexts[3]->lookupdirty)
	{
		error = build_lookup_table(contexts[3], HUFFMAN_DELTARLE_CODES);
		if (error != HUFFERR_NONE)
			return error;
	}

	/* cache the tables locally */
	table02 = contexts[0]->lookup;
	table1 = contexts[1]->lookup;
	table3 = contexts[3]->lookup;

	/* initialize our bit buffer */
	bit_buffer_read_init(&bitbuf, source, slength);

	/* iterate over "height" */
	for (dy = 0; dy < dheight; dy++)
	{
		/* reset RLE counts */
		rleremaining02 = rleremaining1 = rleremaining3 = 0;

		/* iterate over "width" */
		for (dx = 0; dx < dwidth; dx += 4)
		{
			huffman_lookup_value lookup;
			UINT32 bits;
			int data;

			/* ----- offset 0 ----- */

			/* if we have RLE remaining, just store that */
			if (rleremaining02 != 0)
				rleremaining02--;
			else
			{
				/* peek ahead to get maxbits worth of data */
				bits = bit_buffer_peek(&bitbuf, maxbits);

				/* look it up, then remove the actual number of bits for this code */
				lookup = table02[bits];
				bit_buffer_remove(&bitbuf, LOOKUP_BITS(lookup));

				/* compute the data and handle RLE decoding */
				data = LOOKUP_CODE(lookup);

				/* if not an RLE special, just add to the previous; otherwise, start counting RLE */
				if (data < 0x100)
					prevdata02 += (UINT8)data;
				else
					rleremaining02 = code_to_rlecount(data) - 1;
			}

			/* store the updated data value */
			dest[(dx + 0) ^ dxor] = prevdata02;

			/* ----- offset 1 ----- */

			/* if we have RLE remaining, just store that */
			if (rleremaining1 != 0)
				rleremaining1--;
			else
			{
				/* peek ahead to get maxbits worth of data */
				bits = bit_buffer_peek(&bitbuf, maxbits);

				/* look it up, then remove the actual number of bits for this code */
				lookup = table1[bits];
				bit_buffer_remove(&bitbuf, LOOKUP_BITS(lookup));

				/* compute the data and handle RLE decoding */
				data = LOOKUP_CODE(lookup);

				/* if not an RLE special, just add to the previous; otherwise, start counting RLE */
				if (data < 0x100)
					prevdata1 += (UINT8)data;
				else
					rleremaining1 = code_to_rlecount(data) - 1;
			}

			/* store the updated data value */
			dest[(dx + 1) ^ dxor] = prevdata1;

			/* ----- offset 2 (same as 0) ----- */

			/* if we have RLE remaining, just store that */
			if (rleremaining02 != 0)
				rleremaining02--;
			else
			{
				/* peek ahead to get maxbits worth of data */
				bits = bit_buffer_peek(&bitbuf, maxbits);

				/* look it up, then remove the actual number of bits for this code */
				lookup = table02[bits];
				bit_buffer_remove(&bitbuf, LOOKUP_BITS(lookup));

				/* compute the data and handle RLE decoding */
				data = LOOKUP_CODE(lookup);

				/* if not an RLE special, just add to the previous; otherwise, start counting RLE */
				if (data < 0x100)
					prevdata02 += (UINT8)data;
				else
					rleremaining02 = code_to_rlecount(data) - 1;
			}

			/* store the updated data value */
			dest[(dx + 2) ^ dxor] = prevdata02;

			/* ----- offset 3 ----- */

			/* if we have RLE remaining, just store that */
			if (rleremaining3 != 0)
				rleremaining3--;
			else
			{
				/* peek ahead to get maxbits worth of data */
				bits = bit_buffer_peek(&bitbuf, maxbits);

				/* look it up, then remove the actual number of bits for this code */
				lookup = table3[bits];
				bit_buffer_remove(&bitbuf, LOOKUP_BITS(lookup));

				/* compute the data and handle RLE decoding */
				data = LOOKUP_CODE(lookup);

				/* if not an RLE special, just add to the previous; otherwise, start counting RLE */
				if (data < 0x100)
					prevdata3 += (UINT8)data;
				else
					rleremaining3 = code_to_rlecount(data) - 1;
			}

			/* store the updated data value */
			dest[(dx + 3) ^ dxor] = prevdata3;
		}

		/* advance to the next row */
		dest += dstride;
	}

	/* determine the actual length and indicate overflow */
	*actlength = bit_buffer_read_offset(&bitbuf);
	return bitbuf.overflow ? HUFFERR_INPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}



/***************************************************************************
    INTERNAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    import_tree - import a huffman tree from a
    source data stream
-------------------------------------------------*/

static huffman_error import_tree(huffman_context *context, const UINT8 *source, UINT32 slength, UINT32 *actlength, UINT32 numcodes)
{
	huffman_error error;
	bit_buffer bitbuf;
	int curnode;
	int numbits;

	/* initialize the input buffer */
	bit_buffer_read_init(&bitbuf, source, slength);

	/* bits per entry depends on the maxbits */
	if (context->maxbits >= 16)
		numbits = 5;
	else if (context->maxbits >= 8)
		numbits = 4;
	else
		numbits = 3;

	/* loop until we read all the nodes */
	for (curnode = 0; curnode < numcodes; )
	{
		int nodebits = bit_buffer_read(&bitbuf, numbits);

		/* a non-one value is just raw */
		if (nodebits != 1)
			context->huffnode[curnode++].numbits = nodebits;

		/* a one value is an escape code */
		else
		{
			nodebits = bit_buffer_read(&bitbuf, numbits);

			/* a double 1 is just a single 1 */
			if (nodebits == 1)
				context->huffnode[curnode++].numbits = nodebits;

			/* otherwise, we need one for value for the repeat count */
			else
			{
				int repcount = bit_buffer_read(&bitbuf, numbits) + 3;
				while (repcount--)
					context->huffnode[curnode++].numbits = nodebits;
			}
		}
	}

	/* assign canonical codes for all nodes based on their code lengths */
	error = assign_canonical_codes(context, numcodes);
	if (error != HUFFERR_NONE)
		return error;

	/* make sure we ended up with the right number */
	if (curnode != numcodes)
		return HUFFERR_INVALID_DATA;

	*actlength = bit_buffer_read_offset(&bitbuf);
	return bitbuf.overflow ? HUFFERR_INPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


/*-------------------------------------------------
    export_tree - export a huffman tree to a
    target data stream
-------------------------------------------------*/

static huffman_error export_tree(huffman_context *context, UINT8 *dest, UINT32 dlength, UINT32 *actlength, UINT32 numcodes)
{
	bit_buffer bitbuf;
	int repcount;
	int lastval;
	int numbits;
	int i;

	/* initialize the output buffer */
	bit_buffer_write_init(&bitbuf, dest, dlength);

	/* bits per entry depends on the maxbits */
	if (context->maxbits >= 16)
		numbits = 5;
	else if (context->maxbits >= 8)
		numbits = 4;
	else
		numbits = 3;

	/* RLE encode the lengths */
	lastval = ~0;
	repcount = 0;
	for (i = 0; i < numcodes; i++)
	{
		int newval = context->huffnode[i].numbits;

		/* if we match the previous value, just bump the repcount */
		if (newval == lastval)
			repcount++;

		/* otherwise, we need to flush the previous repeats */
		else
		{
			if (repcount != 0)
				write_rle_tree_bits(&bitbuf, lastval, repcount, numbits);
			lastval = newval;
			repcount = 1;
		}
	}

	/* flush the last value */
	write_rle_tree_bits(&bitbuf, lastval, repcount, numbits);
	*actlength = bit_buffer_flush(&bitbuf);
	return bitbuf.overflow ? HUFFERR_OUTPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


/*-------------------------------------------------
    write_rle_tree_bits - write an RLE encoded
    set of data to a target stream
-------------------------------------------------*/

static void write_rle_tree_bits(bit_buffer *bitbuf, int value, int repcount, int numbits)
{
	/* loop until we have output all of the repeats */
	while (repcount > 0)
	{
		/* if we have a 1, write it twice as it is an escape code */
		if (value == 1)
		{
			bit_buffer_write(bitbuf, 1, numbits);
			bit_buffer_write(bitbuf, 1, numbits);
			repcount--;
		}

		/* if we have two or fewer in a row, write them raw */
		else if (repcount <= 2)
		{
			bit_buffer_write(bitbuf, value, numbits);
			repcount--;
		}

		/* otherwise, write a triple using 1 as the escape code */
		else
		{
			int cur_reps = MIN(repcount - 3, (1 << numbits) - 1);
			bit_buffer_write(bitbuf, 1, numbits);
			bit_buffer_write(bitbuf, value, numbits);
			bit_buffer_write(bitbuf, cur_reps, numbits);
			repcount -= cur_reps + 3;
		}
	}
}


/*-------------------------------------------------
    tree_node_compare - compare two tree nodes
    by weight
-------------------------------------------------*/

static int CLIB_DECL tree_node_compare(const void *item1, const void *item2)
{
	const huffman_node *node1 = *(const huffman_node **)item1;
	const huffman_node *node2 = *(const huffman_node **)item2;
	return node2->weight - node1->weight;
}


/*-------------------------------------------------
    compute_optimal_tree - common backend for
    computing a tree based on the data histogram
-------------------------------------------------*/

static huffman_error compute_optimal_tree(huffman_context *context, const UINT32 *datahisto, UINT32 numcodes)
{
	UINT32 lowerweight, upperweight;
	UINT32 sdatacount;
	int i;

	/* compute the number of data items in the histogram */
	sdatacount = 0;
	for (i = 0; i < numcodes; i++)
		sdatacount += datahisto[i];

	/* binary search to achieve the optimum encoding */
	lowerweight = 0;
	upperweight = sdatacount * 2;
	while (TRUE)
	{
		UINT32 curweight = (upperweight + lowerweight) / 2;
		int curmaxbits;

		/* build a tree using the current weight */
		curmaxbits = huffman_build_tree(context, datahisto, sdatacount, curweight, numcodes);

		/* apply binary search here */
		if (curmaxbits <= context->maxbits)
		{
			lowerweight = curweight;

			/* early out if it worked with the raw weights, or if we're done searching */
			if (curweight == sdatacount || (upperweight - lowerweight) <= 1)
				break;
		}
		else
			upperweight = curweight;
	}

	/* assign canonical codes for all nodes based on their code lengths */
	return assign_canonical_codes(context, numcodes);
}


/*-------------------------------------------------
    huffman_build_tree - build a huffman tree
    based on the data distribution
-------------------------------------------------*/

static int huffman_build_tree(huffman_context *context, const UINT32 *datahisto, UINT32 totaldata, UINT32 totalweight, UINT32 numcodes)
{
	huffman_node *list[MAX_HUFFMAN_CODES];
	int listitems;
	int nextalloc;
	int maxbits;
	int i;

	/* make a list of all non-zero nodes */
	listitems = 0;
	memset(context->huffnode, 0, numcodes * sizeof(context->huffnode[0]));
	for (i = 0; i < numcodes; i++)
		if (datahisto[i] != 0)
		{
			list[listitems++] = &context->huffnode[i];
			context->huffnode[i].count = datahisto[i];

			/* scale the weight by the current effective length, ensuring we don't go to 0 */
			context->huffnode[i].weight = (UINT64)datahisto[i] * (UINT64)totalweight / (UINT64)totaldata;
			if (context->huffnode[i].weight == 0)
				context->huffnode[i].weight = 1;
		}

	/* sort the list by weight, largest weight first */
	qsort(list, listitems, sizeof(list[0]), tree_node_compare);

	/* now build the tree */
	nextalloc = MAX_HUFFMAN_CODES;
	while (listitems > 1)
	{
		huffman_node *node0, *node1, *newnode;

		/* remove lowest two items */
		node1 = list[--listitems];
		node0 = list[--listitems];

		/* create new node */
		newnode = &context->huffnode[nextalloc++];
		newnode->parent = NULL;
		node0->parent = node1->parent = newnode;
		newnode->weight = node0->weight + node1->weight;

		/* insert into list at appropriate location */
		for (i = 0; i < listitems; i++)
			if (newnode->weight > list[i]->weight)
			{
				memmove(&list[i+1], &list[i], (listitems - i) * sizeof(list[0]));
				break;
			}
		list[i] = newnode;
		listitems++;
	}

	/* compute the number of bits in each code, and fill in another histogram */
	maxbits = 0;
	for (i = 0; i < numcodes; i++)
	{
		huffman_node *node = &context->huffnode[i];
		node->numbits = 0;

		/* if we have a non-zero weight, compute the number of bits */
		if (node->weight > 0)
		{
			huffman_node *curnode;

			/* determine the number of bits for this node */
			for (curnode = node; curnode->parent != NULL; curnode = curnode->parent)
				node->numbits++;
			if (node->numbits == 0)
				node->numbits = 1;

			/* keep track of the max */
			maxbits = MAX(maxbits, node->numbits);
		}
	}

	return maxbits;
}


/*-------------------------------------------------
    assign_canonical_codes - assign
    canonical codes to all the nodes based on the
    number of bits in each
-------------------------------------------------*/

static huffman_error assign_canonical_codes(huffman_context *context, UINT32 numcodes)
{
	UINT32 bithisto[33];
	int curstart;
	int i;

	/* build up a histogram of bit lengths */
	memset(bithisto, 0, sizeof(bithisto));
	for (i = 0; i < numcodes; i++)
	{
		huffman_node *node = &context->huffnode[i];
		if (node->numbits > context->maxbits)
			return HUFFERR_INTERNAL_INCONSISTENCY;
		if (node->numbits <= 32)
			bithisto[node->numbits]++;
	}

	/* for each code length, determine the starting code number */
	curstart = 0;
	for (i = 32; i > 0; i--)
	{
		UINT32 nextstart = (curstart + bithisto[i]) >> 1;
		if (i != 1 && nextstart * 2 != (curstart + bithisto[i]))
			return HUFFERR_INTERNAL_INCONSISTENCY;
		bithisto[i] = curstart;
		curstart = nextstart;
	}

	/* now assign canonical codes */
	for (i = 0; i < numcodes; i++)
	{
		huffman_node *node = &context->huffnode[i];
		if (node->numbits > 0)
			node->bits = bithisto[node->numbits]++;
	}

	/* if there was a decoding table, get rid of it now */
	context->lookupdirty = TRUE;
	return HUFFERR_NONE;
}


/*-------------------------------------------------
    build_lookup_table - build a lookup
    table for fast decoding
-------------------------------------------------*/

static huffman_error build_lookup_table(huffman_context *context, UINT32 numcodes)
{
	int i;

	/* allocate a table if needed */
	if (context->lookup == NULL)
		context->lookup = (huffman_lookup_value *)malloc((UINT32)sizeof(context->lookup[0]) * (UINT32)(1 << context->maxbits));
	if (context->lookup == NULL)
		return HUFFERR_OUT_OF_MEMORY;

	/* now build */
	for (i = 0; i < numcodes; i++)
	{
		huffman_node *node = &context->huffnode[i];
		if (node->numbits > 0)
		{
			huffman_lookup_value *dest, *destend;

			/* left justify this node's bit values to max bits */
			int shift = context->maxbits - node->numbits;
			UINT32 start = node->bits << shift;
			UINT32 end = ((node->bits + 1) << shift) - 1;
			huffman_lookup_value value;

			/* set up the entry */
			value = (i << 6) | node->numbits;

			/* fill all matching entries */
			dest = &context->lookup[start];
			destend = &context->lookup[end];
			while (dest <= destend)
				*dest++ = value;
		}
	}

	/* no longer dirty */
	context->lookupdirty = FALSE;
	return HUFFERR_NONE;
}
