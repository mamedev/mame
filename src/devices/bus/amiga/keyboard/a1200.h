// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    Amiga 1200 Keyboard

 ***************************************************************************/

#ifndef MAME_BUS_AMIGA_KEYBOARD_A1200_H
#define MAME_BUS_AMIGA_KEYBOARD_A1200_H

#pragma once

#include "keyboard.h"


namespace bus::amiga::keyboard {

//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

class a1200_kbd_device : public device_t, public device_amiga_keyboard_interface
{
public:
	// construction/destruction
	a1200_kbd_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	// from host
	virtual void kdat_w(int state) override;

	DECLARE_INPUT_CHANGED_MEMBER(layout_changed);

protected:
	// MPU I/O
	u8 mpu_portb_r();
	void mpu_porta_w(offs_t offset, u8 data, u8 mem_mask = ~0);
	void mpu_portb_w(offs_t offset, u8 data, u8 mem_mask = ~0);
	void mpu_portc_w(offs_t offset, u8 data, u8 mem_mask = ~0);
	void mpu_tcmp(int state);

	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset_after_children() override;

private:
	required_ioport_array<15>   m_rows;
	required_device<cpu_device> m_mpu;
	output_finder<>             m_led_kbd_caps;

	u16                         m_row_drive;
	bool                        m_host_kdat, m_mpu_kdat;
};

} // namespace bus::amiga::keyboard


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE_NS(A1200_KBD, bus::amiga::keyboard, a1200_kbd_device)

#endif // MAME_BUS_AMIGA_KEYBOARD_A1200_H
