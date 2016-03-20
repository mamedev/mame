// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS0950, TMS0970, TMS1990

*/

#include "tms0970.h"
#include "debugger.h"

// TMS0950 is a TMS1000 with a TMS0980 style opla, it was quickly succeeded by the TMS0970
// - RAM, ROM, microinstructions is the same as TMS1000
// - 10-term inverted output PLA and segment PLA on the top-left
const device_type TMS0950 = &device_creator<tms0950_cpu_device>; // 28-pin DIP, 8 O pins, 11? R pins

// TMS0970 is a stripped-down version of the TMS0980, itself acting more like a TMS1000
// - RAM and ROM is the same as TMS1000
// - main instructions PLAs at the top half, to the right of the midline
//   * see TMS0980 notes, except that the fixed instruction list differs:
//     RETN, SETR, RBIT, SBIT, LDX, COMX, TDO, ..., redir(----0-00), LDP
// - 32-term microinstructions PLA between the RAM and ROM, supporting 15 microinstructions
// - 16-term inverted output PLA and segment PLA above the RAM (rotate opla 90 degrees)
const device_type TMS0970 = &device_creator<tms0970_cpu_device>; // 28-pin DIP, 11 R pins (note: pinout may slightly differ from chip to chip)
const device_type TMS1990 = &device_creator<tms1990_cpu_device>; // 28-pin DIP, ? R pins..


// internal memory maps
static ADDRESS_MAP_START(program_10bit_8, AS_PROGRAM, 8, tms1k_base_device)
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_64x4, AS_DATA, 8, tms1k_base_device)
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END


// device definitions
tms0970_cpu_device::tms0970_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1000_cpu_device(mconfig, TMS0970, "TMS0970", tag, owner, clock, 8 /* o pins */, 11 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 2 /* x width */, 10 /* prg width */, ADDRESS_MAP_NAME(program_10bit_8), 6 /* data width */, ADDRESS_MAP_NAME(data_64x4), "tms0970", __FILE__)
{ }

tms0970_cpu_device::tms0970_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
	: tms1000_cpu_device(mconfig, type, name, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data, shortname, source)
{ }

tms0950_cpu_device::tms0950_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms0970_cpu_device(mconfig, TMS0950, "TMS0950", tag, owner, clock, 8, 11, 6, 8, 2, 10, ADDRESS_MAP_NAME(program_10bit_8), 6, ADDRESS_MAP_NAME(data_64x4), "tms0950", __FILE__)
{ }

tms1990_cpu_device::tms1990_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms0970_cpu_device(mconfig, TMS1990, "TMS1990", tag, owner, clock, 8, 11, 6, 8, 2, 10, ADDRESS_MAP_NAME(program_10bit_8), 6, ADDRESS_MAP_NAME(data_64x4), "tms1990", __FILE__)
{ }


// machine configs
static MACHINE_CONFIG_FRAGMENT(tms0950)

	// microinstructions PLA, output PLA, segment PLA
	MCFG_PLA_ADD("mpla", 8, 16, 30)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("opla", 4, 8, 10)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("spla", 3, 8, 8)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
MACHINE_CONFIG_END

machine_config_constructor tms0950_cpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(tms0950);
}

static MACHINE_CONFIG_FRAGMENT(tms0970)

	// main opcodes PLA, microinstructions PLA, output PLA, segment PLA
	MCFG_PLA_ADD("ipla", 8, 15, 18)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("mpla", 5, 15, 32)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("opla", 4, 8, 16)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("spla", 3, 8, 8)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
MACHINE_CONFIG_END

machine_config_constructor tms0970_cpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(tms0970);
}


// device_reset
void tms0970_cpu_device::device_reset()
{
	// common reset
	tms1k_base_device::device_reset();

	// pre-decode instructionset
	m_fixed_decode.resize(0x100);
	memset(&m_fixed_decode[0], 0, 0x100*sizeof(UINT32));
	m_micro_decode.resize(0x100);
	memset(&m_micro_decode[0], 0, 0x100*sizeof(UINT32));

	for (int op = 0; op < 0x100; op++)
	{
		// upper half of the opcodes is always branch/call
		if (op & 0x80)
			m_fixed_decode[op] = (op & 0x40) ? F_CALL: F_BR;

		// 5 output bits select a microinstruction index
		UINT32 imask = m_ipla->read(op);
		UINT8 msel = imask & 0x1f;

		// but if (from bottom to top) term 1 is active and output bit 5 is 0, R2,R4-R7 directly select a microinstruction index
		if (imask & 0x40 && (imask & 0x20) == 0)
			msel = (op & 0xf) | (op >> 1 & 0x10);

		msel = BITSWAP8(msel,7,6,5,0,1,2,3,4); // lines are reversed
		UINT32 mmask = m_mpla->read(msel);
		mmask ^= 0x09fe; // invert active-negative

		//                             _____  _____  _____  _____  ______  _____  ______  _____              _____
		const UINT32 md[15] = { M_CKM, M_CKP, M_YTP, M_MTP, M_ATN, M_NATN, M_MTN, M_15TN, M_CKN, M_NE, M_C8, M_CIN, M_AUTA, M_AUTY, M_STO };

		for (int bit = 0; bit < 15; bit++)
			if (mmask & (1 << bit))
				m_micro_decode[op] |= md[bit];

		// the other ipla terms each select a fixed instruction
		const UINT32 id[8] = { F_LDP, F_TDO, F_COMX, F_LDX, F_SBIT, F_RBIT, F_SETR, F_RETN };

		for (int bit = 0; bit < 8; bit++)
			if (imask & (0x80 << bit))
				m_fixed_decode[op] |= id[bit];
	}
}


// i/o handling
void tms0970_cpu_device::write_o_output(UINT8 index)
{
	m_o = m_spla->read(index);
	m_write_o(0, m_o & m_o_mask, 0xffff);
}


// opcode deviations
void tms0970_cpu_device::op_setr()
{
	// SETR: set output register
	// DDIG line is a coincidence between the selected output pla row(s) and segment pla row(s)
	int ddig = (m_opla->read(m_a) & m_o) ? 1 : 0;
	m_r = (m_r & ~(1 << m_y)) | (ddig << m_y);
}

void tms0970_cpu_device::op_tdo()
{
	// TDO: transfer digits to output
	write_o_output(m_a & 0x7);
	m_write_r(0, m_r & m_r_mask, 0xffff);
}
