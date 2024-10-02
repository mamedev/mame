// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_LUXOR_ABC800_H
#define MAME_LUXOR_ABC800_H

#pragma once

#include "emupal.h"
#include "softlist.h"
#include "speaker.h"
#include "bus/abcbus/abcbus.h"
#include "bus/abckb/abc800kb.h"
#include "bus/abckb/abckb.h"
#include "bus/rs232/printer.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/teletex800.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/74259.h"
#include "machine/e0516.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80sio.h"
#include "sound/discrete.h"
#include "video/mc6845.h"
#include "video/saa5050.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define ABC800_X01  XTAL(12'000'000)
#define ABC806_X02  XTAL(32'768)

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
#define ABC806_VIDEO_RAM_SIZE   0x20000

#define ABC800_CHAR_WIDTH   6
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
#define CASSETTE_TAG        "cassette"
#define RS232_A_TAG         "rs232a"
#define RS232_B_TAG         "rs232b"
#define ABC_KEYBOARD_PORT_TAG   "kb"
#define TIMER_CTC_TAG       "timer_ctc"
#define TIMER_CASSETTE_TAG  "timer_cass"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc800_state

class abc800_state : public driver_device
{
public:
	abc800_state(const machine_config &mconfig, device_type type, const char *tag, size_t char_ram_size) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_ctc(*this, Z80CTC_TAG),
		m_dart(*this, Z80DART_TAG),
		m_sio(*this, Z80SIO_TAG),
		m_discrete(*this, DISCRETE_TAG),
		m_cassette(*this, CASSETTE_TAG),
		m_quickload(*this, "quickload"),
		m_ram(*this, RAM_TAG),
		m_rom(*this, Z80_TAG),
		m_video_ram(*this, "video_ram", 0x4000, ENDIANNESS_LITTLE),
		m_char_ram(*this, "char_ram", 0x800, ENDIANNESS_LITTLE),
		m_io_sb(*this, "SB"),
		m_ctc_z0(0),
		m_sio_txcb(0),
		m_sio_txdb(1),
		m_sio_rtsb(1),
		m_dfd_out(0),
		m_tape_ctr(4),
		m_char_ram_size(char_ram_size)
	{ }

	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_dart;
	required_device<z80sio_device> m_sio;
	optional_device<discrete_sound_device> m_discrete;
	optional_device<cassette_image_device> m_cassette;
	required_device<snapshot_image_device> m_quickload;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	memory_share_creator<uint8_t> m_video_ram;
	memory_share_creator<uint8_t> m_char_ram;
	required_ioport m_io_sb;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void cassette_output_tick(int state);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	virtual uint8_t m1_r(offs_t offset);
	uint8_t pling_r();
	void hrs_w(uint8_t data);
	void hrc_w(uint8_t data);
	void ctc_z0_w(int state);
	void ctc_z1_w(int state);
	void sio_txdb_w(int state);
	void sio_dtrb_w(int state);
	void sio_rtsb_w(int state);
	void keydtr_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER( ctc_tick );
	TIMER_DEVICE_CALLBACK_MEMBER( cassette_input_tick );

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	// memory state
	int m_keydtr = 0;               // keyboard DTR
	bool m_fetch_charram = false;        // opcode fetched from character RAM region (0x7800-0x7fff)

	// serial state
	uint8_t m_sb = 0;
	int m_ctc_z0;
	int m_sio_txcb;
	int m_sio_txdb;
	int m_sio_rtsb;
	int m_dfd_out;
	int m_dfd_in = 0;
	int m_tape_ctr;

	// video state
	size_t m_char_ram_size = 0;
	uint8_t m_hrs = 0;                    // HR picture start scanline
	uint8_t m_fgctl = 0;                  // HR foreground control

	// timers
	emu_timer *m_cassette_timer;
	void common(machine_config &config);
	void abc800_m1(address_map &map) ATTR_COLD;
	void abc800_mem(address_map &map) ATTR_COLD;
	void abc800_io(address_map &map) ATTR_COLD;
	void abc800m_io(address_map &map) ATTR_COLD;
	void abc800c_io(address_map &map) ATTR_COLD;
};


// ======================> abc800m_state

class abc800m_state : public abc800_state
{
public:
	abc800m_state(const machine_config &mconfig, device_type type, const char *tag) :
		abc800_state(mconfig, type, tag, ABC800M_CHAR_RAM_SIZE),
		m_crtc(*this, MC6845_TAG),
		m_palette(*this, "palette"),
		m_fgctl_prom(*this, "hru2"),
		m_char_rom(*this, MC6845_TAG)
	{ }

	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_memory_region m_fgctl_prom;
	required_memory_region m_char_rom;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	MC6845_UPDATE_ROW( abc800m_update_row );
	void abc800m(machine_config &config);
	void abc800m_video(machine_config &config);
};


// ======================> abc800c_state

