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
typedef uint8_t       x86code;

// this structure tracks information about a link
struct emit_link
{
	x86code *       target;
	uint8_t           size;
};

// structure for describing memory references
class x86_memref
{
public:
	x86_memref(uint8_t basereg, uint8_t indreg, uint8_t scale, int32_t disp)
		: m_base(basereg),
			m_index(indreg),
			m_scale(scale),
			m_disp(disp) { }

	x86_memref operator+(int32_t offset) { return x86_memref(m_base, m_index, m_scale, m_disp + offset); }

	uint8_t m_base;
	uint8_t m_index;
	uint8_t m_scale;
	int32_t m_disp;
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
const uint8_t OP_16BIT        = 0x10;
const uint8_t OP_32BIT        = 0x00;
const uint8_t OP_64BIT        = 0x08;

// 16 registers on x64, only 8 on x86
#if (X86EMIT_SIZE == 64)
const int REG_MAX           = 16;
#else
const int REG_MAX           = 8;
#endif

// invalid register index for "none"
const uint8_t REG_NONE        = REG_MAX;

// 8-bit registers -- note that we assume a flat model for 64-bit
const uint8_t REG_AL          = 0;
const uint8_t REG_CL          = 1;
const uint8_t REG_DL          = 2;
const uint8_t REG_BL          = 3;
#if (X86EMIT_SIZE == 32)
const uint8_t REG_AH          = 4;
const uint8_t REG_CH          = 5;
const uint8_t REG_DH          = 6;
const uint8_t REG_BH          = 7;
#else
const uint8_t REG_SPL         = 4;
const uint8_t REG_BPL         = 5;
const uint8_t REG_SIL         = 6;
const uint8_t REG_DIL         = 7;
const uint8_t REG_R8L         = 8;
const uint8_t REG_R9L         = 9;
const uint8_t REG_R10L        = 10;
const uint8_t REG_R11L        = 11;
const uint8_t REG_R12L        = 12;
const uint8_t REG_R13L        = 13;
const uint8_t REG_R14L        = 14;
const uint8_t REG_R15L        = 15;
#endif

// 16-bit registers
const uint8_t REG_AX          = 0;
const uint8_t REG_CX          = 1;
const uint8_t REG_DX          = 2;
const uint8_t REG_BX          = 3;
const uint8_t REG_SP          = 4;
const uint8_t REG_BP          = 5;
const uint8_t REG_SI          = 6;
const uint8_t REG_DI          = 7;
#if (X86EMIT_SIZE == 64)
const uint8_t REG_R8W         = 8;
const uint8_t REG_R9W         = 9;
const uint8_t REG_R10W        = 10;
const uint8_t REG_R11W        = 11;
const uint8_t REG_R12W        = 12;
const uint8_t REG_R13W        = 13;
const uint8_t REG_R14W        = 14;
const uint8_t REG_R15W        = 15;
#endif

// 32-bit registers
const uint8_t REG_EAX         = 0;
const uint8_t REG_ECX         = 1;
const uint8_t REG_EDX         = 2;
const uint8_t REG_EBX         = 3;
const uint8_t REG_ESP         = 4;
const uint8_t REG_EBP         = 5;
const uint8_t REG_ESI         = 6;
const uint8_t REG_EDI         = 7;
#if (X86EMIT_SIZE == 64)
const uint8_t REG_R8D         = 8;
const uint8_t REG_R9D         = 9;
const uint8_t REG_R10D        = 10;
const uint8_t REG_R11D        = 11;
const uint8_t REG_R12D        = 12;
const uint8_t REG_R13D        = 13;
const uint8_t REG_R14D        = 14;
const uint8_t REG_R15D        = 15;
#endif

// 64-bit registers
#if (X86EMIT_SIZE == 64)
const uint8_t REG_RAX         = 0;
const uint8_t REG_RCX         = 1;
const uint8_t REG_RDX         = 2;
const uint8_t REG_RBX         = 3;
const uint8_t REG_RSP         = 4;
const uint8_t REG_RBP         = 5;
const uint8_t REG_RSI         = 6;
const uint8_t REG_RDI         = 7;
const uint8_t REG_R8          = 8;
const uint8_t REG_R9          = 9;
const uint8_t REG_R10         = 10;
const uint8_t REG_R11         = 11;
const uint8_t REG_R12         = 12;
const uint8_t REG_R13         = 13;
const uint8_t REG_R14         = 14;
const uint8_t REG_R15         = 15;
#endif

// 64-bit MMX registers
const uint8_t REG_MM0         = 0;
const uint8_t REG_MM1         = 1;
const uint8_t REG_MM2         = 2;
const uint8_t REG_MM3         = 3;
const uint8_t REG_MM4         = 4;
const uint8_t REG_MM5         = 5;
const uint8_t REG_MM6         = 6;
const uint8_t REG_MM7         = 7;
#if (X86EMIT_SIZE == 64)
const uint8_t REG_MM8         = 8;
const uint8_t REG_MM9         = 9;
const uint8_t REG_MM10        = 10;
const uint8_t REG_MM11        = 11;
const uint8_t REG_MM12        = 12;
const uint8_t REG_MM13        = 13;
const uint8_t REG_MM14        = 14;
const uint8_t REG_MM15        = 15;
#endif

// 128-bit XMM registers
const uint8_t REG_XMM0        = 0;
const uint8_t REG_XMM1        = 1;
const uint8_t REG_XMM2        = 2;
const uint8_t REG_XMM3        = 3;
const uint8_t REG_XMM4        = 4;
const uint8_t REG_XMM5        = 5;
const uint8_t REG_XMM6        = 6;
const uint8_t REG_XMM7        = 7;
#if (X86EMIT_SIZE == 64)
const uint8_t REG_XMM8        = 8;
const uint8_t REG_XMM9        = 9;
const uint8_t REG_XMM10       = 10;
const uint8_t REG_XMM11       = 11;
const uint8_t REG_XMM12       = 12;
const uint8_t REG_XMM13       = 13;
const uint8_t REG_XMM14       = 14;
const uint8_t REG_XMM15       = 15;
#endif

// conditions
const uint8_t COND_A          = 7;
const uint8_t COND_AE         = 3;
const uint8_t COND_B          = 2;
const uint8_t COND_BE         = 6;
const uint8_t COND_C          = 2;
const uint8_t COND_E          = 4;
const uint8_t COND_Z          = 4;
const uint8_t COND_G          = 15;
const uint8_t COND_GE         = 13;
const uint8_t COND_L          = 12;
const uint8_t COND_LE         = 14;
const uint8_t COND_NA         = 6;
const uint8_t COND_NAE        = 2;
const uint8_t COND_NB         = 3;
const uint8_t COND_NBE        = 7;
const uint8_t COND_NC         = 3;
const uint8_t COND_NE         = 5;
const uint8_t COND_NG         = 14;
const uint8_t COND_NGE        = 12;
const uint8_t COND_NL         = 13;
const uint8_t COND_NLE        = 15;
const uint8_t COND_NO         = 1;
const uint8_t COND_NP         = 11;
const uint8_t COND_NS         = 9;
const uint8_t COND_NZ         = 5;
const uint8_t COND_O          = 0;
const uint8_t COND_P          = 10;
const uint8_t COND_PE         = 10;
const uint8_t COND_PO         = 11;
const uint8_t COND_S          = 8;

// floating point rounding modes
const uint8_t FPRND_NEAR      = 0;
const uint8_t FPRND_DOWN      = 1;
const uint8_t FPRND_UP        = 2;
const uint8_t FPRND_CHOP      = 3;



//**************************************************************************
//  OPCODE DEFINITIONS
//**************************************************************************

// opcode flags (in upper 8 bits of opcode)
const uint32_t OPFLAG_8BITREG             = (1 << 24);
const uint32_t OPFLAG_8BITRM              = (1 << 25);

// single byte opcodes
const uint32_t OP_ADD_Eb_Gb               = (0x00 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_ADD_Ev_Gv               = 0x01;
const uint32_t OP_ADD_Gb_Eb               = (0x02 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_ADD_Gv_Ev               = 0x03;
const uint32_t OP_ADD_AL_Ib               = 0x04;
const uint32_t OP_ADD_rAX_Iz              = 0x05;
const uint32_t OP_PUSH_ES                 = 0x06;
const uint32_t OP_POP_ES                  = 0x07;
const uint32_t OP_OR_Eb_Gb                = (0x08 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_OR_Ev_Gv                = 0x09;
const uint32_t OP_OR_Gb_Eb                = (0x0a | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_OR_Gv_Ev                = 0x0b;
const uint32_t OP_OR_AL_Ib                = 0x0c;
const uint32_t OP_OR_eAX_Iv               = 0x0d;
const uint32_t OP_PUSH_CS                 = 0x0e;
const uint32_t OP_EXTENDED                = 0x0f;

const uint32_t OP_ADC_Eb_Gb               = (0x10 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_ADC_Ev_Gv               = 0x11;
const uint32_t OP_ADC_Gb_Eb               = (0x12 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_ADC_Gv_Ev               = 0x13;
const uint32_t OP_ADC_AL_Ib               = 0x14;
const uint32_t OP_ADC_rAX_Iz              = 0x15;
const uint32_t OP_PUSH_SS                 = 0x16;
const uint32_t OP_POP_SS                  = 0x17;
const uint32_t OP_SBB_Eb_Gb               = (0x18 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_SBB_Ev_Gv               = 0x19;
const uint32_t OP_SBB_Gb_Eb               = (0x1a | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_SBB_Gv_Ev               = 0x1b;
const uint32_t OP_SBB_AL_Ib               = 0x1c;
const uint32_t OP_SBB_eAX_Iv              = 0x1d;
const uint32_t OP_PUSH_DS                 = 0x1e;
const uint32_t OP_POP_DS                  = 0x1f;

const uint32_t OP_AND_Eb_Gb               = (0x20 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_AND_Ev_Gv               = 0x21;
const uint32_t OP_AND_Gb_Eb               = (0x22 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_AND_Gv_Ev               = 0x23;
const uint32_t OP_AND_AL_Ib               = 0x24;
const uint32_t OP_AND_rAX_Iz              = 0x25;
const uint32_t PREFIX_ES                  = 0x26;
const uint32_t OP_DAA                     = 0x27;
const uint32_t OP_SUB_Eb_Gb               = (0x28 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_SUB_Ev_Gv               = 0x29;
const uint32_t OP_SUB_Gb_Eb               = (0x2a | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_SUB_Gv_Ev               = 0x2b;
const uint32_t OP_SUB_AL_Ib               = 0x2c;
const uint32_t OP_SUB_eAX_Iv              = 0x2d;
const uint32_t PREFIX_CS                  = 0x2e;
const uint32_t OP_DAS                     = 0x2f;

const uint32_t OP_XOR_Eb_Gb               = (0x30 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_XOR_Ev_Gv               = 0x31;
const uint32_t OP_XOR_Gb_Eb               = (0x32 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_XOR_Gv_Ev               = 0x33;
const uint32_t OP_XOR_AL_Ib               = 0x34;
const uint32_t OP_XOR_rAX_Iz              = 0x35;
const uint32_t PREFIX_SS                  = 0x36;
const uint32_t OP_AAA                     = 0x37;
const uint32_t OP_CMP_Eb_Gb               = (0x38 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_CMP_Ev_Gv               = 0x39;
const uint32_t OP_CMP_Gb_Eb               = (0x3a | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_CMP_Gv_Ev               = 0x3b;
const uint32_t OP_CMP_AL_Ib               = 0x3c;
const uint32_t OP_CMP_eAX_Iv              = 0x3d;
const uint32_t PREFIX_DS                  = 0x3e;
const uint32_t OP_AAS                     = 0x3f;

const uint32_t OP_REX                     = 0x40;
const uint32_t OP_REX_B                   = 0x41;
const uint32_t OP_REX_X                   = 0x42;
const uint32_t OP_REX_XB                  = 0x43;
const uint32_t OP_REX_R                   = 0x44;
const uint32_t OP_REX_RB                  = 0x45;
const uint32_t OP_REX_RX                  = 0x46;
const uint32_t OP_REX_RXB                 = 0x47;
const uint32_t OP_REX_W                   = 0x48;
const uint32_t OP_REX_WB                  = 0x49;
const uint32_t OP_REX_WX                  = 0x4a;
const uint32_t OP_REX_WXB                 = 0x4b;
const uint32_t OP_REX_WR                  = 0x4c;
const uint32_t OP_REX_WRB                 = 0x4d;
const uint32_t OP_REX_WRX                 = 0x4e;
const uint32_t OP_REX_WRXB                = 0x4f;

const uint32_t OP_PUSH_rAX                = 0x50;
const uint32_t OP_PUSH_rCX                = 0x51;
const uint32_t OP_PUSH_rDX                = 0x52;
const uint32_t OP_PUSH_rBX                = 0x53;
const uint32_t OP_PUSH_rSP                = 0x54;
const uint32_t OP_PUSH_rBP                = 0x55;
const uint32_t OP_PUSH_rSI                = 0x56;
const uint32_t OP_PUSH_rDI                = 0x57;
const uint32_t OP_POP_rAX                 = 0x58;
const uint32_t OP_POP_rCX                 = 0x59;
const uint32_t OP_POP_rDX                 = 0x5a;
const uint32_t OP_POP_rBX                 = 0x5b;
const uint32_t OP_POP_rSP                 = 0x5c;
const uint32_t OP_POP_rBP                 = 0x5d;
const uint32_t OP_POP_rSI                 = 0x5e;
const uint32_t OP_POP_rDI                 = 0x5f;

const uint32_t OP_PUSHA                   = 0x60;
const uint32_t OP_POPA                    = 0x61;
const uint32_t OP_BOUND_Gv_Ma             = 0x62;
const uint32_t OP_ARPL_Ew_Gw              = 0x63;
const uint32_t OP_MOVSXD_Gv_Ev            = 0x63;
const uint32_t PREFIX_FS                  = 0x64;
const uint32_t PREFIX_GS                  = 0x65;
const uint32_t PREFIX_OPSIZE              = 0x66;
const uint32_t PREFIX_ADSIZE              = 0x67;
const uint32_t OP_PUSH_Iz                 = 0x68;
const uint32_t OP_IMUL_Gv_Ev_Iz           = 0x69;
const uint32_t OP_PUSH_Ib                 = 0x6a;
const uint32_t OP_IMUL_Gv_Ev_Ib           = 0x6b;
const uint32_t OP_INS_Yb_DX               = 0x6c;
const uint32_t OP_INS_Yz_DX               = 0x6d;
const uint32_t OP_OUTS_DX_Xb              = 0x6e;
const uint32_t OP_OUTS_DX_Xz              = 0x6f;

const uint32_t OP_JCC_O_Jb                = 0x70;
const uint32_t OP_JCC_NO_Jb               = 0x71;
const uint32_t OP_JCC_B_Jb                = 0x72;
const uint32_t OP_JCC_C_Jb                = 0x72;
const uint32_t OP_JCC_NAE_Jb              = 0x72;
const uint32_t OP_JCC_AE_Jb               = 0x73;
const uint32_t OP_JCC_NB_Jb               = 0x73;
const uint32_t OP_JCC_NC_Jb               = 0x73;
const uint32_t OP_JCC_E_Jb                = 0x74;
const uint32_t OP_JCC_Z_Jb                = 0x74;
const uint32_t OP_JCC_NE_Jb               = 0x75;
const uint32_t OP_JCC_NZ_Jb               = 0x75;
const uint32_t OP_JCC_BE_Jb               = 0x76;
const uint32_t OP_JCC_NA_Jb               = 0x76;
const uint32_t OP_JCC_A_Jb                = 0x77;
const uint32_t OP_JCC_NBE_Jb              = 0x77;
const uint32_t OP_JCC_S_Jb                = 0x78;
const uint32_t OP_JCC_NS_Jb               = 0x79;
const uint32_t OP_JCC_P_Jb                = 0x7a;
const uint32_t OP_JCC_PE_Jb               = 0x7a;
const uint32_t OP_JCC_NP_Jb               = 0x7b;
const uint32_t OP_JCC_PO_Jb               = 0x7b;
const uint32_t OP_JCC_L_Jb                = 0x7c;
const uint32_t OP_JCC_NGE_Jb              = 0x7c;
const uint32_t OP_JCC_NL_Jb               = 0x7d;
const uint32_t OP_JCC_GE_Jb               = 0x7d;
const uint32_t OP_JCC_LE_Jb               = 0x7e;
const uint32_t OP_JCC_NG_Jb               = 0x7e;
const uint32_t OP_JCC_NLE_Jb              = 0x7f;
const uint32_t OP_JCC_G_Jb                = 0x7f;

const uint32_t OP_G1_Eb_Ib                = (0x80 | OPFLAG_8BITRM);
const uint32_t OP_G1_Ev_Iz                = 0x81;
const uint32_t OP_G1_Eb_Ibx               = (0x82 | OPFLAG_8BITRM);
const uint32_t OP_G1_Ev_Ib                = 0x83;
const uint32_t OP_TEST_Eb_Gb              = (0x84 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_TEST_Ev_Gv              = 0x85;
const uint32_t OP_XCHG_Eb_Gb              = (0x86 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_XCHG_Ev_Gv              = 0x87;
const uint32_t OP_MOV_Eb_Gb               = (0x88 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_MOV_Ev_Gv               = 0x89;
const uint32_t OP_MOV_Gb_Eb               = (0x8a | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_MOV_Gv_Ev               = 0x8b;
const uint32_t OP_MOV_Ev_Sw               = 0x8c;
const uint32_t OP_LEA_Gv_M                = 0x8d;
const uint32_t OP_MOV_Sw_Ew               = 0x8e;
const uint32_t OP_G1A_Ev                  = 0x8f;

const uint32_t OP_NOP                     = 0x90;
const uint32_t OP_PAUSE                   = 0x90;
const uint32_t OP_XCHG_rCX                = 0x91;
const uint32_t OP_XCHG_rDX                = 0x92;
const uint32_t OP_XCHG_rBX                = 0x93;
const uint32_t OP_XCHG_rSP                = 0x94;
const uint32_t OP_XCHG_rBP                = 0x95;
const uint32_t OP_XCHG_rSI                = 0x96;
const uint32_t OP_XCHG_rDI                = 0x97;
const uint32_t OP_CBW                     = 0x98;
const uint32_t OP_CWDE                    = 0x98;
const uint32_t OP_CDQE                    = 0x98;
const uint32_t OP_CWD                     = 0x99;
const uint32_t OP_CDQ                     = 0x99;
const uint32_t OP_CQO                     = 0x99;
const uint32_t OP_CALLF_Ap                = 0x9a;
const uint32_t OP_FWAIT                   = 0x9b;
const uint32_t OP_PUSHF_Fv                = 0x9c;
const uint32_t OP_POPF_Fv                 = 0x9d;
const uint32_t OP_SAHF                    = 0x9e;
const uint32_t OP_LAHF                    = 0x9f;

const uint32_t OP_MOV_AL_Ob               = 0xa0;
const uint32_t OP_MOV_rAX_Ov              = 0xa1;
const uint32_t OP_MOV_Ob_AL               = 0xa2;
const uint32_t OP_MOV_Ov_rAX              = 0xa3;
const uint32_t OP_MOVS_Xb_Yb              = 0xa4;
const uint32_t OP_MOVS_Xv_Yv              = 0xa5;
const uint32_t OP_CMPS_Xb_Yb              = 0xa6;
const uint32_t OP_CMPS_Xv_Yv              = 0xa7;
const uint32_t OP_TEST_AL_Ib              = 0xa8;
const uint32_t OP_TEST_rAX_Iz             = 0xa9;
const uint32_t OP_STOS_Yb_AL              = 0xaa;
const uint32_t OP_STOS_Yv_rAX             = 0xab;
const uint32_t OP_LODS_AL_Xb              = 0xac;
const uint32_t OP_LODS_rAX_Xv             = 0xad;
const uint32_t OP_SCAS_AL_Yb              = 0xae;
const uint32_t OP_SCAC_rAX_Yv             = 0xaf;

const uint32_t OP_MOV_AL_Ib               = 0xb0;
const uint32_t OP_MOV_CL_Ib               = 0xb1;
const uint32_t OP_MOV_DL_Ib               = 0xb2;
const uint32_t OP_MOV_BL_Ib               = 0xb3;
const uint32_t OP_MOV_AH_Ib               = 0xb4;
const uint32_t OP_MOV_CH_Ib               = 0xb5;
const uint32_t OP_MOV_DH_Ib               = 0xb6;
const uint32_t OP_MOV_BH_Ib               = 0xb7;
const uint32_t OP_MOV_rAX_Iv              = 0xb8;
const uint32_t OP_MOV_rCX_Iv              = 0xb9;
const uint32_t OP_MOV_rDX_Iv              = 0xba;
const uint32_t OP_MOV_rBX_Iv              = 0xbb;
const uint32_t OP_MOV_rSP_Iv              = 0xbc;
const uint32_t OP_MOV_rBP_Iv              = 0xbd;
const uint32_t OP_MOV_rSI_Iv              = 0xbe;
const uint32_t OP_MOV_rDI_Iv              = 0xbf;

const uint32_t OP_G2_Eb_Ib                = (0xc0 | OPFLAG_8BITRM);
const uint32_t OP_G2_Ev_Ib                = 0xc1;
const uint32_t OP_RETN_Iw                 = 0xc2;
const uint32_t OP_RETN                    = 0xc3;
const uint32_t OP_LES_Gz_Mp               = 0xc4;
const uint32_t OP_LDS_Gz_Mp               = 0xc5;
const uint32_t OP_G11_Eb_Ib               = (0xc6 | OPFLAG_8BITRM);
const uint32_t OP_G11_Ev_Iz               = 0xc7;
const uint32_t OP_ENTER_Iw_Ib             = 0xc8;
const uint32_t OP_LEAVE                   = 0xc9;
const uint32_t OP_RETF_Iw                 = 0xca;
const uint32_t OP_RETF                    = 0xcb;
const uint32_t OP_INT_3                   = 0xcc;
const uint32_t OP_INT_Ib                  = 0xcd;
const uint32_t OP_INTO                    = 0xce;
const uint32_t OP_IRET                    = 0xcf;

const uint32_t OP_G2_Eb_1                 = (0xd0 | OPFLAG_8BITRM);
const uint32_t OP_G2_Ev_1                 = 0xd1;
const uint32_t OP_G2_Eb_CL                = (0xd2 | OPFLAG_8BITRM);
const uint32_t OP_G2_Ev_CL                = 0xd3;
const uint32_t OP_AAM                     = 0xd4;
const uint32_t OP_AAD                     = 0xd5;
const uint32_t OP_XLAT                    = 0xd7;
const uint32_t OP_ESC_D8                  = 0xd8;
const uint32_t OP_ESC_D9                  = 0xd9;
const uint32_t OP_ESC_DA                  = 0xda;
const uint32_t OP_ESC_DB                  = 0xdb;
const uint32_t OP_ESC_DC                  = 0xdc;
const uint32_t OP_ESC_DD                  = 0xdd;
const uint32_t OP_ESC_DE                  = 0xde;
const uint32_t OP_ESC_DF                  = 0xdf;

const uint32_t OP_LOOPNE_Jb               = 0xe0;
const uint32_t OP_LOOPE_Jb                = 0xe1;
const uint32_t OP_LOOP_Jb                 = 0xe2;
const uint32_t OP_JrCXZ_Jb                = 0xe3;
const uint32_t OP_IN_AL_Ib                = 0xe4;
const uint32_t OP_IN_eAX_Ib               = 0xe5;
const uint32_t OP_OUT_Ib_AL               = 0xe6;
const uint32_t OP_OUT_Ib_eAX              = 0xe7;
const uint32_t OP_CALL_Jz                 = 0xe8;
const uint32_t OP_JMP_Jz                  = 0xe9;
const uint32_t OP_JMPF_AP                 = 0xea;
const uint32_t OP_JMP_Jb                  = 0xeb;
const uint32_t OP_IN_AL_DX                = 0xec;
const uint32_t OP_IN_eAX_D                = 0xed;
const uint32_t OP_OUT_DX_AL               = 0xee;
const uint32_t OP_OUT_DX_eAX              = 0xef;

const uint32_t PREFIX_LOCK                = 0xf0;
const uint32_t PREFIX_REPNE               = 0xf2;
const uint32_t PREFIX_REPE                = 0xf3;
const uint32_t OP_HLT                     = 0xf4;
const uint32_t OP_CMC                     = 0xf5;
const uint32_t OP_G3_Eb                   = (0xf6 | OPFLAG_8BITRM);
const uint32_t OP_G3_Ev                   = 0xf7;
const uint32_t OP_CLC                     = 0xf8;
const uint32_t OP_STC                     = 0xf9;
const uint32_t OP_CLI                     = 0xfa;
const uint32_t OP_STI                     = 0xfb;
const uint32_t OP_CLD                     = 0xfc;
const uint32_t OP_STD                     = 0xfd;
const uint32_t OP_G4                      = 0xfe;
const uint32_t OP_G5                      = 0xff;


// double byte opcodes
const uint32_t OP_G6                      = 0x0f00;
const uint32_t OP_G7                      = 0x0f01;
const uint32_t OP_LAR_Gv_Ew               = 0x0f02;
const uint32_t OP_LSL_Gv_Ew               = 0x0f03;
const uint32_t OP_SYSCALL                 = 0x0f05;
const uint32_t OP_CLTS                    = 0x0f06;
const uint32_t OP_SYSRET                  = 0x0f07;
const uint32_t OP_INVD                    = 0x0f08;
const uint32_t OP_WBINVD                  = 0x0f09;
const uint32_t OP_UD2                     = 0x0f0b;
const uint32_t OP_NOP0d_Ev                = 0x0f0d;

const uint32_t OP_MOVUPS_Vps_Wps          = 0x0f10;
const uint32_t OP_MOVSS_Vss_Wss           = 0xf30f10;
const uint32_t OP_MOVUPD_Vpd_Wpd          = 0x660f10;
const uint32_t OP_MOVSD_Vsd_Wsd           = 0xf20f10;
const uint32_t OP_MOVUPS_Wps_Vps          = 0x0f11;
const uint32_t OP_MOVSS_Wss_Vss           = 0xf30f11;
const uint32_t OP_MOVUPD_Wpd_Vpd          = 0x660f11;
const uint32_t OP_MOVSD_Wsd_Vsd           = 0xf20f11;
const uint32_t OP_MOVLPS_Vq_Mq            = 0x0f12;
const uint32_t OP_MOVLPD_Vq_Mq            = 0x660f12;
const uint32_t OP_MOVHLPS_Vq_Uq           = 0x0f12;
const uint32_t OP_MOVDDUP_Vq_Wq           = 0xf20f12;
const uint32_t OP_MOVSLDUP_Vq_Wq          = 0xf30f12;
const uint32_t OP_MOVLPS_Mq_Vq            = 0x0f13;
const uint32_t OP_MOVLPD_Mq_Vq            = 0x660f13;
const uint32_t OP_UNPCKLPS_Vps_Wq         = 0x0f14;
const uint32_t OP_UNPCKLPD_Vpd_Wq         = 0x660f14;
const uint32_t OP_UNPCKHPS_Vps_Wq         = 0x0f15;
const uint32_t OP_UNPCKHPD_Vpd_Wq         = 0x660f15;
const uint32_t OP_MOVHPS_Vq_Mq            = 0x0f16;
const uint32_t OP_MOVHPD_Vq_Mq            = 0x660f16;
const uint32_t OP_MOVLHPS_Vq_Uq           = 0x0f16;
const uint32_t OP_MOVSHDUP_Vq_Wq          = 0xf30f16;
const uint32_t OP_MOVHPS_Mq_Vq            = 0x0f17;
const uint32_t OP_MOVHPD_Mq_Vq            = 0x660f17;
const uint32_t OP_PREFETCH_G16            = 0x0f18;
const uint32_t OP_NOP1f_Ev                = 0x0f1f;

const uint32_t OP_MOV_Rd_Cd               = 0x0f20;
const uint32_t OP_MOV_Rd_Dd               = 0x0f21;
const uint32_t OP_MOV_Cd_Rd               = 0x0f22;
const uint32_t OP_MOV_Dd_Rd               = 0x0f23;
const uint32_t OP_MOVAPS_Vps_Wps          = 0x0f28;
const uint32_t OP_MOVAPD_Vpd_Wpd          = 0x660f28;
const uint32_t OP_MOVAPS_Wps_Vps          = 0x0f29;
const uint32_t OP_MOVAPD_Wpd_Vpd          = 0x660f29;
const uint32_t OP_CVTPI2PS_Vps_Qq         = 0x0f2a;
const uint32_t OP_CVTSI2SS_Vss_Ed         = 0xf30f2a;
const uint32_t OP_CVTPI2PD_Vpd_Qq         = 0x660f2a;
const uint32_t OP_CVTSI2SD_Vsd_Ed         = 0xf20f2a;
const uint32_t OP_MOVNTPS_Mps_Vps         = 0x0f2b;
const uint32_t OP_MOVNTPD_Mpd_Vpd         = 0x660f2b;
const uint32_t OP_CVTTPS2PI_Pq_Wq         = 0x0f2c;
const uint32_t OP_CVTTSS2SI_Gd_Wss        = 0xf30f2c;
const uint32_t OP_CVTTPD2PI_Pq_Wpd        = 0x660f2c;
const uint32_t OP_CVTTSD2SI_Gd_Wsd        = 0xf20f2c;
const uint32_t OP_CVTPS2PI_Pq_Wq          = 0x0f2d;
const uint32_t OP_CVTSS2SI_Gd_Wss         = 0xf30f2d;
const uint32_t OP_CVTPD2PI_Pq_Wpd         = 0x660f2d;
const uint32_t OP_CVTSD2SI_Gd_Wsd         = 0xf20f2d;
const uint32_t OP_UCOMISS_Vss_Wss         = 0x0f2e;
const uint32_t OP_UCOMISD_Vsd_Wsd         = 0x660f2e;
const uint32_t OP_COMISS_Vss_Wss          = 0x0f2f;
const uint32_t OP_COMISD_Vsd_Wsd          = 0x660f2f;

const uint32_t OP_WRMSR                   = 0x0f30;
const uint32_t OP_RDTSC                   = 0x0f31;
const uint32_t OP_RDMSR                   = 0x0f32;
const uint32_t OP_RDPMC                   = 0x0f33;
const uint32_t OP_SYSENTER                = 0x0f34;
const uint32_t OP_SYSEXIT                 = 0x0f35;
const uint32_t OP_GETSEC                  = 0x0f37;

const uint32_t OP_CMOV_O_Gv_Ev            = 0x0f40;
const uint32_t OP_CMOV_NO_Gv_Ev           = 0x0f41;
const uint32_t OP_CMOV_B_Gv_Ev            = 0x0f42;
const uint32_t OP_CMOV_C_Gv_Ev            = 0x0f42;
const uint32_t OP_CMOV_AE_Gv_Ev           = 0x0f43;
const uint32_t OP_CMOV_NC_Gv_Ev           = 0x0f43;
const uint32_t OP_CMOV_E_Gv_Ev            = 0x0f44;
const uint32_t OP_CMOV_Z_Gv_Ev            = 0x0f44;
const uint32_t OP_CMOV_NE_Gv_Ev           = 0x0f45;
const uint32_t OP_CMOV_NZ_Gv_Ev           = 0x0f45;
const uint32_t OP_CMOV_BE_Gv_Ev           = 0x0f46;
const uint32_t OP_CMOV_A_Gv_Ev            = 0x0f47;
const uint32_t OP_CMOV_S_Gv_Ev            = 0x0f48;
const uint32_t OP_CMOV_NS_Gv_Ev           = 0x0f49;
const uint32_t OP_CMOV_P_Gv_Ev            = 0x0f4a;
const uint32_t OP_CMOV_PE_Gv_Ev           = 0x0f4a;
const uint32_t OP_CMOV_NP_Gv_Ev           = 0x0f4b;
const uint32_t OP_CMOV_PO_Gv_Ev           = 0x0f4b;
const uint32_t OP_CMOV_L_Gv_Ev            = 0x0f4c;
const uint32_t OP_CMOV_NGE_Gv_Ev          = 0x0f4c;
const uint32_t OP_CMOV_NL_Gv_Ev           = 0x0f4d;
const uint32_t OP_CMOV_GE_Gv_Ev           = 0x0f4d;
const uint32_t OP_CMOV_LE_Gv_Ev           = 0x0f4e;
const uint32_t OP_CMOV_NG_Gv_Ev           = 0x0f4e;
const uint32_t OP_CMOV_NLE_Gv_Ev          = 0x0f4f;
const uint32_t OP_CMOV_G_Gv_Ev            = 0x0f4f;

const uint32_t OP_MOVMSKPS_Gd_Ups         = 0x0f50;
const uint32_t OP_MOVMSKPD_Gd_Upd         = 0x660f50;
const uint32_t OP_SQRTPS_Vps_Wps          = 0x0f51;
const uint32_t OP_SQRTSS_Vss_Wss          = 0xf30f51;
const uint32_t OP_SQRTPD_Vpd_Wpd          = 0x660f51;
const uint32_t OP_SQRTSD_Vsd_Wsd          = 0xf20f51;
const uint32_t OP_RSQRTPS_Vps_Wps         = 0x0f52;
const uint32_t OP_RSQRTSS_Vss_Wss         = 0xf30f52;
const uint32_t OP_RCPPS_Vps_Wps           = 0x0f53;
const uint32_t OP_RCPSS_Vss_Wss           = 0xf30f53;
const uint32_t OP_ANDPS_Vps_Wps           = 0x0f54;
const uint32_t OP_ANDPD_Vpd_Wpd           = 0x660f54;
const uint32_t OP_ANDNPS_Vps_Wps          = 0x0f55;
const uint32_t OP_ANDNPD_Vpd_Wpd          = 0x660f55;
const uint32_t OP_ORPS_Vps_Wps            = 0x0f56;
const uint32_t OP_ORPD_Vpd_Wpd            = 0x660f56;
const uint32_t OP_XORPS_Vps_Wps           = 0x0f57;
const uint32_t OP_XORPD_Vpd_Wpd           = 0x660f57;
const uint32_t OP_ADDPS_Vps_Wps           = 0x0f58;
const uint32_t OP_ADDSS_Vss_Wss           = 0xf30f58;
const uint32_t OP_ADDPD_Vpd_Wpd           = 0x660f58;
const uint32_t OP_ADDSD_Vsd_Wsd           = 0xf20f58;
const uint32_t OP_MULPS_Vps_Wps           = 0x0f59;
const uint32_t OP_MULSS_Vss_Wss           = 0xf30f59;
const uint32_t OP_MULPD_Vpd_Wpd           = 0x660f59;
const uint32_t OP_MULSD_Vsd_Wsd           = 0xf20f59;
const uint32_t OP_CVTPS2PD_Vpd_Wq         = 0x0f5a;
const uint32_t OP_CVTSS2SD_Vsd_Wss        = 0xf30f5a;
const uint32_t OP_CVTPD2PS_Vps_Wpd        = 0x660f5a;
const uint32_t OP_CVTSD2SS_Vss_Wsd        = 0xf20f5a;
const uint32_t OP_CVTDQ2PS_Vps_Wdq        = 0x0f5b;
const uint32_t OP_CVTPS2DQ_Vdq_Wps        = 0x660f5b;
const uint32_t OP_CVTTPS2DQ_Vdq_Wps       = 0xf30f5b;
const uint32_t OP_SUBPS_Vps_Wps           = 0x0f5c;
const uint32_t OP_SUBSS_Vss_Wss           = 0xf30f5c;
const uint32_t OP_SUBPD_Vpd_Wpd           = 0x660f5c;
const uint32_t OP_SUBSD_Vsd_Wsd           = 0xf20f5c;
const uint32_t OP_MINPS_Vps_Wps           = 0x0f5d;
const uint32_t OP_MINSS_Vss_Wss           = 0xf30f5d;
const uint32_t OP_MINPD_Vpd_Wpd           = 0x660f5d;
const uint32_t OP_MINSD_Vsd_Wsd           = 0xf20f5d;
const uint32_t OP_DIVPS_Vps_Wps           = 0x0f5e;
const uint32_t OP_DIVSS_Vss_Wss           = 0xf30f5e;
const uint32_t OP_DIVPD_Vpd_Wpd           = 0x660f5e;
const uint32_t OP_DIVSD_Vsd_Wsd           = 0xf20f5e;
const uint32_t OP_MAXPS_Vps_Wps           = 0x0f5f;
const uint32_t OP_MAXSS_Vss_Wss           = 0xf30f5f;
const uint32_t OP_MAXPD_Vpd_Wpd           = 0x660f5f;
const uint32_t OP_MAXSD_Vsd_Wsd           = 0xf20f5f;

const uint32_t OP_PUNPCKLBW_Pq_Qd         = 0x0f60;
const uint32_t OP_PUNPCKLBW_Vdq_Wdq       = 0x660f60;
const uint32_t OP_PUNPCKLWD_Pq_Qd         = 0x0f61;
const uint32_t OP_PUNPCKLWD_Vdq_Wdq       = 0x660f61;
const uint32_t OP_PUNPCKLDQ_Pq_Qd         = 0x0f62;
const uint32_t OP_PUNPCKLDQ_Vdq_Wdq       = 0x660f62;
const uint32_t OP_PACKSSWB_Pq_Qq          = 0x0f63;
const uint32_t OP_PACKSSWB_Vdq_Wdq        = 0x660f63;
const uint32_t OP_PCMPGTB_Pq_Qq           = 0x0f64;
const uint32_t OP_PCMPGTB_Vdq_Wdq         = 0x660f64;
const uint32_t OP_PCMPGTW_Pq_Qq           = 0x0f65;
const uint32_t OP_PCMPGTW_Vdq_Wdq         = 0x660f65;
const uint32_t OP_PCMPGTD_Pq_Qq           = 0x0f66;
const uint32_t OP_PCMPGTD_Vdq_Wdq         = 0x660f66;
const uint32_t OP_PACKUSWB_Pq_Qq          = 0x0f67;
const uint32_t OP_PACKUSWB_Vdq_Wdq        = 0x660f67;
const uint32_t OP_PUNPCKHBW_Pq_Qq         = 0x0f68;
const uint32_t OP_PUNPCKHBW_Vdq_Qdq       = 0x660f68;
const uint32_t OP_PUNPCKHWD_Pq_Qq         = 0x0f69;
const uint32_t OP_PUNPCKHWD_Vdq_Qdq       = 0x660f69;
const uint32_t OP_PUNPCKHDQ_Pq_Qq         = 0x0f6a;
const uint32_t OP_PUNPCKHDQ_Vdq_Qdq       = 0x660f6a;
const uint32_t OP_PACKSSDW_Pq_Qq          = 0x0f6b;
const uint32_t OP_PACKSSDW_Vdq_Qdq        = 0x660f6b;
const uint32_t OP_PUNPCKLQDQ_Vdq_Wdq      = 0x660f6c;
const uint32_t OP_PUNPCKHQDQ_Vdq_Wdq      = 0x660f6d;
const uint32_t OP_MOVD_Pd_Ed              = 0x0f6e;
const uint32_t OP_MOVD_Vd_Ed              = 0x660f6e;
const uint32_t OP_MOVQ_Pq_Qq              = 0x0f6f;
const uint32_t OP_MOVDQA_Vdq_Wdq          = 0x660f6f;
const uint32_t OP_MOVDQU_Vdq_Wdq          = 0xf30f6f;

const uint32_t OP_PSHUFW_Pq_Qq_Ib         = 0x0f70;
const uint32_t OP_PSHUFD_Vdq_Wdq_Ib       = 0x660f70;
const uint32_t OP_PSHUFHW_Vdq_Wdq_Ib      = 0xf30f70;
const uint32_t OP_PSHUFLW_Vdq_Wdq_Ib      = 0xf20f70;
const uint32_t OP_G12                     = 0x0f71;
const uint32_t OP_G13                     = 0x0f72;
const uint32_t OP_G14                     = 0x0f73;
const uint32_t OP_PCMPEQB_Pq_Qq           = 0x0f74;
const uint32_t OP_PCMPEQB_Vdq_Wdq         = 0x660f74;
const uint32_t OP_PCMPEQW_Pq_Qq           = 0x0f75;
const uint32_t OP_PCMPEQW_Vdq_Wdq         = 0x660f75;
const uint32_t OP_PCMPEQD_Pq_Qq           = 0x0f76;
const uint32_t OP_PCMPEQD_Vdq_Wdq         = 0x660f76;
const uint32_t OP_EMMS                    = 0x0f77;
const uint32_t OP_VMREAD_Ed_Gd            = 0x0f78;
const uint32_t OP_VMWRITE_Gd_Ed           = 0x0f79;
const uint32_t OP_HADDPD_Vpd_Wpd          = 0x660f7c;
const uint32_t OP_HADDPS_Vps_Wps          = 0xf20f7c;
const uint32_t OP_HSUBPD_Vpd_Wpd          = 0x660f7d;
const uint32_t OP_HSUBPS_Vps_Wps          = 0xf20f7d;
const uint32_t OP_MOVD_Ed_Pd              = 0x0f7e;
const uint32_t OP_MOVD_Ed_Vd              = 0x660f7e;
const uint32_t OP_MOVQ_Vq_Wq              = 0xf30f7e;
const uint32_t OP_MOVQ_Qq_Pq              = 0x0f7f;
const uint32_t OP_MOVDQA_Wdq_Vdq          = 0x660f7f;
const uint32_t OP_MOVDQU_Wdq_Vdq          = 0xf30f7f;

const uint32_t OP_JCC_O_Jv                = 0x0f80;
const uint32_t OP_JCC_NO_Jv               = 0x0f81;
const uint32_t OP_JCC_B_Jv                = 0x0f82;
const uint32_t OP_JCC_C_Jv                = 0x0f82;
const uint32_t OP_JCC_NAE_Jv              = 0x0f82;
const uint32_t OP_JCC_AE_Jv               = 0x0f83;
const uint32_t OP_JCC_NB_Jv               = 0x0f83;
const uint32_t OP_JCC_NC_Jv               = 0x0f83;
const uint32_t OP_JCC_E_Jv                = 0x0f84;
const uint32_t OP_JCC_Z_Jv                = 0x0f84;
const uint32_t OP_JCC_NE_Jv               = 0x0f85;
const uint32_t OP_JCC_NZ_Jv               = 0x0f85;
const uint32_t OP_JCC_BE_Jv               = 0x0f86;
const uint32_t OP_JCC_NA_Jv               = 0x0f86;
const uint32_t OP_JCC_A_Jv                = 0x0f87;
const uint32_t OP_JCC_NBE_Jv              = 0x0f87;
const uint32_t OP_JCC_S_Jv                = 0x0f88;
const uint32_t OP_JCC_NS_Jv               = 0x0f89;
const uint32_t OP_JCC_P_Jv                = 0x0f8a;
const uint32_t OP_JCC_PE_Jv               = 0x0f8a;
const uint32_t OP_JCC_NP_Jv               = 0x0f8b;
const uint32_t OP_JCC_PO_Jv               = 0x0f8b;
const uint32_t OP_JCC_L_Jv                = 0x0f8c;
const uint32_t OP_JCC_NGE_Jv              = 0x0f8c;
const uint32_t OP_JCC_NL_Jv               = 0x0f8d;
const uint32_t OP_JCC_GE_Jv               = 0x0f8d;
const uint32_t OP_JCC_LE_Jv               = 0x0f8e;
const uint32_t OP_JCC_NG_Jv               = 0x0f8e;
const uint32_t OP_JCC_NLE_Jv              = 0x0f8f;
const uint32_t OP_JCC_G_Jv                = 0x0f8f;

const uint32_t OP_SETCC_O_Eb              = (0x0f90 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_NO_Eb             = (0x0f91 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_B_Eb              = (0x0f92 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_C_Eb              = (0x0f92 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_NAE_Eb            = (0x0f92 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_AE_Eb             = (0x0f93 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_NB_Eb             = (0x0f93 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_NC_Eb             = (0x0f93 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_E_Eb              = (0x0f94 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_Z_Eb              = (0x0f94 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_NE_Eb             = (0x0f95 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_NZ_Eb             = (0x0f95 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_BE_Eb             = (0x0f96 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_NA_Eb             = (0x0f96 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_A_Eb              = (0x0f97 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_NBE_Eb            = (0x0f97 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_S_Eb              = (0x0f98 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_NS_Eb             = (0x0f99 | OPFLAG_8BITRM);
const uint32_t OP_SETCC_P_Eb              = (0x0f9a | OPFLAG_8BITRM);
const uint32_t OP_SETCC_PE_Eb             = (0x0f9a | OPFLAG_8BITRM);
const uint32_t OP_SETCC_NP_Eb             = (0x0f9b | OPFLAG_8BITRM);
const uint32_t OP_SETCC_PO_Eb             = (0x0f9b | OPFLAG_8BITRM);
const uint32_t OP_SETCC_L_Eb              = (0x0f9c | OPFLAG_8BITRM);
const uint32_t OP_SETCC_NGE_Eb            = (0x0f9c | OPFLAG_8BITRM);
const uint32_t OP_SETCC_NL_Eb             = (0x0f9d | OPFLAG_8BITRM);
const uint32_t OP_SETCC_GE_Eb             = (0x0f9d | OPFLAG_8BITRM);
const uint32_t OP_SETCC_LE_Eb             = (0x0f9e | OPFLAG_8BITRM);
const uint32_t OP_SETCC_NG_Eb             = (0x0f9e | OPFLAG_8BITRM);
const uint32_t OP_SETCC_NLE_Eb            = (0x0f9f | OPFLAG_8BITRM);
const uint32_t OP_SETCC_G_Eb              = (0x0f9f | OPFLAG_8BITRM);

const uint32_t OP_PUSH_FS                 = 0x0fa0;
const uint32_t OP_POP_FS                  = 0x0fa1;
const uint32_t OP_CPUID                   = 0x0fa2;
const uint32_t OP_BT_Ev_Gv                = 0x0fa3;
const uint32_t OP_SHLD_Ev_Gv_Ib           = 0x0fa4;
const uint32_t OP_SHLD_Ev_Gv_CL           = 0x0fa5;
const uint32_t OP_PUSH_GS                 = 0x0fa8;
const uint32_t OP_POP_GS                  = 0x0fa9;
const uint32_t OP_RSM                     = 0x0faa;
const uint32_t OP_BTS_Ev_Gv               = 0x0fab;
const uint32_t OP_SHRD_Ev_Gv_Ib           = 0x0fac;
const uint32_t OP_SHRD_Ev_Gv_CL           = 0x0fad;
const uint32_t OP_G16                     = 0x0fae;
const uint32_t OP_IMUL_Gv_Ev              = 0x0faf;

const uint32_t OP_CMPXCHG_Eb_Gb           = (0x0fb0 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_CMPXCHG_Ev_Gv           = 0x0fb1;
const uint32_t OP_LSS_Mp                  = 0x0fb2;
const uint32_t OP_BTR_Ev_Gv               = 0x0fb3;
const uint32_t OP_LFS_Mp                  = 0x0fb4;
const uint32_t OP_LGS_Mp                  = 0x0fb5;
const uint32_t OP_MOVZX_Gv_Eb             = (0x0fb6 | OPFLAG_8BITRM);
const uint32_t OP_MOVZX_Gv_Ew             = 0x0fb7;
const uint32_t OP_JMPE                    = 0x0fb8;
const uint32_t OP_POPCNT_Gv_Ev            = 0xf30fb8;
const uint32_t OP_G10_INVALID             = 0x0fb9;
const uint32_t OP_G8_Ev_Ib                = 0x0fba;
const uint32_t OP_BTC_Ev_Gv               = 0x0fbb;
const uint32_t OP_BSF_Gv_Ev               = 0x0fbc;
const uint32_t OP_BSR_Gv_Ev               = 0x0fbd;
const uint32_t OP_MOVSX_Gv_Eb             = (0x0fbe | OPFLAG_8BITRM);
const uint32_t OP_MOVSX_Gv_Ew             = 0x0fbf;

const uint32_t OP_XADD_Eb_Gb              = (0x0fc0 | OPFLAG_8BITRM | OPFLAG_8BITREG);
const uint32_t OP_XADD_Ev_Gv              = 0x0fc1;
const uint32_t OP_CMPPS_Vps_Wps_Ib        = 0x0fc2;
const uint32_t OP_CMPSS_Vss_Wss_Ib        = 0xf30fc2;
const uint32_t OP_CMPPD_Vpd_Wpd_Ib        = 0x660fc2;
const uint32_t OP_CMPSD_Vsd_Wsd_Ib        = 0xf20fc2;
const uint32_t OP_MOVNTI_Md_Gd            = 0x0fc3;
const uint32_t OP_PINSRW_Pw_Ew_Ib         = 0x0fc4;
const uint32_t OP_PINSRW_Vw_Ew_Ib         = 0x660fc4;
const uint32_t OP_PEXTRW_Gw_Pw_Ib         = 0x0fc5;
const uint32_t OP_PEXTRW_Gw_Vw_Ib         = 0x660fc5;
const uint32_t OP_SHUFPS_Vps_Wps_Ib       = 0x0fc6;
const uint32_t OP_SHUFPD_Vpd_Wpd_Ib       = 0x660fc6;
const uint32_t OP_G9                      = 0x0fc7;
const uint32_t OP_BSWAP_EAX               = 0x0fc8;
const uint32_t OP_BSWAP_ECX               = 0x0fc9;
const uint32_t OP_BSWAP_EDX               = 0x0fca;
const uint32_t OP_BSWAP_EBX               = 0x0fcb;
const uint32_t OP_BSWAP_ESP               = 0x0fcc;
const uint32_t OP_BSWAP_EBP               = 0x0fcd;
const uint32_t OP_BSWAP_ESI               = 0x0fce;
const uint32_t OP_BSWAP_EDI               = 0x0fcf;

const uint32_t OP_ADDSUBPD_Vpd_Wpd        = 0x0fd0;
const uint32_t OP_ADDSUBPS_Vps_Wps        = 0xf20fd0;
const uint32_t OP_PSRLW_Pq_Qq             = 0x0fd1;
const uint32_t OP_PSRLW_Vdq_Wdq           = 0x660fd1;
const uint32_t OP_PSRLD_Pq_Qq             = 0x0fd2;
const uint32_t OP_PSRLD_Vdq_Wdq           = 0x660fd2;
const uint32_t OP_PSRLQ_Pq_Qq             = 0x0fd3;
const uint32_t OP_PSRLQ_Vdq_Wdq           = 0x660fd3;
const uint32_t OP_PADDQ_Pq_Qq             = 0x0fd4;
const uint32_t OP_PADDQ_Vdq_Wdq           = 0x660fd4;
const uint32_t OP_PMULLW_Pq_Qq            = 0x0fd5;
const uint32_t OP_PMULLW_Vdq_Wdq          = 0x660fd5;
const uint32_t OP_MOVQ_Wq_Vq              = 0x0fd6;
const uint32_t OP_MOVQ2DQ_Vdq_Qq          = 0xf30fd6;
const uint32_t OP_MOVDQ2Q_Pq_Vq           = 0xf20fd6;
const uint32_t OP_PMOVMSKB_Gd_Pq          = 0x0fd7;
const uint32_t OP_PMOVMSKB_Gd_Vdq         = 0x660fd7;
const uint32_t OP_PSUBUSB_Pq_Qq           = 0x0fd8;
const uint32_t OP_PSUBUSB_Vdq_Wdq         = 0x660fd8;
const uint32_t OP_PSUBUSW_Pq_Qq           = 0x0fd9;
const uint32_t OP_PSUBUSW_Vdq_Wdq         = 0x660fd9;
const uint32_t OP_PMINUB_Pq_Qq            = 0x0fda;
const uint32_t OP_PMINUB_Vdq_Wdq          = 0x660fda;
const uint32_t OP_PAND_Pq_Qq              = 0x0fdb;
const uint32_t OP_PAND_Vdq_Wdq            = 0x660fdb;
const uint32_t OP_PADDUSB_Pq_Qq           = 0x0fdc;
const uint32_t OP_PADDUSB_Vdq_Wdq         = 0x660fdc;
const uint32_t OP_PADDUSW_Pq_Qq           = 0x0fdd;
const uint32_t OP_PADDUSW_Vdq_Wdq         = 0x660fdd;
const uint32_t OP_PMAXUB_Pq_Qq            = 0x0fde;
const uint32_t OP_PMAXUB_Vdq_Wdq          = 0x660fde;
const uint32_t OP_PANDN_Pq_Qq             = 0x0fdf;
const uint32_t OP_PANDN_Vdq_Wdq           = 0x660fdf;

const uint32_t OP_PAVGB_Pq_Qq             = 0x0fe0;
const uint32_t OP_PAVGB_Vdq_Wdq           = 0x660fe0;
const uint32_t OP_PSRAW_Pq_Qq             = 0x0fe1;
const uint32_t OP_PSRAW_Vdq_Wdq           = 0x660fe1;
const uint32_t OP_PSRAD_Pq_Qq             = 0x0fe2;
const uint32_t OP_PSRAD_Vdq_Wdq           = 0x660fe2;
const uint32_t OP_PAVGW_Pq_Qq             = 0x0fe3;
const uint32_t OP_PAVGW_Vdq_Wdq           = 0x660fe3;
const uint32_t OP_PMULHUW_Pq_Qq           = 0x0fe4;
const uint32_t OP_PMULHUW_Vdq_Wdq         = 0x660fe4;
const uint32_t OP_PMULHW_Pq_Qq            = 0x0fe5;
const uint32_t OP_PMULHW_Vdq_Wdq          = 0x660fe5;
const uint32_t OP_CVTPD2DQ_Vdq_Wpd        = 0xf20fe6;
const uint32_t OP_CVTTPD2DQ_Vdq_Wpd       = 0x660fe6;
const uint32_t OP_CVTDQ2PD_Vpd_Wq         = 0xf30fe6;
const uint32_t OP_MOVNTQ_Mq_Vq            = 0x0fe7;
const uint32_t OP_MOVNTDQ_Mdq_Vdq         = 0x660fe7;
const uint32_t OP_PSUBSB_Pq_Qq            = 0x0fe8;
const uint32_t OP_PSUBSB_Vdq_Wdq          = 0x660fe8;
const uint32_t OP_PSUBSW_Pq_Qq            = 0x0fe9;
const uint32_t OP_PSUBSW_Vdq_Wdq          = 0x660fe9;
const uint32_t OP_PMINSW_Pq_Qq            = 0x0fea;
const uint32_t OP_PMINSW_Vdq_Wdq          = 0x660fea;
const uint32_t OP_POR_Pq_Qq               = 0x0feb;
const uint32_t OP_POR_Vdq_Wdq             = 0x660feb;
const uint32_t OP_PADDSB_Pq_Qq            = 0x0fec;
const uint32_t OP_PADDSB_Vdq_Wdq          = 0x660fec;
const uint32_t OP_PADDSW_Pq_Qq            = 0x0fed;
const uint32_t OP_PADDSW_Vdq_Wdq          = 0x660fed;
const uint32_t OP_PMAXSW_Pq_Qq            = 0x0fee;
const uint32_t OP_PMAXSW_Vdq_Wdq          = 0x660fee;
const uint32_t OP_PXOR_Pq_Qq              = 0x0fef;
const uint32_t OP_PXOR_Vdq_Wdq            = 0x660fef;

const uint32_t OP_LDDQU_Vdq_Mdq           = 0xf20ff0;
const uint32_t OP_PSLLW_Pq_Qq             = 0x0ff1;
const uint32_t OP_PSLLW_Vdq_Wdq           = 0x660ff1;
const uint32_t OP_PSLLD_Pq_Qq             = 0x0ff2;
const uint32_t OP_PSLLD_Vdq_Wdq           = 0x660ff2;
const uint32_t OP_PSLLQ_Pq_Qq             = 0x0ff3;
const uint32_t OP_PSLLQ_Vdq_Wdq           = 0x660ff3;
const uint32_t OP_PMULUDQ_Pq_Qq           = 0x0ff4;
const uint32_t OP_PMULUDQ_Vdq_Wdq         = 0x660ff4;
const uint32_t OP_PMADDWD_Pq_Qq           = 0x0ff5;
const uint32_t OP_PMADDWD_Vdq_Wdq         = 0x660ff5;
const uint32_t OP_PSADBW_Pq_Qq            = 0x0ff6;
const uint32_t OP_PSADBW_Vdq_Wdq          = 0x660ff6;
const uint32_t OP_MASKMOVQ_Pq_Qq          = 0x0ff7;
const uint32_t OP_MASKMOVDQU_Vdq_Wdq      = 0x660ff7;
const uint32_t OP_PSUBB_Pq_Qq             = 0x0ff8;
const uint32_t OP_PSUBB_Vdq_Wdq           = 0x660ff8;
const uint32_t OP_PSUBW_Pq_Qq             = 0x0ff9;
const uint32_t OP_PSUBW_Vdq_Wdq           = 0x660ff9;
const uint32_t OP_PSUBD_Pq_Qq             = 0x0ffa;
const uint32_t OP_PSUBD_Vdq_Wdq           = 0x660ffa;
const uint32_t OP_PSUBQ_Pq_Qq             = 0x0ffb;
const uint32_t OP_PSUBQ_Vdq_Wdq           = 0x660ffb;
const uint32_t OP_PADDB_Pq_Qq             = 0x0ffc;
const uint32_t OP_PADDB_Vdq_Wdq           = 0x660ffc;
const uint32_t OP_PADDW_Pq_Qq             = 0x0ffd;
const uint32_t OP_PADDW_Vdq_Wdq           = 0x660ffd;
const uint32_t OP_PADDD_Pq_Qq             = 0x0ffe;
const uint32_t OP_PADDD_Vdq_Wdq           = 0x660ffe;


// triple byte opcodes (0f 38)
const uint32_t OP_PSHUFB_Pq_Qq            = 0x0f3800;
const uint32_t OP_PSHUFB_Vdq_Wdq          = 0x660f3800;
const uint32_t OP_PHADDW_Pq_Qq            = 0x0f3801;
const uint32_t OP_PHADDW_Vdq_Wdq          = 0x660f3801;
const uint32_t OP_PHADDD_Pq_Qq            = 0x0f3802;
const uint32_t OP_PHADDD_Vdq_Wdq          = 0x660f3802;
const uint32_t OP_PHADDSW_Pq_Qq           = 0x0f3803;
const uint32_t OP_PHADDSW_Vdq_Wdq         = 0x660f3803;
const uint32_t OP_PMADDUBSW_Pq_Qq         = 0x0f3804;
const uint32_t OP_PMADDUBSW_Vdq_Wdq       = 0x660f3804;
const uint32_t OP_PHSUBW_Pq_Qq            = 0x0f3805;
const uint32_t OP_PHSUBW_Vdq_Wdq          = 0x660f3805;
const uint32_t OP_PHSUBD_Pq_Qq            = 0x0f3806;
const uint32_t OP_PHSUBD_Vdq_Wdq          = 0x660f3806;
const uint32_t OP_PHSUBSW_Pq_Qq           = 0x0f3807;
const uint32_t OP_PHSUBSW_Vdq_Wdq         = 0x660f3807;
const uint32_t OP_PSIGNB_Pq_Qq            = 0x0f3808;
const uint32_t OP_PSIGNB_Vdq_Wdq          = 0x660f3808;
const uint32_t OP_PSIGNW_Pq_Qq            = 0x0f3809;
const uint32_t OP_PSIGNW_Vdq_Wdq          = 0x660f3809;
const uint32_t OP_PSIGND_Pq_Qq            = 0x0f380a;
const uint32_t OP_PSIGND_Vdq_Wdq          = 0x660f380a;
const uint32_t OP_PMULHRSW_Pq_Qq          = 0x0f380b;
const uint32_t OP_PMULHRSW_Vdq_Wdq        = 0x660f380b;

const uint32_t OP_PBLENDVB_Vdq_Wdq        = 0x660f3810;
const uint32_t OP_PBLENDVPS_Vdq_Wdq       = 0x660f3814;
const uint32_t OP_PBLENDVPD_Vdq_Wdq       = 0x660f3815;
const uint32_t OP_PTEST_Vdq_Wdq           = 0x660f3817;
const uint32_t OP_PABSB_Pq_Qq             = 0x0f381c;
const uint32_t OP_PABSB_Vdq_Wdq           = 0x660f381c;
const uint32_t OP_PABSW_Pq_Qq             = 0x0f381d;
const uint32_t OP_PABSW_Vdq_Wdq           = 0x660f381d;
const uint32_t OP_PABSD_Pq_Qq             = 0x0f381e;
const uint32_t OP_PABSD_Vdq_Wdq           = 0x660f381e;

const uint32_t OP_PMOVSXBW_Vdq_Udq        = 0x660f3820;
const uint32_t OP_PMOVSXBD_Vdq_Udq        = 0x660f3821;
const uint32_t OP_PMOVSXBQ_Vdq_Udq        = 0x660f3822;
const uint32_t OP_PMOVSXWD_Vdq_Udq        = 0x660f3823;
const uint32_t OP_PMOVSXWQ_Vdq_Udq        = 0x660f3824;
const uint32_t OP_PMOVSXDQ_Vdq_Udq        = 0x660f3825;
const uint32_t OP_PMULDQ_Vdq_Udq          = 0x660f3828;
const uint32_t OP_PCMPEQQ_Vdq_Udq         = 0x660f3829;
const uint32_t OP_MOVNTDQA_Vdq_Udq        = 0x660f382a;
const uint32_t OP_PACKUSDW_Vdq_Udq        = 0x660f382b;

const uint32_t OP_PMOVZXBW_Vdq_Udq        = 0x660f3830;
const uint32_t OP_PMOVZXBD_Vdq_Udq        = 0x660f3831;
const uint32_t OP_PMOVZXBQ_Vdq_Udq        = 0x660f3832;
const uint32_t OP_PMOVZXWD_Vdq_Udq        = 0x660f3833;
const uint32_t OP_PMOVZXWQ_Vdq_Udq        = 0x660f3834;
const uint32_t OP_PMOVZXDQ_Vdq_Udq        = 0x660f3835;
const uint32_t OP_PMINSB_Vdq_Udq          = 0x660f3838;
const uint32_t OP_PMINSD_Vdq_Udq          = 0x660f3839;
const uint32_t OP_PMINUW_Vdq_Udq          = 0x660f383a;
const uint32_t OP_PMINUD_Vdq_Udq          = 0x660f383b;
const uint32_t OP_PMAXSB_Vdq_Udq          = 0x660f383c;
const uint32_t OP_PMAXSD_Vdq_Udq          = 0x660f383d;
const uint32_t OP_PMAXUW_Vdq_Udq          = 0x660f383e;
const uint32_t OP_PMAXUD_Vdq_Udq          = 0x660f383f;

const uint32_t OP_MULLD_Vdq_Wdq           = 0x660f3840;
const uint32_t OP_PHMINPOSUW_Vdq_Wdq      = 0x660f3841;

const uint32_t OP_NVEPT_Gd_Mdq            = 0x660f3880;
const uint32_t OP_NVVPID_Gd_Mdq           = 0x660f3881;

const uint32_t OP_MOVBE_Gv_Mv             = 0x0f38f0;
const uint32_t OP_CRC32_Gd_Eb             = 0xf20f38f0;
const uint32_t OP_MOVBE_Mv_Gv             = 0x0f38f1;
const uint32_t OP_CRC32_Gd_Ev             = 0xf20f38f1;


// triple byte opcodes (0f 3a)
const uint32_t OP_ROUNDPS_Vdq_Wdq_Ib      = 0x660f3a08;
const uint32_t OP_ROUNDPD_Vdq_Wdq_Ib      = 0x660f3a09;
const uint32_t OP_ROUNDSS_Vss_Wss_Ib      = 0x660f3a0a;
const uint32_t OP_ROUNDSD_Vsd_Wsd_Ib      = 0x660f3a0b;
const uint32_t OP_BLENDPS_Vdq_Wdq_Ib      = 0x660f3a0c;
const uint32_t OP_BLENDPD_Vdq_Wdq_Ib      = 0x660f3a0d;
const uint32_t OP_PBLENDW_Vdq_Wdq_Ib      = 0x660f3a0e;
const uint32_t OP_PALIGNR_Pq_Qq_Ib        = 0x0f3a0f;
const uint32_t OP_PALIGNR_Vdq_Wdq_Ib      = 0x660f3a0f;

const uint32_t OP_EXTRB_Rd_Vdq_Ib         = 0x660f3a14;
const uint32_t OP_EXTRW_Rd_Vdq_Ib         = 0x660f3a15;
const uint32_t OP_EXTRD_Rd_Vdq_Ib         = 0x660f3a16;
const uint32_t OP_EXTRACTPS_Ed_Vdq_Ib     = 0x660f3a17;

const uint32_t OP_PINSRB_Vdq_Rd_Ib        = 0x660f3a20;
const uint32_t OP_INSERTPS_Vdq_Udq_Ib     = 0x660f3a21;
const uint32_t OP_PINSRD_Vdq_Ed_Ib        = 0x660f3a22;

const uint32_t OP_DPPS_Vdq_Wdq_Ib         = 0x660f3a40;
const uint32_t OP_DPPD_Vdq_Wdq_Ib         = 0x660f3a41;
const uint32_t OP_MPSADBW_Vdq_Wdq_Ib      = 0x660f3a42;

const uint32_t OP_PCMPESTRM_Vdq_Wdq_Ib    = 0x660f3a60;
const uint32_t OP_PCMPESTRI_Vdq_Wdq_Ib    = 0x660f3a61;
const uint32_t OP_PCMPISTRM_Vdq_Wdq_Ib    = 0x660f3a62;
const uint32_t OP_PCMPISTRI_Vdq_Wdq_Ib    = 0x660f3a63;


// floating point opcodes
const uint32_t OP_FADD_ST0_STn            = 0xd8c0;
const uint32_t OP_FMUL_ST0_STn            = 0xd8c8;
const uint32_t OP_FCOM_ST0_STn            = 0xd8d0;
const uint32_t OP_FCOMP_ST0_STn           = 0xd8d8;
const uint32_t OP_FSUB_ST0_STn            = 0xd8e0;
const uint32_t OP_FSUBR_ST0_STn           = 0xd8e8;
const uint32_t OP_FDIV_ST0_STn            = 0xd8f0;
const uint32_t OP_FDIVR_ST0_STn           = 0xd8f8;
const uint32_t OP_FLD_ST0_STn             = 0xd9c0;
const uint32_t OP_FXCH_ST0_STn            = 0xd9c8;
const uint32_t OP_FNOP                    = 0xd9d0;
const uint32_t OP_FCHS                    = 0xd9e0;
const uint32_t OP_FABS                    = 0xd9e1;
const uint32_t OP_FTST                    = 0xd9e4;
const uint32_t OP_FXAM                    = 0xd9e5;
const uint32_t OP_FLD1                    = 0xd9e8;
const uint32_t OP_FLDL2T                  = 0xd9e9;
const uint32_t OP_FLDL2E                  = 0xd9ea;
const uint32_t OP_FLDPI                   = 0xd9eb;
const uint32_t OP_FLDLG2                  = 0xd9ec;
const uint32_t OP_FLDLN2                  = 0xd9ed;
const uint32_t OP_FLDZ                    = 0xd9ee;
const uint32_t OP_F2XM1                   = 0xd9f0;
const uint32_t OP_FYL2X                   = 0xd9f1;
const uint32_t OP_FPTAN                   = 0xd9f2;
const uint32_t OP_FPATAN                  = 0xd9f3;
const uint32_t OP_FXTRACT                 = 0xd9f4;
const uint32_t OP_FPREM1                  = 0xd9f5;
const uint32_t OP_FDECSTP                 = 0xd9f6;
const uint32_t OP_FINCSTP                 = 0xd9f7;
const uint32_t OP_FPREM                   = 0xd9f8;
const uint32_t OP_FYL2XP1                 = 0xd9f9;
const uint32_t OP_FSQRT                   = 0xd9fa;
const uint32_t OP_FSINCOS                 = 0xd9fb;
const uint32_t OP_FRNDINT                 = 0xd9fc;
const uint32_t OP_FSCALE                  = 0xd9fd;
const uint32_t OP_FSIN                    = 0xd9fe;
const uint32_t OP_FCOS                    = 0xd9ff;
const uint32_t OP_FCMOVB_ST0_STn          = 0xdac0;
const uint32_t OP_FCMOVE_ST0_STn          = 0xdac8;
const uint32_t OP_FCMOVBE_ST0_STn         = 0xdad0;
const uint32_t OP_FCMOVU_ST0_STn          = 0xdad8;
const uint32_t OP_FUCOMPP                 = 0xdae9;
const uint32_t OP_FCMOVNB_ST0_STn         = 0xdbc0;
const uint32_t OP_FCMOVNE_ST0_STn         = 0xdbc8;
const uint32_t OP_FCMOVNBE_ST0_STn        = 0xdbd0;
const uint32_t OP_FCMOVNU_ST0_STn         = 0xdbd8;
const uint32_t OP_FCLEX                   = 0xdbe2;
const uint32_t OP_FINIT                   = 0xdbe3;
const uint32_t OP_FUCOMI_ST0_STn          = 0xdbe8;
const uint32_t OP_FCOMI_ST0_STn           = 0xdbf0;
const uint32_t OP_FADD_STn_ST0            = 0xdcc0;
const uint32_t OP_FMUL_STn_ST0            = 0xdcc8;
const uint32_t OP_FSUBR_STn_ST0           = 0xdce0;
const uint32_t OP_FSUB_STn_ST0            = 0xdce8;
const uint32_t OP_FDIVR_STn_ST0           = 0xdcf0;
const uint32_t OP_FDIV_STn_ST0            = 0xdcf8;
const uint32_t OP_FFREE_STn               = 0xddc0;
const uint32_t OP_FST_STn                 = 0xddd0;
const uint32_t OP_FSTP_STn                = 0xddd8;
const uint32_t OP_FUCOM_STn_ST0           = 0xdde0;
const uint32_t OP_FUCOMP_STn              = 0xdde8;
const uint32_t OP_FADDP_STn_ST0           = 0xdec0;
const uint32_t OP_FMULP_STn_ST0           = 0xdec8;
const uint32_t OP_FCOMPP                  = 0xded9;
const uint32_t OP_FSUBRP_STn_ST0          = 0xdee0;
const uint32_t OP_FSUBP_STn_ST0           = 0xdee8;
const uint32_t OP_FDIVRP_STn_ST0          = 0xdef0;
const uint32_t OP_FDIVP_STn_ST0           = 0xdef8;
const uint32_t OP_FSTSW_AX                = 0xdfe0;
const uint32_t OP_FCOMIP_ST0_STn          = 0xdff0;



//**************************************************************************
//  MEMORY REFERENCES
//**************************************************************************

inline x86_memref MBD(uint8_t base, int32_t disp) { return x86_memref(base, REG_NONE, 1, disp); }
inline x86_memref MBISD(uint8_t base, uint8_t ind, uint8_t scale, int32_t disp) { return x86_memref(base, ind, scale, disp); }

#if (X86EMIT_SIZE == 32)
inline x86_memref MABS(const void *mem) { return x86_memref(REG_NONE, REG_NONE, 1, reinterpret_cast<uintptr_t>(const_cast<void *>(mem))); }
inline x86_memref MABSI(const void *mem, uint8_t index) { return x86_memref(index, REG_NONE, 1, reinterpret_cast<uintptr_t>(const_cast<void *>(mem))); }
inline x86_memref MABSI(const void *mem, uint8_t index, uint8_t scale) { return x86_memref(REG_NONE, index, scale, reinterpret_cast<uintptr_t>(const_cast<void *>(mem))); }
#endif



//**************************************************************************
//  CORE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  make_modrm - assemble a modrm byte from the
//  three components
//-------------------------------------------------

inline uint8_t make_modrm(uint8_t mode, uint8_t reg, uint8_t rm)
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

inline uint8_t make_sib(uint8_t scale, uint8_t index, uint8_t base)
{
	static const uint8_t scale_lookup[9] = { 0<<6, 0<<6, 1<<6, 0<<6, 2<<6, 0<<6, 0<<6, 0<<6, 3<<6 };
	assert(scale == 1 || scale == 2 || scale == 4 || scale == 8);
	assert(index < REG_MAX);
	assert(base < REG_MAX);
	return scale_lookup[scale] | ((index & 7) << 3) | (base & 7);
}


//-------------------------------------------------
//  emit_byte - emit a byte
//-------------------------------------------------

inline void emit_byte(x86code *&emitptr, uint8_t byte)
{
	*((uint8_t *)emitptr) = byte;
	emitptr += 1;
}


//-------------------------------------------------
//  emit_word - emit a word
//-------------------------------------------------

inline void emit_word(x86code *&emitptr, uint16_t word)
{
	*((uint16_t *)emitptr) = word;
	emitptr += 2;
}


//-------------------------------------------------
//  emit_dword - emit a dword
//-------------------------------------------------

inline void emit_dword(x86code *&emitptr, uint32_t dword)
{
	*((uint32_t *)emitptr) = dword;
	emitptr += 4;
}


//-------------------------------------------------
//  emit_qword - emit a dword
//-------------------------------------------------

inline void emit_qword(x86code *&emitptr, uint64_t qword)
{
	*((uint64_t *)emitptr) = qword;
	emitptr += 8;
}



//**************************************************************************
//  GENERIC OPCODE EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_op - emit a 1, 2, or 3-byte opcode,
//  along with any necessary REX prefixes for x64
//-------------------------------------------------

inline void emit_op(x86code *&emitptr, uint32_t op, uint8_t opsize, uint8_t reg, uint8_t sib, uint8_t rm)
{
	if (opsize == OP_16BIT)
		emit_byte(emitptr, PREFIX_OPSIZE);

	bool has_prefix = (op & 0xff0000) == 0x660000 || (op & 0xff0000) == 0xf20000 || (op & 0xff0000) == 0xf30000;

	if (has_prefix)
		emit_byte(emitptr, op >> 16);

#if (X86EMIT_SIZE == 64)
{
	uint8_t rex;

	assert(opsize == OP_16BIT || opsize == OP_32BIT || opsize == OP_64BIT);

	rex = (opsize & 8) | ((reg & 8) >> 1) | ((sib & 8) >> 2) | ((rm & 8) >> 3);
	if (rex != 0 || ((op & OPFLAG_8BITREG) && reg >= 4) || ((op & OPFLAG_8BITRM) && rm >= 4))
		emit_byte(emitptr, OP_REX + rex);
}
#else
	assert(opsize != OP_64BIT);
#endif

	if ((op & 0xff0000) != 0 && !has_prefix)
		emit_byte(emitptr, op >> 16);
	if ((op & 0xff00) != 0)
		emit_byte(emitptr, op >> 8);
	emit_byte(emitptr, op);
}


//-------------------------------------------------
//  emit_op_simple - emit a simple opcode
//-------------------------------------------------

inline void emit_op_simple(x86code *&emitptr, uint32_t op, uint8_t opsize)
{
	emit_op(emitptr, op, opsize, 0, 0, 0);
}


//-------------------------------------------------
//  emit_op_reg - emit a simple opcode that has
//  a register parameter
//-------------------------------------------------

inline void emit_op_reg(x86code *&emitptr, uint32_t op, uint8_t opsize, uint8_t reg)
{
	emit_op(emitptr, op, opsize, 0, 0, reg);
}


//-------------------------------------------------
//  emit_op_modrm_reg - emit an opcode with a
//  register modrm byte
//-------------------------------------------------

inline void emit_op_modrm_reg(x86code *&emitptr, uint32_t op, uint8_t opsize, uint8_t reg, uint8_t rm)
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

inline void emit_op_modrm_mem(x86code *&emitptr, uint32_t op, uint8_t opsize, uint8_t reg, x86_memref memref)
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
		else if ((int8_t)memref.m_disp == memref.m_disp)
		{
			emit_byte(emitptr, make_modrm(1, reg, memref.m_base));
			emit_byte(emitptr, (int8_t)memref.m_disp);
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
		else if ((int8_t)memref.m_disp == memref.m_disp)
		{
			emit_byte(emitptr, make_modrm(1, reg, 4));
			emit_byte(emitptr, make_sib(memref.m_scale, memref.m_index, memref.m_base));
			emit_byte(emitptr, (int8_t)memref.m_disp);
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

inline void emit_op_modrm_reg_imm8(x86code *&emitptr, uint32_t op, uint8_t opsize, uint8_t reg, uint8_t rm, uint8_t imm)
{
	emit_op_modrm_reg(emitptr, op, opsize, reg, rm);
	emit_byte(emitptr, imm);
}


//-------------------------------------------------
//  emit_op_modrm_mem_imm8 - emit an opcode with a
//  memory modrm byte and an 8-bit immediate
//-------------------------------------------------

inline void emit_op_modrm_mem_imm8(x86code *&emitptr, uint32_t op, uint8_t opsize, uint8_t reg, x86_memref memref, uint8_t imm)
{
	emit_op_modrm_mem(emitptr, op, opsize, reg, memref);
	emit_byte(emitptr, imm);
}


//-------------------------------------------------
//  emit_op_modrm_reg_imm16 - emit an opcode with a
//  register modrm byte and a 16-bit immediate
//-------------------------------------------------

inline void emit_op_modrm_reg_imm16(x86code *&emitptr, uint32_t op, uint8_t opsize, uint8_t reg, uint8_t rm, uint16_t imm)
{
	emit_op_modrm_reg(emitptr, op, opsize, reg, rm);
	emit_word(emitptr, imm);
}


//-------------------------------------------------
//  emit_op_modrm_mem_imm16 - emit an opcode with a
//  memory modrm byte and a 16-bit immediate
//-------------------------------------------------

inline void emit_op_modrm_mem_imm16(x86code *&emitptr, uint32_t op, uint8_t opsize, uint8_t reg, x86_memref memref, uint16_t imm)
{
	emit_op_modrm_mem(emitptr, op, opsize, reg, memref);
	emit_word(emitptr, imm);
}


//-------------------------------------------------
//  emit_op_modrm_reg_imm816 - emit an opcode with
//  a register modrm byte and an 8-bit or 16-bit
//  immediate
//-------------------------------------------------

inline void emit_op_modrm_reg_imm816(x86code *&emitptr, uint32_t op8, uint32_t op16, uint8_t opsize, uint8_t reg, uint8_t rm, uint16_t imm)
{
	if ((int8_t)imm == (int16_t)imm)
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

inline void emit_op_modrm_mem_imm816(x86code *&emitptr, uint32_t op8, uint32_t op16, uint8_t opsize, uint8_t reg, x86_memref memref, uint16_t imm)
{
	if ((int8_t)imm == (int16_t)imm)
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

inline void emit_op_modrm_reg_imm32(x86code *&emitptr, uint32_t op, uint8_t opsize, uint8_t reg, uint8_t rm, uint32_t imm)
{
	emit_op_modrm_reg(emitptr, op, opsize, reg, rm);
	emit_dword(emitptr, imm);
}


//-------------------------------------------------
//  emit_op_modrm_mem_imm32 - emit an opcode with a
//  memory modrm byte and a 32-bit immediate
//-------------------------------------------------

inline void emit_op_modrm_mem_imm32(x86code *&emitptr, uint32_t op, uint8_t opsize, uint8_t reg, x86_memref memref, uint32_t imm)
{
	emit_op_modrm_mem(emitptr, op, opsize, reg, memref);
	emit_dword(emitptr, imm);
}


//-------------------------------------------------
//  emit_op_modrm_reg_imm832 - emit an opcode with
//  a register modrm byte and an 8-bit or 32-bit
//  immediate
//-------------------------------------------------

inline void emit_op_modrm_reg_imm832(x86code *&emitptr, uint32_t op8, uint32_t op32, uint8_t opsize, uint8_t reg, uint8_t rm, uint32_t imm)
{
	if ((int8_t)imm == (int32_t)imm)
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

inline void emit_op_modrm_mem_imm832(x86code *&emitptr, uint32_t op8, uint32_t op32, uint8_t opsize, uint8_t reg, x86_memref memref, uint32_t imm)
{
	if ((int8_t)imm == (int32_t)imm)
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
	int64_t delta = destptr - linkinfo.target;
	if (linkinfo.size == 1)
	{
		assert((int8_t)delta == delta);
		((int8_t *)linkinfo.target)[-1] = (int8_t)delta;
	}
	else if (linkinfo.size == 2)
	{
		assert((int16_t)delta == delta);
		((int16_t *)linkinfo.target)[-1] = (int16_t)delta;
	}
	else
	{
		assert((int32_t)delta == delta);
		((int32_t *)linkinfo.target)[-1] = (int32_t)delta;
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
	int32_t delta = target - (emitptr + 2);
	emit_link link;

	if ((int8_t)delta == delta)
		emit_jmp_short_link(emitptr, link);
	else
		emit_jmp_near_link(emitptr, link);
	resolve_link(target, link);
}


//-------------------------------------------------
//  emit_jcc_*
//-------------------------------------------------

inline void emit_jcc_short_link(x86code *&emitptr, uint8_t cond, emit_link &linkinfo)
{
	emit_op_simple(emitptr, OP_JCC_O_Jb + cond, OP_32BIT);
	emit_byte(emitptr, 0);
	linkinfo.target = emitptr;
	linkinfo.size = 1;
}

inline void emit_jcc_near_link(x86code *&emitptr, uint8_t cond, emit_link &linkinfo)
{
	emit_op_simple(emitptr, OP_JCC_O_Jv + cond, OP_32BIT);
	emit_dword(emitptr, 0);
	linkinfo.target = emitptr;
	linkinfo.size = 4;
}

inline void emit_jcc(x86code *&emitptr, uint8_t cond, x86code *target)
{
	int32_t delta = emitptr + 2 - target;
	emit_link link;

	if ((int8_t)delta == delta)
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
inline void emit_call_r32(x86code *&emitptr, uint8_t dreg)        { emit_op_modrm_reg(emitptr, OP_G5, OP_32BIT, 2, dreg); }
inline void emit_call_m32(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 2, memref); }
inline void emit_jmp_r32(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G5, OP_32BIT, 4, dreg); }
inline void emit_jmp_m32(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 4, memref); }
#else
inline void emit_call_r64(x86code *&emitptr, uint8_t dreg)        { emit_op_modrm_reg(emitptr, OP_G5, OP_32BIT, 2, dreg); }
inline void emit_call_m64(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 2, memref); }
inline void emit_jmp_r64(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G5, OP_32BIT, 4, dreg); }
inline void emit_jmp_m64(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 4, memref); }
#endif


//-------------------------------------------------
//  emit_ret_*
//-------------------------------------------------

inline void emit_ret_imm(x86code *&emitptr, uint16_t imm)
{
	emit_op_simple(emitptr, OP_RETN_Iw, OP_32BIT);
	emit_word(emitptr, imm);
}



//**************************************************************************
//  PUSH/POP EMITTERS
//**************************************************************************

inline void emit_push_imm(x86code *&emitptr, int32_t imm)
{
	if ((int8_t)imm == imm)
	{
		emit_op_simple(emitptr, OP_PUSH_Ib, OP_32BIT);
		emit_byte(emitptr, (int8_t)imm);
	}
	else
	{
		emit_op_simple(emitptr, OP_PUSH_Iz, OP_32BIT);
		emit_dword(emitptr, imm);
	}
}

#if (X86EMIT_SIZE == 32)

inline void emit_push_r32(x86code *&emitptr, uint8_t reg)                         { emit_op_reg(emitptr, OP_PUSH_rAX + (reg & 7), OP_32BIT, reg); }
inline void emit_push_m32(x86code *&emitptr, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 6, memref); }
inline void emit_pop_r32(x86code *&emitptr, uint8_t reg)                          { emit_op_reg(emitptr, OP_POP_rAX + (reg & 7), OP_32BIT, reg); }
inline void emit_pop_m32(x86code *&emitptr, x86_memref memref)                  { emit_op_modrm_mem(emitptr, OP_G1A_Ev, OP_32BIT, 0, memref); }

#else

inline void emit_push_r64(x86code *&emitptr, uint8_t reg)                         { emit_op_reg(emitptr, OP_PUSH_rAX + (reg & 7), OP_32BIT, reg); }
inline void emit_push_m64(x86code *&emitptr, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 6, memref); }
inline void emit_pop_r64(x86code *&emitptr, uint8_t reg)                          { emit_op_reg(emitptr, OP_POP_rAX + (reg & 7), OP_32BIT, reg); }
inline void emit_pop_m64(x86code *&emitptr, x86_memref memref)                  { emit_op_modrm_mem(emitptr, OP_G1A_Ev, OP_32BIT, 0, memref); }

#endif



//**************************************************************************
//  MOVE EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_mov_r8_*
//-------------------------------------------------

inline void emit_mov_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)
{
	emit_op_reg(emitptr, OP_MOV_AL_Ib | (dreg & 7), OP_32BIT, dreg);
	emit_byte(emitptr, imm);
}

inline void emit_mov_r8_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_MOV_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_mov_r8_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_MOV_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_mov_m8_r8(x86code *&emitptr, x86_memref memref, uint8_t sreg)    { emit_op_modrm_mem(emitptr, OP_MOV_Eb_Gb, OP_32BIT, sreg, memref); }
inline void emit_mov_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G11_Eb_Ib, OP_32BIT, 0, memref, imm); }


//-------------------------------------------------
//  emit_xchg_r8_*
//-------------------------------------------------

inline void emit_xchg_r8_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)
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

inline void emit_mov_r16_imm(x86code *&emitptr, uint8_t dreg, uint16_t imm)
{
	emit_op_reg(emitptr, OP_MOV_rAX_Iv | (dreg & 7), OP_16BIT, dreg);
	emit_word(emitptr, imm);
}

inline void emit_mov_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                         { emit_op_modrm_reg(emitptr, OP_MOV_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_mov_r16_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)                  { emit_op_modrm_mem(emitptr, OP_MOV_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_mov_m16_imm(x86code *&emitptr, x86_memref memref, uint16_t imm)                  { emit_op_modrm_mem_imm16(emitptr, OP_G11_Ev_Iz, OP_16BIT, 0, memref, imm); }
inline void emit_mov_m16_r16(x86code *&emitptr, x86_memref memref, uint8_t sreg)                  { emit_op_modrm_mem(emitptr, OP_MOV_Ev_Gv, OP_16BIT, sreg, memref); }
inline void emit_movsx_r16_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                        { emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Eb, OP_16BIT, dreg, sreg); }
inline void emit_movsx_r16_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Eb, OP_16BIT, dreg, memref); }
inline void emit_movzx_r16_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                        { emit_op_modrm_reg(emitptr, OP_MOVZX_Gv_Eb, OP_16BIT, dreg, sreg); }
inline void emit_movzx_r16_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Eb, OP_16BIT, dreg, memref); }
inline void emit_cmovcc_r16_r16(x86code *&emitptr, uint8_t cond, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_16BIT, dreg, sreg); }
inline void emit_cmovcc_r16_m16(x86code *&emitptr, uint8_t cond, uint8_t dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_16BIT, dreg, memref); }


//-------------------------------------------------
//  emit_xchg_r16_*
//-------------------------------------------------

inline void emit_xchg_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)
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

inline void emit_mov_r32_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)
{
	emit_op_reg(emitptr, OP_MOV_rAX_Iv | (dreg & 7), OP_32BIT, dreg);
	emit_dword(emitptr, imm);
}

inline void emit_mov_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                         { emit_op_modrm_reg(emitptr, OP_MOV_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_mov_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)                  { emit_op_modrm_mem(emitptr, OP_MOV_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_mov_m32_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)                  { emit_op_modrm_mem_imm32(emitptr, OP_G11_Ev_Iz, OP_32BIT, 0, memref, imm); }
inline void emit_mov_m32_r32(x86code *&emitptr, x86_memref memref, uint8_t sreg)                  { emit_op_modrm_mem(emitptr, OP_MOV_Ev_Gv, OP_32BIT, sreg, memref); }
inline void emit_movsx_r32_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                        { emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Eb, OP_32BIT, dreg, sreg); }
inline void emit_movsx_r32_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Eb, OP_32BIT, dreg, memref); }
inline void emit_movsx_r32_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                       { emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Ew, OP_32BIT, dreg, sreg); }
inline void emit_movsx_r32_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Ew, OP_32BIT, dreg, memref); }
inline void emit_movzx_r32_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)
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
inline void emit_movzx_r32_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Eb, OP_32BIT, dreg, memref); }
inline void emit_movzx_r32_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                       { emit_op_modrm_reg(emitptr, OP_MOVZX_Gv_Ew, OP_32BIT, dreg, sreg); }
inline void emit_movzx_r32_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Ew, OP_32BIT, dreg, memref); }
inline void emit_cmovcc_r32_r32(x86code *&emitptr, uint8_t cond, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_32BIT, dreg, sreg); }
inline void emit_cmovcc_r32_m32(x86code *&emitptr, uint8_t cond, uint8_t dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_32BIT, dreg, memref); }


//-------------------------------------------------
//  emit_xchg_r32_*
//-------------------------------------------------

inline void emit_xchg_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)
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

inline void emit_mov_r64_imm(x86code *&emitptr, uint8_t dreg, uint64_t imm)
{
	if ((uint32_t)imm == imm)
	{
		emit_op_reg(emitptr, OP_MOV_rAX_Iv | (dreg & 7), OP_32BIT, dreg);
		emit_dword(emitptr, imm);
	}
	else if ((int32_t)imm == imm)
		emit_op_modrm_reg_imm32(emitptr, OP_G11_Ev_Iz, OP_64BIT, 0, dreg, imm);
	else
	{
		emit_op_reg(emitptr, OP_MOV_rAX_Iv | (dreg & 7), OP_64BIT, dreg);
		emit_qword(emitptr, imm);
	}
}

inline void emit_mov_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                         { emit_op_modrm_reg(emitptr, OP_MOV_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_mov_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)                  { emit_op_modrm_mem(emitptr, OP_MOV_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_mov_m64_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)                  { emit_op_modrm_mem_imm32(emitptr, OP_G11_Ev_Iz, OP_64BIT, 0, memref, imm); }
inline void emit_mov_m64_r64(x86code *&emitptr, x86_memref memref, uint8_t sreg)                  { emit_op_modrm_mem(emitptr, OP_MOV_Ev_Gv, OP_64BIT, sreg, memref); }
inline void emit_movsx_r64_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                        { emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Eb, OP_64BIT, dreg, sreg); }
inline void emit_movsx_r64_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Eb, OP_64BIT, dreg, memref); }
inline void emit_movsx_r64_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                       { emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Ew, OP_64BIT, dreg, sreg); }
inline void emit_movsx_r64_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Ew, OP_64BIT, dreg, memref); }
inline void emit_movsxd_r64_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                      { emit_op_modrm_reg(emitptr, OP_MOVSXD_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_movsxd_r64_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_MOVSXD_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_movzx_r64_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                        { emit_op_modrm_reg(emitptr, OP_MOVZX_Gv_Eb, OP_64BIT, dreg, sreg); }
inline void emit_movzx_r64_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Eb, OP_64BIT, dreg, memref); }
inline void emit_movzx_r64_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                       { emit_op_modrm_reg(emitptr, OP_MOVZX_Gv_Ew, OP_64BIT, dreg, sreg); }
inline void emit_movzx_r64_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Ew, OP_64BIT, dreg, memref); }
inline void emit_cmovcc_r64_r64(x86code *&emitptr, uint8_t cond, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_64BIT, dreg, sreg); }
inline void emit_cmovcc_r64_m64(x86code *&emitptr, uint8_t cond, uint8_t dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_64BIT, dreg, memref); }


//-------------------------------------------------
//  emit_xchg_r64_*
//-------------------------------------------------

inline void emit_xchg_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)
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

inline void emit_add_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)           { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 0, dreg, imm); }
inline void emit_add_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 0, memref, imm); }
inline void emit_add_r8_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_ADD_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_add_r8_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ADD_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_add_m8_r8(x86code *&emitptr, x86_memref memref, uint8_t sreg)    { emit_op_modrm_mem(emitptr, OP_ADD_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_or_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)            { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 1, dreg, imm); }
inline void emit_or_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)     { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 1, memref, imm); }
inline void emit_or_r8_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)            { emit_op_modrm_reg(emitptr, OP_OR_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_or_r8_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_OR_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_or_m8_r8(x86code *&emitptr, x86_memref memref, uint8_t sreg)     { emit_op_modrm_mem(emitptr, OP_OR_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_adc_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)           { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 2, dreg, imm); }
inline void emit_adc_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 2, memref, imm); }
inline void emit_adc_r8_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_ADC_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_adc_r8_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_ADC_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_adc_m8_r8(x86code *&emitptr, x86_memref memref, uint8_t sreg)    { emit_op_modrm_mem(emitptr, OP_ADC_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_sbb_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)           { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 3, dreg, imm); }
inline void emit_sbb_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 3, memref, imm); }
inline void emit_sbb_r8_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_SBB_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_sbb_r8_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_SBB_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_sbb_m8_r8(x86code *&emitptr, x86_memref memref, uint8_t sreg)    { emit_op_modrm_mem(emitptr, OP_SBB_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_and_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)           { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 4, dreg, imm); }
inline void emit_and_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 4, memref, imm); }
inline void emit_and_r8_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_AND_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_and_r8_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_AND_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_and_m8_r8(x86code *&emitptr, x86_memref memref, uint8_t sreg)    { emit_op_modrm_mem(emitptr, OP_AND_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_sub_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)           { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 5, dreg, imm); }
inline void emit_sub_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 5, memref, imm); }
inline void emit_sub_r8_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_SUB_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_sub_r8_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_SUB_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_sub_m8_r8(x86code *&emitptr, x86_memref memref, uint8_t sreg)    { emit_op_modrm_mem(emitptr, OP_SUB_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_xor_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)           { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 6, dreg, imm); }
inline void emit_xor_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 6, memref, imm); }
inline void emit_xor_r8_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_XOR_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_xor_r8_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_XOR_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_xor_m8_r8(x86code *&emitptr, x86_memref memref, uint8_t sreg)    { emit_op_modrm_mem(emitptr, OP_XOR_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_cmp_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)           { emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 7, dreg, imm); }
inline void emit_cmp_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 7, memref, imm); }
inline void emit_cmp_r8_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_CMP_Gb_Eb, OP_32BIT, dreg, sreg); }
inline void emit_cmp_r8_m8(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CMP_Gb_Eb, OP_32BIT, dreg, memref); }
inline void emit_cmp_m8_r8(x86code *&emitptr, x86_memref memref, uint8_t sreg)    { emit_op_modrm_mem(emitptr, OP_CMP_Eb_Gb, OP_32BIT, sreg, memref); }

