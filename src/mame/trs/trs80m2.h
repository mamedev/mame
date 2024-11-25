// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_TRS_TRS80M2_H
#define MAME_TRS_TRS80M2_H

#pragma once


#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/m68000/m68000.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/floppy.h"
#include "machine/am9519.h"
#include "machine/keyboard.h"
#include "machine/ram.h"
#include "trs80m2kb.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "video/mc6845.h"
#include "emupal.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "u12"
#define Z80CTC_TAG      "u19"
#define Z80DMA_TAG      "u20"
#define Z80PIO_TAG      "u22"
#define Z80SIO_TAG      "u18"
#define FD1791_TAG      "u6"
#define MC6845_TAG      "u11"
#define CENTRONICS_TAG  "j2"
#define M68000_TAG      "m16_u22"
#define AM9519A_TAG     "m16_u11"

class trs80m2_state : public driver_device
{
public:
	trs80m2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_ctc(*this, Z80CTC_TAG),
		m_dmac(*this, Z80DMA_TAG),
		m_pio(*this, Z80PIO_TAG),
		m_crtc(*this, MC6845_TAG),
		m_palette(*this, "palette"),
		m_fdc(*this, FD1791_TAG),
		m_centronics(*this, CENTRONICS_TAG),
		m_floppy0(*this, FD1791_TAG":0"),
		m_floppy1(*this, FD1791_TAG":1"),
		m_floppy2(*this, FD1791_TAG":2"),
		m_floppy3(*this, FD1791_TAG":3"),
		m_floppy(nullptr),
		m_ram(*this, RAM_TAG),
		m_kb(*this, TRS80M2_KEYBOARD_TAG),
		m_rom(*this, Z80_TAG),
		m_char_rom(*this, MC6845_TAG),
		m_video_ram(*this, "video_ram", 0x800, ENDIANNESS_LITTLE)
	{
	}

	static constexpr feature_type imperfect_features() { return feature::KEYBOARD; }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	void drvslt_w(uint8_t data);
	void rom_enable_w(uint8_t data);
	uint8_t keyboard_r();
	uint8_t rtc_r();
	uint8_t nmi_r();
	void nmi_w(uint8_t data);
	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);
	void de_w(int state);
	void vsync_w(int state);
	uint8_t pio_pa_r();
	void pio_pa_w(uint8_t data);
	void strobe_w(int state);
	void kb_clock_w(int state);
	void kbd_w(u8 data);

	MC6845_UPDATE_ROW( crtc_update_row );

	uint8_t io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, uint8_t data);

	void write_centronics_busy(int state);
	void write_centronics_fault(int state);
	void write_centronics_perror(int state);

	void trs80m2(machine_config &config);
	void m68000_mem(address_map &map) ATTR_COLD;
	void z80_io(address_map &map) ATTR_COLD;
	void z80_mem(address_map &map) ATTR_COLD;
protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dma_device> m_dmac;
	required_device<z80pio_device> m_pio;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<fd1791_device> m_fdc;
	required_device<centronics_device> m_centronics;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	floppy_image_device *m_floppy;
	required_device<ram_device> m_ram;
	required_device<trs80m2_keyboard_device> m_kb;
	required_memory_region m_rom;
	required_memory_region m_char_rom;
	memory_share_creator<uint8_t> m_video_ram;

	// memory state
	int m_boot_rom;
	int m_bank;
	int m_msel;

	// keyboard state
	uint8_t m_key_latch;
	uint8_t m_key_data;
	int m_key_bit;
	int m_kbclk;
	int m_kbdata;
	int m_kbirq;

	// video state
	int m_blnkvid;
	int m_80_40_char_en;
	int m_de;
	int m_rtc_int;
	int m_enable_rtc_int;

	int m_centronics_busy;
	int m_centronics_fault;
	int m_centronics_perror;
};

class trs80m16_state : public trs80m2_state
{
public:
	trs80m16_state(const machine_config &mconfig, device_type type, const char *tag)
		: trs80m2_state(mconfig, type, tag)
		, m_subcpu(*this, M68000_TAG)
		, m_uic(*this, AM9519A_TAG)
	{
	}

	void ual_w(uint8_t data);
	void tcl_w(uint8_t data);

	void trs80m16(machine_config &config);
	void m16_z80_io(address_map &map) ATTR_COLD;
protected:
	virtual void machine_start() override ATTR_COLD;

	required_device<cpu_device> m_subcpu;
	required_device<am9519_device> m_uic;

	uint16_t m_ual = 0;
	uint8_t m_limit[2];
	uint8_t m_offset[2];
};

#endif // MAME_TRS_TRS80M2_H