class abc800c_state : public abc800_state
{
public:
	abc800c_state(const machine_config &mconfig, device_type type, const char *tag) :
		abc800_state(mconfig, type, tag, ABC800C_CHAR_RAM_SIZE),
		m_trom(*this, SAA5052_TAG),
		m_palette(*this, "palette"),
		m_fgctl_prom(*this, "hru2")
	{ }

	required_device<saa5052_device> m_trom;
	required_device<palette_device> m_palette;
	required_memory_region m_fgctl_prom;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t char_ram_r(offs_t offset);
	void abc800c_palette(palette_device &palette) const;
	void abc800c(machine_config &config);
	void abc800c_video(machine_config &config);
};


// ======================> abc802_state

class abc802_state : public abc800_state
{
public:
	abc802_state(const machine_config &mconfig, device_type type, const char *tag) :
		abc800_state(mconfig, type, tag, ABC802_CHAR_RAM_SIZE),
		m_crtc(*this, MC6845_TAG),
		m_palette(*this, "palette"),
		m_char_rom(*this, MC6845_TAG),
		m_config(*this, "CONFIG")
	{ }

	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_memory_region m_char_rom;
	required_ioport m_config;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	void lrs_w(int state);
	void mux80_40_w(int state);
	void vs_w(int state);
	MC6845_UPDATE_ROW( abc802_update_row );

	// cpu state
	int m_lrs = 0;                  // low RAM select

	// video state
	int m_flshclk_ctr = 0;          // flash clock counter
	int m_flshclk = 0;              // flash clock
	int m_80_40_mux = 0;            // 40/80 column mode

	void abc802(machine_config &config);
	void abc802_video(machine_config &config);
	void abc802_m1(address_map &map) ATTR_COLD;
	void abc802_mem(address_map &map) ATTR_COLD;
	void abc802_io(address_map &map) ATTR_COLD;
};


// ======================> abc806_state

class abc806_state : public abc800_state
{
public:
	abc806_state(const machine_config &mconfig, device_type type, const char *tag) :
		abc800_state(mconfig, type, tag, ABC806_CHAR_RAM_SIZE),
		m_crtc(*this, MC6845_TAG),
		m_palette(*this, "palette"),
		m_rtc(*this, E0516_TAG),
		m_sto(*this, "sto"),
		m_rad_prom(*this, "rad"),
		m_hru2_prom(*this, "hru"),
		m_char_rom(*this, MC6845_TAG),
		m_attr_ram(*this, "attr_ram", 0x800, ENDIANNESS_LITTLE)
	{ }

	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<e0516_device> m_rtc;
	required_device<addressable_latch_device> m_sto;
	required_memory_region m_rad_prom;
	required_memory_region m_hru2_prom;
	required_memory_region m_char_rom;
	memory_share_creator<uint8_t> m_attr_ram;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void read_pal_p4(offs_t offset, bool m1l, bool xml, offs_t &m, bool &romd, bool &ramd, bool &hre, bool &vr);
	void hr_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t m1_r(offs_t offset) override;
	uint8_t mai_r(offs_t offset);
	void mao_w(offs_t offset, uint8_t data);
	void hrs_w(uint8_t data);
	void hrc_w(offs_t offset, uint8_t data);
	uint8_t charram_r(offs_t offset);
	void charram_w(offs_t offset, uint8_t data);
	uint8_t ami_r();
	void amo_w(uint8_t data);
	uint8_t cli_r(offs_t offset);
	void sso_w(uint8_t data);
	uint8_t sti_r();
	void sto_w(uint8_t data);
	void eme_w(int state);
	void _40_w(int state);
	void hru2_a8_w(int state);
	void prot_ini_w(int state);
	void txoff_w(int state);
	void prot_din_w(int state);
	void hs_w(int state);
	void vs_w(int state);
	void abc806_palette(palette_device &palette) const;
	MC6845_UPDATE_ROW( abc806_update_row );

	// memory state
	int m_eme = 0;                  // extended memory enable
	uint8_t m_map[16]{};            // memory page register

	// video state
	int m_txoff = 0;                // text display enable
	int m_40 = 0;                   // 40/80 column mode
	int m_flshclk_ctr = 0;          // flash clock counter
	int m_flshclk = 0;              // flash clock
	uint8_t m_attr_data = 0;          // attribute data latch
	uint8_t m_hrc[16]{};            // HR palette
	uint8_t m_sync = 0;               // line synchronization delay
	uint8_t m_v50_addr = 0;           // vertical sync PROM address
	int m_hru2_a8 = 0;              // HRU II PROM address line 8
	uint32_t m_vsync_shift = 0;       // vertical sync shift register
	int m_vsync = 0;                // vertical sync
	int m_d_vsync = 0;              // delayed vertical sync

	void abc806(machine_config &config);
	void abc806_video(machine_config &config);
	void abc806_io(address_map &map) ATTR_COLD;
	void abc806_mem(address_map &map) ATTR_COLD;
};

#endif // MAME_LUXOR_ABC800_H
