// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __TMC600__
#define __TMC600__


#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "bus/centronics/ctronics.h"
#include "machine/ram.h"
#include "sound/cdp1869.h"

#define SCREEN_TAG      "screen"
#define CDP1802_TAG     "cdp1802"
#define CDP1869_TAG     "cdp1869"
#define CENTRONICS_TAG  "centronics"

#define TMC600_PAGE_RAM_SIZE    0x400
#define TMC600_PAGE_RAM_MASK    0x3ff

class tmc600_state : public driver_device
{
public:
	tmc600_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, CDP1802_TAG),
			m_vis(*this, CDP1869_TAG),
			m_cassette(*this, "cassette"),
			m_centronics(*this, "centronics"),
			m_ram(*this, RAM_TAG),
			m_char_rom(*this, "chargen"),
			m_page_ram(*this, "page_ram"),
			m_color_ram(*this, "color_ram"),
			m_run(*this, "RUN"),
			m_y0(*this, "Y0"),
			m_y1(*this, "Y1"),
			m_y2(*this, "Y2"),
			m_y3(*this, "Y3"),
			m_y4(*this, "Y4"),
			m_y5(*this, "Y5"),
			m_y6(*this, "Y6"),
			m_y7(*this, "Y7")
	{ }

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1869_device> m_vis;
	required_device<cassette_image_device> m_cassette;
	required_device<centronics_device> m_centronics;
	required_device<ram_device> m_ram;
	required_region_ptr<UINT8> m_char_rom;
	required_shared_ptr<UINT8> m_page_ram;
	optional_shared_ptr<UINT8> m_color_ram;
	required_ioport m_run;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;

	virtual void machine_start() override;

	virtual void video_start() override;

	DECLARE_WRITE8_MEMBER( keyboard_latch_w );
	DECLARE_WRITE8_MEMBER( vismac_register_w );
	DECLARE_WRITE8_MEMBER( vismac_data_w );
	DECLARE_WRITE8_MEMBER( page_ram_w );
	DECLARE_READ_LINE_MEMBER( clear_r );
	DECLARE_READ_LINE_MEMBER( ef2_r );
	DECLARE_READ_LINE_MEMBER( ef3_r );
	DECLARE_WRITE_LINE_MEMBER( q_w );

	UINT8 get_color(UINT16 pma);

	// video state
	int m_vismac_reg_latch;     // video register latch
	int m_vismac_color_latch;   // color latch
	int m_vismac_bkg_latch;     // background color latch
	int m_blink;                // cursor blink

	// keyboard state
	ioport_port* m_key_row[8];
	int m_keylatch;             // key latch

	TIMER_DEVICE_CALLBACK_MEMBER(blink_tick);
	CDP1869_CHAR_RAM_READ_MEMBER(tmc600_char_ram_r);
	CDP1869_PCB_READ_MEMBER(tmc600_pcb_r);
};

// ---------- defined in video/tmc600.c ----------

MACHINE_CONFIG_EXTERN( tmc600_video );

#endif
