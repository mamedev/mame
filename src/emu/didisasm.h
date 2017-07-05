// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    didisasm.h

    Device disassembly interfaces.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DIDISASM_H
#define MAME_EMU_DIDISASM_H

#include "disasmintf.h"

#define MCFG_DEVICE_DISASSEMBLE_OVERRIDE(_class, _func) \
	dynamic_cast<device_disasm_interface *>(device)->set_dasm_override(dasm_override_delegate(&_class::_func, #_class "::" #_func, nullptr, (_class *)nullptr));

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef device_delegate<offs_t (std::ostream &stream, offs_t pc, const util::disasm_interface::data_buffer &opcodes, const util::disasm_interface::data_buffer &params)> dasm_override_delegate;

// ======================> device_disasm_interface
	
// class representing interface-specific live disasm
class device_disasm_interface : public device_interface
{
public:
	// construction/destruction
	device_disasm_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_disasm_interface() = default;

	// Override
	void set_dasm_override(dasm_override_delegate dasm_override);

	// disassembler request
	util::disasm_interface *get_disassembler();

protected:
	// disassembler creation
	virtual util::disasm_interface *create_disassembler() = 0;

	// delegate resolving
	virtual void interface_pre_start() override;

private:
	std::unique_ptr<util::disasm_interface> m_disasm;
	dasm_override_delegate m_dasm_override;
};

// iterator
typedef device_interface_iterator<device_disasm_interface> disasm_interface_iterator;

class device_disasm_indirect : public util::disasm_interface
{
public:
	device_disasm_indirect(util::disasm_interface *upper, dasm_override_delegate &dasm_override);
	virtual ~device_disasm_indirect() = default;

	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual u32 page2_address_bits() const override;
	virtual offs_t pc_linear_to_real(offs_t pc) const override;
	virtual offs_t pc_real_to_linear(offs_t pc) const override;
	virtual u8  decrypt8 (u8  value, offs_t pc, bool opcode) const override;
	virtual u16 decrypt16(u16 value, offs_t pc, bool opcode) const override;
	virtual u32 decrypt32(u32 value, offs_t pc, bool opcode) const override;
	virtual u64 decrypt64(u64 value, offs_t pc, bool opcode) const override;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	std::unique_ptr<util::disasm_interface> m_disasm;
	dasm_override_delegate &m_dasm_override;
};

#endif  /* MAME_EMU_DIDISASM_H */
