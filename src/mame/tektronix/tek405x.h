// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_TEKTRONIX_TEK405X_H
#define MAME_TEKTRONIX_TEK405X_H

#pragma once

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/ieee488/ieee488.h"

#include "cpu/m6800/m6800.h"

#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/ram.h"
#include "machine/timer.h"

#include "sound/spkrdev.h"

#include "video/vector.h"

#include "emupal.h"


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
		m_special(*this, "SPECIAL"),
		m_lamps(*this, "lamp%u", 1U)
	{ }

	void tek4051(machine_config &config);

private:
	void bankswitch(uint8_t data);
	void update_irq();
	void update_nmi();
	void scan_keyboard();

	void lbs_w(uint8_t data);

	uint8_t x_pia_pa_r();
	void x_pia_pa_w(uint8_t data);
	void x_pia_pb_w(uint8_t data);
	void adot_w(int state);
	void bufclk_w(int state);
	void x_pia_irqa_w(int state);
	void x_pia_irqb_w(int state);

	uint8_t sa_r();
	void y_pia_pa_w(uint8_t data);
	void sb_w(uint8_t data);
	void sot_w(int state);
	void y_pia_irqa_w(int state);
	void y_pia_irqb_w(int state);

	uint8_t kb_pia_pa_r();
	uint8_t kb_pia_pb_r();
	void kb_pia_pb_w(uint8_t data);
	void kb_halt_w(int state);
	void kb_pia_irqa_w(int state);
	void kb_pia_irqb_w(int state);

	uint8_t tape_pia_pa_r();
	void tape_pia_pa_w(uint8_t data);
	void tape_pia_pb_w(uint8_t data);
	void tape_pia_irqa_w(int state);
	void tape_pia_irqb_w(int state);

	void dio_w(uint8_t data);
	uint8_t gpib_pia_pb_r();
	void gpib_pia_pb_w(uint8_t data);
	void talk_w(int state);
	void gpib_pia_irqa_w(int state);
	void gpib_pia_irqb_w(int state);

	void com_pia_pa_w(uint8_t data);
	uint8_t com_pia_pb_r();
	void com_pia_pb_w(uint8_t data);
	void com_pia_irqa_w(int state);
	void com_pia_irqb_w(int state);
	void acia_irq_w(int state);
	void write_acia_clock(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_tick);
	void tek4051_mem(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

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
	output_finder<3> m_lamps;

	// interrupts
	int m_x_pia_irqa = 0;
	int m_x_pia_irqb = 0;
	int m_y_pia_irqa = 0;
	int m_y_pia_irqb = 0;
	int m_tape_pia_irqa = 0;
	int m_tape_pia_irqb = 0;
	int m_kb_pia_irqa = 0;
	int m_kb_pia_irqb = 0;
	int m_gpib_pia_irqa = 0;
	int m_gpib_pia_irqb = 0;
	int m_com_pia_irqa = 0;
	int m_com_pia_irqb = 0;
	int m_acia_irq = 0;

	// keyboard
	int m_kc = 0;

	// GPIB
	int m_talk = 0;
};

class tek4052_state : public driver_device
{
public:
	tek4052_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, AM2901A_TAG),
		m_ram(*this, RAM_TAG)
	{ }

	void tek4052(machine_config &config);

private:
	void tek4052_mem(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
};

#endif // MAME_TEKTRONIX_TEK405X_H
