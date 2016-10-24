// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __TEK405X__
#define __TEK405X__

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/ram.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "bus/ieee488/ieee488.h"
#include "machine/ram.h"
#include "sound/speaker.h"
#include "video/vector.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#define MC6800_TAG          "u61"
#define MC6820_Y_TAG        "u561"
#define MC6820_X_TAG        "u565"
#define MC6820_TAPE_TAG     "u361"
#define MC6820_KB_TAG       "u461"
#define MC6820_GPIB_TAG     "u265"
#define MC6820_COM_TAG      "u5"
#define MC6850_TAG          "u25"
#define RS232_TAG           "rs232"
#define SCREEN_TAG          "screen"

#define AM2901A_TAG         "am2901a"

class tek4051_state : public driver_device
{
public:
	tek4051_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, MC6800_TAG),
		m_gpib_pia(*this, MC6820_GPIB_TAG),
		m_com_pia(*this, MC6820_COM_TAG),
		m_acia(*this, MC6850_TAG),
		m_acia_clock(*this, "acia_clock"),
		m_gpib(*this, IEEE488_TAG),
		m_speaker(*this, "speaker"),
		m_ram(*this, RAM_TAG),
		m_rom(*this, MC6800_TAG),
		m_bsofl_rom(*this, "020_0147_00"),
		m_bscom_rom(*this, "021_0188_00"),
		m_special(*this, "SPECIAL")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_gpib_pia;
	required_device<pia6821_device> m_com_pia;
	required_device<acia6850_device> m_acia;
	required_device<clock_device> m_acia_clock;
	required_device<ieee488_device> m_gpib;
	required_device<speaker_sound_device> m_speaker;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_memory_region m_bsofl_rom;
	required_memory_region m_bscom_rom;
	required_ioport m_special;

	virtual void machine_start() override;

	virtual void video_start() override;

	void bankswitch(uint8_t data);
	void update_irq();
	void update_nmi();
	void scan_keyboard();

	void lbs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t x_pia_pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void x_pia_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void x_pia_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adot_w(int state);
	void bufclk_w(int state);
	void x_pia_irqa_w(int state);
	void x_pia_irqb_w(int state);

	uint8_t sa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void y_pia_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sot_w(int state);
	void y_pia_irqa_w(int state);
	void y_pia_irqb_w(int state);

	uint8_t kb_pia_pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t kb_pia_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void kb_pia_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kb_halt_w(int state);
	void kb_pia_irqa_w(int state);
	void kb_pia_irqb_w(int state);

	uint8_t tape_pia_pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tape_pia_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tape_pia_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tape_pia_irqa_w(int state);
	void tape_pia_irqb_w(int state);

	void dio_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t gpib_pia_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void gpib_pia_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void talk_w(int state);
	void gpib_pia_irqa_w(int state);
	void gpib_pia_irqb_w(int state);

	void com_pia_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t com_pia_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void com_pia_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void com_pia_irqa_w(int state);
	void com_pia_irqb_w(int state);
	void acia_irq_w(int state);
	void write_acia_clock(int state);

	// interrupts
	int m_x_pia_irqa;
	int m_x_pia_irqb;
	int m_y_pia_irqa;
	int m_y_pia_irqb;
	int m_tape_pia_irqa;
	int m_tape_pia_irqb;
	int m_kb_pia_irqa;
	int m_kb_pia_irqb;
	int m_gpib_pia_irqa;
	int m_gpib_pia_irqb;
	int m_com_pia_irqa;
	int m_com_pia_irqb;
	int m_acia_irq;

	// keyboard
	int m_kbhalt;
	int m_kc;

	// GPIB
	int m_talk;
	void keyboard_tick(timer_device &timer, void *ptr, int32_t param);
};

class tek4052_state : public driver_device
{
public:
	tek4052_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, AM2901A_TAG),
			m_ram(*this, RAM_TAG)
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;

	virtual void machine_start() override;

	virtual void video_start() override;
};

#endif
