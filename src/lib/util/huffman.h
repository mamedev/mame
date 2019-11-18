// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    huffman.h

    Static Huffman compression and decompression helpers.

***************************************************************************/

#ifndef MAME_UTIL_HUFFMAN_H
#define MAME_UTIL_HUFFMAN_H

#pragma once

#include "osdcore.h"
#include "bitstream.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum huffman_error
{
	HUFFERR_NONE = 0,
	HUFFERR_TOO_MANY_BITS,
	HUFFERR_INVALID_DATA,
	HUFFERR_INPUT_BUFFER_TOO_SMALL,
	HUFFERR_OUTPUT_BUFFER_TOO_SMALL,
	HUFFERR_INTERNAL_INCONSISTENCY,
	HUFFERR_TOO_MANY_CONTEXTS
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> huffman_context_base

// base class for encoding and decoding
class huffman_context_base
{
protected:
	typedef uint16_t lookup_value;

	// a node in the huffman tree
	struct node_t
	{
		node_t *            m_parent;               // pointer to parent node
		uint32_t              m_count;                // number of hits on this node
		uint32_t              m_weight;               // assigned weight of this node
		uint32_t              m_bits;                 // bits used to encode the node
		uint8_t               m_numbits;              // number of bits needed for this node
	};

	// construction/destruction
	huffman_context_base(int numcodes, int maxbits, lookup_value *lookup, uint32_t *histo, node_t *nodes);

	// tree creation
	huffman_error compute_tree_from_histo();

	// static tree import; huffman is notably more efficient
	huffman_error import_tree_rle(bitstream_in &bitbuf);
	huffman_error import_tree_huffman(bitstream_in &bitbuf);

	// static tree export
	huffman_error export_tree_rle(bitstream_out &bitbuf);
	huffman_error export_tree_huffman(bitstream_out &bitbuf);

	// internal helpers
	void write_rle_tree_bits(bitstream_out &bitbuf, int value, int repcount, int numbits);
	static int CLIB_DECL tree_node_compare(const void *item1, const void *item2);
	int build_tree(uint32_t totaldata, uint32_t totalweight);
	huffman_error assign_canonical_codes();
	void build_lookup_table();

protected:
	// internal state
	uint32_t                  m_numcodes;             // number of total codes being processed
	uint8_t                   m_maxbits;              // maximum bits per code
	uint8_t                   m_prevdata;             // value of the previous data (for delta-RLE encoding)
	int                     m_rleremaining;         // number of RLE bytes remaining (for delta-RLE encoding)
	lookup_value *          m_lookup;               // pointer to the lookup table
	uint32_t *                m_datahisto;            // histogram of data values
	node_t *                m_huffnode;             // array of nodes
};


// ======================> huffman_encoder

// template class for encoding
template<int _NumCodes = 256, int _MaxBits = 16>
class huffman_encoder : public huffman_context_base
{
public:
	// pass through to the underlying constructor
	huffman_encoder()
		: huffman_context_base(_NumCodes, _MaxBits, nullptr, m_datahisto_array, m_huffnode_array) { histo_reset(); }

	// single item operations
	void histo_reset() { memset(m_datahisto_array, 0, sizeof(m_datahisto_array)); }
	void histo_one(uint32_t data);
	void encode_one(bitstream_out &bitbuf, uint32_t data);

	// expose tree computation and export
	using huffman_context_base::compute_tree_from_histo;
	using huffman_context_base::export_tree_rle;
	using huffman_context_base::export_tree_huffman;

private:
	// array versions of the info we need
	uint32_t                  m_datahisto_array[_NumCodes];
	node_t                  m_huffnode_array[_NumCodes * 2];
};


// ======================> huffman_decoder

// template class for decoding
template<int _NumCodes = 256, int _MaxBits = 16>
class huffman_decoder : public huffman_context_base
{
public:
	// pass through to the underlying constructor
	huffman_decoder()
		: huffman_context_base(_NumCodes, _MaxBits, m_lookup_array, nullptr, m_huffnode_array) { }

	// single item operations
	uint32_t decode_one(bitstream_in &bitbuf);

	// expose tree import
	using huffman_context_base::import_tree_rle;
	using huffman_context_base::import_tree_huffman;

private:
	// array versions of the info we need
	node_t                  m_huffnode_array[_NumCodes];
	lookup_value            m_lookup_array[1 << _MaxBits];
};


// ======================> huffman_8bit_encoder

// generic 8-bit encoder/decoder
class huffman_8bit_encoder : public huffman_encoder<>
{
public:
	// construction/destruction
	huffman_8bit_encoder();

	// operations
	huffman_error encode(const uint8_t *source, uint32_t slength, uint8_t *dest, uint32_t destlength, uint32_t &complength);
};


// ======================> huffman_8bit_decoder

// generic 8-bit encoder/decoder
class huffman_8bit_decoder : public huffman_decoder<>
{
public:
	// construction/destruction
	huffman_8bit_decoder();

	// operations
	huffman_error decode(const uint8_t *source, uint32_t slength, uint8_t *dest, uint32_t destlength);
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  histo_one - update the histogram
//-------------------------------------------------

template<int _NumCodes, int _MaxBits>
inline void huffman_encoder<_NumCodes, _MaxBits>::histo_one(uint32_t data)
{
	m_datahisto[data]++;
}


//-------------------------------------------------
//  encode_one - encode a single code to the
//  huffman stream
//-------------------------------------------------

template<int _NumCodes, int _MaxBits>
inline void huffman_encoder<_NumCodes, _MaxBits>::encode_one(bitstream_out &bitbuf, uint32_t data)
{
	// write the data
	node_t &node = m_huffnode[data];
	bitbuf.write(node.m_bits, node.m_numbits);
}


//-------------------------------------------------
//  decode_one - decode a single code from the
//  huffman stream
//-------------------------------------------------

template<int _NumCodes, int _MaxBits>
inline uint32_t huffman_decoder<_NumCodes, _MaxBits>::decode_one(bitstream_in &bitbuf)
{
	// peek ahead to get maxbits worth of data
	uint32_t bits = bitbuf.peek(m_maxbits);

	// look it up, then remove the actual number of bits for this code
	lookup_value lookup = m_lookup[bits];
	bitbuf.remove(lookup & 0x1f);

	// return the value
	return lookup >> 5;
}

#endif // MAME_UTIL_HUFFMAN_H