inline void emit_test_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_op_modrm_reg_imm8(emitptr, OP_G3_Eb, OP_32BIT, 0, dreg, imm); }
inline void emit_test_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_op_modrm_mem_imm8(emitptr, OP_G3_Eb, OP_32BIT, 0, memref, imm); }
inline void emit_test_r8_r8(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_TEST_Eb_Gb, OP_32BIT, sreg, dreg); }
inline void emit_test_m8_r8(x86code *&emitptr, x86_memref memref, uint8_t sreg)   { emit_op_modrm_mem(emitptr, OP_TEST_Eb_Gb, OP_32BIT, sreg, memref); }


//-------------------------------------------------
//  emit_arith_r16_*
//-------------------------------------------------

inline void emit_add_r16_imm(x86code *&emitptr, uint8_t dreg, uint16_t imm)         { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 0, dreg, imm); }
inline void emit_add_m16_imm(x86code *&emitptr, x86_memref memref, uint16_t imm)  { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 0, memref, imm); }
inline void emit_add_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_ADD_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_add_r16_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_ADD_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_add_m16_r16(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_ADD_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_or_r16_imm(x86code *&emitptr, uint8_t dreg, uint16_t imm)          { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 1, dreg, imm); }
inline void emit_or_m16_imm(x86code *&emitptr, x86_memref memref, uint16_t imm)   { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 1, memref, imm); }
inline void emit_or_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_OR_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_or_r16_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_OR_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_or_m16_r16(x86code *&emitptr, x86_memref memref, uint8_t sreg)   { emit_op_modrm_mem(emitptr, OP_OR_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_adc_r16_imm(x86code *&emitptr, uint8_t dreg, uint16_t imm)         { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 2, dreg, imm); }
inline void emit_adc_m16_imm(x86code *&emitptr, x86_memref memref, uint16_t imm)  { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 2, memref, imm); }
inline void emit_adc_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_ADC_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_adc_r16_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_ADC_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_adc_m16_r16(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_ADC_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_sbb_r16_imm(x86code *&emitptr, uint8_t dreg, uint16_t imm)         { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 3, dreg, imm); }
inline void emit_sbb_m16_imm(x86code *&emitptr, x86_memref memref, uint16_t imm)  { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 3, memref, imm); }
inline void emit_sbb_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_SBB_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_sbb_r16_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_SBB_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_sbb_m16_r16(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_SBB_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_and_r16_imm(x86code *&emitptr, uint8_t dreg, uint16_t imm)         { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 4, dreg, imm); }
inline void emit_and_m16_imm(x86code *&emitptr, x86_memref memref, uint16_t imm)  { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 4, memref, imm); }
inline void emit_and_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_AND_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_and_r16_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_AND_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_and_m16_r16(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_AND_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_sub_r16_imm(x86code *&emitptr, uint8_t dreg, uint16_t imm)         { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 5, dreg, imm); }
inline void emit_sub_m16_imm(x86code *&emitptr, x86_memref memref, uint16_t imm)  { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 5, memref, imm); }
inline void emit_sub_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_SUB_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_sub_r16_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_SUB_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_sub_m16_r16(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_SUB_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_xor_r16_imm(x86code *&emitptr, uint8_t dreg, uint16_t imm)         { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 6, dreg, imm); }
inline void emit_xor_m16_imm(x86code *&emitptr, x86_memref memref, uint16_t imm)  { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 6, memref, imm); }
inline void emit_xor_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_XOR_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_xor_r16_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_XOR_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_xor_m16_r16(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_XOR_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_cmp_r16_imm(x86code *&emitptr, uint8_t dreg, uint16_t imm)         { emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 7, dreg, imm); }
inline void emit_cmp_m16_imm(x86code *&emitptr, x86_memref memref, uint16_t imm)  { emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 7, memref, imm); }
inline void emit_cmp_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_CMP_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_cmp_r16_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_CMP_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_cmp_m16_r16(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_CMP_Ev_Gv, OP_16BIT, sreg, memref); }

