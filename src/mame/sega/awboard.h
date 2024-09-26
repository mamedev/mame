// license:BSD-3-Clause
// copyright-holders:Olivier Galibert,Andreas Naive
#ifndef MAME_SEGA_AWBOARD_H
#define MAME_SEGA_AWBOARD_H

#pragma once

#include "naomig1.h"


class aw_rom_board : public naomi_g1_device
{
public:
	aw_rom_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void submap(address_map &map) override ATTR_COLD;

	void epr_offsetl_w(uint16_t data);          // 5f7000
	void epr_offseth_w(uint16_t data);          // 5f7004
	void mpr_record_index_w(uint16_t data);     // 5f700c
	void mpr_first_file_index_w(uint16_t data); // 5f7010
	void mpr_file_offsetl_w(uint16_t data);     // 5f7014
	void mpr_file_offseth_w(uint16_t data);     // 5f7018
	uint16_t pio_r();                           // 5f7080
	void pio_w(uint16_t data);                  // 5f7080

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void dma_get_position(uint8_t *&base, uint32_t &limit, bool to_mainram) override;
	virtual void dma_advance(uint32_t size) override;

private:
	enum { EPR, MPR_RECORD, MPR_FILE };

	required_memory_region m_region;
	uint8_t  rombd_key;
	uint32_t mpr_offset, mpr_bank;
	uint32_t epr_offset, mpr_file_offset;
	uint16_t mpr_record_index, mpr_first_file_index;
	uint16_t decrypted_buf[16];

	uint32_t dma_offset, dma_limit;

	struct sbox_set {
		uint8_t S0[32];
		uint8_t S1[16];
		uint8_t S2[16];
		uint8_t S3[8];
	};

	static const int permutation_table[4][16];
	static const sbox_set sboxes_table[4];
	static const int xor_table[16];
	static uint16_t decrypt(uint16_t cipherText, uint32_t address, const uint8_t key);
	uint16_t decrypt16(uint32_t address) { return decrypt(m_region->as_u16(address), address, rombd_key); }

	void recalc_dma_offset(int mode);
};

DECLARE_DEVICE_TYPE(AW_ROM_BOARD, aw_rom_board)

#endif // MAME_SEGA_AWBOARD_H
