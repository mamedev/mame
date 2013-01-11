
#include "emu.h"
#include "naomim2.h"

const device_type NAOMI_M2_BOARD = &device_creator<naomi_m2_board>;

naomi_m2_board::naomi_m2_board(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: naomi_board(mconfig, NAOMI_M2_BOARD, "NAOMI-M2-BOARD", tag, owner, clock)
{
	key_tag = 0;
}

void naomi_m2_board::static_set_tags(device_t &device, const char *_key_tag)
{
	naomi_m2_board &dev = downcast<naomi_m2_board &>(device);
	dev.key_tag = _key_tag;
}

void naomi_m2_board::device_start()
{
	naomi_board::device_start();

	const UINT8 *key_data = machine().root_device().memregion(key_tag)->base();
	key = (key_data[0] << 24) | (key_data[1] << 16) | (key_data[2] << 8) | key_data[3];

	ram = auto_alloc_array(machine(), UINT8, RAM_SIZE);
	buffer = auto_alloc_array(machine(), UINT8, BUFFER_SIZE);
	line_buffer = auto_alloc_array(machine(), UINT8, LINE_SIZE);
	line_buffer_prev = auto_alloc_array(machine(), UINT8, LINE_SIZE);

	save_pointer(NAME(ram), RAM_SIZE);
	save_pointer(NAME(buffer), BUFFER_SIZE);
	save_pointer(NAME(line_buffer), LINE_SIZE);
	save_pointer(NAME(line_buffer_prev), LINE_SIZE);
	save_item(NAME(rom_cur_address));
	save_item(NAME(prot_cur_address));
	save_item(NAME(subkey));
	save_item(NAME(enc_ready));
	save_item(NAME(dec_hist));
	save_item(NAME(dec_header));
	save_item(NAME(buffer_pos));
	save_item(NAME(line_buffer_pos));
	save_item(NAME(line_buffer_size));
}

void naomi_m2_board::device_reset()
{
	naomi_board::device_reset();

	memset(ram, 0, RAM_SIZE);
	memset(buffer, 0, BUFFER_SIZE);
	memset(line_buffer, 0, LINE_SIZE);
	memset(line_buffer_prev, 0, LINE_SIZE);

	rom_cur_address = 0;
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

void naomi_m2_board::board_setup_address(UINT32 address, bool is_dma)
{
	rom_cur_address = address;
}

void naomi_m2_board::board_get_buffer(UINT8 *&base, UINT32 &limit)
{
	if(rom_cur_address & 0x40000000) {

		if(rom_cur_address == 0x4001fffe) {
			if(!enc_ready)
				enc_start();
			if(dec_header & FLAG_COMPRESSED) {
				if(line_buffer_pos == line_buffer_size)
					line_fill();
				base = line_buffer + line_buffer_pos;
				line_buffer_pos += 2;
			} else {
				if(buffer_pos == BUFFER_SIZE)
					enc_fill();
				base = buffer + buffer_pos;
				buffer_pos += 2;
			}
			limit = 2;

		} else
			throw emu_fatalerror("NAOMIM2: Unsupported, read from %08x", rom_cur_address);

	} else {
		base = m_region->base() + (rom_cur_address & 0x1fffffff);
		limit = m_region->bytes() - (rom_cur_address & 0x1fffffff);
	}
}

void naomi_m2_board::board_advance(UINT32 size)
{
	rom_cur_address += size;
}

void naomi_m2_board::board_write(offs_t offset, UINT16 data)
{
	if(offset & 0x40000000) {
		if((offset & 0x0f000000) == 0x02000000) {
			offset &= RAM_SIZE-1;
			ram[offset] = data;
			ram[offset+1] = data >> 8;
			return;
		}
		switch(offset & 0x1fffffff) {
		case 0x1fff8: prot_cur_address = (prot_cur_address & 0xffff0000) | data; enc_ready = false; return;
		case 0x1fffa: prot_cur_address = (prot_cur_address & 0x0000ffff) | (data << 16); enc_ready = false; return;
		case 0x1fffc: subkey = data; enc_ready = false; return;
		}
	}
	throw emu_fatalerror("NAOMIM2: unhandled board write %08x, %04x\n", offset, data);
}

/***************************************************************************
    DECRYPTION EMULATION

By convention, we label the three known cart protection methods this way (using Deunan Knute's wording):
M1: DMA read of protected ROM area
M2: special read of ROM area which supplies decryption key first
M3: normal read followed by write to cart's decryption buffer (up to 64kB), followed by M2 but from buffer area

Notes below refer to M2 & M3.

The encryption is done by a stream cipher operating in counter mode, which use a 16-bits internal block cipher.

There are 2 "control bits" at the start of the decrypted stream which control the mode of operation: bit #1 set to 1 means
that the decrypted stream needs to be decompressed after being decrypted. More on this later.

The next 16-bits are part of the header (they don't belong to the plaintext), but his meaning is unclear. It has been
conjectured that it could stablish when to "reset" the process and start processing a new stream (based on some tests
on WWFROYAL, in which the decryption's output doesn't seem to be valid for more than some dozens of words), but some
more testing would be needed for clarifying that.

After those 18 heading bits, we find the proper plaintext. It must be noted that, due to the initial 2 special bits,
the 16-bits words of the plaintext are shifted 2 bits respect to the word-boundaries of the output stream of the
internal block-cipher. So, at a given step, the internal block cipher will output 16-bits, 14 of which will go to a
given plaintext word, and the remaining 2 to the next plaintext word.

The underlying block cipher consists of two 4-round Feistel Networks (FN): the first one takes the counter (16 bits),
the game-key (>=26 bits) and the sequence-key (16 bits) and output a middle result (16 bits) which will act as another key
for the second one. The second FN will take the encrypted word (16 bits), the game-key, the sequence-key and the result
from the first FN and will output the decrypted word (16 bits).

Each round of the Feistel Networks use four substitution sboxes, each having 6 inputs and 2 outputs. The input can be the
XOR of at most two "sources bits", being source bits the bits from the previous round and the bits from the different keys.

The underlying block cipher has the same structure than the one used by the CPS-2 (Capcom Play System 2) and,
indeed, some of the used sboxes are exactly the same and appear in the same FN/round in both systems (this is not evident,
as you need to apply a bitswapping and some XORs to the input & output of the sboxes to get the same values due). However,
the key scheduling used by this implementation is much weaker than the CPS-2's one. Many s-boxes inputs are XORed with any
key bit.

Due to the small key-length, no sophisticated attacks are needed to recover the keys; a brute-force attack knowing just
some (encrypted word-decrypted word) pairs suffice. However, due to the weak key scheduling, it should be noted that some
related keys can produce the same output bytes for some (short) input sequences.

The only difference in the decryption process between M2 and M3 is the initialization of the counter. In M3, the counter is
always set to 0 at the beginning of the decryption while, in M2, the bits #1-#16 of the ciphertext's address are used
to initialize the counter.

Note that this implementation considers that the counter initialization for ram decryption is 0 simply because the ram is
mapped to multiples of 128K.

Due to the nature of the cipher, there are some degrees of freedom when choosing the s-boxes and keys values; by example,
you could apply a fixed bitswapping and XOR to the keys and the decryption would remain the same as long as you change
accordingly the s-boxes' definitions. So the order of the bits in the keys is arbitrary, and the s-boxes values have been
chosen so as to make the key for CAPSNK equal to 0.

It can be observed that a couple of sboxes have incomplete tables (a 255 value indicate an unknown value). The recovered keys
as of december/2010 show small randomness and big correlations, making possible that some unseen bits could make the
decryption need those incomplete parts.

****************************************************************************************/

const naomi_m2_board::sbox naomi_m2_board::fn1_sboxes[4][4] = {
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
				2,2,2,3,1,1,0,1,0,1,2,2,3,3,0,2,0,3,2,3,3,0,2,1,0,3,1,0,0,2,3,2,
				3,2,0,3,2,0,1,0,3,3,1,1,2,2,2,0,2,1,3,1,1,1,1,2,2,2,3,0,1,3,0,0,
			},
			{1,2,5,6,7,-1},
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


const naomi_m2_board::sbox naomi_m2_board::fn2_sboxes[4][4] = {
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
				0,2,3,2,1,1,0,0,2,1,0,3,3,0,0,0,3,2,0,2,1,1,2,1,0,0,3,1,2,2,3,1,
				3,1,3,0,0,0,1,3,1,0,0,3,2,2,3,1,1,3,0,0,2,1,3,3,1,3,1,2,3,1,2,1,
			},
			{0,3,5,6,-1,-1},
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
				1,2,3,2,0,3,2,3,0,1,1,0,0,2,2,3,2,0,0,3,0,2,3,3,2,2,1,0,2,1,0,3,
				1,0,2,0,1,1,0,1,0,0,1,0,3,0,3,3,2,2,0,2,1,1,1,0,3,0,1,3,2,3,2,1,
			},
			{2,3,4,6,7,-1},
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
				0,3,0,1,0,2,3,3,1,0,1,3,2,2,1,1,3,3,3,0,2,0,2,0,0,0,2,3,1,1,0,0,
				3,3,0,3,3,0,0,2,1,1,1,0,2,2,2,0,3,0,3,1,2,2,0,3,0,0,3,2,0,3,2,1,
			},
			{1,4,5,6,7,-1},
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
				2,2,3,2,0,3,2,3,1,1,2,0,2,3,1,3,0,0,0,3,2,0,1,0,1,3,2,3,3,3,1,0,
				// unused?
				255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
			},
			{1,2,4,7,-1,-1},
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

