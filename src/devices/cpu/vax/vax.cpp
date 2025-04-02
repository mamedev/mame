// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    DEC VAX CPUs

    Currently these devices are just stubs with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "vax.h"
#include "vaxdasm.h"

// device type definitions
DEFINE_DEVICE_TYPE(KD32A, kd32a_device, "kd32a", "DEC KD32-A MicroVAX I CPU")
DEFINE_DEVICE_TYPE(DC333, dc333_device, "dc333", "DEC DC333 MicroVAX II CPU")
DEFINE_DEVICE_TYPE(DC341, dc341_device, "dc341", "DEC DC341 CVAX CPU")


vax_cpu_device::vax_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int addrwidth)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, addrwidth, 0, 32, 9)
	, m_psl(0)
	, m_icount(0)
{
}

kd32a_device::kd32a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: vax_cpu_device(mconfig, KD32A, tag, owner, clock, 30)
{
}

dc333_device::dc333_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: vax_cpu_device(mconfig, DC333, tag, owner, clock, 32)
{
}

dc341_device::dc341_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: vax_cpu_device(mconfig, DC341, tag, owner, clock, 32)
{
}

std::unique_ptr<util::disasm_interface> vax_cpu_device::create_disassembler()
{
	return std::make_unique<vax_disassembler>();
}

device_memory_interface::space_config_vector vax_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
	};
}


void vax_cpu_device::device_start()
{
	address_space &space = this->space(AS_PROGRAM);
	space.specific(m_program_space);
	space.cache(m_cache);

	std::fill_n(&m_gpr[0], 16, 0);

	set_icountptr(m_icount);

	state_add(STATE_GENPC, "GENPC", m_gpr[15]).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_gpr[15]).noshow();
	for (int i = 0; i < 12; i++)
		state_add(VAX_R0 + i, util::string_format("R%d", i).c_str(), m_gpr[i]);
	state_add(VAX_AP, "AP", m_gpr[12]);
	state_add(VAX_FP, "FP", m_gpr[13]);
	state_add(VAX_SP, "SP", m_gpr[14]);
	state_add(VAX_PC, "PC", m_gpr[15]);
	state_add(VAX_PSL, "PSL", m_psl);

	save_item(NAME(m_gpr));
	save_item(NAME(m_psl));
}

void vax_cpu_device::device_reset()
{
	m_gpr[15] = 0;
	m_psl = 0x041f0000;
}

void dc341_device::device_reset()
{
	vax_cpu_device::device_reset();

	m_gpr[15] = 0x20040000; // 0x20060000 in run mode
}


void vax_cpu_device::execute_run()
{
	debugger_instruction_hook(m_gpr[15]);

	m_icount = 0;
}
