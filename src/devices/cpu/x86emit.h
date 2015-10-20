// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    x86emit.h

    Generic x86/x64 code emitters.

****************************************************************************

    Important note:

    It is assumed in 64-bit mode that all byte register accesses are
    intended to be to the low 8 bits (SPL,BPL,SIL,DIL) and not to the
    upper half (AH,CH,DH,BH).

***************************************************************************/


//**************************************************************************
//  SHARED DEFINITIONS
//**************************************************************************

#ifndef __X86EMIT_H__
#define __X86EMIT_H__

// use x86code * to reference generated code
typedef UINT8       x86code;

// this structure tracks information about a link
struct emit_link
{
	x86code *       target;
	UINT8           size;
};

// structure for describing memory references
class x86_memref
{
public:
	x86_memref(UINT8 basereg, UINT8 indreg, UINT8 scale, INT32 disp)
		: m_base(basereg),
			m_index(indreg),
			m_scale(scale),
			m_disp(disp) { }

	x86_memref operator+(INT32 offset) { return x86_memref(m_base, m_index, m_scale, m_disp + offset); }

	UINT8 m_base;
	UINT8 m_index;
	UINT8 m_scale;
	INT32 m_disp;
};

#endif



//**************************************************************************
//  32-BIT and 64-BIT SPECIFIC DEFINITIONS
//**************************************************************************

#ifdef X86EMIT_SIZE

#if (X86EMIT_SIZE != 32 && X86EMIT_SIZE != 64)
#error Must specify X86EMIT_SIZE as either 32 or 64!
#endif

