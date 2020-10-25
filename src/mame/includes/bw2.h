// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_INCLUDES_BW2_H
#define MAME_INCLUDES_BW2_H

#pragma once

#include "bus/bw2/exp.h"
#include "cpu/z80/z80.h"
#include "formats/bw2_dsk.h"
#include "imagedev/floppy.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "video/msm6255.h"
#include "emupal.h"
#include "rendlay.h"

#define Z80_TAG         "ic1"
#define I8255A_TAG      "ic4"
#define WD2797_TAG      "ic5"
#define MSM6255_TAG     "ic49"
#define CENTRONICS_TAG  "centronics"
#define RS232_TAG       "rs232"
#define SCREEN_TAG      "screen"

class bw2_state : public driver_device
{
public:
	bw2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_uart(*this, "ic7"),
		m_fdc(*this, WD2797_TAG),
		m_lcdc(*this, MSM6255_TAG),
		m_pit(*this, "ic6"),
		m_centronics(*this, CENTRONICS_TAG),
		m_exp(*this, BW2_EXPANSION_SLOT_TAG),
		m_ram(*this, RAM_TAG),
		m_floppy0(*this, WD2797_TAG":0"),
		m_floppy1(*this, WD2797_TAG":1"),
		m_floppy(nullptr),
		m_rom(*this, Z80_TAG),
		m_y(*this, "Y%u", 0),
		m_video_ram(*this, "videoram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_uart;
	required_device<wd2797_device> m_fdc;
	required_device<msm6255_device> m_lcdc;
	required_device<pit8253_device> m_pit;
	required_device<centronics_device> m_centronics;
	required_device<bw2_expansion_slot_device> m_exp;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;
	required_memory_region m_rom;
	required_ioport_array<10> m_y;

	virtual void machine_start() override;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void ppi_pa_w(uint8_t data);
	uint8_t ppi_pb_r();
	void ppi_pc_w(uint8_t data);
	uint8_t  ppi_pc_r();

	DECLARE_WRITE_LINE_MEMBER( mtron_w );

	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	// keyboard state
	uint8_t m_kb;

	// memory state
	uint8_t m_bank;

	// floppy state
	int m_mtron;
	int m_mfdbk;

	// video state
	optional_shared_ptr<uint8_t> m_video_ram;
	void bw2_palette(palette_device &palette) const;

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	int m_centronics_busy;
	void bw2(machine_config &config);
	void bw2_io(address_map &map);
	void bw2_mem(address_map &map);
	void lcdc_map(address_map &map);
};

#endif // MAME_INCLUDES_BW2_H
