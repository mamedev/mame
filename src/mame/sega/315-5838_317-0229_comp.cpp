// license:BSD-3-Clause
// copyright-holders:David Haywood, Samuel Neves, Peter Wilhelmsen, Morten Shearman Kirkegaard
/***************************************************************************

    Sega 315-5838 / 317-0229 / 317-0230 / 317-0231
    Fixed word substitution followed by programmable prefix decompression.

    315-5838: DecAthlete (graphics)
    317-0229: Dead or Alive (startup string), Name Club 1/2 (printer data)
    317-0230: Print Club Pooh 2/3, Nightmare Before Christmas
    317-0231: Print Club Love Love 1/2, Sony Creative 5/6
    The protected Print Club startup blocks are graphics descriptors, not
    executable code. These associations identify functional transforms;
    chip markings have not been independently confirmed for every board.

    Word transform
    --------------
    Each encrypted 16-bit word is transformed independently. There is no
    evolving key, feedback or address dependence in the word function.
    Number input bits x0..x15, with x0 least significant. Form four 3-bit
    values (the first element below is the least significant bit):

        A = [x5 ^ x12, x7 ^ x9, x14]
        B = [x6,       x8,      x15]
        C = [x1 ^ x8,  x7,      x12]
        D = [x3,       x4,      x10]

    Route these through four bijective, eight-entry substitution tables.
    Table output bits 0..2 go to [0,1,7], [2,5,9], [4,10,13] and [6,11,15]
    of the resulting word. Routing and table values depend on the variant:

        315-5838: A B C D        317-0229: D C A B
        317-0230: C D B A        317-0231: B A D C

    The four remaining output bits [3,8,12,14] are affine functions of
    [x0, x0^x2, x11^x14, x4^x13, 1]. Each parameter mask selects terms to
    XOR. CIPHERS below contains the routing, substitutions and affine masks
    for all variants, including a uniform representation of DecAthlete.

    Compression and interface
    -------------------------
    The transformed words contain compressed bits, consumed MSB first.
    A mode write with bit 7 clear starts a 24-halfword tree upload. Pairs
    describe (code length in the high byte, dictionary index in the low
    byte), followed by a 12-bit-aligned starting code. Mode bit 7 set starts
    a 256-byte dictionary upload, high byte first in each written halfword.
    Mode bit 15 does not change these observed operations.

    For a code length n, shift the starting pattern right by 12-n. The next
    distinct pattern is the exclusive upper bound; the final group extends
    to 2^n. The dictionary index is the group's base index plus the code's
    offset from its start. Repeated final descriptors must not terminate
    the range: several variants finish before length 12. The dictionary is
    a byte alphabet, not an LZ history or a cryptographic key.

    Source writes set the word address and discard buffered compressed
    bits. Output reads return two decoded bytes, high byte first. The host
    supplies encrypted words through a callback, so cartridge banking and
    RAM byte order stay in the drivers. Model 2's discarded initial read is
    also a bus-level detail, not an extra character in the compressed text.

    Recovery method
    ---------------
    The useful known plaintext was the Print Club graphics metadata. Its
    unprotected maps reference 16x16, 8-bit tiles in increments of eight.
    Thus (maximum tile reference / 8 + 1) * 256 gives a graphics block's
    size. Contiguous blocks ending at the maps determine their addresses.
    This reconstructs protected (address, length-in-longwords) descriptors.

    Re-encoding these bytes with the uploaded tables gives compressed
    plaintext bits aligned with encrypted ROM words. It is these word
    pairs, not final graphics bytes, that constrain the substitution.
    Pooh 2/3 supplied 42 complete pairs plus partial tails; Love Love supplied
    56 complete pairs. For each output bit, enumerate supports of at most
    five input bits and fit constant, linear and quadratic terms over GF(2).
    Retain all nullspace solutions, then check the resulting candidates for
    bijectivity, unused bits and samples withheld from fitting. Pooh left
    two candidate circuits; only one was bijective. Love Love's fit was
    unique in this function class. Both admit the four-lane structure above.

    DOA compares 48 characters, encoding only 187 compressed bits. Its first
    discarded halfword is not assumed to contain two spaces. The shared
    lane structure restricts these bits to twelve complete candidates.
    Only one passes Name Club 2's independent RCDD2 test: 563,044 decoded
    bytes repeated 33 times, expected result 0x12477c39. That routine has
    only about eleven effective checksum bits, so it discriminates these
    candidates rather than proving uniqueness over arbitrary algorithms.
    The additional Nightmare and Sony Creative sets validate descriptor
    contents not used in recovering the transforms.

    As a control, recompressing 128 DecAthlete bytes observed from working
    MAME gives 47 complete pairs. The same quadratic fit recovers its whole
    transform, agreeing with the original implementation on all 65,536
    inputs. This uses an emulator plaintext oracle, not new hardware data.
    All four functions are bijective; their table-driven representation was
    exhaustively compared with the recovered Boolean functions. Finite
    samples establish uniqueness only within the tested function classes,
    and do not establish physical gate layout or exact device timing.

***************************************************************************/

