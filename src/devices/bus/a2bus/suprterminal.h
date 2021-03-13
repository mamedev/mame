// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    suprterminal.h

    M&R Enterprises SUP'R'TERMINAL 80-column card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_SUPRTERMINAL_H
#define MAME_BUS_A2BUS_SUPRTERMINAL_H

#pragma once

#include "a2bus.h"
#include "video/mc6845.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_suprterminal_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_suprterminal_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	a2bus_suprterminal_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// overrides of standard a2bus slot functions
	virtual u8 read_cnxx(u8 offset) override;
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_c800(u16 offset) override;
	virtual void write_c800(u16 offset, u8 data) override;
	virtual bool take_c800() override { return true; }

private:
	required_device<mc6845_device> m_crtc;
	required_region_ptr<u8> m_rom;

	MC6845_UPDATE_ROW(crtc_update_row);

	std::unique_ptr<u8[]> m_vram;
	std::unique_ptr<u8[]> m_fontram;
//  u8 m_fontram[0x400];
	bool m_bRasterRAM, m_bCharBank1, m_bC800IsRAM;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_SUPRTERMINAL, a2bus_suprterminal_device)

#endif  // MAME_BUS_A2BUS_SUPRTERMINAL_H
