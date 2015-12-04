// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __MIKROMIKKO__
#define __MIKROMIKKO__

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "formats/mm_dsk.h"
#include "machine/am9517a.h"
#include "machine/i8212.h"
#include "machine/mm1kb.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/z80dart.h"
#include "machine/upd765.h"
#include "video/i8275.h"
#include "video/upd7220.h"

#define SCREEN_TAG      "screen"
#define I8085A_TAG      "ic40"
#define I8212_TAG       "ic12"
#define I8237_TAG       "ic45"
#define I8253_TAG       "ic6"
#define UPD765_TAG      "ic15"
#define I8275_TAG       "ic59"
#define UPD7201_TAG     "ic11"
#define UPD7220_TAG     "ic101"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"
#define RS232_C_TAG     "rs232c"
#define KB_TAG          "kb"

class mm1_state : public driver_device
{
public:
	mm1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, I8085A_TAG),
		m_iop(*this, I8212_TAG),
		m_dmac(*this, I8237_TAG),
		m_pit(*this, I8253_TAG),
		m_crtc(*this, I8275_TAG),
		m_fdc(*this, UPD765_TAG),
		m_mpsc(*this, UPD7201_TAG),
		m_hgdc(*this, UPD7220_TAG),
		m_palette(*this, "palette"),
		m_floppy0(*this, UPD765_TAG ":0:525qd"),
		m_floppy1(*this, UPD765_TAG ":1:525qd"),
		m_rs232a(*this, RS232_A_TAG),
		m_rs232b(*this, RS232_B_TAG),
		m_rs232c(*this, RS232_C_TAG),
		m_ram(*this, RAM_TAG),
		m_rom(*this, I8085A_TAG),
		m_mmu_rom(*this, "address"),
		m_char_rom(*this, "chargen"),
		m_video_ram(*this, "video_ram"),
		m_a8(0),
		m_recall(0),
		m_dack3(1),
		m_tc(CLEAR_LINE),
		m_fdc_tc(0)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<i8212_device> m_iop;
	required_device<am9517a_device> m_dmac;
	required_device<pit8253_device> m_pit;
	required_device<i8275_device> m_crtc;
	required_device<upd765a_device> m_fdc;
	required_device<upd7201_device> m_mpsc;
	required_device<upd7220_device> m_hgdc;
	required_device<palette_device> m_palette;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;
	required_device<rs232_port_device> m_rs232a;
	required_device<rs232_port_device> m_rs232b;
	required_device<rs232_port_device> m_rs232c;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_memory_region m_mmu_rom;
	required_memory_region m_char_rom;
	required_shared_ptr<UINT16> m_video_ram;

	virtual void machine_start();
	virtual void machine_reset();

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE8_MEMBER( ls259_w );
	DECLARE_WRITE_LINE_MEMBER( dma_hrq_w );
	DECLARE_READ8_MEMBER( mpsc_dack_r );
	DECLARE_WRITE8_MEMBER( mpsc_dack_w );
	DECLARE_WRITE_LINE_MEMBER( dma_eop_w );
	DECLARE_WRITE_LINE_MEMBER( dack3_w );
	DECLARE_WRITE_LINE_MEMBER( itxc_w );
	DECLARE_WRITE_LINE_MEMBER( irxc_w );
	DECLARE_WRITE_LINE_MEMBER( auxc_w );
	DECLARE_WRITE_LINE_MEMBER( drq2_w );
	DECLARE_WRITE_LINE_MEMBER( drq1_w );
	DECLARE_READ_LINE_MEMBER( dsra_r );

	void update_tc();
	void fdc_intrq_w(bool state);
	void fdc_drq_w(bool state);

	int m_a8;

	// video state
	int m_llen;

	// serial state
	int m_intc;
	int m_rx21;
	int m_tx21;
	int m_rcl;

	// floppy state
	int m_recall;
	int m_dack3;
	int m_tc;
	int m_fdc_tc;

	DECLARE_FLOPPY_FORMATS( floppy_formats );
	I8275_DRAW_CHARACTER_MEMBER( crtc_display_pixels );
	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	DECLARE_PALETTE_INIT( mm1 );
};


//----------- defined in video/mikromik.c -----------

MACHINE_CONFIG_EXTERN( mm1m6_video );


#endif
