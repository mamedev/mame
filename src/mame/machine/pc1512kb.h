// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Amstrad PC1512 Keyboard emulation

**********************************************************************/

#ifndef MAME_MACHINE_PC1512KB_H
#define MAME_MACHINE_PC1512KB_H

#pragma once


#include "bus/vcs_ctrl/ctrl.h"
#include "cpu/mcs48/mcs48.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define PC1512_KEYBOARD_TAG "kb"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc1512_keyboard_device

class pc1512_keyboard_device :  public device_t
{
public:
	// construction/destruction
	pc1512_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto clock_wr_callback() { return m_write_clock.bind(); }
	auto data_wr_callback() { return m_write_data.bind(); }

	DECLARE_WRITE_LINE_MEMBER( data_w );
	DECLARE_WRITE_LINE_MEMBER( clock_w );
	DECLARE_WRITE_LINE_MEMBER( m1_w );
	DECLARE_WRITE_LINE_MEMBER( m2_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	enum
	{
		LED_CAPS = 0,
		LED_NUM
	};

	required_device<i8048_device> m_maincpu;
	required_device<vcs_control_port_device> m_joy;

	required_ioport_array<11> m_y;
	output_finder<2> m_leds;

	devcb_write_line   m_write_clock;
	devcb_write_line   m_write_data;

	int m_data_in;
	int m_clock_in;
	int m_kb_y;
	int m_joy_com;
	int m_m1;
	int m_m2;

	emu_timer *m_reset_timer;

	uint8_t kb_bus_r();
	void kb_p1_w(uint8_t data);
	uint8_t kb_p2_r();
	void kb_p2_w(uint8_t data);
	DECLARE_READ_LINE_MEMBER( kb_t0_r );
	DECLARE_READ_LINE_MEMBER( kb_t1_r );
};


// device type definition
DECLARE_DEVICE_TYPE(PC1512_KEYBOARD, pc1512_keyboard_device)


#endif // MAME_MACHINE_PC1512KB_H
