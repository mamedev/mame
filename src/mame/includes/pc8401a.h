// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __PC8401A__
#define __PC8401A__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/ram.h"
#include "machine/upd1990a.h"
#include "video/mc6845.h"
#include "video/sed1330.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

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
	pc8401a_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_rtc(*this, UPD1990A_TAG),
			m_lcdc(*this, SED1330_TAG),
			m_crtc(*this, MC6845_TAG),
			m_screen_lcd(*this, SCREEN_TAG),
			m_cart(*this, "cartslot"),
			m_io_cart(*this, "io_cart"),
			m_ram(*this, RAM_TAG),
			m_rom(*this, Z80_TAG),
			m_crt_ram(*this, "crt_ram"),
			m_io_y(*this, "Y")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<upd1990a_device> m_rtc;
	required_device<sed1330_device> m_lcdc;
	optional_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen_lcd;
	required_device<generic_slot_device> m_cart;
	required_device<generic_slot_device> m_io_cart;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	optional_shared_ptr<UINT8> m_crt_ram;
	required_ioport_array<10> m_io_y;

	memory_region *m_cart_rom;

	virtual void machine_start() override;
	virtual void video_start() override;

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
	pc8500_state(const machine_config &mconfig, device_type type, std::string tag)
		: pc8401a_state(mconfig, type, tag)
	{ }

	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

// ---------- defined in video/pc8401a.c ----------

MACHINE_CONFIG_EXTERN( pc8401a_video );
MACHINE_CONFIG_EXTERN( pc8500_video );

#endif