const int naomi_m2_board::fn1_game_key_scheduling[38][2] = {
	{1,29},  {1,71},  {2,4},   {2,54},  {3,8},   {4,56},  {4,73},  {5,11},
	{6,51},  {7,92},  {8,89},  {9,9},   {9,10},  {9,39},  {9,41},  {9,58},
	{9,59},  {9,86},  {10,90}, {11,6},  {12,64}, {13,49}, {14,44}, {15,40},
	{16,69}, {17,15}, {18,23}, {18,43}, {19,82}, {20,81}, {21,32}, {21,61},
	{22,5},  {23,66}, {24,13}, {24,45}, {25,12}, {25,35}
};

const int naomi_m2_board::fn2_game_key_scheduling[34][2] = {
	{0,0},   {1,3},   {2,11},  {3,20},  {4,22},  {5,23},  {6,29},  {7,38},
	{8,39},  {9,47},  {9,55},  {9,86},  {9,87},  {9,90},  {10,50}, {10,53},
	{11,57}, {12,59}, {13,61}, {13,64}, {14,63}, {15,67}, {16,72}, {17,83},
	{18,88}, {19,94}, {20,35}, {21,17}, {21,92}, {22,6},  {22,11}, {23,85},
	{24,16}, {25,25}
};

const int naomi_m2_board::fn1_sequence_key_scheduling[20][2] = {
	{0,52},  {1,34},  {2,17},  {3,36}, {4,84},  {4,88},  {5,57},  {6,48},
	{6,68},  {7,76},  {8,83},  {9,30}, {10,22}, {10,41}, {11,38}, {12,55},
	{13,74}, {14,19}, {14,80}, {15,26}
};

