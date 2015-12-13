// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PSXCPU disassembler for the MAME project written by smf
 *
 */

#include "emu.h"
#include "psx.h"
#include "gte.h"

static char *make_signed_hex_str_16( UINT32 value )
{
	static char s_hex[ 20 ];

	if( value & 0x8000 )
	{
		sprintf( s_hex, "-$%x", -value & 0xffff );
	}
	else
	{
		sprintf( s_hex, "$%x", value & 0xffff );
	}

	return s_hex;
}

static const char *const s_cpugenreg[] =
{
	"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

static const char *const s_cp0genreg[] =
{
	"!Index", "!Random", "!EntryLo", "BPC", "!Context", "BDA", "TAR", "DCIC",
	"BadA", "BDAM", "!EntryHi", "BPCM", "SR", "Cause", "EPC", "PRId",
	"cp0r16", "cp0r17", "cp0r18", "cp0r19", "cp0r20", "cp0r21", "cp0r22", "cp0r23",
	"cp0r24", "cp0r25", "cp0r26", "cp0r27", "cp0r28", "cp0r29", "cp0r30", "cp0r31"
};

static const char *const s_cp0ctlreg[] =
{
	"cp0cr0", "cp0cr1", "cp0cr2", "cp0cr3", "cp0cr4", "cp0cr5", "cp0cr6", "cp0cr7",
	"cp0cr8", "cp0cr9", "cp0cr10", "cp0cr11", "cp0cr12", "cp0cr13", "cp0cr14", "cp0cr15",
	"cp0cr16", "cp0cr17", "cp0cr18", "cp0cr19", "cp0cr20", "cp0cr21", "cp0cr22", "cp0cr23",
	"cp0cr24", "cp0cr25", "cp0cr26", "cp0cr27", "cp0cr28", "cp0cr29", "cp0cr30", "cp0cr31"
};

static const char *const s_cp1genreg[] =
{
	"cp1r0", "cp1r1", "cp1r2", "cp1r3", "cp1r4", "cp1r5", "cp1r6", "cp1r7",
	"cp1r8", "cp1r9", "cp1r10", "cp1r11", "cp1r12", "cp1r13", "cp1r14", "cp1r15",
	"cp1r16", "cp1r17", "cp1r18", "cp1r19", "cp1r20", "cp1r21", "cp1r22", "cp1r22",
	"cp1r23", "cp1r24", "cp1r25", "cp1r26", "cp1r27", "cp1r28", "cp1r29", "cp1r30"
};

static const char *const s_cp1ctlreg[] =
{
	"cp1cr0", "cp1cr1", "cp1cr2", "cp1cr3", "cp1cr4", "cp1cr5", "cp1cr6", "cp1cr7",
	"cp1cr8", "cp1cr9", "cp1cr10", "cp1cr11", "cp1cr12", "cp1cr13", "cp1cr14", "cp1cr15",
	"cp1cr16", "cp1cr17", "cp1cr18", "cp1cr19", "cp1cr20", "cp1cr21", "cp1cr22", "cp1cr23",
	"cp1cr24", "cp1cr25", "cp1cr26", "cp1cr27", "cp1cr28", "cp1cr29", "cp1cr30", "cp1cr31"
};

static const char *const s_cp2genreg[] =
{
	"vxy0", "vz0", "vxy1", "vz1", "vxy2", "vz2", "rgb", "otz",
	"ir0", "ir1", "ir2", "ir3", "sxy0", "sxy1", "sxy2", "sxyp",
	"sz0", "sz1", "sz2", "sz3", "rgb0", "rgb1", "rgb2", "cp2cr23",
	"mac0", "mac1", "mac2", "mac3", "irgb", "orgb", "lzcs", "lzcr"
};

static const char *const s_cp2ctlreg[] =
{
	"r11r12", "r13r21", "r22r23", "r31r32", "r33", "trx", "try", "trz",
	"l11l12", "l13l21", "l22l23", "l31l32", "l33", "rbk", "gbk", "bbk",
	"lr1lr2", "lr3lg1", "lg2lg3", "lb1lb2", "lb3", "rfc", "gfc", "bfc",
	"ofx", "ofy", "h", "dqa", "dqb", "zsf3", "zsf4", "flag"
};

static const char *const s_cp3genreg[] =
{
	"cp3r0", "cp3r1", "cp3r2", "cp3r3", "cp3r4", "cp3r5", "cp3r6", "cp3r7",
	"cp3r8", "cp3r9", "cp3r10", "cp3r11", "cp3r12", "cp3r13", "cp3r14", "cp3r15",
	"cp3r16", "cp3r17", "cp3r18", "cp3r19", "cp3r20", "cp3r21", "cp3r22", "cp3r22",
	"cp3r23", "cp3r24", "cp3r25", "cp3r26", "cp3r27", "cp3r28", "cp3r29", "cp3r30"
};

static const char *const s_cp3ctlreg[] =
{
	"cp3cr0", "cp3cr1", "cp3cr2", "cp3cr3", "cp3cr4", "cp3cr5", "cp3cr6", "cp3cr7",
	"cp3cr8", "cp3cr9", "cp3cr10", "cp3cr11", "cp3cr12", "cp3cr13", "cp3cr14", "cp3cr15",
	"cp3cr16", "cp3cr17", "cp3cr18", "cp3cr19", "cp3cr20", "cp3cr21", "cp3cr22", "cp3cr23",
	"cp3cr24", "cp3cr25", "cp3cr26", "cp3cr27", "cp3cr28", "cp3cr29", "cp3cr30", "cp3cr31"
};

static const char *const s_gtesf[] =
{
	" sf=0", " sf=12"
};

static const char *const s_gtemx[] =
{
	"rm", "lm", "cm", "0"
};

static const char *const s_gtev[] =
{
	"v0", "v1", "v2", "ir"
};

static const char *const s_gtecv[] =
{
	"tr", "bk", "fc", "0"
};

static const char *const s_gtelm[] =
{
	" lm=s16", " lm=u15"
};

static char *effective_address( psxcpu_state *state, UINT32 pc, UINT32 op )
{
	static char s_address[ 20 ];

	if( state != nullptr && state->pc() == pc )
	{
		sprintf( s_address, "%s(%s) ; 0x%08x", make_signed_hex_str_16( INS_IMMEDIATE( op ) ), s_cpugenreg[ INS_RS( op ) ],
			(UINT32)( state->r( INS_RS( op ) ) + (INT16)INS_IMMEDIATE( op ) ) );
		return s_address;
	}
	sprintf( s_address, "%s(%s)", make_signed_hex_str_16( INS_IMMEDIATE( op ) ), s_cpugenreg[ INS_RS( op ) ] );
	return s_address;
}

static UINT32 relative_address( psxcpu_state *state, UINT32 pc, UINT32 op )
{
	UINT32 nextpc = pc + 4;
	if( state != nullptr && state->pc() == pc && state->delayr() == PSXCPU_DELAYR_PC )
	{
		nextpc = state->delayv();
	}

	return nextpc + ( PSXCPU_WORD_EXTEND( INS_IMMEDIATE( op ) ) << 2 );
}

static UINT32 jump_address( psxcpu_state *state, UINT32 pc, UINT32 op )
{
	UINT32 nextpc = pc + 4;
	if( state != nullptr && state->pc() == pc && state->delayr() == PSXCPU_DELAYR_PC )
	{
		nextpc = state->delayv();
	}
	return ( nextpc & 0xf0000000 ) + ( INS_TARGET( op ) << 2 );
}

static UINT32 fetch_op( const UINT8 *opram )
{
	return ( opram[ 3 ] << 24 ) | ( opram[ 2 ] << 16 ) | ( opram[ 1 ] << 8 ) | ( opram[ 0 ] << 0 );
}

static char *upper_address( UINT32 op, const UINT8 *opram )
{
	static char s_address[ 20 ];
	UINT32 nextop = fetch_op( opram );

	if( INS_OP( nextop ) == OP_ORI && INS_RT( op ) == INS_RS( nextop ) )
	{
		sprintf( s_address, "$%04x ; 0x%08x", INS_IMMEDIATE( op ), ( INS_IMMEDIATE( op ) << 16 ) | INS_IMMEDIATE( nextop ) );
	}
	else if( INS_OP( nextop ) == OP_ADDIU && INS_RT( op ) == INS_RS( nextop ) )
	{
		sprintf( s_address, "$%04x ; 0x%08x", INS_IMMEDIATE( op ), ( INS_IMMEDIATE( op ) << 16 ) + (INT16) INS_IMMEDIATE( nextop ) );
	}
	else
	{
		sprintf( s_address, "$%04x", INS_IMMEDIATE( op ) );
	}

	return s_address;
}

unsigned DasmPSXCPU( psxcpu_state *state, char *buffer, UINT32 pc, const UINT8 *opram )
{
	UINT32 op;
	const UINT8 *oldopram;
	UINT32 flags = 0;

	oldopram = opram;
	op = fetch_op( opram );
	opram += 4;

	sprintf( buffer, "dw      $%08x", op );

	switch( INS_OP( op ) )
	{
	case OP_SPECIAL:
		switch( INS_FUNCT( op ) )
		{
		case FUNCT_SLL:
			if( op == 0 )
			{
				/* the standard nop is "sll     zero,zero,$0000" */
				sprintf( buffer, "nop" );
			}
			else
			{
				sprintf( buffer, "sll     %s,%s,$%02x", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RT( op ) ], INS_SHAMT( op ) );
			}
			break;
		case FUNCT_SRL:
			sprintf( buffer, "srl     %s,%s,$%02x", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RT( op ) ], INS_SHAMT( op ) );
			break;
		case FUNCT_SRA:
			sprintf( buffer, "sra     %s,%s,$%02x", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RT( op ) ], INS_SHAMT( op ) );
			break;
		case FUNCT_SLLV:
			sprintf( buffer, "sllv    %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ] );
			break;
		case FUNCT_SRLV:
			sprintf( buffer, "srlv    %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ] );
			break;
		case FUNCT_SRAV:
			sprintf( buffer, "srav    %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ] );
			break;
		case FUNCT_JR:
			sprintf( buffer, "jr      %s", s_cpugenreg[ INS_RS( op ) ] );
			if( INS_RS( op ) == 31 )
			{
				flags = DASMFLAG_STEP_OUT;
			}
			break;
		case FUNCT_JALR:
			sprintf( buffer, "jalr    %s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ] );
			flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA( 1 );
			break;
		case FUNCT_SYSCALL:
			sprintf( buffer, "syscall $%05x", INS_CODE( op ) );
			flags = DASMFLAG_STEP_OVER;
			break;
		case FUNCT_BREAK:
			sprintf( buffer, "break   $%05x", INS_CODE( op ) );
			flags = DASMFLAG_STEP_OVER;
			break;
		case FUNCT_MFHI:
			sprintf( buffer, "mfhi    %s", s_cpugenreg[ INS_RD( op ) ] );
			break;
		case FUNCT_MTHI:
			sprintf( buffer, "mthi    %s", s_cpugenreg[ INS_RS( op ) ] );
			break;
		case FUNCT_MFLO:
			sprintf( buffer, "mflo    %s", s_cpugenreg[ INS_RD( op ) ] );
			break;
		case FUNCT_MTLO:
			sprintf( buffer, "mtlo    %s", s_cpugenreg[ INS_RS( op ) ] );
			break;
		case FUNCT_MULT:
			sprintf( buffer, "mult    %s,%s", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_MULTU:
			sprintf( buffer, "multu   %s,%s", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_DIV:
			sprintf( buffer, "div     %s,%s", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_DIVU:
			sprintf( buffer, "divu    %s,%s", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_ADD:
			sprintf( buffer, "add     %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_ADDU:
			sprintf( buffer, "addu    %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_SUB:
			sprintf( buffer, "sub     %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_SUBU:
			sprintf( buffer, "subu    %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_AND:
			sprintf( buffer, "and     %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_OR:
			sprintf( buffer, "or      %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_XOR:
			sprintf( buffer, "xor     %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_NOR:
			sprintf( buffer, "nor     %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_SLT:
			sprintf( buffer, "slt     %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		case FUNCT_SLTU:
			sprintf( buffer, "sltu    %s,%s,%s", s_cpugenreg[ INS_RD( op ) ], s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ] );
			break;
		}
		break;
	case OP_REGIMM:
		switch( INS_RT_REGIMM( op ) )
		{
		case RT_BLTZ:
			if( INS_RT( op ) == RT_BLTZAL )
			{
				sprintf( buffer, "bltzal  %s,$%08x", s_cpugenreg[ INS_RS( op ) ], relative_address( state, pc, op ) );
				flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA( 1 );
			}
			else
			{
				sprintf( buffer, "bltz    %s,$%08x", s_cpugenreg[ INS_RS( op ) ], relative_address( state, pc, op ) );
			}
			break;
		case RT_BGEZ:
			if( INS_RT( op ) == RT_BGEZAL )
			{
				sprintf( buffer, "bgezal  %s,$%08x", s_cpugenreg[ INS_RS( op ) ], relative_address( state, pc, op ) );
				flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA( 1 );
			}
			else
			{
				sprintf( buffer, "bgez    %s,$%08x", s_cpugenreg[ INS_RS( op ) ], relative_address( state, pc, op ) );
			}
			break;
		}
		break;
	case OP_J:
		sprintf( buffer, "j       $%08x", jump_address( state, pc, op ) );
		break;
	case OP_JAL:
		sprintf( buffer, "jal     $%08x", jump_address( state, pc, op ) );
		flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA( 1 );
		break;
	case OP_BEQ:
		sprintf( buffer, "beq     %s,%s,$%08x", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ], relative_address( state, pc, op ) );
		break;
	case OP_BNE:
		sprintf( buffer, "bne     %s,%s,$%08x", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ], relative_address( state, pc, op ) );
		break;
	case OP_BLEZ:
		sprintf( buffer, "blez    %s,%s,$%08x", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ], relative_address( state, pc, op ) );
		break;
	case OP_BGTZ:
		sprintf( buffer, "bgtz    %s,%s,$%08x", s_cpugenreg[ INS_RS( op ) ], s_cpugenreg[ INS_RT( op ) ], relative_address( state, pc, op ) );
		break;
	case OP_ADDI:
		sprintf( buffer, "addi    %s,%s,%s", s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ], make_signed_hex_str_16( INS_IMMEDIATE( op ) ) );
		break;
	case OP_ADDIU:
		sprintf( buffer, "addiu   %s,%s,%s", s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ], make_signed_hex_str_16( INS_IMMEDIATE( op ) ) );
		break;
	case OP_SLTI:
		sprintf( buffer, "slti    %s,%s,%s", s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ], make_signed_hex_str_16( INS_IMMEDIATE( op ) ) );
		break;
	case OP_SLTIU:
		sprintf( buffer, "sltiu   %s,%s,%s", s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ], make_signed_hex_str_16( INS_IMMEDIATE( op ) ) );
		break;
	case OP_ANDI:
		sprintf( buffer, "andi    %s,%s,$%04x", s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ], INS_IMMEDIATE( op ) );
		break;
	case OP_ORI:
		sprintf( buffer, "ori     %s,%s,$%04x", s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ], INS_IMMEDIATE( op ) );
		break;
	case OP_XORI:
		sprintf( buffer, "xori    %s,%s,$%04x", s_cpugenreg[ INS_RT( op ) ], s_cpugenreg[ INS_RS( op ) ], INS_IMMEDIATE( op ) );
		break;
	case OP_LUI:
		sprintf( buffer, "lui     %s,%s", s_cpugenreg[ INS_RT( op ) ], upper_address( op, opram ) );
		break;
	case OP_COP0:
		switch( INS_RS( op ) )
		{
		case RS_MFC:
			sprintf( buffer, "mfc0    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp0genreg[ INS_RD( op ) ] );
			break;
		case RS_CFC:
			sprintf( buffer, "!cfc0    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp0ctlreg[ INS_RD( op ) ] );
			break;
		case RS_MTC:
			sprintf( buffer, "mtc0    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp0genreg[ INS_RD( op ) ] );
			break;
		case RS_CTC:
			sprintf( buffer, "!ctc0    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp0ctlreg[ INS_RD( op ) ] );
			break;
		case RS_BC:
		case RS_BC_ALT:
			switch( INS_BC( op ) )
			{
			case BC_BCF:
				sprintf( buffer, "bc0f    $%08x", relative_address( state, pc, op ) );
				break;
			case BC_BCT:
				sprintf( buffer, "bc0t    $%08x", relative_address( state, pc, op ) );
				break;
			}
			break;
		default:
			switch( INS_CO( op ) )
			{
			case 1:
				sprintf( buffer, "cop0    $%07x", INS_COFUN( op ) );

				switch( INS_CF( op ) )
				{
				case CF_TLBR:
					sprintf( buffer, "!tlbr" );
					break;
				case CF_TLBWI:
					sprintf( buffer, "!tlbwi" );
					break;
				case CF_TLBWR:
					sprintf( buffer, "!tlbwr" );
					break;
				case CF_TLBP:
					sprintf( buffer, "!tlbp" );
					break;
				case CF_RFE:
					sprintf( buffer, "rfe" );
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
			sprintf( buffer, "mfc1    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp1genreg[ INS_RD( op ) ] );
			break;
		case RS_CFC:
			sprintf( buffer, "cfc1    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp1ctlreg[ INS_RD( op ) ] );
			break;
		case RS_MTC:
			sprintf( buffer, "mtc1    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp1genreg[ INS_RD( op ) ] );
			break;
		case RS_CTC:
			sprintf( buffer, "ctc1    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp1ctlreg[ INS_RD( op ) ] );
			break;
		case RS_BC:
		case RS_BC_ALT:
			switch( INS_BC( op ) )
			{
			case BC_BCF:
				sprintf( buffer, "bc1f    $%08x", relative_address( state, pc, op ) );
				break;
			case BC_BCT:
				sprintf( buffer, "bc1t    $%08x", relative_address( state, pc, op ) );
				break;
			}
			break;
		default:
			switch( INS_CO( op ) )
			{
			case 1:
				sprintf( buffer, "cop1    $%07x", INS_COFUN( op ) );
				break;
			}
			break;
		}
		break;
	case OP_COP2:
		switch( INS_RS( op ) )
		{
		case RS_MFC:
			sprintf( buffer, "mfc2    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp2genreg[ INS_RD( op ) ] );
			break;
		case RS_CFC:
			sprintf( buffer, "cfc2    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp2ctlreg[ INS_RD( op ) ] );
			break;
		case RS_MTC:
			sprintf( buffer, "mtc2    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp2genreg[ INS_RD( op ) ] );
			break;
		case RS_CTC:
			sprintf( buffer, "ctc2    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp2ctlreg[ INS_RD( op ) ] );
			break;
		case RS_BC:
		case RS_BC_ALT:
			switch( INS_BC( op ) )
			{
			case BC_BCF:
				sprintf( buffer, "bc2f    $%08x", relative_address( state, pc, op ) );
				break;
			case BC_BCT:
				sprintf( buffer, "bc2t    $%08x", relative_address( state, pc, op ) );
				break;
			}
			break;
		default:
			switch( INS_CO( op ) )
			{
			case 1:
				sprintf( buffer, "cop2    $%07x", INS_COFUN( op ) );

				switch( GTE_FUNCT( op ) )
				{
				case 0x00: // drop through to RTPS
				case 0x01:
					sprintf( buffer, "rtps%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x06:
					sprintf( buffer, "nclip" );
					break;
				case 0x0c:
					sprintf( buffer, "op%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x10:
					sprintf( buffer, "dpcs%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x11:
					sprintf( buffer, "intpl%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x12:
					sprintf( buffer, "mvmva%s%s %s + %s * %s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ],
						s_gtecv[ GTE_CV( op ) ], s_gtemx[ GTE_MX( op ) ], s_gtev[ GTE_V( op ) ] );
					break;
				case 0x13:
					sprintf( buffer, "ncds%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x14:
					sprintf( buffer, "cdp%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x16:
					sprintf( buffer, "ncdt%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x1b:
					sprintf( buffer, "nccs%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x1c:
					sprintf( buffer, "cc%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x1e:
					sprintf( buffer, "ncs%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x20:
					sprintf( buffer, "nct%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x28:
					sprintf( buffer, "sqr%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x1a: // end of NCDT
				case 0x29:
					sprintf( buffer, "dpcl%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x2a:
					sprintf( buffer, "dpct%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x2d:
					sprintf( buffer, "avsz3" );
					break;
				case 0x2e:
					sprintf( buffer, "avsz4" );
					break;
				case 0x30:
					sprintf( buffer, "rtpt%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x3d:
					sprintf( buffer, "gpf%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x3e:
					sprintf( buffer, "gpl%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
					break;
				case 0x3f:
					sprintf( buffer, "ncct%s%s", s_gtesf[ GTE_SF( op ) ], s_gtelm[ GTE_LM( op ) ] );
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
			sprintf( buffer, "mfc3    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp3genreg[ INS_RD( op ) ] );
			break;
		case RS_CFC:
			sprintf( buffer, "cfc3    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp3ctlreg[ INS_RD( op ) ] );
			break;
		case RS_MTC:
			sprintf( buffer, "mtc3    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp3genreg[ INS_RD( op ) ] );
			break;
		case RS_CTC:
			sprintf( buffer, "ctc3    %s,%s",  s_cpugenreg[ INS_RT( op ) ], s_cp3ctlreg[ INS_RD( op ) ] );
			break;
		case RS_BC:
		case RS_BC_ALT:
			switch( INS_BC( op ) )
			{
			case BC_BCF:
				sprintf( buffer, "bc3f    $%08x", relative_address( state, pc, op ) );
				break;
			case BC_BCT:
				sprintf( buffer, "bc3t    $%08x", relative_address( state, pc, op ) );
				break;
			}
			break;
		default:
			switch( INS_CO( op ) )
			{
			case 1:
				sprintf( buffer, "cop3    $%07x", INS_COFUN( op ) );
				break;
			}
			break;
		}
		break;
	case OP_LB:
		sprintf( buffer, "lb      %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_LH:
		sprintf( buffer, "lh      %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_LWL:
		sprintf( buffer, "lwl     %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_LW:
		sprintf( buffer, "lw      %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_LBU:
		sprintf( buffer, "lbu     %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_LHU:
		sprintf( buffer, "lhu     %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_LWR:
		sprintf( buffer, "lwr     %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_SB:
		sprintf( buffer, "sb      %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_SH:
		sprintf( buffer, "sh      %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_SWL:
		sprintf( buffer, "swl     %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_SW:
		sprintf( buffer, "sw      %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_SWR:
		sprintf( buffer, "swr     %s,%s", s_cpugenreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_LWC0:
		sprintf( buffer, "lwc0    %s,%s", s_cp0genreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_LWC1:
		sprintf( buffer, "lwc1    %s,%s", s_cp1genreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_LWC2:
		sprintf( buffer, "lwc2    %s,%s", s_cp2genreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_LWC3:
		sprintf( buffer, "lwc3    %s,%s", s_cp2genreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_SWC0:
		sprintf( buffer, "swc0    %s,%s", s_cp0genreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_SWC1:
		sprintf( buffer, "swc1    %s,%s", s_cp1genreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_SWC2:
		sprintf( buffer, "swc2    %s,%s", s_cp2genreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	case OP_SWC3:
		sprintf( buffer, "swc3    %s,%s", s_cp2genreg[ INS_RT( op ) ], effective_address( state, pc, op ) );
		break;
	}
	return ( opram - oldopram ) | flags | DASMFLAG_SUPPORTED;
}

CPU_DISASSEMBLE( psxcpu_generic )
{
	return DasmPSXCPU( nullptr, buffer, pc, opram );
}
