// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __BULLET__
#define __BULLET__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "bus/centronics/ctronics.h"
#include "machine/ram.h"
#include "bus/scsi/scsi.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"

#define Z80_TAG         "u20"
#define Z80CTC_TAG      "u1"
#define Z80DMA_TAG      "u50"
#define Z80DART_TAG     "u45"
#define Z80PIO_TAG      "z80pio"
#define MB8877_TAG      "u55"
#define CENTRONICS_TAG  "centronics"
#define SCSIBUS_TAG     "scsi"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"

class bullet_state : public driver_device
{
public:
	bullet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, Z80_TAG),
		m_ctc(*this, Z80CTC_TAG),
		m_dart(*this, Z80DART_TAG),
		m_dmac(*this, Z80DMA_TAG),
		m_fdc(*this, MB8877_TAG),
		m_ram(*this, RAM_TAG),
		m_floppy0(*this, MB8877_TAG":0"),
		m_floppy1(*this, MB8877_TAG":1"),
		m_floppy2(*this, MB8877_TAG":2"),
		m_floppy3(*this, MB8877_TAG":3"),
		m_floppy4(*this, MB8877_TAG":4"),
		m_floppy5(*this, MB8877_TAG":5"),
		m_floppy6(*this, MB8877_TAG":6"),
		m_floppy7(*this, MB8877_TAG":7"),
		m_floppy(nullptr),
		m_centronics(*this, CENTRONICS_TAG),
		m_rom(*this, Z80_TAG),
		m_sw1(*this, "SW1"),
		m_fdrdy(0),
		m_exdsk_sw(false),
		m_hdcon_sw(false)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_dart;
	required_device<z80dma_device> m_dmac;
	required_device<mb8877_t> m_fdc;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_device<floppy_connector> m_floppy4;
	required_device<floppy_connector> m_floppy5;
	required_device<floppy_connector> m_floppy6;
	required_device<floppy_connector> m_floppy7;
	floppy_image_device *m_floppy;
	required_device<centronics_device> m_centronics;
	required_memory_region m_rom;
	required_ioport m_sw1;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ8_MEMBER( mreq_r );
	DECLARE_WRITE8_MEMBER( mreq_w );
	DECLARE_READ8_MEMBER( info_r );
	DECLARE_READ8_MEMBER( brom_r );
	DECLARE_WRITE8_MEMBER( brom_w );
	DECLARE_READ8_MEMBER( win_r );
	DECLARE_WRITE8_MEMBER( wstrobe_w );
	DECLARE_WRITE8_MEMBER( exdsk_w );
	DECLARE_WRITE8_MEMBER( exdma_w );
	DECLARE_WRITE8_MEMBER( hdcon_w );
	DECLARE_WRITE8_MEMBER( segst_w );
	DECLARE_READ8_MEMBER( dma_mreq_r );
	DECLARE_WRITE8_MEMBER( dma_mreq_w );
	DECLARE_READ8_MEMBER( pio_pb_r );
	DECLARE_WRITE_LINE_MEMBER( dartardy_w );
	DECLARE_WRITE_LINE_MEMBER( dartbrdy_w );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_busy );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_perror );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_select );
	DECLARE_WRITE_LINE_MEMBER( write_centronics_fault );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );

	void update_dma_rdy();
	// memory state
	int m_segst;
	int m_brom;

	// DMA state
	UINT8 m_exdma;
	int m_buf;
	bool m_fdrdy;
	int m_dartardy;
	int m_dartbrdy;
	int m_winrdy;
	int m_exrdy1;
	int m_exrdy2;
	bool m_exdsk_sw;
	bool m_hdcon_sw;

	int m_centronics_busy;
	int m_centronics_perror;
	int m_centronics_select;
	int m_centronics_fault;

	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);
	DECLARE_WRITE_LINE_MEMBER(dart_rxtxca_w);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
};

class bulletf_state : public bullet_state
{
public:
	bulletf_state(const machine_config &mconfig, device_type type, const char *tag) :
		bullet_state(mconfig, type, tag),
		m_floppy8(*this, MB8877_TAG":8"),
		m_floppy9(*this, MB8877_TAG":9"),
		m_scsibus(*this, SCSIBUS_TAG),
		m_scsi_data_in(*this, "scsi_data_in"),
		m_scsi_data_out(*this, "scsi_data_out"),
		m_scsi_ctrl_in(*this, "scsi_ctrl_in")
	{
	}

	required_device<floppy_connector> m_floppy8;
	required_device<floppy_connector> m_floppy9;
	required_device<SCSI_PORT_DEVICE> m_scsibus;
	required_device<input_buffer_device> m_scsi_data_in;
	required_device<output_latch_device> m_scsi_data_out;
	required_device<input_buffer_device> m_scsi_ctrl_in;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ8_MEMBER( mreq_r );
	DECLARE_WRITE8_MEMBER( mreq_w );
	DECLARE_WRITE8_MEMBER( xdma0_w );
	DECLARE_WRITE8_MEMBER( xfdc_w );
	DECLARE_WRITE8_MEMBER( mbank_w );
	DECLARE_READ8_MEMBER( hwsts_r );
	DECLARE_READ8_MEMBER( scsi_r );
	DECLARE_WRITE8_MEMBER( scsi_w );

	DECLARE_READ8_MEMBER( dma_mreq_r );
	DECLARE_WRITE8_MEMBER( dma_mreq_w );
	DECLARE_WRITE8_MEMBER( pio_pa_w );
	DECLARE_WRITE_LINE_MEMBER( cstrb_w );
	DECLARE_WRITE_LINE_MEMBER( req_w );

	void update_dma_rdy();

	int m_rome;
	UINT8 m_xdma0;
	UINT8 m_mbank;
	int m_wack;
	int m_wrdy;
};

#endif