// put emitters into their own namespace so they don't clash
#if (X86EMIT_SIZE == 32)
namespace x86emit
#else
namespace x64emit
#endif
{
//**************************************************************************
//  CONSTANTS
//**************************************************************************

// opcode size flag; low 4 bits must match REX low 4 bits, hence the odd numbers here
const UINT8 OP_16BIT        = 0x10;
const UINT8 OP_32BIT        = 0x00;
const UINT8 OP_64BIT        = 0x08;

// 16 registers on x64, only 8 on x86
#if (X86EMIT_SIZE == 64)
const int REG_MAX           = 16;
#else
const int REG_MAX           = 8;
#endif

// invalid register index for "none"
const UINT8 REG_NONE        = REG_MAX;

// 8-bit registers -- note that we assume a flat model for 64-bit
const UINT8 REG_AL          = 0;
const UINT8 REG_CL          = 1;
const UINT8 REG_DL          = 2;
const UINT8 REG_BL          = 3;
#if (X86EMIT_SIZE == 32)
const UINT8 REG_AH          = 4;
const UINT8 REG_CH          = 5;
const UINT8 REG_DH          = 6;
const UINT8 REG_BH          = 7;
#else
const UINT8 REG_SPL         = 4;
const UINT8 REG_BPL         = 5;
const UINT8 REG_SIL         = 6;
const UINT8 REG_DIL         = 7;
const UINT8 REG_R8L         = 8;
const UINT8 REG_R9L         = 9;
const UINT8 REG_R10L        = 10;
const UINT8 REG_R11L        = 11;
const UINT8 REG_R12L        = 12;
const UINT8 REG_R13L        = 13;
const UINT8 REG_R14L        = 14;
const UINT8 REG_R15L        = 15;
#endif

// 16-bit registers
const UINT8 REG_AX          = 0;
const UINT8 REG_CX          = 1;
const UINT8 REG_DX          = 2;
const UINT8 REG_BX          = 3;
const UINT8 REG_SP          = 4;
const UINT8 REG_BP          = 5;
const UINT8 REG_SI          = 6;
const UINT8 REG_DI          = 7;
#if (X86EMIT_SIZE == 64)
const UINT8 REG_R8W         = 8;
const UINT8 REG_R9W         = 9;
const UINT8 REG_R10W        = 10;
const UINT8 REG_R11W        = 11;
const UINT8 REG_R12W        = 12;
const UINT8 REG_R13W        = 13;
const UINT8 REG_R14W        = 14;
const UINT8 REG_R15W        = 15;
#endif

// 32-bit registers
const UINT8 REG_EAX         = 0;
const UINT8 REG_ECX         = 1;
const UINT8 REG_EDX         = 2;
const UINT8 REG_EBX         = 3;
const UINT8 REG_ESP         = 4;
const UINT8 REG_EBP         = 5;
const UINT8 REG_ESI         = 6;
const UINT8 REG_EDI         = 7;
#if (X86EMIT_SIZE == 64)
const UINT8 REG_R8D         = 8;
const UINT8 REG_R9D         = 9;
const UINT8 REG_R10D        = 10;
const UINT8 REG_R11D        = 11;
const UINT8 REG_R12D        = 12;
const UINT8 REG_R13D        = 13;
const UINT8 REG_R14D        = 14;
const UINT8 REG_R15D        = 15;
#endif

// 64-bit registers
#if (X86EMIT_SIZE == 64)
const UINT8 REG_RAX         = 0;
const UINT8 REG_RCX         = 1;
const UINT8 REG_RDX         = 2;
const UINT8 REG_RBX         = 3;
const UINT8 REG_RSP         = 4;
const UINT8 REG_RBP         = 5;
const UINT8 REG_RSI         = 6;
const UINT8 REG_RDI         = 7;
const UINT8 REG_R8          = 8;
const UINT8 REG_R9          = 9;
const UINT8 REG_R10         = 10;
const UINT8 REG_R11         = 11;
const UINT8 REG_R12         = 12;
const UINT8 REG_R13         = 13;
const UINT8 REG_R14         = 14;
const UINT8 REG_R15         = 15;
#endif

// 64-bit MMX registers
const UINT8 REG_MM0         = 0;
const UINT8 REG_MM1         = 1;
const UINT8 REG_MM2         = 2;
const UINT8 REG_MM3         = 3;
const UINT8 REG_MM4         = 4;
const UINT8 REG_MM5         = 5;
const UINT8 REG_MM6         = 6;
const UINT8 REG_MM7         = 7;
#if (X86EMIT_SIZE == 64)
const UINT8 REG_MM8         = 8;
const UINT8 REG_MM9         = 9;
const UINT8 REG_MM10        = 10;
const UINT8 REG_MM11        = 11;
const UINT8 REG_MM12        = 12;
const UINT8 REG_MM13        = 13;
const UINT8 REG_MM14        = 14;
const UINT8 REG_MM15        = 15;
#endif

// 128-bit XMM registers
const UINT8 REG_XMM0        = 0;
const UINT8 REG_XMM1        = 1;
const UINT8 REG_XMM2        = 2;
const UINT8 REG_XMM3        = 3;
const UINT8 REG_XMM4        = 4;
const UINT8 REG_XMM5        = 5;
const UINT8 REG_XMM6        = 6;
const UINT8 REG_XMM7        = 7;
#if (X86EMIT_SIZE == 64)
const UINT8 REG_XMM8        = 8;
const UINT8 REG_XMM9        = 9;
const UINT8 REG_XMM10       = 10;
const UINT8 REG_XMM11       = 11;
const UINT8 REG_XMM12       = 12;
const UINT8 REG_XMM13       = 13;
const UINT8 REG_XMM14       = 14;
const UINT8 REG_XMM15       = 15;
#endif

// conditions
const UINT8 COND_A          = 7;
const UINT8 COND_AE         = 3;
const UINT8 COND_B          = 2;
const UINT8 COND_BE         = 6;
const UINT8 COND_C          = 2;
const UINT8 COND_E          = 4;
const UINT8 COND_Z          = 4;
const UINT8 COND_G          = 15;
const UINT8 COND_GE         = 13;
const UINT8 COND_L          = 12;
const UINT8 COND_LE         = 14;
const UINT8 COND_NA         = 6;
const UINT8 COND_NAE        = 2;
const UINT8 COND_NB         = 3;
const UINT8 COND_NBE        = 7;
const UINT8 COND_NC         = 3;
const UINT8 COND_NE         = 5;
const UINT8 COND_NG         = 14;
const UINT8 COND_NGE        = 12;
const UINT8 COND_NL         = 13;
const UINT8 COND_NLE        = 15;
const UINT8 COND_NO         = 1;
const UINT8 COND_NP         = 11;
const UINT8 COND_NS         = 9;
const UINT8 COND_NZ         = 5;
const UINT8 COND_O          = 0;
const UINT8 COND_P          = 10;
const UINT8 COND_PE         = 10;
const UINT8 COND_PO         = 11;
const UINT8 COND_S          = 8;

// floating point rounding modes
const UINT8 FPRND_NEAR      = 0;
const UINT8 FPRND_DOWN      = 1;
const UINT8 FPRND_UP        = 2;
const UINT8 FPRND_CHOP      = 3;



//**************************************************************************
//  OPCODE DEFINITIONS
//**************************************************************************

// opcode flags (in upper 8 bits of opcode)
const UINT32 OPFLAG_8BITREG             = (1 << 24);
const UINT32 OPFLAG_8BITRM              = (1 << 25);

// single byte opcodes
const UINT32 OP_ADD_Eb_Gb               = (0x00 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_ADD_Ev_Gv               = 0x01;
const UINT32 OP_ADD_Gb_Eb               = (0x02 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_ADD_Gv_Ev               = 0x03;
const UINT32 OP_ADD_AL_Ib               = 0x04;
const UINT32 OP_ADD_rAX_Iz              = 0x05;
const UINT32 OP_PUSH_ES                 = 0x06;
const UINT32 OP_POP_ES                  = 0x07;
const UINT32 OP_OR_Eb_Gb                = (0x08 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_OR_Ev_Gv                = 0x09;
const UINT32 OP_OR_Gb_Eb                = (0x0a | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_OR_Gv_Ev                = 0x0b;
const UINT32 OP_OR_AL_Ib                = 0x0c;
const UINT32 OP_OR_eAX_Iv               = 0x0d;
const UINT32 OP_PUSH_CS                 = 0x0e;
const UINT32 OP_EXTENDED                = 0x0f;

const UINT32 OP_ADC_Eb_Gb               = (0x10 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_ADC_Ev_Gv               = 0x11;
const UINT32 OP_ADC_Gb_Eb               = (0x12 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_ADC_Gv_Ev               = 0x13;
const UINT32 OP_ADC_AL_Ib               = 0x14;
const UINT32 OP_ADC_rAX_Iz              = 0x15;
const UINT32 OP_PUSH_SS                 = 0x16;
const UINT32 OP_POP_SS                  = 0x17;
const UINT32 OP_SBB_Eb_Gb               = (0x18 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_SBB_Ev_Gv               = 0x19;
const UINT32 OP_SBB_Gb_Eb               = (0x1a | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_SBB_Gv_Ev               = 0x1b;
const UINT32 OP_SBB_AL_Ib               = 0x1c;
const UINT32 OP_SBB_eAX_Iv              = 0x1d;
const UINT32 OP_PUSH_DS                 = 0x1e;
const UINT32 OP_POP_DS                  = 0x1f;

const UINT32 OP_AND_Eb_Gb               = (0x20 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_AND_Ev_Gv               = 0x21;
const UINT32 OP_AND_Gb_Eb               = (0x22 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_AND_Gv_Ev               = 0x23;
const UINT32 OP_AND_AL_Ib               = 0x24;
const UINT32 OP_AND_rAX_Iz              = 0x25;
const UINT32 PREFIX_ES                  = 0x26;
const UINT32 OP_DAA                     = 0x27;
const UINT32 OP_SUB_Eb_Gb               = (0x28 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_SUB_Ev_Gv               = 0x29;
const UINT32 OP_SUB_Gb_Eb               = (0x2a | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_SUB_Gv_Ev               = 0x2b;
const UINT32 OP_SUB_AL_Ib               = 0x2c;
const UINT32 OP_SUB_eAX_Iv              = 0x2d;
const UINT32 PREFIX_CS                  = 0x2e;
const UINT32 OP_DAS                     = 0x2f;

const UINT32 OP_XOR_Eb_Gb               = (0x30 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_XOR_Ev_Gv               = 0x31;
const UINT32 OP_XOR_Gb_Eb               = (0x32 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_XOR_Gv_Ev               = 0x33;
const UINT32 OP_XOR_AL_Ib               = 0x34;
const UINT32 OP_XOR_rAX_Iz              = 0x35;
const UINT32 PREFIX_SS                  = 0x36;
const UINT32 OP_AAA                     = 0x37;
const UINT32 OP_CMP_Eb_Gb               = (0x38 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_CMP_Ev_Gv               = 0x39;
const UINT32 OP_CMP_Gb_Eb               = (0x3a | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_CMP_Gv_Ev               = 0x3b;
const UINT32 OP_CMP_AL_Ib               = 0x3c;
const UINT32 OP_CMP_eAX_Iv              = 0x3d;
const UINT32 PREFIX_DS                  = 0x3e;
const UINT32 OP_AAS                     = 0x3f;

const UINT32 OP_REX                     = 0x40;
const UINT32 OP_REX_B                   = 0x41;
const UINT32 OP_REX_X                   = 0x42;
const UINT32 OP_REX_XB                  = 0x43;
const UINT32 OP_REX_R                   = 0x44;
const UINT32 OP_REX_RB                  = 0x45;
const UINT32 OP_REX_RX                  = 0x46;
const UINT32 OP_REX_RXB                 = 0x47;
const UINT32 OP_REX_W                   = 0x48;
const UINT32 OP_REX_WB                  = 0x49;
const UINT32 OP_REX_WX                  = 0x4a;
const UINT32 OP_REX_WXB                 = 0x4b;
const UINT32 OP_REX_WR                  = 0x4c;
const UINT32 OP_REX_WRB                 = 0x4d;
const UINT32 OP_REX_WRX                 = 0x4e;
const UINT32 OP_REX_WRXB                = 0x4f;

const UINT32 OP_PUSH_rAX                = 0x50;
const UINT32 OP_PUSH_rCX                = 0x51;
const UINT32 OP_PUSH_rDX                = 0x52;
const UINT32 OP_PUSH_rBX                = 0x53;
const UINT32 OP_PUSH_rSP                = 0x54;
const UINT32 OP_PUSH_rBP                = 0x55;
const UINT32 OP_PUSH_rSI                = 0x56;
const UINT32 OP_PUSH_rDI                = 0x57;
const UINT32 OP_POP_rAX                 = 0x58;
const UINT32 OP_POP_rCX                 = 0x59;
const UINT32 OP_POP_rDX                 = 0x5a;
const UINT32 OP_POP_rBX                 = 0x5b;
const UINT32 OP_POP_rSP                 = 0x5c;
const UINT32 OP_POP_rBP                 = 0x5d;
const UINT32 OP_POP_rSI                 = 0x5e;
const UINT32 OP_POP_rDI                 = 0x5f;

const UINT32 OP_PUSHA                   = 0x60;
const UINT32 OP_POPA                    = 0x61;
const UINT32 OP_BOUND_Gv_Ma             = 0x62;
const UINT32 OP_ARPL_Ew_Gw              = 0x63;
const UINT32 OP_MOVSXD_Gv_Ev            = 0x63;
const UINT32 PREFIX_FS                  = 0x64;
const UINT32 PREFIX_GS                  = 0x65;
const UINT32 PREFIX_OPSIZE              = 0x66;
const UINT32 PREFIX_ADSIZE              = 0x67;
const UINT32 OP_PUSH_Iz                 = 0x68;
const UINT32 OP_IMUL_Gv_Ev_Iz           = 0x69;
const UINT32 OP_PUSH_Ib                 = 0x6a;
const UINT32 OP_IMUL_Gv_Ev_Ib           = 0x6b;
const UINT32 OP_INS_Yb_DX               = 0x6c;
const UINT32 OP_INS_Yz_DX               = 0x6d;
const UINT32 OP_OUTS_DX_Xb              = 0x6e;
const UINT32 OP_OUTS_DX_Xz              = 0x6f;

const UINT32 OP_JCC_O_Jb                = 0x70;
const UINT32 OP_JCC_NO_Jb               = 0x71;
const UINT32 OP_JCC_B_Jb                = 0x72;
const UINT32 OP_JCC_C_Jb                = 0x72;
const UINT32 OP_JCC_NAE_Jb              = 0x72;
const UINT32 OP_JCC_AE_Jb               = 0x73;
const UINT32 OP_JCC_NB_Jb               = 0x73;
const UINT32 OP_JCC_NC_Jb               = 0x73;
const UINT32 OP_JCC_E_Jb                = 0x74;
const UINT32 OP_JCC_Z_Jb                = 0x74;
const UINT32 OP_JCC_NE_Jb               = 0x75;
const UINT32 OP_JCC_NZ_Jb               = 0x75;
const UINT32 OP_JCC_BE_Jb               = 0x76;
const UINT32 OP_JCC_NA_Jb               = 0x76;
const UINT32 OP_JCC_A_Jb                = 0x77;
const UINT32 OP_JCC_NBE_Jb              = 0x77;
const UINT32 OP_JCC_S_Jb                = 0x78;
const UINT32 OP_JCC_NS_Jb               = 0x79;
const UINT32 OP_JCC_P_Jb                = 0x7a;
const UINT32 OP_JCC_PE_Jb               = 0x7a;
const UINT32 OP_JCC_NP_Jb               = 0x7b;
const UINT32 OP_JCC_PO_Jb               = 0x7b;
const UINT32 OP_JCC_L_Jb                = 0x7c;
const UINT32 OP_JCC_NGE_Jb              = 0x7c;
const UINT32 OP_JCC_NL_Jb               = 0x7d;
const UINT32 OP_JCC_GE_Jb               = 0x7d;
const UINT32 OP_JCC_LE_Jb               = 0x7e;
const UINT32 OP_JCC_NG_Jb               = 0x7e;
const UINT32 OP_JCC_NLE_Jb              = 0x7f;
const UINT32 OP_JCC_G_Jb                = 0x7f;

const UINT32 OP_G1_Eb_Ib                = (0x80 | OPFLAG_8BITRM);
const UINT32 OP_G1_Ev_Iz                = 0x81;
const UINT32 OP_G1_Eb_Ibx               = (0x82 | OPFLAG_8BITRM);
const UINT32 OP_G1_Ev_Ib                = 0x83;
const UINT32 OP_TEST_Eb_Gb              = (0x84 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_TEST_Ev_Gv              = 0x85;
const UINT32 OP_XCHG_Eb_Gb              = (0x86 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_XCHG_Ev_Gv              = 0x87;
const UINT32 OP_MOV_Eb_Gb               = (0x88 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_MOV_Ev_Gv               = 0x89;
const UINT32 OP_MOV_Gb_Eb               = (0x8a | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_MOV_Gv_Ev               = 0x8b;
const UINT32 OP_MOV_Ev_Sw               = 0x8c;
const UINT32 OP_LEA_Gv_M                = 0x8d;
const UINT32 OP_MOV_Sw_Ew               = 0x8e;
const UINT32 OP_G1A_Ev                  = 0x8f;

const UINT32 OP_NOP                     = 0x90;
const UINT32 OP_PAUSE                   = 0x90;
const UINT32 OP_XCHG_rCX                = 0x91;
const UINT32 OP_XCHG_rDX                = 0x92;
const UINT32 OP_XCHG_rBX                = 0x93;
const UINT32 OP_XCHG_rSP                = 0x94;
const UINT32 OP_XCHG_rBP                = 0x95;
const UINT32 OP_XCHG_rSI                = 0x96;
const UINT32 OP_XCHG_rDI                = 0x97;
const UINT32 OP_CBW                     = 0x98;
const UINT32 OP_CWDE                    = 0x98;
const UINT32 OP_CDQE                    = 0x98;
const UINT32 OP_CWD                     = 0x99;
const UINT32 OP_CDQ                     = 0x99;
const UINT32 OP_CQO                     = 0x99;
const UINT32 OP_CALLF_Ap                = 0x9a;
const UINT32 OP_FWAIT                   = 0x9b;
const UINT32 OP_PUSHF_Fv                = 0x9c;
const UINT32 OP_POPF_Fv                 = 0x9d;
const UINT32 OP_SAHF                    = 0x9e;
const UINT32 OP_LAHF                    = 0x9f;

const UINT32 OP_MOV_AL_Ob               = 0xa0;
const UINT32 OP_MOV_rAX_Ov              = 0xa1;
const UINT32 OP_MOV_Ob_AL               = 0xa2;
const UINT32 OP_MOV_Ov_rAX              = 0xa3;
const UINT32 OP_MOVS_Xb_Yb              = 0xa4;
const UINT32 OP_MOVS_Xv_Yv              = 0xa5;
const UINT32 OP_CMPS_Xb_Yb              = 0xa6;
const UINT32 OP_CMPS_Xv_Yv              = 0xa7;
const UINT32 OP_TEST_AL_Ib              = 0xa8;
const UINT32 OP_TEST_rAX_Iz             = 0xa9;
const UINT32 OP_STOS_Yb_AL              = 0xaa;
const UINT32 OP_STOS_Yv_rAX             = 0xab;
const UINT32 OP_LODS_AL_Xb              = 0xac;
const UINT32 OP_LODS_rAX_Xv             = 0xad;
const UINT32 OP_SCAS_AL_Yb              = 0xae;
const UINT32 OP_SCAC_rAX_Yv             = 0xaf;

const UINT32 OP_MOV_AL_Ib               = 0xb0;
const UINT32 OP_MOV_CL_Ib               = 0xb1;
const UINT32 OP_MOV_DL_Ib               = 0xb2;
const UINT32 OP_MOV_BL_Ib               = 0xb3;
const UINT32 OP_MOV_AH_Ib               = 0xb4;
const UINT32 OP_MOV_CH_Ib               = 0xb5;
const UINT32 OP_MOV_DH_Ib               = 0xb6;
const UINT32 OP_MOV_BH_Ib               = 0xb7;
const UINT32 OP_MOV_rAX_Iv              = 0xb8;
const UINT32 OP_MOV_rCX_Iv              = 0xb9;
const UINT32 OP_MOV_rDX_Iv              = 0xba;
const UINT32 OP_MOV_rBX_Iv              = 0xbb;
const UINT32 OP_MOV_rSP_Iv              = 0xbc;
const UINT32 OP_MOV_rBP_Iv              = 0xbd;
const UINT32 OP_MOV_rSI_Iv              = 0xbe;
const UINT32 OP_MOV_rDI_Iv              = 0xbf;

const UINT32 OP_G2_Eb_Ib                = (0xc0 | OPFLAG_8BITRM);
const UINT32 OP_G2_Ev_Ib                = 0xc1;
const UINT32 OP_RETN_Iw                 = 0xc2;
const UINT32 OP_RETN                    = 0xc3;
const UINT32 OP_LES_Gz_Mp               = 0xc4;
const UINT32 OP_LDS_Gz_Mp               = 0xc5;
const UINT32 OP_G11_Eb_Ib               = (0xc6 | OPFLAG_8BITRM);
const UINT32 OP_G11_Ev_Iz               = 0xc7;
const UINT32 OP_ENTER_Iw_Ib             = 0xc8;
const UINT32 OP_LEAVE                   = 0xc9;
const UINT32 OP_RETF_Iw                 = 0xca;
const UINT32 OP_RETF                    = 0xcb;
const UINT32 OP_INT_3                   = 0xcc;
const UINT32 OP_INT_Ib                  = 0xcd;
const UINT32 OP_INTO                    = 0xce;
const UINT32 OP_IRET                    = 0xcf;

const UINT32 OP_G2_Eb_1                 = (0xd0 | OPFLAG_8BITRM);
const UINT32 OP_G2_Ev_1                 = 0xd1;
const UINT32 OP_G2_Eb_CL                = (0xd2 | OPFLAG_8BITRM);
const UINT32 OP_G2_Ev_CL                = 0xd3;
const UINT32 OP_AAM                     = 0xd4;
const UINT32 OP_AAD                     = 0xd5;
const UINT32 OP_XLAT                    = 0xd7;
const UINT32 OP_ESC_D8                  = 0xd8;
const UINT32 OP_ESC_D9                  = 0xd9;
const UINT32 OP_ESC_DA                  = 0xda;
const UINT32 OP_ESC_DB                  = 0xdb;
const UINT32 OP_ESC_DC                  = 0xdc;
const UINT32 OP_ESC_DD                  = 0xdd;
const UINT32 OP_ESC_DE                  = 0xde;
const UINT32 OP_ESC_DF                  = 0xdf;

const UINT32 OP_LOOPNE_Jb               = 0xe0;
const UINT32 OP_LOOPE_Jb                = 0xe1;
const UINT32 OP_LOOP_Jb                 = 0xe2;
const UINT32 OP_JrCXZ_Jb                = 0xe3;
const UINT32 OP_IN_AL_Ib                = 0xe4;
const UINT32 OP_IN_eAX_Ib               = 0xe5;
const UINT32 OP_OUT_Ib_AL               = 0xe6;
const UINT32 OP_OUT_Ib_eAX              = 0xe7;
const UINT32 OP_CALL_Jz                 = 0xe8;
const UINT32 OP_JMP_Jz                  = 0xe9;
const UINT32 OP_JMPF_AP                 = 0xea;
const UINT32 OP_JMP_Jb                  = 0xeb;
const UINT32 OP_IN_AL_DX                = 0xec;
const UINT32 OP_IN_eAX_D                = 0xed;
const UINT32 OP_OUT_DX_AL               = 0xee;
const UINT32 OP_OUT_DX_eAX              = 0xef;

const UINT32 PREFIX_LOCK                = 0xf0;
const UINT32 PREFIX_REPNE               = 0xf2;
const UINT32 PREFIX_REPE                = 0xf3;
const UINT32 OP_HLT                     = 0xf4;
const UINT32 OP_CMC                     = 0xf5;
const UINT32 OP_G3_Eb                   = (0xf6 | OPFLAG_8BITRM);
const UINT32 OP_G3_Ev                   = 0xf7;
const UINT32 OP_CLC                     = 0xf8;
const UINT32 OP_STC                     = 0xf9;
const UINT32 OP_CLI                     = 0xfa;
const UINT32 OP_STI                     = 0xfb;
const UINT32 OP_CLD                     = 0xfc;
const UINT32 OP_STD                     = 0xfd;
const UINT32 OP_G4                      = 0xfe;
const UINT32 OP_G5                      = 0xff;


// double byte opcodes
const UINT32 OP_G6                      = 0x0f00;
const UINT32 OP_G7                      = 0x0f01;
const UINT32 OP_LAR_Gv_Ew               = 0x0f02;
const UINT32 OP_LSL_Gv_Ew               = 0x0f03;
const UINT32 OP_SYSCALL                 = 0x0f05;
const UINT32 OP_CLTS                    = 0x0f06;
const UINT32 OP_SYSRET                  = 0x0f07;
const UINT32 OP_INVD                    = 0x0f08;
const UINT32 OP_WBINVD                  = 0x0f09;
const UINT32 OP_UD2                     = 0x0f0b;
const UINT32 OP_NOP0d_Ev                = 0x0f0d;

const UINT32 OP_MOVUPS_Vps_Wps          = 0x0f10;
const UINT32 OP_MOVSS_Vss_Wss           = 0xf30f10;
const UINT32 OP_MOVUPD_Vpd_Wpd          = 0x660f10;
const UINT32 OP_MOVSD_Vsd_Wsd           = 0xf20f10;
const UINT32 OP_MOVUPS_Wps_Vps          = 0x0f11;
const UINT32 OP_MOVSS_Wss_Vss           = 0xf30f11;
const UINT32 OP_MOVUPD_Wpd_Vpd          = 0x660f11;
const UINT32 OP_MOVSD_Wsd_Vsd           = 0xf20f11;
const UINT32 OP_MOVLPS_Vq_Mq            = 0x0f12;
const UINT32 OP_MOVLPD_Vq_Mq            = 0x660f12;
const UINT32 OP_MOVHLPS_Vq_Uq           = 0x0f12;
const UINT32 OP_MOVDDUP_Vq_Wq           = 0xf20f12;
const UINT32 OP_MOVSLDUP_Vq_Wq          = 0xf30f12;
const UINT32 OP_MOVLPS_Mq_Vq            = 0x0f13;
const UINT32 OP_MOVLPD_Mq_Vq            = 0x660f13;
const UINT32 OP_UNPCKLPS_Vps_Wq         = 0x0f14;
const UINT32 OP_UNPCKLPD_Vpd_Wq         = 0x660f14;
const UINT32 OP_UNPCKHPS_Vps_Wq         = 0x0f15;
const UINT32 OP_UNPCKHPD_Vpd_Wq         = 0x660f15;
const UINT32 OP_MOVHPS_Vq_Mq            = 0x0f16;
const UINT32 OP_MOVHPD_Vq_Mq            = 0x660f16;
const UINT32 OP_MOVLHPS_Vq_Uq           = 0x0f16;
const UINT32 OP_MOVSHDUP_Vq_Wq          = 0xf30f16;
const UINT32 OP_MOVHPS_Mq_Vq            = 0x0f17;
const UINT32 OP_MOVHPD_Mq_Vq            = 0x660f17;
const UINT32 OP_PREFETCH_G16            = 0x0f18;
const UINT32 OP_NOP1f_Ev                = 0x0f1f;

const UINT32 OP_MOV_Rd_Cd               = 0x0f20;
const UINT32 OP_MOV_Rd_Dd               = 0x0f21;
const UINT32 OP_MOV_Cd_Rd               = 0x0f22;
const UINT32 OP_MOV_Dd_Rd               = 0x0f23;
const UINT32 OP_MOVAPS_Vps_Wps          = 0x0f28;
const UINT32 OP_MOVAPD_Vpd_Wpd          = 0x660f28;
const UINT32 OP_MOVAPS_Wps_Vps          = 0x0f29;
const UINT32 OP_MOVAPD_Wpd_Vpd          = 0x660f29;
const UINT32 OP_CVTPI2PS_Vps_Qq         = 0x0f2a;
const UINT32 OP_CVTSI2SS_Vss_Ed         = 0xf30f2a;
const UINT32 OP_CVTPI2PD_Vpd_Qq         = 0x660f2a;
const UINT32 OP_CVTSI2SD_Vsd_Ed         = 0xf20f2a;
const UINT32 OP_MOVNTPS_Mps_Vps         = 0x0f2b;
const UINT32 OP_MOVNTPD_Mpd_Vpd         = 0x660f2b;
const UINT32 OP_CVTTPS2PI_Pq_Wq         = 0x0f2c;
const UINT32 OP_CVTTSS2SI_Gd_Wss        = 0xf30f2c;
const UINT32 OP_CVTTPD2PI_Pq_Wpd        = 0x660f2c;
const UINT32 OP_CVTTSD2SI_Gd_Wsd        = 0xf20f2c;
const UINT32 OP_CVTPS2PI_Pq_Wq          = 0x0f2d;
const UINT32 OP_CVTSS2SI_Gd_Wss         = 0xf30f2d;
const UINT32 OP_CVTPD2PI_Pq_Wpd         = 0x660f2d;
const UINT32 OP_CVTSD2SI_Gd_Wsd         = 0xf20f2d;
const UINT32 OP_UCOMISS_Vss_Wss         = 0x0f2e;
const UINT32 OP_UCOMISD_Vsd_Wsd         = 0x660f2e;
const UINT32 OP_COMISS_Vss_Wss          = 0x0f2f;
const UINT32 OP_COMISD_Vsd_Wsd          = 0x660f2f;

const UINT32 OP_WRMSR                   = 0x0f30;
const UINT32 OP_RDTSC                   = 0x0f31;
const UINT32 OP_RDMSR                   = 0x0f32;
const UINT32 OP_RDPMC                   = 0x0f33;
const UINT32 OP_SYSENTER                = 0x0f34;
const UINT32 OP_SYSEXIT                 = 0x0f35;
const UINT32 OP_GETSEC                  = 0x0f37;

const UINT32 OP_CMOV_O_Gv_Ev            = 0x0f40;
const UINT32 OP_CMOV_NO_Gv_Ev           = 0x0f41;
const UINT32 OP_CMOV_B_Gv_Ev            = 0x0f42;
const UINT32 OP_CMOV_C_Gv_Ev            = 0x0f42;
const UINT32 OP_CMOV_AE_Gv_Ev           = 0x0f43;
const UINT32 OP_CMOV_NC_Gv_Ev           = 0x0f43;
const UINT32 OP_CMOV_E_Gv_Ev            = 0x0f44;
const UINT32 OP_CMOV_Z_Gv_Ev            = 0x0f44;
const UINT32 OP_CMOV_NE_Gv_Ev           = 0x0f45;
const UINT32 OP_CMOV_NZ_Gv_Ev           = 0x0f45;
const UINT32 OP_CMOV_BE_Gv_Ev           = 0x0f46;
const UINT32 OP_CMOV_A_Gv_Ev            = 0x0f47;
const UINT32 OP_CMOV_S_Gv_Ev            = 0x0f48;
const UINT32 OP_CMOV_NS_Gv_Ev           = 0x0f49;
const UINT32 OP_CMOV_P_Gv_Ev            = 0x0f4a;
const UINT32 OP_CMOV_PE_Gv_Ev           = 0x0f4a;
const UINT32 OP_CMOV_NP_Gv_Ev           = 0x0f4b;
const UINT32 OP_CMOV_PO_Gv_Ev           = 0x0f4b;
const UINT32 OP_CMOV_L_Gv_Ev            = 0x0f4c;
const UINT32 OP_CMOV_NGE_Gv_Ev          = 0x0f4c;
const UINT32 OP_CMOV_NL_Gv_Ev           = 0x0f4d;
const UINT32 OP_CMOV_GE_Gv_Ev           = 0x0f4d;
const UINT32 OP_CMOV_LE_Gv_Ev           = 0x0f4e;
const UINT32 OP_CMOV_NG_Gv_Ev           = 0x0f4e;
const UINT32 OP_CMOV_NLE_Gv_Ev          = 0x0f4f;
const UINT32 OP_CMOV_G_Gv_Ev            = 0x0f4f;

const UINT32 OP_MOVMSKPS_Gd_Ups         = 0x0f50;
const UINT32 OP_MOVMSKPD_Gd_Upd         = 0x660f50;
const UINT32 OP_SQRTPS_Vps_Wps          = 0x0f51;
const UINT32 OP_SQRTSS_Vss_Wss          = 0xf30f51;
const UINT32 OP_SQRTPD_Vpd_Wpd          = 0x660f51;
const UINT32 OP_SQRTSD_Vsd_Wsd          = 0xf20f51;
const UINT32 OP_RSQRTPS_Vps_Wps         = 0x0f52;
const UINT32 OP_RSQRTSS_Vss_Wss         = 0xf30f52;
const UINT32 OP_RCPPS_Vps_Wps           = 0x0f53;
const UINT32 OP_RCPSS_Vss_Wss           = 0xf30f53;
const UINT32 OP_ANDPS_Vps_Wps           = 0x0f54;
const UINT32 OP_ANDPD_Vpd_Wpd           = 0x660f54;
const UINT32 OP_ANDNPS_Vps_Wps          = 0x0f55;
const UINT32 OP_ANDNPD_Vpd_Wpd          = 0x660f55;
const UINT32 OP_ORPS_Vps_Wps            = 0x0f56;
const UINT32 OP_ORPD_Vpd_Wpd            = 0x660f56;
const UINT32 OP_XORPS_Vps_Wps           = 0x0f57;
const UINT32 OP_XORPD_Vpd_Wpd           = 0x660f57;
const UINT32 OP_ADDPS_Vps_Wps           = 0x0f58;
const UINT32 OP_ADDSS_Vss_Wss           = 0xf30f58;
const UINT32 OP_ADDPD_Vpd_Wpd           = 0x660f58;
const UINT32 OP_ADDSD_Vsd_Wsd           = 0xf20f58;
const UINT32 OP_MULPS_Vps_Wps           = 0x0f59;
const UINT32 OP_MULSS_Vss_Wss           = 0xf30f59;
const UINT32 OP_MULPD_Vpd_Wpd           = 0x660f59;
const UINT32 OP_MULSD_Vsd_Wsd           = 0xf20f59;
const UINT32 OP_CVTPS2PD_Vpd_Wq         = 0x0f5a;
const UINT32 OP_CVTSS2SD_Vsd_Wss        = 0xf30f5a;
const UINT32 OP_CVTPD2PS_Vps_Wpd        = 0x660f5a;
const UINT32 OP_CVTSD2SS_Vss_Wsd        = 0xf20f5a;
const UINT32 OP_CVTDQ2PS_Vps_Wdq        = 0x0f5b;
const UINT32 OP_CVTPS2DQ_Vdq_Wps        = 0x660f5b;
const UINT32 OP_CVTTPS2DQ_Vdq_Wps       = 0xf30f5b;
const UINT32 OP_SUBPS_Vps_Wps           = 0x0f5c;
const UINT32 OP_SUBSS_Vss_Wss           = 0xf30f5c;
const UINT32 OP_SUBPD_Vpd_Wpd           = 0x660f5c;
const UINT32 OP_SUBSD_Vsd_Wsd           = 0xf20f5c;
const UINT32 OP_MINPS_Vps_Wps           = 0x0f5d;
const UINT32 OP_MINSS_Vss_Wss           = 0xf30f5d;
const UINT32 OP_MINPD_Vpd_Wpd           = 0x660f5d;
const UINT32 OP_MINSD_Vsd_Wsd           = 0xf20f5d;
const UINT32 OP_DIVPS_Vps_Wps           = 0x0f5e;
const UINT32 OP_DIVSS_Vss_Wss           = 0xf30f5e;
const UINT32 OP_DIVPD_Vpd_Wpd           = 0x660f5e;
const UINT32 OP_DIVSD_Vsd_Wsd           = 0xf20f5e;
const UINT32 OP_MAXPS_Vps_Wps           = 0x0f5f;
const UINT32 OP_MAXSS_Vss_Wss           = 0xf30f5f;
const UINT32 OP_MAXPD_Vpd_Wpd           = 0x660f5f;
const UINT32 OP_MAXSD_Vsd_Wsd           = 0xf20f5f;

const UINT32 OP_PUNPCKLBW_Pq_Qd         = 0x0f60;
const UINT32 OP_PUNPCKLBW_Vdq_Wdq       = 0x660f60;
const UINT32 OP_PUNPCKLWD_Pq_Qd         = 0x0f61;
const UINT32 OP_PUNPCKLWD_Vdq_Wdq       = 0x660f61;
const UINT32 OP_PUNPCKLDQ_Pq_Qd         = 0x0f62;
const UINT32 OP_PUNPCKLDQ_Vdq_Wdq       = 0x660f62;
const UINT32 OP_PACKSSWB_Pq_Qq          = 0x0f63;
const UINT32 OP_PACKSSWB_Vdq_Wdq        = 0x660f63;
const UINT32 OP_PCMPGTB_Pq_Qq           = 0x0f64;
const UINT32 OP_PCMPGTB_Vdq_Wdq         = 0x660f64;
const UINT32 OP_PCMPGTW_Pq_Qq           = 0x0f65;
const UINT32 OP_PCMPGTW_Vdq_Wdq         = 0x660f65;
const UINT32 OP_PCMPGTD_Pq_Qq           = 0x0f66;
const UINT32 OP_PCMPGTD_Vdq_Wdq         = 0x660f66;
const UINT32 OP_PACKUSWB_Pq_Qq          = 0x0f67;
const UINT32 OP_PACKUSWB_Vdq_Wdq        = 0x660f67;
const UINT32 OP_PUNPCKHBW_Pq_Qq         = 0x0f68;
const UINT32 OP_PUNPCKHBW_Vdq_Qdq       = 0x660f68;
const UINT32 OP_PUNPCKHWD_Pq_Qq         = 0x0f69;
const UINT32 OP_PUNPCKHWD_Vdq_Qdq       = 0x660f69;
const UINT32 OP_PUNPCKHDQ_Pq_Qq         = 0x0f6a;
const UINT32 OP_PUNPCKHDQ_Vdq_Qdq       = 0x660f6a;
const UINT32 OP_PACKSSDW_Pq_Qq          = 0x0f6b;
const UINT32 OP_PACKSSDW_Vdq_Qdq        = 0x660f6b;
const UINT32 OP_PUNPCKLQDQ_Vdq_Wdq      = 0x660f6c;
const UINT32 OP_PUNPCKHQDQ_Vdq_Wdq      = 0x660f6d;
const UINT32 OP_MOVD_Pd_Ed              = 0x0f6e;
const UINT32 OP_MOVD_Vd_Ed              = 0x660f6e;
const UINT32 OP_MOVQ_Pq_Qq              = 0x0f6f;
const UINT32 OP_MOVDQA_Vdq_Wdq          = 0x660f6f;
const UINT32 OP_MOVDQU_Vdq_Wdq          = 0xf30f6f;

const UINT32 OP_PSHUFW_Pq_Qq_Ib         = 0x0f70;
const UINT32 OP_PSHUFD_Vdq_Wdq_Ib       = 0x660f70;
const UINT32 OP_PSHUFHW_Vdq_Wdq_Ib      = 0xf30f70;
const UINT32 OP_PSHUFLW_Vdq_Wdq_Ib      = 0xf20f70;
const UINT32 OP_G12                     = 0x0f71;
const UINT32 OP_G13                     = 0x0f72;
const UINT32 OP_G14                     = 0x0f73;
const UINT32 OP_PCMPEQB_Pq_Qq           = 0x0f74;
const UINT32 OP_PCMPEQB_Vdq_Wdq         = 0x660f74;
const UINT32 OP_PCMPEQW_Pq_Qq           = 0x0f75;
const UINT32 OP_PCMPEQW_Vdq_Wdq         = 0x660f75;
const UINT32 OP_PCMPEQD_Pq_Qq           = 0x0f76;
const UINT32 OP_PCMPEQD_Vdq_Wdq         = 0x660f76;
const UINT32 OP_EMMS                    = 0x0f77;
const UINT32 OP_VMREAD_Ed_Gd            = 0x0f78;
const UINT32 OP_VMWRITE_Gd_Ed           = 0x0f79;
const UINT32 OP_HADDPD_Vpd_Wpd          = 0x660f7c;
const UINT32 OP_HADDPS_Vps_Wps          = 0xf20f7c;
const UINT32 OP_HSUBPD_Vpd_Wpd          = 0x660f7d;
const UINT32 OP_HSUBPS_Vps_Wps          = 0xf20f7d;
const UINT32 OP_MOVD_Ed_Pd              = 0x0f7e;
const UINT32 OP_MOVD_Ed_Vd              = 0x660f7e;
const UINT32 OP_MOVQ_Vq_Wq              = 0xf30f7e;
const UINT32 OP_MOVQ_Qq_Pq              = 0x0f7f;
const UINT32 OP_MOVDQA_Wdq_Vdq          = 0x660f7f;
const UINT32 OP_MOVDQU_Wdq_Vdq          = 0xf30f7f;

const UINT32 OP_JCC_O_Jv                = 0x0f80;
const UINT32 OP_JCC_NO_Jv               = 0x0f81;
const UINT32 OP_JCC_B_Jv                = 0x0f82;
const UINT32 OP_JCC_C_Jv                = 0x0f82;
const UINT32 OP_JCC_NAE_Jv              = 0x0f82;
const UINT32 OP_JCC_AE_Jv               = 0x0f83;
const UINT32 OP_JCC_NB_Jv               = 0x0f83;
const UINT32 OP_JCC_NC_Jv               = 0x0f83;
const UINT32 OP_JCC_E_Jv                = 0x0f84;
const UINT32 OP_JCC_Z_Jv                = 0x0f84;
const UINT32 OP_JCC_NE_Jv               = 0x0f85;
const UINT32 OP_JCC_NZ_Jv               = 0x0f85;
const UINT32 OP_JCC_BE_Jv               = 0x0f86;
const UINT32 OP_JCC_NA_Jv               = 0x0f86;
const UINT32 OP_JCC_A_Jv                = 0x0f87;
const UINT32 OP_JCC_NBE_Jv              = 0x0f87;
const UINT32 OP_JCC_S_Jv                = 0x0f88;
const UINT32 OP_JCC_NS_Jv               = 0x0f89;
const UINT32 OP_JCC_P_Jv                = 0x0f8a;
const UINT32 OP_JCC_PE_Jv               = 0x0f8a;
const UINT32 OP_JCC_NP_Jv               = 0x0f8b;
const UINT32 OP_JCC_PO_Jv               = 0x0f8b;
const UINT32 OP_JCC_L_Jv                = 0x0f8c;
const UINT32 OP_JCC_NGE_Jv              = 0x0f8c;
const UINT32 OP_JCC_NL_Jv               = 0x0f8d;
const UINT32 OP_JCC_GE_Jv               = 0x0f8d;
const UINT32 OP_JCC_LE_Jv               = 0x0f8e;
const UINT32 OP_JCC_NG_Jv               = 0x0f8e;
const UINT32 OP_JCC_NLE_Jv              = 0x0f8f;
const UINT32 OP_JCC_G_Jv                = 0x0f8f;

const UINT32 OP_SETCC_O_Eb              = (0x0f90 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_NO_Eb             = (0x0f91 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_B_Eb              = (0x0f92 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_C_Eb              = (0x0f92 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_NAE_Eb            = (0x0f92 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_AE_Eb             = (0x0f93 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_NB_Eb             = (0x0f93 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_NC_Eb             = (0x0f93 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_E_Eb              = (0x0f94 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_Z_Eb              = (0x0f94 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_NE_Eb             = (0x0f95 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_NZ_Eb             = (0x0f95 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_BE_Eb             = (0x0f96 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_NA_Eb             = (0x0f96 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_A_Eb              = (0x0f97 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_NBE_Eb            = (0x0f97 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_S_Eb              = (0x0f98 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_NS_Eb             = (0x0f99 | OPFLAG_8BITRM);
const UINT32 OP_SETCC_P_Eb              = (0x0f9a | OPFLAG_8BITRM);
const UINT32 OP_SETCC_PE_Eb             = (0x0f9a | OPFLAG_8BITRM);
const UINT32 OP_SETCC_NP_Eb             = (0x0f9b | OPFLAG_8BITRM);
const UINT32 OP_SETCC_PO_Eb             = (0x0f9b | OPFLAG_8BITRM);
const UINT32 OP_SETCC_L_Eb              = (0x0f9c | OPFLAG_8BITRM);
const UINT32 OP_SETCC_NGE_Eb            = (0x0f9c | OPFLAG_8BITRM);
const UINT32 OP_SETCC_NL_Eb             = (0x0f9d | OPFLAG_8BITRM);
const UINT32 OP_SETCC_GE_Eb             = (0x0f9d | OPFLAG_8BITRM);
const UINT32 OP_SETCC_LE_Eb             = (0x0f9e | OPFLAG_8BITRM);
const UINT32 OP_SETCC_NG_Eb             = (0x0f9e | OPFLAG_8BITRM);
const UINT32 OP_SETCC_NLE_Eb            = (0x0f9f | OPFLAG_8BITRM);
const UINT32 OP_SETCC_G_Eb              = (0x0f9f | OPFLAG_8BITRM);

const UINT32 OP_PUSH_FS                 = 0x0fa0;
const UINT32 OP_POP_FS                  = 0x0fa1;
const UINT32 OP_CPUID                   = 0x0fa2;
const UINT32 OP_BT_Ev_Gv                = 0x0fa3;
const UINT32 OP_SHLD_Ev_Gv_Ib           = 0x0fa4;
const UINT32 OP_SHLD_Ev_Gv_CL           = 0x0fa5;
const UINT32 OP_PUSH_GS                 = 0x0fa8;
const UINT32 OP_POP_GS                  = 0x0fa9;
const UINT32 OP_RSM                     = 0x0faa;
const UINT32 OP_BTS_Ev_Gv               = 0x0fab;
const UINT32 OP_SHRD_Ev_Gv_Ib           = 0x0fac;
const UINT32 OP_SHRD_Ev_Gv_CL           = 0x0fad;
const UINT32 OP_G16                     = 0x0fae;
const UINT32 OP_IMUL_Gv_Ev              = 0x0faf;

const UINT32 OP_CMPXCHG_Eb_Gb           = (0x0fb0 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_CMPXCHG_Ev_Gv           = 0x0fb1;
const UINT32 OP_LSS_Mp                  = 0x0fb2;
const UINT32 OP_BTR_Ev_Gv               = 0x0fb3;
const UINT32 OP_LFS_Mp                  = 0x0fb4;
const UINT32 OP_LGS_Mp                  = 0x0fb5;
const UINT32 OP_MOVZX_Gv_Eb             = (0x0fb6 | OPFLAG_8BITRM);
const UINT32 OP_MOVZX_Gv_Ew             = 0x0fb7;
const UINT32 OP_JMPE                    = 0x0fb8;
const UINT32 OP_POPCNT_Gv_Ev            = 0xf30fb8;
const UINT32 OP_G10_INVALID             = 0x0fb9;
const UINT32 OP_G8_Ev_Ib                = 0x0fba;
const UINT32 OP_BTC_Ev_Gv               = 0x0fbb;
const UINT32 OP_BSF_Gv_Ev               = 0x0fbc;
const UINT32 OP_BSR_Gv_Ev               = 0x0fbd;
const UINT32 OP_MOVSX_Gv_Eb             = (0x0fbe | OPFLAG_8BITRM);
const UINT32 OP_MOVSX_Gv_Ew             = 0x0fbf;

const UINT32 OP_XADD_Eb_Gb              = (0x0fc0 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const UINT32 OP_XADD_Ev_Gv              = 0x0fc1;
const UINT32 OP_CMPPS_Vps_Wps_Ib        = 0x0fc2;
const UINT32 OP_CMPSS_Vss_Wss_Ib        = 0xf30fc2;
const UINT32 OP_CMPPD_Vpd_Wpd_Ib        = 0x660fc2;
const UINT32 OP_CMPSD_Vsd_Wsd_Ib        = 0xf20fc2;
const UINT32 OP_MOVNTI_Md_Gd            = 0x0fc3;
const UINT32 OP_PINSRW_Pw_Ew_Ib         = 0x0fc4;
const UINT32 OP_PINSRW_Vw_Ew_Ib         = 0x660fc4;
const UINT32 OP_PEXTRW_Gw_Pw_Ib         = 0x0fc5;
const UINT32 OP_PEXTRW_Gw_Vw_Ib         = 0x660fc5;
const UINT32 OP_SHUFPS_Vps_Wps_Ib       = 0x0fc6;
const UINT32 OP_SHUFPD_Vpd_Wpd_Ib       = 0x660fc6;
const UINT32 OP_G9                      = 0x0fc7;
const UINT32 OP_BSWAP_EAX               = 0x0fc8;
const UINT32 OP_BSWAP_ECX               = 0x0fc9;
const UINT32 OP_BSWAP_EDX               = 0x0fca;
const UINT32 OP_BSWAP_EBX               = 0x0fcb;
const UINT32 OP_BSWAP_ESP               = 0x0fcc;
const UINT32 OP_BSWAP_EBP               = 0x0fcd;
const UINT32 OP_BSWAP_ESI               = 0x0fce;
const UINT32 OP_BSWAP_EDI               = 0x0fcf;

const UINT32 OP_ADDSUBPD_Vpd_Wpd        = 0x0fd0;
const UINT32 OP_ADDSUBPS_Vps_Wps        = 0xf20fd0;
const UINT32 OP_PSRLW_Pq_Qq             = 0x0fd1;
const UINT32 OP_PSRLW_Vdq_Wdq           = 0x660fd1;
const UINT32 OP_PSRLD_Pq_Qq             = 0x0fd2;
const UINT32 OP_PSRLD_Vdq_Wdq           = 0x660fd2;
const UINT32 OP_PSRLQ_Pq_Qq             = 0x0fd3;
const UINT32 OP_PSRLQ_Vdq_Wdq           = 0x660fd3;
const UINT32 OP_PADDQ_Pq_Qq             = 0x0fd4;
const UINT32 OP_PADDQ_Vdq_Wdq           = 0x660fd4;
const UINT32 OP_PMULLW_Pq_Qq            = 0x0fd5;
const UINT32 OP_PMULLW_Vdq_Wdq          = 0x660fd5;
const UINT32 OP_MOVQ_Wq_Vq              = 0x0fd6;
const UINT32 OP_MOVQ2DQ_Vdq_Qq          = 0xf30fd6;
const UINT32 OP_MOVDQ2Q_Pq_Vq           = 0xf20fd6;
const UINT32 OP_PMOVMSKB_Gd_Pq          = 0x0fd7;
const UINT32 OP_PMOVMSKB_Gd_Vdq         = 0x660fd7;
const UINT32 OP_PSUBUSB_Pq_Qq           = 0x0fd8;
const UINT32 OP_PSUBUSB_Vdq_Wdq         = 0x660fd8;
const UINT32 OP_PSUBUSW_Pq_Qq           = 0x0fd9;
const UINT32 OP_PSUBUSW_Vdq_Wdq         = 0x660fd9;
const UINT32 OP_PMINUB_Pq_Qq            = 0x0fda;
const UINT32 OP_PMINUB_Vdq_Wdq          = 0x660fda;
const UINT32 OP_PAND_Pq_Qq              = 0x0fdb;
const UINT32 OP_PAND_Vdq_Wdq            = 0x660fdb;
const UINT32 OP_PADDUSB_Pq_Qq           = 0x0fdc;
const UINT32 OP_PADDUSB_Vdq_Wdq         = 0x660fdc;
const UINT32 OP_PADDUSW_Pq_Qq           = 0x0fdd;
const UINT32 OP_PADDUSW_Vdq_Wdq         = 0x660fdd;
const UINT32 OP_PMAXUB_Pq_Qq            = 0x0fde;
const UINT32 OP_PMAXUB_Vdq_Wdq          = 0x660fde;
const UINT32 OP_PANDN_Pq_Qq             = 0x0fdf;
const UINT32 OP_PANDN_Vdq_Wdq           = 0x660fdf;

const UINT32 OP_PAVGB_Pq_Qq             = 0x0fe0;
const UINT32 OP_PAVGB_Vdq_Wdq           = 0x660fe0;
const UINT32 OP_PSRAW_Pq_Qq             = 0x0fe1;
const UINT32 OP_PSRAW_Vdq_Wdq           = 0x660fe1;
const UINT32 OP_PSRAD_Pq_Qq             = 0x0fe2;
const UINT32 OP_PSRAD_Vdq_Wdq           = 0x660fe2;
const UINT32 OP_PAVGW_Pq_Qq             = 0x0fe3;
const UINT32 OP_PAVGW_Vdq_Wdq           = 0x660fe3;
const UINT32 OP_PMULHUW_Pq_Qq           = 0x0fe4;
const UINT32 OP_PMULHUW_Vdq_Wdq         = 0x660fe4;
const UINT32 OP_PMULHW_Pq_Qq            = 0x0fe5;
const UINT32 OP_PMULHW_Vdq_Wdq          = 0x660fe5;
const UINT32 OP_CVTPD2DQ_Vdq_Wpd        = 0xf20fe6;
const UINT32 OP_CVTTPD2DQ_Vdq_Wpd       = 0x660fe6;
const UINT32 OP_CVTDQ2PD_Vpd_Wq         = 0xf30fe6;
const UINT32 OP_MOVNTQ_Mq_Vq            = 0x0fe7;
const UINT32 OP_MOVNTDQ_Mdq_Vdq         = 0x660fe7;
const UINT32 OP_PSUBSB_Pq_Qq            = 0x0fe8;
const UINT32 OP_PSUBSB_Vdq_Wdq          = 0x660fe8;
const UINT32 OP_PSUBSW_Pq_Qq            = 0x0fe9;
const UINT32 OP_PSUBSW_Vdq_Wdq          = 0x660fe9;
const UINT32 OP_PMINSW_Pq_Qq            = 0x0fea;
const UINT32 OP_PMINSW_Vdq_Wdq          = 0x660fea;
const UINT32 OP_POR_Pq_Qq               = 0x0feb;
const UINT32 OP_POR_Vdq_Wdq             = 0x660feb;
const UINT32 OP_PADDSB_Pq_Qq            = 0x0fec;
const UINT32 OP_PADDSB_Vdq_Wdq          = 0x660fec;
const UINT32 OP_PADDSW_Pq_Qq            = 0x0fed;
const UINT32 OP_PADDSW_Vdq_Wdq          = 0x660fed;
const UINT32 OP_PMAXSW_Pq_Qq            = 0x0fee;
const UINT32 OP_PMAXSW_Vdq_Wdq          = 0x660fee;
const UINT32 OP_PXOR_Pq_Qq              = 0x0fef;
const UINT32 OP_PXOR_Vdq_Wdq            = 0x660fef;

const UINT32 OP_LDDQU_Vdq_Mdq           = 0xf20ff0;
const UINT32 OP_PSLLW_Pq_Qq             = 0x0ff1;
const UINT32 OP_PSLLW_Vdq_Wdq           = 0x660ff1;
const UINT32 OP_PSLLD_Pq_Qq             = 0x0ff2;
const UINT32 OP_PSLLD_Vdq_Wdq           = 0x660ff2;
const UINT32 OP_PSLLQ_Pq_Qq             = 0x0ff3;
const UINT32 OP_PSLLQ_Vdq_Wdq           = 0x660ff3;
const UINT32 OP_PMULUDQ_Pq_Qq           = 0x0ff4;
const UINT32 OP_PMULUDQ_Vdq_Wdq         = 0x660ff4;
const UINT32 OP_PMADDWD_Pq_Qq           = 0x0ff5;
const UINT32 OP_PMADDWD_Vdq_Wdq         = 0x660ff5;
const UINT32 OP_PSADBW_Pq_Qq            = 0x0ff6;
const UINT32 OP_PSADBW_Vdq_Wdq          = 0x660ff6;
const UINT32 OP_MASKMOVQ_Pq_Qq          = 0x0ff7;
const UINT32 OP_MASKMOVDQU_Vdq_Wdq      = 0x660ff7;
const UINT32 OP_PSUBB_Pq_Qq             = 0x0ff8;
const UINT32 OP_PSUBB_Vdq_Wdq           = 0x660ff8;
const UINT32 OP_PSUBW_Pq_Qq             = 0x0ff9;
const UINT32 OP_PSUBW_Vdq_Wdq           = 0x660ff9;
const UINT32 OP_PSUBD_Pq_Qq             = 0x0ffa;
const UINT32 OP_PSUBD_Vdq_Wdq           = 0x660ffa;
const UINT32 OP_PSUBQ_Pq_Qq             = 0x0ffb;
const UINT32 OP_PSUBQ_Vdq_Wdq           = 0x660ffb;
const UINT32 OP_PADDB_Pq_Qq             = 0x0ffc;
const UINT32 OP_PADDB_Vdq_Wdq           = 0x660ffc;
const UINT32 OP_PADDW_Pq_Qq             = 0x0ffd;
const UINT32 OP_PADDW_Vdq_Wdq           = 0x660ffd;
const UINT32 OP_PADDD_Pq_Qq             = 0x0ffe;
const UINT32 OP_PADDD_Vdq_Wdq           = 0x660ffe;


// triple byte opcodes (0f 38)
const UINT32 OP_PSHUFB_Pq_Qq            = 0x0f3800;
const UINT32 OP_PSHUFB_Vdq_Wdq          = 0x660f3800;
const UINT32 OP_PHADDW_Pq_Qq            = 0x0f3801;
const UINT32 OP_PHADDW_Vdq_Wdq          = 0x660f3801;
const UINT32 OP_PHADDD_Pq_Qq            = 0x0f3802;
const UINT32 OP_PHADDD_Vdq_Wdq          = 0x660f3802;
const UINT32 OP_PHADDSW_Pq_Qq           = 0x0f3803;
const UINT32 OP_PHADDSW_Vdq_Wdq         = 0x660f3803;
const UINT32 OP_PMADDUBSW_Pq_Qq         = 0x0f3804;
const UINT32 OP_PMADDUBSW_Vdq_Wdq       = 0x660f3804;
const UINT32 OP_PHSUBW_Pq_Qq            = 0x0f3805;
const UINT32 OP_PHSUBW_Vdq_Wdq          = 0x660f3805;
const UINT32 OP_PHSUBD_Pq_Qq            = 0x0f3806;
const UINT32 OP_PHSUBD_Vdq_Wdq          = 0x660f3806;
const UINT32 OP_PHSUBSW_Pq_Qq           = 0x0f3807;
const UINT32 OP_PHSUBSW_Vdq_Wdq         = 0x660f3807;
const UINT32 OP_PSIGNB_Pq_Qq            = 0x0f3808;
const UINT32 OP_PSIGNB_Vdq_Wdq          = 0x660f3808;
const UINT32 OP_PSIGNW_Pq_Qq            = 0x0f3809;
const UINT32 OP_PSIGNW_Vdq_Wdq          = 0x660f3809;
const UINT32 OP_PSIGND_Pq_Qq            = 0x0f380a;
const UINT32 OP_PSIGND_Vdq_Wdq          = 0x660f380a;
const UINT32 OP_PMULHRSW_Pq_Qq          = 0x0f380b;
const UINT32 OP_PMULHRSW_Vdq_Wdq        = 0x660f380b;

const UINT32 OP_PBLENDVB_Vdq_Wdq        = 0x660f3810;
const UINT32 OP_PBLENDVPS_Vdq_Wdq       = 0x660f3814;
const UINT32 OP_PBLENDVPD_Vdq_Wdq       = 0x660f3815;
const UINT32 OP_PTEST_Vdq_Wdq           = 0x660f3817;
const UINT32 OP_PABSB_Pq_Qq             = 0x0f381c;
const UINT32 OP_PABSB_Vdq_Wdq           = 0x660f381c;
const UINT32 OP_PABSW_Pq_Qq             = 0x0f381d;
const UINT32 OP_PABSW_Vdq_Wdq           = 0x660f381d;
const UINT32 OP_PABSD_Pq_Qq             = 0x0f381e;
const UINT32 OP_PABSD_Vdq_Wdq           = 0x660f381e;

const UINT32 OP_PMOVSXBW_Vdq_Udq        = 0x660f3820;
const UINT32 OP_PMOVSXBD_Vdq_Udq        = 0x660f3821;
const UINT32 OP_PMOVSXBQ_Vdq_Udq        = 0x660f3822;
const UINT32 OP_PMOVSXWD_Vdq_Udq        = 0x660f3823;
const UINT32 OP_PMOVSXWQ_Vdq_Udq        = 0x660f3824;
const UINT32 OP_PMOVSXDQ_Vdq_Udq        = 0x660f3825;
const UINT32 OP_PMULDQ_Vdq_Udq          = 0x660f3828;
const UINT32 OP_PCMPEQQ_Vdq_Udq         = 0x660f3829;
const UINT32 OP_MOVNTDQA_Vdq_Udq        = 0x660f382a;
const UINT32 OP_PACKUSDW_Vdq_Udq        = 0x660f382b;

const UINT32 OP_PMOVZXBW_Vdq_Udq        = 0x660f3830;
const UINT32 OP_PMOVZXBD_Vdq_Udq        = 0x660f3831;
const UINT32 OP_PMOVZXBQ_Vdq_Udq        = 0x660f3832;
const UINT32 OP_PMOVZXWD_Vdq_Udq        = 0x660f3833;
const UINT32 OP_PMOVZXWQ_Vdq_Udq        = 0x660f3834;
const UINT32 OP_PMOVZXDQ_Vdq_Udq        = 0x660f3835;
const UINT32 OP_PMINSB_Vdq_Udq          = 0x660f3838;
const UINT32 OP_PMINSD_Vdq_Udq          = 0x660f3839;
const UINT32 OP_PMINUW_Vdq_Udq          = 0x660f383a;
const UINT32 OP_PMINUD_Vdq_Udq          = 0x660f383b;
const UINT32 OP_PMAXSB_Vdq_Udq          = 0x660f383c;
const UINT32 OP_PMAXSD_Vdq_Udq          = 0x660f383d;
const UINT32 OP_PMAXUW_Vdq_Udq          = 0x660f383e;
const UINT32 OP_PMAXUD_Vdq_Udq          = 0x660f383f;

const UINT32 OP_MULLD_Vdq_Wdq           = 0x660f3840;
const UINT32 OP_PHMINPOSUW_Vdq_Wdq      = 0x660f3841;

const UINT32 OP_NVEPT_Gd_Mdq            = 0x660f3880;
const UINT32 OP_NVVPID_Gd_Mdq           = 0x660f3881;

const UINT32 OP_MOVBE_Gv_Mv             = 0x0f38f0;
const UINT32 OP_CRC32_Gd_Eb             = 0xf20f38f0;
const UINT32 OP_MOVBE_Mv_Gv             = 0x0f38f1;
const UINT32 OP_CRC32_Gd_Ev             = 0xf20f38f1;


// triple byte opcodes (0f 3a)
const UINT32 OP_ROUNDPS_Vdq_Wdq_Ib      = 0x660f3a08;
const UINT32 OP_ROUNDPD_Vdq_Wdq_Ib      = 0x660f3a09;
const UINT32 OP_ROUNDSS_Vss_Wss_Ib      = 0x660f3a0a;
const UINT32 OP_ROUNDSD_Vsd_Wsd_Ib      = 0x660f3a0b;
const UINT32 OP_BLENDPS_Vdq_Wdq_Ib      = 0x660f3a0c;
const UINT32 OP_BLENDPD_Vdq_Wdq_Ib      = 0x660f3a0d;
const UINT32 OP_PBLENDW_Vdq_Wdq_Ib      = 0x660f3a0e;
const UINT32 OP_PALIGNR_Pq_Qq_Ib        = 0x0f3a0f;
const UINT32 OP_PALIGNR_Vdq_Wdq_Ib      = 0x660f3a0f;

const UINT32 OP_EXTRB_Rd_Vdq_Ib         = 0x660f3a14;
const UINT32 OP_EXTRW_Rd_Vdq_Ib         = 0x660f3a15;
const UINT32 OP_EXTRD_Rd_Vdq_Ib         = 0x660f3a16;
const UINT32 OP_EXTRACTPS_Ed_Vdq_Ib     = 0x660f3a17;

const UINT32 OP_PINSRB_Vdq_Rd_Ib        = 0x660f3a20;
const UINT32 OP_INSERTPS_Vdq_Udq_Ib     = 0x660f3a21;
const UINT32 OP_PINSRD_Vdq_Ed_Ib        = 0x660f3a22;

const UINT32 OP_DPPS_Vdq_Wdq_Ib         = 0x660f3a40;
const UINT32 OP_DPPD_Vdq_Wdq_Ib         = 0x660f3a41;
const UINT32 OP_MPSADBW_Vdq_Wdq_Ib      = 0x660f3a42;

const UINT32 OP_PCMPESTRM_Vdq_Wdq_Ib    = 0x660f3a60;
const UINT32 OP_PCMPESTRI_Vdq_Wdq_Ib    = 0x660f3a61;
const UINT32 OP_PCMPISTRM_Vdq_Wdq_Ib    = 0x660f3a62;
const UINT32 OP_PCMPISTRI_Vdq_Wdq_Ib    = 0x660f3a63;


// floating point opcodes
const UINT32 OP_FADD_ST0_STn            = 0xd8c0;
const UINT32 OP_FMUL_ST0_STn            = 0xd8c8;
const UINT32 OP_FCOM_ST0_STn            = 0xd8d0;
const UINT32 OP_FCOMP_ST0_STn           = 0xd8d8;
const UINT32 OP_FSUB_ST0_STn            = 0xd8e0;
const UINT32 OP_FSUBR_ST0_STn           = 0xd8e8;
const UINT32 OP_FDIV_ST0_STn            = 0xd8f0;
const UINT32 OP_FDIVR_ST0_STn           = 0xd8f8;
const UINT32 OP_FLD_ST0_STn             = 0xd9c0;
const UINT32 OP_FXCH_ST0_STn            = 0xd9c8;
const UINT32 OP_FNOP                    = 0xd9d0;
const UINT32 OP_FCHS                    = 0xd9e0;
const UINT32 OP_FABS                    = 0xd9e1;
const UINT32 OP_FTST                    = 0xd9e4;
const UINT32 OP_FXAM                    = 0xd9e5;
const UINT32 OP_FLD1                    = 0xd9e8;
const UINT32 OP_FLDL2T                  = 0xd9e9;
const UINT32 OP_FLDL2E                  = 0xd9ea;
const UINT32 OP_FLDPI                   = 0xd9eb;
const UINT32 OP_FLDLG2                  = 0xd9ec;
const UINT32 OP_FLDLN2                  = 0xd9ed;
const UINT32 OP_FLDZ                    = 0xd9ee;
const UINT32 OP_F2XM1                   = 0xd9f0;
const UINT32 OP_FYL2X                   = 0xd9f1;
const UINT32 OP_FPTAN                   = 0xd9f2;
const UINT32 OP_FPATAN                  = 0xd9f3;
const UINT32 OP_FXTRACT                 = 0xd9f4;
const UINT32 OP_FPREM1                  = 0xd9f5;
const UINT32 OP_FDECSTP                 = 0xd9f6;
const UINT32 OP_FINCSTP                 = 0xd9f7;
const UINT32 OP_FPREM                   = 0xd9f8;
const UINT32 OP_FYL2XP1                 = 0xd9f9;
const UINT32 OP_FSQRT                   = 0xd9fa;
const UINT32 OP_FSINCOS                 = 0xd9fb;
const UINT32 OP_FRNDINT                 = 0xd9fc;
const UINT32 OP_FSCALE                  = 0xd9fd;
const UINT32 OP_FSIN                    = 0xd9fe;
const UINT32 OP_FCOS                    = 0xd9ff;
const UINT32 OP_FCMOVB_ST0_STn          = 0xdac0;
const UINT32 OP_FCMOVE_ST0_STn          = 0xdac8;
const UINT32 OP_FCMOVBE_ST0_STn         = 0xdad0;
const UINT32 OP_FCMOVU_ST0_STn          = 0xdad8;
const UINT32 OP_FUCOMPP                 = 0xdae9;
const UINT32 OP_FCMOVNB_ST0_STn         = 0xdbc0;
const UINT32 OP_FCMOVNE_ST0_STn         = 0xdbc8;
const UINT32 OP_FCMOVNBE_ST0_STn        = 0xdbd0;
const UINT32 OP_FCMOVNU_ST0_STn         = 0xdbd8;
const UINT32 OP_FCLEX                   = 0xdbe2;
const UINT32 OP_FINIT                   = 0xdbe3;
const UINT32 OP_FUCOMI_ST0_STn          = 0xdbe8;
const UINT32 OP_FCOMI_ST0_STn           = 0xdbf0;
const UINT32 OP_FADD_STn_ST0            = 0xdcc0;
const UINT32 OP_FMUL_STn_ST0            = 0xdcc8;
const UINT32 OP_FSUBR_STn_ST0           = 0xdce0;
const UINT32 OP_FSUB_STn_ST0            = 0xdce8;
const UINT32 OP_FDIVR_STn_ST0           = 0xdcf0;
const UINT32 OP_FDIV_STn_ST0            = 0xdcf8;
const UINT32 OP_FFREE_STn               = 0xddc0;
const UINT32 OP_FST_STn                 = 0xddd0;
const UINT32 OP_FSTP_STn                = 0xddd8;
const UINT32 OP_FUCOM_STn_ST0           = 0xdde0;
const UINT32 OP_FUCOMP_STn              = 0xdde8;
const UINT32 OP_FADDP_STn_ST0           = 0xdec0;
const UINT32 OP_FMULP_STn_ST0           = 0xdec8;
const UINT32 OP_FCOMPP                  = 0xded9;
const UINT32 OP_FSUBRP_STn_ST0          = 0xdee0;
const UINT32 OP_FSUBP_STn_ST0           = 0xdee8;
const UINT32 OP_FDIVRP_STn_ST0          = 0xdef0;
const UINT32 OP_FDIVP_STn_ST0           = 0xdef8;
const UINT32 OP_FSTSW_AX                = 0xdfe0;
const UINT32 OP_FCOMIP_ST0_STn          = 0xdff0;



//**************************************************************************
//  MEMORY REFERENCES
//**************************************************************************

inline x86_memref MBD(UINT8 base, INT32 disp) { return x86_memref(base, REG_NONE, 1, disp); }
inline x86_memref MBISD(UINT8 base, UINT8 ind, UINT8 scale, INT32 disp) { return x86_memref(base, ind, scale, disp); }

#if (X86EMIT_SIZE == 32)
inline x86_memref MABS(const void *mem) { return x86_memref(REG_NONE, REG_NONE, 1, reinterpret_cast<FPTR>(const_cast<void *>(mem))); }
inline x86_memref MABSI(const void *mem, UINT8 index) { return x86_memref(index, REG_NONE, 1, reinterpret_cast<FPTR>(const_cast<void *>(mem))); }
inline x86_memref MABSI(const void *mem, UINT8 index, UINT8 scale) { return x86_memref(REG_NONE, index, scale, reinterpret_cast<FPTR>(const_cast<void *>(mem))); }
#endif



//**************************************************************************
//  CORE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  make_modrm - assemble a modrm byte from the
//  three components
//-------------------------------------------------

inline UINT8 make_modrm(UINT8 mode, UINT8 reg, UINT8 rm)
{
	assert(mode < 4);
	assert(reg < REG_MAX);
	assert(rm < REG_MAX);
	return (mode << 6) | ((reg & 7) << 3) | (rm & 7);
}


//-------------------------------------------------
//  make_sib - assemble an sib byte from the
//  three components
//-------------------------------------------------

inline UINT8 make_sib(UINT8 scale, UINT8 index, UINT8 base)
{
	static const UINT8 scale_lookup[9] = { 0<<6, 0<<6, 1<<6, 0<<6, 2<<6, 0<<6, 0<<6, 0<<6, 3<<6 };
	assert(scale == 1 || scale == 2 || scale == 4 || scale == 8);
	assert(index < REG_MAX);
	assert(base < REG_MAX);
	return scale_lookup[scale] | ((index & 7) << 3) | (base & 7);
}


//-------------------------------------------------
//  emit_byte - emit a byte
//-------------------------------------------------

inline void emit_byte(x86code *&emitptr, UINT8 byte)
{
	*((UINT8 *)emitptr) = byte;
	emitptr += 1;
}


//-------------------------------------------------
//  emit_word - emit a word
//-------------------------------------------------

inline void emit_word(x86code *&emitptr, UINT16 word)
{
	*((UINT16 *)emitptr) = word;
	emitptr += 2;
}


//-------------------------------------------------
//  emit_dword - emit a dword
//-------------------------------------------------

inline void emit_dword(x86code *&emitptr, UINT32 dword)
{
	*((UINT32 *)emitptr) = dword;
	emitptr += 4;
}


//-------------------------------------------------
//  emit_qword - emit a dword
//-------------------------------------------------

inline void emit_qword(x86code *&emitptr, UINT64 qword)
{
	*((UINT64 *)emitptr) = qword;
	emitptr += 8;
}



//**************************************************************************
//  GENERIC OPCODE EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_op - emit a 1, 2, or 3-byte opcode,
//  along with any necessary REX prefixes for x64
//-------------------------------------------------

inline void emit_op(x86code *&emitptr, UINT32 op, UINT8 opsize, UINT8 reg, UINT8 sib, UINT8 rm)
{
	if (opsize == OP_16BIT)
		emit_byte(emitptr, PREFIX_OPSIZE);

#if (X86EMIT_SIZE == 64)
{
	UINT8 rex;

	assert(opsize == OP_16BIT || opsize == OP_32BIT || opsize == OP_64BIT);

	rex = (opsize & 8) | ((reg & 8) >> 1) | ((sib & 8) >> 2) | ((rm & 8) >> 3);
	if (rex != 0 || ((op & OPFLAG_8BITREG) && reg >= 4) || ((op & OPFLAG_8BITRM) && rm >= 4))
		emit_byte(emitptr, OP_REX + rex);
}
#else
	assert(opsize != OP_64BIT);
#endif

	if ((op & 0xff0000) != 0)
		emit_byte(emitptr, op >> 16);
	if ((op & 0xff00) != 0)
		emit_byte(emitptr, op >> 8);
	emit_byte(emitptr, op);
}


//-------------------------------------------------
//  emit_op_simple - emit a simple opcode
//-------------------------------------------------

inline void emit_op_simple(x86code *&emitptr, UINT32 op, UINT8 opsize)
{
	emit_op(emitptr, op, opsize, 0, 0, 0);
}


//-------------------------------------------------
//  emit_op_reg - emit a simple opcode that has
//  a register parameter
//-------------------------------------------------

inline void emit_op_reg(x86code *&emitptr, UINT32 op, UINT8 opsize, UINT8 reg)
{
	emit_op(emitptr, op, opsize, 0, 0, reg);
}


//-------------------------------------------------
//  emit_op_modrm_reg - emit an opcode with a
//  register modrm byte
//-------------------------------------------------

inline void emit_op_modrm_reg(x86code *&emitptr, UINT32 op, UINT8 opsize, UINT8 reg, UINT8 rm)
{
	assert(reg < REG_MAX);
	assert(rm < REG_MAX);

	emit_op(emitptr, op, opsize, reg, 0, rm);
	emit_byte(emitptr, make_modrm(3, reg, rm));
}


//-------------------------------------------------
//  emit_op_modrm_mem - emit an opcode with a
//  memory modrm byte
//-------------------------------------------------

inline void emit_op_modrm_mem(x86code *&emitptr, UINT32 op, UINT8 opsize, UINT8 reg, x86_memref memref)
{
	assert(reg < REG_MAX);
	assert(memref.m_base < REG_MAX || memref.m_base == REG_NONE);
	assert(memref.m_index < REG_MAX || memref.m_index == REG_NONE);

	// turn off the OPFLAG_8BITRM flag since it only applies to register operands
	op &= ~OPFLAG_8BITRM;

	// displacement only case
	if (memref.m_base == REG_NONE && memref.m_index == REG_NONE)
	{
#if (X86EMIT_SIZE == 32)
		emit_op(emitptr, op, opsize, reg, 0, 5);
		emit_byte(emitptr, make_modrm(0, reg, 5));
		emit_dword(emitptr, memref.m_disp);
#else
		fatalerror("Invalid absolute mode in 64-bits!\n");
#endif
	}

	// base only case
	else if (memref.m_index == REG_NONE && (memref.m_base & 7) != REG_ESP)
	{
		emit_op(emitptr, op, opsize, reg, 0, memref.m_base);

		// mode 0 for no offset
		if (memref.m_disp == 0 && (memref.m_base & 7) != REG_EBP)
			emit_byte(emitptr, make_modrm(0, reg, memref.m_base));

		// mode 1 for 1-byte offset
		else if ((INT8)memref.m_disp == memref.m_disp)
		{
			emit_byte(emitptr, make_modrm(1, reg, memref.m_base));
			emit_byte(emitptr, (INT8)memref.m_disp);
		}

		// mode 2 for 4-byte offset
		else
		{
			emit_byte(emitptr, make_modrm(2, reg, memref.m_base));
			emit_dword(emitptr, memref.m_disp);
		}
	}

	// full BISD case
	else
	{
		assert(memref.m_index != REG_ESP);
		emit_op(emitptr, op, opsize, reg, memref.m_index, memref.m_base);

		// a "none" index == REG_ESP
		if (memref.m_index == REG_NONE)
			memref.m_index = REG_ESP;

		// no base is a special case
		if (memref.m_base == REG_NONE)
		{
			emit_byte(emitptr, make_modrm(0, reg, 4));
			emit_byte(emitptr, make_sib(memref.m_scale, memref.m_index, REG_EBP));
			emit_dword(emitptr, memref.m_disp);
		}

		// mode 0 for no offset
		else if (memref.m_disp == 0 && (memref.m_base & 7) != REG_EBP)
		{
			emit_byte(emitptr, make_modrm(0, reg, 4));
			emit_byte(emitptr, make_sib(memref.m_scale, memref.m_index, memref.m_base));
		}

		// mode 1 for 1-byte offset
		else if ((INT8)memref.m_disp == memref.m_disp)
		{
			emit_byte(emitptr, make_modrm(1, reg, 4));
			emit_byte(emitptr, make_sib(memref.m_scale, memref.m_index, memref.m_base));
			emit_byte(emitptr, (INT8)memref.m_disp);
		}

		// mode 2 for 4-byte offset
		else
		{
			emit_byte(emitptr, make_modrm(2, reg, 4));
			emit_byte(emitptr, make_sib(memref.m_scale, memref.m_index, memref.m_base));
			emit_dword(emitptr, memref.m_disp);
		}
	}
}



//**************************************************************************
//  GENERIC OPCODE + IMMEDIATE EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_op_modrm_reg_imm8 - emit an opcode with a
//  register modrm byte and an 8-bit immediate
//-------------------------------------------------

inline void emit_op_modrm_reg_imm8(x86code *&emitptr, UINT32 op, UINT8 opsize, UINT8 reg, UINT8 rm, UINT8 imm)
{
	emit_op_modrm_reg(emitptr, op, opsize, reg, rm);
	emit_byte(emitptr, imm);
}


//-------------------------------------------------
//  emit_op_modrm_mem_imm8 - emit an opcode with a
//  memory modrm byte and an 8-bit immediate
//-------------------------------------------------

inline void emit_op_modrm_mem_imm8(x86code *&emitptr, UINT32 op, UINT8 opsize, UINT8 reg, x86_memref memref, UINT8 imm)
{
	emit_op_modrm_mem(emitptr, op, opsize, reg, memref);
	emit_byte(emitptr, imm);
}


//-------------------------------------------------
//  emit_op_modrm_reg_imm16 - emit an opcode with a
//  register modrm byte and a 16-bit immediate
//-------------------------------------------------

inline void emit_op_modrm_reg_imm16(x86code *&emitptr, UINT32 op, UINT8 opsize, UINT8 reg, UINT8 rm, UINT16 imm)
{
	emit_op_modrm_reg(emitptr, op, opsize, reg, rm);
	emit_word(emitptr, imm);
}


//-------------------------------------------------
//  emit_op_modrm_mem_imm16 - emit an opcode with a
//  memory modrm byte and a 16-bit immediate
//-------------------------------------------------

inline void emit_op_modrm_mem_imm16(x86code *&emitptr, UINT32 op, UINT8 opsize, UINT8 reg, x86_memref memref, UINT16 imm)
{
	emit_op_modrm_mem(emitptr, op, opsize, reg, memref);
	emit_word(emitptr, imm);
}


//-------------------------------------------------
//  emit_op_modrm_reg_imm816 - emit an opcode with
//  a register modrm byte and an 8-bit or 16-bit
//  immediate
//-------------------------------------------------

inline void emit_op_modrm_reg_imm816(x86code *&emitptr, UINT32 op8, UINT32 op16, UINT8 opsize, UINT8 reg, UINT8 rm, UINT16 imm)
{
	if ((INT8)imm == (INT16)imm)
	{
		emit_op_modrm_reg(emitptr, op8, opsize, reg, rm);
		emit_byte(emitptr, imm);
	}
	else
	{
		emit_op_modrm_reg(emitptr, op16, opsize, reg, rm);
		emit_word(emitptr, imm);
	}
}


//-------------------------------------------------
//  emit_op_modrm_mem_imm816 - emit an opcode with
//  a memory modrm byte and an 8-bit or 16-bit
//  immediate
//-------------------------------------------------

inline void emit_op_modrm_mem_imm816(x86code *&emitptr, UINT32 op8, UINT32 op16, UINT8 opsize, UINT8 reg, x86_memref memref, UINT16 imm)
{
	if ((INT8)imm == (INT16)imm)
	{
		emit_op_modrm_mem(emitptr, op8, opsize, reg, memref);
		emit_byte(emitptr, imm);
	}
	else
	{
		emit_op_modrm_mem(emitptr, op16, opsize, reg, memref);
		emit_word(emitptr, imm);
	}
}


//-------------------------------------------------
//  emit_op_modrm_reg_imm32 - emit an opcode with a
//  register modrm byte and a 32-bit immediate
//-------------------------------------------------

inline void emit_op_modrm_reg_imm32(x86code *&emitptr, UINT32 op, UINT8 opsize, UINT8 reg, UINT8 rm, UINT32 imm)
{
	emit_op_modrm_reg(emitptr, op, opsize, reg, rm);
	emit_dword(emitptr, imm);
}


//-------------------------------------------------
//  emit_op_modrm_mem_imm32 - emit an opcode with a
//  memory modrm byte and a 32-bit immediate
//-------------------------------------------------

inline void emit_op_modrm_mem_imm32(x86code *&emitptr, UINT32 op, UINT8 opsize, UINT8 reg, x86_memref memref, UINT32 imm)
{
	emit_op_modrm_mem(emitptr, op, opsize, reg, memref);
	emit_dword(emitptr, imm);
}


//-------------------------------------------------
//  emit_op_modrm_reg_imm832 - emit an opcode with
//  a register modrm byte and an 8-bit or 32-bit
//  immediate
//-------------------------------------------------

inline void emit_op_modrm_reg_imm832(x86code *&emitptr, UINT32 op8, UINT32 op32, UINT8 opsize, UINT8 reg, UINT8 rm, UINT32 imm)
{
	if ((INT8)imm == (INT32)imm)
	{
		emit_op_modrm_reg(emitptr, op8, opsize, reg, rm);
		emit_byte(emitptr, imm);
	}
	else
	{
		emit_op_modrm_reg(emitptr, op32, opsize, reg, rm);
		emit_dword(emitptr, imm);
	}
}


//-------------------------------------------------
//  emit_op_modrm_mem_imm832 - emit an opcode with
//  a memory modrm byte and an 8-bit or 32-bit
//  immediate
//-------------------------------------------------

inline void emit_op_modrm_mem_imm832(x86code *&emitptr, UINT32 op8, UINT32 op32, UINT8 opsize, UINT8 reg, x86_memref memref, UINT32 imm)
{
	if ((INT8)imm == (INT32)imm)
	{
		emit_op_modrm_mem(emitptr, op8, opsize, reg, memref);
		emit_byte(emitptr, imm);
	}
	else
	{
		emit_op_modrm_mem(emitptr, op32, opsize, reg, memref);
		emit_dword(emitptr, imm);
	}
}



//**************************************************************************
//  SIMPLE OPCODE EMITTERS
//**************************************************************************

inline void emit_nop(x86code *&emitptr)    { emit_op_simple(emitptr, OP_NOP, OP_32BIT); }
inline void emit_int_3(x86code *&emitptr)  { emit_op_simple(emitptr, OP_INT_3, OP_32BIT); }
inline void emit_ret(x86code *&emitptr)    { emit_op_simple(emitptr, OP_RETN, OP_32BIT); }
inline void emit_cdq(x86code *&emitptr)    { emit_op_simple(emitptr, OP_CDQ, OP_32BIT); }
inline void emit_clc(x86code *&emitptr)    { emit_op_simple(emitptr, OP_CLC, OP_32BIT); }
inline void emit_stc(x86code *&emitptr)    { emit_op_simple(emitptr, OP_STC, OP_32BIT); }
inline void emit_cmc(x86code *&emitptr)    { emit_op_simple(emitptr, OP_CMC, OP_32BIT); }
inline void emit_pushf(x86code *&emitptr)  { emit_op_simple(emitptr, OP_PUSHF_Fv, OP_32BIT); }
inline void emit_popf(x86code *&emitptr)   { emit_op_simple(emitptr, OP_POPF_Fv, OP_32BIT); }
inline void emit_cpuid(x86code *&emitptr)  { emit_op_simple(emitptr, OP_CPUID, OP_32BIT); }

#if (X86EMIT_SIZE == 32)
inline void emit_pushad(x86code *&emitptr) { emit_op_simple(emitptr, OP_PUSHA, OP_32BIT); }
inline void emit_popad(x86code *&emitptr)  { emit_op_simple(emitptr, OP_POPA, OP_32BIT); }
inline void emit_lahf(x86code *&emitptr)   { emit_op_simple(emitptr, OP_LAHF, OP_32BIT); }
inline void emit_sahf(x86code *&emitptr)   { emit_op_simple(emitptr, OP_SAHF, OP_32BIT); }
#endif

#if (X86EMIT_SIZE == 64)
inline void emit_cqo(x86code *&emitptr)    { emit_op_simple(emitptr, OP_CQO, OP_64BIT); }
#endif



//**************************************************************************
//  CALL/JUMP EMITTERS
//**************************************************************************

//-------------------------------------------------
//  resolve_link - resolve a link in a jump
//  instruction
//-------------------------------------------------

inline void resolve_link(x86code *&destptr, const emit_link &linkinfo)
{
	INT64 delta = destptr - linkinfo.target;
	if (linkinfo.size == 1)
	{
		assert((INT8)delta == delta);
		((INT8 *)linkinfo.target)[-1] = (INT8)delta;
	}
	else if (linkinfo.size == 2)
	{
		assert((INT16)delta == delta);
		((INT16 *)linkinfo.target)[-1] = (INT16)delta;
	}
	else
	{
		assert((INT32)delta == delta);
		((INT32 *)linkinfo.target)[-1] = (INT32)delta;
	}
}


//-------------------------------------------------
//  emit_call_*
//-------------------------------------------------

inline void emit_call_link(x86code *&emitptr, emit_link &linkinfo)
{
	emit_op_simple(emitptr, OP_CALL_Jz, OP_32BIT);
	emit_dword(emitptr, 0);
	linkinfo.target = emitptr;
	linkinfo.size = 4;
}

inline void emit_call(x86code *&emitptr, x86code *target)
{
	emit_link link;
	emit_call_link(emitptr, link);
	resolve_link(target, link);
}


//-------------------------------------------------
//  emit_jmp_*
//-------------------------------------------------

inline void emit_jmp_short_link(x86code *&emitptr, emit_link &linkinfo)
{
	emit_op_simple(emitptr, OP_JMP_Jb, OP_32BIT);
	emit_byte(emitptr, 0);
	linkinfo.target = emitptr;
	linkinfo.size = 1;
}

inline void emit_jmp_near_link(x86code *&emitptr, emit_link &linkinfo)
{
	emit_op_simple(emitptr, OP_JMP_Jz, OP_32BIT);
	emit_dword(emitptr, 0);
	linkinfo.target = emitptr;
	linkinfo.size = 4;
}

inline void emit_jmp(x86code *&emitptr, x86code *target)
{
	INT32 delta = target - (emitptr + 2);
	emit_link link;

	if ((INT8)delta == delta)
		emit_jmp_short_link(emitptr, link);
	else
		emit_jmp_near_link(emitptr, link);
	resolve_link(target, link);
}


//-------------------------------------------------
//  emit_jcc_*
//-------------------------------------------------

inline void emit_jcc_short_link(x86code *&emitptr, UINT8 cond, emit_link &linkinfo)
{
	emit_op_simple(emitptr, OP_JCC_O_Jb + cond, OP_32BIT);
	emit_byte(emitptr, 0);
	linkinfo.target = emitptr;
	linkinfo.size = 1;
}

inline void emit_jcc_near_link(x86code *&emitptr, UINT8 cond, emit_link &linkinfo)
{
	emit_op_simple(emitptr, OP_JCC_O_Jv + cond, OP_32BIT);
	emit_dword(emitptr, 0);
	linkinfo.target = emitptr;
	linkinfo.size = 4;
}

inline void emit_jcc(x86code *&emitptr, UINT8 cond, x86code *target)
{
	INT32 delta = emitptr + 2 - target;
	emit_link link;

	if ((INT8)delta == delta)
		emit_jcc_short_link(emitptr, cond, link);
	else
		emit_jcc_near_link(emitptr, cond, link);
	resolve_link(target, link);
}


//-------------------------------------------------
//  emit_jecxz_*
//-------------------------------------------------

inline void emit_jecxz_link(x86code *&emitptr, emit_link &linkinfo)
{
	emit_op_simple(emitptr, OP_JrCXZ_Jb, OP_32BIT);
	emit_byte(emitptr, 0);
	linkinfo.target = emitptr;
	linkinfo.size = 1;
}

inline void emit_jecxz(x86code *&emitptr, x86code *target)
{
	emit_link link;
	emit_jecxz_link(emitptr, link);
	resolve_link(target, link);
}

#if (X86EMIT_SIZE == 64)

inline void emit_jrcxz_link(x86code *&emitptr, emit_link &linkinfo)
{
	emit_op_simple(emitptr, OP_JrCXZ_Jb, OP_64BIT);
	emit_byte(emitptr, 0);
	linkinfo.target = emitptr;
	linkinfo.size = 1;
}

inline void emit_jrcxz(x86code *&emitptr, x86code *target)
{
	emit_link link;
	emit_jrcxz_link(emitptr, link);
	resolve_link(target, link);
}

#endif


//-------------------------------------------------
//  emit_call/jmp_*
//-------------------------------------------------

#if (X86EMIT_SIZE == 32)
inline void emit_call_r32(x86code *&emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G5, OP_32BIT, 2, dreg); }
inline void emit_call_m32(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 2, memref); }
inline void emit_jmp_r32(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G5, OP_32BIT, 4, dreg); }
inline void emit_jmp_m32(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 4, memref); }
#else
inline void emit_call_r64(x86code *&emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G5, OP_32BIT, 2, dreg); }
inline void emit_call_m64(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 2, memref); }
inline void emit_jmp_r64(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G5, OP_32BIT, 4, dreg); }
inline void emit_jmp_m64(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 4, memref); }
#endif


//-------------------------------------------------
//  emit_ret_*
//-------------------------------------------------

inline void emit_ret_imm(x86code *&emitptr, UINT16 imm)
{
	emit_op_simple(emitptr, OP_RETN_Iw, OP_32BIT);
	emit_word(emitptr, imm);
}



//**************************************************************************
//  PUSH/POP EMITTERS
//**************************************************************************

inline void emit_push_imm(x86code *&emitptr, INT32 imm)
{
	if ((INT8)imm == imm)
	{
		emit_op_simple(emitptr, OP_PUSH_Ib, OP_32BIT);
		emit_byte(emitptr, (INT8)imm);
	}
	else
	{
		emit_op_simple(emitptr, OP_PUSH_Iz, OP_32BIT);
		emit_dword(emitptr, imm);
	}
}

#if (X86EMIT_SIZE == 32)

inline void emit_push_r32(x86code *&emitptr, UINT8 reg)                         { emit_op_reg(emitptr, OP_PUSH_rAX + (reg & 7), OP_32BIT, reg); }
inline void emit_push_m32(x86code *&emitptr, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 6, memref); }
inline void emit_pop_r32(x86code *&emitptr, UINT8 reg)                          { emit_op_reg(emitptr, OP_POP_rAX + (reg & 7), OP_32BIT, reg); }
inline void emit_pop_m32(x86code *&emitptr, x86_memref memref)                  { emit_op_modrm_mem(emitptr, OP_G1A_Ev, OP_32BIT, 0, memref); }

#else

inline void emit_push_r64(x86code *&emitptr, UINT8 reg)                         { emit_op_reg(emitptr, OP_PUSH_rAX + (reg & 7), OP_32BIT, reg); }
inline void emit_push_m64(x86code *&emitptr, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 6, memref); }
inline void emit_pop_r64(x86code *&emitptr, UINT8 reg)                          { emit_op_reg(emitptr, OP_POP_rAX + (reg & 7), OP_32BIT, reg); }
inline void emit_pop_m64(x86code *&emitptr, x86_memref memref)                  { emit_op_modrm_mem(emitptr, OP_G1A_Ev, OP_32BIT, 0, memref); }

#endif



//**************************************************************************
//  MOVE EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_mov_r8_*
//-------------------------------------------------

inline void emit_mov_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)
{
	emit_op_reg(emitptr, OP_MOV_AL_Ib | (dreg & 7), OP_32BIT, dreg);
	emit_byte(emitptr, imm);
}

inline void emit_mov_r8_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_MOV_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_mov_r8_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_MOV_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_mov_m8_r8(x86code *&emitptr, x86_memref memref, UINT8 sreg)    { emit_op_modrm_mem(emitptr, OP_MOV_Eb_Gb, OP_32BIT, sreg, memref); }
inline void emit_mov_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G11_Eb_Ib, OP_32BIT, 0, memref, imm); }


//-------------------------------------------------
//  emit_xchg_r8_*
//-------------------------------------------------

inline void emit_xchg_r8_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)
{
	if (dreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (sreg & 7), OP_32BIT, sreg);
	else if (sreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (dreg & 7), OP_32BIT, dreg);
	else
		emit_op_modrm_reg(emitptr, OP_XCHG_Eb_Gb, OP_32BIT, dreg, sreg);
}


//-------------------------------------------------
//  emit_mov_r16_*
//-------------------------------------------------

inline void emit_mov_r16_imm(x86code *&emitptr, UINT8 dreg, UINT16 imm)
{
	emit_op_reg(emitptr, OP_MOV_rAX_Iv | (dreg & 7), OP_16BIT, dreg);
	emit_word(emitptr, imm);
}

inline void emit_mov_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                         { emit_op_modrm_reg(emitptr, OP_MOV_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_mov_r16_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)                  { emit_op_modrm_mem(emitptr, OP_MOV_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_mov_m16_imm(x86code *&emitptr, x86_memref memref, UINT16 imm)                  { emit_op_modrm_mem_imm16(emitptr, OP_G11_Ev_Iz, OP_16BIT, 0, memref, imm); }
inline void emit_mov_m16_r16(x86code *&emitptr, x86_memref memref, UINT8 sreg)                  { emit_op_modrm_mem(emitptr, OP_MOV_Ev_Gv, OP_16BIT, sreg, memref); }
inline void emit_movsx_r16_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                        { emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Eb, OP_16BIT, dreg, sreg); }
inline void emit_movsx_r16_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Eb, OP_16BIT, dreg, memref); }
inline void emit_movzx_r16_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                        { emit_op_modrm_reg(emitptr, OP_MOVZX_Gv_Eb, OP_16BIT, dreg, sreg); }
inline void emit_movzx_r16_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Eb, OP_16BIT, dreg, memref); }
inline void emit_cmovcc_r16_r16(x86code *&emitptr, UINT8 cond, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_16BIT, dreg, sreg); }
inline void emit_cmovcc_r16_m16(x86code *&emitptr, UINT8 cond, UINT8 dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_16BIT, dreg, memref); }


//-------------------------------------------------
//  emit_xchg_r16_*
//-------------------------------------------------

inline void emit_xchg_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)
{
	if (dreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (sreg & 7), OP_16BIT, sreg);
	else if (sreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (dreg & 7), OP_16BIT, dreg);
	else
		emit_op_modrm_reg(emitptr, OP_XCHG_Ev_Gv, OP_16BIT, dreg, sreg);
}


//-------------------------------------------------
//  emit_mov_r32_*
//-------------------------------------------------

inline void emit_mov_r32_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)
{
	emit_op_reg(emitptr, OP_MOV_rAX_Iv | (dreg & 7), OP_32BIT, dreg);
	emit_dword(emitptr, imm);
}

inline void emit_mov_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                         { emit_op_modrm_reg(emitptr, OP_MOV_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_mov_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)                  { emit_op_modrm_mem(emitptr, OP_MOV_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_mov_m32_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)                  { emit_op_modrm_mem_imm32(emitptr, OP_G11_Ev_Iz, OP_32BIT, 0, memref, imm); }
inline void emit_mov_m32_r32(x86code *&emitptr, x86_memref memref, UINT8 sreg)                  { emit_op_modrm_mem(emitptr, OP_MOV_Ev_Gv, OP_32BIT, sreg, memref); }
inline void emit_movsx_r32_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                        { emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Eb, OP_32BIT, dreg, sreg); }
inline void emit_movsx_r32_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Eb, OP_32BIT, dreg, memref); }
inline void emit_movsx_r32_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                       { emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Ew, OP_32BIT, dreg, sreg); }
inline void emit_movsx_r32_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Ew, OP_32BIT, dreg, memref); }
inline void emit_movzx_r32_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)
{
#if (X86EMIT_SIZE == 32)
	if (sreg >= 4)
	{
		emit_op_modrm_reg(emitptr, OP_MOV_Gv_Ev, OP_32BIT, dreg, sreg);                         // mov dreg,sreg
		emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 4, dreg, 0xff);   // and dreg,0xff
	}
	else
#endif
	emit_op_modrm_reg(emitptr, OP_MOVZX_Gv_Eb, OP_32BIT, dreg, sreg);
}
inline void emit_movzx_r32_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Eb, OP_32BIT, dreg, memref); }
inline void emit_movzx_r32_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                       { emit_op_modrm_reg(emitptr, OP_MOVZX_Gv_Ew, OP_32BIT, dreg, sreg); }
inline void emit_movzx_r32_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Ew, OP_32BIT, dreg, memref); }
inline void emit_cmovcc_r32_r32(x86code *&emitptr, UINT8 cond, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_32BIT, dreg, sreg); }
inline void emit_cmovcc_r32_m32(x86code *&emitptr, UINT8 cond, UINT8 dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_32BIT, dreg, memref); }


//-------------------------------------------------
//  emit_xchg_r32_*
//-------------------------------------------------

inline void emit_xchg_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)
{
	if (dreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (sreg & 7), OP_32BIT, sreg);
	else if (sreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (dreg & 7), OP_32BIT, dreg);
	else
		emit_op_modrm_reg(emitptr, OP_XCHG_Ev_Gv, OP_32BIT, dreg, sreg);
}


//-------------------------------------------------
//  emit_mov_r64_*
//-------------------------------------------------

#if (X86EMIT_SIZE == 64)

inline void emit_mov_r64_imm(x86code *&emitptr, UINT8 dreg, UINT64 imm)
{
	if ((UINT32)imm == imm)
	{
		emit_op_reg(emitptr, OP_MOV_rAX_Iv | (dreg & 7), OP_32BIT, dreg);
		emit_dword(emitptr, imm);
	}
	else if ((INT32)imm == imm)
		emit_op_modrm_reg_imm32(emitptr, OP_G11_Ev_Iz, OP_64BIT, 0, dreg, imm);
	else
	{
		emit_op_reg(emitptr, OP_MOV_rAX_Iv | (dreg & 7), OP_64BIT, dreg);
		emit_qword(emitptr, imm);
	}
}

inline void emit_mov_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                         { emit_op_modrm_reg(emitptr, OP_MOV_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_mov_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)                  { emit_op_modrm_mem(emitptr, OP_MOV_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_mov_m64_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)                  { emit_op_modrm_mem_imm32(emitptr, OP_G11_Ev_Iz, OP_64BIT, 0, memref, imm); }
inline void emit_mov_m64_r64(x86code *&emitptr, x86_memref memref, UINT8 sreg)                  { emit_op_modrm_mem(emitptr, OP_MOV_Ev_Gv, OP_64BIT, sreg, memref); }
inline void emit_movsx_r64_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                        { emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Eb, OP_64BIT, dreg, sreg); }
inline void emit_movsx_r64_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Eb, OP_64BIT, dreg, memref); }
inline void emit_movsx_r64_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                       { emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Ew, OP_64BIT, dreg, sreg); }
inline void emit_movsx_r64_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Ew, OP_64BIT, dreg, memref); }
inline void emit_movsxd_r64_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                      { emit_op_modrm_reg(emitptr, OP_MOVSXD_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_movsxd_r64_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_MOVSXD_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_movzx_r64_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                        { emit_op_modrm_reg(emitptr, OP_MOVZX_Gv_Eb, OP_64BIT, dreg, sreg); }
inline void emit_movzx_r64_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Eb, OP_64BIT, dreg, memref); }
inline void emit_movzx_r64_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                       { emit_op_modrm_reg(emitptr, OP_MOVZX_Gv_Ew, OP_64BIT, dreg, sreg); }
inline void emit_movzx_r64_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Ew, OP_64BIT, dreg, memref); }
inline void emit_cmovcc_r64_r64(x86code *&emitptr, UINT8 cond, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_64BIT, dreg, sreg); }
inline void emit_cmovcc_r64_m64(x86code *&emitptr, UINT8 cond, UINT8 dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_64BIT, dreg, memref); }


