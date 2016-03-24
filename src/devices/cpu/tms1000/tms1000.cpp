// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS1000, TMS1070, TMS1040, TMS1200

*/

#include "tms1000.h"
#include "debugger.h"

// TMS1000
// - 64x4bit RAM array at the bottom-left
// - 1024x8bit ROM array at the bottom-right
//   * FYI, the row-selector to the left of it is laid out as:
//     3,4,11,12,19,20,27,28,35,36,43,44,51,52,59,60,0,7,8,15,16,23,24,31,32,39,40,47,48,55,56,63,
//     2,5,10,13,18,21,26,29,34,37,42,45,50,53,58,61,1,6,9,14,17,22,25,30,33,38,41,46,49,54,57,62
// - 30-term microinstructions PLA(mpla) at the top half, to the right of the midline, supporting 16 microinstructions
// - 20-term output PLA(opla) at the top-left
// - the ALU is between the opla and mpla
const device_type TMS1000 = &device_creator<tms1000_cpu_device>; // 28-pin DIP, 11 R pins
const device_type TMS1070 = &device_creator<tms1070_cpu_device>; // high voltage version
const device_type TMS1040 = &device_creator<tms1040_cpu_device>; // same as TMS1070 with just a different pinout?
const device_type TMS1200 = &device_creator<tms1200_cpu_device>; // 40-pin DIP, 13 R pins
// TMS1270 has 10 O pins, how does that work?


// internal memory maps
static ADDRESS_MAP_START(program_10bit_8, AS_PROGRAM, 8, tms1k_base_device)
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_64x4, AS_DATA, 8, tms1k_base_device)
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END


// device definitions
tms1000_cpu_device::tms1000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1k_base_device(mconfig, TMS1000, "TMS1000", tag, owner, clock, 8 /* o pins */, 11 /* r pins */, 6 /* pc bits */, 8 /* byte width */, 2 /* x width */, 10 /* prg width */, ADDRESS_MAP_NAME(program_10bit_8), 6 /* data width */, ADDRESS_MAP_NAME(data_64x4), "tms1000", __FILE__)
{ }

tms1000_cpu_device::tms1000_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
	: tms1k_base_device(mconfig, type, name, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data, shortname, source)
{ }

tms1070_cpu_device::tms1070_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1000_cpu_device(mconfig, TMS1070, "TMS1070", tag, owner, clock, 8, 11, 6, 8, 2, 10, ADDRESS_MAP_NAME(program_10bit_8), 6, ADDRESS_MAP_NAME(data_64x4), "tms1070", __FILE__)
{ }

tms1040_cpu_device::tms1040_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1000_cpu_device(mconfig, TMS1040, "TMS1040", tag, owner, clock, 8, 11, 6, 8, 2, 10, ADDRESS_MAP_NAME(program_10bit_8), 6, ADDRESS_MAP_NAME(data_64x4), "tms1040", __FILE__)
{ }

tms1200_cpu_device::tms1200_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms1000_cpu_device(mconfig, TMS1200, "TMS1200", tag, owner, clock, 8, 13, 6, 8, 2, 10, ADDRESS_MAP_NAME(program_10bit_8), 6, ADDRESS_MAP_NAME(data_64x4), "tms1200", __FILE__)
{ }


// machine configs
static MACHINE_CONFIG_FRAGMENT(tms1000)

	// microinstructions PLA, output PLA
	MCFG_PLA_ADD("mpla", 8, 16, 30)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("opla", 5, 8, 20)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
MACHINE_CONFIG_END

machine_config_constructor tms1000_cpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(tms1000);
}


// disasm
offs_t tms1000_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(tms1000);
	return CPU_DISASSEMBLE_NAME(tms1000)(this, buffer, pc, oprom, opram, options);
}


// device_reset
void tms1000_cpu_device::device_reset()
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
		//                                              _____              _____  ______  _____  ______  _____  _____  _____  _____
		const UINT32 md[16] = { M_STSL, M_AUTY, M_AUTA, M_CIN, M_C8, M_NE, M_CKN, M_15TN, M_MTN, M_NATN, M_ATN, M_MTP, M_YTP, M_CKP, M_CKM, M_STO };
		UINT16 mask = m_mpla->read(op);
		mask ^= 0x3fc8; // invert active-negative

		for (int bit = 0; bit < 16; bit++)
			if (mask & (1 << bit))
				m_micro_decode[op] |= md[bit];
	}

	// the fixed instruction set is not programmable
	m_fixed_decode[0x00] = F_COMX;
	m_fixed_decode[0x0a] = F_TDO;
	m_fixed_decode[0x0b] = F_CLO;
	m_fixed_decode[0x0c] = F_RSTR;
	m_fixed_decode[0x0d] = F_SETR;
	m_fixed_decode[0x0f] = F_RETN;

	for (int i = 0x10; i < 0x20; i++) m_fixed_decode[i] = F_LDP;
	for (int i = 0x30; i < 0x34; i++) m_fixed_decode[i] = F_SBIT;
	for (int i = 0x34; i < 0x38; i++) m_fixed_decode[i] = F_RBIT;
	for (int i = 0x3c; i < 0x40; i++) m_fixed_decode[i] = F_LDX;

	for (int i = 0x80; i < 0xc0; i++) m_fixed_decode[i] = F_BR;
	for (int i = 0xc0; i < 0x100; i++) m_fixed_decode[i] = F_CALL;
}
