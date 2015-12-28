// license:BSD-3-Clause
// copyright-holders:Andreas Naive, Olivier Galibert, David Haywood
/*
  re: Tecmo World Cup '98 (ST-V) (from ANY)

  I got one of the card in subject open it up to check the rom version
  and made a discovery...
  The protection chip has the part number on it "315-5881", it's the same
  used on naomi M2 carts as you can see here
  http://imagizer.imageshack.us/a/img540/7634/BsqvD8.jpg

  The same chip 315-5881 but with a Lattice IspLSI2032 (Sega part
  315-6050) was used on some Model3 games...

*/

#include "emu.h"
#include "machine/315-5881_crypt.h"

extern const device_type SEGA315_5881_CRYPT = &device_creator<sega_315_5881_crypt_device>;


sega_315_5881_crypt_device::sega_315_5881_crypt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEGA315_5881_CRYPT, "Sega 315-5881 Encryption", tag, owner, clock, "sega315_5881", __FILE__)
{
}



void sega_315_5881_crypt_device::device_start()
{
	buffer = std::make_unique<UINT8[]>(BUFFER_SIZE);
	line_buffer = std::make_unique<UINT8[]>(LINE_SIZE);
	line_buffer_prev = std::make_unique<UINT8[]>(LINE_SIZE);

	m_read.bind_relative_to(*owner());

	save_pointer(NAME(buffer.get()), BUFFER_SIZE);
	save_pointer(NAME(line_buffer.get()), LINE_SIZE);
	save_pointer(NAME(line_buffer_prev.get()), LINE_SIZE);
	save_item(NAME(prot_cur_address));
	save_item(NAME(subkey));
	save_item(NAME(enc_ready));
	save_item(NAME(dec_hist));
	save_item(NAME(dec_header));
	save_item(NAME(buffer_pos));
	save_item(NAME(line_buffer_pos));
	save_item(NAME(line_buffer_size));

	std::string skey = parameter("key");
	if(!skey.empty())
		key = strtoll(skey.c_str(), nullptr, 16);
	else
	{
		logerror("%s: Warning: key not provided\n", tag());
		key = 0;
	}
}

void sega_315_5881_crypt_device::device_reset()
{
	memset(buffer.get(), 0, BUFFER_SIZE);
	memset(line_buffer.get(), 0, LINE_SIZE);
	memset(line_buffer_prev.get(), 0, LINE_SIZE);

	prot_cur_address = 0;
	subkey = 0;
	dec_hist = 0;
	dec_header = 0;
	enc_ready = false;

	buffer_pos = 0;
	line_buffer_pos = 0;
	line_buffer_size = 0;
	buffer_bit = 0;
}

UINT16 sega_315_5881_crypt_device::do_decrypt(UINT8 *&base)
{
	if(!enc_ready)
		enc_start();
	if(dec_header & FLAG_COMPRESSED) {
		if (line_buffer_pos == line_buffer_size) // if there's no data left to read..
		{
			if (done_compression == 1)
				enc_start();



			line_fill();
		}
		base = line_buffer.get() + line_buffer_pos;
		line_buffer_pos += 2;
	} else {
		if(buffer_pos == BUFFER_SIZE)
			enc_fill();
		base = buffer.get() + buffer_pos;
		buffer_pos += 2;
	}

	return (base[0] << 8) | base[1];
}

void sega_315_5881_crypt_device::set_addr_low(UINT16 data)
{
	prot_cur_address = (prot_cur_address & 0xffff0000) | data;
	enc_ready = false;
}

void sega_315_5881_crypt_device::set_addr_high(UINT16 data)
{
	prot_cur_address = (prot_cur_address & 0x0000ffff) | (data << 16);
	enc_ready = false;

	buffer_bit = 7;
	buffer_bit2 = 15;
}

void sega_315_5881_crypt_device::set_subkey(UINT16 data)
{
	subkey = data;
	enc_ready = false;
}

