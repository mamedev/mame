// license:BSD-3-Clause
// copyright-holders:David Shah, David Haywood
/***************************************************************************

    m6502_vh2009.h

    6502 with instruction scrambling

***************************************************************************/

#ifndef MAME_CPU_M6502_VH2009_H
#define MAME_CPU_M6502_VH2009_H

#pragma once

#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m6502d.h"

class m6502_vh2009 : public m6502_device {
public:
	m6502_vh2009(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	class mi_decrypt : public mi_default {
	public:

		bool m_scramble_en = false;
		bool m_next_scramble = false;

		virtual ~mi_decrypt() {}
		virtual uint8_t read_sync(uint16_t adr) override;

		uint8_t descramble(uint8_t op);
	};

	class disassembler : public m6502_disassembler {
	public:
		mi_decrypt *mintf;

		disassembler(mi_decrypt *m);
		virtual ~disassembler() = default;
		virtual u32 interface_flags() const override;
		virtual u8 decrypt8(u8 value, offs_t pc, bool opcode) const override;
	};

	virtual void device_reset() override;
	virtual void device_start() override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

DECLARE_DEVICE_TYPE(M6502_VH2009, m6502_vh2009)

#endif // MAME_CPU_M6502_VH2009_H
