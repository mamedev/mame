// license:BSD-3-Clause
// copyright-holders:hap
/*

Hitachi HMCS400 MCU family cores

TODO:
- do the LAW/LWA opcodes not work on early revisions of HMCS400? the 1988 user
  manual warns that the W register is write-only, and that there is no efficient
  way to save this register when using interrupts
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
// rev 2 apparently added LAW/LWA opcodes

// HMCS402C/CL/AC, 64 pins DP-64S or FP-64, 2Kx10 ROM, 160x4 RAM
DEFINE_DEVICE_TYPE(HD614022, hd614022_device, "hd614022", "Hitachi HD614022") // C, rev 2
DEFINE_DEVICE_TYPE(HD614023, hd614023_device, "hd614023", "Hitachi HD614023") // C, rev 1
DEFINE_DEVICE_TYPE(HD614025, hd614025_device, "hd614025", "Hitachi HD614025") // CL, rev 2
DEFINE_DEVICE_TYPE(HD614026, hd614026_device, "hd614026", "Hitachi HD614026") // CL, rev 1
DEFINE_DEVICE_TYPE(HD614028, hd614028_device, "hd614028", "Hitachi HD614028") // AC, rev 2
DEFINE_DEVICE_TYPE(HD614029, hd614029_device, "hd614029", "Hitachi HD614029") // AC, rev 1

// HMCS404C/CL/AC, 64 pins DP-64S or FP-64, 4Kx10 ROM, 256x4 RAM
DEFINE_DEVICE_TYPE(HD614042, hd614042_device, "hd614042", "Hitachi HD614042") // C, rev 2
DEFINE_DEVICE_TYPE(HD614043, hd614043_device, "hd614043", "Hitachi HD614043") // C, rev 1
DEFINE_DEVICE_TYPE(HD614045, hd614045_device, "hd614045", "Hitachi HD614045") // CL, rev 2
DEFINE_DEVICE_TYPE(HD614046, hd614046_device, "hd614046", "Hitachi HD614046") // CL, rev 1
DEFINE_DEVICE_TYPE(HD614048, hd614048_device, "hd614048", "Hitachi HD614048") // AC, rev 2
DEFINE_DEVICE_TYPE(HD614049, hd614049_device, "hd614049", "Hitachi HD614049") // AC, rev 1

// HMCS408C/CL/AC, 64 pins DP-64S or FP-64, 8Kx10 ROM, 512x4 RAM
DEFINE_DEVICE_TYPE(HD614080, hd614080_device, "hd614080", "Hitachi HD614080") // C, rev 2
DEFINE_DEVICE_TYPE(HD614081, hd614081_device, "hd614081", "Hitachi HD614081") // C, rev 1
DEFINE_DEVICE_TYPE(HD614085, hd614085_device, "hd614085", "Hitachi HD614085") // CL, rev 2
DEFINE_DEVICE_TYPE(HD614086, hd614086_device, "hd614086", "Hitachi HD614086") // CL, rev 1
DEFINE_DEVICE_TYPE(HD614088, hd614088_device, "hd614088", "Hitachi HD614088") // AC, rev 2
DEFINE_DEVICE_TYPE(HD614089, hd614089_device, "hd614089", "Hitachi HD614089") // AC, rev 1


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
	m_divider(8)
{ }

hmcs400_cpu_device::~hmcs400_cpu_device() { }


hmcs402_cpu_device::hmcs402_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	hmcs400_cpu_device(mconfig, type, tag, owner, clock, 0x800, 96)
{ }

hd614022_device::hd614022_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614022, tag, owner, clock)
{ }
hd614023_device::hd614023_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614023, tag, owner, clock)
{ }
hd614025_device::hd614025_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614025, tag, owner, clock)
{ }
hd614026_device::hd614026_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614026, tag, owner, clock)
{ }
hd614028_device::hd614028_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614028, tag, owner, clock)
{ }
hd614029_device::hd614029_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs402_cpu_device(mconfig, HD614029, tag, owner, clock)
{ }


hmcs404_cpu_device::hmcs404_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	hmcs400_cpu_device(mconfig, type, tag, owner, clock, 0x1000, 192)
{ }

hd614042_device::hd614042_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614042, tag, owner, clock)
{ }
hd614043_device::hd614043_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614043, tag, owner, clock)
{ }
hd614045_device::hd614045_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614045, tag, owner, clock)
{ }
hd614046_device::hd614046_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614046, tag, owner, clock)
{ }
hd614048_device::hd614048_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614048, tag, owner, clock)
{ }
hd614049_device::hd614049_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs404_cpu_device(mconfig, HD614049, tag, owner, clock)
{ }


hmcs408_cpu_device::hmcs408_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	hmcs400_cpu_device(mconfig, type, tag, owner, clock, 0x2000, 448)
{
	m_has_div = true;
}

hd614080_device::hd614080_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614080, tag, owner, clock)
{ }
hd614081_device::hd614081_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614081, tag, owner, clock)
{ }
hd614085_device::hd614085_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614085, tag, owner, clock)
{ }
hd614086_device::hd614086_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614086, tag, owner, clock)
{ }
hd614088_device::hd614088_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614088, tag, owner, clock)
{ }
hd614089_device::hd614089_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hmcs408_cpu_device(mconfig, HD614089, tag, owner, clock)
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
	m_param = 0;

	// register for savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_op));
	save_item(NAME(m_param));

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

u16 hmcs400_cpu_device::fetch()
{
	u16 data = m_program->read_word(m_pc) & 0x3ff;
	m_pc = (m_pc + 1) & 0x3fff;
	m_icount--;

	return data;
}

void hmcs400_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		// fetch next opcode
		m_prev_pc = m_pc;
		debugger_instruction_hook(m_pc);
		m_op = fetch();

		// 2-byte opcodes
		if ((m_op >= 0x100 && m_op < 0x140) || (m_op >= 0x150 && m_op < 0x1b0))
			m_param = fetch();

		op_illegal();
	}
}
