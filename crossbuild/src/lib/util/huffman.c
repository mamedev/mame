/***************************************************************************

    huffman.c

    Video compression and decompression helpers.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

***************************************************************************/

#include "huffman.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_HUFFMAN_NODES		(256 + 256)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _bit_buffer bit_buffer;
struct _bit_buffer
{
	UINT32			buffer;
	int				bits;
	union
	{
		const UINT8 *read;
		UINT8 *		write;
	} data;
	UINT32			doffset;
	UINT32			dlength;
	int				overflow;
};


typedef struct _huffman_node huffman_node;
struct _huffman_node
{
	huffman_node *	parent;
	UINT32			count;
	UINT32			weight;
	UINT32			bits;
	UINT8			numbits;
};


struct _huffman_context
{
	UINT8			maxbits;
	UINT8			lookupdirty;
	huffman_node 	huffnode[MAX_HUFFMAN_NODES];
	UINT32			lookupmask;
	huffman_lookup_value *lookup;
};



/***************************************************************************
    PROTOTYPES
***************************************************************************/

static void huffman_write_rle_tree_bits(bit_buffer *bitbuf, int value, int repcount, int numbits);
static int CLIB_DECL huffman_tree_node_compare(const void *item1, const void *item2);
static int huffman_build_tree(huffman_context *context, const UINT32 *datahisto, UINT32 totaldata, UINT32 totalweight);
static huffman_error huffman_assign_canonical_codes(huffman_context *context);
static huffman_error huffman_build_lookup_table(huffman_context *context);



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
    the buffer, returning the right-justified
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
	*context = malloc(sizeof(**context));
	if (*context == NULL)
		return HUFFERR_OUT_OF_MEMORY;

	/* set the info */
	memset(*context, 0, sizeof(**context));
	(*context)->maxbits = maxbits;
	(*context)->lookupmask = (1 << maxbits) - 1;
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
    huffman_compute_tree - compute an optimal
    huffman tree for the given source data
-------------------------------------------------*/

huffman_error huffman_compute_tree(huffman_context *context, const UINT8 *source, UINT32 slength, UINT32 sstride)
{
	UINT32 lowerweight, upperweight;
	UINT32 datahisto[256];
	int i;

	/* build the data histogram */
	memset(datahisto, 0, sizeof(datahisto));
	for (i = 0; i < slength; i += sstride)
		datahisto[source[i]]++;

	/* binary search to achieve the optimum encoding */
	lowerweight = 0;
	upperweight = slength * 2;
	while (TRUE)
	{
		UINT32 curweight = (upperweight + lowerweight) / 2;
		int curmaxbits;

		/* build a tree using the current weight */
		curmaxbits = huffman_build_tree(context, datahisto, slength, curweight);

		/* apply binary search here */
		if (curmaxbits <= context->maxbits)
		{
			lowerweight = curweight;

			/* early out if it worked with the raw weights, or if we're done searching */
			if (curweight == slength || (upperweight - lowerweight) <= 1)
				break;
		}
		else
			upperweight = curweight;
	}

	/* assign canonical codes for all nodes based on their code lengths */
	return huffman_assign_canonical_codes(context);
}


/*-------------------------------------------------
    huffman_import_tree - import a huffman tree
    from a source data stream
-------------------------------------------------*/

huffman_error huffman_import_tree(huffman_context *context, const UINT8 *source, UINT32 slength, UINT32 *actlength)
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
	for (curnode = 0; curnode < 256; )
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
	error = huffman_assign_canonical_codes(context);
	if (error != HUFFERR_NONE)
		return error;

	/* make sure we ended up with the right number */
	if (curnode != 256)
		return HUFFERR_INVALID_DATA;

	*actlength = bit_buffer_read_offset(&bitbuf);
	return bitbuf.overflow ? HUFFERR_INPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_export_tree - export a huffman tree
    to a target data stream
-------------------------------------------------*/

huffman_error huffman_export_tree(huffman_context *context, UINT8 *dest, UINT32 dlength, UINT32 *actlength)
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
	for (i = 0; i < 256; i++)
	{
		int newval = context->huffnode[i].numbits;

		/* if we match the previous value, just bump the repcount */
		if (newval == lastval)
			repcount++;

		/* otherwise, we need to flush the previous repeats */
		else
		{
			if (repcount != 0)
				huffman_write_rle_tree_bits(&bitbuf, lastval, repcount, numbits);
			lastval = newval;
			repcount = 1;
		}
	}

	/* flush the last value */
	huffman_write_rle_tree_bits(&bitbuf, lastval, repcount, numbits);
	*actlength = bit_buffer_flush(&bitbuf);
	return bitbuf.overflow ? HUFFERR_OUTPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_get_lookup_table - return a pointer to
    the lookup table
