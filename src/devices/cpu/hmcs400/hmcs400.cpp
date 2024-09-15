// license:BSD-3-Clause
// copyright-holders:hap
/*

Hitachi HMCS400 MCU family cores

TODO:
- what happens when accessing ROM/RAM out of address range? Hitachi documentation
  says 'unused', but maybe it's mirrored?

*/

#include "emu.h"
#include "hmcs400.h"
#include "hmcs400d.h"


//-------------------------------------------------
//  device types
//-------------------------------------------------

// C = standard
// CL = low-power
// AC = high-speed

// HMCS408, HMCS414, HMCS424 have a mask option for the system clock divider
// HMCS408 and newer MCUs add 2 opcodes: LAW and LWA

// HMCS402C/CL/AC, 64 pins DP-64S or FP-64, 2Kx10 ROM, 160x4 RAM
DEFINE_DEVICE_TYPE(HD614022, hd614022_device, "hd614022", "Hitachi HD614022") // C
DEFINE_DEVICE_TYPE(HD614025, hd614025_device, "hd614025", "Hitachi HD614025") // CL
DEFINE_DEVICE_TYPE(HD614028, hd614028_device, "hd614028", "Hitachi HD614028") // AC

// HMCS404C/CL/AC, 64 pins DP-64S or FP-64, 4Kx10 ROM, 256x4 RAM
DEFINE_DEVICE_TYPE(HD614042, hd614042_device, "hd614042", "Hitachi HD614042") // C
DEFINE_DEVICE_TYPE(HD614045, hd614045_device, "hd614045", "Hitachi HD614045") // CL
DEFINE_DEVICE_TYPE(HD614048, hd614048_device, "hd614048", "Hitachi HD614048") // AC

// HMCS408C/CL/AC, 64 pins DP-64S or FP-64, 8Kx10 ROM, 512x4 RAM
DEFINE_DEVICE_TYPE(HD614080, hd614080_device, "hd614080", "Hitachi HD614080") // C
DEFINE_DEVICE_TYPE(HD614085, hd614085_device, "hd614085", "Hitachi HD614085") // CL
DEFINE_DEVICE_TYPE(HD614088, hd614088_device, "hd614088", "Hitachi HD614088") // AC


//-------------------------------------------------
//  constructor
//-------------------------------------------------

hmcs400_cpu_device::hmcs400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 rom_size, u32 ram_size) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 16, 14, -1, address_map_constructor(FUNC(hmcs400_cpu_device::program_map), this)),
	m_data_config("data", ENDIANNESS_LITTLE, 8, 10, 0, address_map_constructor(FUNC(hmcs400_cpu_device::data_map), this)),
	m_rom_size(rom_size),
	m_ram_size(ram_size),
	m_has_div(false),
	m_has_law(false),
	m_divider(8)
{ }

hmcs400_cpu_device::~hmcs400_cpu_device() { }


hmcs402_cpu_device::hmcs402_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	hmcs400_cpu_device(mconfig, type, tag, owner, clock, 0x800, 96)
{ }

hd614022_device::hd614022_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614022, tag, owner, clock)
{ }
hd614025_device::hd614025_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614025, tag, owner, clock)
{ }
hd614028_device::hd614028_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614028, tag, owner, clock)
{ }


hmcs404_cpu_device::hmcs404_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	hmcs400_cpu_device(mconfig, type, tag, owner, clock, 0x1000, 192)
{ }

hd614042_device::hd614042_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614042, tag, owner, clock)
{ }
hd614045_device::hd614045_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614045, tag, owner, clock)
{ }
hd614048_device::hd614048_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614048, tag, owner, clock)
{ }


hmcs408_cpu_device::hmcs408_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	hmcs400_cpu_device(mconfig, type, tag, owner, clock, 0x2000, 448)
{
	m_has_div = true;
	m_has_law = true;
}

hd614080_device::hd614080_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614080, tag, owner, clock)
{ }
hd614085_device::hd614085_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614085, tag, owner, clock)
{ }
hd614088_device::hd614088_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614088, tag, owner, clock)
{ }


//-------------------------------------------------
//  initialization
//-------------------------------------------------

void hmcs400_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);

	// zerofill
	m_pc = 0;
	m_prev_pc = 0;
	m_op = 0;

	// register for savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_op));

	// register state for debugger
	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).formatstr("%04X").noshow();

	int state_count = 0;
	state_add(++state_count, "PC", m_pc).formatstr("%04X"); // 1

	set_icountptr(m_icount);
}

void hmcs400_cpu_device::device_reset()
{
	m_pc = 0;
}

std::unique_ptr<util::disasm_interface> hmcs400_cpu_device::create_disassembler()
{
	return std::make_unique<hmcs400_disassembler>();
}


//-------------------------------------------------
//  internal memory maps
//-------------------------------------------------

void hmcs400_cpu_device::program_map(address_map &map)
{
	map.unmap_value_high();
	map(0, m_rom_size - 1).rom();
}

void hmcs400_cpu_device::data_map(address_map &map)
{
	map(0x020, 0x020 + m_ram_size - 1).ram();
	map(0x3c0, 0x3ff).ram();
}

device_memory_interface::space_config_vector hmcs400_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void hmcs400_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		// fetch next opcode
		m_prev_pc = m_pc;
		debugger_instruction_hook(m_pc);
		m_op = m_program->read_word(m_pc) & 0x3ff;
		m_pc = (m_pc + 1) & 0x3fff;

		m_icount--;

		op_illegal();
	}
}
