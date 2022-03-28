// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell A5000/A5900 MCU

TODO:
- what happens when 0 or >1 keys are held down on READ?

*/

#include "emu.h"
#include "a5000.h"

#include "rw5000d.h"


DEFINE_DEVICE_TYPE(A5000, a5000_cpu_device, "a5000", "Rockwell A5000")
DEFINE_DEVICE_TYPE(A5900, a5900_cpu_device, "a5900", "Rockwell A5900")


// constructor
a5000_cpu_device::a5000_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	b5000_cpu_device(mconfig, type, tag, owner, clock, prgwidth, program, datawidth, data)
{ }

a5000_cpu_device::a5000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	a5000_cpu_device(mconfig, A5000, tag, owner, clock, 9, address_map_constructor(FUNC(a5000_cpu_device::program_448x8), this), 6, address_map_constructor(FUNC(a5000_cpu_device::data_45x4), this))
{ }

a5900_cpu_device::a5900_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	a5000_cpu_device(mconfig, A5900, tag, owner, clock, 9, address_map_constructor(FUNC(a5900_cpu_device::program_512x8), this), 6, address_map_constructor(FUNC(a5900_cpu_device::data_45x4), this))
{ }


// internal memory maps
void a5900_cpu_device::program_512x8(address_map &map)
{
	map(0x000, 0x1ff).rom();
}


// disasm
std::unique_ptr<util::disasm_interface> a5000_cpu_device::create_disassembler()
{
	return std::make_unique<a5000_disassembler>();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

void a5000_cpu_device::execute_one()
{
	switch (m_op)
	{
		case 0x02: op_illegal(); break;
		case 0x03: op_tkb(); break;
		case 0x76: m_mtd_step = 1; break;
		case 0x77: op_atb(); break;

		// rest is same as B5000
		default: b5000_cpu_device::execute_one(); break;
	}
}


//-------------------------------------------------
//  changed opcodes (no need for separate file)
//-------------------------------------------------

void a5000_cpu_device::op_mtd_step()
{
	assert(m_mtd_step > 0);

	// MTD: load strobe + segments (multi step)
	switch (m_mtd_step)
	{
		// step 1: disable strobe and segment drivers
		case 1:
			m_write_str(0);
			op_kseg();
			break;

		// step 2: load strobe from Bl
		case 2:
			m_write_str(1 << m_prev_bl);
			break;

		// step 4: load segment drivers
		case 4:
			seg_w(m_seg | decode_digit(m_prev3_c << 4 | ram_r()));
			m_mtd_step = 0;
			return;

		default:
			break;
	}
	m_mtd_step++;
}

void a5000_cpu_device::op_read()
{
	// READ: add _KB info to A, skip next on no overflow
	m_a += ~((count_leading_zeros_32(m_read_kb() & 0xf) - 28) & 3) & 0xf;
	m_skip = !BIT(m_a, 4);
	m_a &= 0xf;
}
