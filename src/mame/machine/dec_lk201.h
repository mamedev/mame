// license:BSD-3-Clause
// copyright-holders:R. Belmont, M. Burke
#ifndef MAME_MACHINE_DEC_LK201_H
#define MAME_MACHINE_DEC_LK201_H

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
	lk201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(ddr_r);
	DECLARE_WRITE8_MEMBER(ddr_w);
	DECLARE_READ8_MEMBER(ports_r);
	DECLARE_WRITE8_MEMBER(ports_w);
	DECLARE_READ8_MEMBER(sci_r);
	DECLARE_WRITE8_MEMBER(sci_w);
	DECLARE_READ8_MEMBER(spi_r);
	DECLARE_WRITE8_MEMBER(spi_w);
	DECLARE_READ8_MEMBER(timer_r);
	DECLARE_WRITE8_MEMBER(timer_w);

	auto tx_handler() { return m_tx_handler.bind(); }

	void lk201_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

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

	required_ioport m_kbd0;
	required_ioport m_kbd1;
	required_ioport m_kbd2;
	required_ioport m_kbd3;
	required_ioport m_kbd4;
	required_ioport m_kbd5;
	required_ioport m_kbd6;
	required_ioport m_kbd7;
	required_ioport m_kbd8;
	required_ioport m_kbd9;
	required_ioport m_kbd10;
	required_ioport m_kbd11;
	required_ioport m_kbd12;
	required_ioport m_kbd13;
	required_ioport m_kbd14;
	required_ioport m_kbd15;
	required_ioport m_kbd16;
	required_ioport m_kbd17;

	void send_port(address_space &space, uint8_t offset, uint8_t data);
	void update_interrupts();

	int m_kbd_state;

	devcb_write_line m_tx_handler;
};

// device type definition
DECLARE_DEVICE_TYPE(LK201, lk201_device)

#endif // MAME_MACHINE_DEC_LK201_H
