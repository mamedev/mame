// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __XOR100__
#define __XOR100__

#include "emu.h"
#include "bus/s100/s100.h"
#include "cpu/z80/z80.h"
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

class xor100_state : public driver_device
{
public:
	xor100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_dbrg(*this, COM5016_TAG),
			m_uart_a(*this, I8251_A_TAG),
			m_uart_b(*this, I8251_B_TAG),
			m_fdc(*this, WD1795_TAG),
			m_ctc(*this, Z80CTC_TAG),
			m_ram(*this, RAM_TAG),
			m_centronics(*this, CENTRONICS_TAG),
			m_s100(*this, S100_TAG),
			m_floppy0(*this, WD1795_TAG":0"),
			m_floppy1(*this, WD1795_TAG":1"),
			m_floppy2(*this, WD1795_TAG":2"),
			m_floppy3(*this, WD1795_TAG":3"),
			m_rom(*this, Z80_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<com8116_device> m_dbrg;
	required_device<i8251_device> m_uart_a;
	required_device<i8251_device> m_uart_b;
	required_device<fd1795_t> m_fdc;
	required_device<z80ctc_device> m_ctc;
	required_device<ram_device> m_ram;
	required_device<centronics_device> m_centronics;
	required_device<s100_bus_t> m_s100;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_memory_region m_rom;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void mmu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void prom_toggle_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t prom_disable_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void baud_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fdc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fdc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t fdc_wait_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fdc_dcont_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fdc_dsel_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fdc_intrq_w(bool state);
	void fdc_drq_w(bool state);

	void bankswitch();
	void post_load();

	// memory state
	int m_mode;
	int m_bank;

	// floppy state
	bool m_fdc_irq;
	bool m_fdc_drq;
	int m_fdc_dden;
	void com5016_fr_w(int state);
	void com5016_ft_w(int state);
	uint8_t i8255_pc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ctc_z0_w(int state);
	void ctc_z1_w(int state);
	void ctc_z2_w(int state);

	int m_centronics_busy;
	int m_centronics_select;
	void write_centronics_busy(int state);
	void write_centronics_select(int state);
};

#endif
