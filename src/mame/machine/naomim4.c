
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
// the algorithm is comprised by a 16-bits "key" and a 16-bits IV (initialization vector) --though they
// will be merged in a only 32-bits number in the code--. The hardware auto-reset the feed value
// to the cart-based IV every 16 blocks (32 bytes); that reset is not address-based, but index-based.

const device_type NAOMI_M4_BOARD = &device_creator<naomi_m4_board>;

const UINT8 naomi_m4_board::k_sboxes[4][16] = {
    {13,14,1,11,7,9,10,0,15,6,4,5,8,2,12,3},
    {12,3,14,6,7,15,2,13,1,4,11,0,9,10,8,5},
    {6,12,0,10,1,5,14,9,7,2,15,13,4,11,3,8},
    {9,12,8,7,10,4,0,15,1,11,14,2,13,5,6,3}
};

naomi_m4_board::naomi_m4_board(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: naomi_board(mconfig, NAOMI_M4_BOARD, "NAOMI-M1-BOARD", tag, owner, clock)
{
	key_tag = 0;
}

void naomi_m4_board::static_set_tags(device_t &device, const char *_key_tag)
{
	naomi_m4_board &dev = downcast<naomi_m4_board &>(device);
	dev.key_tag = _key_tag;
}

void naomi_m4_board::device_start()
{
	naomi_board::device_start();

	const UINT8 *key_data = machine().root_device().memregion(key_tag)->base();
	key = (key_data[2] << 8) | key_data[3];
	iv = (key_data[0] << 8) | key_data[1];
	buffer = auto_alloc_array(machine(), UINT8, BUFFER_SIZE);
	enc_init();

	save_pointer(NAME(buffer), BUFFER_SIZE);
	save_item(NAME(rom_cur_address));
	save_item(NAME(buffer_actual_size));
	save_item(NAME(encryption));
	save_item(NAME(counter));
}

void naomi_m4_board::enc_init()
{
	one_round = auto_alloc_array(machine(), UINT16, 0x10000);

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
	counter = 0;
	cur_iv = 0;
}

void naomi_m4_board::board_setup_address(UINT32 address, bool is_dma)
{
	rom_cur_address = address & 0x1fffffff;
	encryption = !(address & 0x20000000);

	if(encryption) {
		enc_reset();
		enc_fill();
	}
}

void naomi_m4_board::board_get_buffer(UINT8 *&base, UINT32 &limit)
{
	if(encryption) {
		base = buffer;
		limit = BUFFER_SIZE;

	} else {
		base = m_region->base() + rom_cur_address;
		limit = m_region->bytes() - rom_cur_address;
	}
}

void naomi_m4_board::board_advance(UINT32 size)
{
	if(encryption) {
		if(size < buffer_actual_size) {
			memmove(buffer, buffer + size, buffer_actual_size - size);
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
	cur_iv = iv;
	counter = 0;
}

void naomi_m4_board::enc_fill()
{
	const UINT8 *base = m_region->base() + rom_cur_address;
	while(buffer_actual_size < BUFFER_SIZE) {
		UINT16 enc = base[0] | (base[1] << 8);
		UINT16 output_whitening = key ^ cur_iv;
		cur_iv = one_round[enc ^ cur_iv];
		UINT16 dec = one_round[key ^ cur_iv] ^ output_whitening;

		buffer[buffer_actual_size++] = dec;
		buffer[buffer_actual_size++] = dec >> 8;

		base += 2;
		rom_cur_address += 2;

		counter++;
		if(counter == 16) {
			counter = 0;
			cur_iv = iv;
		}
	}
}
