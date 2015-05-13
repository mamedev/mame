// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __VIXEN__
#define __VIXEN__

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/i8155.h"
#include "machine/i8251.h"
#include "bus/ieee488/ieee488.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "sound/discrete.h"

#define Z8400A_TAG      "5f"
#define FDC1797_TAG     "5n"
#define P8155H_TAG      "2n"
#define P8155H_IO_TAG   "c7"
#define P8251A_TAG      "c3"
#define DISCRETE_TAG    "discrete"
#define SCREEN_TAG      "screen"
#define RS232_TAG       "rs232"

class vixen_state : public driver_device
{
public:
	vixen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z8400A_TAG),
			m_fdc(*this, FDC1797_TAG),
			m_io_i8155(*this, P8155H_IO_TAG),
			m_usart(*this, P8251A_TAG),
			m_discrete(*this, DISCRETE_TAG),
			m_ieee488(*this, IEEE488_TAG),
			m_palette(*this, "palette"),
			m_ram(*this, RAM_TAG),
			m_floppy0(*this, FDC1797_TAG":0"),
			m_floppy1(*this, FDC1797_TAG":1"),
			m_rs232(*this, RS232_TAG),
			m_rom(*this, Z8400A_TAG),
			m_sync_rom(*this, "video"),
			m_char_rom(*this, "chargen"),
			m_video_ram(*this, "video_ram"),
			m_key(*this, "KEY"),
			m_cmd_d1(0),
			m_fdint(0),
			m_vsync(0),
			m_srq(1),
			m_atn(1),
			m_rxrdy(0),
			m_txrdy(0)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<fd1797_t> m_fdc;
	required_device<i8155_device> m_io_i8155;
	required_device<i8251_device> m_usart;
	required_device<discrete_sound_device> m_discrete;
	required_device<ieee488_device> m_ieee488;
	required_device<palette_device> m_palette;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<rs232_port_device> m_rs232;
	required_region_ptr<UINT8> m_rom;
	required_region_ptr<UINT8> m_sync_rom;
	required_region_ptr<UINT8> m_char_rom;
	required_shared_ptr<UINT8> m_video_ram;
	required_ioport_array<8> m_key;

	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void update_interrupt();

	DECLARE_WRITE8_MEMBER( ctl_w );
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_WRITE8_MEMBER( cmd_w );
	DECLARE_READ8_MEMBER( ieee488_r );
	DECLARE_READ8_MEMBER( port3_r );
	DECLARE_READ8_MEMBER( i8155_pa_r );
	DECLARE_WRITE8_MEMBER( i8155_pb_w );
	DECLARE_WRITE8_MEMBER( i8155_pc_w );
	DECLARE_WRITE8_MEMBER( io_i8155_pb_w );
	DECLARE_WRITE8_MEMBER( io_i8155_pc_w );
	DECLARE_WRITE_LINE_MEMBER( io_i8155_to_w );
	DECLARE_WRITE_LINE_MEMBER( srq_w );
	DECLARE_WRITE_LINE_MEMBER( atn_w );
	DECLARE_WRITE_LINE_MEMBER( rxrdy_w );
	DECLARE_WRITE_LINE_MEMBER( txrdy_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_intrq_w );
	DIRECT_UPDATE_MEMBER(vixen_direct_update_handler);

	// memory state
	int m_reset;

	// keyboard state
	UINT8 m_col;

	// interrupt state
	int m_cmd_d0;
	int m_cmd_d1;

	bool m_fdint;
	int m_vsync;

	int m_srq;
	int m_atn;
	int m_enb_srq_int;
	int m_enb_atn_int;

	int m_rxrdy;
	int m_txrdy;
	int m_int_clk;
	int m_enb_xmt_int;
	int m_enb_rcv_int;
	int m_enb_ring_int;

	// video state
	int m_alt;
	int m_256;

	DECLARE_DRIVER_INIT(vixen);
	TIMER_DEVICE_CALLBACK_MEMBER(vsync_tick);
	IRQ_CALLBACK_MEMBER(vixen_int_ack);
};

#endif
