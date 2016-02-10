// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __ABC800__
#define __ABC800__

#include "bus/abcbus/abcbus.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/cassette.h"
#include "bus/abckb/abckb.h"
#include "bus/abckb/abc800kb.h"
#include "machine/e0516.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/ram.h"
#include "sound/discrete.h"
#include "video/mc6845.h"
#include "video/saa5050.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define ABC800_X01  XTAL_12MHz
#define ABC806_X02  XTAL_32_768kHz

#define ABC802_AT0  0x01
#define ABC802_AT1  0x02
#define ABC802_ATD  0x40
#define ABC802_ATE  0x80
#define ABC802_INV  0x80

#define ABC800C_CHAR_RAM_SIZE   0x400
#define ABC800M_CHAR_RAM_SIZE   0x800
#define ABC800_VIDEO_RAM_SIZE   0x4000
#define ABC802_CHAR_RAM_SIZE    0x800
#define ABC806_CHAR_RAM_SIZE    0x800
#define ABC806_ATTR_RAM_SIZE    0x800
#define ABC806_VIDEO_RAM_SIZE   0x20000

#define ABC800_CHAR_WIDTH   12
#define ABC800_CCLK         ABC800_X01/ABC800_CHAR_WIDTH

#define SCREEN_TAG          "screen"
#define Z80_TAG             "z80"
#define E0516_TAG           "j13"
#define MC6845_TAG          "b12"
#define SAA5052_TAG         "5c"
#define Z80CTC_TAG          "z80ctc"
#define Z80SIO_TAG          "z80sio"
#define Z80DART_TAG         "z80dart"
#define DISCRETE_TAG        "discrete"
#define RS232_A_TAG         "rs232a"
#define RS232_B_TAG         "rs232b"
#define ABC_KEYBOARD_PORT_TAG   "kb"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc800_state

class abc800_state : public driver_device
{
public:
	abc800_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_ctc(*this, Z80CTC_TAG),
		m_dart(*this, Z80DART_TAG),
		m_sio(*this, Z80SIO_TAG),
		m_discrete(*this, "discrete"),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_rom(*this, Z80_TAG),
		m_video_ram(*this, "video_ram"),
		m_char_ram(*this, "char_ram"),
		m_io_sb(*this, "SB"),
		m_ctc_z0(0),
		m_sio_txcb(0),
		m_sio_txdb(1),
		m_sio_rtsb(1),
		m_dfd_out(0),
		m_tape_ctr(4)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_dart;
	required_device<z80sio2_device> m_sio;
	optional_device<discrete_sound_device> m_discrete;
	optional_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	optional_shared_ptr<UINT8> m_video_ram;
	optional_shared_ptr<UINT8> m_char_ram;
	required_ioport m_io_sb;

	enum
	{
		TIMER_ID_CTC,
		TIMER_ID_CASSETTE
	};

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	void bankswitch();
	void clock_cassette(int state);

	DECLARE_READ8_MEMBER( pling_r );
	DECLARE_WRITE8_MEMBER( hrs_w );
	DECLARE_WRITE8_MEMBER( hrc_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z0_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z1_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z2_w );
	DECLARE_WRITE_LINE_MEMBER( sio_txdb_w );
	DECLARE_WRITE_LINE_MEMBER( sio_dtrb_w );
	DECLARE_WRITE_LINE_MEMBER( sio_rtsb_w );

	// memory state
	int m_fetch_charram;        // opcode fetched from character RAM region (0x7800-0x7fff)

	// sound state
	int m_pling;

	// serial state
	UINT8 m_sb;
	int m_ctc_z0;
	int m_sio_txcb;
	int m_sio_txdb;
	int m_sio_rtsb;
	int m_dfd_out;
	int m_dfd_in;
	int m_tape_ctr;

	// video state
	UINT8 m_hrs;                    // HR picture start scanline
	UINT8 m_fgctl;                  // HR foreground control

	// timers
	emu_timer *m_ctc_timer;
	emu_timer *m_cassette_timer;
};


// ======================> abc800m_state

class abc800m_state : public abc800_state
{
public:
	abc800m_state(const machine_config &mconfig, device_type type, const char *tag) :
		abc800_state(mconfig, type, tag),
		m_crtc(*this, MC6845_TAG),
		m_palette(*this, "palette"),
		m_fgctl_prom(*this, "hru2"),
		m_char_rom(*this, MC6845_TAG)
	{ }

	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_memory_region m_fgctl_prom;
	required_memory_region m_char_rom;

	DECLARE_DRIVER_INIT(driver_init);

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_DIRECT_UPDATE_MEMBER( direct_update_handler );
	MC6845_UPDATE_ROW( abc800m_update_row );
};


// ======================> abc800c_state

class abc800c_state : public abc800_state
{
public:
	abc800c_state(const machine_config &mconfig, device_type type, const char *tag) :
		abc800_state(mconfig, type, tag),
		m_trom(*this, SAA5052_TAG),
		m_palette(*this, "palette"),
		m_fgctl_prom(*this, "hru2")
	{ }

