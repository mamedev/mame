// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-80 keyboard emulation

**********************************************************************/
#ifndef MAME_LUXOR_ABC80KB_H
#define MAME_LUXOR_ABC80KB_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc80_keyboard_device

class abc80_keyboard_device :  public device_t
{
public:
	// construction/destruction
	abc80_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto keydown_wr_callback() { return m_write_keydown.bind(); }

	uint8_t data_r();

	void keyboard_mem(address_map &map) ATTR_COLD;
	void keyboard_io(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	devcb_write_line m_write_keydown;

	required_device<cpu_device> m_maincpu;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC80_KEYBOARD, abc80_keyboard_device)



#endif // MAME_LUXOR_ABC80KB_H
