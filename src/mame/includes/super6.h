// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __SUPER6__
#define __SUPER6__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/com8116.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"

#define Z80_TAG         "u30"
#define Z80CTC_TAG      "u20"
#define Z80DART_TAG     "u38"
#define Z80DMA_TAG      "u21"
#define Z80PIO_TAG      "u7"
#define WD2793_TAG      "u27"
#define BR1945_TAG      "u31"
#define SCREEN_TAG      "screen"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"

class super6_state : public driver_device
{
public:
	super6_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_ctc(*this, Z80CTC_TAG),
			m_dart(*this, Z80DART_TAG),
			m_dma(*this, Z80DMA_TAG),
			m_pio(*this, Z80PIO_TAG),
			m_fdc(*this, WD2793_TAG),
			m_brg(*this, BR1945_TAG),
			m_ram(*this, RAM_TAG),
			m_floppy0(*this, WD2793_TAG":0"),
			m_floppy1(*this, WD2793_TAG":1"),
			m_rom(*this, Z80_TAG),
			m_j7(*this, "J7")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_dart;
	required_device<z80dma_device> m_dma;
	required_device<z80pio_device> m_pio;
	required_device<wd2793_t> m_fdc;
	required_device<com8116_device> m_brg;
	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_region_ptr<UINT8> m_rom;
	required_ioport m_j7;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ8_MEMBER( fdc_r );
	DECLARE_WRITE8_MEMBER( fdc_w );
	DECLARE_WRITE8_MEMBER( s100_w );
	DECLARE_WRITE8_MEMBER( bank0_w );
	DECLARE_WRITE8_MEMBER( bank1_w );
	DECLARE_WRITE8_MEMBER( baud_w );
	DECLARE_WRITE_LINE_MEMBER( fr_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_intrq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );

	void bankswitch();

	// memory state
	UINT8 m_s100;
	UINT8 m_bank0;
	UINT8 m_bank1;
	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
};

#endif
