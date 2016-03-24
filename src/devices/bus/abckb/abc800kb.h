// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-800 keyboard emulation

**********************************************************************/

#pragma once

#ifndef __ABC800_KEYBOARD__
#define __ABC800_KEYBOARD__


#include "emu.h"
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
	abc800_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	// abc_keyboard_interface overrides
	virtual void txd_w(int state) override;

	// not really public
	DECLARE_READ8_MEMBER( kb_p1_r );
	DECLARE_WRITE8_MEMBER( kb_p1_w );
	DECLARE_WRITE8_MEMBER( kb_p2_w );
	DECLARE_READ8_MEMBER( kb_t1_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	inline void serial_output(int state);
	inline void serial_clock();
	inline void key_down(int state);

	required_device<cpu_device> m_maincpu;
	required_ioport m_x0;
	required_ioport m_x1;
	required_ioport m_x2;
	required_ioport m_x3;
	required_ioport m_x4;
	required_ioport m_x5;
	required_ioport m_x6;
	required_ioport m_x7;
	required_ioport m_x8;
	required_ioport m_x9;
	required_ioport m_x10;
	required_ioport m_x11;

	int m_row;
	int m_txd;
	int m_clk;
	int m_stb;
	int m_keydown;

	// timers
	emu_timer *m_serial_timer;
};


// device type definition
extern const device_type ABC800_KEYBOARD;



#endif