/***************************************************************************
    DECRYPTION EMULATION

By convention, we label the three known cart protection methods this way (using Deunan Knute's wording):
M1: DMA read of protected ROM area
M2: special read of ROM area which supplies decryption key first
M3: normal read followed by write to cart's decryption buffer (up to 64kB), followed by M2 but from buffer area

Notes below refer to M2 & M3.

The encryption is done by a stream cipher operating in counter mode, which use a 16-bits internal block cipher.

Every stream can be composed by several substreams; there are 18 header bits at the start of every substream, with
a 1+9+8 format; the highest bit control the mode of operation: set to 1 means that the substream needs to be decompressed
after being decrypted. The other two blocks (A||B) encode the length of the substream, as (A+1)*(B+1). When a
substream end, the header of the next one, if existing, follows inmediatly.

The next 16-bits are part of the header (they don't belong to the plaintext), but his meaning is unclear. It has been
conjectured that it could stablish when to "reset" the process and start processing a new stream (based on some tests
on WWFROYAL, in which the decryption's output doesn't seem to be valid for more than some dozens of words), but some
more testing would be needed for clarifying that.

After those 18 heading bits, we find the proper plaintext. It must be noted that, due to the initial 2 special bits,
the 16-bits words of the plaintext are shifted 2 bits respect to the word-boundaries of the output stream of the
internal block-cipher. So, at a given step, the internal block cipher will output 16-bits, 14 of which will go to a
given plaintext word, and the remaining 2 to the next plaintext word.

The underlying block cipher consists of two 4-round Feistel Networks (FN): the first one takes the counter (16 bits),
the game-key (>=30 bits; probably 64) and the sequence-key (16 bits) and output a middle result (16 bits) which will act
as another key for the second one. The second FN will take the encrypted word (16 bits), the game-key, the sequence-key
and the result from the first FN and will output the decrypted word (16 bits).

Each round of the Feistel Networks use four substitution sboxes, each having 6 inputs and 2 outputs. The input is the
XOR of at most one bit from the previous round and at most one bit from the different keys.

The underlying block cipher has the same structure than the one used by the CPS-2 (Capcom Play System 2) and,
indeed, some of the used sboxes are exactly the same and appear in the same FN/round in both systems (this is not evident,
as you need to apply a bitswapping and some XORs to the input & output of the sboxes to get the same values due).

Note that this implementation considers that the counter initialization for ram decryption is 0 simply because the ram is
mapped to multiples of 128K.

Due to the nature of the cipher, there are some degrees of freedom when choosing the s-boxes and keys values; by example,
you could apply a fixed bitswapping and XOR to the keys and the decryption would remain the same as long as you change
accordingly the s-boxes' definitions. So the order of the bits in the keys is arbitrary, and the s-boxes values have been
chosen so as to make the key for CAPSNK equal to 0.

It can be observed that a couple of sboxes have incomplete tables (a 255 value indicate an unknown value). The recovered keys
as of january/2015 show small randomness and big correlations, making possible that some unseen bits could make the
decryption need those incomplete parts.

SEGA apparently used his security part label (317-xxxx-yyy) as part of the key; the mapping of the current keys to the chip label
is given by the following function:

void key2label(uint32_t key)
{
    int bcd0 = ((BIT(key,17)<<3)|(BIT(key,7)<<2)|(BIT(key,14)<<1)|BIT(key,19))^9;
    int bcd1 = ((BIT(key,20)<<3)|(BIT(key,1)<<2)|(BIT(key,4)<<1)|BIT(key,13))^5;
    int bcd2 = (BIT(key,9)<<1)|BIT(key,22);
    int bcd3 = ((BIT(key,9)<<2)|BIT(key,9))^5;

    char chiplabel[13];
    sprintf(chiplabel, "317-%d%d%d%d-%s", bcd3, bcd2, bcd1, bcd0, (BIT(key,5)?"JPN":"COM"));

    printf("%s", chiplabel);
}

Given the use of the BCD-encoded security module labels, it's expected that at least other 6 additional bits be present in the
real keys but undetected in the current implementation (due to them being set to fixed values on all the known 315-5881 chip labels).
That would rise the bit count at least to 35.

Other key bits not directly related to the 315-5881 label still show low entropies, making possible that
they be derived from other non-random sources.

In the second Feistel Network, every key bit seem to be used at most once (the various uses of current bit #9 are fictitious, as
that bit really represent various bits in the real key; see comments on the use of the labels above). Given that, it seems probable
that the real key is 64 bits long, exactly as in the related CPS-2 scheme, and the designers tried to cover all 96 input bits with
the bits provening from the game key, the sequence key and the result from the first feistel network (64+16+16=96). In the first
Feistel Network, as only 80 bits are available, some bits would be used twice (as can be partially seen in the current implementation).
The fact that only 30 bits out of the expected 64 have been observed till now would be due to the generation of the key by composing
low-entropy sources.

****************************************************************************************/

