// license:BSD-3-Clause
// copyright-holders:hap
/*

  Matsushita MN1400, MN1405

*/

#include "emu.h"
#include "mn1400.h"

#include "mn1400d.h"


// device definitions
DEFINE_DEVICE_TYPE(MN1400_40PINS, mn1400_cpu_device, "mn1400", "Matsushita MN1400 (40 pins)")
DEFINE_DEVICE_TYPE(MN1400_28PINS, mn1400_reduced_cpu_device, "mn1400_reduced", "Matsushita MN1400 (28 pins)")
DEFINE_DEVICE_TYPE(MN1405, mn1405_cpu_device, "mn1405", "Matsushita MN1405")


// constructor
mn1400_cpu_device::mn1400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data) :
	mn1400_base_device(mconfig, type, tag, owner, clock, stack_levels, prgwidth, program, datawidth, data)
{ }

mn1400_cpu_device::mn1400_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mn1400_cpu_device(mconfig, MN1400_40PINS, tag, owner, clock, 2 /* stack levels */, 10 /* rom bits */, address_map_constructor(FUNC(mn1400_cpu_device::program_1kx8), this), 6 /* ram bits */, address_map_constructor(FUNC(mn1400_cpu_device::data_64x4), this))
{ }

mn1400_reduced_cpu_device::mn1400_reduced_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mn1400_cpu_device(mconfig, MN1400_28PINS, tag, owner, clock, 2, 10, address_map_constructor(FUNC(mn1400_reduced_cpu_device::program_1kx8), this), 6, address_map_constructor(FUNC(mn1400_reduced_cpu_device::data_64x4), this))
{
	// CO5-CO9, DO0-DO3 (scrambled)
	set_c_mask(0x3e0);
	set_d_mask(0xf, 0x5321);
}

mn1405_cpu_device::mn1405_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mn1400_cpu_device(mconfig, MN1405, tag, owner, clock, 2, 11, address_map_constructor(FUNC(mn1405_cpu_device::program_2kx8), this), 7, address_map_constructor(FUNC(mn1405_cpu_device::data_128x4), this))
{ }


// disasm
std::unique_ptr<util::disasm_interface> mn1400_cpu_device::create_disassembler()
{
	return std::make_unique<mn1400_disassembler>();
}


//-------------------------------------------------
//  execute
//-------------------------------------------------

bool mn1400_cpu_device::op_has_param(u8 op)
{
	// branch opcodes are 2 bytes
	return ((op & 0xf8) == 0x38 || (op & 0xf0) == 0x40 || (op & 0xf0) == 0xe0);
}

void mn1400_cpu_device::execute_one()
{
	switch (m_op & 0xf0)
	{
		case 0x50: op_li(); break;
		case 0x60: op_ly(); break;
		case 0x70: op_andi(); break;
		case 0x80: op_ai(); break;
		case 0x90: op_ci(); break;
		case 0xa0: op_cy(); break;
		case 0xb0: op_sm(); break;
		case 0xc0: op_rm(); break;
		case 0xd0: op_tb(); break;
		case 0xe0: op_bpcz(); break;
		case 0xf0: op_otie(); break;

		default:
			switch (m_op & 0xf8)
			{
		case 0x30: op_lx(); break;
		case 0x38: op_bs01(); break;
		case 0x40: op_jmp(); break;
		case 0x48: op_cal(); break;

		default:
			switch (m_op)
			{
		case 0x00: op_nop(); break;
		case 0x01: op_tax(); break;
		case 0x02: op_tya(); break;
		case 0x03: op_tay(); break;
		case 0x04: op_and(); break;
		case 0x05: op_or(); break;
		case 0x06: op_xor(); break;
		case 0x07: op_a(); break;
		case 0x08: op_cpl(); break;
		case 0x09: op_c(); break;
		case 0x0a: op_st(); break;
		case 0x0b: op_stic(); break;
		case 0x0c: op_stdc(); break;
		case 0x0d: op_l(); break;
		case 0x0e: op_lic(); break;
		case 0x0f: op_ldc(); break;

		case 0x10: op_ote(); break;
		case 0x11: op_otmd(); break;
		case 0x12: op_otd(); break;
		case 0x13: op_cco(); break;
		case 0x14: op_ina(); break;
		case 0x15: op_inb(); break;
		case 0x16: op_rco(); break;
		case 0x17: op_sco(); break;
		case 0x18: op_tacl(); break;
		case 0x19: op_tacu(); break;
		case 0x1a: op_tcal(); break;
		case 0x1b: op_tcau(); break;
		case 0x1c: op_dc(); break;
		case 0x1d: op_ec(); break;
		case 0x1e: op_sl(); break;
		case 0x1f: op_ret(); break;

		case 0x20: case 0x21: case 0x22: case 0x23: op_ld(); break;
		case 0x24: case 0x25: case 0x26: case 0x27: op_std(); break;

		case 0x28: op_rc(); break;
		case 0x29: op_rp(); break;
		case 0x2a: op_sc(); break;
		case 0x2b: op_sp(); break;
		case 0x2c: op_icy(); break;
		case 0x2d: op_dcy(); break;
		case 0x2e: op_icm(); break;
		case 0x2f: op_dcm(); break;

		default: op_illegal(); break;
			}
			break; // 0xff

			}
			break; // 0xfc

	} // 0xf0
}
