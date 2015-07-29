// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

void sega_decode_2(UINT8 *rom, UINT8 *decrypted,
			const UINT8 xor_table[128],const int swap_table[128]);

void sega_decode_317(UINT8 *rom, UINT8 *decrypted, int shift);
