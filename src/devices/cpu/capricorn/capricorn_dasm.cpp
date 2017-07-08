// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// ********************************************************************************
// * HP Capricorn processor disassembler
// ********************************************************************************

#include "capricorn.h"

typedef offs_t (*fn_dis_param)(std::ostream &stream , offs_t pc , const uint8_t *oprom);

typedef struct {
	uint8_t m_op_mask;
	uint8_t m_opcode;
	const char *m_mnemonic;
	bool m_has_mb;
	char m_addr_mode;
	fn_dis_param m_param_fn;
	uint32_t m_dasm_flags;
} dis_entry_t;

static void direct_addr(std::ostream &stream , const uint8_t *oprom)
{
	util::stream_format(stream , "$%02x%02x" , oprom[ 1 ] , oprom[ 0 ]);
}

static offs_t param_arp_drp(std::ostream &stream , offs_t pc , const uint8_t *oprom)
{
	stream << "R";
	util::stream_format(stream , "%02o" , oprom[ 0 ] & 0x3f);
	return 0;
}

static offs_t param_dr(std::ostream &stream , offs_t pc , const uint8_t *oprom)
{
	stream << "DR";
	return 0;
}

static offs_t param_dr_ar(std::ostream &stream , offs_t pc , const uint8_t *oprom)
{
	stream << "DR,AR";
	return 0;
}

static offs_t param_dr_lit(std::ostream &stream , offs_t pc , const uint8_t *oprom)
{
	stream << "DR,=";
	// Here we assume that multi-byte instructions operate on 2 bytes because we
	// have no way of knowing how many they are (the actual number of bytes is
	// dynamically determined by the value of DRP register at run-time)
	unsigned bytes = BIT(oprom[ 0 ] , 0) ? 2 : 1;

	for (unsigned i = 1; i <= bytes; i++) {
		util::stream_format(stream , "$%02x " , oprom[ i ]);
	}

	return bytes;
}

static offs_t param_dr_lit_dir(std::ostream &stream , offs_t pc ,const uint8_t *oprom)
{
	stream << "DR,=";
	direct_addr(stream , &oprom[ 1 ]);
	return 2;
}

static offs_t param_dr_idx_dir(std::ostream &stream , offs_t pc ,const uint8_t *oprom)
{
	stream << "DR,XAR,";
	direct_addr(stream , &oprom[ 1 ]);
	return 2;
}

static offs_t param_xr_lit(std::ostream &stream , offs_t pc ,const uint8_t *oprom)
{
	stream << "XR,";
	direct_addr(stream , &oprom[ 1 ]);
	return 2;
}

static offs_t param_lit_dir(std::ostream &stream , offs_t pc ,const uint8_t *oprom)
{
	stream << "=";
	direct_addr(stream , &oprom[ 1 ]);
	return 2;
}

static offs_t param_dr_id_ar(std::ostream &stream , offs_t pc , const uint8_t *oprom)
{
	stream << "DR," << (BIT(oprom[ 0 ] , 1) ? '-' : '+') << "AR";
	return 0;
}

static offs_t param_jmp_off(std::ostream &stream , offs_t pc , const uint8_t *oprom)
{
	uint16_t off = oprom[ 1 ];
	if (BIT(off , 7)) {
		off -= 0x100;
	}
	util::stream_format(stream , "$%04x" , (pc + 2 + off) & 0xffff);
	return 1;
}

