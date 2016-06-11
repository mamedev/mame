// license:BSD-3-Clause
// copyright-holders:R. Belmont, M. Burke
#pragma once

#ifndef __LK201_H__
#define __LK201_H__

#include "emu.h"
#include "sound/beep.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LK_CMD_POWER_UP     0xfd    /* init power-up sequence */
#define LK_STAT_PWRUP_OK	0x00	/* the power-up self test OK */
#define LK_STAT_PWRUP_KDOWN	0x3d	/* a key was down during the test */
#define LK_STAT_PWRUP_ERROR	0x3e	/* keyboard self test failure */

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

#define LK_PARAM		    0x80	/* start/end parameter list */

#define LK_CMD_RESUME		0x8b	/* resume transmission to the host */
#define LK_CMD_INHIBIT		0x89	/* stop transmission to the host */

#define LK_CMD_TMP_NORPT	0xc1	/* disable typematic - TEMPORARY AUTO-REPEAT INHIBIT */
/* for the currently pressed key */
#define LK_CMD_ENB_RPT		0xe3	/* enable typematic - ENABLE AUTO-REPEAT ACROSSS KEYBOARD */
/* for RPT_DOWN groups */
#define LK_CMD_DIS_RPT		0xe1	/* disable typematic - DISABLE AUTO-REPEAT ACROSSS KEYBOARD */
/* for RPT_DOWN groups */
#define LK_CMD_RPT_TO_DOWN	0xd9	/* CHANGE ALL AUTO-REPEAT TO "down only" (alt.description: set RPT_DOWN groups to DOWN ) */
#define LK_CMD_REQ_ID		0xab	/* request the keyboard ID */
#define LK_CMD_POWER_UP		0xfd	/* init power-up sequence */
#define LK_CMD_TEST_MODE	0xcb	/* enter the factory test mode */
#define LK_CMD_TEST_EXIT	0x80	/* exit the factory test mode */
#define LK_CMD_SET_DEFAULTS	0xd3	/* set power-up defaults */

/* LK201 RESPONDS TO SPECIAL KEYS: */
#define LK_KEY_SHIFT	0xae
#define LK_KEY_CTRL		0xaf
#define LK_KEY_LOCK		0xb0
#define LK_KEY_COMP		0xb1

/* LK201 SPECIAL RESPONSES (codes above 64 decimal): */
#define LK_KEY_RELEASE		0xb3	/* all keys released */
#define LK_KEY_REPEAT		0xb4	/* repeat the last key */
#define LK_STAT_RESUME_ERR	0xb5	/* keystrokes lost while inhibited */
#define LK_STAT_ERROR		0xb6	/* an invalid command received */
#define LK_STAT_INHIBIT_ACK	0xb7	/* transmission inhibited - KEYBOARD LOCKED ACK */ 
#define LK_STAT_TEST_ACK	0xb8	/* the factory test mode entered - TEST MODE ACK */
#define LK_STAT_MODE_KEYDOWN	0xb9	/* a key is down on a change - PREFIX TO KEYS DOWN */

/* to the DOWN_UP mode; */
/* the keycode follows */
#define LK_STAT_MODE_ACK	0xba	/* the mode command succeeded - MODE CHANGE ACK */

// TCR - Timer Compare Register 
#define TCR_OCIE 0x40 // Bit 6 (output compare IRQ enable)
#define TCR_OLVL 0x01 // Bit 1 (output level)         TODO: defines result (level) after successful compare!    TODO 2: AUTO-RESETS (when...?)

// TSR - Timer Status Register 
#define TSR_OCFL 0x40 // TSR (68HC05 output compare flag)  TODO: SET BY INTERAL COMPARE !

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_LK201_TX_HANDLER(_cb) \
	devcb = &lk201_device::set_tx_handler(*device, DEVCB_##_cb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> lk201_device

class lk201_device : public device_t, public device_serial_interface
{
public:
	// construction/destruction
	lk201_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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

	template<class _Object> static devcb_base &set_tx_handler(device_t &device, _Object wr) { return downcast<lk201_device &>(device).m_tx_handler.set_callback(wr); }

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

private:
	UINT8 ddrs[3];
	UINT8 ports[3];
	UINT8 led_data;
	UINT8 kbd_data;

	union {
		struct {
			UINT8 tcr;
			UINT8 tsr;
			UINT8 icrh;
			UINT8 icrl;
			UINT8 ocrh;
			UINT8 ocrl;
			UINT8 crh;
			UINT8 crl;
			UINT8 acrh;
			UINT8 acrl;
		};
		UINT8 regs[10];
	} m_timer;

	emu_timer *m_count;
	emu_timer *m_beeper;

	UINT8 sci_ctl2;
	UINT8 sci_status;
	//UINT8 sci_data;

	UINT8 spi_status;
	UINT8 spi_data;

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

	void send_port(address_space &space, UINT8 offset, UINT8 data);
	void update_interrupts();

	int m_kbd_state;

	devcb_write_line m_tx_handler;
};

// device type definition
extern const device_type LK201;

#endif
