// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS0980, TMS1980

*/

#include "emu.h"
#include "tms0980.h"
#include "tms1k_dasm.h"

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
DEFINE_DEVICE_TYPE(TMS0980, tms0980_cpu_device, "tms0980", "Texas Instruments TMS0980") // 28-pin DIP, 9 R pins

// TMS1980 is a TMS0980 with a TMS1x00 style opla
// - RAM, ROM, and main instructions PLAs is the same as TMS0980
// - one of the microinstructions redirects to a RSTR instruction, like on TMS0270
// - 32-term output PLA above the RAM, 7 bits! (rotate opla 270 degrees)
DEFINE_DEVICE_TYPE(TMS1980, tms1980_cpu_device, "tms1980", "Texas Instruments TMS1980") // 28-pin DIP, 7 O pins, 10 R pins, high voltage


// internal memory maps
void tms0980_cpu_device::ram_144x4(address_map &map)
{
	map(0x00, 0x7f).ram();
	map(0x80, 0x8f).ram().mirror(0x70); // DAM
}


// device definitions
tms0980_cpu_device::tms0980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms0980_cpu_device(mconfig, TMS0980, tag, owner, clock, 8 /* o pins */, 9 /* r pins */, 7 /* pc bits */, 9 /* byte width */, 4 /* x width */, 1 /* stack levels */, 11 /* rom width */, address_map_constructor(FUNC(tms0980_cpu_device::rom_11bit), this), 8 /* ram width */, address_map_constructor(FUNC(tms0980_cpu_device::ram_144x4), this))
{ }

tms0980_cpu_device::tms0980_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map) :
	tms0970_cpu_device(mconfig, type, tag, owner, clock, o_pins, r_pins, pc_bits, byte_bits, x_bits, stack_levels, rom_width, rom_map, ram_width, ram_map)
{ }

tms1980_cpu_device::tms1980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tms0980_cpu_device(mconfig, TMS1980, tag, owner, clock, 7, 10, 7, 9, 4, 1, 11, address_map_constructor(FUNC(tms1980_cpu_device::rom_11bit), this), 8, address_map_constructor(FUNC(tms1980_cpu_device::ram_144x4), this))
{ }


// machine configs
void tms0980_cpu_device::device_add_mconfig(machine_config &config)
{
	// main opcodes PLA, microinstructions PLA, output PLA, segment PLA
	PLA(config, "ipla", 9, 22, 24).set_format(pla_device::FMT::BERKELEY);
	PLA(config, "mpla", 6, 20, 64).set_format(pla_device::FMT::BERKELEY);
	PLA(config, "opla", 4, 8, 16).set_format(pla_device::FMT::BERKELEY);
	PLA(config, "spla", 3, 8, 8).set_format(pla_device::FMT::BERKELEY);
}

void tms1980_cpu_device::device_add_mconfig(machine_config &config)
{
	// main opcodes PLA, microinstructions PLA, output PLA
	PLA(config, "ipla", 9, 22, 24).set_format(pla_device::FMT::BERKELEY);
	PLA(config, "mpla", 6, 22, 64).set_format(pla_device::FMT::BERKELEY);
	PLA(config, "opla", 5, 7, 32).set_format(pla_device::FMT::BERKELEY);
}


// disasm
std::unique_ptr<util::disasm_interface> tms0980_cpu_device::create_disassembler()
{
	return std::make_unique<tms0980_disassembler>();
}


// device_reset
u32 tms0980_cpu_device::decode_fixed(u16 op)
{
	u32 decode = 0;
	u32 mask = m_ipla->read(op);

	// 1 line per PLA row, no OR-mask
	const u32 id[15] = { F_LDP, F_SBL, F_OFF, F_RBIT, F_SAL, F_XDA, F_REAC, F_SETR, F_RETN, F_SBIT, F_TDO, F_COMX8, F_COMX, F_LDX, F_SEAC };

	for (int bit = 0; bit < 15; bit++)
		if (mask & (0x80 << bit))
			decode |= id[bit];

	return decode;
}

u32 tms0980_cpu_device::decode_micro(u8 sel)
{
	u32 decode = 0;
	sel = bitswap<8>(sel,7,6,0,1,2,3,4,5); // lines are reversed
	u32 mask = m_mpla->read(sel);
	mask ^= 0x43fc3; // invert active-negative

	// M_RSTR is specific to TMS02x0/TMS1980, it redirects to F_RSTR
	// M_UNK1 is specific to TMS0270, unknown/unused yet and apparently not connected on every TMS0270
	//                   _______  ______                                _____  _____  _____  _____  ______  _____  ______  _____                            _____
	const u32 md[22] = { M_NDMTP, M_DMTP, M_AUTY, M_AUTA, M_CKM, M_SSE, M_CKP, M_YTP, M_MTP, M_ATN, M_NATN, M_MTN, M_15TN, M_CKN, M_NE, M_C8, M_SSS, M_CME, M_CIN, M_STO, M_RSTR, M_UNK1 };

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
	memset(&m_fixed_decode[0], 0, 0x200*sizeof(u32));
	m_micro_decode.resize(0x200);
	memset(&m_micro_decode[0], 0, 0x200*sizeof(u32));

	for (u16 op = 0; op < 0x200; op++)
	{
		// upper half of the opcodes is always branch/call
		if (op & 0x100)
			m_fixed_decode[op] = (op & 0x80) ? F_CALL: F_BR;

		// 6 output bits select a microinstruction index
		m_micro_decode[op] = m_decode_micro.isnull() ? decode_micro(m_ipla->read(op) & 0x3f) : m_decode_micro(op);

		// the other ipla terms each select a fixed instruction
		m_fixed_decode[op] |= decode_fixed(op);
	}

	// like on TMS0970, one of the terms directly select a microinstruction index (via R4-R8),
	// but it can't be pre-determined when it's active
	m_micro_direct.resize(0x40);
	memset(&m_micro_decode[0], 0, 0x40*sizeof(u32));

	for (int op = 0; op < 0x40; op++)
		m_micro_direct[op] = m_decode_micro.isnull() ? decode_micro(op) : m_decode_micro(op + 0x200);
}


// program counter/opcode decode
u32 tms0980_cpu_device::read_micro()
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
	debugger_instruction_hook(m_rom_address);
	m_opcode = m_program->read_word(m_rom_address) & 0x1ff;
	m_c4 = bitswap<8>(m_opcode,7,6,5,4,0,1,2,3) & 0xf; // opcode operand is bitswapped for most opcodes

	m_fixed = m_fixed_decode[m_opcode];
	m_micro = read_micro();

	// redirect mpla fixed instructions
	if (m_micro & M_RSTR) m_fixed |= F_RSTR;
	if (m_micro & M_SETR) m_fixed |= F_SETR;

	next_pc();
}


// i/o handling
u8 tms0980_cpu_device::read_k_input()
{
	u8 k = m_read_k() & 0x1f;
	u8 k3 = (k & 0x10) ? 3: 0; // the K3 line is simply K1|K2
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
