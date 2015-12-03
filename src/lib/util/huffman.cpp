// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    huffman.c

    Static Huffman compression and decompression helpers.

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

#include <stdlib.h>
#include <assert.h>

#include "coretmpl.h"
#include "huffman.h"



//**************************************************************************
//  MACROS
//**************************************************************************

#define MAKE_LOOKUP(code,bits)  (((code) << 5) | ((bits) & 0x1f))



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  huffman_context_base - create an encoding/
//  decoding context
//-------------------------------------------------

huffman_context_base::huffman_context_base(int numcodes, int maxbits, lookup_value *lookup, UINT32 *histo, node_t *nodes)
	: m_numcodes(numcodes),
		m_maxbits(maxbits),
		m_prevdata(0),
		m_rleremaining(0),
		m_lookup(lookup),
		m_datahisto(histo),
		m_huffnode(nodes)
{
	// limit to 24 bits
	if (maxbits > 24)
		throw HUFFERR_TOO_MANY_BITS;
}


//-------------------------------------------------
//  import_tree_rle - import an RLE-encoded
//  huffman tree from a source data stream
//-------------------------------------------------

huffman_error huffman_context_base::import_tree_rle(bitstream_in &bitbuf)
{
	// bits per entry depends on the maxbits
	int numbits;
	if (m_maxbits >= 16)
		numbits = 5;
	else if (m_maxbits >= 8)
		numbits = 4;
	else
		numbits = 3;

	// loop until we read all the nodes
	int curnode;
	for (curnode = 0; curnode < m_numcodes; )
	{
		// a non-one value is just raw
		int nodebits = bitbuf.read(numbits);
		if (nodebits != 1)
			m_huffnode[curnode++].m_numbits = nodebits;

		// a one value is an escape code
		else
		{
			// a double 1 is just a single 1
			nodebits = bitbuf.read(numbits);
			if (nodebits == 1)
				m_huffnode[curnode++].m_numbits = nodebits;

			// otherwise, we need one for value for the repeat count
			else
			{
				int repcount = bitbuf.read(numbits) + 3;
				while (repcount--)
					m_huffnode[curnode++].m_numbits = nodebits;
			}
		}
	}

	// make sure we ended up with the right number
	if (curnode != m_numcodes)
		return HUFFERR_INVALID_DATA;

	// assign canonical codes for all nodes based on their code lengths
	huffman_error error = assign_canonical_codes();
	if (error != HUFFERR_NONE)
		return error;

	// build the lookup table
	build_lookup_table();

	// determine final input length and report errors
	return bitbuf.overflow() ? HUFFERR_INPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


//-------------------------------------------------
//  export_tree_rle - export a huffman tree to an
//  RLE target data stream
//-------------------------------------------------

huffman_error huffman_context_base::export_tree_rle(bitstream_out &bitbuf)
{
	// bits per entry depends on the maxbits
	int numbits;
	if (m_maxbits >= 16)
		numbits = 5;
	else if (m_maxbits >= 8)
		numbits = 4;
	else
		numbits = 3;

	// RLE encode the lengths
	int lastval = ~0;
	int repcount = 0;
	for (int curcode = 0; curcode < m_numcodes; curcode++)
	{
		// if we match the previous value, just bump the repcount
		int newval = m_huffnode[curcode].m_numbits;
		if (newval == lastval)
			repcount++;

		// otherwise, we need to flush the previous repeats
		else
		{
			if (repcount != 0)
				write_rle_tree_bits(bitbuf, lastval, repcount, numbits);
			lastval = newval;
			repcount = 1;
		}
	}

	// flush the last value
	write_rle_tree_bits(bitbuf, lastval, repcount, numbits);
	return bitbuf.overflow() ? HUFFERR_OUTPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


//-------------------------------------------------
//  import_tree_huffman - import a huffman-encoded
//  huffman tree from a source data stream
//-------------------------------------------------

huffman_error huffman_context_base::import_tree_huffman(bitstream_in &bitbuf)
{
	// start by parsing the lengths for the small tree
	huffman_decoder<24, 6> smallhuff;
	smallhuff.m_huffnode[0].m_numbits = bitbuf.read(3);
	int start = bitbuf.read(3) + 1;
	int count = 0;
	for (int index = 1; index < 24; index++)
	{
		if (index < start || count == 7)
			smallhuff.m_huffnode[index].m_numbits = 0;
		else
		{
			count = bitbuf.read(3);
			smallhuff.m_huffnode[index].m_numbits = (count == 7) ? 0 : count;
		}
	}

	// then regenerate the tree
	huffman_error error = smallhuff.assign_canonical_codes();
	if (error != HUFFERR_NONE)
		return error;
	smallhuff.build_lookup_table();

	// determine the maximum length of an RLE count
	UINT32 temp = m_numcodes - 9;
	UINT8 rlefullbits = 0;
	while (temp != 0)
		temp >>= 1, rlefullbits++;

	// now process the rest of the data
	int last = 0;
	int curcode;
	for (curcode = 0; curcode < m_numcodes; )
	{
		int value = smallhuff.decode_one(bitbuf);
		if (value != 0)
			m_huffnode[curcode++].m_numbits = last = value - 1;
		else
		{
			int count = bitbuf.read(3) + 2;
			if (count == 7+2)
				count += bitbuf.read(rlefullbits);
			for ( ; count != 0 && curcode < m_numcodes; count--)
				m_huffnode[curcode++].m_numbits = last;
		}
	}

	// make sure we ended up with the right number
	if (curcode != m_numcodes)
		return HUFFERR_INVALID_DATA;

	// assign canonical codes for all nodes based on their code lengths
	error = assign_canonical_codes();
	if (error != HUFFERR_NONE)
		return error;

	// build the lookup table
	build_lookup_table();

	// determine final input length and report errors
	return bitbuf.overflow() ? HUFFERR_INPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


//-------------------------------------------------
//  export_tree_huffman - export a huffman tree to
//  a huffman target data stream
//-------------------------------------------------

huffman_error huffman_context_base::export_tree_huffman(bitstream_out &bitbuf)
{
	// first RLE compress the lengths of all the nodes
	dynamic_buffer rle_data(m_numcodes);
	UINT8 *dest = &rle_data[0];
	std::vector<UINT16> rle_lengths(m_numcodes/3);
	UINT16 *lengths = &rle_lengths[0];
	int last = ~0;
	int repcount = 0;

	// use a small huffman context to create a tree (ignoring RLE lengths)
	huffman_encoder<24, 6> smallhuff;

	// RLE-compress the lengths
	for (int curcode = 0; curcode < m_numcodes; curcode++)
	{
		// if this is the end of a repeat, flush any accumulation
		int newval = m_huffnode[curcode].m_numbits;
		if (newval != last && repcount > 0)
		{
			if (repcount == 1)
				smallhuff.histo_one(*dest++ = last + 1);
			else
				smallhuff.histo_one(*dest++ = 0), *lengths++ = repcount - 2;
		}

		// if same as last, just track repeats
		if (newval == last)
			repcount++;

		// otherwise, write it and start a new run
		else
		{
			smallhuff.histo_one(*dest++ = newval + 1);
			last = newval;
			repcount = 0;
		}
	}

	// flush any final RLE counts
	if (repcount > 0)
	{
		if (repcount == 1)
			smallhuff.histo_one(*dest++ = last + 1);
		else
			smallhuff.histo_one(*dest++ = 0), *lengths++ = repcount - 2;
	}

	// compute an optimal tree
	smallhuff.compute_tree_from_histo();

	// determine the first and last non-zero nodes
	int first_non_zero = 31, last_non_zero = 0;
	for (int index = 1; index < smallhuff.m_numcodes; index++)
		if (smallhuff.m_huffnode[index].m_numbits != 0)
		{
			if (first_non_zero == 31)
				first_non_zero = index;
			last_non_zero = index;
		}

	// clamp first non-zero to be 8 at a maximum
	first_non_zero = MIN(first_non_zero, 8);

	// output the lengths of the each small tree node, starting with the RLE
	// token (0), followed by the first_non_zero value, followed by the data
	// terminated by a 7
	bitbuf.write(smallhuff.m_huffnode[0].m_numbits, 3);
	bitbuf.write(first_non_zero - 1, 3);
	for (int index = first_non_zero; index <= last_non_zero; index++)
		bitbuf.write(smallhuff.m_huffnode[index].m_numbits, 3);
	bitbuf.write(7, 3);

	// determine the maximum length of an RLE count
	UINT32 temp = m_numcodes - 9;
	UINT8 rlefullbits = 0;
	while (temp != 0)
		temp >>= 1, rlefullbits++;

	// now encode the RLE data
	lengths = &rle_lengths[0];
	for (UINT8 *src = &rle_data[0]; src < dest; src++)
	{
		// encode the data
		UINT8 data = *src;
		smallhuff.encode_one(bitbuf, data);

		// if this is an RLE token, encode the length following
		if (data == 0)
		{
			int count = *lengths++;
			if (count < 7)
				bitbuf.write(count, 3);
			else
				bitbuf.write(7, 3), bitbuf.write(count - 7, rlefullbits);
		}
	}

	// flush the final buffer
	return bitbuf.overflow() ? HUFFERR_OUTPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}


//-------------------------------------------------
//  compute_tree_from_histo - common backend for
//  computing a tree based on the data histogram
//-------------------------------------------------

huffman_error huffman_context_base::compute_tree_from_histo()
{
	// compute the number of data items in the histogram
	UINT32 sdatacount = 0;
	for (int i = 0; i < m_numcodes; i++)
		sdatacount += m_datahisto[i];

	// binary search to achieve the optimum encoding
	UINT32 lowerweight = 0;
	UINT32 upperweight = sdatacount * 2;
	while (1)
	{
		// build a tree using the current weight
		UINT32 curweight = (upperweight + lowerweight) / 2;
		int curmaxbits = build_tree(sdatacount, curweight);

		// apply binary search here
		if (curmaxbits <= m_maxbits)
		{
			lowerweight = curweight;

			// early out if it worked with the raw weights, or if we're done searching
			if (curweight == sdatacount || (upperweight - lowerweight) <= 1)
				break;
		}
		else
			upperweight = curweight;
	}

	// assign canonical codes for all nodes based on their code lengths
	return assign_canonical_codes();
}



//**************************************************************************
//  INTERNAL FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  write_rle_tree_bits - write an RLE encoded
//  set of data to a target stream
//-------------------------------------------------

void huffman_context_base::write_rle_tree_bits(bitstream_out &bitbuf, int value, int repcount, int numbits)
{
	// loop until we have output all of the repeats
	while (repcount > 0)
	{
		// if we have a 1, write it twice as it is an escape code
		if (value == 1)
		{
			bitbuf.write(1, numbits);
			bitbuf.write(1, numbits);
			repcount--;
		}

		// if we have two or fewer in a row, write them raw
		else if (repcount <= 2)
		{
			bitbuf.write(value, numbits);
			repcount--;
		}

		// otherwise, write a triple using 1 as the escape code
		else
		{
			int cur_reps = MIN(repcount - 3, (1 << numbits) - 1);
			bitbuf.write(1, numbits);
			bitbuf.write(value, numbits);
			bitbuf.write(cur_reps, numbits);
			repcount -= cur_reps + 3;
		}
	}
}


//-------------------------------------------------
//  tree_node_compare - compare two tree nodes
//  by weight
//-------------------------------------------------

int CLIB_DECL huffman_context_base::tree_node_compare(const void *item1, const void *item2)
{
	const node_t *node1 = *(const node_t **)item1;
	const node_t *node2 = *(const node_t **)item2;
	if (node2->m_weight != node1->m_weight)
		return node2->m_weight - node1->m_weight;
	if (node2->m_bits - node1->m_bits == 0)
		fprintf(stderr, "identical node sort keys, should not happen!\n");
	return (int)node1->m_bits - (int)node2->m_bits;
}


//-------------------------------------------------
//  build_tree - build a huffman tree based on the
//  data distribution
//-------------------------------------------------

int huffman_context_base::build_tree(UINT32 totaldata, UINT32 totalweight)
{
	// make a list of all non-zero nodes
	std::vector<node_t *> list(m_numcodes * 2);
	int listitems = 0;
	memset(m_huffnode, 0, m_numcodes * sizeof(m_huffnode[0]));
	for (int curcode = 0; curcode < m_numcodes; curcode++)
		if (m_datahisto[curcode] != 0)
		{
			list[listitems++] = &m_huffnode[curcode];
			m_huffnode[curcode].m_count = m_datahisto[curcode];
			m_huffnode[curcode].m_bits = curcode;

			// scale the weight by the current effective length, ensuring we don't go to 0
			m_huffnode[curcode].m_weight = UINT64(m_datahisto[curcode]) * UINT64(totalweight) / UINT64(totaldata);
			if (m_huffnode[curcode].m_weight == 0)
				m_huffnode[curcode].m_weight = 1;
		}
/*
        fprintf(stderr, "Pre-sort:\n");
        for (int i = 0; i < listitems; i++) {
            fprintf(stderr, "weight: %d code: %d\n", list[i]->m_weight, list[i]->m_bits);
        }
*/
	// sort the list by weight, largest weight first
	qsort(&list[0], listitems, sizeof(list[0]), tree_node_compare);
/*
        fprintf(stderr, "Post-sort:\n");
        for (int i = 0; i < listitems; i++) {
            fprintf(stderr, "weight: %d code: %d\n", list[i]->m_weight, list[i]->m_bits);
        }
        fprintf(stderr, "===================\n");
*/
	// now build the tree
	int nextalloc = m_numcodes;
	while (listitems > 1)
	{
		// remove lowest two items
		node_t &node1 = *list[--listitems];
		node_t &node0 = *list[--listitems];

		// create new node
		node_t &newnode = m_huffnode[nextalloc++];
		newnode.m_parent = nullptr;
		node0.m_parent = node1.m_parent = &newnode;
		newnode.m_weight = node0.m_weight + node1.m_weight;

		// insert into list at appropriate location
		int curitem;
		for (curitem = 0; curitem < listitems; curitem++)
			if (newnode.m_weight > list[curitem]->m_weight)
			{
				memmove(&list[curitem+1], &list[curitem], (listitems - curitem) * sizeof(list[0]));
				break;
			}
		list[curitem] = &newnode;
		listitems++;
	}

	// compute the number of bits in each code, and fill in another histogram
	int maxbits = 0;
	for (int curcode = 0; curcode < m_numcodes; curcode++)
	{
		node_t &node = m_huffnode[curcode];
		node.m_numbits = 0;
		node.m_bits = 0;

		// if we have a non-zero weight, compute the number of bits
		if (node.m_weight > 0)
		{
			// determine the number of bits for this node
			for (node_t *curnode = &node; curnode->m_parent != nullptr; curnode = curnode->m_parent)
				node.m_numbits++;
			if (node.m_numbits == 0)
				node.m_numbits = 1;

			// keep track of the max
			maxbits = MAX(maxbits, node.m_numbits);
		}
	}
	return maxbits;
}


//-------------------------------------------------
//  assign_canonical_codes - assign canonical codes
//  to all the nodes based on the number of bits
//  in each
//-------------------------------------------------

huffman_error huffman_context_base::assign_canonical_codes()
{
	// build up a histogram of bit lengths
	UINT32 bithisto[33] = { 0 };
	for (int curcode = 0; curcode < m_numcodes; curcode++)
	{
		node_t &node = m_huffnode[curcode];
		if (node.m_numbits > m_maxbits)
			return HUFFERR_INTERNAL_INCONSISTENCY;
		if (node.m_numbits <= 32)
			bithisto[node.m_numbits]++;
	}

	// for each code length, determine the starting code number
	UINT32 curstart = 0;
	for (int codelen = 32; codelen > 0; codelen--)
	{
		UINT32 nextstart = (curstart + bithisto[codelen]) >> 1;
		if (codelen != 1 && nextstart * 2 != (curstart + bithisto[codelen]))
			return HUFFERR_INTERNAL_INCONSISTENCY;
		bithisto[codelen] = curstart;
		curstart = nextstart;
	}

	// now assign canonical codes
	for (int curcode = 0; curcode < m_numcodes; curcode++)
	{
		node_t &node = m_huffnode[curcode];
		if (node.m_numbits > 0)
			node.m_bits = bithisto[node.m_numbits]++;
	}
	return HUFFERR_NONE;
}


//-------------------------------------------------
//  build_lookup_table - build a lookup table for
//  fast decoding
//-------------------------------------------------

void huffman_context_base::build_lookup_table()
{
	// iterate over all codes
	for (int curcode = 0; curcode < m_numcodes; curcode++)
	{
		// process all nodes which have non-zero bits
		node_t &node = m_huffnode[curcode];
		if (node.m_numbits > 0)
		{
			// set up the entry
			lookup_value value = MAKE_LOOKUP(curcode, node.m_numbits);

			// fill all matching entries
			int shift = m_maxbits - node.m_numbits;
			lookup_value *dest = &m_lookup[node.m_bits << shift];
			lookup_value *destend = &m_lookup[((node.m_bits + 1) << shift) - 1];
			while (dest <= destend)
				*dest++ = value;
		}
	}
}



//**************************************************************************
//  8-BIT ENCODER
//**************************************************************************

//-------------------------------------------------
//  huffman_8bit_encoder - constructor
//-------------------------------------------------

huffman_8bit_encoder::huffman_8bit_encoder()
{
}


//-------------------------------------------------
//  encode - encode a full buffer
//-------------------------------------------------

huffman_error huffman_8bit_encoder::encode(const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dlength, UINT32 &complength)
{
	// first compute the histogram
	histo_reset();
	for (UINT32 cur = 0; cur < slength; cur++)
		histo_one(source[cur]);

	// then compute the tree
	huffman_error err = compute_tree_from_histo();
	if (err != HUFFERR_NONE)
		return err;

	// export the tree
	bitstream_out bitbuf(dest, dlength);
	err = export_tree_huffman(bitbuf);
	if (err != HUFFERR_NONE)
		return err;

	// then encode the data
	for (UINT32 cur = 0; cur < slength; cur++)
		encode_one(bitbuf, source[cur]);
	complength = bitbuf.flush();
	return bitbuf.overflow() ? HUFFERR_OUTPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}



//**************************************************************************
//  8-BIT DECODER
//**************************************************************************

//-------------------------------------------------
//  huffman_8bit_decoder - constructor
//-------------------------------------------------

huffman_8bit_decoder::huffman_8bit_decoder()
{
}

/**
 * @fn  huffman_error huffman_8bit_decoder::decode(const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dlength)
 *
 * @brief   -------------------------------------------------
 *            decode - decode a full buffer
 *          -------------------------------------------------.
 *
 * @param   source          Source for the.
 * @param   slength         The slength.
 * @param [in,out]  dest    If non-null, destination for the.
 * @param   dlength         The dlength.
 *
 * @return  A huffman_error.
 */

huffman_error huffman_8bit_decoder::decode(const UINT8 *source, UINT32 slength, UINT8 *dest, UINT32 dlength)
{
	// first import the tree
	bitstream_in bitbuf(source, slength);
	huffman_error err = import_tree_huffman(bitbuf);
	if (err != HUFFERR_NONE)
		return err;

	// then decode the data
	for (UINT32 cur = 0; cur < dlength; cur++)
		dest[cur] = decode_one(bitbuf);
	bitbuf.flush();
	return bitbuf.overflow() ? HUFFERR_INPUT_BUFFER_TOO_SMALL : HUFFERR_NONE;
}
