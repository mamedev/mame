// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6510.h

    6502 with 6 i/o pins, also known as 8500

***************************************************************************/

#ifndef MAME_CPU_M6502_M6510_H
#define MAME_CPU_M6502_M6510_H

#pragma once

#include "m6502.h"

class m6510_device : public m6502_device {
public:
	m6510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t get_port();
	void set_pulls(uint8_t pullup, uint8_t pulldown);

	auto read_callback() { return read_port.bind(); }
	auto write_callback() { return write_port.bind(); }

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

protected:
	m6510_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	class mi_6510 : public memory_interface {
	public:
		m6510_device *base;

		mi_6510(m6510_device *base);
		virtual ~mi_6510() {}
		virtual uint8_t read(uint16_t adr) override;
		virtual uint8_t read_sync(uint16_t adr) override;
		virtual uint8_t read_arg(uint16_t adr) override;
		virtual void write(uint16_t adr, uint8_t val) override;
	};

	devcb_read8  read_port;
	devcb_write8 write_port;

	uint8_t pullup, floating, dir, port, drive;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t dir_r();
	void dir_w(uint8_t data);
	uint8_t port_r();
	void port_w(uint8_t data);

	void init_port();
	void update_port();

#define O(o) void o ## _full(); void o ## _partial()

	// 6510 undocumented instructions in a C64 context
	// implementation follows what the test suites expect (usually an extra and)
	O(anc_10_imm);
	O(ane_10_imm);
	O(arr_10_imm);
	O(asr_10_imm);
	O(las_10_aby);
	O(lxa_10_imm);

#undef O
};

class m6508_device : public m6510_device {
public:
	m6508_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;

	class mi_6508 : public memory_interface {
	public:
		m6508_device *base;

		mi_6508(m6508_device *base);
		virtual ~mi_6508() {}
		virtual uint8_t read(uint16_t adr) override;
		virtual uint8_t read_sync(uint16_t adr) override;
		virtual uint8_t read_arg(uint16_t adr) override;
		virtual void write(uint16_t adr, uint8_t val) override;
	};

	std::unique_ptr<uint8_t[]> ram_page;
};

enum {
	M6510_IRQ_LINE = m6502_device::IRQ_LINE,
	M6510_NMI_LINE = m6502_device::NMI_LINE
};

DECLARE_DEVICE_TYPE(M6510, m6510_device)
DECLARE_DEVICE_TYPE(M6508, m6508_device)

#endif // MAME_CPU_M6502_M6510_H
