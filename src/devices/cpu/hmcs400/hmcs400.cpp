// license:BSD-3-Clause
// copyright-holders:hap
/*

Hitachi HMCS400 MCU family cores

It's the successor to HMCS40, it was mainly used in consumer electronics, not
much in games.

Compared to HMCS40, it accepts a higher clock speed, and it has more versatile
peripherals, like a serial interface. The opcodes were mostly kept the same.
They added an extra RAM addressing mode, and interrupt-related opcodes were
removed (interrupt flags are via memory-mapped I/O).

TODO:
- do the LAW/LWA opcodes not work on early revisions of HMCS400? the 1988 user
  manual warns that the W register is write-only, and that there is no efficient
  way to save this register when using interrupts
- what happens when accessing ROM/RAM out of address range? Hitachi documentation
  says 'unused', but maybe it's mirrored?
- current I/O ports are hardcoded for HMS402/4/8, which will need to be changed
  when other MCU types are added

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
	m_divider(8),
	m_read_r(*this, 0),
	m_write_r(*this),
	m_read_d(*this, 0),
	m_write_d(*this)
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
	m_sp = 0;
	m_op = 0;
	m_param = 0;
	m_i = 0;

	m_a = 0;
	m_b = 0;
	m_w = 0;
	m_x = 0;
	m_spx = 0;
	m_y = 0;
	m_spy = 0;
	m_st = 0;
	m_ca = 0;

	// register for savestates
	save_item(NAME(m_pc));
	save_item(NAME(m_prev_pc));
	save_item(NAME(m_sp));
	save_item(NAME(m_op));

	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_w));
	save_item(NAME(m_x));
	save_item(NAME(m_spx));
	save_item(NAME(m_y));
	save_item(NAME(m_spy));
	save_item(NAME(m_st));
	save_item(NAME(m_ca));

	save_item(NAME(m_r));
	save_item(NAME(m_d));

	// register state for debugger
	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).formatstr("%04X").noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_st).formatstr("%2s").noshow();

	m_state_count = 0;
	state_add(++m_state_count, "PC", m_pc).formatstr("%04X"); // 1
	state_add(++m_state_count, "SP", m_sp).formatstr("%03X"); // 2
	state_add(++m_state_count, "A", m_a).formatstr("%01X"); // 3
	state_add(++m_state_count, "B", m_b).formatstr("%01X"); // 4
	state_add(++m_state_count, "W", m_w).formatstr("%01X"); // 5
	state_add(++m_state_count, "X", m_x).formatstr("%01X"); // 6
	state_add(++m_state_count, "SPX", m_spx).formatstr("%01X"); // 7
	state_add(++m_state_count, "Y", m_y).formatstr("%01X"); // 8
	state_add(++m_state_count, "SPY", m_spy).formatstr("%01X"); // 9

	state_add(++m_state_count, "ST", m_st).formatstr("%01X").noshow(); // 10
	state_add(++m_state_count, "CA", m_ca).formatstr("%01X").noshow(); // 11

	set_icountptr(m_icount);
}

void hmcs400_cpu_device::device_reset()
{
	m_pc = 0;
	m_sp = 0x3ff;
	m_st = 1;

	// all I/O ports set to input
	reset_io();
}


//-------------------------------------------------
//  disasm
//-------------------------------------------------

void hmcs400_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c",
				m_ca ? 'C':'c',
				m_st ? 'S':'s'
			);
			break;

		default: break;
	}
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
//  i/o ports
//-------------------------------------------------

void hmcs400_cpu_device::reset_io()
{
	// D4-D15 are high-voltage
	m_d_mask = m_d = 0x000f;
	m_write_d(m_d_mask);

	for (int i = 0; i < 10; i++)
	{
		// R0-R2 and RA are high-voltage
		u8 mask = (i >= 3 && i <= 9) ? 0xf : 0;

		m_r_mask[i] = m_r[i] = mask;
		m_write_r[i](i, mask);
	}
}

u8 hmcs400_cpu_device::read_r(u8 index)
{
	// reads from write-only or non-existent ports are invalid
	bool write_only = index == 0 || (index >= 6 && index <= 8);
	if (write_only || index > 10)
	{
		logerror("read from %s port R%X at $%04X\n", write_only ? "output" : "unknown", index, m_prev_pc);
		return 0xf;
	}

	u8 mask = (index == 10) ? 3 : 0xf; // port A is 2-bit
	u8 inp = m_read_r[index](index);

	if (m_read_r[index].isunset())
	{
		inp = m_r_mask[index];
		logerror("read from unmapped port R%X at $%04X\n", index, m_prev_pc);
	}

	if (m_r_mask[index])
		return (inp & m_r[index]) & mask;
	else
		return (inp | m_r[index]) & mask;
}

void hmcs400_cpu_device::write_r(u8 index, u8 data)
{
	data &= 0xf;

	// ignore writes to read-only or non-existent ports
	if (index > 8)
		return;

	if (m_write_r[index].isunset())
		logerror("write $%X to unmapped port R%d at $%04X\n", data, index, m_prev_pc);

	m_r[index] = data;
	m_write_r[index](index, data);
}

int hmcs400_cpu_device::read_d(u8 index)
{
	index &= 0xf;
	u16 mask = 1 << index;
	u16 inp = m_read_d(0, mask);

	if (m_read_d.isunset())
	{
		inp = m_d_mask;
		logerror("read from unmapped port D%d at $%04X\n", index, m_prev_pc);
	}

	if (m_d_mask & mask)
		return BIT(inp & m_d, index);
	else
		return BIT(inp | m_d, index);
}

void hmcs400_cpu_device::write_d(u8 index, int state)
{
	index &= 0xf;
	u16 mask = 1 << index;

	if (m_write_d.isunset())
		logerror("write %d to unmapped port D%d at $%04X\n", state, index, m_prev_pc);

	m_d = (m_d & ~mask) | (state ? mask : 0);
	m_write_d(0, m_d, mask);
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void hmcs400_cpu_device::cycle()
{
	m_icount--;
}

u16 hmcs400_cpu_device::fetch()
{
	u16 data = m_program->read_word(m_pc);
	m_pc = (m_pc + 1) & 0x3fff;
	cycle();

	return data & 0x3ff;
}

void hmcs400_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		// fetch next opcode
		m_prev_pc = m_pc;
		debugger_instruction_hook(m_pc);
		m_op = fetch();
		m_i = m_op & 0xf;

		// 2-byte opcodes / RAM address
		if ((m_op >= 0x100 && m_op < 0x140) || (m_op >= 0x150 && m_op < 0x1b0))
			m_param = fetch();
		else
			m_param = (m_w << 8 | m_x << 4 | m_y) & 0x3ff;

		// handle opcode
		switch (m_op & 0x3f0)
		{
			case 0x1c0: case 0x1d0: case 0x1e0: case 0x1f0: op_cal(); break;

			case 0x020: case 0x120: op_inem(); break;
			case 0x030: case 0x130: op_ilem(); break;
			case 0x070: op_ynei(); break;
			case 0x0b0: op_tbr(); break;
			case 0x150: op_jmpl(); break;
			case 0x160: op_call(); break;
			case 0x170: op_brl(); break;
			case 0x1a0: op_lmi(); break;
			case 0x1b0: op_p(); break;

			case 0x200: op_lbi(); break;
			case 0x210: op_lyi(); break;
			case 0x220: op_lxi(); break;
			case 0x230: op_lai(); break;
			case 0x240: op_lbr(); break;
			case 0x250: op_lar(); break;
			case 0x260: op_redd(); break;
			case 0x270: op_lamr(); break;
			case 0x280: op_ai(); break;
			case 0x290: op_lmiiy(); break;
			case 0x2a0: op_tdd(); break;
			case 0x2b0: op_alei(); break;
			case 0x2c0: op_lrb(); break;
			case 0x2d0: op_lra(); break;
			case 0x2e0: op_sedd(); break;
			case 0x2f0: op_xmra(); break;

			default:
				if ((m_op & 0x300) == 0x300)
				{
					op_br(); break;
				}

				switch (m_op & 0x3fc)
				{
			case 0x084: case 0x184: op_sem(); break;
			case 0x088: case 0x188: op_rem(); break;
			case 0x08c: case 0x18c: op_tm(); break;

			case 0x000: op_xsp(); break;
			case 0x040: op_lbm(); break;
			case 0x080: op_xma(); break;
			case 0x090: op_lam(); break;
			case 0x094: op_lma(); break;
			case 0x0c0: op_xmb(); break;
			case 0x0f0: op_lwi(); break;

			default:
				switch (m_op)
				{
			case 0x004: case 0x104: op_anem(); break;
			case 0x008: case 0x108: op_am(); break;
			case 0x00c: case 0x10c: op_orm(); break;
			case 0x014: case 0x114: op_alem(); break;
			case 0x018: case 0x118: op_amc(); break;
			case 0x01c: case 0x11c: op_eorm(); break;
			case 0x098: case 0x198: op_smc(); break;
			case 0x09c: case 0x19c: op_anm(); break;

			case 0x010: op_rtn(); break;
			case 0x011: op_rtni(); break;
			case 0x044: op_bnem(); break;
			case 0x048: op_lab(); break;
			case 0x04c: op_ib(); break;
			case 0x050: case 0x051: op_lmaiy(); break;
			case 0x054: op_ayy(); break;
			case 0x058: op_laspy(); break;
			case 0x05c: op_iy(); break;
			case 0x060: op_nega(); break;
			case 0x064: op_red(); break;
			case 0x068: op_laspx(); break;
			case 0x06f: op_tc(); break;

			case 0x0a0: op_rotr(); break;
			case 0x0a1: op_rotl(); break;
			case 0x0a6: op_daa(); break;
			case 0x0aa: op_das(); break;
			case 0x0af: op_lay(); break;
			case 0x0c4: op_blem(); break;
			case 0x0c8: op_lba(); break;
			case 0x0cf: op_db(); break;
			case 0x0d0: case 0x0d1: op_lmady(); break;
			case 0x0d4: op_syy(); break;
			case 0x0d8: op_lya(); break;
			case 0x0df: op_dy(); break;
			case 0x0e0: op_td(); break;
			case 0x0e4: op_sed(); break;
			case 0x0e8: op_lxa(); break;
			case 0x0ec: op_rec(); break;
			case 0x0ef: op_sec(); break;

			case 0x100: op_law(); break;
			case 0x110: op_lwa(); break;
			case 0x140: op_comb(); break;
			case 0x144: op_or(); break;
			case 0x148: op_sts(); break;
			case 0x14c: op_sby(); break;
			case 0x14d: op_stop(); break;
			case 0x180: op_xma(); break;
			case 0x190: op_lam(); break;
			case 0x194: op_lma(); break;

			default: op_illegal(); break;
				}
				break; // 0x3ff

				}
				break; // 0x3fc

		} // 0x3f0
	}
}