const sega_315_5881_crypt_device::sbox sega_315_5881_crypt_device::fn1_sboxes[4][4] = {
	{   // 1st round
		{
			{
				0,3,2,2,1,3,1,2,3,2,1,2,1,2,3,1,3,2,2,0,2,1,3,0,0,3,2,3,2,1,2,0,
				2,3,1,1,2,2,1,1,1,0,2,3,3,0,2,1,1,1,1,1,3,0,3,2,1,0,1,2,0,3,1,3,
			},
			{3,4,5,7,-1,-1},
			{0,4}
		},

		{
			{
				2,2,2,0,3,3,0,1,2,2,3,2,3,0,2,2,1,1,0,3,3,2,0,2,0,1,0,1,2,3,1,1,
				0,1,3,3,1,3,3,1,2,3,2,0,0,0,2,2,0,3,1,3,0,3,2,2,0,3,0,3,1,1,0,2,
			},
			{0,1,2,5,6,7},
			{1,6}
		},

		{
			{
				0,1,3,0,3,1,1,1,1,2,3,1,3,0,2,3,3,2,0,2,1,1,2,1,1,3,1,0,0,2,0,1,
				1,3,1,0,0,3,2,3,2,0,3,3,0,0,0,0,1,2,3,3,2,0,3,2,1,0,0,0,2,2,3,3,
			},
			{0,2,5,6,7,-1},
			{2,3}
		},

		{
			{
				3,2,1,2,1,2,3,2,0,3,2,2,3,1,3,3,0,2,3,0,3,3,2,1,1,1,2,0,2,2,0,1,
				1,3,3,0,0,3,0,3,0,2,1,3,2,1,0,0,0,1,1,2,0,1,0,0,0,1,3,3,2,0,3,3,
			},
			{1,2,3,4,6,7},
			{5,7}
		},
	},
	{   // 2nd round
		{
			{
				3,3,1,2,0,0,2,2,2,1,2,1,3,1,1,3,3,0,0,3,0,3,3,2,1,1,3,2,3,2,1,3,
				2,3,0,1,3,2,0,1,2,1,3,1,2,2,3,3,3,1,2,2,0,3,1,2,2,1,3,0,3,0,1,3,
			},
			{0,1,3,4,5,7},
			{0,4}
		},

		{
			{
				2,0,1,0,0,3,2,0,3,3,1,2,1,3,0,2,0,2,0,0,0,2,3,1,3,1,1,2,3,0,3,0,
				3,0,2,0,0,2,2,1,0,2,3,3,1,3,1,0,1,3,3,0,0,1,3,1,0,2,0,3,2,1,0,1,
			},
			{0,1,3,4,6,-1},
			{1,5}
		},

		{
			{
				2,2,2,3,1,1,0,1,3,3,1,1,2,2,2,0,0,3,2,3,3,0,2,1,2,2,3,0,1,3,0,0,
				3,2,0,3,2,0,1,0,0,1,2,2,3,3,0,2,2,1,3,1,1,1,1,2,0,3,1,0,0,2,3,2,
			},
			{1,2,5,6,7,6},
			{2,7}
		},

		{
			{
				0,1,3,3,3,1,3,3,1,0,2,0,2,0,0,3,1,2,1,3,1,2,3,2,2,0,1,3,0,3,3,3,
				0,0,0,2,1,1,2,3,2,2,3,1,1,2,0,2,0,2,1,3,1,1,3,3,1,1,3,0,2,3,0,0,
			},
			{2,3,4,5,6,7},
			{3,6}
		},
	},
	{   // 3rd round
		{
			{
				0,0,1,0,1,0,0,3,2,0,0,3,0,1,0,2,0,3,0,0,2,0,3,2,2,1,3,2,2,1,1,2,
				0,0,0,3,0,1,1,0,0,2,1,0,3,1,2,2,2,0,3,1,3,0,1,2,2,1,1,1,0,2,3,1,
			},
			{1,2,3,4,5,7},
			{0,5}
		},

		{
			{
				1,2,1,0,3,1,1,2,0,0,2,3,2,3,1,3,2,0,3,2,2,3,1,1,1,1,0,3,2,0,0,1,
				1,0,0,1,3,1,2,3,0,0,2,3,3,0,1,0,0,2,3,0,1,2,0,1,3,3,3,1,2,0,2,1,
			},
			{0,2,4,5,6,7},
			{1,6}
		},

		{
			{
				0,3,0,2,1,2,0,0,1,1,0,0,3,1,1,0,0,3,0,0,2,3,3,2,3,1,2,0,0,2,3,0,
				// unused?
				255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			},
			{0,2,4,6,7,-1},
			{2,3}
		},

		{
			{
				0,0,1,0,0,1,0,2,3,3,0,3,3,2,3,0,2,2,2,0,3,2,0,3,1,0,0,3,3,0,0,0,
				2,2,1,0,2,0,3,2,0,0,3,1,3,3,0,0,2,1,1,2,1,0,1,1,0,3,1,2,0,2,0,3,
			},
			{0,1,2,3,6,-1},
			{4,7}
		},
	},
	{   // 4th round
		{
			{
				0,3,3,3,3,3,2,0,0,1,2,0,2,2,2,2,1,1,0,2,2,1,3,2,3,2,0,1,2,3,2,1,
				3,2,2,3,1,0,1,0,0,2,0,1,2,1,2,3,1,2,1,1,2,2,1,0,1,3,2,3,2,0,3,1,
			},
			{0,1,3,4,5,6},
			{0,5}
		},

		{
			{
				0,3,0,0,2,0,3,1,1,1,2,2,2,1,3,1,2,2,1,3,2,2,3,3,0,3,1,0,3,2,0,1,
				3,0,2,0,1,0,2,1,3,3,1,2,2,0,2,3,3,2,3,0,1,1,3,3,0,2,1,3,0,2,2,3,
			},
			{0,1,2,3,5,7},
			{1,7}
		},

		{
			{
				0,1,2,3,3,3,3,1,2,0,2,3,2,1,0,1,2,2,1,2,0,3,2,0,1,1,0,1,3,1,3,1,
				3,1,0,0,1,0,0,0,0,1,2,2,1,1,3,3,1,2,3,3,3,2,3,0,2,2,1,3,3,0,2,0,
			},
			{2,3,4,5,6,7},
			{2,3}
		},

		{
			{
				0,2,1,1,3,2,0,3,1,0,1,0,3,2,1,1,2,2,0,3,1,0,1,2,2,2,3,3,0,0,0,0,
				1,2,1,0,2,1,2,2,2,3,2,3,0,1,3,0,0,1,3,0,0,1,1,0,1,0,0,0,0,2,0,1,
			},
			{0,1,2,4,6,7},
			{4,6}
		},
	},
};


