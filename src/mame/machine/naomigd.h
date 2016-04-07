// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef _NAOMIGD_H_
#define _NAOMIGD_H_

#include "machine/naomibd.h"

#define MCFG_NAOMI_GDROM_BOARD_ADD(_tag, _image_tag, _pic_tag, _eeprom_tag, _irq_cb) \
	MCFG_NAOMI_BOARD_ADD(_tag, NAOMI_GDROM_BOARD, _eeprom_tag, NULL, _irq_cb) \
	naomi_gdrom_board::static_set_tags(*device, "^" _image_tag, "^" _pic_tag);

class naomi_gdrom_board : public naomi_board
{
public:
	naomi_gdrom_board(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void static_set_tags(device_t &device, const char *_image_tag, const char *_pic_tag);

	UINT8 *memory(UINT32 &size) { size = dimm_data_size; return dimm_data; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void board_setup_address(UINT32 address, bool is_dma) override;
	virtual void board_get_buffer(UINT8 *&base, UINT32 &limit) override;
	virtual void board_advance(UINT32 size) override;

private:
	enum { FILENAME_LENGTH=24 };

	const char *image_tag, *pic_tag;

	UINT32 dimm_cur_address;

	// Note: voluntarily not saved into the state
	UINT8 *dimm_data;
	UINT32 dimm_data_size;

	static const UINT32 DES_LEFTSWAP[];
	static const UINT32 DES_RIGHTSWAP[];
	static const UINT32 DES_SBOX1[];
	static const UINT32 DES_SBOX2[];
	static const UINT32 DES_SBOX3[];
	static const UINT32 DES_SBOX4[];
	static const UINT32 DES_SBOX5[];
	static const UINT32 DES_SBOX6[];
	static const UINT32 DES_SBOX7[];
	static const UINT32 DES_SBOX8[];
	static const UINT32 DES_MASK_TABLE[];
	static const UINT8 DES_ROTATE_TABLE[16];

	void find_file(const char *name, const UINT8 *dir_sector, UINT32 &file_start, UINT32 &file_size);

	inline void permutate(UINT32 &a, UINT32 &b, UINT32 m, int shift);
	void des_generate_subkeys(const UINT64 key, UINT32 *subkeys);
	UINT64 des_encrypt_decrypt(bool decrypt, UINT64 src, const UINT32 *des_subkeys);
	UINT64 rev64(UINT64 src);
	UINT64 read_to_qword(const UINT8 *region);
	void write_from_qword(UINT8 *region, UINT64 qword);
};

extern const device_type NAOMI_GDROM_BOARD;

#endif
