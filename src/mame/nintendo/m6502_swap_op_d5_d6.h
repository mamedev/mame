// license:BSD-3-Clause
// copyright-holders:David Shah, David Haywood
/***************************************************************************

    m6502_swap_op_d5_d6.h

    6502 / RP2A03 with instruction scrambling

***************************************************************************/

#ifndef MAME_M6502_SWAP_OP_D5_D6_H
#define MAME_M6502_SWAP_OP_D5_D6_H

#pragma once

#include "cpu/m6502/rp2a03.h"
#include "cpu/m6502/rp2a03d.h"

class m6502_swap_op_d5_d6 : public m6502_device {
public:
	m6502_swap_op_d5_d6(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_encryption_state(bool state);
protected:
	class mi_decrypt : public mi_default {
	public:

		bool m_encryption_enabled;

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

	virtual void device_reset() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};

class rp2a03_core_swap_op_d5_d6 : public rp2a03_core_device {
public:
	rp2a03_core_swap_op_d5_d6(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_encryption_state(bool state);
protected:
	class mi_decrypt : public mi_default {
	public:

		bool m_encryption_enabled;

		virtual ~mi_decrypt() {}
		virtual uint8_t read_sync(uint16_t adr) override;

		uint8_t descramble(uint8_t op);
	};

	class disassembler : public rp2a03_disassembler {
	public:
		mi_decrypt *mintf;

		disassembler(mi_decrypt *m);
		virtual ~disassembler() = default;
		virtual u32 interface_flags() const override;
		virtual u8 decrypt8(u8 value, offs_t pc, bool opcode) const override;
	};

	virtual void device_reset() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
};


DECLARE_DEVICE_TYPE(M6502_SWAP_OP_D5_D6, m6502_swap_op_d5_d6)
DECLARE_DEVICE_TYPE(RP2A03_CORE_SWAP_OP_D5_D6, rp2a03_core_swap_op_d5_d6)

#endif // MAME_M6502_SWAP_OP_D5_D6_H
