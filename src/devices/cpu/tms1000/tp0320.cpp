// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TP0320

  TODO:
  - lots

*/

#include "emu.h"
#include "tp0320.h"
#include "tms1k_dasm.h"

// TP0320 is TI's first CMOS MCU with integrated LCD controller, the die is still very similar to TMS0980
// - 2048x9bit ROM, same as on TMS0980 with different row-select
// - 192x4bit RAM array at the bottom-left (set up as 16x12x4)
// - 16x4bit LCD RAM, above main RAM array
// - main instructions PLAs at the same position as TMS0980, fixed opcodes:
//   * LDP, RETN, OFF, bb?, be?, b9?, ba?, RBIT, SBIT, COMX8, bc?, LDX, XDA, TDO, SEAC, REAC, SAL, SBL
// - 64-term microinstructions PLA between the RAM and ROM, similar to TMS0980,
//   plus separate lines for custom opcode handling like TMS0270, used for SETR and RSTR
// - 24-term output PLA above LCD RAM
DEFINE_DEVICE_TYPE(TP0320, tp0320_cpu_device, "tp0320", "Texas Instruments TP0320") // 28-pin SDIP, ..


// internal memory maps
void tp0320_cpu_device::data_192x4(address_map &map)
{
	map(0x00, 0x7f).ram();
	map(0x80, 0xbf).ram().mirror(0x40); // DAM
}


// device definitions
tp0320_cpu_device::tp0320_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms0980_cpu_device(mconfig, TP0320, tag, owner, clock, 7 /* o pins */, 10 /* r pins */, 7 /* pc bits */, 9 /* byte width */, 4 /* x width */, 11 /* prg width */, address_map_constructor(FUNC(tp0320_cpu_device::program_11bit_9), this), 8 /* data width */, address_map_constructor(FUNC(tp0320_cpu_device::data_192x4), this))
{
}


// machine configs
void tp0320_cpu_device::device_add_mconfig(machine_config &config)
{
	// main opcodes PLA(partial), microinstructions PLA
	PLA(config, "ipla", 9, 6, 8).set_format(pla_device::FMT::BERKELEY);
	PLA(config, "mpla", 6, 22, 64).set_format(pla_device::FMT::BERKELEY);
}


// disasm
std::unique_ptr<util::disasm_interface> tp0320_cpu_device::create_disassembler()
{
	return std::make_unique<tp0320_disassembler>();
}


// device_reset
u32 tp0320_cpu_device::decode_micro(u8 sel)
{
	u32 decode = 0;

	sel = bitswap<8>(sel,7,6,0,1,2,3,4,5); // lines are reversed
	u32 mask = m_mpla->read(sel);
	mask ^= 0x0bff0; // invert active-negative

	//                                                 _____  _______  ______  _____  _____  ______  _____  _____  ______  _____         _____
	const u32 md[22] = { M_AUTA, M_AUTY, M_SSS, M_STO, M_YTP, M_NDMTP, M_DMTP, M_MTP, M_CKP, M_15TN, M_CKN, M_MTN, M_NATN, M_ATN, M_CME, M_CIN, M_SSE, M_CKM, M_NE, M_C8, M_SETR, M_RSTR };

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
