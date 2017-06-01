// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1400, TMS1470, TMS1600, TMS1670

  TODO:
  - emulate TMS1600 L-pins

*/

#include "emu.h"
#include "tms1400.h"
#include "debugger.h"

// TMS1400 follows the TMS1100, it doubles the ROM size again (4 chapters of 16 pages), and adds a 3-level callstack
// - rotate the view and mirror the OR-mask to get the proper layout of the mpla, the default is identical to tms1100
// - the opla size is increased from 20 to 32 terms
DEFINE_DEVICE_TYPE(TMS1400, tms1400_cpu_device, "tms1400", "TMS1400") // 28-pin DIP, 11 R pins (TMS1400CR is same, but with TMS1100 pinout)
DEFINE_DEVICE_TYPE(TMS1470, tms1470_cpu_device, "tms1470", "TMS1470") // high voltage version, 1 R pin removed for Vdd

// TMS1600 adds more I/O to the TMS1400, input pins are doubled with added L1,2,4,8
// - rotate the view and mirror the OR-mask to get the proper layout of the mpla, the default is identical to tms1100
// - the opla size is increased from 20 to 32 terms
DEFINE_DEVICE_TYPE(TMS1600, tms1600_cpu_device, "tms1600", "TMS1600") // 40-pin DIP, 16 R pins
DEFINE_DEVICE_TYPE(TMS1670, tms1670_cpu_device, "tms1670", "TMS1670") // high voltage version


// internal memory maps
static ADDRESS_MAP_START(program_12bit_8, AS_PROGRAM, 8, tms1k_base_device)
	AM_RANGE(0x000, 0xfff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_128x4, AS_DATA, 8, tms1k_base_device)
	AM_RANGE(0x00, 0x7f) AM_RAM
ADDRESS_MAP_END


// device definitions
tms1400_cpu_device::tms1400_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms1400_cpu_device(mconfig, TMS1400, tag, owner, clock, 8 /* o pins */, 11 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 3 /* x width */, 12 /* prg width */, ADDRESS_MAP_NAME(program_12bit_8), 7 /* data width */, ADDRESS_MAP_NAME(data_128x4))
{
}

tms1400_cpu_device::tms1400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data)
	: tms1100_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data)
{
}

tms1470_cpu_device::tms1470_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms1400_cpu_device(mconfig, TMS1470, tag, owner, clock, 8, 10, 6, 8, 3, 12, ADDRESS_MAP_NAME(program_12bit_8), 7, ADDRESS_MAP_NAME(data_128x4))
{
}


tms1600_cpu_device::tms1600_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms1600_cpu_device(mconfig, TMS1600, tag, owner, clock, 8, 16, 6, 8, 3, 12, ADDRESS_MAP_NAME(program_12bit_8), 7, ADDRESS_MAP_NAME(data_128x4))
{
}

tms1600_cpu_device::tms1600_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data)
	: tms1400_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data)
{
}

tms1670_cpu_device::tms1670_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms1600_cpu_device(mconfig, TMS1670, tag, owner, clock, 8, 16, 6, 8, 3, 12, ADDRESS_MAP_NAME(program_12bit_8), 7, ADDRESS_MAP_NAME(data_128x4))
{
}


// machine configs
MACHINE_CONFIG_MEMBER(tms1400_cpu_device::device_add_mconfig)

	// microinstructions PLA, output PLA
	MCFG_PLA_ADD("mpla", 8, 16, 30)
	MCFG_PLA_FILEFORMAT(BERKELEY)
	MCFG_PLA_ADD("opla", 5, 8, 32)
	MCFG_PLA_FILEFORMAT(BERKELEY)
MACHINE_CONFIG_END


// device_reset
void tms1400_cpu_device::device_reset()
{
	tms1100_cpu_device::device_reset();

	// small differences in 00-3f area
	m_fixed_decode[0x0b] = F_TPC;
}


// opcode deviations
void tms1400_cpu_device::op_br()
{
	// BR/BL: conditional branch
	if (m_status)
	{
		m_pa = m_pb; // don't care about clatch
		m_ca = m_cb;
		m_pc = m_opcode & m_pc_mask;
	}
}

void tms1400_cpu_device::op_call()
{
	// CALL/CALLL: conditional call
	if (m_status)
	{
		// 3-level stack, mask clatch 3 bits (no need to mask others)
		m_clatch = (m_clatch << 1 | 1) & 7;

		m_sr = m_sr << m_pc_bits | m_pc;
		m_pc = m_opcode & m_pc_mask;

		m_ps = m_ps << 4 | m_pa;
		m_pa = m_pb;

		m_cs = m_cs << 2 | m_ca;
		m_ca = m_cb;
	}
	else
	{
		m_pb = m_pa;
		m_cb = m_ca;
	}
}

void tms1400_cpu_device::op_retn()
{
	// RETN: return from subroutine
	if (m_clatch & 1)
	{
		m_clatch >>= 1;

		m_pc = m_sr & m_pc_mask;
		m_sr >>= m_pc_bits;

		m_pa = m_pb = m_ps & 0xf;
		m_ps >>= 4;

		m_ca = m_cb = m_cs & 3;
		m_cs >>= 2;
	}
}
