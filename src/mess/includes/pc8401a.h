// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __PC8401A__
#define __PC8401A__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cartslot.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/ram.h"
#include "machine/upd1990a.h"
#include "video/mc6845.h"
#include "video/sed1330.h"

#define SCREEN_TAG      "screen"
#define CRT_SCREEN_TAG  "screen2"

#define Z80_TAG         "z80"
#define I8255A_TAG      "i8255a"
#define UPD1990A_TAG    "upd1990a"
#define AY8910_TAG      "ay8910"
#define SED1330_TAG     "sed1330"
#define MC6845_TAG      "mc6845"
#define I8251_TAG       "i8251"
#define RS232_TAG       "rs232"

#define PC8401A_CRT_VIDEORAM_SIZE   0x2000

class pc8401a_state : public driver_device
{
public:
	pc8401a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_rtc(*this, UPD1990A_TAG),
			m_lcdc(*this, SED1330_TAG),
			m_crtc(*this, MC6845_TAG),
			m_screen_lcd(*this, SCREEN_TAG),
			m_ram(*this, RAM_TAG),
			m_rom(*this, Z80_TAG),
			m_io_rom(*this, "iorom"),
			m_crt_ram(*this, "crt_ram"),
			m_y0(*this, "Y0"),
			m_y1(*this, "Y1"),
			m_y2(*this, "Y2"),
			m_y3(*this, "Y3"),
			m_y4(*this, "Y4"),
			m_y5(*this, "Y5"),
			m_y6(*this, "Y6"),
			m_y7(*this, "Y7"),
			m_y8(*this, "Y8"),
			m_y9(*this, "Y9")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<upd1990a_device> m_rtc;
	required_device<sed1330_device> m_lcdc;
	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen_lcd;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_memory_region m_io_rom;
	optional_shared_ptr<UINT8> m_crt_ram;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_y8;
	required_ioport m_y9;

	virtual void machine_start();

	virtual void video_start();

	DECLARE_WRITE8_MEMBER( mmr_w );
	DECLARE_READ8_MEMBER( mmr_r );
	DECLARE_READ8_MEMBER( rtc_r );
	DECLARE_WRITE8_MEMBER( rtc_cmd_w );
	DECLARE_WRITE8_MEMBER( rtc_ctrl_w );
	DECLARE_READ8_MEMBER( io_rom_data_r );
	DECLARE_WRITE8_MEMBER( io_rom_addr_w );
	DECLARE_READ8_MEMBER( port70_r );
	DECLARE_READ8_MEMBER( port71_r );
	DECLARE_WRITE8_MEMBER( port70_w );
	DECLARE_WRITE8_MEMBER( port71_w );
	DECLARE_READ8_MEMBER( ppi_pc_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );
	DECLARE_PALETTE_INIT(pc8401a);

	void scan_keyboard();
	void bankswitch(UINT8 data);

	// keyboard state
	int m_key_strobe;           // key pressed

	// memory state
	UINT8 m_mmr;                // memory mapping register
	UINT32 m_io_addr;           // I/O ROM address counter

	UINT8 m_key_latch;
	TIMER_DEVICE_CALLBACK_MEMBER(pc8401a_keyboard_tick);
};

class pc8500_state : public pc8401a_state
{
public:
	pc8500_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc8401a_state(mconfig, type, tag)
	{ }

	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

// ---------- defined in video/pc8401a.c ----------

MACHINE_CONFIG_EXTERN( pc8401a_video );
MACHINE_CONFIG_EXTERN( pc8500_video );

#endif
