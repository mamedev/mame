// license:BSD-3-Clause
// copyright-holders:Olivier Galibert,Andreas Naive

#include "emu.h"
#include "naomim4.h"

// Decoder for M4-type NAOMI cart encryption

// In hardware, the decryption is managed by the XC3S50 Xilinx Spartan FPGA (IC2)
// and the annexed PIC16C621A PIC MCU (IC3).
// - The FPGA control the clock line of the security PIC.
// - The protocol between the FPGA and the MCU is nibble-based, though it hasn't been RE for now.
// - The decryption algorithm is clearly nibble-based too.

// The decryption algorithm itself implements a stream cipher built on top of a 16-bits block cipher.
// The underlying block-cipher is a SP-network of 2 rounds (both identical in structure). In every
// round, the substitution phase is done using 4 fixed 4-to-4 sboxes acting on every nibble. The permutation
// phase is indeed a nibble-based linear combination.
// With that block cipher, a stream cipher is constructed by feeding the output result of the 1st round
// of a certain 16-bits block as a whitening value for the next block. The cart dependent data used by
// the algorithm is a 32-bits key stored in the PIC16C621A. The hardware auto-reset the feed value
// to the cart-based IV every 16 blocks (32 bytes); that reset is not address-based, but index-based.

const device_type NAOMI_M4_BOARD = &device_creator<naomi_m4_board>;

const UINT8 naomi_m4_board::k_sboxes[4][16] = {
	{9,8,2,11,1,14,5,15,12,6,0,3,7,13,10,4},
	{2,10,0,15,14,1,11,3,7,12,13,8,4,9,5,6},
	{4,11,3,8,7,2,15,13,1,5,14,9,6,12,0,10},
	{1,13,8,2,0,5,6,14,4,11,15,10,12,3,7,9}
};

// from S29GL512N datasheet
static UINT8 cfidata[] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x51,0x00,0x52,0x00,0x59,0x00,0x02,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x27,0x00,0x36,0x00,0x00,0x00,0x00,0x00,0x07,0x00,
0x07,0x00,0x0a,0x00,0x00,0x00,0x01,0x00,0x05,0x00,0x04,0x00,0x00,0x00,0x1a,0x00,0x02,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x01,0x00,0xff,0x00,0x01,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x50,0x00,0x52,0x00,0x49,0x00,0x31,0x00,0x33,0x00,0x10,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0xb5,0x00,0xc5,0x00,0x04,0x00,
0x01,0x00
};

DEVICE_ADDRESS_MAP_START(submap, 16, naomi_m4_board)
	AM_RANGE(0x1a, 0x1b) AM_READ(m4_id_r)

	AM_INHERIT_FROM(naomi_board::submap)
ADDRESS_MAP_END

