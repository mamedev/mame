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
#include "imagedev/floppy.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/kr2376.h"
#include "machine/ram.h"
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


class z80ne_state : public driver_device
{
public:
	z80ne_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vdg(*this, "mc6847"),
		m_videoram(*this, "videoram"),
		m_uart(*this, "uart"),
		m_uart_clock(*this, "uart_clock"),
		m_lx387_kr2376(*this, "lx387_kr2376"),
		m_maincpu(*this, "z80ne"),
		m_cassette1(*this, "cassette"),
		m_cassette2(*this, "cassette2"),
		m_ram(*this, RAM_TAG),
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
		m_io_lx387_brk(*this, "LX387_BRK"),
		m_io_modifiers(*this, "MODIFIERS"),
		m_lx383_digits(*this, "digit%u", 0U)
	{
	}

	void lx387(machine_config &config);
	void z80net(machine_config &config);
	void z80ne(machine_config &config);
	void z80netb(machine_config &config);

	void init_z80net();
	void init_z80netb();
	void init_z80ne();

	DECLARE_INPUT_CHANGED_MEMBER(z80ne_reset);
	DECLARE_INPUT_CHANGED_MEMBER(z80ne_nmi);

	DECLARE_READ8_MEMBER(lx388_mc6847_videoram_r);

protected:
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	optional_device<mc6847_base_device> m_vdg;
	optional_shared_ptr<uint8_t> m_videoram;
	required_device<ay31015_device> m_uart;
	required_device<clock_device> m_uart_clock;
	optional_device<kr2376_device> m_lx387_kr2376;
	uint8_t m_lx383_scan_counter;
	uint8_t m_lx383_key[LX383_KEYS];
	int m_lx383_downsampler;
	uint8_t m_lx385_ctrl;
	emu_timer *m_cassette_timer;
	emu_timer *m_kbd_timer;
	z80ne_cass_data_t m_cass_data;
	DECLARE_READ8_MEMBER(lx383_r);
	DECLARE_WRITE8_MEMBER(lx383_w);
	DECLARE_READ8_MEMBER(lx385_ctrl_r);
	DECLARE_WRITE8_MEMBER(lx385_ctrl_w);
	DECLARE_WRITE_LINE_MEMBER(lx385_uart_tx_clock_w);
	DECLARE_READ_LINE_MEMBER(lx387_shift_r);
	DECLARE_READ_LINE_MEMBER(lx387_control_r);
	DECLARE_READ8_MEMBER(lx387_data_r);
	DECLARE_READ8_MEMBER(lx388_read_field_sync);
	DECLARE_MACHINE_START(z80ne);
	DECLARE_MACHINE_RESET(z80ne);
	DECLARE_MACHINE_START(z80netb);
	DECLARE_MACHINE_RESET(z80netb);
	DECLARE_MACHINE_START(z80net);
	DECLARE_MACHINE_RESET(z80net);
	DECLARE_MACHINE_RESET(z80ne_base);

	TIMER_CALLBACK_MEMBER(z80ne_cassette_tc);
	TIMER_CALLBACK_MEMBER(z80ne_kbd_scan);

	void z80ne_io(address_map &map);
	void z80ne_mem(address_map &map);
	void z80net_io(address_map &map);
	void z80net_mem(address_map &map);
	void z80netb_mem(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette1;
	required_device<cassette_image_device> m_cassette2;
	required_device<ram_device> m_ram;
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
	optional_ioport m_io_lx387_brk;
	optional_ioport m_io_modifiers;
	output_finder<8> m_lx383_digits;

	emu_timer *m_timer_nmi;
	emu_timer *m_timer_reset;

	cassette_image_device *cassette_device_image();
	void reset_lx387();
	void reset_lx382_banking();
};

class z80netf_state : public z80ne_state
{
public:
	z80netf_state(const machine_config &mconfig, device_type type, const char *tag)
		: z80ne_state(mconfig, type, tag),
		m_io_config(*this, "CONFIG"),
		m_floppy(*this, "wd1771:%u", 0U),
		m_wd1771(*this, "wd1771"),
		m_drv_led(*this, "drv%u", 0U)
	{
	}

	void z80netf(machine_config &config);

	void init_z80netf();

private:
	struct wd17xx_state_t
	{
		int drq;
		int intrq;
		uint8_t drive; /* current drive */
		uint8_t head;  /* current head */
	};

	void z80netf_io(address_map &map);
	void z80netf_mem(address_map &map);

	DECLARE_MACHINE_START(z80netf);
	DECLARE_MACHINE_RESET(z80netf);

	DECLARE_WRITE8_MEMBER(lx390_motor_w);
	DECLARE_READ8_MEMBER(lx390_reset_bank);
	DECLARE_READ8_MEMBER(lx390_fdc_r);
	DECLARE_WRITE8_MEMBER(lx390_fdc_w);

	void reset_lx390_banking();

	required_ioport m_io_config;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<fd1771_device> m_wd1771;
	wd17xx_state_t m_wd17xx_state;
	output_finder<2> m_drv_led;
};

#endif // MAME_INCLUDES_Z80NE_H