const sega_315_5881_crypt_device::sbox sega_315_5881_crypt_device::fn2_sboxes[4][4] = {
	{   // 1st round
		{
			{
				3,3,0,1,0,1,0,0,0,3,0,0,1,3,1,2,0,3,3,3,2,1,0,1,1,1,2,2,2,3,2,2,
				2,1,3,3,1,3,1,1,0,0,1,2,0,2,2,1,1,2,3,1,2,1,3,1,2,2,0,1,3,0,2,2,
			},
			{1,3,4,5,6,7},
			{0,7}
		},

		{
			{
				0,1,3,0,1,1,2,3,2,0,0,3,2,1,3,1,3,3,0,0,1,0,0,3,0,3,3,2,3,2,0,1,
				3,2,3,2,2,1,3,1,1,1,0,3,3,2,2,1,1,2,0,2,0,1,1,0,1,0,1,1,2,0,3,0,
			},
			{0,3,5,6,5,0},
			{1,2}
		},

		{
			{
				0,2,2,1,0,1,2,1,2,0,1,2,3,3,0,1,3,1,1,2,1,2,1,3,3,2,3,3,2,1,0,1,
				0,1,0,2,0,1,1,3,2,0,3,2,1,1,1,3,2,3,0,2,3,0,2,2,1,3,0,1,1,2,2,2,
			},
			{0,2,3,4,7,-1},
			{3,4}
		},

		{
			{
				2,3,1,3,2,0,1,2,0,0,3,3,3,3,3,1,2,0,2,1,2,3,0,2,0,1,0,3,0,2,1,0,
				2,3,0,1,3,0,3,2,3,1,2,0,3,1,1,2,0,3,0,0,2,0,2,1,2,2,3,2,1,2,3,1,
			},
			{1,2,5,6,-1,-1},
			{5,6}
		},
	},
	{   // 2nd round
		{
			{
				2,3,1,3,1,0,3,3,3,2,3,3,2,0,0,3,2,3,0,3,1,1,2,3,1,1,2,2,0,1,0,0,
				2,1,0,1,2,0,1,2,0,3,1,1,2,3,1,2,0,2,0,1,3,0,1,0,2,2,3,0,3,2,3,0,
			},
			{0,1,4,5,6,7},
			{0,7}
		},

		{
			{
				0,2,2,0,2,2,0,3,2,3,2,1,3,2,3,3,1,1,0,0,3,0,2,1,1,3,3,2,3,2,0,1,
				1,2,3,0,1,0,3,0,3,1,0,2,1,2,0,3,2,3,1,2,2,0,3,2,3,0,0,1,2,3,3,3,
			},
			{0,2,3,6,7,-1},
			{1,5}
		},

		{
			{
				1,0,3,0,0,1,2,1,0,0,1,0,0,0,2,3,2,2,0,2,0,1,3,0,2,0,1,3,2,3,0,1,
				1,2,2,2,1,3,0,3,0,1,1,0,3,2,3,3,2,0,0,3,1,2,1,3,3,2,1,0,2,1,2,3,
			},
			{2,3,4,6,7,2},
			{2,3}
		},

		{
			{
				2,3,1,3,1,1,2,3,3,1,1,0,1,0,2,3,2,1,0,0,2,2,0,1,0,2,2,2,0,2,1,0,
				3,1,2,3,1,3,0,2,1,0,1,0,0,1,2,2,3,2,3,1,3,2,1,1,2,0,2,1,3,3,1,0,
			},
			{1,2,3,4,5,6},
			{4,6}
		},
	},
	{   // 3rd round
		{
			{
				0,3,0,1,3,0,0,2,1,0,1,3,2,2,2,0,3,3,3,0,2,2,0,3,0,0,2,3,0,3,2,1,
				3,3,0,3,0,2,3,3,1,1,1,0,2,2,1,1,3,0,3,1,2,0,2,0,0,0,3,2,1,1,0,0,
			},
			{1,4,5,6,7,5},
			{0,5}
		},

		{
			{
				0,3,0,1,3,0,3,1,3,2,2,2,3,0,3,2,2,1,2,2,0,3,2,2,0,0,2,1,1,3,2,3,
				2,3,3,1,2,0,1,2,2,1,0,0,0,0,2,3,1,2,0,3,1,3,1,2,3,2,1,0,3,0,0,2,
			},
			{0,2,3,4,6,7},
			{1,7}
		},

		{
			{
				2,2,0,3,0,3,1,0,1,1,2,3,2,3,1,0,0,0,3,2,2,0,2,3,1,3,2,0,3,3,1,3,
				// unused?
				255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			},
			{1,2,4,7,2,-1},
			{2,4}
		},

		{
			{
				0,2,3,1,3,1,1,0,0,1,3,0,2,1,3,3,2,0,2,1,1,2,3,3,0,0,0,2,0,2,3,0,
				3,3,3,3,2,3,3,2,3,0,1,0,2,3,3,2,0,1,3,1,0,1,2,3,3,0,2,0,3,0,3,3,
			},
			{0,1,2,3,5,7},
			{3,6}
		},
	},
	{   // 4th round
		{
			{
				0,1,1,0,0,1,0,2,3,3,0,1,2,3,0,2,1,0,3,3,2,0,3,0,0,2,1,0,1,0,1,3,
				0,3,3,1,2,0,3,0,1,3,2,0,3,3,1,3,0,2,3,3,2,1,1,2,2,1,2,1,2,0,1,1,
			},
			{0,1,2,4,7,-1},
			{0,5}
		},

		{
			{
				2,0,0,2,3,0,2,3,3,1,1,1,2,1,1,0,0,2,1,0,0,3,1,0,0,3,3,0,1,0,1,2,
				0,2,0,2,0,1,2,3,2,1,1,0,3,3,3,3,3,3,1,0,3,0,0,2,0,3,2,0,2,2,0,1,
			},
			{0,1,3,5,6,-1},
			{1,3}
		},

		{
			{
				0,1,1,2,1,3,1,1,0,0,3,1,1,1,2,0,3,2,0,1,1,2,3,3,3,0,3,0,0,2,0,3,
				3,2,0,0,3,2,3,1,2,3,0,3,2,0,1,2,2,2,0,2,0,1,2,2,3,1,2,2,1,1,1,1,
			},
			{0,2,3,4,5,7},
			{2,7}
		},

		{
			{
				0,1,2,0,3,3,0,3,2,1,3,3,0,3,1,1,3,2,3,2,3,0,0,0,3,0,2,2,3,2,2,3,
				2,2,3,1,2,3,1,2,0,3,0,2,3,1,0,0,3,2,1,2,1,2,1,3,1,0,2,3,3,1,3,2,
			},
			{2,3,4,5,6,7},
			{4,6}
		},
	},
};