-------------------------------------------------*/

huffman_error huffman_get_lookup_table(huffman_context *context, const huffman_lookup_value **table)
{
	if (context->lookupdirty)
	{
		huffman_error error = huffman_build_lookup_table(context);
		if (error != HUFFERR_NONE)
			return error;
	}
	*table = context->lookup;
	return HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_encode_data - encode data using the
    current tree
-------------------------------------------------*/

huffman_error huffman_encode_data(huffman_context *context, const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dlength, UINT32 *actlength)
{
	bit_buffer bitbuf;
	UINT32 soffset;

	/* initialize the output buffer */
	bit_buffer_write_init(&bitbuf, dest, dlength);

	/* loop over source data and encode */
	for (soffset = 0; soffset < slength; soffset++)
	{
		huffman_node *node = &context->huffnode[source[soffset]];
		bit_buffer_write(&bitbuf, node->bits, node->numbits);
	}
	*actlength = bit_buffer_flush(&bitbuf);
	return bitbuf.overflow ? HUFFERR_OUTPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_encode_data_interleaved_2 - encode
    alternating data with two contexts
-------------------------------------------------*/

huffman_error huffman_encode_data_interleaved_2(huffman_context *context1, huffman_context *context2, const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dlength, UINT32 *actlength)
{
	bit_buffer bitbuf;
	UINT32 soffset;

	/* initialize the output buffer */
	bit_buffer_write_init(&bitbuf, dest, dlength);

	/* loop over source data and encode */
	for (soffset = 0; soffset < slength; soffset += 2)
	{
		huffman_node *node;

		node = &context1->huffnode[source[soffset + 0]];
		bit_buffer_write(&bitbuf, node->bits, node->numbits);

		node = &context2->huffnode[source[soffset + 1]];
		bit_buffer_write(&bitbuf, node->bits, node->numbits);
	}
	*actlength = bit_buffer_flush(&bitbuf);
	return bitbuf.overflow ? HUFFERR_OUTPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_decode_data - decode data using the
    current tree
-------------------------------------------------*/

huffman_error huffman_decode_data(huffman_context *context, const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dlength, UINT32 *actlength)
{
	int maxbits = context->maxbits;
	int shiftbits = 32 - maxbits;
	const huffman_lookup_value *table;
	int overflow = FALSE;
	huffman_error error;
	UINT32 doffset = 0;
	UINT32 soffset = 0;
	UINT32 bitbuf = 0;
	int sbits = 0;

	/* regenerate the lookup table if necessary */
	error = huffman_get_lookup_table(context, &table);
	if (error != HUFFERR_NONE)
		return error;

	/* decode until we process all of the destination data */
	for (doffset = 0; doffset < dlength; doffset++)
	{
		huffman_lookup_value lookup;

		/* if we don't have enough bits, load up the buffer */
		if (sbits < maxbits)
		{
			while (sbits <= 24)
			{
				if (soffset < slength)
					bitbuf |= source[soffset] << (24 - sbits);
				soffset++;
				sbits += 8;
			}
			if (sbits < maxbits)
				overflow = TRUE;
		}

		/* lookup the data */
		lookup = table[bitbuf >> shiftbits];

		/* store the upper byte */
		dest[doffset] = lookup >> 8;

		/* count the bits */
		lookup &= 0x1f;
		bitbuf <<= lookup;
		sbits -= lookup;
	}

	/* back off soffset while we have whole bytes */
	while (sbits >= 8)
	{
		sbits -= 8;
		soffset--;
	}
	*actlength = soffset;
	return overflow ? HUFFERR_INPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_decode_data_interleaved_2 - decode
    interleaved data using two contexts
-------------------------------------------------*/

huffman_error huffman_decode_data_interleaved_2(huffman_context *context1, huffman_context *context2, const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dlength, UINT32 *actlength)
{
	int maxbits1 = context1->maxbits, maxbits2 = context2->maxbits;
	const huffman_lookup_value *table1, *table2;
	int shiftbits1 = 32 - maxbits1;
	int shiftbits2 = 32 - maxbits2;
	int overflow = FALSE;
	huffman_error error;
	UINT32 doffset = 0;
	UINT32 soffset = 0;
	UINT32 bitbuf = 0;
	int sbits = 0;

	/* regenerate the lookup table if necessary */
	error = huffman_get_lookup_table(context1, &table1);
	if (error != HUFFERR_NONE)
		return error;
	error = huffman_get_lookup_table(context2, &table2);
	if (error != HUFFERR_NONE)
		return error;

	/* decode until we process all of the destination data */
	for (doffset = 0; doffset < dlength; doffset += 2)
	{
		huffman_lookup_value lookup;

		/* if we don't have enough bits, load up the buffer */
		if (sbits < maxbits1)
		{
			while (sbits <= 24)
			{
				if (soffset < slength)
					bitbuf |= source[soffset] << (24 - sbits);
				soffset++;
				sbits += 8;
			}
			if (sbits < maxbits1)
				overflow = TRUE;
		}

		/* lookup the data */
		lookup = table1[bitbuf >> shiftbits1];

		/* store the upper byte */
		dest[doffset + 0] = lookup >> 8;

		/* count the bits */
		lookup &= 0x1f;
		bitbuf <<= lookup;
		sbits -= lookup;

		/* if we don't have enough bits, load up the buffer */
		if (sbits < maxbits2)
		{
			while (sbits <= 24)
			{
				if (soffset < slength)
					bitbuf |= source[soffset] << (24 - sbits);
				soffset++;
				sbits += 8;
			}
			if (sbits < maxbits2)
				overflow = TRUE;
		}

		/* lookup the data */
		lookup = table2[bitbuf >> shiftbits2];

		/* store the upper byte */
		dest[doffset + 1] = lookup >> 8;

		/* count the bits */
		lookup &= 0x1f;
		bitbuf <<= lookup;
		sbits -= lookup;
	}

	/* back off soffset while we have whole bytes */
	while (sbits >= 8)
	{
		sbits -= 8;
		soffset--;
	}
	*actlength = soffset;
	return overflow ? HUFFERR_INPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


/*-------------------------------------------------
    huffman_write_rle_tree_bits - write an RLE
    encoded set of data to a target stream
-------------------------------------------------*/

static void huffman_write_rle_tree_bits(bit_buffer *bitbuf, int value, int repcount, int numbits)
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
    huffman_tree_node_compare - compare two
    tree nodes by weight
-------------------------------------------------*/

static int CLIB_DECL huffman_tree_node_compare(const void *item1, const void *item2)
{
	const huffman_node *node1 = *(const huffman_node **)item1;
	const huffman_node *node2 = *(const huffman_node **)item2;
	return node2->weight - node1->weight;
}


/*-------------------------------------------------
    huffman_build_tree - build a huffman tree
    based on the data distribution
-------------------------------------------------*/

static int huffman_build_tree(huffman_context *context, const UINT32 *datahisto, UINT32 totaldata, UINT32 totalweight)
{
	huffman_node *list[256];
	int listitems;
	int nextalloc;
	int maxbits;
	int i;

	/* make a list of all non-zero nodes */
	listitems = 0;
	memset(context->huffnode, 0, 256 * sizeof(context->huffnode[0]));
	for (i = 0; i < 256; i++)
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
	qsort(list, listitems, sizeof(list[0]), huffman_tree_node_compare);

	/* now build the tree */
	nextalloc = 256;
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
	for (i = 0; i < 256; i++)
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
    huffman_assign_canonical_codes - assign
    canonical codes to all the nodes based on the
    number of bits in each
-------------------------------------------------*/

static huffman_error huffman_assign_canonical_codes(huffman_context *context)
{
	UINT32 bithisto[33];
	int curstart;
	int i;

	/* build up a histogram of bit lengths */
	memset(bithisto, 0, sizeof(bithisto));
	for (i = 0; i < 256; i++)
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
	for (i = 0; i < 256; i++)
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
    huffman_build_lookup_table - build a lookup
    table for fast decoding
-------------------------------------------------*/

static huffman_error huffman_build_lookup_table(huffman_context *context)
{
	int i;

	/* allocate a table if needed */
	if (context->lookup == NULL)
		context->lookup = malloc((UINT32)sizeof(context->lookup[0]) * (UINT32)(1 << context->maxbits));
	if (context->lookup == NULL)
		return HUFFERR_OUT_OF_MEMORY;

	/* now build */
	for (i = 0; i < 256; i++)
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
			value = (i << 8) | node->numbits;

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
