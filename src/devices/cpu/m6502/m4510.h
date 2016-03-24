// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m4510.h

    65ce02 with a mmu and a port

***************************************************************************/

#ifndef __M4510_H__
#define __M4510_H__

#include "m65ce02.h"

class m4510_device : public m65ce02_device {
public:
	m4510_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

	bool get_nomap() const { return nomap; }

protected:
	UINT32 map_offset[2];
	UINT8 map_enable;
	bool nomap;

	class mi_4510_normal : public memory_interface {
	public:
		m4510_device *base;

		mi_4510_normal(m4510_device *base);
		virtual ~mi_4510_normal() {}
		virtual UINT8 read(UINT16 adr) override;
		virtual UINT8 read_sync(UINT16 adr) override;
		virtual UINT8 read_arg(UINT16 adr) override;
		virtual void write(UINT16 adr, UINT8 val) override;
	};

	class mi_4510_nd : public mi_4510_normal {
	public:
		mi_4510_nd(m4510_device *base);
		virtual ~mi_4510_nd() {}
		virtual UINT8 read_sync(UINT16 adr) override;
		virtual UINT8 read_arg(UINT16 adr) override;
	};

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual bool memory_translate(address_spacenum spacenum, int intention, offs_t &address) override;

	inline UINT32 map(UINT16 adr) {
		if(map_enable & (1 << (adr >> 13))) {
			nomap = false;
			return adr + map_offset[adr >> 15];
		}
		nomap = true;
		return adr;
	}

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

extern const device_type M4510;

#endif