#include "emu.h"
#include "315-5838_317-0229_comp.h"

DEFINE_DEVICE_TYPE(SEGA315_5838_COMP, sega_315_5838_comp_device, "sega315_5838", "Sega 315-5838 / 317-0229 Compression and Encryption")

sega_315_5838_comp_device::sega_315_5838_comp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SEGA315_5838_COMP, tag, owner, clock),
	m_source_cb(*this, 0xffff)
{
}

void sega_315_5838_comp_device::device_start()
{
	save_item(NAME(m_tree));
	save_item(NAME(m_dictionary));
	save_item(NAME(m_tree_words));
	save_item(NAME(m_dictionary_bytes));
	save_item(NAME(m_upload_dictionary));
	save_item(NAME(m_source));
	save_item(NAME(m_word));
	save_item(NAME(m_bits));
	save_item(NAME(m_output));
	save_item(NAME(m_abort));
}

void sega_315_5838_comp_device::device_reset()
{
	std::fill(std::begin(m_tree), std::end(m_tree), 0);
	std::fill(std::begin(m_dictionary), std::end(m_dictionary), 0);
	m_tree_words = 0;
	m_dictionary_bytes = 0;
	m_upload_dictionary = false;
	m_source = 0;
	m_word = 0;
	m_bits = 0;
	m_output = 0xffff;
	m_abort = false;
}

u16 sega_315_5838_comp_device::decipher(u16 ciphertext) const
{
	static constexpr cipher_parameters CIPHERS[] =
	{
		{ // 315-5838
			{ 0, 1, 2, 3 },
			{ { 6, 7, 2, 0, 5, 1, 3, 4 }, { 5, 3, 7, 2, 1, 6, 4, 0 },
			  { 2, 0, 6, 1, 3, 7, 4, 5 }, { 6, 5, 7, 4, 1, 3, 0, 2 } },
			{ 0b10100, 0b11000, 0b00010, 0b10101 }
		},
		{ // 317-0229
			{ 3, 2, 0, 1 },
			{ { 6, 0, 5, 4, 7, 2, 1, 3 }, { 5, 6, 1, 3, 7, 0, 4, 2 },
			  { 2, 6, 0, 1, 3, 4, 7, 5 }, { 6, 5, 7, 4, 1, 3, 0, 2 } },
			{ 0b10010, 0b10001, 0b00100, 0b11010 }
		},
		{ // 317-0230
			{ 2, 3, 1, 0 },
			{ { 6, 2, 7, 0, 5, 3, 1, 4 }, { 5, 3, 7, 2, 1, 6, 4, 0 },
			  { 2, 1, 3, 5, 6, 0, 4, 7 }, { 6, 1, 3, 5, 7, 0, 2, 4 } },
			{ 0b10100, 0b11000, 0b00010, 0b10101 }
		},
		{ // 317-0231
			{ 1, 0, 3, 2 },
			{ { 6, 0, 5, 4, 7, 2, 1, 3 }, { 5, 1, 6, 3, 7, 4, 0, 2 },
			  { 2, 1, 3, 5, 6, 0, 4, 7 }, { 6, 3, 1, 5, 7, 2, 0, 4 } },
			{ 0b10010, 0b10001, 0b00100, 0b11010 }
		}
	};
	static constexpr u8 OUTPUT_BITS[4][3] = { { 0, 1, 7 }, { 2, 5, 9 }, { 4, 10, 13 }, { 6, 11, 15 } };
	static constexpr u8 AFFINE_BITS[4] = { 3, 8, 12, 14 };

	auto const x = [ciphertext](unsigned bit) { return BIT(ciphertext, bit); };
	u8 const lanes[4] =
	{
		u8((x(5) ^ x(12)) | ((x(7) ^ x(9)) << 1) | (x(14) << 2)),
		u8(x(6) | (x(8) << 1) | (x(15) << 2)),
		u8((x(1) ^ x(8)) | (x(7) << 1) | (x(12) << 2)),
		u8(x(3) | (x(4) << 1) | (x(10) << 2))
	};
	u8 const affine = x(0) | ((x(0) ^ x(2)) << 1) | ((x(11) ^ x(14)) << 2) | ((x(4) ^ x(13)) << 3) | 16;
	cipher_parameters const &cipher = CIPHERS[unsigned(m_variant)];
	u16 result = 0;
	for (unsigned group = 0; group < 4; ++group)
	{
		u8 const value = cipher.sboxes[group][lanes[cipher.routing[group]]];
		for (unsigned bit = 0; bit < 3; ++bit)
			result |= BIT(value, bit) << OUTPUT_BITS[group][bit];

		u8 parity = affine & cipher.affine[group];
		parity ^= parity >> 4;
		parity ^= parity >> 2;
		parity ^= parity >> 1;
		result |= BIT(parity, 0) << AFFINE_BITS[group];
	}
	return result;
}

