// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m4510.h

    65ce02 with a mmu and a cia integrated

***************************************************************************/

#ifndef MAME_CPU_M6502_M4510_H
#define MAME_CPU_M6502_M4510_H

#pragma once

#include "m65ce02.h"

class m4510_device : public m65ce02_device {
public:
	m4510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

	bool get_nomap() const { return nomap; }

	uint8_t get_port();
	void set_pulls(uint8_t pullup, uint8_t pulldown);

	auto read_callback() { return read_port.bind(); }
	auto write_callback() { return write_port.bind(); }

protected:
	uint32_t map_offset[2];
	uint8_t map_enable;
	bool nomap;

	class mi_4510 : public memory_interface {
	public:
		m4510_device *base;

		mi_4510(m4510_device *base);
		virtual ~mi_4510() {}
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
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	inline uint32_t map(uint16_t adr) {
		if(map_enable & (1 << (adr >> 13))) {
			nomap = false;
			return adr + map_offset[adr >> 15];
		}
		nomap = true;
		return adr;
	}

	uint8_t dir_r();
	void dir_w(uint8_t data);
	uint8_t port_r();
	void port_w(uint8_t data);

	void init_port();
	void update_port();

#define O(o) void o ## _full(); void o ## _partial()

	// 4510 opcodes
	O(eom_imp);
	O(map_imp);

#undef O
};

enum {
	M4510_IRQ_LINE = m6502_device::IRQ_LINE,
	M4510_NMI_LINE = m6502_device::NMI_LINE
};

DECLARE_DEVICE_TYPE(M4510, m4510_device)

#endif // MAME_CPU_M6502_M4510_H
