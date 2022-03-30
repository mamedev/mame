// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_BFM_ADR2_H
#define MAME_INCLUDES_BFM_ADR2_H

#pragma once

#include "tilemap.h"


class bfm_adder2_device : public device_t, public device_gfx_interface
{
public:
	// construction/destruction
	bfm_adder2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	TILE_GET_INFO_MEMBER( get_tile0_info );
	TILE_GET_INFO_MEMBER( get_tile1_info );

	uint8_t screen_ram_r(offs_t offset);
	void screen_ram_w(offs_t offset, uint8_t data);
	uint8_t normal_ram_r(offs_t offset);
	void normal_ram_w(offs_t offset, uint8_t data);
	void adder2_rom_page_w(uint8_t data);
	void adder2_c001_w(uint8_t data);
	void adder2_screen_page_w(uint8_t data);
	uint8_t adder2_vbl_ctrl_r();
	void adder2_vbl_ctrl_w(uint8_t data);
	uint8_t adder2_uart_ctrl_r();
	void adder2_uart_ctrl_w(uint8_t data);
	uint8_t adder2_uart_rx_r();
	void adder2_uart_tx_w(uint8_t data);
	uint8_t adder2_irq_r();

	void vid_uart_tx_w(uint8_t data);
	void vid_uart_ctrl_w(uint8_t data);
	uint8_t vid_uart_rx_r();
	uint8_t vid_uart_ctrl_r();

	void adder2_decode_char_roms();

	void adder2_memmap(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	int m_screen_page_reg = 0;              // access/display select
	int m_c101 = 0;
	int m_rx = 0;
	bool m_vbl_triggered = 0;               // flag, VBL IRQ triggered
	bool m_acia_triggered = 0;              // flag, ACIA receive IRQ

	uint8_t m_adder_ram[0xE80]{};         // normal RAM
	uint8_t m_screen_ram[2][0x1180]{};    // paged  display RAM

	tilemap_t *m_tilemap0 = nullptr;  // tilemap screen0
	tilemap_t *m_tilemap1 = nullptr;  // tilemap screen1

	bool m_data_from_sc2 = false;
	bool m_data_to_sc2 = false;

	uint8_t m_adder2_data = 0;
	uint8_t m_sc2data = 0;

	optional_device<cpu_device> m_cpu;

	uint32_t update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(adder2_vbl_w);
};

// device type definition
DECLARE_DEVICE_TYPE(BFM_ADDER2, bfm_adder2_device)


#endif // MAME_INCLUDES_BFM_ADR2_H