//-------------------------------------------------
//  emit_xchg_r64_*
//-------------------------------------------------

inline void emit_xchg_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)
{
	if (dreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (sreg & 7), OP_64BIT, sreg);
	else if (sreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (dreg & 7), OP_64BIT, dreg);
	else
		emit_op_modrm_reg(emitptr, OP_XCHG_Ev_Gv, OP_64BIT, dreg, sreg);
}

#endif


//**************************************************************************
//  ARITHMETIC EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_arith_r8_*
//-------------------------------------------------

inline void emit_add_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)           { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 0, dreg, imm); }
inline void emit_add_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 0, memref, imm); }
inline void emit_add_r8_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_ADD_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_add_r8_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ADD_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_add_m8_r8(x86code *&emitptr, x86_memref memref, UINT8 sreg)    { emit_op_modrm_mem(emitptr, OP_ADD_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_or_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)            { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 1, dreg, imm); }
inline void emit_or_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)     { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 1, memref, imm); }
inline void emit_or_r8_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)            { emit_op_modrm_reg(emitptr, OP_OR_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_or_r8_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_OR_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_or_m8_r8(x86code *&emitptr, x86_memref memref, UINT8 sreg)     { emit_op_modrm_mem(emitptr, OP_OR_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_adc_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)           { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 2, dreg, imm); }
inline void emit_adc_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 2, memref, imm); }
inline void emit_adc_r8_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_ADC_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_adc_r8_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ADC_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_adc_m8_r8(x86code *&emitptr, x86_memref memref, UINT8 sreg)    { emit_op_modrm_mem(emitptr, OP_ADC_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_sbb_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)           { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 3, dreg, imm); }
inline void emit_sbb_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 3, memref, imm); }
inline void emit_sbb_r8_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_SBB_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_sbb_r8_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_SBB_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_sbb_m8_r8(x86code *&emitptr, x86_memref memref, UINT8 sreg)    { emit_op_modrm_mem(emitptr, OP_SBB_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_and_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)           { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 4, dreg, imm); }
inline void emit_and_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 4, memref, imm); }
inline void emit_and_r8_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_AND_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_and_r8_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_AND_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_and_m8_r8(x86code *&emitptr, x86_memref memref, UINT8 sreg)    { emit_op_modrm_mem(emitptr, OP_AND_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_sub_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)           { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 5, dreg, imm); }
inline void emit_sub_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 5, memref, imm); }
inline void emit_sub_r8_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_SUB_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_sub_r8_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_SUB_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_sub_m8_r8(x86code *&emitptr, x86_memref memref, UINT8 sreg)    { emit_op_modrm_mem(emitptr, OP_SUB_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_xor_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)           { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 6, dreg, imm); }
inline void emit_xor_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 6, memref, imm); }
inline void emit_xor_r8_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_XOR_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_xor_r8_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_XOR_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_xor_m8_r8(x86code *&emitptr, x86_memref memref, UINT8 sreg)    { emit_op_modrm_mem(emitptr, OP_XOR_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_cmp_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)           { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 7, dreg, imm); }
inline void emit_cmp_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 7, memref, imm); }
inline void emit_cmp_r8_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_CMP_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_cmp_r8_m8(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CMP_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_cmp_m8_r8(x86code *&emitptr, x86_memref memref, UINT8 sreg)    { emit_op_modrm_mem(emitptr, OP_CMP_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_test_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_op_modrm_reg_imm8(emitptr, OP_G3_Eb, OP_32BIT, 0, dreg, imm); }
inline void emit_test_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_op_modrm_mem_imm8(emitptr, OP_G3_Eb, OP_32BIT, 0, memref, imm); }
inline void emit_test_r8_r8(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_TEST_Eb_Gb, OP_32BIT, sreg, dreg); }
inline void emit_test_m8_r8(x86code *&emitptr, x86_memref memref, UINT8 sreg)   { emit_op_modrm_mem(emitptr, OP_TEST_Eb_Gb, OP_32BIT, sreg, memref); }


