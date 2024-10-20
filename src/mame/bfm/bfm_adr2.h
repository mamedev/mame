// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_BFM_BFM_ADR2_H
#define MAME_BFM_BFM_ADR2_H

#pragma once

#include "tilemap.h"


class bfm_adder2_device : public device_t, public device_gfx_interface
{
public:
	// construction/destruction
	bfm_adder2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void vid_uart_tx_w(uint8_t data);
	void vid_uart_ctrl_w(uint8_t data);
	uint8_t vid_uart_rx_r();
	uint8_t vid_uart_ctrl_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<cpu_device> m_cpu;
	required_shared_ptr_array<uint8_t, 2> m_screen_ram; // paged display RAM
	memory_view m_screen_ram_view;
	required_memory_bank m_rombank;

	uint8_t m_screen_page_reg = 0;              // access/display select
	uint8_t m_c101 = 0;
	bool m_vbl_triggered = 0;               // flag, VBL IRQ triggered
	bool m_acia_triggered = 0;              // flag, ACIA receive IRQ

	tilemap_t *m_tilemap[2]{};

	bool m_data_from_sc2 = false;
	bool m_data_to_sc2 = false;

	uint8_t m_data = 0;
	uint8_t m_sc2data = 0;

	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);

	template <uint8_t Which> void screen_ram_w(offs_t offset, uint8_t data);
	void rom_page_w(uint8_t data);
	void c001_w(uint8_t data);
	void screen_page_w(uint8_t data);
	uint8_t vbl_ctrl_r();
	void vbl_ctrl_w(uint8_t data);
	uint8_t uart_ctrl_r();
	void uart_ctrl_w(uint8_t data);
	uint8_t uart_rx_r();
	void uart_tx_w(uint8_t data);
	uint8_t irq_r();

	void decode_char_roms();

	void prg_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vbl_w(int state);
};

// device type definition
DECLARE_DEVICE_TYPE(BFM_ADDER2, bfm_adder2_device)


#endif // MAME_BFM_BFM_ADR2_H
