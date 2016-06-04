// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TP0320

  TODO:
  - lots

*/

#include "tp0320.h"
#include "debugger.h"

// TP0320 is TI's first CMOS MCU with integrated LCD controller, the die is still very similar to TMS0980
// - 2048x9bit ROM, same as on TMS0980 with different row-select
// - 192x4bit RAM array at the bottom-left (set up as 16x12x4)
// - 16x4bit LCD RAM, above main RAM array
// - main instructions PLAs at the same position as TMS0980, fixed opcodes:
//   * LDP, RETN, OFF, bb?, be?, b9?, ba?, RBIT, SBIT, COMX8, bc?, LDX, XDA, TDO, SEAC, REAC, SAL, SBL
// - 64-term microinstructions PLA between the RAM and ROM, similar to TMS0980,
//   plus separate lines for custom opcode handling like TMS0270, used for SETR and RSTR
// - 24-term output PLA above LCD RAM
const device_type TP0320 = &device_creator<tp0320_cpu_device>; // 28-pin SDIP, ..


// internal memory maps
static ADDRESS_MAP_START(program_11bit_9, AS_PROGRAM, 16, tms1k_base_device)
	AM_RANGE(0x000, 0xfff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_192x4, AS_DATA, 8, tms1k_base_device)
	AM_RANGE(0x00, 0x7f) AM_RAM
	AM_RANGE(0x80, 0xbf) AM_RAM AM_MIRROR(0x40) // DAM
ADDRESS_MAP_END


// device definitions
tp0320_cpu_device::tp0320_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms0980_cpu_device(mconfig, TP0320, "TP0320", tag, owner, clock, 7 /* o pins */, 10 /* r pins */, 7 /* pc bits */, 9 /* byte width */, 4 /* x width */, 12 /* prg width */, ADDRESS_MAP_NAME(program_11bit_9), 8 /* data width */, ADDRESS_MAP_NAME(data_192x4), "tp0320", __FILE__)
{ }


// machine configs
static MACHINE_CONFIG_FRAGMENT(tp0320)

	// main opcodes PLA(partial), microinstructions PLA
	MCFG_PLA_ADD("ipla", 9, 6, 8)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("mpla", 6, 22, 64)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
MACHINE_CONFIG_END

machine_config_constructor tp0320_cpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(tp0320);
}


// disasm
offs_t tp0320_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(tp0320);
	return CPU_DISASSEMBLE_NAME(tp0320)(this, buffer, pc, oprom, opram, options);
}


// device_reset
UINT32 tp0320_cpu_device::decode_micro(UINT8 sel)
{
	UINT32 decode = 0;

	sel = BITSWAP8(sel,7,6,0,1,2,3,4,5); // lines are reversed
	UINT32 mask = m_mpla->read(sel);
	mask ^= 0x0bff0; // invert active-negative

	//                                                    _____  _______  ______  _____  _____  ______  _____  _____  ______  _____         _____
	const UINT32 md[22] = { M_AUTA, M_AUTY, M_SSS, M_STO, M_YTP, M_NDMTP, M_DMTP, M_MTP, M_CKP, M_15TN, M_CKN, M_MTN, M_NATN, M_ATN, M_CME, M_CIN, M_SSE, M_CKM, M_NE, M_C8, M_SETR, M_RSTR };

	for (int bit = 0; bit < 22 && bit < m_mpla->outputs(); bit++)
		if (mask & (1 << bit))
			decode |= md[bit];

	return decode;
}

void tp0320_cpu_device::device_reset()
{
	// common reset
	tms0980_cpu_device::device_reset();

	// fixed instructionset isn't fully understood yet
	m_fixed_decode[0x19] = F_XDA;
	m_fixed_decode[0xb0] = F_TDO;
	m_fixed_decode[0xb1] = F_SAL;
	m_fixed_decode[0xb2] = F_COMX8;
	m_fixed_decode[0xb3] = F_SBL;
	m_fixed_decode[0xb4] = F_REAC;
	m_fixed_decode[0xb5] = F_SEAC;
	m_fixed_decode[0xb6] = F_OFF;
	m_fixed_decode[0xbf] = F_RETN;

	for (int i = 0x80; i < 0x90; i++) m_fixed_decode[i] = F_LDP;
	for (int i = 0x90; i < 0xa0; i++) m_fixed_decode[i] = F_LDX;
	for (int i = 0xa0; i < 0xa4; i++) m_fixed_decode[i] = F_SBIT;
	for (int i = 0xa4; i < 0xa8; i++) m_fixed_decode[i] = F_RBIT;
}
