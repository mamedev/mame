// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_KONAMI_KONAMI1_H
#define MAME_KONAMI_KONAMI1_H

#pragma once


#include "cpu/m6809/m6809.h"
#include "cpu/m6809/6x09dasm.h"

class konami1_device : public m6809_base_device {
public:
	konami1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_encryption_boundary(uint16_t adr);

protected:
	class mi_konami1 : public mi_default {
	public:
		uint16_t m_boundary;
		mi_konami1(uint16_t boundary);
		virtual ~mi_konami1() {}
		virtual uint8_t read_opcode(uint16_t adr) override;
	};

	class disassembler : public m6809_disassembler {
	public:
		uint16_t m_boundary;
		disassembler(uint16_t boundary) : m6809_disassembler(), m_boundary(boundary) {}
		virtual ~disassembler() = default;
		virtual u32 interface_flags() const override;
		virtual u8 decrypt8(u8 value, offs_t pc, bool opcode) const override;
	};

	uint16_t m_boundary;

	virtual void device_start() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

DECLARE_DEVICE_TYPE(KONAMI1, konami1_device)

#endif // MAME_KONAMI_KONAMI1_H