inline void emit_test_r16_imm(x86code *&emitptr, uint8_t dreg, uint16_t imm)        { emit_op_modrm_reg_imm16(emitptr, OP_G3_Ev, OP_16BIT, 0, dreg, imm); }
inline void emit_test_m16_imm(x86code *&emitptr, x86_memref memref, uint16_t imm) { emit_op_modrm_mem_imm16(emitptr, OP_G3_Ev, OP_16BIT, 0, memref, imm); }
inline void emit_test_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)        { emit_op_modrm_reg(emitptr, OP_TEST_Ev_Gv, OP_16BIT, sreg, dreg); }
inline void emit_test_m16_r16(x86code *&emitptr, x86_memref memref, uint8_t sreg) { emit_op_modrm_mem(emitptr, OP_TEST_Ev_Gv, OP_16BIT, sreg, memref); }


//-------------------------------------------------
//  emit_arith_r32_*
//-------------------------------------------------

inline void emit_add_r32_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 0, dreg, imm); }
inline void emit_add_m32_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 0, memref, imm); }
inline void emit_add_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_ADD_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_add_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_ADD_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_add_m32_r32(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_ADD_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_or_r32_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)          { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 1, dreg, imm); }
inline void emit_or_m32_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)   { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 1, memref, imm); }
inline void emit_or_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_OR_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_or_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_OR_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_or_m32_r32(x86code *&emitptr, x86_memref memref, uint8_t sreg)   { emit_op_modrm_mem(emitptr, OP_OR_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_adc_r32_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 2, dreg, imm); }
inline void emit_adc_m32_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 2, memref, imm); }
inline void emit_adc_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_ADC_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_adc_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_ADC_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_adc_m32_r32(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_ADC_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_sbb_r32_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 3, dreg, imm); }
inline void emit_sbb_m32_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 3, memref, imm); }
inline void emit_sbb_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_SBB_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_sbb_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_SBB_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_sbb_m32_r32(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_SBB_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_and_r32_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 4, dreg, imm); }
inline void emit_and_m32_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 4, memref, imm); }
inline void emit_and_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_AND_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_and_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_AND_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_and_m32_r32(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_AND_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_sub_r32_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 5, dreg, imm); }
inline void emit_sub_m32_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 5, memref, imm); }
inline void emit_sub_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_SUB_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_sub_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_SUB_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_sub_m32_r32(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_SUB_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_xor_r32_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 6, dreg, imm); }
inline void emit_xor_m32_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 6, memref, imm); }
inline void emit_xor_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_XOR_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_xor_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_XOR_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_xor_m32_r32(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_XOR_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_cmp_r32_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 7, dreg, imm); }
inline void emit_cmp_m32_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 7, memref, imm); }
inline void emit_cmp_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_CMP_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_cmp_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_CMP_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_cmp_m32_r32(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_CMP_Ev_Gv, OP_32BIT, sreg, memref); }

