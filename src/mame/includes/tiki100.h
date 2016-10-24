// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __TIKI100__
#define __TIKI100__

#include "emu.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "bus/tiki100/exp.h"
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
	tiki100_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_ctc(*this, Z80CTC_TAG),
		m_fdc(*this, FD1797_TAG),
		m_pio(*this, Z80PIO_TAG),
		m_dart(*this, Z80DART_TAG),
		m_psg(*this, AY8912_TAG),
		m_ram(*this, RAM_TAG),
		m_floppy0(*this, FD1797_TAG":0"),
		m_floppy1(*this, FD1797_TAG":1"),
		m_cassette(*this, CASSETTE_TAG),
		m_centronics(*this, CENTRONICS_TAG),
		m_exp(*this, TIKI100_BUS_TAG),
		m_rom(*this, Z80_TAG),
		m_prom(*this, "u4"),
		m_video_ram(*this, "video_ram"),
		m_y(*this, "Y%u", 1),
		m_st_io(*this, "ST"),
		m_palette(*this, "palette"),
		m_rome(1),
		m_vire(1)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<fd1797_t> m_fdc;
	required_device<z80pio_device> m_pio;
	required_device<z80dart_device> m_dart;
	required_device<ay8912_device> m_psg;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<cassette_image_device> m_cassette;
	required_device<centronics_device> m_centronics;
	required_device<tiki100_bus_t> m_exp;
	required_memory_region m_rom;
	required_memory_region m_prom;
	optional_shared_ptr<uint8_t> m_video_ram;
	required_ioport_array<12> m_y;
	required_ioport m_st_io;
	required_device<palette_device> m_palette;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t mrq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mrq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t iorq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void iorq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void keyboard_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void video_mode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void system_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bar0_w(int state);
	void bar2_w(int state);
	void video_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t pio_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pio_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	void ctc_tick(timer_device &timer, void *ptr, int32_t param);
	void tape_tick(timer_device &timer, void *ptr, int32_t param);

	void write_centronics_ack(int state);
	void write_centronics_busy(int state);
	void write_centronics_perror(int state);

	void busrq_w(int state);

	enum
	{
		ROM0 = 0x01,
		ROM1 = 0x02,
		VIR  = 0x04,
		RAM  = 0x08
	};

	// memory state
	bool m_rome;
	bool m_vire;

	// video state
	uint8_t m_scroll;
	uint8_t m_mode;
	uint8_t m_palette_val;

	// keyboard state
	int m_keylatch;

	// printer state
	int m_centronics_ack;
	int m_centronics_busy;
	int m_centronics_perror;

	// serial state
	bool m_st;
};

#endif
