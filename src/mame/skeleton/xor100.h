// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_INCLUDES_XOR100_H
#define MAME_INCLUDES_XOR100_H

#pragma once

#include "bus/s100/s100.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/ram.h"
#include "machine/com8116.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "5b"
#define I8251_A_TAG     "12b"
#define I8251_B_TAG     "14b"
#define I8255A_TAG      "8a"
#define COM5016_TAG     "15c"
#define Z80CTC_TAG      "11b"
#define WD1795_TAG      "wd1795"
#define CENTRONICS_TAG  "centronics"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"
#define S100_TAG        "s100"

class xor100_state : public driver_device
{
public:
	xor100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_uart_a(*this, I8251_A_TAG)
		, m_uart_b(*this, I8251_B_TAG)
		, m_fdc(*this, WD1795_TAG)
		, m_ctc(*this, Z80CTC_TAG)
		, m_ram(*this, RAM_TAG)
		, m_centronics(*this, CENTRONICS_TAG)
		, m_s100(*this, S100_TAG)
		, m_floppy0(*this, WD1795_TAG":0")
		, m_floppy1(*this, WD1795_TAG":1")
		, m_floppy2(*this, WD1795_TAG":2")
		, m_floppy3(*this, WD1795_TAG":3")
		, m_rom(*this, Z80_TAG)
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
	{ }

	void xor100(machine_config &config);

private:
	void mmu_w(uint8_t data);
	void prom_toggle_w(uint8_t data);
	uint8_t prom_disable_r();
	uint8_t fdc_wait_r();
	void fdc_dcont_w(uint8_t data);
	void fdc_dsel_w(uint8_t data);
	void fdc_intrq_w(bool state);
	void fdc_drq_w(bool state);

	uint8_t i8255_pc_r();
	DECLARE_WRITE_LINE_MEMBER(ctc_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z2_w);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_select);

	void xor100_io(address_map &map);
	void xor100_mem(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void bankswitch();
	void post_load();

	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_uart_a;
	required_device<i8251_device> m_uart_b;
	required_device<fd1795_device> m_fdc;
	required_device<z80ctc_device> m_ctc;
	required_device<ram_device> m_ram;
	required_device<centronics_device> m_centronics;
	required_device<s100_bus_device> m_s100;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_memory_region m_rom;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;

	// memory state
	int m_mode = 0;
	int m_bank = 0;

	// floppy state
	bool m_fdc_irq = false;
	bool m_fdc_drq = false;
	int m_fdc_dden = 0;

	int m_centronics_busy = 0;
	int m_centronics_select = 0;
};

#endif // MAME_INCLUDES_XOR100_H
