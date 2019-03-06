// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    National Semiconductor High-Performance microController (HPC)

    Currently this device is just a stub with no actual execution core.

****************************************************************************

    HPC feature options by part number

    HPC16xxx    Military temperature range (-55°C to +125°C)
    HPC26xxx    Automotive temperature range (-40°C to +105°C)
    HPC36xxx    Industrial temperature range (-40°C to +85°C)
    HPC46xxx    Commercial temperature range (0°C to +75°C)

    HPCxx1xx    8-channel A/D converter
    HPCxx0xx    No A/D converter

    HPCxxx6x    16k bytes of on-chip ROM
    HPCxxx8x    8k bytes of on-chip ROM
    HPCxxx0x    No on-chip ROM

    HPCxxxx4    512 bytes of on-chip RAM
    HPCxxxx3    256 bytes of on-chip RAM

***************************************************************************/

#include "emu.h"
#include "hpc.h"
#include "hpcdasm.h"

// device type definitions
DEFINE_DEVICE_TYPE(HPC46003, hpc46003_device, "hpc46003", "HPC46003")
DEFINE_DEVICE_TYPE(HPC46104, hpc46104_device, "hpc46104", "HPC46104")


void hpc46003_device::internal_map(address_map &map)
{
	map(0x0000, 0x00bf).ram();
	map(0x00c0, 0x00c0).rw(FUNC(hpc46003_device::psw_r), FUNC(hpc46003_device::psw_w));
	map(0x00c4, 0x00cf).ram().share("core_regs");
	// TODO: many other internal registers
	map(0x01c0, 0x01ff).ram();
}

void hpc46104_device::internal_map(address_map &map)
{
	map(0x0000, 0x00bf).ram();
	map(0x00c0, 0x00c0).rw(FUNC(hpc46104_device::psw_r), FUNC(hpc46104_device::psw_w));
	map(0x00c4, 0x00cf).ram().share("core_regs");
	// TODO: many other internal registers
	map(0x01c0, 0x02ff).ram();
}

std::unique_ptr<util::disasm_interface> hpc46003_device::create_disassembler()
{
	return std::make_unique<hpc16083_disassembler>();
}

std::unique_ptr<util::disasm_interface> hpc46104_device::create_disassembler()
{
	return std::make_unique<hpc16164_disassembler>();
}


hpc_device::hpc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 16, 0, map)
	, m_program(nullptr)
	, m_core_regs(*this, "core_regs")
	, m_psw(0)
	, m_icount(0)
{
}

hpc46003_device::hpc46003_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: hpc_device(mconfig, HPC46003, tag, owner, clock, address_map_constructor(FUNC(hpc46003_device::internal_map), this))
{
}

hpc46104_device::hpc46104_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: hpc_device(mconfig, HPC46104, tag, owner, clock, address_map_constructor(FUNC(hpc46104_device::internal_map), this))
{
}

device_memory_interface::space_config_vector hpc_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
	};
}


void hpc_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	set_icountptr(m_icount);

	state_add(HPC_PSW, "PSW", m_psw);
	state_add(HPC_SP, "SP", m_core_regs[0]);
	state_add(HPC_PC, "PC", m_core_regs[1]);
	state_add(STATE_GENPC, "GENPC", m_core_regs[1]).callimport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_core_regs[1]).callimport().noshow();
	state_add(HPC_A, "A", m_core_regs[2]);
	state_add(HPC_K, "K", m_core_regs[3]);
	state_add(HPC_B, "B", m_core_regs[4]);
	state_add(HPC_X, "X", m_core_regs[5]);

	save_item(NAME(m_psw));
}

void hpc_device::device_reset()
{
	m_psw = 0x00;
	std::fill_n(&m_core_regs[0], 6, 0x0000);
}


u8 hpc_device::psw_r()
{
	return m_psw;
}

void hpc_device::psw_w(u8 data)
{
	m_psw = data;
}


void hpc_device::execute_run()
{
	m_core_regs[1] = m_program->read_word(0xfffe);
	debugger_instruction_hook(m_core_regs[1]);

	m_icount = 0;
}

void hpc_device::execute_set_input(int inputnum, int state)
{
	// TODO
}
