// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CBM 500/600/700 High Resolution Graphics cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_CBM2_HRG_H
#define MAME_BUS_CBM2_HRG_H

#pragma once

#include "exp.h"
#include "video/ef9365.h"

#include "screen.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm2_hrg_device

class cbm2_hrg_device : public device_t,
					public device_cbm2_expansion_card_interface
{
public:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// construction/destruction
	cbm2_hrg_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, offs_t page_mask);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);
	void msl_w(uint8_t data) { m_msl = data; }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<ef9365_device> m_gdc;
	required_device<palette_device> m_palette;

private:
	void control_w(uint8_t data);
	uint8_t readback_r();

	offs_t page_offset(int bit) const { return (BIT(m_control, bit) * 0x4000) & m_page_mask; }

	required_memory_region m_bank3;

	offs_t const m_page_mask;

	std::unique_ptr<uint8_t[]> m_ram;
	uint8_t m_control;
	uint8_t m_readback;
	uint8_t m_msl;
};


// ======================> cbm2_hrg_a_device

class cbm2_hrg_a_device :  public cbm2_hrg_device
{
public:
	// construction/destruction
	cbm2_hrg_a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void hrg_a_map(address_map &map) ATTR_COLD;
};


// ======================> cbm2_hrg_b_device

class cbm2_hrg_b_device :  public cbm2_hrg_device
{
public:
	// construction/destruction
	cbm2_hrg_b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void hrg_b_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(CBM2_HRG_A, cbm2_hrg_a_device)
DECLARE_DEVICE_TYPE(CBM2_HRG_B, cbm2_hrg_b_device)


#endif // MAME_BUS_CBM2_HRG_H