const int naomi_m2_board::fn2_sequence_key_scheduling[16] = {77,34,8,42,36,27,69,66,13,9,79,31,49,7,24,64};

const int naomi_m2_board::fn2_middle_result_scheduling[16] = {1,10,44,68,74,78,81,95,2,4,30,40,41,51,53,58};

int naomi_m2_board::feistel_function(int input, const struct sbox *sboxes, UINT32 subkeys)
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

UINT16 naomi_m2_board::block_decrypt(UINT32 game_key, UINT16 sequence_key, UINT16 counter, UINT16 data)
{
	int j;
	int aux,aux2;
	int A,B;
	int middle_result;
	UINT32 fn1_subkeys[4];
	UINT32 fn2_subkeys[4];

	/* Game-key scheduling; this could be done just once per game at initialization time */
	memset(fn1_subkeys,0,sizeof(UINT32)*4);
	memset(fn2_subkeys,0,sizeof(UINT32)*4);

	for (j=0; j<38; ++j) {
		if (BIT(game_key, fn1_game_key_scheduling[j][0])!=0) {
			aux = fn1_game_key_scheduling[j][1]%24;
			aux2 = fn1_game_key_scheduling[j][1]/24;
			fn1_subkeys[aux2] ^= (1<<aux);
		}
	}

	for (j=0; j<34; ++j) {
		if (BIT(game_key, fn2_game_key_scheduling[j][0])!=0) {
			aux = fn2_game_key_scheduling[j][1]%24;
			aux2 = fn2_game_key_scheduling[j][1]/24;
			fn2_subkeys[aux2] ^= (1<<aux);
		}
	}
	/********************************************************/

	/* Sequence-key scheduling; this could be done just once per decryption run */
	for (j=0; j<20; ++j) {
		if (BIT(sequence_key,fn1_sequence_key_scheduling[j][0])!=0) {
			aux = fn1_sequence_key_scheduling[j][1]%24;
			aux2 = fn1_sequence_key_scheduling[j][1]/24;
			fn1_subkeys[aux2] ^= (1<<aux);
		}
	}

	for (j=0; j<16; ++j) {
		if (BIT(sequence_key,j)!=0) {
			aux = fn2_sequence_key_scheduling[j]%24;
			aux2 = fn2_sequence_key_scheduling[j]/24;
			fn2_subkeys[aux2] ^= (1<<aux);
		}
	}

	// subkeys bits 10 & 41
	fn2_subkeys[0] ^= (BIT(sequence_key,2)<<10);
	fn2_subkeys[1] ^= (BIT(sequence_key,4)<<17);
	/**************************************************************/

	// First Feistel Network

	aux = BITSWAP16(counter,5,12,14,13,9,3,6,4,    8,1,15,11,0,7,10,2);

	// 1st round
	B = aux >> 8;
	A = (aux & 0xff) ^ feistel_function(B,fn1_sboxes[0],fn1_subkeys[0]);

	// 2nd round
	B = B ^ feistel_function(A,fn1_sboxes[1],fn1_subkeys[1]);

	// 3rd round
	A = A ^ feistel_function(B,fn1_sboxes[2],fn1_subkeys[2]);

	// 4th round
	B = B ^ feistel_function(A,fn1_sboxes[3],fn1_subkeys[3]);

	middle_result = (B<<8)|A;


	/* Middle-result-key sheduling */
	for (j=0; j<16; ++j) {
		if (BIT(middle_result,j)!=0) {
			aux = fn2_middle_result_scheduling[j]%24;
			aux2 = fn2_middle_result_scheduling[j]/24;
			fn2_subkeys[aux2] ^= (1<<aux);
		}
	}
	/*********************/

	// Second Feistel Network

	aux = BITSWAP16(data,14,3,8,12,13,7,15,4,    6,2,9,5,11,0,1,10);

	// 1st round
	B = aux >> 8;
	A = (aux & 0xff) ^ feistel_function(B,fn2_sboxes[0],fn2_subkeys[0]);

	// 2nd round
	B = B ^ feistel_function(A,fn2_sboxes[1],fn2_subkeys[1]);

	// 3rd round
	A = A ^ feistel_function(B,fn2_sboxes[2],fn2_subkeys[2]);

	// 4th round
	B = B ^ feistel_function(A,fn2_sboxes[3],fn2_subkeys[3]);

	aux = (B<<8)|A;

	aux = BITSWAP16(aux,15,7,6,14,13,12,5,4,    3,2,11,10,9,1,0,8);

	return aux;
}

