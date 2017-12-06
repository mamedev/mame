// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    didisasm.c

    Device disasm interfaces.

***************************************************************************/

#include "emu.h"


device_disasm_interface::device_disasm_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "disasm")
{
}

util::disasm_interface *device_disasm_interface::get_disassembler()
{
	if(!m_disasm) {
		if(m_dasm_override.isnull())
			m_disasm.reset(create_disassembler());
		else
			m_disasm = std::make_unique<device_disasm_indirect>(create_disassembler(), m_dasm_override);
	}
	return m_disasm.get();
}

void device_disasm_interface::interface_pre_start()
{
	m_dasm_override.bind_relative_to(*device().owner());
}

void device_disasm_interface::set_dasm_override(dasm_override_delegate dasm_override)
{
	m_dasm_override = dasm_override;
}

device_disasm_indirect::device_disasm_indirect(util::disasm_interface *upper, dasm_override_delegate &dasm_override) : m_dasm_override(dasm_override)
{
	m_disasm.reset(upper);
}

u32 device_disasm_indirect::interface_flags() const
{
	return m_disasm->interface_flags();
}

u32 device_disasm_indirect::page_address_bits() const
{
	return m_disasm->page_address_bits();
}

u32 device_disasm_indirect::page2_address_bits() const
{
	return m_disasm->page2_address_bits();
}

offs_t device_disasm_indirect::pc_linear_to_real(offs_t pc) const
{
	return m_disasm->pc_linear_to_real(pc);
}

offs_t device_disasm_indirect::pc_real_to_linear(offs_t pc) const
{
	return m_disasm->pc_real_to_linear(pc);
}

u8  device_disasm_indirect::decrypt8 (u8  value, offs_t pc, bool opcode) const
{
	return m_disasm->decrypt8(value, pc, opcode);
}

u16 device_disasm_indirect::decrypt16(u16 value, offs_t pc, bool opcode) const
{
	return m_disasm->decrypt16(value, pc, opcode);
}

u32 device_disasm_indirect::decrypt32(u32 value, offs_t pc, bool opcode) const
{
	return m_disasm->decrypt32(value, pc, opcode);
}

u64 device_disasm_indirect::decrypt64(u64 value, offs_t pc, bool opcode) const
{
	return m_disasm->decrypt64(value, pc, opcode);
}

u32 device_disasm_indirect::opcode_alignment() const
{
	return m_disasm->opcode_alignment();
}

offs_t device_disasm_indirect::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t result = m_dasm_override(stream, pc, opcodes, params);
	if(!result)
		result = m_disasm->disassemble(stream, pc, opcodes, params);
	return result;
}


