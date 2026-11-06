// license:BSD-3-Clause
// copyright-holders:R. Belmont, M. Burke
#ifndef MAME_DEC_LK201_H
#define MAME_DEC_LK201_H

#pragma once

#include "cpu/m6805/m68hc05.h"
#include "sound/beep.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LK_CMD_LEDS_ON          0x13    /* light LEDs - 1st param: led bitmask */
#define LK_CMD_LEDS_OFF         0x11    /* turn off LEDs */

#define LK_CMD_DIS_KEYCLK       0x99    /* disable the keyclick */
#define LK_CMD_ENB_KEYCLK       0x1b    /* enable the keyclick - 1st param: volume */
#define LK_CMD_DIS_CTLCLK       0xb9    /* disable the Ctrl keyclick */
#define LK_CMD_ENB_CTLCLK       0xbb    /* enable the Ctrl keyclick */
#define LK_CMD_SOUND_CLK        0x9f    /* emit a keyclick  - 1st param: volume */
#define LK_CMD_DIS_BELL         0xa1    /* disable the bell */
#define LK_CMD_ENB_BELL         0x23    /* enable the bell - 1st param: volume */
#define LK_CMD_BELL             0xa7    /* emit a bell - 1st param: volume */

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
	auto tx_handler() { return m_tx_handler.bind(); }
	void rx_w(int state) { m_maincpu->rdi_w(state); }

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void rows_w(unsigned port, uint8_t data, uint8_t mask);
	void portc_w(uint8_t data, uint8_t mask);
	void sck_w(int state);
	void mosi_w(int state) { m_mosi = state; }
	int miso_r() { return BIT(m_kbd_sr, 7); }

	uint8_t scan_matrix();
	void update_matrix_load();
	void update_latch();

	TIMER_CALLBACK_MEMBER(beep_hold_done);

	required_device<m68hc05c4_device> m_maincpu;
	required_device<beep_device> m_speaker;

	required_ioport_array<18> m_kbd;

	output_finder<> m_led_wait;
	output_finder<> m_led_compose;
	output_finder<> m_led_lock;
	output_finder<> m_led_hold;

	uint8_t m_rows[3];      // key matrix row drivers on ports A, B and C (bits 0 and 1 only)
	bool m_kbd_load;        // PC6 level
	bool m_latch_strobe;    // PC7 level
	bool m_sck;
	int m_mosi;
	uint8_t m_kbd_sr;       // matrix column input shift register
	uint8_t m_led_sr;       // LED and bell volume output shift register
	emu_timer *m_beep_hold;
	bool m_beep_on;

	devcb_write_line m_tx_handler;
};

// device type definition
DECLARE_DEVICE_TYPE(LK201, lk201_device)

#endif // MAME_DEC_LK201_H
