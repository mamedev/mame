// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-99 keyboard emulation

*********************************************************************/

#ifndef MAME_BUS_ABCKB_ABC99_H
#define MAME_BUS_ABCKB_ABC99_H

#pragma once

#include "abckb.h"

#include "bus/abckb/r8.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/watchdog.h"
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

	ioport_value cursor_x4_r();
	ioport_value cursor_x6_r();

	DECLARE_INPUT_CHANGED_MEMBER( keyboard_reset );

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// abc_keyboard_interface overrides
	virtual void txd_w(int state) override;
	virtual void reset_w(int state) override;

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
	TIMER_CALLBACK_MEMBER(serial_clock);

	uint8_t key_y_r();
	void key_x_w(offs_t offset, uint8_t data);
	void z2_p1_w(uint8_t data);
	uint8_t z2_p2_r();
	int z2_t1_r() { return m_t1_z2; }

	void led_w(uint8_t data);
	uint8_t z5_p1_r();
	void z5_p2_w(uint8_t data);
	int z5_t1_r() { return m_t1_z5; }

	void keyboard_io(address_map &map) ATTR_COLD;
	void keyboard_mem(address_map &map) ATTR_COLD;
	void mouse_mem(address_map &map) ATTR_COLD;

	emu_timer *m_serial_timer;

	required_device<i8035_device> m_maincpu;
	required_device<i8035_device> m_mousecpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<speaker_sound_device> m_speaker;
	required_device<luxor_r8_device> m_mouse;
	required_ioport_array<16> m_x;
	required_ioport m_z14;
	required_ioport m_cursor;
	output_finder<11> m_leds;

	int m_keylatch;
	int m_si;
	int m_si_en;
	int m_so_z2;
	int m_so_z5;
	int m_t1_z2;
	int m_t1_z5;
	int m_led_en;
	int m_reset;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC99, abc99_device)


#endif // MAME_BUS_ABCKB_ABC99_H
