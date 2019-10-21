// license:BSD-3-Clause
// copyright-holders:windyfairy
#include "emu.h"
#include "speaker.h"

#include "k573fpga.h"


k573fpga_device::k573fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, KONAMI_573_DIGITAL_FPGA, tag, owner, clock),
	use_ddrsbm_fpga(false)
{
}

void k573fpga_device::device_start()
{
}

void k573fpga_device::device_reset()
{
	mp3_cur_adr = 0;
	mp3_end_adr = 0;
	crypto_key1 = 0;
	crypto_key2 = 0;
	crypto_key3 = 0;
}

u16 k573fpga_device::get_mpeg_ctrl()
{
	if ((mpeg_ctrl_flag & 0xe000) == 0xe000) {
		// This has been tested with real hardware, but this flag is always held 0x1000 when the audio is being played
		return 0x1000;
	}

	return 0x0000;
}

void k573fpga_device::set_mpeg_ctrl(u16 data)
{
	logerror("FPGA MPEG control %c%c%c | %08x %08x\n",
				data & 0x8000 ? '#' : '.',
				data & 0x4000 ? '#' : '.',
				data & 0x2000 ? '#' : '.',
				mp3_cur_adr, mp3_end_adr);

	mpeg_ctrl_flag = data;
}

u16 k573fpga_device::decrypt_default(u16 v)
{
	u16 m = crypto_key1 ^ crypto_key2;

	v = bitswap<16>(
		v,
		15 - BIT(m, 0xF),
		14 + BIT(m, 0xF),
		13 - BIT(m, 0xE),
		12 + BIT(m, 0xE),
		11 - BIT(m, 0xB),
		10 + BIT(m, 0xB),
		9 - BIT(m, 0x9),
		8 + BIT(m, 0x9),
		7 - BIT(m, 0x8),
		6 + BIT(m, 0x8),
		5 - BIT(m, 0x5),
		4 + BIT(m, 0x5),
		3 - BIT(m, 0x3),
		2 + BIT(m, 0x3),
		1 - BIT(m, 0x2),
		0 + BIT(m, 0x2)
	);

	v ^= (BIT(m, 0xD) << 14) ^
		(BIT(m, 0xC) << 12) ^
		(BIT(m, 0xA) << 10) ^
		(BIT(m, 0x7) << 8) ^
		(BIT(m, 0x6) << 6) ^
		(BIT(m, 0x4) << 4) ^
		(BIT(m, 0x1) << 2) ^
		(BIT(m, 0x0) << 0);

	v ^= bitswap<16>(
		(u16)crypto_key3,
		7, 0, 6, 1,
		5, 2, 4, 3,
		3, 4, 2, 5,
		1, 6, 0, 7
	);

	crypto_key1 = (crypto_key1 & 0x8000) | ((crypto_key1 << 1) & 0x7FFE) | ((crypto_key1 >> 14) & 1);

	if(((crypto_key1 >> 15) ^ crypto_key1) & 1)
		crypto_key2 = (crypto_key2 << 1) | (crypto_key2 >> 15);

	crypto_key3++;

	return v;
}

u16 k573fpga_device::decrypt_ddrsbm(u16 data)
{
	u8 key[16] = {0};
	u16 key_state = bitswap<16>(
		crypto_key1,
		13, 11, 9, 7,
		5, 3, 1, 15,
		14, 12, 10, 8,
		6, 4, 2, 0
	);

	for(int i = 0; i < 8; i++) {
		key[i * 2] = key_state & 0xff;
		key[i * 2 + 1] = (key_state >> 8) & 0xff;
		key_state = ((key_state & 0x8080) >> 7) | ((key_state & 0x7f7f) << 1);
	}

	u16 output_word = 0;
	for(int cur_bit = 0; cur_bit < 8; cur_bit++) {
		int even_bit_shift = cur_bit * 2;
		int odd_bit_shift = cur_bit * 2 + 1;
		bool is_even_bit_set = data & (1 << even_bit_shift);
		bool is_odd_bit_set = data & (1 << odd_bit_shift);
		bool is_key_bit_set = key[crypto_key3 & 15] & (1 << cur_bit);
		bool is_scramble_bit_set = key[(crypto_key3 - 1) & 15] & (1 << cur_bit);

		if(is_scramble_bit_set)
			std::swap(is_even_bit_set, is_odd_bit_set);

		if(is_even_bit_set ^ is_key_bit_set)
			output_word |= 1 << even_bit_shift;

		if(is_odd_bit_set)
			output_word |= 1 << odd_bit_shift;
	}

	crypto_key3++;

	return output_word;
}

u16 k573fpga_device::get_decrypted()
{
	if(mp3_cur_adr >= mp3_end_adr || (mpeg_ctrl_flag & 0xe000) != 0xe000) {
		return 0;
	}

	u16 src = ram[mp3_cur_adr >> 1];
	u16 result = use_ddrsbm_fpga ? decrypt_ddrsbm(src) : decrypt_default(src);
	mp3_cur_adr += 2;

	return result;
}

DEFINE_DEVICE_TYPE(KONAMI_573_DIGITAL_FPGA, k573fpga_device, "k573fpga", "Konami 573 Digital I/O FPGA")