//-------------------------------------------------
//  emit_arith_r16_*
//-------------------------------------------------

inline void emit_add_r16_imm(x86code *&emitptr, UINT8 dreg, UINT16 imm)         { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 0, dreg, imm); }
inline void emit_add_m16_imm(x86code *&emitptr, x86_memref memref, UINT16 imm)  { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 0, memref, imm); }
inline void emit_add_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_ADD_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_add_r16_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_ADD_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_add_m16_r16(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_ADD_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_or_r16_imm(x86code *&emitptr, UINT8 dreg, UINT16 imm)          { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 1, dreg, imm); }
inline void emit_or_m16_imm(x86code *&emitptr, x86_memref memref, UINT16 imm)   { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 1, memref, imm); }
inline void emit_or_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_OR_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_or_r16_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_OR_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_or_m16_r16(x86code *&emitptr, x86_memref memref, UINT8 sreg)   { emit_op_modrm_mem(emitptr, OP_OR_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_adc_r16_imm(x86code *&emitptr, UINT8 dreg, UINT16 imm)         { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 2, dreg, imm); }
inline void emit_adc_m16_imm(x86code *&emitptr, x86_memref memref, UINT16 imm)  { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 2, memref, imm); }
inline void emit_adc_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_ADC_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_adc_r16_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_ADC_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_adc_m16_r16(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_ADC_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_sbb_r16_imm(x86code *&emitptr, UINT8 dreg, UINT16 imm)         { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 3, dreg, imm); }
inline void emit_sbb_m16_imm(x86code *&emitptr, x86_memref memref, UINT16 imm)  { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 3, memref, imm); }
inline void emit_sbb_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_SBB_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_sbb_r16_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_SBB_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_sbb_m16_r16(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_SBB_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_and_r16_imm(x86code *&emitptr, UINT8 dreg, UINT16 imm)         { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 4, dreg, imm); }
inline void emit_and_m16_imm(x86code *&emitptr, x86_memref memref, UINT16 imm)  { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 4, memref, imm); }
inline void emit_and_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_AND_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_and_r16_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_AND_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_and_m16_r16(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_AND_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_sub_r16_imm(x86code *&emitptr, UINT8 dreg, UINT16 imm)         { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 5, dreg, imm); }
inline void emit_sub_m16_imm(x86code *&emitptr, x86_memref memref, UINT16 imm)  { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 5, memref, imm); }
inline void emit_sub_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_SUB_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_sub_r16_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_SUB_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_sub_m16_r16(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_SUB_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_xor_r16_imm(x86code *&emitptr, UINT8 dreg, UINT16 imm)         { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 6, dreg, imm); }
inline void emit_xor_m16_imm(x86code *&emitptr, x86_memref memref, UINT16 imm)  { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 6, memref, imm); }
inline void emit_xor_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_XOR_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_xor_r16_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_XOR_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_xor_m16_r16(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_XOR_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_cmp_r16_imm(x86code *&emitptr, UINT8 dreg, UINT16 imm)         { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 7, dreg, imm); }
inline void emit_cmp_m16_imm(x86code *&emitptr, x86_memref memref, UINT16 imm)  { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 7, memref, imm); }
inline void emit_cmp_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_CMP_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_cmp_r16_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_CMP_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_cmp_m16_r16(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_CMP_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_test_r16_imm(x86code *&emitptr, UINT8 dreg, UINT16 imm)        { emit_op_modrm_reg_imm16(emitptr, OP_G3_Ev, OP_16BIT, 0, dreg, imm); }
inline void emit_test_m16_imm(x86code *&emitptr, x86_memref memref, UINT16 imm) { emit_op_modrm_mem_imm16(emitptr, OP_G3_Ev, OP_16BIT, 0, memref, imm); }
inline void emit_test_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_TEST_Ev_Gv, OP_16BIT, sreg, dreg); }
inline void emit_test_m16_r16(x86code *&emitptr, x86_memref memref, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_TEST_Ev_Gv, OP_16BIT, sreg, memref); }


