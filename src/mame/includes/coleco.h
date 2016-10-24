// license:BSD-3-Clause
// copyright-holders:Mike Balfour, Ben Bruscella, Sean Young
#pragma once

#ifndef __COLECO__
#define __COLECO__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"
#include "machine/coleco.h"
#include "bus/coleco/exp.h"

class coleco_state : public driver_device
{
public:
	coleco_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_cart(*this, COLECOVISION_CARTRIDGE_SLOT_TAG),
			m_ram(*this, "ram"),
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

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint8_t cart_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t paddle_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t paddle_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void paddle_off_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void paddle_on_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void paddle_d7reset_callback(void *ptr, int32_t param);
	void paddle_irqreset_callback(void *ptr, int32_t param);
	void paddle_pulse_callback(void *ptr, int32_t param);
	void paddle_update_callback(timer_device &timer, void *ptr, int32_t param);
	void coleco_vdp_interrupt(int state);
	image_init_result device_image_load_czz50_cart(device_image_interface &image);

	uint8_t coleco_paddle_read(int port, int joy_mode, uint8_t joy_status);
	uint8_t coleco_scan_paddles(uint8_t *joy_status0, uint8_t *joy_status1);

private:
	required_device<cpu_device> m_maincpu;
	required_device<colecovision_cartridge_slot_device> m_cart;
	required_shared_ptr<uint8_t> m_ram;

	int m_joy_mode;
	int m_last_nmi_state;

	// analog controls
	attotime m_joy_pulse_reload[2];
	emu_timer *m_joy_pulse_timer[2];
	emu_timer *m_joy_irq_timer[2];
	emu_timer *m_joy_d7_timer[2];
	int m_joy_irq_state[2];
	int m_joy_d7_state[2];
	uint8_t m_joy_analog_state[2];
	uint8_t m_joy_analog_reload[2];

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

#endif
