// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_NAOMIBD_H
#define MAME_MACHINE_NAOMIBD_H

#pragma once

#include "machine/naomig1.h"
#include "machine/x76f100.h"

class naomi_board : public naomi_g1_device
{
public:
	// Can be patched in the underlying class
	virtual void submap(address_map &map) override;

	DECLARE_WRITE16_MEMBER(rom_offseth_w);          // 5f7000
	DECLARE_WRITE16_MEMBER(rom_offsetl_w);          // 5f7004
	DECLARE_READ16_MEMBER( rom_data_r);             // 5f7008
	DECLARE_WRITE16_MEMBER(rom_data_w);             // 5f7008
	DECLARE_WRITE16_MEMBER(dma_offseth_w);          // 5f700c
	DECLARE_WRITE16_MEMBER(dma_offsetl_w);          // 5f7010
	DECLARE_WRITE16_MEMBER(dma_count_w);            // 5f7014

	DECLARE_WRITE16_MEMBER(boardid_w);              // 5f7078
	DECLARE_READ16_MEMBER( boardid_r);              // 5f707c

	DECLARE_READ16_MEMBER( default_r);

protected:
	naomi_board(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void dma_get_position(uint8_t *&base, uint32_t &limit, bool to_mainram) override;
	virtual void dma_advance(uint32_t size) override;

	// To be defined in the underlying class
	virtual void board_setup_address(uint32_t address, bool is_dma) = 0;
	virtual void board_get_buffer(uint8_t *&base, uint32_t &limit) = 0;
	virtual void board_advance(uint32_t size) = 0;

	// To be optionally defined in the underlying class
	virtual void board_write(offs_t offset, uint16_t data);

	uint32_t rom_offset;
	optional_device<x76f100_device> eeprom;

private:
	uint32_t dma_offset, dma_cur_offset;
	uint16_t dma_count;
	bool pio_ready, dma_ready;
};

#endif // MAME_MACHINE_NAOMIBD_H