//-------------------------------------------------
//  emit_arith_r32_*
//-------------------------------------------------

inline void emit_add_r32_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 0, dreg, imm); }
inline void emit_add_m32_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 0, memref, imm); }
inline void emit_add_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_ADD_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_add_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_ADD_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_add_m32_r32(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_ADD_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_or_r32_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)          { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 1, dreg, imm); }
inline void emit_or_m32_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)   { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 1, memref, imm); }
inline void emit_or_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_OR_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_or_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_OR_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_or_m32_r32(x86code *&emitptr, x86_memref memref, UINT8 sreg)   { emit_op_modrm_mem(emitptr, OP_OR_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_adc_r32_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 2, dreg, imm); }
inline void emit_adc_m32_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 2, memref, imm); }
inline void emit_adc_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_ADC_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_adc_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_ADC_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_adc_m32_r32(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_ADC_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_sbb_r32_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 3, dreg, imm); }
inline void emit_sbb_m32_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 3, memref, imm); }
inline void emit_sbb_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_SBB_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_sbb_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_SBB_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_sbb_m32_r32(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_SBB_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_and_r32_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 4, dreg, imm); }
inline void emit_and_m32_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 4, memref, imm); }
inline void emit_and_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_AND_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_and_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_AND_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_and_m32_r32(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_AND_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_sub_r32_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 5, dreg, imm); }
inline void emit_sub_m32_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 5, memref, imm); }
inline void emit_sub_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_SUB_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_sub_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_SUB_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_sub_m32_r32(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_SUB_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_xor_r32_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 6, dreg, imm); }
inline void emit_xor_m32_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 6, memref, imm); }
inline void emit_xor_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_XOR_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_xor_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_XOR_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_xor_m32_r32(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_XOR_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_cmp_r32_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 7, dreg, imm); }
inline void emit_cmp_m32_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 7, memref, imm); }
inline void emit_cmp_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_CMP_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_cmp_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_CMP_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_cmp_m32_r32(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_CMP_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_test_r32_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)        { emit_op_modrm_reg_imm32(emitptr, OP_G3_Ev, OP_32BIT, 0, dreg, imm); }
inline void emit_test_m32_imm(x86code *&emitptr, x86_memref memref, UINT32 imm) { emit_op_modrm_mem_imm32(emitptr, OP_G3_Ev, OP_32BIT, 0, memref, imm); }
inline void emit_test_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_TEST_Ev_Gv, OP_32BIT, sreg, dreg); }
inline void emit_test_m32_r32(x86code *&emitptr, x86_memref memref, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_TEST_Ev_Gv, OP_32BIT, sreg, memref); }


