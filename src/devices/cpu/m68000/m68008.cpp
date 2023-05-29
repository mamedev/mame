// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "m68008.h"
#include "m68kdasm.h"

DEFINE_DEVICE_TYPE(M68008,      m68008_device,      "m68008",       "Motorola MC68008") // 48-pin plastic or ceramic DIP
DEFINE_DEVICE_TYPE(M68008FN,    m68008fn_device,    "m68008fn",     "Motorola MC68008FN") // 52-pin PLCC

std::unique_ptr<util::disasm_interface> m68008_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68008);
}

std::unique_ptr<util::disasm_interface> m68008fn_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68008);
}

m68008_device::m68008_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68008_device(mconfig, M68008, tag, owner, clock)
{
}

m68008_device::m68008_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: m68000_device(mconfig, type, tag, owner, clock),
	  m_mmu8(nullptr)
{
	m_cpu_space_config.m_addr_width = m_cpu_space_config.m_logaddr_width = 20;
	m_program_config.m_addr_width = m_program_config.m_logaddr_width = 20;
	m_opcodes_config.m_addr_width = m_opcodes_config.m_logaddr_width = 20;
	m_uprogram_config.m_addr_width = m_uprogram_config.m_logaddr_width = 20;
	m_uopcodes_config.m_addr_width = m_uopcodes_config.m_logaddr_width = 20;
	m_cpu_space_config.m_data_width = 8;
	m_program_config.m_data_width = 8;
	m_opcodes_config.m_data_width = 8;
	m_uprogram_config.m_data_width = 8;
	m_uopcodes_config.m_data_width = 8;

	m_disable_specifics = true;
}

void m68008_device::device_start()
{
	m68000_device::device_start();

	m_s_program->specific(m_r_program8);
	m_s_opcodes->specific(m_r_opcodes8);
	m_s_uprogram->specific(m_r_uprogram8);
	m_s_uopcodes->specific(m_r_uopcodes8);
	m_s_cpu_space->specific(m_cpu_space8);

	// Theoretically UB, in practice works, the alternative (putting
	// everything in m68000_device) is annoying
	if(m_mmu8) {
		m_handlers_f = reinterpret_cast<const handler *>(s_handlers_if8);
		m_handlers_p = reinterpret_cast<const handler *>(s_handlers_ip8);
	} else {
		m_handlers_f = reinterpret_cast<const handler *>(s_handlers_df8);
		m_handlers_p = reinterpret_cast<const handler *>(s_handlers_dp8);
	}

}

void m68008_device::update_user_super()
{
	if(m_sr & SR_S) {
		m_sp = 16;
		m_program8 = m_r_program8;
		m_opcodes8 = m_r_opcodes8;
		if(m_mmu8)
			m_mmu8->set_super(true);
	} else {
		m_sp = 15;
		m_program8 = m_r_uprogram8;
		m_opcodes8 = m_r_uopcodes8;
		if(m_mmu8)
			m_mmu8->set_super(false);
	}
}


m68008fn_device::m68008fn_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68008_device(mconfig, M68008FN, tag, owner, clock)
{
	m_cpu_space_config.m_addr_width = m_cpu_space_config.m_logaddr_width = 22;
	m_program_config.m_addr_width = m_program_config.m_logaddr_width = 22;
	m_opcodes_config.m_addr_width = m_opcodes_config.m_logaddr_width = 22;
	m_uprogram_config.m_addr_width = m_uprogram_config.m_logaddr_width = 22;
	m_uopcodes_config.m_addr_width = m_uopcodes_config.m_logaddr_width = 22;
	m_cpu_space_config.m_data_width = 8;
	m_program_config.m_data_width = 8;
	m_opcodes_config.m_data_width = 8;
	m_uprogram_config.m_data_width = 8;
	m_uopcodes_config.m_data_width = 8;
}

void m68008fn_device::device_start()
{
	m68008_device::device_start();
}
