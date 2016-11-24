// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// *******************************
// * HP nanoprocessor disassembler
// *******************************

#include "emu.h"
#include "debugger.h"
#include "nanoprocessor.h"

typedef void (*fn_dis_param)(std::ostream& stream , uint8_t opcode , const uint8_t* opram);

typedef struct {
	uint8_t m_op_mask;
	uint8_t m_opcode;
	const char *m_mnemonic;
	fn_dis_param m_param_fn;
	uint32_t m_dasm_flags;
} dis_entry_t;

static void param_bitno(std::ostream& stream , uint8_t opcode , const uint8_t* opram)
{
	stream << (char)('0' + (opcode & 7));
}

static void param_ds(std::ostream& stream , uint8_t opcode , const uint8_t* opram)
{
	util::stream_format(stream , "DS%u" , opcode & 0xf);
}

static void param_reg(std::ostream& stream , uint8_t opcode , const uint8_t* opram)
{
	util::stream_format(stream , "R%u" , opcode & 0xf);
}

static void param_11bit(std::ostream& stream , uint8_t opcode , const uint8_t* opram)
{
	unsigned tmp = ((unsigned)(opcode & 7) << 8) | *opram;
	util::stream_format(stream , "$%03x" , tmp);
}

static void param_page_no(std::ostream& stream , uint8_t opcode , const uint8_t* opram)
{
	stream << (char)('0' + (opcode & 7));
}

static void param_byte(std::ostream& stream , uint8_t opcode , const uint8_t* opram)
{
	util::stream_format(stream , "$%02x" , *opram);
}

static void param_ds_byte(std::ostream& stream , uint8_t opcode , const uint8_t* opram)
{
	util::stream_format(stream , "DS%u,$%02x" , opcode & 0xf , *opram);
}

static void param_reg_byte(std::ostream& stream , uint8_t opcode , const uint8_t* opram)
{
	util::stream_format(stream , "R%u,$%02x" , opcode & 0xf , *opram);
}

static const dis_entry_t dis_table[] = {
	{0xff , 0x00 , "INB" , nullptr , 1},
	{0xff , 0x01 , "DEB" , nullptr , 1},
	{0xff , 0x02 , "IND" , nullptr , 1},
	{0xff , 0x03 , "DED" , nullptr , 1},
	{0xff , 0x04 , "CLA" , nullptr , 1},
	{0xff , 0x05 , "CMA" , nullptr , 1},
	{0xff , 0x06 , "RSA" , nullptr , 1},
	{0xff , 0x07 , "LSA" , nullptr , 1},
	{0xff , 0x08 , "SGT" , nullptr , 1},
	{0xff , 0x09 , "SLT" , nullptr , 1},
	{0xff , 0x0a , "SEQ" , nullptr , 1},
	{0xff , 0x0b , "SAZ" , nullptr , 1},
	{0xff , 0x0c , "SLE" , nullptr , 1},
	{0xff , 0x0d , "SGE" , nullptr , 1},
	{0xff , 0x0e , "SNE" , nullptr , 1},
	{0xff , 0x0f , "SAN" , nullptr , 1},
	{0xf8 , 0x10 , "SBS" , param_bitno , 1},
	{0xff , 0x1f , "SES" , nullptr , 1},
	{0xf8 , 0x18 , "SFS" , param_bitno , 1},
	{0xf8 , 0x20 , "SBN" , param_bitno , 1},
	{0xff , 0x2f , "ENI" , nullptr , 1},
	{0xf8 , 0x28 , "STC" , param_bitno , 1},
	{0xf8 , 0x30 , "SBZ" , param_bitno , 1},
	{0xff , 0x3f , "SEZ" , nullptr , 1},
	{0xf8 , 0x38 , "SFZ" , param_bitno , 1},
	{0xf0 , 0x40 , "INA" , param_ds , 1},
	{0xff , 0x5f , "NOP" , nullptr , 1},
	{0xf0 , 0x50 , "OTA" , param_ds , 1},
	{0xf0 , 0x60 , "LDA" , param_reg , 1},
	{0xf0 , 0x70 , "STA" , param_reg , 1},
	{0xf8 , 0x80 , "JMP" , param_11bit , 2},
	{0xf8 , 0x88 , "JSB" , param_11bit , 2 | DASMFLAG_STEP_OVER},
	{0xf8 , 0x90 , "JAI" , param_page_no , 1},
	{0xf8 , 0x98 , "JAS" , param_page_no , 1 | DASMFLAG_STEP_OVER},
	{0xf8 , 0xa0 , "CBN" , param_bitno , 1},
	{0xff , 0xaf , "DSI" , nullptr , 1},
	{0xf8 , 0xa8 , "CLC" , param_bitno , 1},
	{0xff , 0xb0 , "RTI" , nullptr , 1 | DASMFLAG_STEP_OUT},
	{0xff , 0xb1 , "RTE" , nullptr , 1 | DASMFLAG_STEP_OUT},
	{0xff , 0xb4 , "STE" , nullptr , 1},
	{0xff , 0xb5 , "CLE" , nullptr , 1},
	{0xff , 0xb8 , "RTS" , nullptr , 1 | DASMFLAG_STEP_OUT},
	{0xff , 0xb9 , "RSE" , nullptr , 1 | DASMFLAG_STEP_OUT},
	{0xff , 0xcf , "LDR" , param_byte , 2},
	{0xf0 , 0xc0 , "OTR" , param_ds_byte , 2},
	{0xf0 , 0xd0 , "STR" , param_reg_byte , 2},
	{0xf0 , 0xe0 , "LDI" , param_reg , 1},
	{0xf0 , 0xf0 , "STI" , param_reg , 1},
	// Catchall for undefined opcodes
	{0x00 , 0x00 , "???" , nullptr , 1}
};

CPU_DISASSEMBLE(hp_nanoprocessor)
{
    const uint8_t opcode = *oprom;

    opram++;

	for (const dis_entry_t& ent : dis_table) {
		if ((opcode & ent.m_op_mask) == ent.m_opcode) {
			stream << ent.m_mnemonic << ' ';
			if (ent.m_param_fn != nullptr) {
				ent.m_param_fn(stream , opcode , opram);
			}
			return ent.m_dasm_flags | DASMFLAG_SUPPORTED;
		}
	}
	// Should never ever happen
	return 0;
}
