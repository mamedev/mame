// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Ben Bruscella, Sean Young, Frank Palazzolo
#pragma once

#ifndef MAME_COLECO_COLECO_H
#define MAME_COLECO_COLECO_H


#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "machine/ram.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"
#include "coleco_m.h"
#include "bus/coleco/cartridge/exp.h"

class coleco_state : public driver_device
{
public:
	coleco_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, COLECOVISION_CARTRIDGE_SLOT_TAG),
		m_ctrlsel(*this, "CTRLSEL"),
		m_std_keypad1(*this, "STD_KEYPAD1"),
		m_std_joy1(*this, "STD_JOY1"),
		m_std_keypad2(*this, "STD_KEYPAD2"),
		m_std_joy2(*this, "STD_JOY2"),
		m_sac_keypad1(*this, "SAC_KEYPAD1"),
		m_sac_joy1(*this, "SAC_JOY1"),
		m_sac_slide1(*this, "SAC_SLIDE1"),
		m_sac_keypad2(*this, "SAC_KEYPAD2"),
		m_sac_joy2(*this, "SAC_JOY2"),
		m_sac_slide2(*this, "SAC_SLIDE2"),
		m_driv_wheel1(*this, "DRIV_WHEEL1"),
		m_driv_pedal1(*this, "DRIV_PEDAL1"),
		m_driv_wheel2(*this, "DRIV_WHEEL2"),
		m_driv_pedal2(*this, "DRIV_PEDAL2"),
		m_roller_x(*this, "ROLLER_X"),
		m_roller_y(*this, "ROLLER_Y")
	{ }

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t cart_r(offs_t offset);
	void cart_w(offs_t offset, uint8_t data);

	uint8_t paddle_1_r();
	uint8_t paddle_2_r();
	void paddle_off_w(uint8_t data);
	void paddle_on_w(uint8_t data);

	TIMER_CALLBACK_MEMBER(paddle_d7reset_callback);
	TIMER_CALLBACK_MEMBER(paddle_irqreset_callback);
	TIMER_CALLBACK_MEMBER(paddle_pulse_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(paddle_update_callback);
	void coleco_vdp_interrupt(int state);

	uint8_t coleco_paddle_read(int port, int joy_mode, uint8_t joy_status);
	uint8_t coleco_scan_paddles(uint8_t *joy_status0, uint8_t *joy_status1);

	void colecop(machine_config &config);
	void coleco(machine_config &config);
	void czz50(machine_config &config);
	void dina(machine_config &config);
	void coleco_io_map(address_map &map) ATTR_COLD;
	void coleco_map(address_map &map) ATTR_COLD;
	void czz50_map(address_map &map) ATTR_COLD;

protected:
	required_device<z80_device> m_maincpu;
	required_device<colecovision_cartridge_slot_device> m_cart;

	int m_joy_mode = 0;
	int m_last_nmi_state = 0;

	// analog controls
	attotime m_joy_pulse_reload[2]{};
	emu_timer *m_joy_pulse_timer[2]{};
	emu_timer *m_joy_irq_timer[2]{};
	emu_timer *m_joy_d7_timer[2]{};
	int m_joy_irq_state[2]{};
	int m_joy_d7_state[2]{};
	uint8_t m_joy_analog_state[2]{};
	uint8_t m_joy_analog_reload[2]{};

	optional_ioport m_ctrlsel;
	required_ioport m_std_keypad1;
	required_ioport m_std_joy1;
	required_ioport m_std_keypad2;
	required_ioport m_std_joy2;
	optional_ioport m_sac_keypad1;
	optional_ioport m_sac_joy1;
	optional_ioport m_sac_slide1;
	optional_ioport m_sac_keypad2;
	optional_ioport m_sac_joy2;
	optional_ioport m_sac_slide2;
	optional_ioport m_driv_wheel1;
	optional_ioport m_driv_pedal1;
	optional_ioport m_driv_wheel2;
	optional_ioport m_driv_pedal2;
	optional_ioport m_roller_x;
	optional_ioport m_roller_y;
};

class bit90_state : public coleco_state
{
public:
	bit90_state(const machine_config &mconfig, device_type type, const char *tag) :
		coleco_state(mconfig, type, tag),
		m_bank(*this, "bank"),
		m_ram(*this, RAM_TAG),
		m_io_keyboard(*this, {"ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7"})
	{ }

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void bit90(machine_config &config);

	uint8_t bankswitch_u4_r(address_space &space);
	uint8_t bankswitch_u3_r(address_space &space);
	uint8_t keyboard_r(address_space &space);
	void u32_w(uint8_t data);

	void init();

protected:
	required_memory_bank m_bank;
	required_device<ram_device> m_ram;
	required_ioport_array<8> m_io_keyboard;

private:
	void bit90_map(address_map &map) ATTR_COLD;
	void bit90_io_map(address_map &map) ATTR_COLD;

	uint8_t m_keyselect = 0U;
	uint8_t m_unknown = 0U;
};

#endif // MAME_COLECO_COLECO_H
