// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS0980, TMS1980

*/

#include "tms0980.h"
#include "debugger.h"

// TMS0980
// - 144x4bit RAM array at the bottom-left (128+16, set up as 8x18x4)
// - 2048x9bit ROM array at the bottom-left
// - main instructions PLAs at the top half, to the right of the midline
//   * top section is assumed to be the CKI bus select
//   * middle section is for microinstruction redirection, this part may differ per die
//   * rest is fixed instructions select, from top-to-bottom: SEAC, LDX, COMX, COMX8,
//     TDO, SBIT, RETN, SETR, REAC, XDA, SAL, RBIT, ..., OFF, SBL, LDP, redir(------00- + R0^BL)
// - 64-term microinstructions PLA between the RAM and ROM, supporting 20 microinstructions
// - 16-term inverted output PLA and segment PLA above the RAM (rotate opla 90 degrees)
const device_type TMS0980 = &device_creator<tms0980_cpu_device>; // 28-pin DIP, 9 R pins

// TMS1980 is a TMS0980 with a TMS1x00 style opla
// - RAM, ROM, and main instructions PLAs is the same as TMS0980
// - one of the microinstructions redirects to a RSTR instruction, like on TMS0270
// - 32-term inverted output PLA above the RAM, 7 bits! (rotate opla 270 degrees)
const device_type TMS1980 = &device_creator<tms1980_cpu_device>; // 28-pin DIP, 7 O pins, 10 R pins, high voltage


// internal memory maps
static ADDRESS_MAP_START(program_11bit_9, AS_PROGRAM, 16, tms1k_base_device)
	AM_RANGE(0x000, 0xfff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(data_144x4, AS_DATA, 8, tms1k_base_device)
	AM_RANGE(0x00, 0x7f) AM_RAM
	AM_RANGE(0x80, 0x8f) AM_RAM AM_MIRROR(0x70) // DAM
ADDRESS_MAP_END


// device definitions
tms0980_cpu_device::tms0980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms0970_cpu_device(mconfig, TMS0980, "TMS0980", tag, owner, clock, 8 /* o pins */, 9 /* r pins */, 7 /* pc bits */, 9 /* byte width */, 4 /* x width */, 12 /* prg width */, ADDRESS_MAP_NAME(program_11bit_9), 8 /* data width */, ADDRESS_MAP_NAME(data_144x4), "tms0980", __FILE__)
{ }

tms0980_cpu_device::tms0980_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
	: tms0970_cpu_device(mconfig, type, name, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, prgwidth, program, datawidth, data, shortname, source)
{ }

tms1980_cpu_device::tms1980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: tms0980_cpu_device(mconfig, TMS1980, "TMS1980", tag, owner, clock, 7, 10, 7, 9, 4, 12, ADDRESS_MAP_NAME(program_11bit_9), 8, ADDRESS_MAP_NAME(data_144x4), "tms1980", __FILE__)
{ }


// machine configs
static MACHINE_CONFIG_FRAGMENT(tms0980)

	// main opcodes PLA, microinstructions PLA, output PLA, segment PLA
	MCFG_PLA_ADD("ipla", 9, 22, 24)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("mpla", 6, 20, 64)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("opla", 4, 8, 16)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("spla", 3, 8, 8)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
MACHINE_CONFIG_END

machine_config_constructor tms0980_cpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(tms0980);
}

static MACHINE_CONFIG_FRAGMENT(tms1980)

	// main opcodes PLA, microinstructions PLA, output PLA
	MCFG_PLA_ADD("ipla", 9, 22, 24)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("mpla", 6, 22, 64)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
	MCFG_PLA_ADD("opla", 5, 7, 32)
	MCFG_PLA_FILEFORMAT(PLA_FMT_BERKELEY)
MACHINE_CONFIG_END

machine_config_constructor tms1980_cpu_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(tms1980);
}


// disasm
offs_t tms0980_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(tms0980);
	return CPU_DISASSEMBLE_NAME(tms0980)(this, buffer, pc, oprom, opram, options);
}


// device_reset
UINT32 tms0980_cpu_device::decode_fixed(UINT16 op)
{
	UINT32 decode = 0;
	UINT32 mask = m_ipla->read(op);

	// 1 line per PLA row, no OR-mask
	const UINT32 id[15] = { F_LDP, F_SBL, F_OFF, F_RBIT, F_SAL, F_XDA, F_REAC, F_SETR, F_RETN, F_SBIT, F_TDO, F_COMX8, F_COMX, F_LDX, F_SEAC };

	for (int bit = 0; bit < 15; bit++)
		if (mask & (0x80 << bit))
			decode |= id[bit];

	return decode;
}

