// license:BSD-3-Clause
// copyright-holders:R. Belmont, M. Burke
#ifndef MAME_DEC_LK201_H
#define MAME_DEC_LK201_H

#pragma once

#include "cpu/m6805/m68hc05.h"
#include "sound/beep.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> lk201_device

class lk201_device : public device_t
{
public:
	// construction/destruction
	lk201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// serial line to and from the host, through the MCU's SCI
	auto tx_handler() { return m_maincpu.lookup()->sci_tdo(); }
	void rx_w(int state) { m_maincpu->sci_rdi_w(state); }

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	template <u32 N> void rows_w(offs_t offset, u8 data, u8 mask);
	void portc_w(offs_t offset, u8 data, u8 mask);
	void sck_w(int state);
	void mosi_w(int state) { m_mosi = state; }
	u8 portd_r() { return 0x20 | (BIT(m_kbd_sr, 7) << 2); } // PD2 is MISO; PD5 is /SS, inferred high (the firmware never reads port D)

	u8 scan_matrix();
	void update_matrix_load();
	void update_latch();

	required_device<m68hc05c4_device> m_maincpu;
	required_device<beep_device> m_speaker;

	required_ioport_array<18> m_kbd;

	output_finder<> m_led_wait;
	output_finder<> m_led_compose;
	output_finder<> m_led_lock;
	output_finder<> m_led_hold;

	u8 m_rows[3];           // key matrix row drivers on ports A, B and C (bits 0 and 1 only)
	bool m_pc6;             // matrix load, active low
	bool m_pc7;             // LED and bell volume latch strobe
	bool m_sck;
	u8 m_mosi;
	u8 m_kbd_sr;            // matrix column input shift register
	u8 m_led_sr;            // LED and bell volume output shift register
};

// device type definition
DECLARE_DEVICE_TYPE(LK201, lk201_device)

#endif // MAME_DEC_LK201_H
