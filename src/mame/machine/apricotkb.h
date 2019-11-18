// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Apricot keyboard emulation

*********************************************************************/

#ifndef MAME_MACHINE_APRICOTKB_H
#define MAME_MACHINE_APRICOTKB_H

#pragma once




//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define APRICOT_KEYBOARD_TAG    "aprikb"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apricot_keyboard_device

class apricot_keyboard_device :  public device_t
{
public:
	// construction/destruction
	apricot_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto txd_wr_callback() { return m_write_txd.bind(); }

	uint8_t read_keyboard();

	DECLARE_READ8_MEMBER( kb_lo_r );
	DECLARE_READ8_MEMBER( kb_hi_r );
	DECLARE_READ8_MEMBER( kb_p6_r );
	DECLARE_WRITE8_MEMBER( kb_p3_w );
	DECLARE_WRITE8_MEMBER( kb_y0_w );
	DECLARE_WRITE8_MEMBER( kb_y4_w );
	DECLARE_WRITE8_MEMBER( kb_y8_w );
	DECLARE_WRITE8_MEMBER( kb_yc_w );

	void apricot_keyboard_io(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	devcb_write_line   m_write_txd;

	required_ioport_array<13> m_y;
	required_ioport m_modifiers;

	uint16_t m_kb_y;
};


// device type definition
DECLARE_DEVICE_TYPE(APRICOT_KEYBOARD, apricot_keyboard_device)



#endif // MAME_MACHINE_APRICOTKB_H