const int sega_315_5881_crypt_device::fn1_game_key_scheduling[FN1GK][2] = {
	{1,29},  {1,71},  {2,4},   {2,54},  {3,8},   {4,56},  {4,73},  {5,11},
	{6,51},  {7,92},  {8,89},  {9,9},   {9,39},  {9,58},  {10,90}, {11,6},
	{12,64}, {13,49}, {14,44}, {15,40}, {16,69}, {17,15}, {18,23}, {18,43},
	{19,82}, {20,81}, {21,32}, {22,5},  {23,66}, {24,13}, {24,45}, {25,12},
	{25,35}, {26,61}, {27,10}, {27,59}, {28,25}, {29,86}
};

const int sega_315_5881_crypt_device::fn2_game_key_scheduling[FN2GK][2] = {
	{0,0},   {1,3},   {2,11},  {3,20},  {4,22},  {5,23},  {6,29},  {7,38},
	{8,39},  {9,55},  {9,86},  {9,87},  {10,50}, {11,57}, {12,59}, {13,61},
	{14,63}, {15,67}, {16,72}, {17,83}, {18,88}, {19,94}, {20,35}, {21,17},
	{22,6},  {23,85}, {24,16}, {25,25}, {26,92}, {27,47}, {28,28}, {29,90}
};

const int sega_315_5881_crypt_device::fn1_sequence_key_scheduling[20][2] = {
	{0,52},  {1,34},  {2,17},  {3,36}, {4,84},  {4,88},  {5,57},  {6,48},
	{6,68},  {7,76},  {8,83},  {9,30}, {10,22}, {10,41}, {11,38}, {12,55},
	{13,74}, {14,19}, {14,80}, {15,26}
};

const int sega_315_5881_crypt_device::fn2_sequence_key_scheduling[16] = {77,34,8,42,36,27,69,66,13,9,79,31,49,7,24,64};

const int sega_315_5881_crypt_device::fn2_middle_result_scheduling[16] = {1,10,44,68,74,78,81,95,2,4,30,40,41,51,53,58};

int sega_315_5881_crypt_device::feistel_function(int input, const struct sbox *sboxes, UINT32 subkeys)
{
	int k,m;
	int aux;
	int result=0;

	for (m=0; m<4; ++m) { // 4 sboxes
		for (k=0, aux=0; k<6; ++k)
			if (sboxes[m].inputs[k]!=-1)
				aux |= BIT(input, sboxes[m].inputs[k]) << k;

		aux = sboxes[m].table[(aux^subkeys)&0x3f];

		for (k=0; k<2; ++k)
			result |= BIT(aux,k) << sboxes[m].outputs[k];

		subkeys >>=6;
	}

	return result;
}