naomi_m4_board::naomi_m4_board(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: naomi_board(mconfig, NAOMI_M4_BOARD, "Sega NAOMI M4 Board", tag, owner, clock, "naomi_m4_board", __FILE__)
{
	key_tag = nullptr;
}

void naomi_m4_board::static_set_tags(device_t &device, const char *_key_tag)
{
	naomi_m4_board &dev = downcast<naomi_m4_board &>(device);
	dev.key_tag = _key_tag;
}

void naomi_m4_board::device_start()
{
	naomi_board::device_start();

	const UINT8 *key_data = memregion(key_tag)->base();
	subkey1 = (key_data[0x5e2] << 8) | key_data[0x5e0];
	subkey2 = (key_data[0x5e6] << 8) | key_data[0x5e4];

	buffer = std::make_unique<UINT8[]>(BUFFER_SIZE);
	enc_init();

	save_pointer(NAME(buffer.get()), BUFFER_SIZE);
	save_item(NAME(rom_cur_address));
	save_item(NAME(buffer_actual_size));
	save_item(NAME(encryption));
	save_item(NAME(cfi_mode));
	save_item(NAME(counter));
}

void naomi_m4_board::enc_init()
{
	one_round = std::make_unique<UINT16[]>(0x10000);

	for(int round_input = 0; round_input < 0x10000; round_input++) {
		UINT8 input_nibble[4];
		UINT8 output_nibble[4];

		for (int nibble_idx = 0; nibble_idx < 4; ++nibble_idx) {
			input_nibble[nibble_idx] = (round_input >> (nibble_idx*4)) & 0xf;
			output_nibble[nibble_idx] = 0;
		}

		UINT8 aux_nibble = input_nibble[3];
		for (int nibble_idx = 0; nibble_idx < 4; ++nibble_idx) { // 4 s-boxes per round
			aux_nibble ^= k_sboxes[nibble_idx][input_nibble[nibble_idx]];
			for (int i = 0; i < 4; ++i)  // diffusion of the bits
				output_nibble[(nibble_idx - i) & 3] |= aux_nibble & (1 << i);
		}

		UINT16 result = 0;
		for (int nibble_idx = 0; nibble_idx < 4; ++nibble_idx)
			result |= (output_nibble[nibble_idx] << (4 * nibble_idx));

		one_round[round_input] = result;
	}
}

void naomi_m4_board::device_reset()
{
	naomi_board::device_reset();
	rom_cur_address = 0;
	buffer_actual_size = 0;
	encryption = false;
	cfi_mode = false;
	counter = 0;
	iv = 0;
}

void naomi_m4_board::board_setup_address(UINT32 address, bool is_dma)
{
	rom_cur_address = address & 0x1ffffffe;
	encryption = rom_offset & 0x40000000;

	if(encryption) {
		enc_reset();
		enc_fill();
	}
}

void naomi_m4_board::board_get_buffer(UINT8 *&base, UINT32 &limit)
{
	static UINT8 retzero[2] = { 0, 0 };

	if (cfi_mode) {
		int fpr_num = 0;

		if (rombdid_tag && memregion(rombdid_tag) != nullptr)
		{
			fpr_num = *memregion(rombdid_tag)->base() & 0x7f;

		}

		if (((rom_cur_address >> 26) & 0x07) < fpr_num) {
			base = &cfidata[rom_cur_address & 0xffff];
			limit = 2;
			return;
		}
	}

	if(encryption) {
		base = buffer.get();
		limit = BUFFER_SIZE;

	} else {
		UINT32 size = m_region->bytes();
		if (rom_cur_address < size)
		{
			base = m_region->base() + rom_cur_address;
			limit = size - rom_cur_address;
		} else {
			base = retzero;
			limit = 2;
		}
	}
}

void naomi_m4_board::board_advance(UINT32 size)
{
	if(encryption) {
		if(size < buffer_actual_size) {
			memmove(buffer.get(), buffer.get() + size, buffer_actual_size - size);
			buffer_actual_size -= size;
		} else
			buffer_actual_size = 0;
		enc_fill();

	} else
		rom_cur_address += size;
}

void naomi_m4_board::enc_reset()
{
	buffer_actual_size = 0;
	iv = 0;
	counter = 0;
}

UINT16 naomi_m4_board::decrypt_one_round(UINT16 word, UINT16 subkey)
{
	return one_round[word ^ subkey] ^ subkey ;
}

void naomi_m4_board::enc_fill()
{
	const UINT8 *base = m_region->base() + rom_cur_address;
	while(buffer_actual_size < BUFFER_SIZE) {
		UINT16 enc = base[0] | (base[1] << 8);
		UINT16 dec = iv;
		iv = decrypt_one_round(enc ^ iv, subkey1);
		dec ^= decrypt_one_round(iv, subkey2);

		buffer[buffer_actual_size++] = dec;
		buffer[buffer_actual_size++] = dec >> 8;

		base += 2;
		rom_cur_address += 2;

		counter++;
		if(counter == 16) {
			counter = 0;
			iv = 0;
		}
	}
}

READ16_MEMBER(naomi_m4_board::m4_id_r)
{
	UINT16 epr_flag = 0;

	if (rombdid_tag && memregion(rombdid_tag) != nullptr)
	{
		epr_flag = *memregion(rombdid_tag)->base() & 0x80;

	}

	return 0x5500 | epr_flag;
}

void naomi_m4_board::board_write(offs_t offset, UINT16 data)
{
	if (((offset&0xffff) == 0x00aa) && (data == 0x0098))
		cfi_mode = true;
	if (((offset&0xffff) == 0x0000) && (data == 0x00f0))
		cfi_mode = false;
}
