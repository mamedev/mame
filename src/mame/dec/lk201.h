// license:BSD-3-Clause
// copyright-holders:R. Belmont, M. Burke
#ifndef MAME_DEC_LK201_H
#define MAME_DEC_LK201_H

#pragma once

#include "sound/beep.h"

#include "diserial.h"


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

// TCR - Timer Compare Register
#define TCR_OCIE 0x40 // Bit 6 (output compare IRQ enable)
#define TCR_OLVL 0x01 // Bit 1 (output level)

// TSR - Timer Status Register
#define TSR_OCFL 0x40 // TSR (68HC05 output compare flag)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> lk201_device

class lk201_device : public device_t, public device_serial_interface
{
public:
	// construction/destruction
	lk201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t ddr_r(offs_t offset);
	void ddr_w(offs_t offset, uint8_t data);
	uint8_t ports_r(offs_t offset);
	void ports_w(offs_t offset, uint8_t data);
	uint8_t sci_r(offs_t offset);
	void sci_w(offs_t offset, uint8_t data);
	uint8_t spi_r(offs_t offset);
	void spi_w(offs_t offset, uint8_t data);
	uint8_t timer_r(offs_t offset);
	void timer_w(offs_t offset, uint8_t data);

	auto tx_handler() { return m_tx_handler.bind(); }

	void lk201_map(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	TIMER_CALLBACK_MEMBER(timer_irq);
	TIMER_CALLBACK_MEMBER(beeper_off);

private:
	uint8_t ddrs[3];
	uint8_t ports[3];
	uint8_t led_data;
	uint8_t kbd_data;

	union {
		struct {
			uint8_t tcr;
			uint8_t tsr;
			uint8_t icrh;
			uint8_t icrl;
			uint8_t ocrh;
			uint8_t ocrl;
			uint8_t crh;
			uint8_t crl;
			uint8_t acrh;
			uint8_t acrl;
		};
		uint8_t regs[10];
	} m_timer;

	emu_timer *m_count;
	emu_timer *m_beeper;

	uint8_t sci_ctl2;
	uint8_t sci_status;
	//uint8_t sci_data;

	uint8_t spi_status;
	uint8_t spi_data;

	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_speaker;

	required_ioport_array<18> m_kbd;

	output_finder<> m_led_wait;
	output_finder<> m_led_compose;
	output_finder<> m_led_lock;
	output_finder<> m_led_hold;

	void send_port(uint8_t offset, uint8_t data);
	void update_interrupts();

	int m_kbd_state;

	devcb_write_line m_tx_handler;
};

// device type definition
DECLARE_DEVICE_TYPE(LK201, lk201_device)

#endif // MAME_DEC_LK201_H