void sega_315_5838_comp_device::source_w(u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_source);
	m_source &= 0x007f'ffff;
	m_word = 0;
	m_bits = 0;
	m_abort = false;
}

void sega_315_5838_comp_device::table_w(offs_t offset, u16 data)
{
	if (!BIT(offset, 0))
	{
		m_upload_dictionary = BIT(data, 7);
		if (m_upload_dictionary)
			m_dictionary_bytes = 0;
		else
			m_tree_words = 0;
	}
	else if (m_upload_dictionary)
	{
		if (m_dictionary_bytes < std::size(m_dictionary))
		{
			m_dictionary[m_dictionary_bytes++] = data >> 8;
			m_dictionary[m_dictionary_bytes++] = data & 0xff;
		}
	}
	else if (m_tree_words < std::size(m_tree))
	{
		m_tree[m_tree_words++] = data;
	}
}

u8 sega_315_5838_comp_device::decompress_byte()
{
	if (m_abort || m_tree_words != std::size(m_tree) || m_dictionary_bytes != std::size(m_dictionary))
		return 0xff;

	u16 code = 0;
	for (unsigned length = 1; length <= 12; ++length)
	{
		if (!m_bits)
		{
			m_word = decipher(m_source_cb(m_source));
			m_source = (m_source + 1) & 0x007f'ffff;
			m_bits = 16;
		}
		code = (code << 1) | BIT(m_word, --m_bits);

		for (unsigned entry = 0; entry < std::size(m_tree); entry += 2)
		{
			if (BIT(m_tree[entry], 8, 8) != length)
				continue;

			u16 const first = m_tree[entry + 1] >> (12 - length);
			u16 limit = 1U << length;
			for (unsigned next = entry + 2; next < std::size(m_tree); next += 2)
			{
				if (m_tree[next + 1] != m_tree[entry + 1])
				{
					limit = m_tree[next + 1] >> (12 - length);
					break;
				}
			}

			if (code >= first && code < limit)
			{
				unsigned const index = BIT(m_tree[entry], 0, 8) + code - first;
				if (index < std::size(m_dictionary))
					return m_dictionary[index];
			}
		}
	}

	// Invalid tables or data must not loop forever or read past the dictionary.
	logerror("Invalid compressed code at source word %06x\n", m_source);
	m_abort = true;
	return 0xff;
}

u16 sega_315_5838_comp_device::data_r()
{
	if (!machine().side_effects_disabled())
	{
		m_output = u16(decompress_byte()) << 8;
		m_output |= decompress_byte();
	}
	return m_output;
}