//-------------------------------------------------
//  emit_arith_r64_*
//-------------------------------------------------

#if (X86EMIT_SIZE == 64)

inline void emit_add_r64_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 0, dreg, imm); }
inline void emit_add_m64_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 0, memref, imm); }
inline void emit_add_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_ADD_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_add_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_ADD_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_add_m64_r64(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_ADD_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_or_r64_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)          { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 1, dreg, imm); }
inline void emit_or_m64_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)   { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 1, memref, imm); }
inline void emit_or_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_OR_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_or_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_OR_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_or_m64_r64(x86code *&emitptr, x86_memref memref, UINT8 sreg)   { emit_op_modrm_mem(emitptr, OP_OR_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_adc_r64_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 2, dreg, imm); }
inline void emit_adc_m64_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 2, memref, imm); }
inline void emit_adc_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_ADC_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_adc_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_ADC_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_adc_m64_r64(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_ADC_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_sbb_r64_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 3, dreg, imm); }
inline void emit_sbb_m64_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 3, memref, imm); }
inline void emit_sbb_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_SBB_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_sbb_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_SBB_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_sbb_m64_r64(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_SBB_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_and_r64_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 4, dreg, imm); }
inline void emit_and_m64_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 4, memref, imm); }
inline void emit_and_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_AND_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_and_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_AND_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_and_m64_r64(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_AND_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_sub_r64_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 5, dreg, imm); }
inline void emit_sub_m64_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 5, memref, imm); }
inline void emit_sub_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_SUB_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_sub_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_SUB_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_sub_m64_r64(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_SUB_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_xor_r64_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 6, dreg, imm); }
inline void emit_xor_m64_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 6, memref, imm); }
inline void emit_xor_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_XOR_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_xor_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_XOR_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_xor_m64_r64(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_XOR_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_cmp_r64_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 7, dreg, imm); }
inline void emit_cmp_m64_imm(x86code *&emitptr, x86_memref memref, UINT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 7, memref, imm); }
inline void emit_cmp_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_CMP_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_cmp_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_CMP_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_cmp_m64_r64(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_CMP_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_test_r64_imm(x86code *&emitptr, UINT8 dreg, UINT32 imm)        { emit_op_modrm_reg_imm32(emitptr, OP_G3_Ev, OP_64BIT, 0, dreg, imm); }
inline void emit_test_m64_imm(x86code *&emitptr, x86_memref memref, UINT32 imm) { emit_op_modrm_mem_imm32(emitptr, OP_G3_Ev, OP_64BIT, 0, memref, imm); }
inline void emit_test_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_TEST_Ev_Gv, OP_64BIT, sreg, dreg); }
inline void emit_test_m64_r64(x86code *&emitptr, x86_memref memref, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_TEST_Ev_Gv, OP_64BIT, sreg, memref); }

#endif



//**************************************************************************
//  SHIFT EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_shift_reg_imm
//-------------------------------------------------

inline void emit_shift_reg_imm(x86code *&emitptr, UINT32 op1, UINT32 opn, UINT8 opsize, UINT8 opindex, UINT8 dreg, UINT8 imm)
{
	if (imm == 1)
		emit_op_modrm_reg(emitptr, op1, opsize, opindex, dreg);
	else
		emit_op_modrm_reg_imm8(emitptr, opn, opsize, opindex, dreg, imm);
}

inline void emit_shift_mem_imm(x86code *&emitptr, UINT32 op1, UINT32 opn, UINT8 opsize, UINT8 opindex, x86_memref memref, UINT8 imm)
{
	if (imm == 1)
		emit_op_modrm_mem(emitptr, op1, opsize, opindex, memref);
	else
		emit_op_modrm_mem_imm8(emitptr, opn, opsize, opindex, memref, imm);
}


//-------------------------------------------------
//  emit_shift_r8_*
//-------------------------------------------------

inline void emit_rol_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)           { emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 0, dreg, imm); }
inline void emit_rol_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 0, memref, imm); }
inline void emit_rol_r8_cl(x86code *&emitptr, UINT8 dreg)                       { emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 0, dreg); }
inline void emit_rol_m8_cl(x86code *&emitptr, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 0, memref); }

inline void emit_ror_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)           { emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 1, dreg, imm); }
inline void emit_ror_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 1, memref, imm); }
inline void emit_ror_r8_cl(x86code *&emitptr, UINT8 dreg)                       { emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 1, dreg); }
inline void emit_ror_m8_cl(x86code *&emitptr, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 1, memref); }

inline void emit_rcl_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)           { emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 2, dreg, imm); }
inline void emit_rcl_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 2, memref, imm); }
inline void emit_rcl_r8_cl(x86code *&emitptr, UINT8 dreg)                       { emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 2, dreg); }
inline void emit_rcl_m8_cl(x86code *&emitptr, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 2, memref); }

inline void emit_rcr_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)           { emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 3, dreg, imm); }
inline void emit_rcr_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 3, memref, imm); }
inline void emit_rcr_r8_cl(x86code *&emitptr, UINT8 dreg)                       { emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 3, dreg); }
inline void emit_rcr_m8_cl(x86code *&emitptr, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 3, memref); }

inline void emit_shl_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)           { emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 4, dreg, imm); }
inline void emit_shl_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 4, memref, imm); }
inline void emit_shl_r8_cl(x86code *&emitptr, UINT8 dreg)                       { emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 4, dreg); }
inline void emit_shl_m8_cl(x86code *&emitptr, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 4, memref); }

inline void emit_shr_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)           { emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 5, dreg, imm); }
inline void emit_shr_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 5, memref, imm); }
inline void emit_shr_r8_cl(x86code *&emitptr, UINT8 dreg)                       { emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 5, dreg); }
inline void emit_shr_m8_cl(x86code *&emitptr, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 5, memref); }

inline void emit_sar_r8_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)           { emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 7, dreg, imm); }
inline void emit_sar_m8_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)    { emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 7, memref, imm); }
inline void emit_sar_r8_cl(x86code *&emitptr, UINT8 dreg)                       { emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 7, dreg); }
inline void emit_sar_m8_cl(x86code *&emitptr, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 7, memref); }


//-------------------------------------------------
//  emit_shift_r16_*
//-------------------------------------------------

inline void emit_rol_r16_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 0, dreg, imm); }
inline void emit_rol_m16_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 0, memref, imm); }
inline void emit_rol_r16_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 0, dreg); }
inline void emit_rol_m16_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 0, memref); }

inline void emit_ror_r16_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 1, dreg, imm); }
inline void emit_ror_m16_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 1, memref, imm); }
inline void emit_ror_r16_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 1, dreg); }
inline void emit_ror_m16_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 1, memref); }

inline void emit_rcl_r16_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 2, dreg, imm); }
inline void emit_rcl_m16_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 2, memref, imm); }
inline void emit_rcl_r16_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 2, dreg); }
inline void emit_rcl_m16_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 2, memref); }

inline void emit_rcr_r16_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 3, dreg, imm); }
inline void emit_rcr_m16_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 3, memref, imm); }
inline void emit_rcr_r16_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 3, dreg); }
inline void emit_rcr_m16_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 3, memref); }

inline void emit_shl_r16_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 4, dreg, imm); }
inline void emit_shl_m16_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 4, memref, imm); }
inline void emit_shl_r16_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 4, dreg); }
inline void emit_shl_m16_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 4, memref); }

inline void emit_shr_r16_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 5, dreg, imm); }
inline void emit_shr_m16_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 5, memref, imm); }
inline void emit_shr_r16_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 5, dreg); }
inline void emit_shr_m16_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 5, memref); }

inline void emit_sar_r16_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 7, dreg, imm); }
inline void emit_sar_m16_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 7, memref, imm); }
inline void emit_sar_r16_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 7, dreg); }
inline void emit_sar_m16_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 7, memref); }


//-------------------------------------------------
//  emit_shift_r32_*
//-------------------------------------------------

inline void emit_rol_r32_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 0, dreg, imm); }
inline void emit_rol_m32_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 0, memref, imm); }
inline void emit_rol_r32_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 0, dreg); }
inline void emit_rol_m32_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 0, memref); }

inline void emit_ror_r32_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 1, dreg, imm); }
inline void emit_ror_m32_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 1, memref, imm); }
inline void emit_ror_r32_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 1, dreg); }
inline void emit_ror_m32_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 1, memref); }

inline void emit_rcl_r32_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 2, dreg, imm); }
inline void emit_rcl_m32_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 2, memref, imm); }
inline void emit_rcl_r32_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 2, dreg); }
inline void emit_rcl_m32_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 2, memref); }

inline void emit_rcr_r32_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 3, dreg, imm); }
inline void emit_rcr_m32_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 3, memref, imm); }
inline void emit_rcr_r32_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 3, dreg); }
inline void emit_rcr_m32_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 3, memref); }

inline void emit_shl_r32_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 4, dreg, imm); }
inline void emit_shl_m32_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 4, memref, imm); }
inline void emit_shl_r32_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 4, dreg); }
inline void emit_shl_m32_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 4, memref); }

inline void emit_shr_r32_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 5, dreg, imm); }
inline void emit_shr_m32_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 5, memref, imm); }
inline void emit_shr_r32_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 5, dreg); }
inline void emit_shr_m32_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 5, memref); }

inline void emit_sar_r32_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 7, dreg, imm); }
inline void emit_sar_m32_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 7, memref, imm); }
inline void emit_sar_r32_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 7, dreg); }
inline void emit_sar_m32_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 7, memref); }

inline void emit_shld_r32_r32_imm(x86code *&emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_SHLD_Ev_Gv_Ib, OP_32BIT, sreg, dreg, imm); }
inline void emit_shld_m32_r32_imm(x86code *&emitptr, x86_memref memref, UINT8 sreg, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_SHLD_Ev_Gv_Ib, OP_32BIT, sreg, memref, imm); }
inline void emit_shld_r32_r32_cl(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                     { emit_op_modrm_reg(emitptr, OP_SHLD_Ev_Gv_CL, OP_32BIT, sreg, dreg); }
inline void emit_shld_m32_r32_cl(x86code *&emitptr, x86_memref memref, UINT8 sreg)              { emit_op_modrm_mem(emitptr, OP_SHLD_Ev_Gv_CL, OP_32BIT, sreg, memref); }

inline void emit_shrd_r32_r32_imm(x86code *&emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_SHRD_Ev_Gv_Ib, OP_32BIT, sreg, dreg, imm); }
inline void emit_shrd_m32_r32_imm(x86code *&emitptr, x86_memref memref, UINT8 sreg, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_SHRD_Ev_Gv_Ib, OP_32BIT, sreg, memref, imm); }
inline void emit_shrd_r32_r32_cl(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                     { emit_op_modrm_reg(emitptr, OP_SHRD_Ev_Gv_CL, OP_32BIT, sreg, dreg); }
inline void emit_shrd_m32_r32_cl(x86code *&emitptr, x86_memref memref, UINT8 sreg)              { emit_op_modrm_mem(emitptr, OP_SHRD_Ev_Gv_CL, OP_32BIT, sreg, memref); }


//-------------------------------------------------
//  emit_shift_r64_*
//-------------------------------------------------

#if (X86EMIT_SIZE == 64)

inline void emit_rol_r64_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 0, dreg, imm); }
inline void emit_rol_m64_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 0, memref, imm); }
inline void emit_rol_r64_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 0, dreg); }
inline void emit_rol_m64_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 0, memref); }

inline void emit_ror_r64_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 1, dreg, imm); }
inline void emit_ror_m64_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 1, memref, imm); }
inline void emit_ror_r64_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 1, dreg); }
inline void emit_ror_m64_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 1, memref); }

inline void emit_rcl_r64_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 2, dreg, imm); }
inline void emit_rcl_m64_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 2, memref, imm); }
inline void emit_rcl_r64_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 2, dreg); }
inline void emit_rcl_m64_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 2, memref); }

inline void emit_rcr_r64_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 3, dreg, imm); }
inline void emit_rcr_m64_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 3, memref, imm); }
inline void emit_rcr_r64_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 3, dreg); }
inline void emit_rcr_m64_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 3, memref); }

inline void emit_shl_r64_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 4, dreg, imm); }
inline void emit_shl_m64_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 4, memref, imm); }
inline void emit_shl_r64_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 4, dreg); }
inline void emit_shl_m64_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 4, memref); }

inline void emit_shr_r64_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 5, dreg, imm); }
inline void emit_shr_m64_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 5, memref, imm); }
inline void emit_shr_r64_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 5, dreg); }
inline void emit_shr_m64_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 5, memref); }

inline void emit_sar_r64_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 7, dreg, imm); }
inline void emit_sar_m64_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 7, memref, imm); }
inline void emit_sar_r64_cl(x86code *&emitptr, UINT8 dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 7, dreg); }
inline void emit_sar_m64_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 7, memref); }

inline void emit_shld_r64_r64_imm(x86code *&emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_SHLD_Ev_Gv_Ib, OP_64BIT, sreg, dreg, imm); }
inline void emit_shld_m64_r64_imm(x86code *&emitptr, x86_memref memref, UINT8 sreg, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_SHLD_Ev_Gv_Ib, OP_64BIT, sreg, memref, imm); }
inline void emit_shld_r64_r64_cl(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                     { emit_op_modrm_reg(emitptr, OP_SHLD_Ev_Gv_CL, OP_64BIT, sreg, dreg); }
inline void emit_shld_m64_r64_cl(x86code *&emitptr, x86_memref memref, UINT8 sreg)              { emit_op_modrm_mem(emitptr, OP_SHLD_Ev_Gv_CL, OP_64BIT, sreg, memref); }

inline void emit_shrd_r64_r64_imm(x86code *&emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_SHRD_Ev_Gv_Ib, OP_64BIT, sreg, dreg, imm); }
inline void emit_shrd_m64_r64_imm(x86code *&emitptr, x86_memref memref, UINT8 sreg, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_SHRD_Ev_Gv_Ib, OP_64BIT, sreg, memref, imm); }
inline void emit_shrd_r64_r64_cl(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                     { emit_op_modrm_reg(emitptr, OP_SHRD_Ev_Gv_CL, OP_64BIT, sreg, dreg); }
inline void emit_shrd_m64_r64_cl(x86code *&emitptr, x86_memref memref, UINT8 sreg)              { emit_op_modrm_mem(emitptr, OP_SHRD_Ev_Gv_CL, OP_64BIT, sreg, memref); }

#endif



//**************************************************************************
//  GROUP3 EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_group3_r8_*
//-------------------------------------------------

inline void emit_not_r8(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 2, dreg); }
inline void emit_not_m8(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 2, memref); }

