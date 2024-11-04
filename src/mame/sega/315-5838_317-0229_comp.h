// license:BSD-3-Clause
// copyright-holders:David Haywood, Samuel Neves, Peter Wilhelmsen, Morten Shearman Kirkegaard
#ifndef MAME_SEGA_315_5838_371_0229_COMP_H
#define MAME_SEGA_315_5838_371_0229_COMP_H

#pragma once

#include "dirom.h"

// #define SEGA315_DUMP_DEBUG // dump stuff to files to help with decryption efforts

DECLARE_DEVICE_TYPE(SEGA315_5838_COMP, sega_315_5838_comp_device)

class sega_315_5838_comp_device :  public device_t,
								   public device_rom_interface<23>
{
public:
	// construction/destruction
	sega_315_5838_comp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t data_r();

	void data_w_doa(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void data_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void srcaddr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void debug_helper(int id);

	enum
	{
		HACK_MODE_NONE = 0,
		HACK_MODE_NO_KEY,
		HACK_MODE_DOA
	};

	void set_hack_mode(int mode) { m_hackmode = mode; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t source_word_r();

	void write_prot_data(uint32_t data, uint32_t mem_mask, int rev_words);
	void set_prot_addr(uint32_t data, uint32_t mem_mask);

	uint32_t m_srcoffset = 0;
	uint32_t m_srcstart = 0; // failsafe
	bool m_abort = false;

	struct {
		uint16_t mode = 0;
		struct {
			uint8_t len = 0;       /* in bits */
			uint8_t idx = 0;       /* in the dictionary */
			uint16_t pattern = 0;  /* of the first node */
		} tree[13];
		int it2 = 0;
		uint8_t dictionary[256]{};
		int id = 0;
	} m_compstate;

	void set_table_upload_mode_w(uint16_t val);
	void upload_table_data_w(uint16_t val);

	uint8_t get_decompressed_byte(void);
	uint16_t decipher(uint16_t c);

	int m_num_bits_compressed = 0;
	uint16_t m_val_compressed = 0;
	int m_num_bits = 0;
	uint16_t m_val = 0;

	int m_hackmode;
#ifdef SEGA315_DUMP_DEBUG
	FILE* m_fp;
#endif
};

#endif // MAME_SEGA_315_5838_371_0229_COMP_H
