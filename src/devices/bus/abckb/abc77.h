// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-55/77 keyboard emulation

*********************************************************************/

#ifndef MAME_BUS_ABCKB_ABC77_H
#define MAME_BUS_ABCKB_ABC77_H

#pragma once

#include "abckb.h"

#include "cpu/mcs48/mcs48.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"
#include "sound/spkrdev.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc77_device

class abc77_device : public device_t,
					 public abc_keyboard_interface
{
public:
	// construction/destruction
	abc77_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER( keyboard_reset );

protected:
	abc77_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

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
	TIMER_CALLBACK_MEMBER(serial_clock);
	TIMER_CALLBACK_MEMBER(reset_tick);

	required_device<i8035_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<discrete_sound_device> m_discrete;
	required_ioport_array<12> m_x;
	required_ioport m_dsw;

	int m_keylatch;                 // keyboard row latch
	int m_clock;                    // transmit clock
	int m_stb;                      // strobe
	uint8_t m_j3;

	// timers
	emu_timer *m_serial_timer;
	emu_timer *m_reset_timer;

	uint8_t p1_r();
	void p2_w(uint8_t data);
	int t1_r() { return m_clock; }
	void prog_w(int state) { m_stb = state; }
	void j3_w(uint8_t data) { m_j3 = data; }

	void abc77_io(address_map &map) ATTR_COLD;
	void abc77_map(address_map &map) ATTR_COLD;
};


class abc55_device : public abc77_device
{
public:
	// construction/destruction
	abc55_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC77, abc77_device)
DECLARE_DEVICE_TYPE(ABC55, abc55_device)


#endif // MAME_BUS_ABCKB_ABC77_H
