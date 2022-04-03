// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PSXCPU disassembler for the MAME project written by smf
 *
 */

#include "emu.h"
#include "psx.h"
#include "gte.h"

#include "psxdefs.h"
#include "psxdasm.h"


std::string psxcpu_disassembler::make_signed_hex_str_16( uint32_t value )
{
	if( value & 0x8000 )
	{
		return util::string_format("-$%x", -value & 0xffff );
	}
	else
	{
		return util::string_format("$%x", value & 0xffff );
	}
}

const char *const psxcpu_disassembler::s_cpugenreg[] =
{
	"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

const char *const psxcpu_disassembler::s_cp0genreg[] =
{
	"!Index", "!Random", "!EntryLo", "BPC", "!Context", "BDA", "TAR", "DCIC",
	"BadA", "BDAM", "!EntryHi", "BPCM", "SR", "Cause", "EPC", "PRId",
	"cp0r16", "cp0r17", "cp0r18", "cp0r19", "cp0r20", "cp0r21", "cp0r22", "cp0r23",
	"cp0r24", "cp0r25", "cp0r26", "cp0r27", "cp0r28", "cp0r29", "cp0r30", "cp0r31"
};

const char *const psxcpu_disassembler::s_cp0ctlreg[] =
{
	"cp0cr0", "cp0cr1", "cp0cr2", "cp0cr3", "cp0cr4", "cp0cr5", "cp0cr6", "cp0cr7",
	"cp0cr8", "cp0cr9", "cp0cr10", "cp0cr11", "cp0cr12", "cp0cr13", "cp0cr14", "cp0cr15",
	"cp0cr16", "cp0cr17", "cp0cr18", "cp0cr19", "cp0cr20", "cp0cr21", "cp0cr22", "cp0cr23",
	"cp0cr24", "cp0cr25", "cp0cr26", "cp0cr27", "cp0cr28", "cp0cr29", "cp0cr30", "cp0cr31"
};

const char *const psxcpu_disassembler::s_cp1genreg[] =
{
	"cp1r0", "cp1r1", "cp1r2", "cp1r3", "cp1r4", "cp1r5", "cp1r6", "cp1r7",
	"cp1r8", "cp1r9", "cp1r10", "cp1r11", "cp1r12", "cp1r13", "cp1r14", "cp1r15",
	"cp1r16", "cp1r17", "cp1r18", "cp1r19", "cp1r20", "cp1r21", "cp1r22", "cp1r22",
	"cp1r23", "cp1r24", "cp1r25", "cp1r26", "cp1r27", "cp1r28", "cp1r29", "cp1r30"
};

const char *const psxcpu_disassembler::s_cp1ctlreg[] =
{
	"cp1cr0", "cp1cr1", "cp1cr2", "cp1cr3", "cp1cr4", "cp1cr5", "cp1cr6", "cp1cr7",
	"cp1cr8", "cp1cr9", "cp1cr10", "cp1cr11", "cp1cr12", "cp1cr13", "cp1cr14", "cp1cr15",
	"cp1cr16", "cp1cr17", "cp1cr18", "cp1cr19", "cp1cr20", "cp1cr21", "cp1cr22", "cp1cr23",
	"cp1cr24", "cp1cr25", "cp1cr26", "cp1cr27", "cp1cr28", "cp1cr29", "cp1cr30", "cp1cr31"
};

const char *const psxcpu_disassembler::s_cp2genreg[] =
{
	"vxy0", "vz0", "vxy1", "vz1", "vxy2", "vz2", "rgb", "otz",
	"ir0", "ir1", "ir2", "ir3", "sxy0", "sxy1", "sxy2", "sxyp",
	"sz0", "sz1", "sz2", "sz3", "rgb0", "rgb1", "rgb2", "cp2cr23",
	"mac0", "mac1", "mac2", "mac3", "irgb", "orgb", "lzcs", "lzcr"
};

const char *const psxcpu_disassembler::s_cp2ctlreg[] =
{
	"r11r12", "r13r21", "r22r23", "r31r32", "r33", "trx", "try", "trz",
	"l11l12", "l13l21", "l22l23", "l31l32", "l33", "rbk", "gbk", "bbk",
	"lr1lr2", "lr3lg1", "lg2lg3", "lb1lb2", "lb3", "rfc", "gfc", "bfc",
	"ofx", "ofy", "h", "dqa", "dqb", "zsf3", "zsf4", "flag"
};

const char *const psxcpu_disassembler::s_cp3genreg[] =
{
	"cp3r0", "cp3r1", "cp3r2", "cp3r3", "cp3r4", "cp3r5", "cp3r6", "cp3r7",
	"cp3r8", "cp3r9", "cp3r10", "cp3r11", "cp3r12", "cp3r13", "cp3r14", "cp3r15",
	"cp3r16", "cp3r17", "cp3r18", "cp3r19", "cp3r20", "cp3r21", "cp3r22", "cp3r22",
	"cp3r23", "cp3r24", "cp3r25", "cp3r26", "cp3r27", "cp3r28", "cp3r29", "cp3r30"
};

const char *const psxcpu_disassembler::s_cp3ctlreg[] =
{
	"cp3cr0", "cp3cr1", "cp3cr2", "cp3cr3", "cp3cr4", "cp3cr5", "cp3cr6", "cp3cr7",
	"cp3cr8", "cp3cr9", "cp3cr10", "cp3cr11", "cp3cr12", "cp3cr13", "cp3cr14", "cp3cr15",
	"cp3cr16", "cp3cr17", "cp3cr18", "cp3cr19", "cp3cr20", "cp3cr21", "cp3cr22", "cp3cr23",
	"cp3cr24", "cp3cr25", "cp3cr26", "cp3cr27", "cp3cr28", "cp3cr29", "cp3cr30", "cp3cr31"
};

const char *const psxcpu_disassembler::s_gtesf[] =
{
	" sf=0", " sf=12"
};

const char *const psxcpu_disassembler::s_gtemx[] =
{
	"rm", "lm", "cm", "0"
};

const char *const psxcpu_disassembler::s_gtev[] =
{
	"v0", "v1", "v2", "ir"
};

const char *const psxcpu_disassembler::s_gtecv[] =
{
	"tr", "bk", "fc", "0"
};

const char *const psxcpu_disassembler::s_gtelm[] =
{
	" lm=s16", " lm=u15"
};

std::string psxcpu_disassembler::effective_address( uint32_t pc, uint32_t op )
{
	if( m_config && m_config->pc() == pc )
	{
		return util::string_format("%s(%s) ; 0x%08x", make_signed_hex_str_16( INS_IMMEDIATE( op ) ), s_cpugenreg[ INS_RS( op ) ],
								   (uint32_t)( m_config->r( INS_RS( op ) ) + (int16_t)INS_IMMEDIATE( op ) ) );
	}
	return util::string_format("%s(%s)", make_signed_hex_str_16( INS_IMMEDIATE( op ) ), s_cpugenreg[ INS_RS( op ) ] );
}

uint32_t psxcpu_disassembler::relative_address( uint32_t pc, uint32_t op )
{
	uint32_t nextpc = pc + 4;
	if( m_config && m_config->pc() == pc && m_config->delayr() == PSXCPU_DELAYR_PC )
	{
		nextpc = m_config->delayv();
	}

	return nextpc + ( PSXCPU_WORD_EXTEND( INS_IMMEDIATE( op ) ) << 2 );
}

uint32_t psxcpu_disassembler::jump_address( uint32_t pc, uint32_t op )
{
	uint32_t nextpc = pc + 4;
	if( m_config && m_config->pc() == pc && m_config->delayr() == PSXCPU_DELAYR_PC )
	{
		nextpc = m_config->delayv();
	}
	return ( nextpc & 0xf0000000 ) + ( INS_TARGET( op ) << 2 );
}

std::string psxcpu_disassembler::upper_address( uint32_t op, offs_t pos, const data_buffer &opcodes )
{
	uint32_t nextop = opcodes.r32( pos );

	if( INS_OP( nextop ) == OP_ORI && INS_RT( op ) == INS_RS( nextop ) )
	{
		return util::string_format("$%04x ; 0x%08x", INS_IMMEDIATE( op ), ( INS_IMMEDIATE( op ) << 16 ) | INS_IMMEDIATE( nextop ) );
	}
	else if( INS_OP( nextop ) == OP_ADDIU && INS_RT( op ) == INS_RS( nextop ) )
	{
		return util::string_format("$%04x ; 0x%08x", INS_IMMEDIATE( op ), ( INS_IMMEDIATE( op ) << 16 ) + (int16_t) INS_IMMEDIATE( nextop ) );
	}
	else
	{
		return util::string_format("$%04x", INS_IMMEDIATE( op ) );
	}
}

psxcpu_disassembler::psxcpu_disassembler(config *conf) : m_config(conf)
{
}

u32 psxcpu_disassembler::opcode_alignment() const
{
	return 4;
}

offs_t psxcpu_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t op;
	uint32_t flags = 0;
	offs_t pos = pc;
	op = opcodes.r32( pos );
	pos += 4;

	std::streampos current_pos = stream.tellp();

	switch( INS_OP( op ) )
	{
	case OP_SPECIAL:
		switch( INS_FUNCT( op ) )
		{
		case FUNCT_SLL:
			if( op == 0 )
			{
				/* the standard nop is "sll     zero,zero,$0000" */
				util::stream_format( stream, "nop" );
			}
			else
			{
				util::stream_format( stream, "sll     %s,%s,$%02x", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RT( op ) ], INS_SHAMT( op ) );
			}
			break;
		case FUNCT_SRL:
			util::stream_format( stream, "srl     %s,%s,$%02x", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RT( op ) ], INS_SHAMT( op ) );
			break;
		case FUNCT_SRA:
			util::stream_format( stream, "sra     %s,%s,$%02x", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RT( op ) ], INS_SHAMT( op ) );
			break;
		case FUNCT_SLLV:
			util::stream_format( stream, "sllv    %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ] );
			break;
		case FUNCT_SRLV:
			util::stream_format( stream, "srlv    %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ] );
			break;
		case FUNCT_SRAV:
			util::stream_format( stream, "srav    %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ] );
			break;
		case FUNCT_JR:
			util::stream_format( stream, "jr      %s", s_cpugenreg[ INS_RS( op ) ] );
			if( INS_RS( op ) == 31 )
			{
				flags = STEP_OUT;
			}
			break;
		case FUNCT_JALR:
			util::stream_format( stream, "jalr    %s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ] );
			flags = STEP_OVER | step_over_extra( 1 );
			break;
		case FUNCT_SYSCALL:
			util::stream_format( stream, "syscall $%05x", INS_CODE( op ) );
			flags = STEP_OVER;
			break;
		case FUNCT_BREAK:
			util::stream_format( stream, "break   $%05x", INS_CODE( op ) );
			flags = STEP_OVER;
			break;
		case FUNCT_MFHI:
			util::stream_format( stream, "mfhi    %s", s_cpugenreg[ INS_RD( op ) ] );
			break;
		case FUNCT_MTHI:
			util::stream_format( stream, "mthi    %s", s_cpugenreg[ INS_RS( op ) ] );
			break;
		case FUNCT_MFLO:
			util::stream_format( stream, "mflo    %s", s_cpugenreg[ INS_RD( op ) ] );
			break;
		case FUNCT_MTLO:
			util::stream_format( stream, "mtlo    %s", s_cpugenreg[ INS_RS( op ) ] );
			break;
		case FUNCT_MULT:
			util::stream_format( stream, "mult    %s,%s", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_MULTU:
			util::stream_format( stream, "multu   %s,%s", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_DIV:
			util::stream_format( stream, "div     %s,%s", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_DIVU:
			util::stream_format( stream, "divu    %s,%s", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_ADD:
			util::stream_format( stream, "add     %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_ADDU:
			util::stream_format( stream, "addu    %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_SUB:
			util::stream_format( stream, "sub     %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_SUBU:
			util::stream_format( stream, "subu    %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_AND:
			util::stream_format( stream, "and     %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_OR:
			util::stream_format( stream, "or      %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_XOR:
			util::stream_format( stream, "xor     %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_NOR:
			util::stream_format( stream, "nor     %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_SLT:
			util::stream_format( stream, "slt     %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_SLTU:
			util::stream_format( stream, "sltu    %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		}
		break;
	case OP_REGIMM:
		switch( INS_RT_REGIMM( op ) )
		{
		case RT_BLTZ:
			if( INS_RT( op ) == RT_BLTZAL )
			{
				util::stream_format( stream, "bltzal  %s,$%08x", s_cpugenreg[ INS_RS( op ) ], relative_address( pc, op ) );
				if( INS_RS( op ) != 0 )
					flags = STEP_OVER | STEP_COND | step_over_extra( 1 );
			}
			else
			{
				util::stream_format( stream, "bltz    %s,$%08x", s_cpugenreg[ INS_RS( op ) ], relative_address( pc, op ) );
				if( INS_RS( op ) != 0 )
					flags = STEP_COND | step_over_extra( 1 );
			}
			break;
		case RT_BGEZ:
			if( INS_RT( op ) == RT_BGEZAL )
			{
				util::stream_format( stream, "bgezal  %s,$%08x", s_cpugenreg[ INS_RS( op ) ], relative_address( pc, op ) );
				flags = STEP_OVER | step_over_extra( 1 );
				if( INS_RS( op ) != 0 )
					flags |= STEP_COND;
			}
			else
			{
				util::stream_format( stream, "bgez    %s,$%08x", s_cpugenreg[ INS_RS( op ) ], relative_address( pc, op ) );
				if( INS_RS( op ) != 0 )
					flags = STEP_COND | step_over_extra( 1 );
			}
			break;
		}
		break;
	case OP_J:
		util::stream_format( stream, "j       $%08x", jump_address( pc, op ) );
		break;
	case OP_JAL:
		util::stream_format( stream, "jal     $%08x", jump_address( pc, op ) );
		flags = STEP_OVER | step_over_extra( 1 );
		break;
	case OP_BEQ:
		util::stream_format( stream, "beq     %s,%s,$%08x", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ], relative_address( pc, op ) );
		flags = STEP_COND | step_over_extra( 1 );
		break;
	case OP_BNE:
		util::stream_format( stream, "bne     %s,%s,$%08x", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ], relative_address( pc, op ) );
		flags = STEP_COND | step_over_extra( 1 );
		break;
	case OP_BLEZ:
		util::stream_format( stream, "blez    %s,%s,$%08x", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ], relative_address( pc, op ) );
		flags = STEP_COND | step_over_extra( 1 );
		break;
	case OP_BGTZ:
		util::stream_format( stream, "bgtz    %s,%s,$%08x", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ], relative_address( pc, op ) );
		flags = STEP_COND | step_over_extra( 1 );
		break;
	case OP_ADDI:
		util::stream_format( stream, "addi    %s,%s,%s", s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ], make_signed_hex_str_16( INS_IMMEDIATE( op ) ) );
		break;
	case OP_ADDIU:
		util::stream_format( stream, "addiu   %s,%s,%s", s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ], make_signed_hex_str_16( INS_IMMEDIATE( op ) ) );
		break;
	case OP_SLTI:
		util::stream_format( stream, "slti    %s,%s,%s", s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ], make_signed_hex_str_16( INS_IMMEDIATE( op ) ) );
		break;
	case OP_SLTIU:
		util::stream_format( stream, "sltiu   %s,%s,%s", s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ], make_signed_hex_str_16( INS_IMMEDIATE( op ) ) );
		break;
	case OP_ANDI:
		util::stream_format( stream, "andi    %s,%s,$%04x", s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ], INS_IMMEDIATE( op ) );
		break;
	case OP_ORI:
		util::stream_format( stream, "ori     %s,%s,$%04x", s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ], INS_IMMEDIATE( op ) );
		break;
	case OP_XORI:
		util::stream_format( stream, "xori    %s,%s,$%04x", s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ], INS_IMMEDIATE( op ) );
		break;
	case OP_LUI:
		util::stream_format( stream, "lui     %s,%s", s_cpugenreg[ INS_RT( op ) ], upper_address( op, pos, opcodes ) );
		break;
	case OP_COP0:
		switch( INS_RS( op ) )
		{
		case RS_MFC:
			util::stream_format( stream, "mfc0    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp0genreg[ INS_RD( op ) ] );
			break;
		case RS_CFC:
			util::stream_format( stream, "!cfc0    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp0ctlreg[ INS_RD( op ) ] );
			break;
		case RS_MTC:
			util::stream_format( stream, "mtc0    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp0genreg[ INS_RD( op ) ] );
			break;
		case RS_CTC:
			util::stream_format( stream, "!ctc0    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp0ctlreg[ INS_RD( op ) ] );
			break;
		case RS_BC:
		case RS_BC_ALT:
			switch( INS_BC( op ) )
			{
			case BC_BCF:
				util::stream_format( stream, "bc0f    $%08x", relative_address( pc, op ) );
				break;
			case BC_BCT:
				util::stream_format( stream, "bc0t    $%08x", relative_address( pc, op ) );
				break;
			}
			flags = STEP_COND | step_over_extra( 1 );
			break;
		default:
			switch( INS_CO( op ) )
			{
			case 1:
				switch( INS_CF( op ) )
				{
				case CF_TLBR:
					util::stream_format( stream, "!tlbr" );
					break;
				case CF_TLBWI:
					util::stream_format( stream, "!tlbwi" );
					break;
				case CF_TLBWR:
					util::stream_format( stream, "!tlbwr" );
					break;
				case CF_TLBP:
					util::stream_format( stream, "!tlbp" );
					break;
				case CF_RFE:
					util::stream_format( stream, "rfe" );
					break;
				default:
					util::stream_format(stream, "cop0    $%07x", INS_COFUN(op));
					break;
				}
				break;
			}
			break;
		}
		break;
	case OP_COP1:
		switch( INS_RS( op ) )
		{
		case RS_MFC:
			util::stream_format( stream, "mfc1    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp1genreg[ INS_RD( op ) ] );
			break;
		case RS_CFC:
			util::stream_format( stream, "cfc1    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp1ctlreg[ INS_RD( op ) ] );
			break;
		case RS_MTC:
			util::stream_format( stream, "mtc1    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp1genreg[ INS_RD( op ) ] );
			break;
		case RS_CTC:
			util::stream_format( stream, "ctc1    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp1ctlreg[ INS_RD( op ) ] );
			break;
		case RS_BC:
		case RS_BC_ALT:
			switch( INS_BC( op ) )
			{
			case BC_BCF:
				util::stream_format( stream, "bc1f    $%08x", relative_address( pc, op ) );
				break;
			case BC_BCT:
				util::stream_format( stream, "bc1t    $%08x", relative_address( pc, op ) );
				break;
			}
			flags = STEP_COND | step_over_extra( 1 );
			break;
		default:
			switch( INS_CO( op ) )
			{
			case 1:
				util::stream_format( stream, "cop1    $%07x", INS_COFUN( op ) );
				break;
			}
			break;
		}
		break;
	case OP_COP2:
		switch( INS_RS( op ) )
		{
		case RS_MFC:
			util::stream_format( stream, "mfc2    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp2genreg[ INS_RD( op ) ] );
			break;
		case RS_CFC:
			util::stream_format( stream, "cfc2    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp2ctlreg[ INS_RD( op ) ] );
			break;
		case RS_MTC:
			util::stream_format( stream, "mtc2    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp2genreg[ INS_RD( op ) ] );
			break;
		case RS_CTC:
			util::stream_format( stream, "ctc2    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp2ctlreg[ INS_RD( op ) ] );
			break;
		case RS_BC:
		case RS_BC_ALT:
			switch( INS_BC( op ) )
			{
			case BC_BCF:
				util::stream_format( stream, "bc2f    $%08x", relative_address( pc, op ) );
				break;
			case BC_BCT:
				util::stream_format( stream, "bc2t    $%08x", relative_address( pc, op ) );
				break;
			}
			flags = STEP_COND | step_over_extra( 1 );
			break;
		default:
			switch( INS_CO( op ) )
			{
			case 1:
				switch( GTE_FUNCT( op ) )
				{
				case 0x00: // drop through to RTPS
				case 0x01:
					util::stream_format( stream, "rtps%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x06:
					util::stream_format( stream, "nclip" );
					break;
				case 0x0c:
					util::stream_format( stream, "op%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x10:
					util::stream_format( stream, "dpcs%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x11:
					util::stream_format( stream, "intpl%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x12:
					util::stream_format( stream, "mvmva%s%s %s + %s * %s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ],
						s_gtecv[ GTE_CV( op ) ], s_gtemx[ GTE_MX( op ) ], s_gtev[ GTE_V( op ) ] );
					break;
				case 0x13:
					util::stream_format( stream, "ncds%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x14:
					util::stream_format( stream, "cdp%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x16:
					util::stream_format( stream, "ncdt%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x1b:
					util::stream_format( stream, "nccs%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x1c:
					util::stream_format( stream, "cc%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x1e:
					util::stream_format( stream, "ncs%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x20:
					util::stream_format( stream, "nct%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x28:
					util::stream_format( stream, "sqr%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x1a: // end of NCDT
				case 0x29:
					util::stream_format( stream, "dpcl%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x2a:
					util::stream_format( stream, "dpct%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x2d:
					util::stream_format( stream, "avsz3" );
					break;
				case 0x2e:
					util::stream_format( stream, "avsz4" );
					break;
				case 0x30:
					util::stream_format( stream, "rtpt%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x3d:
					util::stream_format( stream, "gpf%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x3e:
					util::stream_format( stream, "gpl%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x3f:
					util::stream_format( stream, "ncct%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				default:
					util::stream_format(stream, "cop2    $%07x", INS_COFUN(op));
					break;
				}
			}
			break;
		}
		break;
	case OP_COP3:
		switch( INS_RS( op ) )
		{
		case RS_MFC:
			util::stream_format( stream, "mfc3    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp3genreg[ INS_RD( op ) ] );
			break;
		case RS_CFC:
			util::stream_format( stream, "cfc3    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp3ctlreg[ INS_RD( op ) ] );
			break;
		case RS_MTC:
			util::stream_format( stream, "mtc3    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp3genreg[ INS_RD( op ) ] );
			break;
		case RS_CTC:
			util::stream_format( stream, "ctc3    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp3ctlreg[ INS_RD( op ) ] );
			break;
		case RS_BC:
		case RS_BC_ALT:
			switch( INS_BC( op ) )
			{
			case BC_BCF:
				util::stream_format( stream, "bc3f    $%08x", relative_address( pc, op ) );
				break;
			case BC_BCT:
				util::stream_format( stream, "bc3t    $%08x", relative_address( pc, op ) );
				break;
			}
			flags = STEP_COND | step_over_extra( 1 );
			break;
		default:
			switch( INS_CO( op ) )
			{
			case 1:
				util::stream_format( stream, "cop3    $%07x", INS_COFUN( op ) );
				break;
			}
			break;
		}
		break;
	case OP_LB:
		util::stream_format( stream, "lb      %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_LH:
		util::stream_format( stream, "lh      %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_LWL:
		util::stream_format( stream, "lwl     %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_LW:
		util::stream_format( stream, "lw      %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_LBU:
		util::stream_format( stream, "lbu     %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_LHU:
		util::stream_format( stream, "lhu     %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_LWR:
		util::stream_format( stream, "lwr     %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_SB:
		util::stream_format( stream, "sb      %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_SH:
		util::stream_format( stream, "sh      %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_SWL:
		util::stream_format( stream, "swl     %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_SW:
		util::stream_format( stream, "sw      %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_SWR:
		util::stream_format( stream, "swr     %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_LWC0:
		util::stream_format( stream, "lwc0    %s,%s", s_cp0genreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_LWC1:
		util::stream_format( stream, "lwc1    %s,%s", s_cp1genreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_LWC2:
		util::stream_format( stream, "lwc2    %s,%s", s_cp2genreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_LWC3:
		util::stream_format( stream, "lwc3    %s,%s", s_cp2genreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_SWC0:
		util::stream_format( stream, "swc0    %s,%s", s_cp0genreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_SWC1:
		util::stream_format( stream, "swc1    %s,%s", s_cp1genreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_SWC2:
		util::stream_format( stream, "swc2    %s,%s", s_cp2genreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	case OP_SWC3:
		util::stream_format( stream, "swc3    %s,%s", s_cp2genreg[ INS_RT( op ) ], effective_address( pc, op ) );
		break;
	}

	// fall back if we have not emitted anything
	if (current_pos == stream.tellp())
	{
		util::stream_format(stream, "dw      $%08x", op);
	}

	return ( pos - pc ) | flags | SUPPORTED;
}