inline void emit_neg_r8(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 3, dreg); }
inline void emit_neg_m8(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 3, memref); }

inline void emit_mul_r8(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 4, dreg); }
inline void emit_mul_m8(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 4, memref); }

inline void emit_imul_r8(x86code *&emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 5, dreg); }
inline void emit_imul_m8(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 5, memref); }

inline void emit_div_r8(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 6, dreg); }
inline void emit_div_m8(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 6, memref); }

inline void emit_idiv_r8(x86code *&emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 7, dreg); }
inline void emit_idiv_m8(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 7, memref); }


//-------------------------------------------------
//  emit_group3_r16_*
//-------------------------------------------------

inline void emit_not_r16(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 2, dreg); }
inline void emit_not_m16(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 2, memref); }

inline void emit_neg_r16(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 3, dreg); }
inline void emit_neg_m16(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 3, memref); }

inline void emit_mul_r16(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 4, dreg); }
inline void emit_mul_m16(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 4, memref); }

inline void emit_imul_r16(x86code *&emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 5, dreg); }
inline void emit_imul_m16(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 5, memref); }

inline void emit_div_r16(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 6, dreg); }
inline void emit_div_m16(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 6, memref); }

inline void emit_idiv_r16(x86code *&emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 7, dreg); }
inline void emit_idiv_m16(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 7, memref); }


//-------------------------------------------------
//  emit_group3_r32_*
//-------------------------------------------------

inline void emit_not_r32(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 2, dreg); }
inline void emit_not_m32(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 2, memref); }

inline void emit_neg_r32(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 3, dreg); }
inline void emit_neg_m32(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 3, memref); }

inline void emit_mul_r32(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 4, dreg); }
inline void emit_mul_m32(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 4, memref); }

inline void emit_imul_r32(x86code *&emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 5, dreg); }
inline void emit_imul_m32(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 5, memref); }

inline void emit_div_r32(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 6, dreg); }
inline void emit_div_m32(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 6, memref); }

inline void emit_idiv_r32(x86code *&emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 7, dreg); }
inline void emit_idiv_m32(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 7, memref); }


//-------------------------------------------------
//  emit_group3_r64_*
//-------------------------------------------------

#if (X86EMIT_SIZE == 64)

inline void emit_not_r64(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 2, dreg); }
inline void emit_not_m64(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 2, memref); }

inline void emit_neg_r64(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 3, dreg); }
inline void emit_neg_m64(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 3, memref); }

inline void emit_mul_r64(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 4, dreg); }
inline void emit_mul_m64(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 4, memref); }

inline void emit_imul_r64(x86code *&emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 5, dreg); }
inline void emit_imul_m64(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 5, memref); }

inline void emit_div_r64(x86code *&emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 6, dreg); }
inline void emit_div_m64(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 6, memref); }

inline void emit_idiv_r64(x86code *&emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 7, dreg); }
inline void emit_idiv_m64(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 7, memref); }

#endif



//**************************************************************************
//  IMUL EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_imul_r16_*
//-------------------------------------------------

inline void emit_imul_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                        { emit_op_modrm_reg(emitptr, OP_IMUL_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_imul_r16_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_IMUL_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_imul_r16_r16_imm(x86code *&emitptr, UINT8 dreg, UINT8 sreg, INT16 imm)         { emit_op_modrm_reg_imm816(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_16BIT, dreg, sreg, imm); }
inline void emit_imul_r16_m16_imm(x86code *&emitptr, UINT8 dreg, x86_memref memref, INT16 imm)  { emit_op_modrm_mem_imm816(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_16BIT, dreg, memref, imm); }

inline void emit_imul_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                        { emit_op_modrm_reg(emitptr, OP_IMUL_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_imul_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_IMUL_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_imul_r32_r32_imm(x86code *&emitptr, UINT8 dreg, UINT8 sreg, INT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_32BIT, dreg, sreg, imm); }
inline void emit_imul_r32_m32_imm(x86code *&emitptr, UINT8 dreg, x86_memref memref, INT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_32BIT, dreg, memref, imm); }

#if (X86EMIT_SIZE == 64)

inline void emit_imul_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)                        { emit_op_modrm_reg(emitptr, OP_IMUL_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_imul_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_IMUL_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_imul_r64_r64_imm(x86code *&emitptr, UINT8 dreg, UINT8 sreg, INT32 imm)         { emit_op_modrm_reg_imm832(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_64BIT, dreg, sreg, imm); }
inline void emit_imul_r64_m64_imm(x86code *&emitptr, UINT8 dreg, x86_memref memref, INT32 imm)  { emit_op_modrm_mem_imm832(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_64BIT, dreg, memref, imm); }

#endif



//**************************************************************************
//  BIT OPERATION EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_bswap_*
//-------------------------------------------------

inline void emit_bswap_r32(x86code *&emitptr, UINT8 dreg)                       { emit_op_reg(emitptr, OP_BSWAP_EAX + (dreg & 7), OP_32BIT, dreg); }
inline void emit_bswap_r64(x86code *&emitptr, UINT8 dreg)                       { emit_op_reg(emitptr, OP_BSWAP_EAX + (dreg & 7), OP_64BIT, dreg); }


//-------------------------------------------------
//  emit_bsr/bsf_r16_*
//-------------------------------------------------

inline void emit_bsf_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_BSF_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_bsf_r16_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_BSF_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_bsr_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_BSR_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_bsr_r16_m16(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_BSR_Gv_Ev, OP_16BIT, dreg, memref); }

inline void emit_bsf_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_BSF_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_bsf_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_BSF_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_bsr_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_BSR_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_bsr_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_BSR_Gv_Ev, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_bsf_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_BSF_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_bsf_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_BSF_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_bsr_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_BSR_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_bsr_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_BSR_Gv_Ev, OP_64BIT, dreg, memref); }
#endif


//-------------------------------------------------
//  emit_bit_r16_*
//-------------------------------------------------

inline void emit_bt_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_BT_Ev_Gv, OP_16BIT, sreg, dreg); }
inline void emit_bt_m16_r16(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_BT_Ev_Gv, OP_16BIT, sreg, memref); }
inline void emit_bt_r16_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 4, dreg, imm); }
inline void emit_bt_m16_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 4, memref, imm); }

inline void emit_bts_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTS_Ev_Gv, OP_16BIT, sreg, dreg); }
inline void emit_bts_m16_r16(x86code *&emitptr, x86_memref memref, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTS_Ev_Gv, OP_16BIT, sreg, memref); }
inline void emit_bts_r16_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 5, dreg, imm); }
inline void emit_bts_m16_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 5, memref, imm); }

inline void emit_btr_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTR_Ev_Gv, OP_16BIT, sreg, dreg); }
inline void emit_btr_m16_r16(x86code *&emitptr, x86_memref memref, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTR_Ev_Gv, OP_16BIT, sreg, memref); }
inline void emit_btr_r16_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 6, dreg, imm); }
inline void emit_btr_m16_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 6, memref, imm); }

inline void emit_btc_r16_r16(x86code *&emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTC_Ev_Gv, OP_16BIT, sreg, dreg); }
inline void emit_btc_m16_r16(x86code *&emitptr, x86_memref memref, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTC_Ev_Gv, OP_16BIT, sreg, memref); }
inline void emit_btc_r16_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 7, dreg, imm); }
inline void emit_btc_m16_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 7, memref, imm); }


//-------------------------------------------------
//  emit_bit_r32_*
//-------------------------------------------------

inline void emit_bt_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_BT_Ev_Gv, OP_32BIT, sreg, dreg); }
inline void emit_bt_m32_r32(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_BT_Ev_Gv, OP_32BIT, sreg, memref); }
inline void emit_bt_r32_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 4, dreg, imm); }
inline void emit_bt_m32_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 4, memref, imm); }

inline void emit_bts_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTS_Ev_Gv, OP_32BIT, sreg, dreg); }
inline void emit_bts_m32_r32(x86code *&emitptr, x86_memref memref, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTS_Ev_Gv, OP_32BIT, sreg, memref); }
inline void emit_bts_r32_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 5, dreg, imm); }
inline void emit_bts_m32_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 5, memref, imm); }

inline void emit_btr_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTR_Ev_Gv, OP_32BIT, sreg, dreg); }
inline void emit_btr_m32_r32(x86code *&emitptr, x86_memref memref, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTR_Ev_Gv, OP_32BIT, sreg, memref); }
inline void emit_btr_r32_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 6, dreg, imm); }
inline void emit_btr_m32_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 6, memref, imm); }

inline void emit_btc_r32_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTC_Ev_Gv, OP_32BIT, sreg, dreg); }
inline void emit_btc_m32_r32(x86code *&emitptr, x86_memref memref, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTC_Ev_Gv, OP_32BIT, sreg, memref); }
inline void emit_btc_r32_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 7, dreg, imm); }
inline void emit_btc_m32_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 7, memref, imm); }


//-------------------------------------------------
//  emit_bit_r64_*
//-------------------------------------------------

#if (X86EMIT_SIZE == 64)

inline void emit_bt_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_BT_Ev_Gv, OP_64BIT, sreg, dreg); }
inline void emit_bt_m64_r64(x86code *&emitptr, x86_memref memref, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_BT_Ev_Gv, OP_64BIT, sreg, memref); }
inline void emit_bt_r64_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)          { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 4, dreg, imm); }
inline void emit_bt_m64_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)   { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 4, memref, imm); }

inline void emit_bts_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTS_Ev_Gv, OP_64BIT, sreg, dreg); }
inline void emit_bts_m64_r64(x86code *&emitptr, x86_memref memref, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTS_Ev_Gv, OP_64BIT, sreg, memref); }
inline void emit_bts_r64_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 5, dreg, imm); }
inline void emit_bts_m64_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 5, memref, imm); }

inline void emit_btr_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTR_Ev_Gv, OP_64BIT, sreg, dreg); }
inline void emit_btr_m64_r64(x86code *&emitptr, x86_memref memref, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTR_Ev_Gv, OP_64BIT, sreg, memref); }
inline void emit_btr_r64_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 6, dreg, imm); }
inline void emit_btr_m64_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 6, memref, imm); }

inline void emit_btc_r64_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTC_Ev_Gv, OP_64BIT, sreg, dreg); }
inline void emit_btc_m64_r64(x86code *&emitptr, x86_memref memref, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTC_Ev_Gv, OP_64BIT, sreg, memref); }
inline void emit_btc_r64_imm(x86code *&emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 7, dreg, imm); }
inline void emit_btc_m64_imm(x86code *&emitptr, x86_memref memref, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 7, memref, imm); }

#endif



//**************************************************************************
//  LEA EMITTERS
//**************************************************************************

inline void emit_lea_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_LEA_Gv_M, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_lea_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_LEA_Gv_M, OP_64BIT, dreg, memref); }
#endif



//**************************************************************************
//  SET EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_setcc_*
//-------------------------------------------------

inline void emit_setcc_r8(x86code *&emitptr, UINT8 cond, UINT8 dreg)            { emit_op_modrm_reg(emitptr, OP_SETCC_O_Eb + cond, OP_32BIT, 0, dreg); }
inline void emit_setcc_m8(x86code *&emitptr, UINT8 cond, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_SETCC_O_Eb + cond, OP_32BIT, 0, memref); }



//**************************************************************************
//  SIMPLE FPU EMITTERS
//**************************************************************************

#if (X86EMIT_SIZE == 32)

inline void emit_fnop(x86code *&emitptr)     { emit_op_simple(emitptr, OP_FNOP, OP_32BIT); }
inline void emit_fchs(x86code *&emitptr)     { emit_op_simple(emitptr, OP_FCHS, OP_32BIT); }
inline void emit_fabs(x86code *&emitptr)     { emit_op_simple(emitptr, OP_FABS, OP_32BIT); }
inline void emit_ftst(x86code *&emitptr)     { emit_op_simple(emitptr, OP_FTST, OP_32BIT); }
inline void emit_fxam(x86code *&emitptr)     { emit_op_simple(emitptr, OP_FXAM, OP_32BIT); }
inline void emit_fld1(x86code *&emitptr)     { emit_op_simple(emitptr, OP_FLD1, OP_32BIT); }
inline void emit_fldl2t(x86code *&emitptr)   { emit_op_simple(emitptr, OP_FLDL2T, OP_32BIT); }
inline void emit_fldl2e(x86code *&emitptr)   { emit_op_simple(emitptr, OP_FLDL2E, OP_32BIT); }
inline void emit_fldpi(x86code *&emitptr)    { emit_op_simple(emitptr, OP_FLDPI, OP_32BIT); }
inline void emit_fldlg2(x86code *&emitptr)   { emit_op_simple(emitptr, OP_FLDLG2, OP_32BIT); }
inline void emit_fldln2(x86code *&emitptr)   { emit_op_simple(emitptr, OP_FLDLN2, OP_32BIT); }
inline void emit_fldz(x86code *&emitptr)     { emit_op_simple(emitptr, OP_FLDZ, OP_32BIT); }
inline void emit_f2xm1(x86code *&emitptr)    { emit_op_simple(emitptr, OP_F2XM1, OP_32BIT); }
inline void emit_fyl2x(x86code *&emitptr)    { emit_op_simple(emitptr, OP_FYL2X, OP_32BIT); }
inline void emit_fptan(x86code *&emitptr)    { emit_op_simple(emitptr, OP_FPTAN, OP_32BIT); }
inline void emit_fpatan(x86code *&emitptr)   { emit_op_simple(emitptr, OP_FPATAN, OP_32BIT); }
inline void emit_fxtract(x86code *&emitptr)  { emit_op_simple(emitptr, OP_FXTRACT, OP_32BIT); }
inline void emit_fprem1(x86code *&emitptr)   { emit_op_simple(emitptr, OP_FPREM1, OP_32BIT); }
inline void emit_fdecstp(x86code *&emitptr)  { emit_op_simple(emitptr, OP_FDECSTP, OP_32BIT); }
inline void emit_fincstp(x86code *&emitptr)  { emit_op_simple(emitptr, OP_FINCSTP, OP_32BIT); }
inline void emit_fprem(x86code *&emitptr)    { emit_op_simple(emitptr, OP_FPREM, OP_32BIT); }
inline void emit_fyl2xp1(x86code *&emitptr)  { emit_op_simple(emitptr, OP_FYL2XP1, OP_32BIT); }
inline void emit_fsqrt(x86code *&emitptr)    { emit_op_simple(emitptr, OP_FSQRT, OP_32BIT); }
inline void emit_fsincos(x86code *&emitptr)  { emit_op_simple(emitptr, OP_FSINCOS, OP_32BIT); }
inline void emit_frndint(x86code *&emitptr)  { emit_op_simple(emitptr, OP_FRNDINT, OP_32BIT); }
inline void emit_fscale(x86code *&emitptr)   { emit_op_simple(emitptr, OP_FSCALE, OP_32BIT); }
inline void emit_fsin(x86code *&emitptr)     { emit_op_simple(emitptr, OP_FSIN, OP_32BIT); }
inline void emit_fcos(x86code *&emitptr)     { emit_op_simple(emitptr, OP_FCOS, OP_32BIT); }
inline void emit_fucompp(x86code *&emitptr)  { emit_op_simple(emitptr, OP_FUCOMPP, OP_32BIT); }
inline void emit_fclex(x86code *&emitptr)    { emit_op_simple(emitptr, OP_FCLEX, OP_32BIT); }
inline void emit_finit(x86code *&emitptr)    { emit_op_simple(emitptr, OP_FINIT, OP_32BIT); }
inline void emit_fcompp(x86code *&emitptr)   { emit_op_simple(emitptr, OP_FCOMPP, OP_32BIT); }
inline void emit_fstsw_ax(x86code *&emitptr) { emit_op_simple(emitptr, OP_FSTSW_AX, OP_32BIT); }

#endif



//**************************************************************************
//  REGISTER-BASED FPU EMITTERS
//**************************************************************************

#if (X86EMIT_SIZE == 32)

inline void emit_ffree_stn(x86code *&emitptr, UINT8 reg)        { emit_op_simple(emitptr, OP_FFREE_STn + reg, OP_32BIT); }
inline void emit_fst_stn(x86code *&emitptr, UINT8 reg)          { emit_op_simple(emitptr, OP_FST_STn + reg, OP_32BIT); }
inline void emit_fstp_stn(x86code *&emitptr, UINT8 reg)         { emit_op_simple(emitptr, OP_FSTP_STn + reg, OP_32BIT); }
inline void emit_fucomp_stn(x86code *&emitptr, UINT8 reg)       { emit_op_simple(emitptr, OP_FUCOMP_STn + reg, OP_32BIT); }

inline void emit_fadd_st0_stn(x86code *&emitptr, UINT8 reg)     { emit_op_simple(emitptr, OP_FADD_ST0_STn + reg, OP_32BIT); }
inline void emit_fmul_st0_stn(x86code *&emitptr, UINT8 reg)     { emit_op_simple(emitptr, OP_FMUL_ST0_STn + reg, OP_32BIT); }
inline void emit_fcom_st0_stn(x86code *&emitptr, UINT8 reg)     { emit_op_simple(emitptr, OP_FCOM_ST0_STn + reg, OP_32BIT); }
inline void emit_fcomp_st0_stn(x86code *&emitptr, UINT8 reg)    { emit_op_simple(emitptr, OP_FCOMP_ST0_STn + reg, OP_32BIT); }
inline void emit_fsub_st0_stn(x86code *&emitptr, UINT8 reg)     { emit_op_simple(emitptr, OP_FSUB_ST0_STn + reg, OP_32BIT); }
inline void emit_fsubr_st0_stn(x86code *&emitptr, UINT8 reg)    { emit_op_simple(emitptr, OP_FSUBR_ST0_STn + reg, OP_32BIT); }
inline void emit_fdiv_st0_stn(x86code *&emitptr, UINT8 reg)     { emit_op_simple(emitptr, OP_FDIV_ST0_STn + reg, OP_32BIT); }
inline void emit_fdivr_st0_stn(x86code *&emitptr, UINT8 reg)    { emit_op_simple(emitptr, OP_FDIVR_ST0_STn + reg, OP_32BIT); }
inline void emit_fld_st0_stn(x86code *&emitptr, UINT8 reg)      { emit_op_simple(emitptr, OP_FLD_ST0_STn + reg, OP_32BIT); }
inline void emit_fxch_st0_stn(x86code *&emitptr, UINT8 reg)     { emit_op_simple(emitptr, OP_FXCH_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmovb_st0_stn(x86code *&emitptr, UINT8 reg)   { emit_op_simple(emitptr, OP_FCMOVB_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmove_st0_stn(x86code *&emitptr, UINT8 reg)   { emit_op_simple(emitptr, OP_FCMOVE_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmovbe_st0_stn(x86code *&emitptr, UINT8 reg)  { emit_op_simple(emitptr, OP_FCMOVBE_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmovu_st0_stn(x86code *&emitptr, UINT8 reg)   { emit_op_simple(emitptr, OP_FCMOVU_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmovnb_st0_stn(x86code *&emitptr, UINT8 reg)  { emit_op_simple(emitptr, OP_FCMOVNB_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmovne_st0_stn(x86code *&emitptr, UINT8 reg)  { emit_op_simple(emitptr, OP_FCMOVNE_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmovnbe_st0_stn(x86code *&emitptr, UINT8 reg) { emit_op_simple(emitptr, OP_FCMOVNBE_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmovnu_st0_stn(x86code *&emitptr, UINT8 reg)  { emit_op_simple(emitptr, OP_FCMOVNU_ST0_STn + reg, OP_32BIT); }
inline void emit_fucomi_st0_stn(x86code *&emitptr, UINT8 reg)   { emit_op_simple(emitptr, OP_FUCOMI_ST0_STn + reg, OP_32BIT); }
inline void emit_fcomi_st0_stn(x86code *&emitptr, UINT8 reg)    { emit_op_simple(emitptr, OP_FCOMI_ST0_STn + reg, OP_32BIT); }
inline void emit_fcomip_st0_stn(x86code *&emitptr, UINT8 reg)   { emit_op_simple(emitptr, OP_FCOMIP_ST0_STn + reg, OP_32BIT); }

inline void emit_fadd_stn_st0(x86code *&emitptr, UINT8 reg)     { emit_op_simple(emitptr, OP_FADD_STn_ST0 + reg, OP_32BIT); }
inline void emit_fmul_stn_st0(x86code *&emitptr, UINT8 reg)     { emit_op_simple(emitptr, OP_FMUL_STn_ST0 + reg, OP_32BIT); }
inline void emit_fsubr_stn_st0(x86code *&emitptr, UINT8 reg)    { emit_op_simple(emitptr, OP_FSUBR_STn_ST0 + reg, OP_32BIT); }
inline void emit_fsub_stn_st0(x86code *&emitptr, UINT8 reg)     { emit_op_simple(emitptr, OP_FSUB_STn_ST0 + reg, OP_32BIT); }
inline void emit_fdivr_stn_st0(x86code *&emitptr, UINT8 reg)    { emit_op_simple(emitptr, OP_FDIVR_STn_ST0 + reg, OP_32BIT); }
inline void emit_fdiv_stn_st0(x86code *&emitptr, UINT8 reg)     { emit_op_simple(emitptr, OP_FDIV_STn_ST0 + reg, OP_32BIT); }
inline void emit_fucom_stn_st0(x86code *&emitptr, UINT8 reg)    { emit_op_simple(emitptr, OP_FUCOM_STn_ST0 + reg, OP_32BIT); }
inline void emit_faddp_stn_st0(x86code *&emitptr, UINT8 reg)    { emit_op_simple(emitptr, OP_FADDP_STn_ST0 + reg, OP_32BIT); }
inline void emit_fmulp_stn_st0(x86code *&emitptr, UINT8 reg)    { emit_op_simple(emitptr, OP_FMULP_STn_ST0 + reg, OP_32BIT); }
inline void emit_fsubrp_stn_st0(x86code *&emitptr, UINT8 reg)   { emit_op_simple(emitptr, OP_FSUBRP_STn_ST0 + reg, OP_32BIT); }
inline void emit_fsubp_stn_st0(x86code *&emitptr, UINT8 reg)    { emit_op_simple(emitptr, OP_FSUBP_STn_ST0 + reg, OP_32BIT); }
inline void emit_fdivrp_stn_st0(x86code *&emitptr, UINT8 reg)   { emit_op_simple(emitptr, OP_FDIVRP_STn_ST0 + reg, OP_32BIT); }
inline void emit_fdivp_stn_st0(x86code *&emitptr, UINT8 reg)    { emit_op_simple(emitptr, OP_FDIVP_STn_ST0 + reg, OP_32BIT); }

inline void emit_faddp(x86code *&emitptr)                       { emit_faddp_stn_st0(emitptr, 1); }
inline void emit_fmulp(x86code *&emitptr)                       { emit_fmulp_stn_st0(emitptr, 1); }
inline void emit_fsubrp(x86code *&emitptr)                      { emit_fsubrp_stn_st0(emitptr, 1); }
inline void emit_fsubp(x86code *&emitptr)                       { emit_fsubp_stn_st0(emitptr, 1); }
inline void emit_fdivrp(x86code *&emitptr)                      { emit_fdivrp_stn_st0(emitptr, 1); }
inline void emit_fdivp(x86code *&emitptr)                       { emit_fdivp_stn_st0(emitptr, 1); }

#endif



//**************************************************************************
//  MEMORY FPU EMITTERS
//**************************************************************************

#if (X86EMIT_SIZE == 32)

inline void emit_fadd_m32(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 0, memref); }
inline void emit_fmul_m32(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 1, memref); }
inline void emit_fcom_m32(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 2, memref); }
inline void emit_fcomp_m32(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 3, memref); }
inline void emit_fsub_m32(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 4, memref); }
inline void emit_fsubr_m32(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 5, memref); }
inline void emit_fdiv_m32(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 6, memref); }
inline void emit_fdivr_m32(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 7, memref); }

inline void emit_fld_m32(x86code *&emitptr, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_ESC_D9, OP_32BIT, 0, memref); }
inline void emit_fst_m32(x86code *&emitptr, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_ESC_D9, OP_32BIT, 2, memref); }
inline void emit_fstp_m32(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_D9, OP_32BIT, 3, memref); }
inline void emit_fldenv_m(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_D9, OP_32BIT, 4, memref); }
inline void emit_fldcw_m16(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_D9, OP_32BIT, 5, memref); }
inline void emit_fstenv_m(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_D9, OP_32BIT, 6, memref); }
inline void emit_fstcw_m16(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_D9, OP_32BIT, 7, memref); }

inline void emit_fiadd_m32(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 0, memref); }
inline void emit_fimul_m32(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 1, memref); }
inline void emit_ficom_m32(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 2, memref); }
inline void emit_ficomp_m32(x86code *&emitptr, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 3, memref); }
inline void emit_fisub_m32(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 4, memref); }
inline void emit_fisubr_m32(x86code *&emitptr, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 5, memref); }
inline void emit_fidiv_m32(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 6, memref); }
inline void emit_fidivr_m32(x86code *&emitptr, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 7, memref); }

