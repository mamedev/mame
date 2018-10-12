// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    didisasm.c

    Device disasm interfaces.

***************************************************************************/

#include "emu.h"
#include "didisasm.h"


namespace {

class device_disasm_indirect : public util::disasm_interface
{
public:
	device_disasm_indirect(std::unique_ptr<util::disasm_interface> &&upper, dasm_override_delegate const &dasm_override)
		: m_disasm(std::move(upper)), m_dasm_override(dasm_override)
	{
	}
	virtual ~device_disasm_indirect() = default;

	virtual u32 interface_flags() const override { return m_disasm->interface_flags(); }
	virtual u32 page_address_bits() const override { return m_disasm->page_address_bits(); }
	virtual u32 page2_address_bits() const override { return m_disasm->page2_address_bits(); }
	virtual offs_t pc_linear_to_real(offs_t pc) const override { return m_disasm->pc_linear_to_real(pc); }
	virtual offs_t pc_real_to_linear(offs_t pc) const override { return m_disasm->pc_real_to_linear(pc); }
	virtual u8  decrypt8 (u8  value, offs_t pc, bool opcode) const override { return m_disasm->decrypt8(value, pc, opcode); }
	virtual u16 decrypt16(u16 value, offs_t pc, bool opcode) const override { return m_disasm->decrypt16(value, pc, opcode); }
	virtual u32 decrypt32(u32 value, offs_t pc, bool opcode) const override { return m_disasm->decrypt32(value, pc, opcode); }
	virtual u64 decrypt64(u64 value, offs_t pc, bool opcode) const override { return m_disasm->decrypt64(value, pc, opcode); }

	virtual u32 opcode_alignment() const override { return m_disasm->opcode_alignment(); }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params) override
	{
		offs_t const result(m_dasm_override(stream, pc, opcodes, params));
		return result ? result : m_disasm->disassemble(stream, pc, opcodes, params);
	}

private:
	std::unique_ptr<util::disasm_interface> const m_disasm;
	dasm_override_delegate const &m_dasm_override;
};

} // anonymous namespace


device_disasm_interface::device_disasm_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "disasm"), m_disasm(), m_dasm_override(), m_started(false)
{
}

util::disasm_interface &device_disasm_interface::get_disassembler()
{
	if (!m_disasm) {
		if (m_dasm_override.isnull())
			m_disasm = create_disassembler();
		else
			m_disasm.reset(new device_disasm_indirect(create_disassembler(), m_dasm_override));
	}
	return *m_disasm;
}

void device_disasm_interface::interface_pre_start()
{
	if (!m_started)
	{
		m_started = true;
		m_dasm_override.bind_relative_to(*device().owner());
	}
}
