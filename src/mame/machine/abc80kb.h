// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-80 keyboard emulation

**********************************************************************/
#ifndef MAME_MACHINE_ABC80KB_H
#define MAME_MACHINE_ABC80KB_H

#pragma once



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define ABC80_KEYBOARD_TAG  "abc80kb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ABC80_KEYBOARD_KEYDOWN_CALLBACK(_write) \
	devcb = &downcast<abc80_keyboard_device &>(*device).set_keydown_wr_callback(DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc80_keyboard_device

class abc80_keyboard_device :  public device_t
{
public:
	// construction/destruction
	abc80_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_keydown_wr_callback(Object &&cb) { return m_write_keydown.set_callback(std::forward<Object>(cb)); }

	uint8_t data_r();

	void abc80_keyboard_io(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	devcb_write_line m_write_keydown;

	required_device<cpu_device> m_maincpu;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC80_KEYBOARD, abc80_keyboard_device)



#endif // MAME_MACHINE_ABC80KB_H
