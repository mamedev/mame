// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// DSP 563xx base emulation

#include "emu.h"
#include "dsp563xx.h"
#include "dsp563xxd.h"

dsp563xx_device::dsp563xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock,
								 address_map_constructor map_p, address_map_constructor map_x, address_map_constructor map_y) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_p_config("p", ENDIANNESS_LITTLE, 32, 24, -2, map_p),
	m_x_config("x", ENDIANNESS_LITTLE, 32, 24, -2, map_x),
	m_y_config("y", ENDIANNESS_LITTLE, 32, 24, -2, map_y)
{
}

void dsp563xx_device::device_start()
{
	space(AS_P).cache(m_p);
	space(AS_X).specific(m_x);
	space(AS_Y).specific(m_y);

	state_add(STATE_GENPC,     "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_pc).noshow();
	state_add(0,               "PC",        m_pc);

	save_item(NAME(m_pc));
	save_item(NAME(m_omr));
	save_item(NAME(m_icount));

	set_icountptr(m_icount);
}

void dsp563xx_device::set_hard_omr(u8 mode)
{
	m_hard_omr = mode;
}

void dsp563xx_device::device_reset()
{
	m_omr = m_hard_omr;
	m_pc = get_reset_vector();
}

void dsp563xx_device::execute_run()
{
	debugger_instruction_hook(m_pc);
	m_icount = 0;
}

device_memory_interface::space_config_vector dsp563xx_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_P, &m_p_config),
		std::make_pair(AS_X, &m_x_config),
		std::make_pair(AS_Y, &m_y_config)
	};

}

void dsp563xx_device::state_import(const device_state_entry &entry)
{
}

void dsp563xx_device::state_export(const device_state_entry &entry)
{
}

void dsp563xx_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
}

std::unique_ptr<util::disasm_interface> dsp563xx_device::create_disassembler()
{
	return std::make_unique<dsp563xx_disassembler>();
}

