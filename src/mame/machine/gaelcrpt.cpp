// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*

Gaelco video RAM encryption

Thanks to GAELCO SA for information on the algorithm.

TODO: the device must be able to know a 32-bit write was from the same
      opcode WITHOUT looking at the host program counter.

*/

#include "emu.h"
#include "gaelcrpt.h"

DEFINE_DEVICE_TYPE(GAELCO_VRAM_ENCRYPTION, gaelco_vram_encryption_device, "gaelco_vram_crypt", "Gaelco VRAM Encryption")


gaelco_vram_encryption_device::gaelco_vram_encryption_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GAELCO_VRAM_ENCRYPTION, tag, owner, clock),
	m_param1(0),
	m_param2(0)
{
}



int gaelco_vram_encryption_device::decrypt(int const enc_prev_word, int const dec_prev_word, int const enc_word)
{
	int const swap = (BIT(dec_prev_word, 8) << 1) | BIT(dec_prev_word, 7);
	int const type = (BIT(dec_prev_word,12) << 1) | BIT(dec_prev_word, 2);
	int res=0;
	int k=0;

	switch (swap)
	{
		case 0: res = bitswap<16>(enc_word,  1, 2, 0,14,12,15, 4, 8,13, 7, 3, 6,11, 5,10, 9); break;
		case 1: res = bitswap<16>(enc_word, 14,10, 4,15, 1, 6,12,11, 8, 0, 9,13, 7, 3, 5, 2); break;
		case 2: res = bitswap<16>(enc_word,  2,13,15, 1,12, 8,14, 4, 6, 0, 9, 5,10, 7, 3,11); break;
		case 3: res = bitswap<16>(enc_word,  3, 8, 1,13,14, 4,15, 0,10, 2, 7,12, 6,11, 9, 5); break;
	}

	res ^= m_param2;

	switch (type)
	{
		case 0:
			k = (0 << 0) |
				(1 << 1) |
				(0 << 2) |
				(1 << 3) |
				(1 << 4) |
				(1 << 5);
			break;

		case 1:
			k = (BIT(dec_prev_word, 0) << 0) |
				(BIT(dec_prev_word, 1) << 1) |
				(BIT(dec_prev_word, 1) << 2) |
				(BIT(enc_prev_word, 3) << 3) |
				(BIT(enc_prev_word, 8) << 4) |
				(BIT(enc_prev_word,15) << 5);
			break;

		case 2:
			k = (BIT(enc_prev_word, 5) << 0) |
				(BIT(dec_prev_word, 5) << 1) |
				(BIT(enc_prev_word, 7) << 2) |
				(BIT(enc_prev_word, 3) << 3) |
				(BIT(enc_prev_word,13) << 4) |
				(BIT(enc_prev_word,14) << 5);
			break;

		case 3:
			k = (BIT(enc_prev_word, 0) << 0) |
				(BIT(enc_prev_word, 9) << 1) |
				(BIT(enc_prev_word, 6) << 2) |
				(BIT(dec_prev_word, 4) << 3) |
				(BIT(enc_prev_word, 2) << 4) |
				(BIT(dec_prev_word,11) << 5);
			break;
	}

	k ^= m_param1;

	res = (res & 0xffc0) | ((res + k) & 0x003f);

	res ^= m_param1;

	switch (type)
	{
		case 0:
			k = (BIT(enc_word, 9) << 0) |
				(BIT(res,2)       << 1) |
				(BIT(enc_word, 5) << 2) |
				(BIT(res,5)       << 3) |
				(BIT(res,4)       << 4);
			break;

		case 1:
			k = (BIT(dec_prev_word, 2) << 0) |  // always 1
				(BIT(enc_prev_word, 4) << 1) |
				(BIT(dec_prev_word,14) << 2) |
				(BIT(res, 1)           << 3) |
				(BIT(dec_prev_word,12) << 4);   // always 0
			break;

		case 2:
			k = (BIT(enc_prev_word, 6) << 0) |
				(BIT(dec_prev_word, 6) << 1) |
				(BIT(dec_prev_word,15) << 2) |
				(BIT(res,0)            << 3) |
				(BIT(dec_prev_word, 7) << 4);
			break;

		case 3:
			k = (BIT(dec_prev_word, 2) << 0) |  // always 1
				(BIT(dec_prev_word, 9) << 1) |
				(BIT(enc_prev_word, 5) << 2) |
				(BIT(dec_prev_word, 1) << 3) |
				(BIT(enc_prev_word,10) << 4);

			break;
	}

	k ^= m_param1;

	res =   (res & 0x003f) |
			((res + (k <<  6)) & 0x07c0) |
			((res + (k << 11)) & 0xf800);

	res ^= (m_param1 << 6) | (m_param1 << 11);

	return bitswap<16>(res, 2,6,0,11,14,12,7,10,5,4,8,3,9,1,13,15);
}



uint16_t gaelco_vram_encryption_device::gaelco_decrypt(cpu_device &cpu, int offset, int data)
{
	int thispc = cpu.pc();
//  int savedata = data;

	/* check if 2nd half of 32 bit */
	if(m_lastpc == thispc && offset == m_lastoffset + 1)
	{
		m_lastpc = 0;
		data = decrypt(m_lastencword, m_lastdecword, data);
	}
	else
	{
		/* code as 1st word */

		m_lastpc = thispc;
		m_lastoffset = offset;
		m_lastencword = data;

		/* high word returned */
		data = decrypt(0, 0, data);

		m_lastdecword = data;

//      logerror("%s : data1 = %4x > %4x @ %8x\n",machine().describe_context(),savedata,data,m_lastoffset);
	}
	return data;
}

void gaelco_vram_encryption_device::device_start()
{
	save_item(NAME(m_lastpc));
	save_item(NAME(m_lastoffset));
	save_item(NAME(m_lastencword));
	save_item(NAME(m_lastdecword));
}

void gaelco_vram_encryption_device::device_reset()
{
	m_lastpc = m_lastoffset = m_lastencword = m_lastdecword = -1;
}