/**************************
This implementation is an "educational" version. It must be noted that it can be speed-optimized in a number of ways.
The most evident one is to factor out the parts of the key-scheduling that must only be done once (like the game-key &
sequence-key parts) as noted in the comments inlined in the function. More sophisticated speed-ups can be gained by
noticing that the weak key-scheduling would allow to create some pregenerated look-up tables for doing most of the work
of the function. Even so, it would still be pretty slow, so caching techniques could be a wiser option here.
**************************/

UINT16 sega_315_5881_crypt_device::block_decrypt(UINT32 game_key, UINT16 sequence_key, UINT16 counter, UINT16 data)
{
	int j;
	int aux, aux2;
	int A, B;
	int middle_result;
	UINT32 fn1_subkeys[4];
	UINT32 fn2_subkeys[4];

	/* Game-key scheduling; this could be done just once per game at initialization time */
	memset(fn1_subkeys, 0, sizeof(UINT32) * 4);
	memset(fn2_subkeys, 0, sizeof(UINT32) * 4);

	for (j = 0; j < FN1GK; ++j) {
		if (BIT(game_key, fn1_game_key_scheduling[j][0]) != 0) {
			aux = fn1_game_key_scheduling[j][1] % 24;
			aux2 = fn1_game_key_scheduling[j][1] / 24;
			fn1_subkeys[aux2] ^= (1 << aux);
		}
	}

	for (j = 0; j < FN2GK; ++j) {
		if (BIT(game_key, fn2_game_key_scheduling[j][0]) != 0) {
			aux = fn2_game_key_scheduling[j][1] % 24;
			aux2 = fn2_game_key_scheduling[j][1] / 24;
			fn2_subkeys[aux2] ^= (1 << aux);
		}
	}
	/********************************************************/

	/* Sequence-key scheduling; this could be done just once per decryption run */
	for (j = 0; j < 20; ++j) {
		if (BIT(sequence_key, fn1_sequence_key_scheduling[j][0]) != 0) {
			aux = fn1_sequence_key_scheduling[j][1] % 24;
			aux2 = fn1_sequence_key_scheduling[j][1] / 24;
			fn1_subkeys[aux2] ^= (1 << aux);
		}
	}

	for (j = 0; j < 16; ++j) {
		if (BIT(sequence_key, j) != 0) {
			aux = fn2_sequence_key_scheduling[j] % 24;
			aux2 = fn2_sequence_key_scheduling[j] / 24;
			fn2_subkeys[aux2] ^= (1 << aux);
		}
	}

	/**************************************************************/

	// First Feistel Network

	aux = BITSWAP16(counter, 5, 12, 14, 13, 9, 3, 6, 4, 8, 1, 15, 11, 0, 7, 10, 2);

	// 1st round
	B = aux >> 8;
	A = (aux & 0xff) ^ feistel_function(B, fn1_sboxes[0], fn1_subkeys[0]);

	// 2nd round
	B ^= feistel_function(A, fn1_sboxes[1], fn1_subkeys[1]);

	// 3rd round
	A ^= feistel_function(B, fn1_sboxes[2], fn1_subkeys[2]);

	// 4th round
	B ^= feistel_function(A, fn1_sboxes[3], fn1_subkeys[3]);

	middle_result = (B << 8) | A;


	/* Middle-result-key sheduling */
	for (j = 0; j < 16; ++j) {
		if (BIT(middle_result, j) != 0) {
			aux = fn2_middle_result_scheduling[j] % 24;
			aux2 = fn2_middle_result_scheduling[j] / 24;
			fn2_subkeys[aux2] ^= (1 << aux);
		}
	}
	/*********************/

	// Second Feistel Network

	aux = BITSWAP16(data, 14, 3, 8, 12, 13, 7, 15, 4, 6, 2, 9, 5, 11, 0, 1, 10);

	// 1st round
	B = aux >> 8;
	A = (aux & 0xff) ^ feistel_function(B, fn2_sboxes[0], fn2_subkeys[0]);

	// 2nd round
	B ^= feistel_function(A, fn2_sboxes[1], fn2_subkeys[1]);

	// 3rd round
	A ^= feistel_function(B, fn2_sboxes[2], fn2_subkeys[2]);

	// 4th round
	B ^= feistel_function(A, fn2_sboxes[3], fn2_subkeys[3]);

	aux = (B << 8) | A;

	aux = BITSWAP16(aux, 15, 7, 6, 14, 13, 12, 5, 4, 3, 2, 11, 10, 9, 1, 0, 8);

	return aux;
}


UINT16 sega_315_5881_crypt_device::get_decrypted_16()
{
	UINT16 enc;

	enc = m_read(prot_cur_address);

	UINT16 dec = block_decrypt(key, subkey, prot_cur_address, enc);
	UINT16 res = (dec & 3) | (dec_hist & 0xfffc);
	dec_hist = dec;

	prot_cur_address ++;

//  printf("get_decrypted_16 %04x\n", res);

	return res;
}


