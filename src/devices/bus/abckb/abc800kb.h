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
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	inline void serial_output(int state);
	inline void serial_clock();
	inline void key_down(int state);

	DECLARE_READ8_MEMBER( kb_p1_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_WRITE8_MEMBER( kb_p2_w );
	DECLARE_READ_LINE_MEMBER( kb_t1_r );

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
