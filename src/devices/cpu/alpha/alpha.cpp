// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the Digital Alpha CPU family.
 *
 * Sources
 *
 *   http://bitsavers.org/pdf/dec/alpha/21064-aa-RISC%20Microprocessor%20Preliminary%20Data%20Sheet-apr92.pdf
 *   http://bitsavers.org/pdf/dec/alpha/Sites_AlphaAXPArchitectureReferenceManual_2ed_1995.pdf
 *
 * TODO
 *   - skeleton only (wip)
 */

#include "emu.h"
#include "debugger.h"
#include "alpha.h"
#include "alphad.h"

#define LOG_GENERAL   (1U << 0)
#define LOG_EXCEPTION (1U << 1)
#define LOG_SYSCALLS  (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_EXCEPTION)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(DEC_21064, dec_21064_device, "21064", "DEC Alpha 21064")

dec_21064_device::dec_21064_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: alpha_device(mconfig, DEC_21064, tag, owner, clock)
{
}

alpha_device::alpha_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_main_config("main", ENDIANNESS_LITTLE, 64, 32, 0)
	, m_icount(0)
{
}

void alpha_device::device_start()
{
	set_icountptr(m_icount);

	save_item(NAME(m_pc));
	save_item(NAME(m_r));
	save_item(NAME(m_f));

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();

	state_add(64, "PC", m_pc);

	// integer registers
	for (unsigned i = 0; i < 32; i++)
		state_add(i, util::string_format("R%d", i).c_str(), m_r[i]);

	// floating point registers
	for (unsigned i = 0; i < 32; i++)
		state_add(i + 32, util::string_format("F%d", i).c_str(), m_f[i]);
}

void alpha_device::device_reset()
{
}

void alpha_device::execute_run()
{
	m_icount = 0;
}

void alpha_device::execute_set_input(int inputnum, int state)
{
}

device_memory_interface::space_config_vector alpha_device::memory_space_config() const
{
	return space_config_vector { std::make_pair(0, &m_main_config)  };
}

bool alpha_device::memory_translate(int spacenum, int intention, offs_t &address)
{
	return true;
}

std::unique_ptr<util::disasm_interface> alpha_device::create_disassembler()
{
	return std::make_unique<alpha_disassembler>();
}
