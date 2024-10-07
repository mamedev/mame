// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_SEGA_NAOMIBD_H
#define MAME_SEGA_NAOMIBD_H

#pragma once

#include "naomig1.h"
#include "machine/x76f100.h"

class naomi_board : public naomi_g1_device
{
public:
	// Can be patched in the underlying class
	virtual void submap(address_map &map) override ATTR_COLD;

	void rom_offseth_w(uint16_t data);          // 5f7000
	void rom_offsetl_w(uint16_t data);          // 5f7004
	uint16_t rom_data_r();                      // 5f7008
	void rom_data_w(uint16_t data);             // 5f7008
	void dma_offseth_w(uint16_t data);          // 5f700c
	void dma_offsetl_w(uint16_t data);          // 5f7010
	void dma_count_w(uint16_t data);            // 5f7014

	void boardid_w(uint16_t data);              // 5f7078
	uint16_t boardid_r();                       // 5f707c

	uint16_t default_r(offs_t offset);

protected:
	naomi_board(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void dma_get_position(uint8_t *&base, uint32_t &limit, bool to_mainram) override;
	virtual void dma_advance(uint32_t size) override;

	// To be defined in the underlying class
	virtual void board_setup_address(uint32_t address, bool is_dma) = 0;
	virtual void board_get_buffer(uint8_t *&base, uint32_t &limit) = 0;
	virtual void board_advance(uint32_t size) = 0;

	// To be optionally defined in the underlying class
	virtual void board_write(offs_t offset, uint16_t data);

	uint32_t rom_offset = 0;
	optional_device<x76f100_device> eeprom;

private:
	uint32_t dma_offset = 0, dma_cur_offset = 0;
	uint16_t dma_count = 0;
	bool pio_ready = false, dma_ready = false;
};

#endif // MAME_SEGA_NAOMIBD_H