UINT32 tms0980_cpu_device::decode_micro(UINT8 sel)
{
	UINT32 decode = 0;
	sel = BITSWAP8(sel,7,6,0,1,2,3,4,5); // lines are reversed
	UINT32 mask = m_mpla->read(sel);
	mask ^= 0x43fc3; // invert active-negative

	// M_RSTR is specific to TMS02x0/TMS1980, it redirects to F_RSTR
	// M_UNK1 is specific to TMS0270, unknown/unused yet and apparently not connected on every TMS0270
	//                      _______  ______                                _____  _____  _____  _____  ______  _____  ______  _____                            _____
	const UINT32 md[22] = { M_NDMTP, M_DMTP, M_AUTY, M_AUTA, M_CKM, M_SSE, M_CKP, M_YTP, M_MTP, M_ATN, M_NATN, M_MTN, M_15TN, M_CKN, M_NE, M_C8, M_SSS, M_CME, M_CIN, M_STO, M_RSTR, M_UNK1 };

	for (int bit = 0; bit < 22 && bit < m_mpla->outputs(); bit++)
		if (mask & (1 << bit))
			decode |= md[bit];

	return decode;
}

void tms0980_cpu_device::device_reset()
{
	// common reset
	tms1k_base_device::device_reset();

	// pre-decode instructionset
	m_fixed_decode.resize(0x200);
	memset(&m_fixed_decode[0], 0, 0x200*sizeof(UINT32));
	m_micro_decode.resize(0x200);
	memset(&m_micro_decode[0], 0, 0x200*sizeof(UINT32));

	for (UINT16 op = 0; op < 0x200; op++)
	{
		// upper half of the opcodes is always branch/call
		if (op & 0x100)
			m_fixed_decode[op] = (op & 0x80) ? F_CALL: F_BR;

		// 6 output bits select a microinstruction index
		m_micro_decode[op] = decode_micro(m_ipla->read(op) & 0x3f);

		// the other ipla terms each select a fixed instruction
		m_fixed_decode[op] |= decode_fixed(op);
	}

	// like on TMS0970, one of the terms directly select a microinstruction index (via R4-R8),
	// but it can't be pre-determined when it's active
	m_micro_direct.resize(0x40);
	memset(&m_micro_decode[0], 0, 0x40*sizeof(UINT32));

	for (int op = 0; op < 0x40; op++)
		m_micro_direct[op] = decode_micro(op);
}


// program counter/opcode decode
UINT32 tms0980_cpu_device::read_micro()
{
	// if ipla term 0 is active, R4-R8 directly select a microinstruction index when R0 or R0^BL is 0
	int r0 = m_opcode >> 8 & 1;
	if (m_ipla->read(m_opcode) & 0x40 && !((r0 & m_bl) ^ r0))
		return m_micro_direct[m_opcode & 0x3f];
	else
		return m_micro_decode[m_opcode];
}

void tms0980_cpu_device::read_opcode()
{
	debugger_instruction_hook(this, m_rom_address << 1);
	m_opcode = m_program->read_word(m_rom_address << 1) & 0x1ff;
	m_c4 = BITSWAP8(m_opcode,7,6,5,4,0,1,2,3) & 0xf; // opcode operand is bitswapped for most opcodes

	m_fixed = m_fixed_decode[m_opcode];
	m_micro = read_micro();

	// redirect mpla fixed instructions
	if (m_micro & M_RSTR) m_fixed |= F_RSTR;
	if (m_micro & M_SETR) m_fixed |= F_SETR;

	next_pc();
}


// i/o handling
UINT8 tms0980_cpu_device::read_k_input()
{
	UINT8 k = m_read_k(0, 0xff) & 0x1f;
	UINT8 k3 = (k & 0x10) ? 3: 0; // the TMS0980 K3 line is simply K1|K2
	return (k & 0xf) | k3;
}

void tms0980_cpu_device::set_cki_bus()
{
	switch (m_opcode & 0x1f8)
	{
		// 000001XXX: K-inputs
		case 0x008:
			m_cki_bus = read_k_input();
			break;

		// 0X0100XXX: select bit
		case 0x020: case 0x0a0:
			m_cki_bus = 1 << (m_c4 >> 2) ^ 0xf;
			break;

		// 0X1XXXXXX: constant
		case 0x040: case 0x048: case 0x050: case 0x058: case 0x060: case 0x068: case 0x070: case 0x078:
		case 0x0c0: case 0x0c8: case 0x0d0: case 0x0d8: case 0x0e0: case 0x0e8: case 0x0f0: case 0x0f8:
			m_cki_bus = m_c4;
			break;

		default:
			m_cki_bus = 0;
			break;
	}
}


// opcode deviations
void tms0980_cpu_device::op_comx()
{
	// COMX: complement X register, but not the MSB
	m_x ^= (m_x_mask >> 1);
}

void tms1980_cpu_device::op_tdo()
{
	// TDO: transfer accumulator and status(not status_latch!) to O-output
	write_o_output(m_status << 4 | m_a);
}
