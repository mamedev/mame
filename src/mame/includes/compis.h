// license:???
// copyright-holders:Per Ola Ingvarsson, Tomas Karlsson
/*****************************************************************************
 *
 * includes/compis.h
 *
 * machine driver header
 *
 * Per Ola Ingvarsson
 * Tomas Karlsson
 *
 ****************************************************************************/

#pragma once

#ifndef __COMPIS__
#define __COMPIS__

#include "emu.h"
#include "bus/isbx/isbx.h"
#include "cpu/i86/i186.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/cassette.h"
#include "machine/compiskb.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/i80130.h"
#include "machine/mm58274c.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/z80dart.h"
#include "video/upd7220.h"

#define I80186_TAG      "ic1"
#define I80130_TAG      "ic15"
#define I8251A_TAG      "ic59"
#define I8253_TAG       "ic60"
#define I8274_TAG       "ic65"
#define MM58174A_TAG    "ic66"
#define I8255_TAG       "ic69"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"
#define CASSETTE_TAG    "cassette"
#define CENTRONICS_TAG  "centronics"
#define ISBX_0_TAG      "isbx0"
#define ISBX_1_TAG      "isbx1"
#define SCREEN_TAG      "screen"
#define COMPIS_KEYBOARD_TAG "compiskb"

class compis_state : public driver_device
{
public:
	compis_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, I80186_TAG),
			m_osp(*this, I80130_TAG),
			m_pit(*this, I8253_TAG),
			m_ppi(*this, I8255_TAG),
			m_mpsc(*this, I8274_TAG),
			m_centronics(*this, CENTRONICS_TAG),
			m_uart(*this, I8251A_TAG),
			m_rtc(*this, MM58174A_TAG),
			m_crtc(*this, "upd7220"),
			m_palette(*this, "palette"),
			m_cassette(*this, CASSETTE_TAG),
			m_isbx0(*this, ISBX_0_TAG),
			m_isbx1(*this, ISBX_1_TAG),
			m_ram(*this, RAM_TAG),
			m_video_ram(*this, "video_ram"),
			m_s8(*this, "S8")
	{ }

	required_device<i80186_cpu_device> m_maincpu;
	required_device<i80130_device> m_osp;
	required_device<pit8253_device> m_pit;
	required_device<i8255_device> m_ppi;
	required_device<i8274_device> m_mpsc;
	required_device<centronics_device> m_centronics;
	required_device<i8251_device> m_uart;
	required_device<mm58274c_device> m_rtc;
	required_device<upd7220_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<cassette_image_device> m_cassette;
	required_device<isbx_slot_device> m_isbx0;
	required_device<isbx_slot_device> m_isbx1;
	required_device<ram_device> m_ram;
	required_shared_ptr<UINT16> m_video_ram;
	required_ioport m_s8;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_WRITE8_MEMBER( tape_mon_w );
	DECLARE_READ16_MEMBER( isbx0_tdma_r );
	DECLARE_WRITE16_MEMBER( isbx0_tdma_w );
	DECLARE_READ16_MEMBER( isbx1_tdma_r );
	DECLARE_WRITE16_MEMBER( isbx1_tdma_w );
	DECLARE_READ16_MEMBER( isbx0_cs_r );
	DECLARE_WRITE16_MEMBER( isbx0_cs_w );
	DECLARE_READ16_MEMBER( isbx0_dack_r );
	DECLARE_WRITE16_MEMBER( isbx0_dack_w );
	DECLARE_READ16_MEMBER( isbx1_cs_r );
	DECLARE_WRITE16_MEMBER( isbx1_cs_w );
	DECLARE_READ16_MEMBER( isbx1_dack_r );
	DECLARE_WRITE16_MEMBER( isbx1_dack_w );

	DECLARE_READ8_MEMBER( compis_irq_callback );

	DECLARE_READ8_MEMBER( ppi_pb_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );

	DECLARE_WRITE_LINE_MEMBER( tmr0_w );
	DECLARE_WRITE_LINE_MEMBER( tmr1_w );
	DECLARE_WRITE_LINE_MEMBER( tmr2_w );
	DECLARE_WRITE_LINE_MEMBER( tmr5_w );

	TIMER_DEVICE_CALLBACK_MEMBER( tape_tick );

	int m_centronics_busy;
	int m_centronics_select;

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_select);

	int m_tmr0;
	int m_unk_video;

	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
};



#endif
