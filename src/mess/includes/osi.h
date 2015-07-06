// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert, Wilbert Pol
#pragma once

#ifndef __OSI__
#define __OSI__


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "formats/basicdsk.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/6850acia.h"
#include "machine/6821pia.h"
#include "machine/ram.h"
#include "sound/discrete.h"
#include "sound/beep.h"

#define SCREEN_TAG      "screen"
#define M6502_TAG       "m6502"
#define DISCRETE_TAG    "discrete"

#define X1          3932160
#define UK101_X1    XTAL_8MHz

#define OSI600_VIDEORAM_SIZE    0x400
#define OSI630_COLORRAM_SIZE    0x400

class sb2m600_state : public driver_device
{
public:
	sb2m600_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, M6502_TAG),
		m_acia_0(*this, "acia_0"),
		m_cassette(*this, "cassette"),
		m_discrete(*this, DISCRETE_TAG),
		m_ram(*this, RAM_TAG),
		m_video_ram(*this, "video_ram"),
		m_color_ram(*this, "color_ram"),
		m_io_row0(*this, "ROW0"),
		m_io_row1(*this, "ROW1"),
		m_io_row2(*this, "ROW2"),
		m_io_row3(*this, "ROW3"),
		m_io_row4(*this, "ROW4"),
		m_io_row5(*this, "ROW5"),
		m_io_row6(*this, "ROW6"),
		m_io_row7(*this, "ROW7"),
		m_io_sound(*this, "Sound"),
		m_io_reset(*this, "Reset"),
		m_beeper(*this, "beeper")
	{
	}

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER( keyboard_r );
	DECLARE_WRITE8_MEMBER( keyboard_w );
	DECLARE_WRITE8_MEMBER( ctrl_w );
	DECLARE_WRITE_LINE_MEMBER( cassette_tx );
	DECLARE_WRITE_LINE_MEMBER( write_cassette_clock );

	void floppy_index_callback(floppy_image_device *floppy, int state);

	DECLARE_PALETTE_INIT(osi630);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void machine_start();
	virtual void video_start();

	enum
	{
		TIMER_SETUP_BEEP
	};

	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia_0;
	required_device<cassette_image_device> m_cassette;
	optional_device<discrete_sound_device> m_discrete;
	required_device<ram_device> m_ram;
	required_shared_ptr<UINT8> m_video_ram;
	optional_shared_ptr<UINT8> m_color_ram;
	required_ioport m_io_row0;
	required_ioport m_io_row1;
	required_ioport m_io_row2;
	required_ioport m_io_row3;
	required_ioport m_io_row4;
	required_ioport m_io_row5;
	required_ioport m_io_row6;
	required_ioport m_io_row7;
	required_ioport m_io_sound;
	required_ioport m_io_reset;
	optional_device<beep_device> m_beeper;

	/* floppy state */
	int m_fdc_index;

	/* keyboard state */
	UINT8 m_keylatch;

	/* video state */
	int m_32;
	int m_coloren;
	UINT8 *m_p_chargen;
};

class c1p_state : public sb2m600_state
{
public:
	c1p_state(const machine_config &mconfig, device_type type, const char *tag) :
		sb2m600_state(mconfig, type, tag),
		m_beep(*this, "beeper")
	{
	}

	required_device<beep_device> m_beep;

	virtual void machine_start();

	DECLARE_WRITE8_MEMBER( osi630_ctrl_w );
	DECLARE_WRITE8_MEMBER( osi630_sound_w );
	DECLARE_DRIVER_INIT(c1p);
};

class c1pmf_state : public c1p_state
{
public:
	c1pmf_state(const machine_config &mconfig, device_type type, const char *tag) :
		c1p_state(mconfig, type, tag),
		m_floppy0(*this, "floppy0"),
		m_floppy1(*this, "floppy1")
	{ }

	DECLARE_READ8_MEMBER( osi470_pia_pa_r );
	DECLARE_WRITE8_MEMBER( osi470_pia_pa_w );
	DECLARE_WRITE8_MEMBER( osi470_pia_pb_w );
	DECLARE_WRITE_LINE_MEMBER( osi470_pia_cb2_w );

protected:
	virtual void machine_start();

private:
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
};

class uk101_state : public sb2m600_state
{
public:
	uk101_state(const machine_config &mconfig, device_type type, const char *tag) :
		sb2m600_state(mconfig, type, tag)
	{
	}

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER( keyboard_w );
};

/* ---------- defined in video/osi.c ---------- */

MACHINE_CONFIG_EXTERN( osi600_video );
MACHINE_CONFIG_EXTERN( uk101_video );
MACHINE_CONFIG_EXTERN( osi630_video );

#endif
