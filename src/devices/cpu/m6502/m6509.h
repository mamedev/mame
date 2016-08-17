// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6509.h

    6502 with banking and extended address bus

***************************************************************************/

#ifndef __M6509_H__
#define __M6509_H__

#include "m6502.h"

class m6509_device : public m6502_device {
public:
	m6509_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

protected:
	class mi_6509_normal : public memory_interface {
	public:
		m6509_device *base;

		mi_6509_normal(m6509_device *base);
		virtual ~mi_6509_normal() {}
		virtual UINT8 read(UINT16 adr) override;
		virtual UINT8 read_9(UINT16 adr) override;
		virtual UINT8 read_sync(UINT16 adr) override;
		virtual UINT8 read_arg(UINT16 adr) override;
		virtual void write(UINT16 adr, UINT8 val) override;
		virtual void write_9(UINT16 adr, UINT8 val) override;
	};

	class mi_6509_nd : public mi_6509_normal {
	public:
		mi_6509_nd(m6509_device *base);
		virtual ~mi_6509_nd() {}
		virtual UINT8 read_sync(UINT16 adr) override;
		virtual UINT8 read_arg(UINT16 adr) override;
	};

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void state_export(const device_state_entry &entry) override;

	UINT32 XPC;

	UINT8 bank_i, bank_y;

	UINT8 bank_i_r() { return bank_i; }
	UINT8 bank_y_r() { return bank_y; }
	void bank_i_w(UINT8 data) { bank_i = data; }
	void bank_y_w(UINT8 data) { bank_y = data; }

	UINT32 adr_in_bank_i(UINT16 adr) { return adr | ((bank_i & 0xf) << 16); }
	UINT32 adr_in_bank_y(UINT16 adr) { return adr | ((bank_y & 0xf) << 16); }

#define O(o) void o ## _full(); void o ## _partial()

	// 6509 opcodes
	O(lda_9_idy);
	O(sta_9_idy);

#undef O
};

enum {
	M6509_IRQ_LINE = m6502_device::IRQ_LINE,
	M6509_NMI_LINE = m6502_device::NMI_LINE,
	M6509_SET_OVERFLOW = m6502_device::V_LINE
};

enum {
	M6509_BI = M6502_IR+1,
	M6509_BY
};

extern const device_type M6509;

#endif