UINT16 naomi_m2_board::get_decrypted_16()
{
	UINT16 enc;

	if((prot_cur_address & 0xffff0000) == 0x01000000) {
		int base = 2*(prot_cur_address & 0x7fff);
		enc = ram[base+1] | (ram[base] << 8);
	} else {
		const UINT8 *base = m_region->base() + 2*prot_cur_address;
		enc = base[1] | (base[0] << 8);
	}

	UINT16 dec = block_decrypt(key, subkey, prot_cur_address, enc);
	UINT16 res = (dec & 3) | (dec_hist & 0xfffc);
	dec_hist = dec;

	prot_cur_address ++;
	return res;
}

void naomi_m2_board::enc_start()
{
	buffer_pos = BUFFER_SIZE;
	dec_header = get_decrypted_16() << 16;
	dec_header |= get_decrypted_16();

	if(dec_header & FLAG_COMPRESSED) {
		line_buffer_size = dec_header & FLAG_LINE_SIZE_512 ? 512 : 256;
		line_buffer_pos = line_buffer_size;
		buffer_bit = 7;
	}
	enc_ready = true;
}

void naomi_m2_board::enc_fill()
{
	assert(buffer_pos == BUFFER_SIZE);
	for(int i = 0; i != BUFFER_SIZE; i+=2) {
		UINT16 val = get_decrypted_16();
		buffer[i] = val;
		buffer[i+1] = val >> 8;
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
const UINT8 naomi_m2_board::trees[9][2][32] = {
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

int naomi_m2_board::get_compressed_bit()
{
	if(buffer_pos == BUFFER_SIZE)
		enc_fill();
	int res = (buffer[buffer_pos^1] >> buffer_bit) & 1;
	buffer_bit--;
	if(buffer_bit == -1) {
		buffer_bit = 7;
		buffer_pos++;
	}
	return res;
}

void naomi_m2_board::line_fill()
{
	assert(line_buffer_pos == line_buffer_size);
	UINT8 *lp = line_buffer;
	UINT8 *lc = line_buffer_prev;
	line_buffer = lc;
	line_buffer_prev = lp;
	line_buffer_pos = 0;

	UINT32 line_buffer_mask = line_buffer_size-1;

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
					lc[i^1] = lp[((i+offset) & line_buffer_mask)^1];
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
}