void sega_315_5881_crypt_device::enc_start()
{
	block_pos = 0;
	done_compression = 0;
	buffer_pos = BUFFER_SIZE;

	if (buffer_bit2 != 15) // if we have remaining bits in the decompression buffer we shouldn't read the next word yet but should instead use the bits we have?? (twcup98) (might just be because we should be pulling bytes not words?)
	{
//      printf("buffer_bit2 is %d\n", buffer_bit2);
		dec_header = (buffer2a & 0x0003) << 16;
	}
	else
	{
		dec_hist = 0; // seems to be needed by astrass at least otherwise any call after the first one will be influenced by the one before it.
		dec_header = get_decrypted_16() << 16;
	}

	dec_header |= get_decrypted_16();

	// the lower header bits are 2 values that multiply together to get the current stream length
	// in astrass the first block is 0xffff (for a 0x10000 block) followed by 0x3f3f (for a 0x1000 block)
	// etc. after each block a new header must be read, it looks like compressed and uncompressed blocks
	// can be mixed like this, I don't know if the length is src length of decompressed length.
	// deathcox and others confirm format as 0x20000 bit as compressed bit, 0x1ff00 bits as block size 1, 0x000ff bits as block size 2
	// for compressed streams the 'line size' is block size 1.

	block_numlines = ((dec_header & 0x000000ff) >> 0) + 1;
	int blocky = ((dec_header & 0x0001ff00) >> 8) + 1;
	block_size = block_numlines * blocky;

	if(dec_header & FLAG_COMPRESSED) {
		line_buffer_size = blocky;
		line_buffer_pos = line_buffer_size;
		buffer_bit = 7;
		buffer_bit2 = 15;
	}

	printf("header %08x\n", dec_header);
	enc_ready = true;
}

void sega_315_5881_crypt_device::enc_fill()
{
	assert(buffer_pos == BUFFER_SIZE);
	for(int i = 0; i != BUFFER_SIZE; i+=2) {
		UINT16 val = get_decrypted_16();
		buffer[i] = val;
		buffer[i+1] = val >> 8;
		block_pos+=2;

		if (!(dec_header & FLAG_COMPRESSED))
		{
			if (block_pos == block_size)
			{
				// if we reach the size specified we need to read a new header
				// todo: for compressed blocks this depends on OUTPUT size, not input size, so things get messy

				enc_start();
			}
		}
	}
	buffer_pos = 0;
}

