// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-800 keyboard emulation

**********************************************************************/

#ifndef MAME_BUS_ABCKB_ABC800KB_H
#define MAME_BUS_ABCKB_ABC800KB_H

#pragma once


#include "cpu/mcs48/mcs48.h"
#include "abckb.h"
#include "sound/discrete.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc800_keyboard_device

class abc800_keyboard_device :  public device_t,
								public abc_keyboard_interface
{
public:
	// construction/destruction
	abc800_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// abc_keyboard_interface overrides
	virtual void txd_w(int state) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void serial_output(int state);
	TIMER_CALLBACK_MEMBER(serial_clock);
	void key_down(int state);

	uint8_t kb_p1_r();
	void kb_p1_w(uint8_t data);
	void kb_p2_w(uint8_t data);
	int kb_t1_r();

	required_device<i8048_device> m_maincpu;
	required_ioport_array<12> m_x;

	int m_row;
	int m_txd;
	int m_clk;
	int m_stb;
	int m_keydown;

	// timers
	emu_timer *m_serial_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC800_KEYBOARD, abc800_keyboard_device)

#endif // MAME_BUS_ABCKB_ABC800KB_H