inline void emit_test_r32_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)        { emit_op_modrm_reg_imm32(emitptr, OP_G3_Ev, OP_32BIT, 0, dreg, imm); }
inline void emit_test_m32_imm(x86code *&emitptr, x86_memref memref, uint32_t imm) { emit_op_modrm_mem_imm32(emitptr, OP_G3_Ev, OP_32BIT, 0, memref, imm); }
inline void emit_test_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)        { emit_op_modrm_reg(emitptr, OP_TEST_Ev_Gv, OP_32BIT, sreg, dreg); }
inline void emit_test_m32_r32(x86code *&emitptr, x86_memref memref, uint8_t sreg) { emit_op_modrm_mem(emitptr, OP_TEST_Ev_Gv, OP_32BIT, sreg, memref); }


//-------------------------------------------------
//  emit_arith_r64_*
//-------------------------------------------------

#if (X86EMIT_SIZE == 64)

inline void emit_add_r64_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 0, dreg, imm); }
inline void emit_add_m64_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 0, memref, imm); }
inline void emit_add_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_ADD_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_add_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_ADD_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_add_m64_r64(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_ADD_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_or_r64_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)          { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 1, dreg, imm); }
inline void emit_or_m64_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)   { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 1, memref, imm); }
inline void emit_or_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_OR_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_or_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_OR_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_or_m64_r64(x86code *&emitptr, x86_memref memref, uint8_t sreg)   { emit_op_modrm_mem(emitptr, OP_OR_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_adc_r64_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 2, dreg, imm); }
inline void emit_adc_m64_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 2, memref, imm); }
inline void emit_adc_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_ADC_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_adc_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_ADC_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_adc_m64_r64(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_ADC_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_sbb_r64_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 3, dreg, imm); }
inline void emit_sbb_m64_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 3, memref, imm); }
inline void emit_sbb_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_SBB_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_sbb_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_SBB_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_sbb_m64_r64(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_SBB_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_and_r64_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 4, dreg, imm); }
inline void emit_and_m64_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 4, memref, imm); }
inline void emit_and_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_AND_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_and_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_AND_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_and_m64_r64(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_AND_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_sub_r64_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 5, dreg, imm); }
inline void emit_sub_m64_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 5, memref, imm); }
inline void emit_sub_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_SUB_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_sub_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_SUB_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_sub_m64_r64(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_SUB_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_xor_r64_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 6, dreg, imm); }
inline void emit_xor_m64_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 6, memref, imm); }
inline void emit_xor_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_XOR_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_xor_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_XOR_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_xor_m64_r64(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_XOR_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_cmp_r64_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 7, dreg, imm); }
inline void emit_cmp_m64_imm(x86code *&emitptr, x86_memref memref, uint32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 7, memref, imm); }
inline void emit_cmp_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_CMP_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_cmp_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_CMP_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_cmp_m64_r64(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_CMP_Ev_Gv, OP_64BIT, sreg, memref); }

