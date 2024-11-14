// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    K7659 keyboard emulation

*********************************************************************/

#ifndef MAME_DDR_K7659KB_H
#define MAME_DDR_K7659KB_H

#pragma once




//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define K7659_KEYBOARD_TAG  "k7659kb"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> k7659_keyboard_device

class k7659_keyboard_device :  public device_t
{
public:
	// construction/destruction
	k7659_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(scan_keyboard);

	emu_timer *m_timer = nullptr;

private:
	required_ioport_array<12> m_keys;
	uint8_t key_pos(uint8_t val);
	uint8_t m_lookup = 0;
	uint8_t m_key = 0;
	const uint8_t *m_p_rom = nullptr;
};


// device type definition
DECLARE_DEVICE_TYPE(K7659_KEYBOARD, k7659_keyboard_device)


#endif // MAME_DDR_K7659KB_H
