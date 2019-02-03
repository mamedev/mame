// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    K7659 keyboard emulation

*********************************************************************/

#ifndef MAME_MACHINE_K7659KB_H
#define MAME_MACHINE_K7659KB_H

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

	DECLARE_READ8_MEMBER(read);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	emu_timer *m_timer;

private:
	uint8_t key_pos(uint8_t val);
	uint8_t m_lookup;
	uint8_t m_key;
	const uint8_t *m_p_rom;
};


// device type definition
DECLARE_DEVICE_TYPE(K7659_KEYBOARD, k7659_keyboard_device)


#endif // MAME_MACHINE_K7659KB_H