inline void emit_test_r64_imm(x86code *&emitptr, uint8_t dreg, uint32_t imm)        { emit_op_modrm_reg_imm32(emitptr, OP_G3_Ev, OP_64BIT, 0, dreg, imm); }
inline void emit_test_m64_imm(x86code *&emitptr, x86_memref memref, uint32_t imm) { emit_op_modrm_mem_imm32(emitptr, OP_G3_Ev, OP_64BIT, 0, memref, imm); }
inline void emit_test_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)        { emit_op_modrm_reg(emitptr, OP_TEST_Ev_Gv, OP_64BIT, sreg, dreg); }
inline void emit_test_m64_r64(x86code *&emitptr, x86_memref memref, uint8_t sreg) { emit_op_modrm_mem(emitptr, OP_TEST_Ev_Gv, OP_64BIT, sreg, memref); }

#endif



//**************************************************************************
//  SHIFT EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_shift_reg_imm
//-------------------------------------------------

inline void emit_shift_reg_imm(x86code *&emitptr, uint32_t op1, uint32_t opn, uint8_t opsize, uint8_t opindex, uint8_t dreg, uint8_t imm)
{
	if (imm == 1)
		emit_op_modrm_reg(emitptr, op1, opsize, opindex, dreg);
	else
		emit_op_modrm_reg_imm8(emitptr, opn, opsize, opindex, dreg, imm);
}

