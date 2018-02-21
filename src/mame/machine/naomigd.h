// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_NAOMIGD_H
#define MAME_MACHINE_NAOMIGD_H

#pragma once

#include "machine/naomibd.h"

#define MCFG_NAOMI_GDROM_BOARD_ADD(_tag, _image_tag, _pic_tag, _eeprom_tag, _irq_cb) \
	MCFG_NAOMI_BOARD_ADD(_tag, NAOMI_GDROM_BOARD, _eeprom_tag, _irq_cb) \
	downcast<naomi_gdrom_board &>(*device).set_tags(_image_tag, _pic_tag);

class naomi_gdrom_board : public naomi_board
{
public:
	naomi_gdrom_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_tags(const char *_image_tag, const char *_pic_tag)
	{
		image_tag = _image_tag;
		pic_tag = _pic_tag;
	}

	uint8_t *memory(uint32_t &size) { size = dimm_data_size; return dimm_data; }

	virtual const tiny_rom_entry *device_rom_region() const override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void board_setup_address(uint32_t address, bool is_dma) override;
	virtual void board_get_buffer(uint8_t *&base, uint32_t &limit) override;
	virtual void board_advance(uint32_t size) override;

private:
	enum { FILENAME_LENGTH=24 };

	const char *image_tag, *pic_tag;

	uint32_t dimm_cur_address;

	// Note: voluntarily not saved into the state
	uint8_t *dimm_data;
	uint32_t dimm_data_size;

	static const uint32_t DES_LEFTSWAP[];
	static const uint32_t DES_RIGHTSWAP[];
	static const uint32_t DES_SBOX1[];
	static const uint32_t DES_SBOX2[];
	static const uint32_t DES_SBOX3[];
	static const uint32_t DES_SBOX4[];
	static const uint32_t DES_SBOX5[];
	static const uint32_t DES_SBOX6[];
	static const uint32_t DES_SBOX7[];
	static const uint32_t DES_SBOX8[];
	static const uint32_t DES_MASK_TABLE[];
	static const uint8_t DES_ROTATE_TABLE[16];

	void find_file(const char *name, const uint8_t *dir_sector, uint32_t &file_start, uint32_t &file_size);

	inline void permutate(uint32_t &a, uint32_t &b, uint32_t m, int shift);
	void des_generate_subkeys(const uint64_t key, uint32_t *subkeys);
	uint64_t des_encrypt_decrypt(bool decrypt, uint64_t src, const uint32_t *des_subkeys);
	uint64_t rev64(uint64_t src);
	uint64_t read_to_qword(const uint8_t *region);
	void write_from_qword(uint8_t *region, uint64_t qword);
};

DECLARE_DEVICE_TYPE(NAOMI_GDROM_BOARD, naomi_gdrom_board)

#endif // MAME_MACHINE_NAOMIGD_H
