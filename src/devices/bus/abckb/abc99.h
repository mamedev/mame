// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-99 keyboard and mouse emulation

*********************************************************************/

#ifndef MAME_BUS_ABCKB_ABC99_H
#define MAME_BUS_ABCKB_ABC99_H

#pragma once

#include "abckb.h"

#include "cpu/mcs48/mcs48.h"
#include "sound/spkrdev.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc99_device

class abc99_device :  public device_t,
						public abc_keyboard_interface
{
public:
	// construction/destruction
	abc99_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER( keyboard_reset );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	// abc_keyboard_interface overrides
	virtual void txd_w(int state) override;

private:
	enum
	{
		LED_1 = 0,
		LED_2,
		LED_3,
		LED_4,
		LED_5,
		LED_6,
		LED_7,
		LED_8,
		LED_INS,
		LED_ALT,
		LED_CAPS_LOCK
	};

	void serial_input();
	void serial_output(int state);
	TIMER_CALLBACK_MEMBER(serial_clock);
	TIMER_CALLBACK_MEMBER(scan_mouse);
	void key_down(int state);

	void z2_p1_w(uint8_t data);
	uint8_t z2_p2_r();
	DECLARE_READ_LINE_MEMBER( z2_t0_r );
	DECLARE_READ_LINE_MEMBER( z2_t1_r );

	void z2_led_w(uint8_t data);
	uint8_t z5_p1_r();
	void z5_p2_w(uint8_t data);
	uint8_t z5_t1_r();

	void abc99_z2_io(address_map &map);
	void abc99_z2_mem(address_map &map);
	void abc99_z5_mem(address_map &map);

	emu_timer *m_serial_timer;
	emu_timer *m_mouse_timer;

	required_device<i8035_device> m_maincpu;
	required_device<i8035_device> m_mousecpu;
	required_device<speaker_sound_device> m_speaker;
	required_ioport m_z14;
	required_ioport m_mouseb;
	output_finder<11> m_leds;

	int m_si;
	int m_si_en;
	int m_so_z2;
	int m_so_z5;
	int m_keydown;
	int m_t1_z2;
	int m_t1_z5;
	int m_led_en;
	int m_reset;
	int m_txd;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC99, abc99_device)


#endif // MAME_BUS_ABCKB_ABC99_H
