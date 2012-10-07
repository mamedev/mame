#pragma once

#ifndef __BULLET__
#define __BULLET__


#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/flopdrv.h"
#include "machine/ctronics.h"
#include "machine/ram.h"
#include "machine/scsicb.h"
#include "machine/scsibus.h"
#include "machine/terminal.h"
#include "machine/wd17xx.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"

#define Z80_TAG			"u20"
#define Z80CTC_TAG		"u1"
#define Z80DMA_TAG		"u50"
#define Z80DART_TAG		"u45"
#define Z80PIO_TAG		"z80pio"
#define MB8877_TAG		"u55"
#define CENTRONICS_TAG	"centronics"
#define SCSIBUS_TAG		"scsi"

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
		  m_floppy0(*this, FLOPPY_0),
		  m_floppy1(*this, FLOPPY_1),
		  m_terminal(*this, TERMINAL_TAG),
		  m_centronics(*this, CENTRONICS_TAG),
		  m_fdrdy(0)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_dart;
	required_device<z80dma_device> m_dmac;
	required_device<device_t> m_fdc;
	required_device<ram_device> m_ram;
	required_device<device_t> m_floppy0;
	required_device<device_t> m_floppy1;
	required_device<serial_terminal_device> m_terminal;
	required_device<centronics_device> m_centronics;

	virtual void machine_start();
	virtual void machine_reset();

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
	DECLARE_WRITE8_MEMBER( dart_w );
	DECLARE_READ8_MEMBER( dma_mreq_r );
	DECLARE_WRITE8_MEMBER( dma_mreq_w );
	DECLARE_READ8_MEMBER( pio_pb_r );
	DECLARE_WRITE_LINE_MEMBER( dartardy_w );
	DECLARE_WRITE_LINE_MEMBER( dartbrdy_w );
	DECLARE_WRITE_LINE_MEMBER( fdrdy_w );

	void bankswitch();
	void update_dma_rdy();

	// memory state
	int m_segst;
	int m_brom;

	// DMA state
	UINT8 m_exdma;
	int m_buf;
	int m_fdrdy;
	int m_dartardy;
	int m_dartbrdy;
	int m_winrdy;
	int m_exrdy1;
	int m_exrdy2;
	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);
	DECLARE_WRITE_LINE_MEMBER(dart_rxtxca_w);
};

class bulletf_state : public bullet_state
{
public:
	bulletf_state(const machine_config &mconfig, device_type type, const char *tag)
		: bullet_state(mconfig, type, tag),
		  m_scsibus(*this, SCSIBUS_TAG ":host")
	{ }

	required_device<scsicb_device> m_scsibus;

	virtual void machine_start();
	virtual void machine_reset();

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
	DECLARE_READ8_MEMBER( pio_pa_r );
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
