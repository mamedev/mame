// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __TIKI100__
#define __TIKI100__


#include "emu.h"
#include "bus/centronics/ctronics.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "formats/tiki100_dsk.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/z80pio.h"
#include "machine/wd_fdc.h"
#include "sound/ay8910.h"

#define Z80_TAG         "z80"
#define Z80DART_TAG     "z80dart"
#define Z80PIO_TAG      "z80pio"
#define Z80CTC_TAG      "z80ctc"
#define FD1797_TAG      "fd1797"
#define AY8912_TAG      "ay8912"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"
#define CASSETTE_TAG    "cassette"
#define CENTRONICS_TAG  "centronics"
#define SCREEN_TAG      "screen"

#define TIKI100_VIDEORAM_SIZE   0x8000
#define TIKI100_VIDEORAM_MASK   0x7fff

#define BANK_ROM        0
#define BANK_RAM        1
#define BANK_VIDEO_RAM  2

class tiki100_state : public driver_device
{
public:
	tiki100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_ctc(*this, Z80CTC_TAG),
		m_fdc(*this, FD1797_TAG),
		m_pio(*this, Z80PIO_TAG),
		m_dart(*this, Z80DART_TAG),
		m_ram(*this, RAM_TAG),
		m_floppy0(*this, FD1797_TAG":0"),
		m_floppy1(*this, FD1797_TAG":1"),
		m_cassette(*this, CASSETTE_TAG),
		m_centronics(*this, CENTRONICS_TAG),
		m_rom(*this, Z80_TAG),
		m_video_ram(*this, "video_ram"),
		m_y1(*this, "Y1"),
		m_y2(*this, "Y2"),
		m_y3(*this, "Y3"),
		m_y4(*this, "Y4"),
		m_y5(*this, "Y5"),
		m_y6(*this, "Y6"),
		m_y7(*this, "Y7"),
		m_y8(*this, "Y8"),
		m_y9(*this, "Y9"),
		m_y10(*this, "Y10"),
		m_y11(*this, "Y11"),
		m_y12(*this, "Y12"),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<fd1797_t> m_fdc;
	required_device<z80pio_device> m_pio;
	required_device<z80dart_device> m_dart;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<cassette_image_device> m_cassette;
	required_device<centronics_device> m_centronics;
	required_memory_region m_rom;
	optional_shared_ptr<UINT8> m_video_ram;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_y8;
	required_ioport m_y9;
	required_ioport m_y10;
	required_ioport m_y11;
	required_ioport m_y12;
	required_device<palette_device> m_palette;

	virtual void machine_start();
	virtual void machine_reset();

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( gfxram_r );
	DECLARE_WRITE8_MEMBER( gfxram_w );
	DECLARE_READ8_MEMBER( keyboard_r );
	DECLARE_WRITE8_MEMBER( keyboard_w );
	DECLARE_WRITE8_MEMBER( video_mode_w );
	DECLARE_WRITE8_MEMBER( palette_w );
	DECLARE_WRITE8_MEMBER( system_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z0_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z2_w );
	DECLARE_WRITE8_MEMBER( video_scroll_w );

	DECLARE_READ8_MEMBER( pio_pb_r );
	DECLARE_WRITE8_MEMBER( pio_pb_w );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	void bankswitch();

	/* memory state */
	int m_rome;
	int m_vire;

	/* video state */
	UINT8 m_scroll;
	UINT8 m_mode;
	UINT8 m_palette_val;

	/* keyboard state */
	int m_keylatch;

	TIMER_DEVICE_CALLBACK_MEMBER( ctc_tick );
	TIMER_DEVICE_CALLBACK_MEMBER( tape_tick );

	DECLARE_WRITE_LINE_MEMBER(write_centronics_ack);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_perror);

	int m_centronics_ack;
	int m_centronics_busy;
	int m_centronics_perror;
};

#endif