inline void emit_shift_mem_imm(x86code *&emitptr, uint32_t op1, uint32_t opn, uint8_t opsize, uint8_t opindex, x86_memref memref, uint8_t imm)
{
	if (imm == 1)
		emit_op_modrm_mem(emitptr, op1, opsize, opindex, memref);
	else
		emit_op_modrm_mem_imm8(emitptr, opn, opsize, opindex, memref, imm);
}


//-------------------------------------------------
//  emit_shift_r8_*
//-------------------------------------------------

inline void emit_rol_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)           { emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 0, dreg, imm); }
inline void emit_rol_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 0, memref, imm); }
inline void emit_rol_r8_cl(x86code *&emitptr, uint8_t dreg)                       { emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 0, dreg); }
inline void emit_rol_m8_cl(x86code *&emitptr, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 0, memref); }

inline void emit_ror_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)           { emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 1, dreg, imm); }
inline void emit_ror_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 1, memref, imm); }
inline void emit_ror_r8_cl(x86code *&emitptr, uint8_t dreg)                       { emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 1, dreg); }
inline void emit_ror_m8_cl(x86code *&emitptr, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 1, memref); }

inline void emit_rcl_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)           { emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 2, dreg, imm); }
inline void emit_rcl_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 2, memref, imm); }
inline void emit_rcl_r8_cl(x86code *&emitptr, uint8_t dreg)                       { emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 2, dreg); }
inline void emit_rcl_m8_cl(x86code *&emitptr, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 2, memref); }

inline void emit_rcr_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)           { emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 3, dreg, imm); }
inline void emit_rcr_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 3, memref, imm); }
inline void emit_rcr_r8_cl(x86code *&emitptr, uint8_t dreg)                       { emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 3, dreg); }
inline void emit_rcr_m8_cl(x86code *&emitptr, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 3, memref); }

inline void emit_shl_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)           { emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 4, dreg, imm); }
inline void emit_shl_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 4, memref, imm); }
inline void emit_shl_r8_cl(x86code *&emitptr, uint8_t dreg)                       { emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 4, dreg); }
inline void emit_shl_m8_cl(x86code *&emitptr, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 4, memref); }

inline void emit_shr_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)           { emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 5, dreg, imm); }
inline void emit_shr_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 5, memref, imm); }
inline void emit_shr_r8_cl(x86code *&emitptr, uint8_t dreg)                       { emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 5, dreg); }
inline void emit_shr_m8_cl(x86code *&emitptr, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 5, memref); }

inline void emit_sar_r8_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)           { emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 7, dreg, imm); }
inline void emit_sar_m8_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)    { emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 7, memref, imm); }
inline void emit_sar_r8_cl(x86code *&emitptr, uint8_t dreg)                       { emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 7, dreg); }
inline void emit_sar_m8_cl(x86code *&emitptr, x86_memref memref)                { emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 7, memref); }


//-------------------------------------------------
//  emit_shift_r16_*
//-------------------------------------------------

inline void emit_rol_r16_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 0, dreg, imm); }
inline void emit_rol_m16_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 0, memref, imm); }
inline void emit_rol_r16_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 0, dreg); }
inline void emit_rol_m16_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 0, memref); }

inline void emit_ror_r16_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 1, dreg, imm); }
inline void emit_ror_m16_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 1, memref, imm); }
inline void emit_ror_r16_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 1, dreg); }
inline void emit_ror_m16_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 1, memref); }

inline void emit_rcl_r16_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 2, dreg, imm); }
inline void emit_rcl_m16_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 2, memref, imm); }
inline void emit_rcl_r16_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 2, dreg); }
inline void emit_rcl_m16_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 2, memref); }

inline void emit_rcr_r16_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 3, dreg, imm); }
inline void emit_rcr_m16_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 3, memref, imm); }
inline void emit_rcr_r16_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 3, dreg); }
inline void emit_rcr_m16_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 3, memref); }

inline void emit_shl_r16_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 4, dreg, imm); }
inline void emit_shl_m16_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 4, memref, imm); }
inline void emit_shl_r16_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 4, dreg); }
inline void emit_shl_m16_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 4, memref); }

inline void emit_shr_r16_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 5, dreg, imm); }
inline void emit_shr_m16_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 5, memref, imm); }
inline void emit_shr_r16_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 5, dreg); }
inline void emit_shr_m16_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 5, memref); }

inline void emit_sar_r16_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 7, dreg, imm); }
inline void emit_sar_m16_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 7, memref, imm); }
inline void emit_sar_r16_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 7, dreg); }
inline void emit_sar_m16_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 7, memref); }


//-------------------------------------------------
//  emit_shift_r32_*
//-------------------------------------------------

inline void emit_rol_r32_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 0, dreg, imm); }
inline void emit_rol_m32_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 0, memref, imm); }
inline void emit_rol_r32_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 0, dreg); }
inline void emit_rol_m32_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 0, memref); }

inline void emit_ror_r32_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 1, dreg, imm); }
inline void emit_ror_m32_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 1, memref, imm); }
inline void emit_ror_r32_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 1, dreg); }
inline void emit_ror_m32_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 1, memref); }

inline void emit_rcl_r32_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 2, dreg, imm); }
inline void emit_rcl_m32_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 2, memref, imm); }
inline void emit_rcl_r32_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 2, dreg); }
inline void emit_rcl_m32_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 2, memref); }

inline void emit_rcr_r32_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 3, dreg, imm); }
inline void emit_rcr_m32_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 3, memref, imm); }
inline void emit_rcr_r32_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 3, dreg); }
inline void emit_rcr_m32_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 3, memref); }

inline void emit_shl_r32_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 4, dreg, imm); }
inline void emit_shl_m32_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 4, memref, imm); }
inline void emit_shl_r32_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 4, dreg); }
inline void emit_shl_m32_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 4, memref); }

inline void emit_shr_r32_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 5, dreg, imm); }
inline void emit_shr_m32_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 5, memref, imm); }
inline void emit_shr_r32_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 5, dreg); }
inline void emit_shr_m32_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 5, memref); }

inline void emit_sar_r32_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 7, dreg, imm); }
inline void emit_sar_m32_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 7, memref, imm); }
inline void emit_sar_r32_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 7, dreg); }
inline void emit_sar_m32_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 7, memref); }

inline void emit_shld_r32_r32_imm(x86code *&emitptr, uint8_t dreg, uint8_t sreg, uint8_t imm)         { emit_op_modrm_reg_imm8(emitptr, OP_SHLD_Ev_Gv_Ib, OP_32BIT, sreg, dreg, imm); }
inline void emit_shld_m32_r32_imm(x86code *&emitptr, x86_memref memref, uint8_t sreg, uint8_t imm)  { emit_op_modrm_mem_imm8(emitptr, OP_SHLD_Ev_Gv_Ib, OP_32BIT, sreg, memref, imm); }
inline void emit_shld_r32_r32_cl(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                     { emit_op_modrm_reg(emitptr, OP_SHLD_Ev_Gv_CL, OP_32BIT, sreg, dreg); }
inline void emit_shld_m32_r32_cl(x86code *&emitptr, x86_memref memref, uint8_t sreg)              { emit_op_modrm_mem(emitptr, OP_SHLD_Ev_Gv_CL, OP_32BIT, sreg, memref); }

inline void emit_shrd_r32_r32_imm(x86code *&emitptr, uint8_t dreg, uint8_t sreg, uint8_t imm)         { emit_op_modrm_reg_imm8(emitptr, OP_SHRD_Ev_Gv_Ib, OP_32BIT, sreg, dreg, imm); }
inline void emit_shrd_m32_r32_imm(x86code *&emitptr, x86_memref memref, uint8_t sreg, uint8_t imm)  { emit_op_modrm_mem_imm8(emitptr, OP_SHRD_Ev_Gv_Ib, OP_32BIT, sreg, memref, imm); }
inline void emit_shrd_r32_r32_cl(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                     { emit_op_modrm_reg(emitptr, OP_SHRD_Ev_Gv_CL, OP_32BIT, sreg, dreg); }
inline void emit_shrd_m32_r32_cl(x86code *&emitptr, x86_memref memref, uint8_t sreg)              { emit_op_modrm_mem(emitptr, OP_SHRD_Ev_Gv_CL, OP_32BIT, sreg, memref); }


//-------------------------------------------------
//  emit_shift_r64_*
//-------------------------------------------------

#if (X86EMIT_SIZE == 64)

inline void emit_rol_r64_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 0, dreg, imm); }
inline void emit_rol_m64_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 0, memref, imm); }
inline void emit_rol_r64_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 0, dreg); }
inline void emit_rol_m64_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 0, memref); }

inline void emit_ror_r64_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 1, dreg, imm); }
inline void emit_ror_m64_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 1, memref, imm); }
inline void emit_ror_r64_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 1, dreg); }
inline void emit_ror_m64_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 1, memref); }

inline void emit_rcl_r64_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 2, dreg, imm); }
inline void emit_rcl_m64_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 2, memref, imm); }
inline void emit_rcl_r64_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 2, dreg); }
inline void emit_rcl_m64_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 2, memref); }

inline void emit_rcr_r64_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 3, dreg, imm); }
inline void emit_rcr_m64_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 3, memref, imm); }
inline void emit_rcr_r64_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 3, dreg); }
inline void emit_rcr_m64_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 3, memref); }

inline void emit_shl_r64_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 4, dreg, imm); }
inline void emit_shl_m64_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 4, memref, imm); }
inline void emit_shl_r64_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 4, dreg); }
inline void emit_shl_m64_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 4, memref); }

inline void emit_shr_r64_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 5, dreg, imm); }
inline void emit_shr_m64_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 5, memref, imm); }
inline void emit_shr_r64_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 5, dreg); }
inline void emit_shr_m64_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 5, memref); }

inline void emit_sar_r64_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 7, dreg, imm); }
inline void emit_sar_m64_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 7, memref, imm); }
inline void emit_sar_r64_cl(x86code *&emitptr, uint8_t dreg)                      { emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 7, dreg); }
inline void emit_sar_m64_cl(x86code *&emitptr, x86_memref memref)               { emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 7, memref); }

