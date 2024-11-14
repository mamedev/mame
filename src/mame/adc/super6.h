// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_ADC_SUPER6_H
#define MAME_ADC_SUPER6_H

#pragma once

#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/z80daisy.h"
#include "machine/com8116.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"

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
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_ctc(*this, Z80CTC_TAG)
		, m_dart(*this, Z80DART_TAG)
		, m_dma(*this, Z80DMA_TAG)
		, m_pio(*this, Z80PIO_TAG)
		, m_fdc(*this, WD2793_TAG)
		, m_brg(*this, BR1945_TAG)
		, m_ram(*this, RAM_TAG)
		, m_floppy(*this, WD2793_TAG":%u", 0U)
		, m_rom(*this, Z80_TAG)
		, m_j7(*this, "J7")
	{ }

	void super6(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t fdc_r();
	void fdc_w(uint8_t data);
	void s100_w(uint8_t data);
	void bank0_w(uint8_t data);
	void bank1_w(uint8_t data);
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	uint8_t io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, uint8_t data);

	void super6_io(address_map &map) ATTR_COLD;
	void super6_mem(address_map &map) ATTR_COLD;

	void bankswitch();

	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_dart;
	required_device<z80dma_device> m_dma;
	required_device<z80pio_device> m_pio;
	required_device<wd2793_device> m_fdc;
	required_device<com8116_device> m_brg;
	required_device<ram_device> m_ram;
	required_device_array<floppy_connector, 2> m_floppy;
	required_region_ptr<uint8_t> m_rom;
	required_ioport m_j7;

	bool m_z80_wait = false;

	// memory state
	uint8_t m_s100 = 0;
	uint8_t m_bank0 = 0;
	uint8_t m_bank1 = 0;
};

#endif // MAME_ADC_SUPER6_H