inline void emit_fild_m32(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_DB, OP_32BIT, 0, memref); }
inline void emit_fisttp_m32(x86code *&emitptr, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_ESC_DB, OP_32BIT, 1, memref); }
inline void emit_fist_m32(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_DB, OP_32BIT, 2, memref); }
inline void emit_fistp_m32(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DB, OP_32BIT, 3, memref); }
inline void emit_fld_m80(x86code *&emitptr, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_ESC_DB, OP_32BIT, 5, memref); }
inline void emit_fstp_m80(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_DB, OP_32BIT, 7, memref); }

inline void emit_fadd_m64(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 0, memref); }
inline void emit_fmul_m64(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 1, memref); }
inline void emit_fcom_m64(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 2, memref); }
inline void emit_fcomp_m64(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 3, memref); }
inline void emit_fsub_m64(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 4, memref); }
inline void emit_fsubr_m64(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 5, memref); }
inline void emit_fdiv_m64(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 6, memref); }
inline void emit_fdivr_m64(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 7, memref); }

inline void emit_fld_m64(x86code *&emitptr, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_ESC_DD, OP_32BIT, 0, memref); }
inline void emit_fisttp_m64(x86code *&emitptr, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_ESC_DD, OP_32BIT, 1, memref); }
inline void emit_fst_m64(x86code *&emitptr, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_ESC_DD, OP_32BIT, 2, memref); }
inline void emit_fstp_m64(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_DD, OP_32BIT, 3, memref); }
inline void emit_frstor_m(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_DD, OP_32BIT, 4, memref); }
inline void emit_fsave_m(x86code *&emitptr, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_ESC_DD, OP_32BIT, 6, memref); }
inline void emit_fstsw_m16(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DD, OP_32BIT, 7, memref); }

inline void emit_fiadd_m16(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 0, memref); }
inline void emit_fimul_m16(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 1, memref); }
inline void emit_ficom_m16(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 2, memref); }
inline void emit_ficomp_m16(x86code *&emitptr, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 3, memref); }
inline void emit_fisub_m16(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 4, memref); }
inline void emit_fisubr_m16(x86code *&emitptr, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 5, memref); }
inline void emit_fidiv_m16(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 6, memref); }
inline void emit_fidivr_m16(x86code *&emitptr, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 7, memref); }

inline void emit_fild_m16(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 0, memref); }
inline void emit_fisttp_m16(x86code *&emitptr, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 1, memref); }
inline void emit_fist_m16(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 2, memref); }
inline void emit_fistp_m16(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 3, memref); }
inline void emit_fbld_m80(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 4, memref); }
inline void emit_fild_m64(x86code *&emitptr, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 5, memref); }
inline void emit_fbstp_m80(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 6, memref); }
inline void emit_fistp_m64(x86code *&emitptr, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 7, memref); }

#endif



//**************************************************************************
//  GROUP16 EMITTERS
//**************************************************************************

inline void emit_ldmxcsr_m32(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G16, OP_32BIT, 2, memref); }
inline void emit_stmxcsr_m32(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G16, OP_32BIT, 3, memref); }



//**************************************************************************
//  MISC SSE EMITTERS
//**************************************************************************

inline void emit_movd_r128_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)               { emit_op_modrm_reg(emitptr, OP_MOVD_Vd_Ed, OP_32BIT, dreg, sreg); }
inline void emit_movd_r128_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)        { emit_op_modrm_mem(emitptr, OP_MOVD_Vd_Ed, OP_32BIT, dreg, memref); }
inline void emit_movd_r32_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)               { emit_op_modrm_reg(emitptr, OP_MOVD_Ed_Vd, OP_32BIT, sreg, dreg); }
inline void emit_movd_m32_r128(x86code *&emitptr, x86_memref memref, UINT8 sreg)        { emit_op_modrm_mem(emitptr, OP_MOVD_Ed_Vd, OP_32BIT, sreg, memref); }

#if (X86EMIT_SIZE == 64)

inline void emit_movq_r128_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)               { emit_op_modrm_reg(emitptr, OP_MOVD_Vd_Ed, OP_64BIT, dreg, sreg); }
inline void emit_movq_r128_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)        { emit_op_modrm_mem(emitptr, OP_MOVD_Vd_Ed, OP_64BIT, dreg, memref); }
inline void emit_movq_r64_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)               { emit_op_modrm_reg(emitptr, OP_MOVD_Ed_Vd, OP_64BIT, sreg, dreg); }
inline void emit_movq_m64_r128(x86code *&emitptr, x86_memref memref, UINT8 sreg)        { emit_op_modrm_mem(emitptr, OP_MOVD_Ed_Vd, OP_64BIT, sreg, memref); }

#endif



//**************************************************************************
//  SSE SCALAR SINGLE EMITTERS
//**************************************************************************

inline void emit_movss_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_MOVSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_movss_r128_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_MOVSS_Vss_Wss, OP_32BIT, dreg, memref); }
inline void emit_movss_m32_r128(x86code *&emitptr, x86_memref memref, UINT8 sreg)       { emit_op_modrm_mem(emitptr, OP_MOVSS_Wss_Vss, OP_32BIT, sreg, memref); }

inline void emit_addss_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_ADDSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_addss_r128_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_ADDSS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_subss_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_SUBSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_subss_r128_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_SUBSS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_mulss_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_MULSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_mulss_r128_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_MULSS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_divss_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_DIVSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_divss_r128_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_DIVSS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_rcpss_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_RCPSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_rcpss_r128_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_RCPSS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_sqrtss_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)            { emit_op_modrm_reg(emitptr, OP_SQRTSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_sqrtss_r128_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_SQRTSS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_rsqrtss_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_RSQRTSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_rsqrtss_r128_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_RSQRTSS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_comiss_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)            { emit_op_modrm_reg(emitptr, OP_COMISS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_comiss_r128_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_COMISS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_ucomiss_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_UCOMISS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_ucomiss_r128_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_UCOMISS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_cvtsi2ss_r128_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSI2SS_Vss_Ed, OP_32BIT, dreg, sreg); }
inline void emit_cvtsi2ss_r128_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTSI2SS_Vss_Ed, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_cvtsi2ss_r128_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSI2SS_Vss_Ed, OP_64BIT, dreg, sreg); }
inline void emit_cvtsi2ss_r128_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTSI2SS_Vss_Ed, OP_64BIT, dreg, memref); }
#endif

inline void emit_cvtsd2ss_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CVTSD2SS_Vss_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_cvtsd2ss_r128_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTSD2SS_Vss_Wsd, OP_32BIT, dreg, memref); }

inline void emit_cvtss2si_r32_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSS2SI_Gd_Wss, OP_32BIT, dreg, sreg); }
inline void emit_cvtss2si_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_CVTSS2SI_Gd_Wss, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_cvtss2si_r64_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSS2SI_Gd_Wss, OP_64BIT, dreg, sreg); }
inline void emit_cvtss2si_r64_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_CVTSS2SI_Gd_Wss, OP_64BIT, dreg, memref); }
#endif

inline void emit_cvttss2si_r32_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CVTTSS2SI_Gd_Wss, OP_32BIT, dreg, sreg); }
inline void emit_cvttss2si_r32_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTTSS2SI_Gd_Wss, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_cvttss2si_r64_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CVTTSS2SI_Gd_Wss, OP_64BIT, dreg, sreg); }
inline void emit_cvttss2si_r64_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTTSS2SI_Gd_Wss, OP_64BIT, dreg, memref); }
#endif

inline void emit_roundss_r128_r128_imm(x86code *&emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)        { emit_op_modrm_reg(emitptr, OP_ROUNDSS_Vss_Wss_Ib, OP_32BIT, dreg, sreg); emit_byte(emitptr, imm); }
inline void emit_roundss_r128_m32_imm(x86code *&emitptr, UINT8 dreg, x86_memref memref, UINT8 imm)  { emit_op_modrm_mem(emitptr, OP_ROUNDSS_Vss_Wss_Ib, OP_32BIT, dreg, memref); emit_byte(emitptr, imm); }



//**************************************************************************
//  SSE PACKED SINGLE EMITTERS
//**************************************************************************

inline void emit_movps_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_MOVAPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_movps_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_MOVAPS_Vps_Wps, OP_32BIT, dreg, memref); }
inline void emit_movps_m128_r128(x86code *&emitptr, x86_memref memref, UINT8 sreg)      { emit_op_modrm_mem(emitptr, OP_MOVAPS_Wps_Vps, OP_32BIT, sreg, memref); }

inline void emit_addps_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_ADDPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_addps_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_ADDPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_subps_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_SUBPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_subps_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_SUBPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_mulps_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_MULPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_mulps_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_MULPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_divps_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_DIVPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_divps_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_DIVPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_rcpps_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_RCPPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_rcpps_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_RCPPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_sqrtps_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)            { emit_op_modrm_reg(emitptr, OP_SQRTPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_sqrtps_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_SQRTPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_rsqrtps_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_RSQRTPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_rsqrtps_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_RSQRTPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_andps_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_ANDPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_andps_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_ANDPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_andnps_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)            { emit_op_modrm_reg(emitptr, OP_ANDNPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_andnps_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ANDNPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_orps_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)              { emit_op_modrm_reg(emitptr, OP_ORPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_orps_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_ORPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_xorps_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_XORPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_xorps_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_XORPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_cvtdq2ps_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CVTDQ2PS_Vps_Wdq, OP_32BIT, dreg, sreg); }
inline void emit_cvtdq2ps_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CVTDQ2PS_Vps_Wdq, OP_32BIT, dreg, memref); }

inline void emit_cvtpd2ps_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CVTPD2PS_Vps_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_cvtpd2ps_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CVTPD2PS_Vps_Wpd, OP_32BIT, dreg, memref); }

inline void emit_cvtps2dq_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CVTPS2DQ_Vdq_Wps, OP_32BIT, dreg, sreg); }
inline void emit_cvtps2dq_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CVTPS2DQ_Vdq_Wps, OP_32BIT, dreg, memref); }

inline void emit_cvttps2dq_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_CVTTPS2DQ_Vdq_Wps, OP_32BIT, dreg, sreg); }
inline void emit_cvttps2dq_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_CVTTPS2DQ_Vdq_Wps, OP_32BIT, dreg, memref); }

inline void emit_roundps_r128_r128_imm(x86code *&emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)        { emit_op_modrm_reg(emitptr, OP_ROUNDPS_Vdq_Wdq_Ib, OP_32BIT, dreg, sreg); emit_byte(emitptr, imm); }
inline void emit_roundps_r128_m128_imm(x86code *&emitptr, UINT8 dreg, x86_memref memref, UINT8 imm) { emit_op_modrm_mem(emitptr, OP_ROUNDPS_Vdq_Wdq_Ib, OP_32BIT, dreg, memref); emit_byte(emitptr, imm); }



//**************************************************************************
//  SSE SCALAR DOUBLE EMITTERS
//**************************************************************************

inline void emit_movsd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_MOVSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_movsd_r128_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_MOVSD_Vsd_Wsd, OP_32BIT, dreg, memref); }
inline void emit_movsd_m64_r128(x86code *&emitptr, x86_memref memref, UINT8 sreg)       { emit_op_modrm_mem(emitptr, OP_MOVSD_Wsd_Vsd, OP_32BIT, sreg, memref); }

inline void emit_addsd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_ADDSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_addsd_r128_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_ADDSD_Vsd_Wsd, OP_32BIT, dreg, memref); }

inline void emit_subsd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_SUBSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_subsd_r128_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_SUBSD_Vsd_Wsd, OP_32BIT, dreg, memref); }

inline void emit_mulsd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_MULSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_mulsd_r128_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_MULSD_Vsd_Wsd, OP_32BIT, dreg, memref); }

inline void emit_divsd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_DIVSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_divsd_r128_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_DIVSD_Vsd_Wsd, OP_32BIT, dreg, memref); }

inline void emit_sqrtsd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)            { emit_op_modrm_reg(emitptr, OP_SQRTSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_sqrtsd_r128_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_SQRTSD_Vsd_Wsd, OP_32BIT, dreg, memref); }

inline void emit_comisd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)            { emit_op_modrm_reg(emitptr, OP_COMISD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_comisd_r128_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_COMISD_Vsd_Wsd, OP_32BIT, dreg, memref); }

inline void emit_ucomisd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_UCOMISD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_ucomisd_r128_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_UCOMISD_Vsd_Wsd, OP_32BIT, dreg, memref); }

inline void emit_cvtsi2sd_r128_r32(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSI2SD_Vsd_Ed, OP_32BIT, dreg, sreg); }
inline void emit_cvtsi2sd_r128_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTSI2SD_Vsd_Ed, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_cvtsi2sd_r128_r64(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSI2SD_Vsd_Ed, OP_64BIT, dreg, sreg); }
inline void emit_cvtsi2sd_r128_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTSI2SD_Vsd_Ed, OP_64BIT, dreg, memref); }
#endif

inline void emit_cvtss2sd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CVTSS2SD_Vsd_Wss, OP_32BIT, dreg, sreg); }
inline void emit_cvtss2sd_r128_m32(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTSS2SD_Vsd_Wss, OP_32BIT, dreg, memref); }

inline void emit_cvtsd2si_r32_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSD2SI_Gd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_cvtsd2si_r32_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_CVTSD2SI_Gd_Wsd, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_cvtsd2si_r64_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSD2SI_Gd_Wsd, OP_64BIT, dreg, sreg); }
inline void emit_cvtsd2si_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_CVTSD2SI_Gd_Wsd, OP_64BIT, dreg, memref); }
#endif

inline void emit_cvttsd2si_r32_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CVTTSD2SI_Gd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_cvttsd2si_r32_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTTSD2SI_Gd_Wsd, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_cvttsd2si_r64_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CVTTSD2SI_Gd_Wsd, OP_64BIT, dreg, sreg); }
inline void emit_cvttsd2si_r64_m64(x86code *&emitptr, UINT8 dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTTSD2SI_Gd_Wsd, OP_64BIT, dreg, memref); }
#endif

inline void emit_roundsd_r128_r128_imm(x86code *&emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)        { emit_op_modrm_reg(emitptr, OP_ROUNDSD_Vsd_Wsd_Ib, OP_32BIT, dreg, sreg); emit_byte(emitptr, imm); }
inline void emit_roundsd_r128_m64_imm(x86code *&emitptr, UINT8 dreg, x86_memref memref, UINT8 imm)  { emit_op_modrm_mem(emitptr, OP_ROUNDSD_Vsd_Wsd_Ib, OP_32BIT, dreg, memref); emit_byte(emitptr, imm); }



//**************************************************************************
//  SSE PACKED DOUBLE EMITTERS
//**************************************************************************

inline void emit_movpd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_MOVAPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_movpd_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_MOVAPD_Vpd_Wpd, OP_32BIT, dreg, memref); }
inline void emit_movpd_m128_r128(x86code *&emitptr, x86_memref memref, UINT8 sreg)      { emit_op_modrm_mem(emitptr, OP_MOVAPD_Wpd_Vpd, OP_32BIT, sreg, memref); }

inline void emit_addpd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_ADDPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_addpd_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_ADDPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_subpd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_SUBPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_subpd_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_SUBPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_mulpd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_MULPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_mulpd_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_MULPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_divpd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_DIVPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_divpd_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_DIVPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_sqrtpd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)            { emit_op_modrm_reg(emitptr, OP_SQRTPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_sqrtpd_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_SQRTPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_andpd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_ANDPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_andpd_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_ANDPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_andnpd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)            { emit_op_modrm_reg(emitptr, OP_ANDNPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_andnpd_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ANDNPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_orpd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)              { emit_op_modrm_reg(emitptr, OP_ORPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_orpd_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_ORPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_xorpd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)             { emit_op_modrm_reg(emitptr, OP_XORPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_xorpd_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_XORPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_cvtdq2pd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CVTDQ2PD_Vpd_Wq, OP_32BIT, dreg, sreg); }
inline void emit_cvtdq2pd_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CVTDQ2PD_Vpd_Wq, OP_32BIT, dreg, memref); }

inline void emit_cvtps2pd_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CVTPS2PD_Vpd_Wq, OP_32BIT, dreg, sreg); }
inline void emit_cvtps2pd_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CVTPS2PD_Vpd_Wq, OP_32BIT, dreg, memref); }

inline void emit_cvtpd2dq_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)          { emit_op_modrm_reg(emitptr, OP_CVTPD2DQ_Vdq_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_cvtpd2dq_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CVTPD2DQ_Vdq_Wpd, OP_32BIT, dreg, memref); }

inline void emit_cvttpd2dq_r128_r128(x86code *&emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_CVTTPD2DQ_Vdq_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_cvttpd2dq_r128_m128(x86code *&emitptr, UINT8 dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_CVTTPD2DQ_Vdq_Wpd, OP_32BIT, dreg, memref); }

inline void emit_roundpd_r128_r128_imm(x86code *&emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)        { emit_op_modrm_reg(emitptr, OP_ROUNDPD_Vdq_Wdq_Ib, OP_32BIT, dreg, sreg); emit_byte(emitptr, imm); }
inline void emit_roundpd_r128_m128_imm(x86code *&emitptr, UINT8 dreg, x86_memref memref, UINT8 imm) { emit_op_modrm_mem(emitptr, OP_ROUNDPD_Vdq_Wdq_Ib, OP_32BIT, dreg, memref); emit_byte(emitptr, imm); }

}

#undef X86EMIT_SIZE

#endif