inline void emit_shld_r64_r64_imm(x86code *&emitptr, uint8_t dreg, uint8_t sreg, uint8_t imm)         { emit_op_modrm_reg_imm8(emitptr, OP_SHLD_Ev_Gv_Ib, OP_64BIT, sreg, dreg, imm); }
inline void emit_shld_m64_r64_imm(x86code *&emitptr, x86_memref memref, uint8_t sreg, uint8_t imm)  { emit_op_modrm_mem_imm8(emitptr, OP_SHLD_Ev_Gv_Ib, OP_64BIT, sreg, memref, imm); }
inline void emit_shld_r64_r64_cl(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                     { emit_op_modrm_reg(emitptr, OP_SHLD_Ev_Gv_CL, OP_64BIT, sreg, dreg); }
inline void emit_shld_m64_r64_cl(x86code *&emitptr, x86_memref memref, uint8_t sreg)              { emit_op_modrm_mem(emitptr, OP_SHLD_Ev_Gv_CL, OP_64BIT, sreg, memref); }

inline void emit_shrd_r64_r64_imm(x86code *&emitptr, uint8_t dreg, uint8_t sreg, uint8_t imm)         { emit_op_modrm_reg_imm8(emitptr, OP_SHRD_Ev_Gv_Ib, OP_64BIT, sreg, dreg, imm); }
inline void emit_shrd_m64_r64_imm(x86code *&emitptr, x86_memref memref, uint8_t sreg, uint8_t imm)  { emit_op_modrm_mem_imm8(emitptr, OP_SHRD_Ev_Gv_Ib, OP_64BIT, sreg, memref, imm); }
inline void emit_shrd_r64_r64_cl(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                     { emit_op_modrm_reg(emitptr, OP_SHRD_Ev_Gv_CL, OP_64BIT, sreg, dreg); }
inline void emit_shrd_m64_r64_cl(x86code *&emitptr, x86_memref memref, uint8_t sreg)              { emit_op_modrm_mem(emitptr, OP_SHRD_Ev_Gv_CL, OP_64BIT, sreg, memref); }

#endif



//**************************************************************************
//  GROUP3 EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_group3_r8_*
//-------------------------------------------------

inline void emit_not_r8(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 2, dreg); }
inline void emit_not_m8(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 2, memref); }

inline void emit_neg_r8(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 3, dreg); }
inline void emit_neg_m8(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 3, memref); }

inline void emit_mul_r8(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 4, dreg); }
inline void emit_mul_m8(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 4, memref); }

inline void emit_imul_r8(x86code *&emitptr, uint8_t dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 5, dreg); }
inline void emit_imul_m8(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 5, memref); }

inline void emit_div_r8(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 6, dreg); }
inline void emit_div_m8(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 6, memref); }

inline void emit_idiv_r8(x86code *&emitptr, uint8_t dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 7, dreg); }
inline void emit_idiv_m8(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 7, memref); }


//-------------------------------------------------
//  emit_group3_r16_*
//-------------------------------------------------

inline void emit_not_r16(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 2, dreg); }
inline void emit_not_m16(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 2, memref); }

inline void emit_neg_r16(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 3, dreg); }
inline void emit_neg_m16(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 3, memref); }

inline void emit_mul_r16(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 4, dreg); }
inline void emit_mul_m16(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 4, memref); }

inline void emit_imul_r16(x86code *&emitptr, uint8_t dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 5, dreg); }
inline void emit_imul_m16(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 5, memref); }

inline void emit_div_r16(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 6, dreg); }
inline void emit_div_m16(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 6, memref); }

inline void emit_idiv_r16(x86code *&emitptr, uint8_t dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 7, dreg); }
inline void emit_idiv_m16(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 7, memref); }


//-------------------------------------------------
//  emit_group3_r32_*
//-------------------------------------------------

inline void emit_not_r32(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 2, dreg); }
inline void emit_not_m32(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 2, memref); }

inline void emit_neg_r32(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 3, dreg); }
inline void emit_neg_m32(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 3, memref); }

inline void emit_mul_r32(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 4, dreg); }
inline void emit_mul_m32(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 4, memref); }

inline void emit_imul_r32(x86code *&emitptr, uint8_t dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 5, dreg); }
inline void emit_imul_m32(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 5, memref); }

inline void emit_div_r32(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 6, dreg); }
inline void emit_div_m32(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 6, memref); }

inline void emit_idiv_r32(x86code *&emitptr, uint8_t dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 7, dreg); }
inline void emit_idiv_m32(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 7, memref); }


//-------------------------------------------------
//  emit_group3_r64_*
//-------------------------------------------------

#if (X86EMIT_SIZE == 64)

inline void emit_not_r64(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 2, dreg); }
inline void emit_not_m64(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 2, memref); }

inline void emit_neg_r64(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 3, dreg); }
inline void emit_neg_m64(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 3, memref); }

inline void emit_mul_r64(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 4, dreg); }
inline void emit_mul_m64(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 4, memref); }

inline void emit_imul_r64(x86code *&emitptr, uint8_t dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 5, dreg); }
inline void emit_imul_m64(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 5, memref); }

inline void emit_div_r64(x86code *&emitptr, uint8_t dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 6, dreg); }
inline void emit_div_m64(x86code *&emitptr, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 6, memref); }

inline void emit_idiv_r64(x86code *&emitptr, uint8_t dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 7, dreg); }
inline void emit_idiv_m64(x86code *&emitptr, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 7, memref); }

#endif



//**************************************************************************
//  IMUL EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_imul_r16_*
//-------------------------------------------------

inline void emit_imul_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                        { emit_op_modrm_reg(emitptr, OP_IMUL_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_imul_r16_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_IMUL_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_imul_r16_r16_imm(x86code *&emitptr, uint8_t dreg, uint8_t sreg, int16_t imm)         { emit_op_modrm_reg_imm816(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_16BIT, dreg, sreg, imm); }
inline void emit_imul_r16_m16_imm(x86code *&emitptr, uint8_t dreg, x86_memref memref, int16_t imm)  { emit_op_modrm_mem_imm816(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_16BIT, dreg, memref, imm); }

inline void emit_imul_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                        { emit_op_modrm_reg(emitptr, OP_IMUL_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_imul_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_IMUL_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_imul_r32_r32_imm(x86code *&emitptr, uint8_t dreg, uint8_t sreg, int32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_32BIT, dreg, sreg, imm); }
inline void emit_imul_r32_m32_imm(x86code *&emitptr, uint8_t dreg, x86_memref memref, int32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_32BIT, dreg, memref, imm); }

#if (X86EMIT_SIZE == 64)

inline void emit_imul_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)                        { emit_op_modrm_reg(emitptr, OP_IMUL_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_imul_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)                 { emit_op_modrm_mem(emitptr, OP_IMUL_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_imul_r64_r64_imm(x86code *&emitptr, uint8_t dreg, uint8_t sreg, int32_t imm)         { emit_op_modrm_reg_imm832(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_64BIT, dreg, sreg, imm); }
inline void emit_imul_r64_m64_imm(x86code *&emitptr, uint8_t dreg, x86_memref memref, int32_t imm)  { emit_op_modrm_mem_imm832(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_64BIT, dreg, memref, imm); }

#endif



//**************************************************************************
//  BIT OPERATION EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_bswap_*
//-------------------------------------------------

inline void emit_bswap_r32(x86code *&emitptr, uint8_t dreg)                       { emit_op_reg(emitptr, OP_BSWAP_EAX + (dreg & 7), OP_32BIT, dreg); }
inline void emit_bswap_r64(x86code *&emitptr, uint8_t dreg)                       { emit_op_reg(emitptr, OP_BSWAP_EAX + (dreg & 7), OP_64BIT, dreg); }


//-------------------------------------------------
//  emit_bsr/bsf_r16_*
//-------------------------------------------------

inline void emit_bsf_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_BSF_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_bsf_r16_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_BSF_Gv_Ev, OP_16BIT, dreg, memref); }
inline void emit_bsr_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_BSR_Gv_Ev, OP_16BIT, dreg, sreg); }
inline void emit_bsr_r16_m16(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_BSR_Gv_Ev, OP_16BIT, dreg, memref); }

inline void emit_bsf_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_BSF_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_bsf_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_BSF_Gv_Ev, OP_32BIT, dreg, memref); }
inline void emit_bsr_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_BSR_Gv_Ev, OP_32BIT, dreg, sreg); }
inline void emit_bsr_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_BSR_Gv_Ev, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_bsf_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_BSF_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_bsf_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_BSF_Gv_Ev, OP_64BIT, dreg, memref); }
inline void emit_bsr_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_BSR_Gv_Ev, OP_64BIT, dreg, sreg); }
inline void emit_bsr_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_BSR_Gv_Ev, OP_64BIT, dreg, memref); }
#endif


//-------------------------------------------------
//  emit_bit_r16_*
//-------------------------------------------------

inline void emit_bt_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_BT_Ev_Gv, OP_16BIT, sreg, dreg); }
inline void emit_bt_m16_r16(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_BT_Ev_Gv, OP_16BIT, sreg, memref); }
inline void emit_bt_r16_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 4, dreg, imm); }
inline void emit_bt_m16_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 4, memref, imm); }

inline void emit_bts_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)        { emit_op_modrm_reg(emitptr, OP_BTS_Ev_Gv, OP_16BIT, sreg, dreg); }
inline void emit_bts_m16_r16(x86code *&emitptr, x86_memref memref, uint8_t sreg) { emit_op_modrm_mem(emitptr, OP_BTS_Ev_Gv, OP_16BIT, sreg, memref); }
inline void emit_bts_r16_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 5, dreg, imm); }
inline void emit_bts_m16_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 5, memref, imm); }

inline void emit_btr_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)        { emit_op_modrm_reg(emitptr, OP_BTR_Ev_Gv, OP_16BIT, sreg, dreg); }
inline void emit_btr_m16_r16(x86code *&emitptr, x86_memref memref, uint8_t sreg) { emit_op_modrm_mem(emitptr, OP_BTR_Ev_Gv, OP_16BIT, sreg, memref); }
inline void emit_btr_r16_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 6, dreg, imm); }
inline void emit_btr_m16_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 6, memref, imm); }

inline void emit_btc_r16_r16(x86code *&emitptr, uint8_t dreg, uint8_t sreg)        { emit_op_modrm_reg(emitptr, OP_BTC_Ev_Gv, OP_16BIT, sreg, dreg); }
inline void emit_btc_m16_r16(x86code *&emitptr, x86_memref memref, uint8_t sreg) { emit_op_modrm_mem(emitptr, OP_BTC_Ev_Gv, OP_16BIT, sreg, memref); }
inline void emit_btc_r16_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 7, dreg, imm); }
inline void emit_btc_m16_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 7, memref, imm); }


//-------------------------------------------------
//  emit_bit_r32_*
//-------------------------------------------------

inline void emit_bt_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_BT_Ev_Gv, OP_32BIT, sreg, dreg); }
inline void emit_bt_m32_r32(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_BT_Ev_Gv, OP_32BIT, sreg, memref); }
inline void emit_bt_r32_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 4, dreg, imm); }
inline void emit_bt_m32_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 4, memref, imm); }

inline void emit_bts_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)        { emit_op_modrm_reg(emitptr, OP_BTS_Ev_Gv, OP_32BIT, sreg, dreg); }
inline void emit_bts_m32_r32(x86code *&emitptr, x86_memref memref, uint8_t sreg) { emit_op_modrm_mem(emitptr, OP_BTS_Ev_Gv, OP_32BIT, sreg, memref); }
inline void emit_bts_r32_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 5, dreg, imm); }
inline void emit_bts_m32_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 5, memref, imm); }

inline void emit_btr_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)        { emit_op_modrm_reg(emitptr, OP_BTR_Ev_Gv, OP_32BIT, sreg, dreg); }
inline void emit_btr_m32_r32(x86code *&emitptr, x86_memref memref, uint8_t sreg) { emit_op_modrm_mem(emitptr, OP_BTR_Ev_Gv, OP_32BIT, sreg, memref); }
inline void emit_btr_r32_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 6, dreg, imm); }
inline void emit_btr_m32_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 6, memref, imm); }

inline void emit_btc_r32_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)        { emit_op_modrm_reg(emitptr, OP_BTC_Ev_Gv, OP_32BIT, sreg, dreg); }
inline void emit_btc_m32_r32(x86code *&emitptr, x86_memref memref, uint8_t sreg) { emit_op_modrm_mem(emitptr, OP_BTC_Ev_Gv, OP_32BIT, sreg, memref); }
inline void emit_btc_r32_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 7, dreg, imm); }
inline void emit_btc_m32_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 7, memref, imm); }


//-------------------------------------------------
//  emit_bit_r64_*
//-------------------------------------------------

#if (X86EMIT_SIZE == 64)

inline void emit_bt_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_BT_Ev_Gv, OP_64BIT, sreg, dreg); }
inline void emit_bt_m64_r64(x86code *&emitptr, x86_memref memref, uint8_t sreg)  { emit_op_modrm_mem(emitptr, OP_BT_Ev_Gv, OP_64BIT, sreg, memref); }
inline void emit_bt_r64_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)          { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 4, dreg, imm); }
inline void emit_bt_m64_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)   { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 4, memref, imm); }

inline void emit_bts_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)        { emit_op_modrm_reg(emitptr, OP_BTS_Ev_Gv, OP_64BIT, sreg, dreg); }
inline void emit_bts_m64_r64(x86code *&emitptr, x86_memref memref, uint8_t sreg) { emit_op_modrm_mem(emitptr, OP_BTS_Ev_Gv, OP_64BIT, sreg, memref); }
inline void emit_bts_r64_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 5, dreg, imm); }
inline void emit_bts_m64_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 5, memref, imm); }

inline void emit_btr_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)        { emit_op_modrm_reg(emitptr, OP_BTR_Ev_Gv, OP_64BIT, sreg, dreg); }
inline void emit_btr_m64_r64(x86code *&emitptr, x86_memref memref, uint8_t sreg) { emit_op_modrm_mem(emitptr, OP_BTR_Ev_Gv, OP_64BIT, sreg, memref); }
inline void emit_btr_r64_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 6, dreg, imm); }
inline void emit_btr_m64_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 6, memref, imm); }

inline void emit_btc_r64_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)        { emit_op_modrm_reg(emitptr, OP_BTC_Ev_Gv, OP_64BIT, sreg, dreg); }
inline void emit_btc_m64_r64(x86code *&emitptr, x86_memref memref, uint8_t sreg) { emit_op_modrm_mem(emitptr, OP_BTC_Ev_Gv, OP_64BIT, sreg, memref); }
inline void emit_btc_r64_imm(x86code *&emitptr, uint8_t dreg, uint8_t imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 7, dreg, imm); }
inline void emit_btc_m64_imm(x86code *&emitptr, x86_memref memref, uint8_t imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 7, memref, imm); }

#endif



//**************************************************************************
//  LEA EMITTERS
//**************************************************************************

inline void emit_lea_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_LEA_Gv_M, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_lea_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_LEA_Gv_M, OP_64BIT, dreg, memref); }
#endif



//**************************************************************************
//  SET EMITTERS
//**************************************************************************

//-------------------------------------------------
//  emit_setcc_*
//-------------------------------------------------

inline void emit_setcc_r8(x86code *&emitptr, uint8_t cond, uint8_t dreg)            { emit_op_modrm_reg(emitptr, OP_SETCC_O_Eb + cond, OP_32BIT, 0, dreg); }
inline void emit_setcc_m8(x86code *&emitptr, uint8_t cond, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_SETCC_O_Eb + cond, OP_32BIT, 0, memref); }



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

inline void emit_ffree_stn(x86code *&emitptr, uint8_t reg)        { emit_op_simple(emitptr, OP_FFREE_STn + reg, OP_32BIT); }
inline void emit_fst_stn(x86code *&emitptr, uint8_t reg)          { emit_op_simple(emitptr, OP_FST_STn + reg, OP_32BIT); }
inline void emit_fstp_stn(x86code *&emitptr, uint8_t reg)         { emit_op_simple(emitptr, OP_FSTP_STn + reg, OP_32BIT); }
inline void emit_fucomp_stn(x86code *&emitptr, uint8_t reg)       { emit_op_simple(emitptr, OP_FUCOMP_STn + reg, OP_32BIT); }