/* node format
0xxxxxxx - next node index
1a0bbccc - end node
           a - 0 = repeat
               1 = fetch
           b - if a = 1
               00 - fetch  0
               01 - fetch  1
               11 - fetch -1
               if a = 0
               000
           c - repeat/fetch counter
               count = ccc + 1
11111111 - empty node
*/
const UINT8 sega_315_5881_crypt_device::trees[9][2][32] = {
	{
		{0x01,0x10,0x0f,0x05,0xc4,0x13,0x87,0x0a,0xcc,0x81,0xce,0x0c,0x86,0x0e,0x84,0xc2,
			0x11,0xc1,0xc3,0xcf,0x15,0xc8,0xcd,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
		{0xc7,0x02,0x03,0x04,0x80,0x06,0x07,0x08,0x09,0xc9,0x0b,0x0d,0x82,0x83,0x85,0xc0,
			0x12,0xc6,0xc5,0x14,0x16,0xca,0xcb,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
	},
	{
		{0x02,0x80,0x05,0x04,0x81,0x10,0x15,0x82,0x09,0x83,0x0b,0x0c,0x0d,0xdc,0x0f,0xde,
			0x1c,0xcf,0xc5,0xdd,0x86,0x16,0x87,0x18,0x19,0x1a,0xda,0xca,0xc9,0x1e,0xce,0xff,},
		{0x01,0x17,0x03,0x0a,0x08,0x06,0x07,0xc2,0xd9,0xc4,0xd8,0xc8,0x0e,0x84,0xcb,0x85,
			0x11,0x12,0x13,0x14,0xcd,0x1b,0xdb,0xc7,0xc0,0xc1,0x1d,0xdf,0xc3,0xc6,0xcc,0xff,},
	},
	{
		{0xc6,0x80,0x03,0x0b,0x05,0x07,0x82,0x08,0x15,0xdc,0xdd,0x0c,0xd9,0xc2,0x14,0x10,
			0x85,0x86,0x18,0x16,0xc5,0xc4,0xc8,0xc9,0xc0,0xcc,0xff,0xff,0xff,0xff,0xff,0xff,},
		{0x01,0x02,0x12,0x04,0x81,0x06,0x83,0xc3,0x09,0x0a,0x84,0x11,0x0d,0x0e,0x0f,0x19,
			0xca,0xc1,0x13,0xd8,0xda,0xdb,0x17,0xde,0xcd,0xcb,0xff,0xff,0xff,0xff,0xff,0xff,},
	},
	{
		{0x01,0x80,0x0d,0x04,0x05,0x15,0x83,0x08,0xd9,0x10,0x0b,0x0c,0x84,0x0e,0xc0,0x14,
			0x12,0xcb,0x13,0xca,0xc8,0xc2,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
		{0xc5,0x02,0x03,0x07,0x81,0x06,0x82,0xcc,0x09,0x0a,0xc9,0x11,0xc4,0x0f,0x85,0xd8,
			0xda,0xdb,0xc3,0xdc,0xdd,0xc1,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
	},
	{
		{0x01,0x80,0x06,0x0c,0x05,0x81,0xd8,0x84,0x09,0xdc,0x0b,0x0f,0x0d,0x0e,0x10,0xdb,
			0x11,0xca,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
		{0xc4,0x02,0x03,0x04,0xcb,0x0a,0x07,0x08,0xd9,0x82,0xc8,0x83,0xc0,0xc1,0xda,0xc2,
			0xc9,0xc3,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
	},
	{
		{0x01,0x02,0x06,0x0a,0x83,0x0b,0x07,0x08,0x09,0x82,0xd8,0x0c,0xd9,0xda,0xff,0xff,
			0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
		{0xc3,0x80,0x03,0x04,0x05,0x81,0xca,0xc8,0xdb,0xc9,0xc0,0xc1,0x0d,0xc2,0xff,0xff,
			0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
	},
	{
		{0x01,0x02,0x03,0x04,0x81,0x07,0x08,0xd8,0xda,0xd9,0xff,0xff,0xff,0xff,0xff,0xff,
			0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
		{0xc2,0x80,0x05,0xc9,0xc8,0x06,0x82,0xc0,0x09,0xc1,0xff,0xff,0xff,0xff,0xff,0xff,
			0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
	},
	{
		{0x01,0x80,0x04,0xc8,0xc0,0xd9,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
			0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
		{0xc1,0x02,0x03,0x81,0x05,0xd8,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
			0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
	},
	{
		{0x01,0xd8,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
			0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
		{0xc0,0x80,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
			0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
	},
};

int sega_315_5881_crypt_device::get_compressed_bit()
{
//  if(buffer_pos == BUFFER_SIZE)
//      enc_fill();

	if (buffer_bit2 == 15)
	{
		buffer_bit2 = 0;
		buffer2a = get_decrypted_16();
		buffer2[0] = buffer2a;
		buffer2[1] = buffer2a >> 8;
	//  block_pos+=2;
		buffer_pos = 0;

	}
	else
	{
		buffer_bit2++;
	}

//  if (buffer_bit ==7) printf("using byte %02x\n", buffer2[(buffer_pos&1) ^ 1]);

	int res = (buffer2[(buffer_pos&1)^1] >> buffer_bit) & 1;
	buffer_bit--;
	if(buffer_bit == -1) {
		buffer_bit = 7;
		buffer_pos++;
	}
	return res;
}
void sega_315_5881_crypt_device::line_fill()
{
	assert(line_buffer_pos == line_buffer_size);
	UINT8 *lp = line_buffer.get();
	UINT8 *lc = line_buffer_prev.get();
	
	line_buffer.swap(line_buffer_prev);

	line_buffer_pos = 0;

	for(int i=0; i != line_buffer_size;) {
		// vlc 0: start of line
		// vlc 1: interior of line
		// vlc 2-9: 7-1 bytes from end of line

		int slot = i ? i < line_buffer_size - 7 ? 1 : (i & 7) + 1 : 0;

		UINT32 tmp = 0;
		while (!(tmp&0x80))
			if(get_compressed_bit())
				tmp = trees[slot][1][tmp];
			else
				tmp = trees[slot][0][tmp];
		if(tmp != 0xff) {
			int count = (tmp & 7) + 1;

			if(tmp&0x40) {
				// Copy from previous line

				static int offsets[4] = {0, 1, 0, -1};
				int offset = offsets[(tmp & 0x18) >> 3];
				for(int j=0; j != count; j++) {
					lc[i^1] = lp[((i+offset) % line_buffer_size)^1];
					i++;
				}

			} else {
				// Get a byte in the stream and write n times
				UINT8 byte;
				byte =         get_compressed_bit()  << 1;
				byte = (byte | get_compressed_bit()) << 1;
				byte = (byte | get_compressed_bit()) << 1;
				byte = (byte | get_compressed_bit()) << 1;
				byte = (byte | get_compressed_bit()) << 1;
				byte = (byte | get_compressed_bit()) << 1;
				byte = (byte | get_compressed_bit()) << 1;
				byte =  byte | get_compressed_bit();
				for(int j=0; j != count; j++)
					lc[(i++)^1] = byte;

			}
		}
	}

	block_pos++;
	if (block_numlines == block_pos)
	{
		done_compression = 1;
	}
	else
	{
	}
}