static const dis_entry_t dis_table[] = {
	{ 0xff , 0x01 , "ARP R*" , false , '\0' , nullptr , 0 },
	{ 0xc0 , 0x00 , "ARP"    , false , '\0' , param_arp_drp , 0 },
	{ 0xff , 0x41 , "DRP R*" , false , '\0' , nullptr , 0 },
	{ 0xc0 , 0x40 , "DRP"    , false , '\0' , param_arp_drp , 0 },
	{ 0xfe , 0x80 , "EL"     , true  , '\0' , param_dr , 0 },
	{ 0xfe , 0x82 , "ER"     , true  , '\0' , param_dr , 0 },
	{ 0xfe , 0x84 , "LL"     , true  , '\0' , param_dr , 0 },
	{ 0xfe , 0x86 , "LR"     , true  , '\0' , param_dr , 0 },
	{ 0xfe , 0x88 , "IC"     , true  , '\0' , param_dr , 0 },
	{ 0xfe , 0x8a , "DC"     , true  , '\0' , param_dr , 0 },
	{ 0xfe , 0x8c , "TC"     , true  , '\0' , param_dr , 0 },
	{ 0xfe , 0x8e , "NC"     , true  , '\0' , param_dr , 0 },
	{ 0xfe , 0x90 , "TS"     , true  , '\0' , param_dr , 0 },
	{ 0xfe , 0x92 , "CL"     , true  , '\0' , param_dr , 0 },
	{ 0xfe , 0x94 , "OR"     , true  , '\0' , param_dr_ar , 0 },
	{ 0xfe , 0x96 , "XR"     , true  , '\0' , param_dr_ar , 0 },
	{ 0xff , 0x98 , "BIN"    , false , '\0' , nullptr , 0 },
	{ 0xff , 0x99 , "BCD"    , false , '\0' , nullptr , 0 },
	{ 0xff , 0x9a , "SAD"    , false , '\0' , nullptr , 0 },
	{ 0xff , 0x9b , "DCE"    , false , '\0' , nullptr , 0 },
	{ 0xff , 0x9c , "ICE"    , false , '\0' , nullptr , 0 },
	{ 0xff , 0x9d , "CLE"    , false , '\0' , nullptr , 0 },
	{ 0xff , 0x9e , "RTN"    , false , '\0' , nullptr , DASMFLAG_STEP_OUT },
	{ 0xff , 0x9f , "PAD"    , false , '\0' , nullptr , 0 },
	{ 0xfe , 0xa0 , "LD"     , true  , '\0' , param_dr_ar , 0 },
	{ 0xfe , 0xa2 , "ST"     , true  , '\0' , param_dr_ar , 0 },
	{ 0xfe , 0xa4 , "LD"     , true  , 'D'  , param_dr_ar , 0 },
	{ 0xfe , 0xa6 , "ST"     , true  , 'D'  , param_dr_ar , 0 },
	{ 0xfe , 0xa8 , "LD"     , true  , '\0' , param_dr_lit , 0 },
	{ 0xfe , 0xaa , "ST"     , true  , '\0' , param_dr_lit , 0 },
	{ 0xfe , 0xac , "LD"     , true  , 'I'  , param_dr_ar , 0 },
	{ 0xfe , 0xae , "ST"     , true  , 'I'  , param_dr_ar , 0 },
	{ 0xfe , 0xb0 , "LD"     , true  , 'D'  , param_dr_lit_dir , 0 },
	{ 0xfe , 0xb2 , "ST"     , true  , 'D'  , param_dr_lit_dir , 0 },
	{ 0xfe , 0xb4 , "LD"     , true  , 'D'  , param_dr_idx_dir , 0 },
	{ 0xfe , 0xb6 , "ST"     , true  , 'D'  , param_dr_idx_dir , 0 },
	{ 0xfe , 0xb8 , "LD"     , true  , 'I'  , param_dr_lit_dir , 0 },
	{ 0xfe , 0xba , "ST"     , true  , 'I'  , param_dr_lit_dir , 0 },
	{ 0xfe , 0xbc , "LD"     , true  , 'I'  , param_dr_idx_dir , 0 },
	{ 0xfe , 0xbe , "ST"     , true  , 'I'  , param_dr_idx_dir , 0 },
	{ 0xfe , 0xc0 , "CM"     , true  , '\0' , param_dr_ar , 0 },
	{ 0xfe , 0xc2 , "AD"     , true  , '\0' , param_dr_ar , 0 },
	{ 0xfe , 0xc4 , "SB"     , true  , '\0' , param_dr_ar , 0 },
	{ 0xff , 0xc6 , "JSB"    , false , '\0' , param_xr_lit , DASMFLAG_STEP_OVER },
	{ 0xff , 0xc7 , "ANM"    , false , '\0' , param_dr_ar , 0 },
	{ 0xfe , 0xc8 , "CM"     , true  , '\0' , param_dr_lit , 0 },
	{ 0xfe , 0xca , "AD"     , true  , '\0' , param_dr_lit , 0 },
	{ 0xfe , 0xcc , "SB"     , true  , '\0' , param_dr_lit , 0 },
	{ 0xff , 0xce , "JSB"    , false , '\0' , param_lit_dir , DASMFLAG_STEP_OVER },
	{ 0xff , 0xcf , "ANM"    , false , '\0' , param_dr_lit , 0 },
	{ 0xfe , 0xd0 , "CM"     , true  , 'D'  , param_dr_lit_dir , 0 },
	{ 0xfe , 0xd2 , "AD"     , true  , 'D'  , param_dr_lit_dir , 0 },
	{ 0xfe , 0xd4 , "SB"     , true  , 'D'  , param_dr_lit_dir , 0 },
	{ 0xff , 0xd7 , "ANM"    , false , 'D'  , param_dr_lit_dir , 0 },
	{ 0xfe , 0xd8 , "CM"     , true  , 'D'  , param_dr_ar , 0 },
	{ 0xfe , 0xda , "AD"     , true  , 'D'  , param_dr_ar , 0 },
	{ 0xfe , 0xdc , "SB"     , true  , 'D'  , param_dr_ar , 0 },
	{ 0xff , 0xdf , "ANM"    , false , 'D'  , param_dr_ar , 0 },
	{ 0xfc , 0xe0 , "PO"     , true  , 'D'  , param_dr_id_ar , 0 },
	{ 0xfc , 0xe4 , "PU"     , true  , 'D'  , param_dr_id_ar , 0 },
	{ 0xfc , 0xe8 , "PO"     , true  , 'I'  , param_dr_id_ar , 0 },
	{ 0xfc , 0xec , "PU"     , true  , 'I'  , param_dr_id_ar , 0 },
	{ 0xff , 0xf0 , "JMP"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xf1 , "JNO"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xf2 , "JOD"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xf3 , "JEV"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xf4 , "JNG"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xf5 , "JPS"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xf6 , "JNZ"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xf7 , "JZR"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xf8 , "JEN"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xf9 , "JEZ"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xfa , "JNC"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xfb , "JCY"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xfc , "JLZ"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xfd , "JLN"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xfe , "JRZ"    , false , '\0' , param_jmp_off , 0 },
	{ 0xff , 0xff , "JRN"    , false , '\0' , param_jmp_off , 0 },
	// *** END ***
	{0 , 0 , nullptr , false , 0 , nullptr , 0 }
};

CPU_DISASSEMBLE(capricorn)
{
	const dis_entry_t *p;
	uint8_t opcode = oprom[ 0 ];

	for (p = dis_table; p->m_op_mask; p++) {
		if ((opcode & p->m_op_mask) == p->m_opcode) {
			offs_t res = 1 | p->m_dasm_flags | DASMFLAG_SUPPORTED;
			stream << p->m_mnemonic;
			if (p->m_has_mb) {
				stream << (BIT(opcode , 0) ? 'M' : 'B');
			}
			if (p->m_addr_mode != '\0') {
				stream << p->m_addr_mode;
			}
			if (p->m_param_fn != nullptr) {
				stream << " ";
				res += p->m_param_fn(stream , pc , oprom);
			}
			return res;
		}
	}

	// Unknown opcode
	stream << "???";
	return 1 | DASMFLAG_SUPPORTED;
}