inline void emit_fadd_st0_stn(x86code *&emitptr, uint8_t reg)     { emit_op_simple(emitptr, OP_FADD_ST0_STn + reg, OP_32BIT); }
inline void emit_fmul_st0_stn(x86code *&emitptr, uint8_t reg)     { emit_op_simple(emitptr, OP_FMUL_ST0_STn + reg, OP_32BIT); }
inline void emit_fcom_st0_stn(x86code *&emitptr, uint8_t reg)     { emit_op_simple(emitptr, OP_FCOM_ST0_STn + reg, OP_32BIT); }
inline void emit_fcomp_st0_stn(x86code *&emitptr, uint8_t reg)    { emit_op_simple(emitptr, OP_FCOMP_ST0_STn + reg, OP_32BIT); }
inline void emit_fsub_st0_stn(x86code *&emitptr, uint8_t reg)     { emit_op_simple(emitptr, OP_FSUB_ST0_STn + reg, OP_32BIT); }
inline void emit_fsubr_st0_stn(x86code *&emitptr, uint8_t reg)    { emit_op_simple(emitptr, OP_FSUBR_ST0_STn + reg, OP_32BIT); }
inline void emit_fdiv_st0_stn(x86code *&emitptr, uint8_t reg)     { emit_op_simple(emitptr, OP_FDIV_ST0_STn + reg, OP_32BIT); }
inline void emit_fdivr_st0_stn(x86code *&emitptr, uint8_t reg)    { emit_op_simple(emitptr, OP_FDIVR_ST0_STn + reg, OP_32BIT); }
inline void emit_fld_st0_stn(x86code *&emitptr, uint8_t reg)      { emit_op_simple(emitptr, OP_FLD_ST0_STn + reg, OP_32BIT); }
inline void emit_fxch_st0_stn(x86code *&emitptr, uint8_t reg)     { emit_op_simple(emitptr, OP_FXCH_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmovb_st0_stn(x86code *&emitptr, uint8_t reg)   { emit_op_simple(emitptr, OP_FCMOVB_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmove_st0_stn(x86code *&emitptr, uint8_t reg)   { emit_op_simple(emitptr, OP_FCMOVE_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmovbe_st0_stn(x86code *&emitptr, uint8_t reg)  { emit_op_simple(emitptr, OP_FCMOVBE_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmovu_st0_stn(x86code *&emitptr, uint8_t reg)   { emit_op_simple(emitptr, OP_FCMOVU_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmovnb_st0_stn(x86code *&emitptr, uint8_t reg)  { emit_op_simple(emitptr, OP_FCMOVNB_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmovne_st0_stn(x86code *&emitptr, uint8_t reg)  { emit_op_simple(emitptr, OP_FCMOVNE_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmovnbe_st0_stn(x86code *&emitptr, uint8_t reg) { emit_op_simple(emitptr, OP_FCMOVNBE_ST0_STn + reg, OP_32BIT); }
inline void emit_fcmovnu_st0_stn(x86code *&emitptr, uint8_t reg)  { emit_op_simple(emitptr, OP_FCMOVNU_ST0_STn + reg, OP_32BIT); }
inline void emit_fucomi_st0_stn(x86code *&emitptr, uint8_t reg)   { emit_op_simple(emitptr, OP_FUCOMI_ST0_STn + reg, OP_32BIT); }
inline void emit_fcomi_st0_stn(x86code *&emitptr, uint8_t reg)    { emit_op_simple(emitptr, OP_FCOMI_ST0_STn + reg, OP_32BIT); }
inline void emit_fcomip_st0_stn(x86code *&emitptr, uint8_t reg)   { emit_op_simple(emitptr, OP_FCOMIP_ST0_STn + reg, OP_32BIT); }

inline void emit_fadd_stn_st0(x86code *&emitptr, uint8_t reg)     { emit_op_simple(emitptr, OP_FADD_STn_ST0 + reg, OP_32BIT); }
inline void emit_fmul_stn_st0(x86code *&emitptr, uint8_t reg)     { emit_op_simple(emitptr, OP_FMUL_STn_ST0 + reg, OP_32BIT); }
inline void emit_fsubr_stn_st0(x86code *&emitptr, uint8_t reg)    { emit_op_simple(emitptr, OP_FSUBR_STn_ST0 + reg, OP_32BIT); }
inline void emit_fsub_stn_st0(x86code *&emitptr, uint8_t reg)     { emit_op_simple(emitptr, OP_FSUB_STn_ST0 + reg, OP_32BIT); }
inline void emit_fdivr_stn_st0(x86code *&emitptr, uint8_t reg)    { emit_op_simple(emitptr, OP_FDIVR_STn_ST0 + reg, OP_32BIT); }
inline void emit_fdiv_stn_st0(x86code *&emitptr, uint8_t reg)     { emit_op_simple(emitptr, OP_FDIV_STn_ST0 + reg, OP_32BIT); }
inline void emit_fucom_stn_st0(x86code *&emitptr, uint8_t reg)    { emit_op_simple(emitptr, OP_FUCOM_STn_ST0 + reg, OP_32BIT); }
inline void emit_faddp_stn_st0(x86code *&emitptr, uint8_t reg)    { emit_op_simple(emitptr, OP_FADDP_STn_ST0 + reg, OP_32BIT); }
inline void emit_fmulp_stn_st0(x86code *&emitptr, uint8_t reg)    { emit_op_simple(emitptr, OP_FMULP_STn_ST0 + reg, OP_32BIT); }
inline void emit_fsubrp_stn_st0(x86code *&emitptr, uint8_t reg)   { emit_op_simple(emitptr, OP_FSUBRP_STn_ST0 + reg, OP_32BIT); }
inline void emit_fsubp_stn_st0(x86code *&emitptr, uint8_t reg)    { emit_op_simple(emitptr, OP_FSUBP_STn_ST0 + reg, OP_32BIT); }
inline void emit_fdivrp_stn_st0(x86code *&emitptr, uint8_t reg)   { emit_op_simple(emitptr, OP_FDIVRP_STn_ST0 + reg, OP_32BIT); }
inline void emit_fdivp_stn_st0(x86code *&emitptr, uint8_t reg)    { emit_op_simple(emitptr, OP_FDIVP_STn_ST0 + reg, OP_32BIT); }

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

inline void emit_movd_r128_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)               { emit_op_modrm_reg(emitptr, OP_MOVD_Vd_Ed, OP_32BIT, dreg, sreg); }
inline void emit_movd_r128_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)        { emit_op_modrm_mem(emitptr, OP_MOVD_Vd_Ed, OP_32BIT, dreg, memref); }
inline void emit_movd_r32_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)               { emit_op_modrm_reg(emitptr, OP_MOVD_Ed_Vd, OP_32BIT, sreg, dreg); }
inline void emit_movd_m32_r128(x86code *&emitptr, x86_memref memref, uint8_t sreg)        { emit_op_modrm_mem(emitptr, OP_MOVD_Ed_Vd, OP_32BIT, sreg, memref); }

#if (X86EMIT_SIZE == 64)

inline void emit_movq_r128_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)               { emit_op_modrm_reg(emitptr, OP_MOVD_Vd_Ed, OP_64BIT, dreg, sreg); }
inline void emit_movq_r128_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)        { emit_op_modrm_mem(emitptr, OP_MOVD_Vd_Ed, OP_64BIT, dreg, memref); }
inline void emit_movq_r64_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)               { emit_op_modrm_reg(emitptr, OP_MOVD_Ed_Vd, OP_64BIT, sreg, dreg); }
inline void emit_movq_m64_r128(x86code *&emitptr, x86_memref memref, uint8_t sreg)        { emit_op_modrm_mem(emitptr, OP_MOVD_Ed_Vd, OP_64BIT, sreg, memref); }

#endif

inline void emit_movdqa_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_MOVDQA_Vdq_Wdq, OP_32BIT, dreg, memref); }
inline void emit_movdqa_m128_r128(x86code *&emitptr, x86_memref memref, uint8_t sreg) { emit_op_modrm_mem(emitptr, OP_MOVDQA_Wdq_Vdq, OP_32BIT, sreg, memref); }
inline void emit_movdqu_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref) { emit_op_modrm_mem(emitptr, OP_MOVDQU_Vdq_Wdq, OP_32BIT, dreg, memref); }
inline void emit_movdqu_m128_r128(x86code *&emitptr, x86_memref memref, uint8_t sreg) { emit_op_modrm_mem(emitptr, OP_MOVDQU_Wdq_Vdq, OP_32BIT, sreg, memref); }


//**************************************************************************
//  SSE SCALAR SINGLE EMITTERS
//**************************************************************************

inline void emit_movss_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_MOVSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_movss_r128_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_MOVSS_Vss_Wss, OP_32BIT, dreg, memref); }
inline void emit_movss_m32_r128(x86code *&emitptr, x86_memref memref, uint8_t sreg)       { emit_op_modrm_mem(emitptr, OP_MOVSS_Wss_Vss, OP_32BIT, sreg, memref); }

inline void emit_addss_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_ADDSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_addss_r128_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_ADDSS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_subss_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_SUBSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_subss_r128_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_SUBSS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_mulss_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_MULSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_mulss_r128_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_MULSS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_divss_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_DIVSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_divss_r128_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_DIVSS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_rcpss_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_RCPSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_rcpss_r128_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_RCPSS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_sqrtss_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)            { emit_op_modrm_reg(emitptr, OP_SQRTSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_sqrtss_r128_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_SQRTSS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_rsqrtss_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_RSQRTSS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_rsqrtss_r128_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_RSQRTSS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_comiss_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)            { emit_op_modrm_reg(emitptr, OP_COMISS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_comiss_r128_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_COMISS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_ucomiss_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_UCOMISS_Vss_Wss, OP_32BIT, dreg, sreg); }
inline void emit_ucomiss_r128_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_UCOMISS_Vss_Wss, OP_32BIT, dreg, memref); }

inline void emit_cvtsi2ss_r128_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSI2SS_Vss_Ed, OP_32BIT, dreg, sreg); }
inline void emit_cvtsi2ss_r128_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTSI2SS_Vss_Ed, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_cvtsi2ss_r128_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSI2SS_Vss_Ed, OP_64BIT, dreg, sreg); }
inline void emit_cvtsi2ss_r128_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTSI2SS_Vss_Ed, OP_64BIT, dreg, memref); }
#endif

inline void emit_cvtsd2ss_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CVTSD2SS_Vss_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_cvtsd2ss_r128_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTSD2SS_Vss_Wsd, OP_32BIT, dreg, memref); }

inline void emit_cvtss2si_r32_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSS2SI_Gd_Wss, OP_32BIT, dreg, sreg); }
inline void emit_cvtss2si_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_CVTSS2SI_Gd_Wss, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_cvtss2si_r64_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSS2SI_Gd_Wss, OP_64BIT, dreg, sreg); }
inline void emit_cvtss2si_r64_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_CVTSS2SI_Gd_Wss, OP_64BIT, dreg, memref); }
#endif

inline void emit_cvttss2si_r32_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CVTTSS2SI_Gd_Wss, OP_32BIT, dreg, sreg); }
inline void emit_cvttss2si_r32_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTTSS2SI_Gd_Wss, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_cvttss2si_r64_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CVTTSS2SI_Gd_Wss, OP_64BIT, dreg, sreg); }
inline void emit_cvttss2si_r64_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTTSS2SI_Gd_Wss, OP_64BIT, dreg, memref); }
#endif

inline void emit_roundss_r128_r128_imm(x86code *&emitptr, uint8_t dreg, uint8_t sreg, uint8_t imm)        { emit_op_modrm_reg(emitptr, OP_ROUNDSS_Vss_Wss_Ib, OP_32BIT, dreg, sreg); emit_byte(emitptr, imm); }
inline void emit_roundss_r128_m32_imm(x86code *&emitptr, uint8_t dreg, x86_memref memref, uint8_t imm)  { emit_op_modrm_mem(emitptr, OP_ROUNDSS_Vss_Wss_Ib, OP_32BIT, dreg, memref); emit_byte(emitptr, imm); }



//**************************************************************************
//  SSE PACKED SINGLE EMITTERS
//**************************************************************************

inline void emit_movps_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_MOVAPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_movps_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_MOVAPS_Vps_Wps, OP_32BIT, dreg, memref); }
inline void emit_movps_m128_r128(x86code *&emitptr, x86_memref memref, uint8_t sreg)      { emit_op_modrm_mem(emitptr, OP_MOVAPS_Wps_Vps, OP_32BIT, sreg, memref); }

inline void emit_addps_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_ADDPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_addps_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_ADDPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_subps_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_SUBPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_subps_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_SUBPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_mulps_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_MULPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_mulps_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_MULPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_divps_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_DIVPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_divps_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_DIVPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_rcpps_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_RCPPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_rcpps_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_RCPPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_sqrtps_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)            { emit_op_modrm_reg(emitptr, OP_SQRTPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_sqrtps_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_SQRTPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_rsqrtps_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_RSQRTPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_rsqrtps_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_RSQRTPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_andps_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_ANDPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_andps_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_ANDPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_andnps_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)            { emit_op_modrm_reg(emitptr, OP_ANDNPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_andnps_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ANDNPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_orps_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)              { emit_op_modrm_reg(emitptr, OP_ORPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_orps_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_ORPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_xorps_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_XORPS_Vps_Wps, OP_32BIT, dreg, sreg); }
inline void emit_xorps_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_XORPS_Vps_Wps, OP_32BIT, dreg, memref); }

inline void emit_cvtdq2ps_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CVTDQ2PS_Vps_Wdq, OP_32BIT, dreg, sreg); }
inline void emit_cvtdq2ps_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CVTDQ2PS_Vps_Wdq, OP_32BIT, dreg, memref); }

inline void emit_cvtpd2ps_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CVTPD2PS_Vps_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_cvtpd2ps_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CVTPD2PS_Vps_Wpd, OP_32BIT, dreg, memref); }

inline void emit_cvtps2dq_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CVTPS2DQ_Vdq_Wps, OP_32BIT, dreg, sreg); }
inline void emit_cvtps2dq_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CVTPS2DQ_Vdq_Wps, OP_32BIT, dreg, memref); }

inline void emit_cvttps2dq_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_CVTTPS2DQ_Vdq_Wps, OP_32BIT, dreg, sreg); }
inline void emit_cvttps2dq_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_CVTTPS2DQ_Vdq_Wps, OP_32BIT, dreg, memref); }

inline void emit_roundps_r128_r128_imm(x86code *&emitptr, uint8_t dreg, uint8_t sreg, uint8_t imm)        { emit_op_modrm_reg(emitptr, OP_ROUNDPS_Vdq_Wdq_Ib, OP_32BIT, dreg, sreg); emit_byte(emitptr, imm); }
inline void emit_roundps_r128_m128_imm(x86code *&emitptr, uint8_t dreg, x86_memref memref, uint8_t imm) { emit_op_modrm_mem(emitptr, OP_ROUNDPS_Vdq_Wdq_Ib, OP_32BIT, dreg, memref); emit_byte(emitptr, imm); }



//**************************************************************************
//  SSE SCALAR DOUBLE EMITTERS
//**************************************************************************

inline void emit_movsd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_MOVSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_movsd_r128_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_MOVSD_Vsd_Wsd, OP_32BIT, dreg, memref); }
inline void emit_movsd_m64_r128(x86code *&emitptr, x86_memref memref, uint8_t sreg)       { emit_op_modrm_mem(emitptr, OP_MOVSD_Wsd_Vsd, OP_32BIT, sreg, memref); }

inline void emit_addsd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_ADDSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_addsd_r128_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_ADDSD_Vsd_Wsd, OP_32BIT, dreg, memref); }

inline void emit_subsd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_SUBSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_subsd_r128_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_SUBSD_Vsd_Wsd, OP_32BIT, dreg, memref); }

inline void emit_mulsd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_MULSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_mulsd_r128_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_MULSD_Vsd_Wsd, OP_32BIT, dreg, memref); }

inline void emit_divsd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_DIVSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_divsd_r128_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_DIVSD_Vsd_Wsd, OP_32BIT, dreg, memref); }

inline void emit_sqrtsd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)            { emit_op_modrm_reg(emitptr, OP_SQRTSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_sqrtsd_r128_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_SQRTSD_Vsd_Wsd, OP_32BIT, dreg, memref); }

inline void emit_comisd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)            { emit_op_modrm_reg(emitptr, OP_COMISD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_comisd_r128_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_COMISD_Vsd_Wsd, OP_32BIT, dreg, memref); }

inline void emit_ucomisd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_UCOMISD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_ucomisd_r128_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_UCOMISD_Vsd_Wsd, OP_32BIT, dreg, memref); }

inline void emit_cvtsi2sd_r128_r32(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSI2SD_Vsd_Ed, OP_32BIT, dreg, sreg); }
inline void emit_cvtsi2sd_r128_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTSI2SD_Vsd_Ed, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_cvtsi2sd_r128_r64(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSI2SD_Vsd_Ed, OP_64BIT, dreg, sreg); }
inline void emit_cvtsi2sd_r128_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTSI2SD_Vsd_Ed, OP_64BIT, dreg, memref); }
#endif

inline void emit_cvtss2sd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CVTSS2SD_Vsd_Wss, OP_32BIT, dreg, sreg); }
inline void emit_cvtss2sd_r128_m32(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTSS2SD_Vsd_Wss, OP_32BIT, dreg, memref); }

inline void emit_cvtsd2si_r32_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSD2SI_Gd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_cvtsd2si_r32_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_CVTSD2SI_Gd_Wsd, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_cvtsd2si_r64_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)           { emit_op_modrm_reg(emitptr, OP_CVTSD2SI_Gd_Wsd, OP_64BIT, dreg, sreg); }
inline void emit_cvtsd2si_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_CVTSD2SI_Gd_Wsd, OP_64BIT, dreg, memref); }
#endif

inline void emit_cvttsd2si_r32_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CVTTSD2SI_Gd_Wsd, OP_32BIT, dreg, sreg); }
inline void emit_cvttsd2si_r32_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTTSD2SI_Gd_Wsd, OP_32BIT, dreg, memref); }

#if (X86EMIT_SIZE == 64)
inline void emit_cvttsd2si_r64_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CVTTSD2SI_Gd_Wsd, OP_64BIT, dreg, sreg); }
inline void emit_cvttsd2si_r64_m64(x86code *&emitptr, uint8_t dreg, x86_memref memref)    { emit_op_modrm_mem(emitptr, OP_CVTTSD2SI_Gd_Wsd, OP_64BIT, dreg, memref); }
#endif

inline void emit_roundsd_r128_r128_imm(x86code *&emitptr, uint8_t dreg, uint8_t sreg, uint8_t imm)        { emit_op_modrm_reg(emitptr, OP_ROUNDSD_Vsd_Wsd_Ib, OP_32BIT, dreg, sreg); emit_byte(emitptr, imm); }
inline void emit_roundsd_r128_m64_imm(x86code *&emitptr, uint8_t dreg, x86_memref memref, uint8_t imm)  { emit_op_modrm_mem(emitptr, OP_ROUNDSD_Vsd_Wsd_Ib, OP_32BIT, dreg, memref); emit_byte(emitptr, imm); }



//**************************************************************************
//  SSE PACKED DOUBLE EMITTERS
//**************************************************************************

inline void emit_movpd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_MOVAPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_movpd_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_MOVAPD_Vpd_Wpd, OP_32BIT, dreg, memref); }
inline void emit_movpd_m128_r128(x86code *&emitptr, x86_memref memref, uint8_t sreg)      { emit_op_modrm_mem(emitptr, OP_MOVAPD_Wpd_Vpd, OP_32BIT, sreg, memref); }

inline void emit_addpd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_ADDPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_addpd_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_ADDPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_subpd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_SUBPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_subpd_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_SUBPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_mulpd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_MULPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_mulpd_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_MULPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_divpd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_DIVPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_divpd_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_DIVPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_sqrtpd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)            { emit_op_modrm_reg(emitptr, OP_SQRTPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_sqrtpd_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_SQRTPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_andpd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_ANDPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_andpd_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_ANDPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_andnpd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)            { emit_op_modrm_reg(emitptr, OP_ANDNPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_andnpd_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)     { emit_op_modrm_mem(emitptr, OP_ANDNPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_orpd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)              { emit_op_modrm_reg(emitptr, OP_ORPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_orpd_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)       { emit_op_modrm_mem(emitptr, OP_ORPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_xorpd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)             { emit_op_modrm_reg(emitptr, OP_XORPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_xorpd_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)      { emit_op_modrm_mem(emitptr, OP_XORPD_Vpd_Wpd, OP_32BIT, dreg, memref); }

inline void emit_cvtdq2pd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CVTDQ2PD_Vpd_Wq, OP_32BIT, dreg, sreg); }
inline void emit_cvtdq2pd_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CVTDQ2PD_Vpd_Wq, OP_32BIT, dreg, memref); }

inline void emit_cvtps2pd_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CVTPS2PD_Vpd_Wq, OP_32BIT, dreg, sreg); }
inline void emit_cvtps2pd_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CVTPS2PD_Vpd_Wq, OP_32BIT, dreg, memref); }

inline void emit_cvtpd2dq_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)          { emit_op_modrm_reg(emitptr, OP_CVTPD2DQ_Vdq_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_cvtpd2dq_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)   { emit_op_modrm_mem(emitptr, OP_CVTPD2DQ_Vdq_Wpd, OP_32BIT, dreg, memref); }

inline void emit_cvttpd2dq_r128_r128(x86code *&emitptr, uint8_t dreg, uint8_t sreg)         { emit_op_modrm_reg(emitptr, OP_CVTTPD2DQ_Vdq_Wpd, OP_32BIT, dreg, sreg); }
inline void emit_cvttpd2dq_r128_m128(x86code *&emitptr, uint8_t dreg, x86_memref memref)  { emit_op_modrm_mem(emitptr, OP_CVTTPD2DQ_Vdq_Wpd, OP_32BIT, dreg, memref); }

inline void emit_roundpd_r128_r128_imm(x86code *&emitptr, uint8_t dreg, uint8_t sreg, uint8_t imm)        { emit_op_modrm_reg(emitptr, OP_ROUNDPD_Vdq_Wdq_Ib, OP_32BIT, dreg, sreg); emit_byte(emitptr, imm); }
inline void emit_roundpd_r128_m128_imm(x86code *&emitptr, uint8_t dreg, x86_memref memref, uint8_t imm) { emit_op_modrm_mem(emitptr, OP_ROUNDPD_Vdq_Wdq_Ib, OP_32BIT, dreg, memref); emit_byte(emitptr, imm); }

}

#undef X86EMIT_SIZE

#endif
