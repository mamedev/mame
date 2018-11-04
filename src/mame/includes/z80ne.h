// license:BSD-3-Clause
// copyright-holders:Roberto Lavarone
/*****************************************************************************
 *
 * includes/z80ne.h
 *
 * Nuova Elettronica Z80NE
 *
 * http://www.z80ne.com/
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_Z80NE_H
#define MAME_INCLUDES_Z80NE_H

#pragma once

#include "video/mc6847.h"
#include "imagedev/cassette.h"
#include "machine/ay31015.h"
#include "machine/kr2376.h"
#include "machine/wd_fdc.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define Z80NE_CPU_SPEED_HZ      1920000 /* 1.92 MHz */

#define LX383_KEYS          16
#define LX383_DOWNSAMPLING  16

#define LX385_TAPE_SAMPLE_FREQ 38400

/* wave duration threshold */
enum z80netape_speed
{
	TAPE_300BPS  = 300, /*  300 bps */
	TAPE_600BPS  = 600, /*  600 bps */
	TAPE_1200BPS = 1200 /* 1200 bps */
};

struct z80ne_cass_data_t {
	struct {
		int length;     /* time cassette level is at input.level */
		int level;      /* cassette level */
		int bit;        /* bit being read */
	} input;
	struct {
		int length;     /* time cassette level is at output.level */
		int level;      /* cassette level */
		int bit;        /* bit to output */
	} output;
	z80netape_speed speed;          /* 300 - 600 - 1200 */
	int wave_filter;
	int wave_length;
	int wave_short;
	int wave_long;
};

struct wd17xx_state_t {
	int drq;
	int intrq;
	uint8_t drive; /* current drive */
	uint8_t head;  /* current head */
};


class z80ne_state : public driver_device
{
public:
	z80ne_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vdg(*this, "mc6847"),
		m_videoram(*this, "videoram"),
		m_ay31015(*this, "ay_3_1015"),
		m_lx388_kr2376(*this, "lx388_kr2376"),
		m_maincpu(*this, "z80ne"),
		m_floppy0(*this, "wd1771:0"),
		m_floppy1(*this, "wd1771:1"),
		m_floppy2(*this, "wd1771:2"),
		m_floppy3(*this, "wd1771:3"),
		m_cassette1(*this, "cassette"),
		m_cassette2(*this, "cassette2"),
		m_wd1771(*this, "wd1771"),
		m_region_z80ne(*this, "z80ne"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_bank4(*this, "bank4"),
		m_io_row0(*this, "ROW0"),
		m_io_row1(*this, "ROW1"),
		m_io_ctrl(*this, "CTRL"),
		m_io_rst(*this, "RST"),
		m_io_lx_385(*this, "LX.385"),
		m_io_lx388_brk(*this, "LX388_BRK"),
		m_io_x0(*this, "X0"),
		m_io_x1(*this, "X1"),
		m_io_x2(*this, "X2"),
		m_io_x3(*this, "X3"),
		m_io_x4(*this, "X4"),
		m_io_x5(*this, "X5"),
		m_io_x6(*this, "X6"),
		m_io_x7(*this, "X7"),
		m_io_modifiers(*this, "MODIFIERS"),
		m_io_config(*this, "CONFIG") { }

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	optional_device<mc6847_base_device> m_vdg;
	optional_shared_ptr<uint8_t> m_videoram;
	required_device<ay31015_device> m_ay31015;
	optional_device<kr2376_device> m_lx388_kr2376;
	uint8_t m_lx383_scan_counter;
	uint8_t m_lx383_key[LX383_KEYS];
	int m_lx383_downsampler;
	uint8_t m_lx385_ctrl;
	emu_timer *m_cassette_timer;
	emu_timer *m_kbd_timer;
	z80ne_cass_data_t m_cass_data;
	wd17xx_state_t m_wd17xx_state;
	DECLARE_READ8_MEMBER(lx383_r);
	DECLARE_WRITE8_MEMBER(lx383_w);
	DECLARE_READ8_MEMBER(lx385_data_r);
	DECLARE_READ8_MEMBER(lx385_ctrl_r);
	DECLARE_WRITE8_MEMBER(lx385_data_w);
	DECLARE_WRITE8_MEMBER(lx385_ctrl_w);
	DECLARE_READ8_MEMBER(lx388_data_r);
	DECLARE_READ8_MEMBER(lx388_read_field_sync);
	DECLARE_DRIVER_INIT(z80netf);
	DECLARE_DRIVER_INIT(z80net);
	DECLARE_DRIVER_INIT(z80netb);
	DECLARE_DRIVER_INIT(z80ne);
	DECLARE_MACHINE_START(z80ne);
	DECLARE_MACHINE_RESET(z80ne);
	DECLARE_MACHINE_START(z80netb);
	DECLARE_MACHINE_RESET(z80netb);
	DECLARE_MACHINE_START(z80netf);
	DECLARE_MACHINE_RESET(z80netf);
	DECLARE_MACHINE_START(z80net);
	DECLARE_MACHINE_RESET(z80net);
	DECLARE_MACHINE_RESET(z80ne_base);
	DECLARE_INPUT_CHANGED_MEMBER(z80ne_reset);
	DECLARE_INPUT_CHANGED_MEMBER(z80ne_nmi);
	TIMER_CALLBACK_MEMBER(z80ne_cassette_tc);
	TIMER_CALLBACK_MEMBER(z80ne_kbd_scan);
	DECLARE_READ8_MEMBER(lx388_mc6847_videoram_r);
	DECLARE_WRITE8_MEMBER(lx390_motor_w);
	DECLARE_READ8_MEMBER(lx390_reset_bank);
	DECLARE_READ8_MEMBER(lx390_fdc_r);
	DECLARE_WRITE8_MEMBER(lx390_fdc_w);

	void z80net(machine_config &config);
	void z80netf(machine_config &config);
	void z80ne(machine_config &config);
	void z80netb(machine_config &config);
	void z80ne_io(address_map &map);
	void z80ne_mem(address_map &map);
	void z80net_io(address_map &map);
	void z80net_mem(address_map &map);
	void z80netb_mem(address_map &map);
	void z80netf_io(address_map &map);
	void z80netf_mem(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	required_device<cpu_device> m_maincpu;
	optional_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	optional_device<floppy_connector> m_floppy2;
	optional_device<floppy_connector> m_floppy3;
	required_device<cassette_image_device> m_cassette1;
	required_device<cassette_image_device> m_cassette2;
	optional_device<fd1771_device> m_wd1771;
	required_memory_region m_region_z80ne;
	optional_memory_bank m_bank1;
	optional_memory_bank m_bank2;
	optional_memory_bank m_bank3;
	optional_memory_bank m_bank4;
	required_ioport m_io_row0;
	required_ioport m_io_row1;
	required_ioport m_io_ctrl;
	required_ioport m_io_rst;
	required_ioport m_io_lx_385;
	optional_ioport m_io_lx388_brk;
	optional_ioport m_io_x0;
	optional_ioport m_io_x1;
	optional_ioport m_io_x2;
	optional_ioport m_io_x3;
	optional_ioport m_io_x4;
	optional_ioport m_io_x5;
	optional_ioport m_io_x6;
	optional_ioport m_io_x7;
	optional_ioport m_io_modifiers;
	optional_ioport m_io_config;

	emu_timer *m_timer_nmi;
	emu_timer *m_timer_reset;

	cassette_image_device *cassette_device_image();
	void reset_lx388();
	void reset_lx382_banking();
	void reset_lx390_banking();
};

#endif // MAME_INCLUDES_Z80NE_H