	required_device<saa5052_device> m_trom;
	required_device<palette_device> m_palette;
	required_memory_region m_fgctl_prom;

	DECLARE_DRIVER_INIT(driver_init);

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	offs_t translate_trom_offset(offs_t offset);
	void hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( char_ram_r );
	DECLARE_DIRECT_UPDATE_MEMBER( direct_update_handler );
	DECLARE_PALETTE_INIT( abc800c );
};


// ======================> abc802_state

class abc802_state : public abc800_state
{
public:
	abc802_state(const machine_config &mconfig, device_type type, const char *tag) :
		abc800_state(mconfig, type, tag),
		m_crtc(*this, MC6845_TAG),
		m_palette(*this, "palette"),
		m_char_rom(*this, MC6845_TAG),
		m_config(*this, "CONFIG")
	{ }

	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_memory_region m_char_rom;
	required_ioport m_config;

	DECLARE_DRIVER_INIT(driver_init);
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void bankswitch();

	DECLARE_READ8_MEMBER( pling_r );
	DECLARE_WRITE_LINE_MEMBER( lrs_w );
	DECLARE_WRITE_LINE_MEMBER( mux80_40_w );
	DECLARE_WRITE_LINE_MEMBER( vs_w );
	DECLARE_DIRECT_UPDATE_MEMBER( direct_update_handler );
	MC6845_UPDATE_ROW( abc802_update_row );

	// cpu state
	int m_lrs;                  // low RAM select

	// video state
	int m_flshclk_ctr;          // flash clock counter
	int m_flshclk;              // flash clock
	int m_80_40_mux;            // 40/80 column mode
};


// ======================> abc806_state

class abc806_state : public abc800_state
{
public:
	abc806_state(const machine_config &mconfig, device_type type, const char *tag) :
		abc800_state(mconfig, type, tag),
		m_crtc(*this, MC6845_TAG),
		m_palette(*this, "palette"),
		m_rtc(*this, E0516_TAG),
		m_rad_prom(*this, "rad"),
		m_hru2_prom(*this, "hru"),
		m_char_rom(*this, MC6845_TAG),
		m_attr_ram(*this, "attr_ram")
	{ }

	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<e0516_device> m_rtc;
	required_memory_region m_rad_prom;
	required_memory_region m_hru2_prom;
	required_memory_region m_char_rom;
	optional_shared_ptr<UINT8> m_attr_ram;

	DECLARE_DRIVER_INIT(driver_init);
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void bankswitch();
	void hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( mai_r );
	DECLARE_WRITE8_MEMBER( mao_w );
	DECLARE_WRITE8_MEMBER( hrs_w );
	DECLARE_WRITE8_MEMBER( hrc_w );
	DECLARE_READ8_MEMBER( charram_r );
	DECLARE_WRITE8_MEMBER( charram_w );
	DECLARE_READ8_MEMBER( ami_r );
	DECLARE_WRITE8_MEMBER( amo_w );
	DECLARE_READ8_MEMBER( cli_r );
	DECLARE_WRITE8_MEMBER( sso_w );
	DECLARE_READ8_MEMBER( sti_r );
	DECLARE_WRITE8_MEMBER( sto_w );
	DECLARE_WRITE_LINE_MEMBER( keydtr_w );
	DECLARE_WRITE_LINE_MEMBER( hs_w );
	DECLARE_WRITE_LINE_MEMBER( vs_w );
	DECLARE_DIRECT_UPDATE_MEMBER( direct_update_handler );
	DECLARE_PALETTE_INIT( abc806 );
	MC6845_UPDATE_ROW( abc806_update_row );

	// memory state
	int m_keydtr;               // keyboard DTR
	int m_eme;                  // extended memory enable
	UINT8 m_map[16];            // memory page register

	// video state
	int m_txoff;                // text display enable
	int m_40;                   // 40/80 column mode
	int m_flshclk_ctr;          // flash clock counter
	int m_flshclk;              // flash clock
	UINT8 m_attr_data;          // attribute data latch
	UINT8 m_hrc[16];            // HR palette
	UINT8 m_sync;               // line synchronization delay
	UINT8 m_v50_addr;           // vertical sync PROM address
	int m_hru2_a8;              // HRU II PROM address line 8
	UINT32 m_vsync_shift;       // vertical sync shift register
	int m_vsync;                // vertical sync
	int m_d_vsync;              // delayed vertical sync
};



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

/*----------- defined in video/abc800.c -----------*/

MACHINE_CONFIG_EXTERN(abc800m_video);
MACHINE_CONFIG_EXTERN(abc800c_video);

/*----------- defined in video/abc802.c -----------*/

MACHINE_CONFIG_EXTERN(abc802_video);

/*----------- defined in video/abc806.c -----------*/

MACHINE_CONFIG_EXTERN(abc806_video);

#endif
