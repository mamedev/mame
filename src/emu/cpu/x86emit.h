/***************************************************************************

    x86emit.h

    Generic x86/x64 code emitters.

    Copyright Aaron Giles
    Released for general use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Important note:

    It is assumed in 64-bit mode that all byte register accesses are
    intended to be to the low 8 bits (SPL,BPL,SIL,DIL) and not to the
    upper half (AH,CH,DH,BH).

***************************************************************************/

#pragma once

#ifndef __X86EMIT_H__
#define __X86EMIT_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* opcode size flag; low 4 bits must match REX low 4 bits, hence the odd numbers here */
#define OP_16BIT				0x10
#define OP_32BIT   				0x00
#define OP_64BIT				0x08

/* 16 registers on x64, only 8 on x86 */
#ifdef PTR64
#define REG_MAX					16
#else
#define REG_MAX					8
#endif

/* invalid register index for "none" */
#define REG_NONE				REG_MAX

/* 8-bit registers -- note that we assume a flat model for 64-bit */
#define REG_AL					0
#define REG_CL					1
#define REG_DL					2
#define REG_BL					3
#ifndef PTR64
#define REG_AH					4
#define REG_CH					5
#define REG_DH					6
#define REG_BH					7
#else
#define REG_SPL					4
#define REG_BPL					5
#define REG_SIL					6
#define REG_DIL					7
#define REG_R8L					8
#define REG_R9L					9
#define REG_R10L				10
#define REG_R11L				11
#define REG_R12L				12
#define REG_R13L				13
#define REG_R14L				14
#define REG_R15L				15
#endif

/* 16-bit registers */
#define REG_AX					0
#define REG_CX					1
#define REG_DX					2
#define REG_BX					3
#define REG_SP					4
#define REG_BP					5
#define REG_SI					6
#define REG_DI					7
#ifdef PTR64
#define REG_R8W					8
#define REG_R9W					9
#define REG_R10W				10
#define REG_R11W				11
#define REG_R12W				12
#define REG_R13W				13
#define REG_R14W				14
#define REG_R15W				15
#endif

/* 32-bit registers */
#define REG_EAX					0
#define REG_ECX					1
#define REG_EDX					2
#define REG_EBX					3
#define REG_ESP					4
#define REG_EBP					5
#define REG_ESI					6
#define REG_EDI					7
#ifdef PTR64
#define REG_R8D					8
#define REG_R9D					9
#define REG_R10D				10
#define REG_R11D				11
#define REG_R12D				12
#define REG_R13D				13
#define REG_R14D				14
#define REG_R15D				15
#endif

/* 64-bit registers */
#ifdef PTR64
#define REG_RAX					0
#define REG_RCX					1
#define REG_RDX					2
#define REG_RBX					3
#define REG_RSP					4
#define REG_RBP					5
#define REG_RSI					6
#define REG_RDI					7
#define REG_R8					8
#define REG_R9					9
#define REG_R10					10
#define REG_R11					11
#define REG_R12					12
#define REG_R13					13
#define REG_R14					14
#define REG_R15					15
#endif

/* 64-bit MMX registers */
#define REG_MM0					0
#define REG_MM1					1
#define REG_MM2					2
#define REG_MM3					3
#define REG_MM4					4
#define REG_MM5					5
#define REG_MM6					6
#define REG_MM7					7
#ifdef PTR64
#define REG_MM8					8
#define REG_MM9					9
#define REG_MM10				10
#define REG_MM11				11
#define REG_MM12				12
#define REG_MM13				13
#define REG_MM14				14
#define REG_MM15				15
#endif

/* 128-bit XMM registers */
#define REG_XMM0				0
#define REG_XMM1				1
#define REG_XMM2				2
#define REG_XMM3				3
#define REG_XMM4				4
#define REG_XMM5				5
#define REG_XMM6				6
#define REG_XMM7				7
#ifdef PTR64
#define REG_XMM8				8
#define REG_XMM9				9
#define REG_XMM10				10
#define REG_XMM11				11
#define REG_XMM12				12
#define REG_XMM13				13
#define REG_XMM14				14
#define REG_XMM15				15
#endif

/* conditions */
#define COND_A					7
#define COND_AE					3
#define COND_B					2
#define COND_BE					6
#define COND_C					2
#define COND_E					4
#define COND_Z					4
#define COND_G					15
#define COND_GE					13
#define COND_L					12
#define COND_LE					14
#define COND_NA					6
#define COND_NAE				2
#define COND_NB					3
#define COND_NBE				7
#define COND_NC					3
#define COND_NE					5
#define COND_NG					14
#define COND_NGE				12
#define COND_NL					13
#define COND_NLE				15
#define COND_NO					1
#define COND_NP					11
#define COND_NS					9
#define COND_NZ					5
#define COND_O					0
#define COND_P					10
#define COND_PE					10
#define COND_PO					11
#define COND_S					8
#define COND_Z					4

/* floating point rounding modes */
#define FPRND_NEAR				0
#define FPRND_DOWN				1
#define FPRND_UP				2
#define FPRND_CHOP				3



/***************************************************************************
    OPCODE DEFINITIONS
***************************************************************************/

/* opcode flags (in upper 8 bits of opcode) */
#define OPFLAG_8BITREG			(1 << 24)
#define OPFLAG_8BITRM			(1 << 25)

/* single byte opcodes */
#define OP_ADD_Eb_Gb			(0x00 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_ADD_Ev_Gv			0x01
#define OP_ADD_Gb_Eb			(0x02 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_ADD_Gv_Ev			0x03
#define OP_ADD_AL_Ib			0x04
#define OP_ADD_rAX_Iz   		0x05
#define OP_PUSH_ES	  			0x06
#define OP_POP_ES	  	 		0x07
#define OP_OR_Eb_Gb	 			(0x08 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_OR_Ev_Gv	 			0x09
#define OP_OR_Gb_Eb	 			(0x0a | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_OR_Gv_Ev	 			0x0b
#define OP_OR_AL_Ib	 			0x0c
#define OP_OR_eAX_Iv			0x0d
#define OP_PUSH_CS	  			0x0e
#define OP_EXTENDED	 			0x0f

#define OP_ADC_Eb_Gb			(0x10 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_ADC_Ev_Gv			0x11
#define OP_ADC_Gb_Eb			(0x12 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_ADC_Gv_Ev			0x13
#define OP_ADC_AL_Ib			0x14
#define OP_ADC_rAX_Iz   		0x15
#define OP_PUSH_SS	  			0x16
#define OP_POP_SS	   			0x17
#define OP_SBB_Eb_Gb			(0x18 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_SBB_Ev_Gv			0x19
#define OP_SBB_Gb_Eb			(0x1a | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_SBB_Gv_Ev			0x1b
#define OP_SBB_AL_Ib			0x1c
#define OP_SBB_eAX_Iv   		0x1d
#define OP_PUSH_DS	  			0x1e
#define OP_POP_DS	   			0x1f

#define OP_AND_Eb_Gb			(0x20 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_AND_Ev_Gv			0x21
#define OP_AND_Gb_Eb			(0x22 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_AND_Gv_Ev			0x23
#define OP_AND_AL_Ib			0x24
#define OP_AND_rAX_Iz   		0x25
#define PREFIX_ES	   			0x26
#define OP_DAA		  			0x27
#define OP_SUB_Eb_Gb			(0x28 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_SUB_Ev_Gv			0x29
#define OP_SUB_Gb_Eb			(0x2a | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_SUB_Gv_Ev			0x2b
#define OP_SUB_AL_Ib			0x2c
#define OP_SUB_eAX_Iv   		0x2d
#define PREFIX_CS	   			0x2e
#define OP_DAS		  			0x2f

#define OP_XOR_Eb_Gb			(0x30 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_XOR_Ev_Gv			0x31
#define OP_XOR_Gb_Eb			(0x32 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_XOR_Gv_Ev			0x33
#define OP_XOR_AL_Ib			0x34
#define OP_XOR_rAX_Iz   		0x35
#define PREFIX_SS	   			0x36
#define OP_AAA		  			0x37
#define OP_CMP_Eb_Gb			(0x38 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_CMP_Ev_Gv			0x39
#define OP_CMP_Gb_Eb			(0x3a | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_CMP_Gv_Ev			0x3b
#define OP_CMP_AL_Ib			0x3c
#define OP_CMP_eAX_Iv   		0x3d
#define PREFIX_DS	   			0x3e
#define OP_AAS		  			0x3f

#define OP_REX					0x40
#define OP_REX_B				0x41
#define OP_REX_X				0x42
#define OP_REX_XB				0x43
#define OP_REX_R				0x44
#define OP_REX_RB				0x45
#define OP_REX_RX				0x46
#define OP_REX_RXB				0x47
#define OP_REX_W				0x48
#define OP_REX_WB				0x49
#define OP_REX_WX				0x4a
#define OP_REX_WXB				0x4b
#define OP_REX_WR				0x4c
#define OP_REX_WRB				0x4d
#define OP_REX_WRX				0x4e
#define OP_REX_WRXB				0x4f

#define OP_PUSH_rAX				0x50
#define OP_PUSH_rCX				0x51
#define OP_PUSH_rDX				0x52
#define OP_PUSH_rBX				0x53
#define OP_PUSH_rSP				0x54
#define OP_PUSH_rBP				0x55
#define OP_PUSH_rSI				0x56
#define OP_PUSH_rDI				0x57
#define OP_POP_rAX				0x58
#define OP_POP_rCX				0x59
#define OP_POP_rDX				0x5a
#define OP_POP_rBX				0x5b
#define OP_POP_rSP				0x5c
#define OP_POP_rBP				0x5d
#define OP_POP_rSI				0x5e
#define OP_POP_rDI				0x5f

#define OP_PUSHA				0x60
#define OP_POPA					0x61
#define OP_BOUND_Gv_Ma			0x62
#define OP_ARPL_Ew_Gw			0x63
#define OP_MOVSXD_Gv_Ev			0x63
#define PREFIX_FS				0x64
#define PREFIX_GS				0x65
#define PREFIX_OPSIZE			0x66
#define PREFIX_ADSIZE			0x67
#define OP_PUSH_Iz				0x68
#define OP_IMUL_Gv_Ev_Iz 		0x69
#define OP_PUSH_Ib				0x6a
#define OP_IMUL_Gv_Ev_Ib		0x6b
#define OP_INS_Yb_DX			0x6c
#define OP_INS_Yz_DX			0x6d
#define OP_OUTS_DX_Xb			0x6e
#define OP_OUTS_DX_Xz			0x6f

#define OP_JCC_O_Jb				0x70
#define OP_JCC_NO_Jb			0x71
#define OP_JCC_B_Jb				0x72
#define OP_JCC_C_Jb				0x72
#define OP_JCC_NAE_Jb			0x72
#define OP_JCC_AE_Jb			0x73
#define OP_JCC_NB_Jb			0x73
#define OP_JCC_NC_Jb			0x73
#define OP_JCC_E_Jb				0x74
#define OP_JCC_Z_Jb				0x74
#define OP_JCC_NE_Jb			0x75
#define OP_JCC_NZ_Jb			0x75
#define OP_JCC_BE_Jb			0x76
#define OP_JCC_NA_Jb			0x76
#define OP_JCC_A_Jb				0x77
#define OP_JCC_NBE_Jb			0x77
#define OP_JCC_S_Jb				0x78
#define OP_JCC_NS_Jb			0x79
#define OP_JCC_P_Jb				0x7a
#define OP_JCC_PE_Jb			0x7a
#define OP_JCC_NP_Jb			0x7b
#define OP_JCC_PO_Jb			0x7b
#define OP_JCC_L_Jb				0x7c
#define OP_JCC_NGE_Jb			0x7c
#define OP_JCC_NL_Jb			0x7d
#define OP_JCC_GE_Jb			0x7d
#define OP_JCC_LE_Jb			0x7e
#define OP_JCC_NG_Jb			0x7e
#define OP_JCC_NLE_Jb			0x7f
#define OP_JCC_G_Jb				0x7f

#define OP_G1_Eb_Ib				(0x80 | OPFLAG_8BITRM)
#define OP_G1_Ev_Iz				0x81
#define OP_G1_Eb_Ibx			(0x82 | OPFLAG_8BITRM)
#define OP_G1_Ev_Ib				0x83
#define OP_TEST_Eb_Gb			(0x84 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_TEST_Ev_Gv			0x85
#define OP_XCHG_Eb_Gb			(0x86 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_XCHG_Ev_Gv			0x87
#define OP_MOV_Eb_Gb			(0x88 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_MOV_Ev_Gv			0x89
#define OP_MOV_Gb_Eb			(0x8a | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_MOV_Gv_Ev			0x8b
#define OP_MOV_Ev_Sw			0x8c
#define OP_LEA_Gv_M				0x8d
#define OP_MOV_Sw_Ew			0x8e
#define OP_G1A_Ev				0x8f

#define OP_NOP					0x90
#define OP_PAUSE				0x90
#define OP_XCHG_rCX				0x91
#define OP_XCHG_rDX				0x92
#define OP_XCHG_rBX				0x93
#define OP_XCHG_rSP				0x94
#define OP_XCHG_rBP				0x95
#define OP_XCHG_rSI				0x96
#define OP_XCHG_rDI				0x97
#define OP_CBW					0x98
#define OP_CWDE					0x98
#define OP_CDQE					0x98
#define OP_CWD					0x99
#define OP_CDQ					0x99
#define OP_CQO					0x99
#define OP_CALLF_Ap				0x9a
#define OP_FWAIT				0x9b
#define OP_PUSHF_Fv				0x9c
#define OP_POPF_Fv				0x9d
#define OP_SAHF					0x9e
#define OP_LAHF					0x9f

#define OP_MOV_AL_Ob			0xa0
#define OP_MOV_rAX_Ov			0xa1
#define OP_MOV_Ob_AL			0xa2
#define OP_MOV_Ov_rAX			0xa3
#define OP_MOVS_Xb_Yb			0xa4
#define OP_MOVS_Xv_Yv			0xa5
#define OP_CMPS_Xb_Yb			0xa6
#define OP_CMPS_Xv_Yv			0xa7
#define OP_TEST_AL_Ib			0xa8
#define OP_TEST_rAX_Iz			0xa9
#define OP_STOS_Yb_AL			0xaa
#define OP_STOS_Yv_rAX			0xab
#define OP_LODS_AL_Xb			0xac
#define OP_LODS_rAX_Xv			0xad
#define OP_SCAS_AL_Yb			0xae
#define OP_SCAC_rAX_Yv			0xaf

#define OP_MOV_AL_Ib			0xb0
#define OP_MOV_CL_Ib			0xb1
#define OP_MOV_DL_Ib			0xb2
#define OP_MOV_BL_Ib			0xb3
#define OP_MOV_AH_Ib			0xb4
#define OP_MOV_CH_Ib			0xb5
#define OP_MOV_DH_Ib			0xb6
#define OP_MOV_BH_Ib			0xb7
#define OP_MOV_rAX_Iv			0xb8
#define OP_MOV_rCX_Iv			0xb9
#define OP_MOV_rDX_Iv			0xba
#define OP_MOV_rBX_Iv			0xbb
#define OP_MOV_rSP_Iv			0xbc
#define OP_MOV_rBP_Iv			0xbd
#define OP_MOV_rSI_Iv			0xbe
#define OP_MOV_rDI_Iv			0xbf

#define OP_G2_Eb_Ib				(0xc0 | OPFLAG_8BITRM)
#define OP_G2_Ev_Ib				0xc1
#define OP_RETN_Iw				0xc2
#define OP_RETN					0xc3
#define OP_LES_Gz_Mp			0xc4
#define OP_LDS_Gz_Mp			0xc5
#define OP_G11_Eb_Ib			(0xc6 | OPFLAG_8BITRM)
#define OP_G11_Ev_Iz			0xc7
#define OP_ENTER_Iw_Ib			0xc8
#define OP_LEAVE				0xc9
#define OP_RETF_Iw				0xca
#define OP_RETF					0xcb
#define OP_INT_3				0xcc
#define OP_INT_Ib				0xcd
#define OP_INTO					0xce
#define OP_IRET					0xcf

#define OP_G2_Eb_1				(0xd0 | OPFLAG_8BITRM)
#define OP_G2_Ev_1				0xd1
#define OP_G2_Eb_CL				(0xd2 | OPFLAG_8BITRM)
#define OP_G2_Ev_CL				0xd3
#define OP_AAM					0xd4
#define OP_AAD					0xd5
#define OP_XLAT					0xd7
#define OP_ESC_D8				0xd8
#define OP_ESC_D9				0xd9
#define OP_ESC_DA				0xda
#define OP_ESC_DB				0xdb
#define OP_ESC_DC				0xdc
#define OP_ESC_DD				0xdd
#define OP_ESC_DE				0xde
#define OP_ESC_DF				0xdf

#define OP_LOOPNE_Jb			0xe0
#define OP_LOOPE_Jb				0xe1
#define OP_LOOP_Jb				0xe2
#define OP_JrCXZ_Jb				0xe3
#define OP_IN_AL_Ib				0xe4
#define OP_IN_eAX_Ib			0xe5
#define OP_OUT_Ib_AL			0xe6
#define OP_OUT_Ib_eAX			0xe7
#define OP_CALL_Jz				0xe8
#define OP_JMP_Jz				0xe9
#define OP_JMPF_AP				0xea
#define OP_JMP_Jb				0xeb
#define OP_IN_AL_DX				0xec
#define OP_IN_eAX_D				0xed
#define OP_OUT_DX_AL			0xee
#define OP_OUT_DX_eAX			0xef

#define PREFIX_LOCK				0xf0
#define PREFIX_REPNE			0xf2
#define PREFIX_REPE				0xf3
#define OP_HLT					0xf4
#define OP_CMC					0xf5
#define OP_G3_Eb				(0xf6 | OPFLAG_8BITRM)
#define OP_G3_Ev				0xf7
#define OP_CLC					0xf8
#define OP_STC					0xf9
#define OP_CLI					0xfa
#define OP_STI					0xfb
#define OP_CLD					0xfc
#define OP_STD					0xfd
#define OP_G4					0xfe
#define OP_G5					0xff


/* double byte opcodes */
#define OP_G6					0x0f00
#define OP_G7					0x0f01
#define OP_LAR_Gv_Ew			0x0f02
#define OP_LSL_Gv_Ew			0x0f03
#define OP_SYSCALL				0x0f05
#define OP_CLTS					0x0f06
#define OP_SYSRET				0x0f07
#define OP_INVD					0x0f08
#define OP_WBINVD				0x0f09
#define OP_UD2					0x0f0b
#define OP_NOP0d_Ev				0x0f0d

#define OP_MOVUPS_Vps_Wps		0x0f10
#define OP_MOVSS_Vss_Wss		0xf30f10
#define OP_MOVUPD_Vpd_Wpd		0x660f10
#define OP_MOVSD_Vsd_Wsd		0xf20f10
#define OP_MOVUPS_Wps_Vps		0x0f11
#define OP_MOVSS_Wss_Vss		0xf30f11
#define OP_MOVUPD_Wpd_Vpd		0x660f11
#define OP_MOVSD_Wsd_Vsd		0xf20f11
#define OP_MOVLPS_Vq_Mq			0x0f12
#define OP_MOVLPD_Vq_Mq			0x660f12
#define OP_MOVHLPS_Vq_Uq		0x0f12
#define OP_MOVDDUP_Vq_Wq		0xf20f12
#define OP_MOVSLDUP_Vq_Wq		0xf30f12
#define OP_MOVLPS_Mq_Vq			0x0f13
#define OP_MOVLPD_Mq_Vq			0x660f13
#define OP_UNPCKLPS_Vps_Wq		0x0f14
#define OP_UNPCKLPD_Vpd_Wq		0x660f14
#define OP_UNPCKHPS_Vps_Wq		0x0f15
#define OP_UNPCKHPD_Vpd_Wq		0x660f15
#define OP_MOVHPS_Vq_Mq			0x0f16
#define OP_MOVHPD_Vq_Mq			0x660f16
#define OP_MOVLHPS_Vq_Uq		0x0f16
#define OP_MOVSHDUP_Vq_Wq		0xf30f16
#define OP_MOVHPS_Mq_Vq			0x0f17
#define OP_MOVHPD_Mq_Vq			0x660f17
#define OP_PREFETCH_G16			0x0f18
#define OP_NOP1f_Ev				0x0f1f

#define OP_MOV_Rd_Cd			0x0f20
#define OP_MOV_Rd_Dd			0x0f21
#define OP_MOV_Cd_Rd			0x0f22
#define OP_MOV_Dd_Rd			0x0f23
#define OP_MOVAPS_Vps_Wps		0x0f28
#define OP_MOVAPD_Vpd_Wpd		0x660f28
#define OP_MOVAPS_Wps_Vps		0x0f29
#define OP_MOVAPD_Wpd_Vpd		0x660f29
#define OP_CVTPI2PS_Vps_Qq		0x0f2a
#define OP_CVTSI2SS_Vss_Ed		0xf30f2a
#define OP_CVTPI2PD_Vpd_Qq		0x660f2a
#define OP_CVTSI2SD_Vsd_Ed		0xf20f2a
#define OP_MOVNTPS_Mps_Vps		0x0f2b
#define OP_MOVNTPD_Mpd_Vpd		0x660f2b
#define OP_CVTTPS2PI_Pq_Wq		0x0f2c
#define OP_CVTTSS2SI_Gd_Wss		0xf30f2c
#define OP_CVTTPD2PI_Pq_Wpd		0x660f2c
#define OP_CVTTSD2SI_Gd_Wsd		0xf20f2c
#define OP_CVTPS2PI_Pq_Wq		0x0f2d
#define OP_CVTSS2SI_Gd_Wss		0xf30f2d
#define OP_CVTPD2PI_Pq_Wpd		0x660f2d
#define OP_CVTSD2SI_Gd_Wsd		0xf20f2d
#define OP_UCOMISS_Vss_Wss		0x0f2e
#define OP_UCOMISD_Vsd_Wsd		0x660f2e
#define OP_COMISS_Vss_Wss		0x0f2f
#define OP_COMISD_Vsd_Wsd		0x660f2f

#define OP_WRMSR				0x0f30
#define OP_RDTSC				0x0f31
#define OP_RDMSR				0x0f32
#define OP_RDPMC				0x0f33
#define OP_SYSENTER				0x0f34
#define OP_SYSEXIT				0x0f35
#define OP_GETSEC				0x0f37

#define OP_CMOV_O_Gv_Ev			0x0f40
#define OP_CMOV_NO_Gv_Ev		0x0f41
#define OP_CMOV_B_Gv_Ev			0x0f42
#define OP_CMOV_C_Gv_Ev			0x0f42
#define OP_CMOV_AE_Gv_Ev		0x0f43
#define OP_CMOV_NC_Gv_Ev		0x0f43
#define OP_CMOV_E_Gv_Ev			0x0f44
#define OP_CMOV_Z_Gv_Ev			0x0f44
#define OP_CMOV_NE_Gv_Ev		0x0f45
#define OP_CMOV_NZ_Gv_Ev		0x0f45
#define OP_CMOV_BE_Gv_Ev		0x0f46
#define OP_CMOV_A_Gv_Ev			0x0f47
#define OP_CMOV_S_Gv_Ev			0x0f48
#define OP_CMOV_NS_Gv_Ev		0x0f49
#define OP_CMOV_P_Gv_Ev			0x0f4a
#define OP_CMOV_PE_Gv_Ev		0x0f4a
#define OP_CMOV_NP_Gv_Ev		0x0f4b
#define OP_CMOV_PO_Gv_Ev		0x0f4b
#define OP_CMOV_L_Gv_Ev			0x0f4c
#define OP_CMOV_NGE_Gv_Ev		0x0f4c
#define OP_CMOV_NL_Gv_Ev		0x0f4d
#define OP_CMOV_GE_Gv_Ev		0x0f4d
#define OP_CMOV_LE_Gv_Ev		0x0f4e
#define OP_CMOV_NG_Gv_Ev		0x0f4e
#define OP_CMOV_NLE_Gv_Ev		0x0f4f
#define OP_CMOV_G_Gv_Ev			0x0f4f

#define OP_MOVMSKPS_Gd_Ups		0x0f50
#define OP_MOVMSKPD_Gd_Upd		0x660f50
#define OP_SQRTPS_Vps_Wps		0x0f51
#define OP_SQRTSS_Vss_Wss		0xf30f51
#define OP_SQRTPD_Vpd_Wpd		0x660f51
#define OP_SQRTSD_Vsd_Wsd		0xf20f51
#define OP_RSQRTPS_Vps_Wps		0x0f52
#define OP_RSQRTSS_Vss_Wss		0xf30f52
#define OP_RCPPS_Vps_Wps		0x0f53
#define OP_RCPSS_Vss_Wss		0xf30f53
#define OP_ANDPS_Vps_Wps		0x0f54
#define OP_ANDPD_Vpd_Wpd		0x660f54
#define OP_ANDNPS_Vps_Wps		0x0f55
#define OP_ANDNPD_Vpd_Wpd		0x660f55
#define OP_ORPS_Vps_Wps			0x0f56
#define OP_ORPD_Vpd_Wpd			0x660f56
#define OP_XORPS_Vps_Wps		0x0f57
#define OP_XORPD_Vpd_Wpd		0x660f57
#define OP_ADDPS_Vps_Wps		0x0f58
#define OP_ADDSS_Vss_Wss		0xf30f58
#define OP_ADDPD_Vpd_Wpd		0x660f58
#define OP_ADDSD_Vsd_Wsd		0xf20f58
#define OP_MULPS_Vps_Wps		0x0f59
#define OP_MULSS_Vss_Wss		0xf30f59
#define OP_MULPD_Vpd_Wpd		0x660f59
#define OP_MULSD_Vsd_Wsd		0xf20f59
#define OP_CVTPS2PD_Vpd_Wq		0x0f5a
#define OP_CVTSS2SD_Vsd_Wss		0xf30f5a
#define OP_CVTPD2PS_Vps_Wpd		0x660f5a
#define OP_CVTSD2SS_Vss_Wsd		0xf20f5a
#define OP_CVTDQ2PS_Vps_Wdq		0x0f5b
#define OP_CVTPS2DQ_Vdq_Wps		0x660f5b
#define OP_CVTTPS2DQ_Vdq_Wps 	0xf30f5b
#define OP_SUBPS_Vps_Wps		0x0f5c
#define OP_SUBSS_Vss_Wss		0xf30f5c
#define OP_SUBPD_Vpd_Wpd		0x660f5c
#define OP_SUBSD_Vsd_Wsd		0xf20f5c
#define OP_MINPS_Vps_Wps		0x0f5d
#define OP_MINSS_Vss_Wss		0xf30f5d
#define OP_MINPD_Vpd_Wpd		0x660f5d
#define OP_MINSD_Vsd_Wsd		0xf20f5d
#define OP_DIVPS_Vps_Wps		0x0f5e
#define OP_DIVSS_Vss_Wss		0xf30f5e
#define OP_DIVPD_Vpd_Wpd		0x660f5e
#define OP_DIVSD_Vsd_Wsd		0xf20f5e
#define OP_MAXPS_Vps_Wps		0x0f5f
#define OP_MAXSS_Vss_Wss		0xf30f5f
#define OP_MAXPD_Vpd_Wpd		0x660f5f
#define OP_MAXSD_Vsd_Wsd		0xf20f5f

#define OP_PUNPCKLBW_Pq_Qd		0x0f60
#define OP_PUNPCKLBW_Vdq_Wdq 	0x660f60
#define OP_PUNPCKLWD_Pq_Qd		0x0f61
#define OP_PUNPCKLWD_Vdq_Wdq 	0x660f61
#define OP_PUNPCKLDQ_Pq_Qd		0x0f62
#define OP_PUNPCKLDQ_Vdq_Wdq 	0x660f62
#define OP_PACKSSWB_Pq_Qq		0x0f63
#define OP_PACKSSWB_Vdq_Wdq		0x660f63
#define OP_PCMPGTB_Pq_Qq		0x0f64
#define OP_PCMPGTB_Vdq_Wdq		0x660f64
#define OP_PCMPGTW_Pq_Qq		0x0f65
#define OP_PCMPGTW_Vdq_Wdq		0x660f65
#define OP_PCMPGTD_Pq_Qq		0x0f66
#define OP_PCMPGTD_Vdq_Wdq		0x660f66
#define OP_PACKUSWB_Pq_Qq		0x0f67
#define OP_PACKUSWB_Vdq_Wdq		0x660f67
#define OP_PUNPCKHBW_Pq_Qq		0x0f68
#define OP_PUNPCKHBW_Vdq_Qdq 	0x660f68
#define OP_PUNPCKHWD_Pq_Qq		0x0f69
#define OP_PUNPCKHWD_Vdq_Qdq 	0x660f69
#define OP_PUNPCKHDQ_Pq_Qq		0x0f6a
#define OP_PUNPCKHDQ_Vdq_Qdq 	0x660f6a
#define OP_PACKSSDW_Pq_Qq		0x0f6b
#define OP_PACKSSDW_Vdq_Qdq		0x660f6b
#define OP_PUNPCKLQDQ_Vdq_Wdq 	0x660f6c
#define OP_PUNPCKHQDQ_Vdq_Wdq 	0x660f6d
#define OP_MOVD_Pd_Ed			0x0f6e
#define OP_MOVD_Vd_Ed			0x660f6e
#define OP_MOVQ_Pq_Qq			0x0f6f
#define OP_MOVDQA_Vdq_Wdq		0x660f6f
#define OP_MOVDQU_Vdq_Wdq		0xf30f6f

#define OP_PSHUFW_Pq_Qq_Ib		0x0f70
#define OP_PSHUFD_Vdq_Wdq_Ib 	0x660f70
#define OP_PSHUFHW_Vdq_Wdq_Ib 	0xf30f70
#define OP_PSHUFLW_Vdq_Wdq_Ib 	0xf20f70
#define OP_G12					0x0f71
#define OP_G13					0x0f72
#define OP_G14					0x0f73
#define OP_PCMPEQB_Pq_Qq		0x0f74
#define OP_PCMPEQB_Vdq_Wdq		0x660f74
#define OP_PCMPEQW_Pq_Qq		0x0f75
#define OP_PCMPEQW_Vdq_Wdq		0x660f75
#define OP_PCMPEQD_Pq_Qq		0x0f76
#define OP_PCMPEQD_Vdq_Wdq		0x660f76
#define OP_EMMS					0x0f77
#define OP_VMREAD_Ed_Gd			0x0f78
#define OP_VMWRITE_Gd_Ed		0x0f79
#define OP_HADDPD_Vpd_Wpd		0x660f7c
#define OP_HADDPS_Vps_Wps		0xf20f7c
#define OP_HSUBPD_Vpd_Wpd		0x660f7d
#define OP_HSUBPS_Vps_Wps		0xf20f7d
#define OP_MOVD_Ed_Pd			0x0f7e
#define OP_MOVD_Ed_Vd			0x660f7e
#define OP_MOVQ_Vq_Wq			0xf30f7e
#define OP_MOVQ_Qq_Pq			0x0f7f
#define OP_MOVDQA_Wdq_Vdq		0x660f7f
#define OP_MOVDQU_Wdq_Vdq		0xf30f7f

#define OP_JCC_O_Jv				0x0f80
#define OP_JCC_NO_Jv			0x0f81
#define OP_JCC_B_Jv				0x0f82
#define OP_JCC_C_Jv				0x0f82
#define OP_JCC_NAE_Jv			0x0f82
#define OP_JCC_AE_Jv			0x0f83
#define OP_JCC_NB_Jv			0x0f83
#define OP_JCC_NC_Jv			0x0f83
#define OP_JCC_E_Jv				0x0f84
#define OP_JCC_Z_Jv				0x0f84
#define OP_JCC_NE_Jv			0x0f85
#define OP_JCC_NZ_Jv			0x0f85
#define OP_JCC_BE_Jv			0x0f86
#define OP_JCC_NA_Jv			0x0f86
#define OP_JCC_A_Jv				0x0f87
#define OP_JCC_NBE_Jv			0x0f87
#define OP_JCC_S_Jv				0x0f88
#define OP_JCC_NS_Jv			0x0f89
#define OP_JCC_P_Jv				0x0f8a
#define OP_JCC_PE_Jv			0x0f8a
#define OP_JCC_NP_Jv			0x0f8b
#define OP_JCC_PO_Jv			0x0f8b
#define OP_JCC_L_Jv				0x0f8c
#define OP_JCC_NGE_Jv			0x0f8c
#define OP_JCC_NL_Jv			0x0f8d
#define OP_JCC_GE_Jv			0x0f8d
#define OP_JCC_LE_Jv			0x0f8e
#define OP_JCC_NG_Jv			0x0f8e
#define OP_JCC_NLE_Jv			0x0f8f
#define OP_JCC_G_Jv				0x0f8f

#define OP_SETCC_O_Eb			(0x0f90 | OPFLAG_8BITRM)
#define OP_SETCC_NO_Eb			(0x0f91 | OPFLAG_8BITRM)
#define OP_SETCC_B_Eb			(0x0f92 | OPFLAG_8BITRM)
#define OP_SETCC_C_Eb			(0x0f92 | OPFLAG_8BITRM)
#define OP_SETCC_NAE_Eb			(0x0f92 | OPFLAG_8BITRM)
#define OP_SETCC_AE_Eb			(0x0f93 | OPFLAG_8BITRM)
#define OP_SETCC_NB_Eb			(0x0f93 | OPFLAG_8BITRM)
#define OP_SETCC_NC_Eb			(0x0f93 | OPFLAG_8BITRM)
#define OP_SETCC_E_Eb			(0x0f94 | OPFLAG_8BITRM)
#define OP_SETCC_Z_Eb			(0x0f94 | OPFLAG_8BITRM)
#define OP_SETCC_NE_Eb			(0x0f95 | OPFLAG_8BITRM)
#define OP_SETCC_NZ_Eb			(0x0f95 | OPFLAG_8BITRM)
#define OP_SETCC_BE_Eb			(0x0f96 | OPFLAG_8BITRM)
#define OP_SETCC_NA_Eb			(0x0f96 | OPFLAG_8BITRM)
#define OP_SETCC_A_Eb			(0x0f97 | OPFLAG_8BITRM)
#define OP_SETCC_NBE_Eb			(0x0f97 | OPFLAG_8BITRM)
#define OP_SETCC_S_Eb			(0x0f98 | OPFLAG_8BITRM)
#define OP_SETCC_NS_Eb			(0x0f99 | OPFLAG_8BITRM)
#define OP_SETCC_P_Eb			(0x0f9a | OPFLAG_8BITRM)
#define OP_SETCC_PE_Eb			(0x0f9a | OPFLAG_8BITRM)
#define OP_SETCC_NP_Eb			(0x0f9b | OPFLAG_8BITRM)
#define OP_SETCC_PO_Eb			(0x0f9b | OPFLAG_8BITRM)
#define OP_SETCC_L_Eb			(0x0f9c | OPFLAG_8BITRM)
#define OP_SETCC_NGE_Eb			(0x0f9c | OPFLAG_8BITRM)
#define OP_SETCC_NL_Eb			(0x0f9d | OPFLAG_8BITRM)
#define OP_SETCC_GE_Eb			(0x0f9d | OPFLAG_8BITRM)
#define OP_SETCC_LE_Eb			(0x0f9e | OPFLAG_8BITRM)
#define OP_SETCC_NG_Eb			(0x0f9e | OPFLAG_8BITRM)
#define OP_SETCC_NLE_Eb			(0x0f9f | OPFLAG_8BITRM)
#define OP_SETCC_G_Eb			(0x0f9f | OPFLAG_8BITRM)

#define OP_PUSH_FS				0x0fa0
#define OP_POP_FS				0x0fa1
#define OP_CPUID				0x0fa2
#define OP_BT_Ev_Gv				0x0fa3
#define OP_SHLD_Ev_Gv_Ib		0x0fa4
#define OP_SHLD_Ev_Gv_CL		0x0fa5
#define OP_PUSH_GS				0x0fa8
#define OP_POP_GS				0x0fa9
#define OP_RSM					0x0faa
#define OP_BTS_Ev_Gv			0x0fab
#define OP_SHRD_Ev_Gv_Ib		0x0fac
#define OP_SHRD_Ev_Gv_CL		0x0fad
#define OP_G16					0x0fae
#define OP_IMUL_Gv_Ev			0x0faf

#define OP_CMPXCHG_Eb_Gb		(0x0fb0 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_CMPXCHG_Ev_Gv		0x0fb1
#define OP_LSS_Mp				0x0fb2
#define OP_BTR_Ev_Gv			0x0fb3
#define OP_LFS_Mp				0x0fb4
#define OP_LGS_Mp				0x0fb5
#define OP_MOVZX_Gv_Eb			(0x0fb6 | OPFLAG_8BITRM)
#define OP_MOVZX_Gv_Ew			0x0fb7
#define OP_JMPE					0x0fb8
#define OP_POPCNT_Gv_Ev			0xf30fb8
#define OP_G10_INVALID			0x0fb9
#define OP_G8_Ev_Ib				0x0fba
#define OP_BTC_Ev_Gv			0x0fbb
#define OP_BSF_Gv_Ev			0x0fbc
#define OP_BSR_Gv_Ev			0x0fbd
#define OP_MOVSX_Gv_Eb			(0x0fbe | OPFLAG_8BITRM)
#define OP_MOVSX_Gv_Ew			0x0fbf

#define OP_XADD_Eb_Gb			(0x0fc0 | OPFLAG_8BITRM | OPFLAG_8BITREG)
#define OP_XADD_Ev_Gv			0x0fc1
#define OP_CMPPS_Vps_Wps_Ib		0x0fc2
#define OP_CMPSS_Vss_Wss_Ib		0xf30fc2
#define OP_CMPPD_Vpd_Wpd_Ib		0x660fc2
#define OP_CMPSD_Vsd_Wsd_Ib		0xf20fc2
#define OP_MOVNTI_Md_Gd			0x0fc3
#define OP_PINSRW_Pw_Ew_Ib		0x0fc4
#define OP_PINSRW_Vw_Ew_Ib		0x660fc4
#define OP_PEXTRW_Gw_Pw_Ib		0x0fc5
#define OP_PEXTRW_Gw_Vw_Ib		0x660fc5
#define OP_SHUFPS_Vps_Wps_Ib	0x0fc6
#define OP_SHUFPD_Vpd_Wpd_Ib	0x660fc6
#define OP_G9					0x0fc7
#define OP_BSWAP_EAX			0x0fc8
#define OP_BSWAP_ECX			0x0fc9
#define OP_BSWAP_EDX			0x0fca
#define OP_BSWAP_EBX			0x0fcb
#define OP_BSWAP_ESP			0x0fcc
#define OP_BSWAP_EBP			0x0fcd
#define OP_BSWAP_ESI			0x0fce
#define OP_BSWAP_EDI			0x0fcf

#define OP_ADDSUBPD_Vpd_Wpd		0x0fd0
#define OP_ADDSUBPS_Vps_Wps		0xf20fd0
#define OP_PSRLW_Pq_Qq			0x0fd1
#define OP_PSRLW_Vdq_Wdq		0x660fd1
#define OP_PSRLD_Pq_Qq			0x0fd2
#define OP_PSRLD_Vdq_Wdq		0x660fd2
#define OP_PSRLQ_Pq_Qq			0x0fd3
#define OP_PSRLQ_Vdq_Wdq		0x660fd3
#define OP_PADDQ_Pq_Qq			0x0fd4
#define OP_PADDQ_Vdq_Wdq		0x660fd4
#define OP_PMULLW_Pq_Qq			0x0fd5
#define OP_PMULLW_Vdq_Wdq		0x660fd5
#define OP_MOVQ_Wq_Vq			0x0fd6
#define OP_MOVQ2DQ_Vdq_Qq		0xf30fd6
#define OP_MOVDQ2Q_Pq_Vq		0xf20fd6
#define OP_PMOVMSKB_Gd_Pq		0x0fd7
#define OP_PMOVMSKB_Gd_Vdq		0x660fd7
#define OP_PSUBUSB_Pq_Qq		0x0fd8
#define OP_PSUBUSB_Vdq_Wdq		0x660fd8
#define OP_PSUBUSW_Pq_Qq		0x0fd9
#define OP_PSUBUSW_Vdq_Wdq		0x660fd9
#define OP_PMINUB_Pq_Qq			0x0fda
#define OP_PMINUB_Vdq_Wdq		0x660fda
#define OP_PAND_Pq_Qq			0x0fdb
#define OP_PAND_Vdq_Wdq			0x660fdb
#define OP_PADDUSB_Pq_Qq		0x0fdc
#define OP_PADDUSB_Vdq_Wdq		0x660fdc
#define OP_PADDUSW_Pq_Qq		0x0fdd
#define OP_PADDUSW_Vdq_Wdq		0x660fdd
#define OP_PMAXUB_Pq_Qq			0x0fde
#define OP_PMAXUB_Vdq_Wdq		0x660fde
#define OP_PANDN_Pq_Qq			0x0fdf
#define OP_PANDN_Vdq_Wdq		0x660fdf

#define OP_PAVGB_Pq_Qq			0x0fe0
#define OP_PAVGB_Vdq_Wdq		0x660fe0
#define OP_PSRAW_Pq_Qq			0x0fe1
#define OP_PSRAW_Vdq_Wdq		0x660fe1
#define OP_PSRAD_Pq_Qq			0x0fe2
#define OP_PSRAD_Vdq_Wdq		0x660fe2
#define OP_PAVGW_Pq_Qq			0x0fe3
#define OP_PAVGW_Vdq_Wdq		0x660fe3
#define OP_PMULHUW_Pq_Qq		0x0fe4
#define OP_PMULHUW_Vdq_Wdq		0x660fe4
#define OP_PMULHW_Pq_Qq			0x0fe5
#define OP_PMULHW_Vdq_Wdq		0x660fe5
#define OP_CVTPD2DQ_Vdq_Wpd		0xf20fe6
#define OP_CVTTPD2DQ_Vdq_Wpd	0x660fe6
#define OP_CVTDQ2PD_Vpd_Wq		0xf30fe6
#define OP_MOVNTQ_Mq_Vq			0x0fe7
#define OP_MOVNTDQ_Mdq_Vdq		0x660fe7
#define OP_PSUBSB_Pq_Qq			0x0fe8
#define OP_PSUBSB_Vdq_Wdq		0x660fe8
#define OP_PSUBSW_Pq_Qq			0x0fe9
#define OP_PSUBSW_Vdq_Wdq		0x660fe9
#define OP_PMINSW_Pq_Qq			0x0fea
#define OP_PMINSW_Vdq_Wdq		0x660fea
#define OP_POR_Pq_Qq			0x0feb
#define OP_POR_Vdq_Wdq			0x660feb
#define OP_PADDSB_Pq_Qq			0x0fec
#define OP_PADDSB_Vdq_Wdq		0x660fec
#define OP_PADDSW_Pq_Qq			0x0fed
#define OP_PADDSW_Vdq_Wdq		0x660fed
#define OP_PMAXSW_Pq_Qq			0x0fee
#define OP_PMAXSW_Vdq_Wdq		0x660fee
#define OP_PXOR_Pq_Qq			0x0fef
#define OP_PXOR_Vdq_Wdq			0x660fef

#define OP_LDDQU_Vdq_Mdq		0xf20ff0
#define OP_PSLLW_Pq_Qq			0x0ff1
#define OP_PSLLW_Vdq_Wdq		0x660ff1
#define OP_PSLLD_Pq_Qq			0x0ff2
#define OP_PSLLD_Vdq_Wdq		0x660ff2
#define OP_PSLLQ_Pq_Qq			0x0ff3
#define OP_PSLLQ_Vdq_Wdq		0x660ff3
#define OP_PMULUDQ_Pq_Qq		0x0ff4
#define OP_PMULUDQ_Vdq_Wdq		0x660ff4
#define OP_PMADDWD_Pq_Qq		0x0ff5
#define OP_PMADDWD_Vdq_Wdq		0x660ff5
#define OP_PSADBW_Pq_Qq			0x0ff6
#define OP_PSADBW_Vdq_Wdq		0x660ff6
#define OP_MASKMOVQ_Pq_Qq		0x0ff7
#define OP_MASKMOVDQU_Vdq_Wdq	0x660ff7
#define OP_PSUBB_Pq_Qq			0x0ff8
#define OP_PSUBB_Vdq_Wdq		0x660ff8
#define OP_PSUBW_Pq_Qq			0x0ff9
#define OP_PSUBW_Vdq_Wdq		0x660ff9
#define OP_PSUBD_Pq_Qq			0x0ffa
#define OP_PSUBD_Vdq_Wdq		0x660ffa
#define OP_PSUBQ_Pq_Qq			0x0ffb
#define OP_PSUBQ_Vdq_Wdq		0x660ffb
#define OP_PADDB_Pq_Qq			0x0ffc
#define OP_PADDB_Vdq_Wdq		0x660ffc
#define OP_PADDW_Pq_Qq			0x0ffd
#define OP_PADDW_Vdq_Wdq		0x660ffd
#define OP_PADDD_Pq_Qq			0x0ffe
#define OP_PADDD_Vdq_Wdq		0x660ffe


/* triple byte opcodes (0f 38) */
#define OP_PSHUFB_Pq_Qq			0x0f3800
#define OP_PSHUFB_Vdq_Wdq		0x660f3800
#define OP_PHADDW_Pq_Qq			0x0f3801
#define OP_PHADDW_Vdq_Wdq		0x660f3801
#define OP_PHADDD_Pq_Qq			0x0f3802
#define OP_PHADDD_Vdq_Wdq		0x660f3802
#define OP_PHADDSW_Pq_Qq		0x0f3803
#define OP_PHADDSW_Vdq_Wdq		0x660f3803
#define OP_PMADDUBSW_Pq_Qq		0x0f3804
#define OP_PMADDUBSW_Vdq_Wdq	0x660f3804
#define OP_PHSUBW_Pq_Qq			0x0f3805
#define OP_PHSUBW_Vdq_Wdq		0x660f3805
#define OP_PHSUBD_Pq_Qq			0x0f3806
#define OP_PHSUBD_Vdq_Wdq		0x660f3806
#define OP_PHSUBSW_Pq_Qq		0x0f3807
#define OP_PHSUBSW_Vdq_Wdq		0x660f3807
#define OP_PSIGNB_Pq_Qq			0x0f3808
#define OP_PSIGNB_Vdq_Wdq		0x660f3808
#define OP_PSIGNW_Pq_Qq			0x0f3809
#define OP_PSIGNW_Vdq_Wdq		0x660f3809
#define OP_PSIGND_Pq_Qq			0x0f380a
#define OP_PSIGND_Vdq_Wdq		0x660f380a
#define OP_PMULHRSW_Pq_Qq		0x0f380b
#define OP_PMULHRSW_Vdq_Wdq		0x660f380b

#define OP_PBLENDVB_Vdq_Wdq		0x660f3810
#define OP_PBLENDVPS_Vdq_Wdq	0x660f3814
#define OP_PBLENDVPD_Vdq_Wdq	0x660f3815
#define OP_PTEST_Vdq_Wdq		0x660f3817
#define OP_PABSB_Pq_Qq			0x0f381c
#define OP_PABSB_Vdq_Wdq		0x660f381c
#define OP_PABSW_Pq_Qq			0x0f381d
#define OP_PABSW_Vdq_Wdq		0x660f381d
#define OP_PABSD_Pq_Qq			0x0f381e
#define OP_PABSD_Vdq_Wdq		0x660f381e

#define OP_PMOVSXBW_Vdq_Udq		0x660f3820
#define OP_PMOVSXBD_Vdq_Udq		0x660f3821
#define OP_PMOVSXBQ_Vdq_Udq		0x660f3822
#define OP_PMOVSXWD_Vdq_Udq		0x660f3823
#define OP_PMOVSXWQ_Vdq_Udq		0x660f3824
#define OP_PMOVSXDQ_Vdq_Udq		0x660f3825
#define OP_PMULDQ_Vdq_Udq		0x660f3828
#define OP_PCMPEQQ_Vdq_Udq		0x660f3829
#define OP_MOVNTDQA_Vdq_Udq		0x660f382a
#define OP_PACKUSDW_Vdq_Udq		0x660f382b

#define OP_PMOVZXBW_Vdq_Udq		0x660f3830
#define OP_PMOVZXBD_Vdq_Udq		0x660f3831
#define OP_PMOVZXBQ_Vdq_Udq		0x660f3832
#define OP_PMOVZXWD_Vdq_Udq		0x660f3833
#define OP_PMOVZXWQ_Vdq_Udq		0x660f3834
#define OP_PMOVZXDQ_Vdq_Udq		0x660f3835
#define OP_PMINSB_Vdq_Udq		0x660f3838
#define OP_PMINSD_Vdq_Udq		0x660f3839
#define OP_PMINUW_Vdq_Udq		0x660f383a
#define OP_PMINUD_Vdq_Udq		0x660f383b
#define OP_PMAXSB_Vdq_Udq		0x660f383c
#define OP_PMAXSD_Vdq_Udq		0x660f383d
#define OP_PMAXUW_Vdq_Udq		0x660f383e
#define OP_PMAXUD_Vdq_Udq		0x660f383f

#define OP_MULLD_Vdq_Wdq		0x660f3840
#define OP_PHMINPOSUW_Vdq_Wdq	0x660f3841

#define OP_NVEPT_Gd_Mdq			0x660f3880
#define OP_NVVPID_Gd_Mdq		0x660f3881

#define OP_MOVBE_Gv_Mv			0x0f38f0
#define OP_CRC32_Gd_Eb			0xf20f38f0
#define OP_MOVBE_Mv_Gv			0x0f38f1
#define OP_CRC32_Gd_Ev			0xf20f38f1


/* triple byte opcodes (0f 3a) */
#define OP_ROUNDPS_Vdq_Wdq_Ib	0x660f3a08
#define OP_ROUNDPD_Vdq_Wdq_Ib	0x660f3a09
#define OP_ROUNDSS_Vss_Wss_Ib	0x660f3a0a
#define OP_ROUNDSD_Vsd_Wsd_Ib	0x660f3a0b
#define OP_BLENDPS_Vdq_Wdq_Ib	0x660f3a0c
#define OP_BLENDPD_Vdq_Wdq_Ib	0x660f3a0d
#define OP_PBLENDW_Vdq_Wdq_Ib	0x660f3a0e
#define OP_PALIGNR_Pq_Qq_Ib		0x0f3a0f
#define OP_PALIGNR_Vdq_Wdq_Ib	0x660f3a0f

#define OP_EXTRB_Rd_Vdq_Ib		0x660f3a14
#define OP_EXTRW_Rd_Vdq_Ib		0x660f3a15
#define OP_EXTRD_Rd_Vdq_Ib		0x660f3a16
#define OP_EXTRACTPS_Ed_Vdq_Ib	0x660f3a17

#define OP_PINSRB_Vdq_Rd_Ib		0x660f3a20
#define OP_INSERTPS_Vdq_Udq_Ib	0x660f3a21
#define OP_PINSRD_Vdq_Ed_Ib		0x660f3a22

#define OP_DPPS_Vdq_Wdq_Ib		0x660f3a40
#define OP_DPPD_Vdq_Wdq_Ib		0x660f3a41
#define OP_MPSADBW_Vdq_Wdq_Ib	0x660f3a42

#define OP_PCMPESTRM_Vdq_Wdq_Ib	0x660f3a60
#define OP_PCMPESTRI_Vdq_Wdq_Ib	0x660f3a61
#define OP_PCMPISTRM_Vdq_Wdq_Ib	0x660f3a62
#define OP_PCMPISTRI_Vdq_Wdq_Ib	0x660f3a63


/* floating point opcodes */
#define OP_FADD_ST0_STn			0xd8c0
#define OP_FMUL_ST0_STn			0xd8c8
#define OP_FCOM_ST0_STn			0xd8d0
#define OP_FCOMP_ST0_STn		0xd8d8
#define OP_FSUB_ST0_STn			0xd8e0
#define OP_FSUBR_ST0_STn		0xd8e8
#define OP_FDIV_ST0_STn			0xd8f0
#define OP_FDIVR_ST0_STn		0xd8f8
#define OP_FLD_ST0_STn			0xd9c0
#define OP_FXCH_ST0_STn			0xd9c8
#define OP_FNOP					0xd9d0
#define OP_FCHS					0xd9e0
#define OP_FABS					0xd9e1
#define OP_FTST					0xd9e4
#define OP_FXAM					0xd9e5
#define OP_FLD1					0xd9e8
#define OP_FLDL2T				0xd9e9
#define OP_FLDL2E				0xd9ea
#define OP_FLDPI				0xd9eb
#define OP_FLDLG2				0xd9ec
#define OP_FLDLN2				0xd9ed
#define OP_FLDZ					0xd9ee
#define OP_F2XM1				0xd9f0
#define OP_FYL2X				0xd9f1
#define OP_FPTAN				0xd9f2
#define OP_FPATAN				0xd9f3
#define OP_FXTRACT				0xd9f4
#define OP_FPREM1				0xd9f5
#define OP_FDECSTP				0xd9f6
#define OP_FINCSTP				0xd9f7
#define OP_FPREM				0xd9f8
#define OP_FYL2XP1				0xd9f9
#define OP_FSQRT				0xd9fa
#define OP_FSINCOS				0xd9fb
#define OP_FRNDINT				0xd9fc
#define OP_FSCALE				0xd9fd
#define OP_FSIN					0xd9fe
#define OP_FCOS					0xd9ff
#define OP_FCMOVB_ST0_STn		0xdac0
#define OP_FCMOVE_ST0_STn		0xdac8
#define OP_FCMOVBE_ST0_STn		0xdad0
#define OP_FCMOVU_ST0_STn		0xdad8
#define OP_FUCOMPP				0xdae9
#define OP_FCMOVNB_ST0_STn		0xdbc0
#define OP_FCMOVNE_ST0_STn		0xdbc8
#define OP_FCMOVNBE_ST0_STn		0xdbd0
#define OP_FCMOVNU_ST0_STn		0xdbd8
#define OP_FCLEX				0xdbe2
#define OP_FINIT				0xdbe3
#define OP_FUCOMI_ST0_STn		0xdbe8
#define OP_FCOMI_ST0_STn		0xdbf0
#define OP_FADD_STn_ST0			0xdcc0
#define OP_FMUL_STn_ST0			0xdcc8
#define OP_FSUBR_STn_ST0		0xdce0
#define OP_FSUB_STn_ST0			0xdce8
#define OP_FDIVR_STn_ST0		0xdcf0
#define OP_FDIV_STn_ST0			0xdcf8
#define OP_FFREE_STn			0xddc0
#define OP_FST_STn				0xddd0
#define OP_FSTP_STn				0xddd8
#define OP_FUCOM_STn_ST0		0xdde0
#define OP_FUCOMP_STn			0xdde8
#define OP_FADDP_STn_ST0		0xdec0
#define OP_FMULP_STn_ST0		0xdec8
#define OP_FCOMPP				0xded9
#define OP_FSUBRP_STn_ST0		0xdee0
#define OP_FSUBP_STn_ST0		0xdee8
#define OP_FDIVRP_STn_ST0		0xdef0
#define OP_FDIVP_STn_ST0		0xdef8
#define OP_FSTSW_AX				0xdfe0
#define OP_FCOMIP_ST0_STn		0xdff0



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* use x86code * to reference generated code */
typedef UINT8					x86code;


/* this structure tracks information about a link */
typedef struct _emit_link emit_link;
struct _emit_link
{
	x86code *		target;
	UINT8			size;
};



/***************************************************************************
    MACROS
***************************************************************************/

#define DECLARE_MEMPARAMS				UINT8 base, UINT8 index, UINT8 scale, INT32 disp
#define MEMPARAMS						base, index, scale, disp

#ifndef PTR64
#define MBISD(base, index, scale, disp)	(base), (index), (scale), (INT32)(disp)
#define MABS(addr)						MBISD(REG_NONE, REG_NONE, 1, (addr))
#define MBD(base, disp)					MBISD((base), REG_NONE, 1, (disp))
#define MISD(index, scale, disp)		MBISD(REG_NONE, (index), (scale), (disp))
#else
#define MBISD(base, index, scale, disp)	(base), (index), (scale), (disp)
#define MBD(base, disp)					MBISD((base), REG_NONE, 1, (disp))
#define MISD(index, scale, disp)		MBISD(REG_NONE, (index), (scale), (disp))
#endif



/***************************************************************************
    CORE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    make_modrm - assemble a modrm byte from the
    three components
-------------------------------------------------*/

INLINE UINT8 make_modrm(UINT8 mode, UINT8 reg, UINT8 rm)
{
	assert(mode < 4);
	assert(reg < REG_MAX);
	assert(rm < REG_MAX);
	return (mode << 6) | ((reg & 7) << 3) | (rm & 7);
}


/*-------------------------------------------------
    make_sib - assemble an sib byte from the
    three components
-------------------------------------------------*/

INLINE UINT8 make_sib(UINT8 scale, UINT8 index, UINT8 base)
{
	static const UINT8 scale_lookup[9] = { 0<<6, 0<<6, 1<<6, 0<<6, 2<<6, 0<<6, 0<<6, 0<<6, 3<<6 };
	assert(scale == 1 || scale == 2 || scale == 4 || scale == 8);
	assert(index < REG_MAX);
	assert(base < REG_MAX);
	return scale_lookup[scale] | ((index & 7) << 3) | (base & 7);
}


/*-------------------------------------------------
    emit_byte - emit a byte
-------------------------------------------------*/

INLINE void emit_byte(x86code **emitptr, UINT8 byte)
{
	*((UINT8 *)*emitptr) = byte;
	*emitptr += 1;
}


/*-------------------------------------------------
    emit_word - emit a word
-------------------------------------------------*/

INLINE void emit_word(x86code **emitptr, UINT16 word)
{
	*((UINT16 *)*emitptr) = word;
	*emitptr += 2;
}


/*-------------------------------------------------
    emit_dword - emit a dword
-------------------------------------------------*/

INLINE void emit_dword(x86code **emitptr, UINT32 dword)
{
	*((UINT32 *)*emitptr) = dword;
	*emitptr += 4;
}


/*-------------------------------------------------
    emit_qword - emit a dword
-------------------------------------------------*/

INLINE void emit_qword(x86code **emitptr, UINT64 qword)
{
	*((UINT64 *)*emitptr) = qword;
	*emitptr += 8;
}



/***************************************************************************
    GENERIC OPCODE EMITTERS
***************************************************************************/

/*-------------------------------------------------
    emit_op - emit a 1, 2, or 3-byte opcode,
    along with any necessary REX prefixes for x64
-------------------------------------------------*/

INLINE void emit_op(x86code **emitptr, UINT32 op, UINT8 opsize, UINT8 reg, UINT8 sib, UINT8 rm)
{
	if (opsize == OP_16BIT)
		emit_byte(emitptr, PREFIX_OPSIZE);

#ifdef PTR64
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


/*-------------------------------------------------
    emit_op_simple - emit a simple opcode
-------------------------------------------------*/

INLINE void emit_op_simple(x86code **emitptr, UINT32 op, UINT8 opsize)
{
	emit_op(emitptr, op, opsize, 0, 0, 0);
}


/*-------------------------------------------------
    emit_op_reg - emit a simple opcode that has
    a register parameter
-------------------------------------------------*/

INLINE void emit_op_reg(x86code **emitptr, UINT32 op, UINT8 opsize, UINT8 reg)
{
	emit_op(emitptr, op, opsize, 0, 0, reg);
}


/*-------------------------------------------------
    emit_op_modrm_reg - emit an opcode with a
    register modrm byte
-------------------------------------------------*/

INLINE void emit_op_modrm_reg(x86code **emitptr, UINT32 op, UINT8 opsize, UINT8 reg, UINT8 rm)
{
	assert(reg < REG_MAX);
	assert(rm < REG_MAX);

	emit_op(emitptr, op, opsize, reg, 0, rm);
	emit_byte(emitptr, make_modrm(3, reg, rm));
}


/*-------------------------------------------------
    emit_op_modrm_mem - emit an opcode with a
    memory modrm byte
-------------------------------------------------*/

INLINE void emit_op_modrm_mem(x86code **emitptr, UINT32 op, UINT8 opsize, UINT8 reg, DECLARE_MEMPARAMS)
{
	assert(reg < REG_MAX);
	assert(base < REG_MAX || base == REG_NONE);
	assert(index < REG_MAX || index == REG_NONE);

	/* turn off the OPFLAG_8BITRM flag since it only applies to register operands */
	op &= ~OPFLAG_8BITRM;

	/* displacement only case */
	if (base == REG_NONE && index == REG_NONE)
	{
#ifndef PTR64
		emit_op(emitptr, op, opsize, reg, 0, 5);
		emit_byte(emitptr, make_modrm(0, reg, 5));
		emit_dword(emitptr, disp);
#else
		fatalerror("Invalid absolute mode in 64-bits!\n");
#endif
	}

	/* base only case */
	else if (index == REG_NONE && (base & 7) != REG_ESP)
	{
		emit_op(emitptr, op, opsize, reg, 0, base);

		/* mode 0 for no offset */
		if (disp == 0 && (base & 7) != REG_EBP)
			emit_byte(emitptr, make_modrm(0, reg, base));

		/* mode 1 for 1-byte offset */
		else if ((INT8)disp == disp)
		{
			emit_byte(emitptr, make_modrm(1, reg, base));
			emit_byte(emitptr, (INT8)disp);
		}

		/* mode 2 for 4-byte offset */
		else
		{
			emit_byte(emitptr, make_modrm(2, reg, base));
			emit_dword(emitptr, disp);
		}
	}

	/* full BISD case */
	else
	{
		assert(index != REG_ESP);
		emit_op(emitptr, op, opsize, reg, index, base);

		/* a "none" index == REG_ESP */
		if (index == REG_NONE)
			index = REG_ESP;

		/* no base is a special case */
		if (base == REG_NONE)
		{
			emit_byte(emitptr, make_modrm(0, reg, 4));
			emit_byte(emitptr, make_sib(scale, index, REG_EBP));
			emit_dword(emitptr, disp);
		}

		/* mode 0 for no offset */
		else if (disp == 0 && (base & 7) != REG_EBP)
		{
			emit_byte(emitptr, make_modrm(0, reg, 4));
			emit_byte(emitptr, make_sib(scale, index, base));
		}

		/* mode 1 for 1-byte offset */
		else if ((INT8)disp == disp)
		{
			emit_byte(emitptr, make_modrm(1, reg, 4));
			emit_byte(emitptr, make_sib(scale, index, base));
			emit_byte(emitptr, (INT8)disp);
		}

		/* mode 2 for 4-byte offset */
		else
		{
			emit_byte(emitptr, make_modrm(2, reg, 4));
			emit_byte(emitptr, make_sib(scale, index, base));
			emit_dword(emitptr, disp);
		}
	}
}



/***************************************************************************
    GENERIC OPCODE + IMMEDIATE EMITTERS
***************************************************************************/

/*-------------------------------------------------
    emit_op_modrm_reg_imm8 - emit an opcode with a
    register modrm byte and an 8-bit immediate
-------------------------------------------------*/

INLINE void emit_op_modrm_reg_imm8(x86code **emitptr, UINT32 op, UINT8 opsize, UINT8 reg, UINT8 rm, UINT8 imm)
{
	emit_op_modrm_reg(emitptr, op, opsize, reg, rm);
	emit_byte(emitptr, imm);
}


/*-------------------------------------------------
    emit_op_modrm_mem_imm8 - emit an opcode with a
    memory modrm byte and an 8-bit immediate
-------------------------------------------------*/

INLINE void emit_op_modrm_mem_imm8(x86code **emitptr, UINT32 op, UINT8 opsize, UINT8 reg, DECLARE_MEMPARAMS, UINT8 imm)
{
	emit_op_modrm_mem(emitptr, op, opsize, reg, MEMPARAMS);
	emit_byte(emitptr, imm);
}


/*-------------------------------------------------
    emit_op_modrm_reg_imm16 - emit an opcode with a
    register modrm byte and a 16-bit immediate
-------------------------------------------------*/

INLINE void emit_op_modrm_reg_imm16(x86code **emitptr, UINT32 op, UINT8 opsize, UINT8 reg, UINT8 rm, UINT16 imm)
{
	emit_op_modrm_reg(emitptr, op, opsize, reg, rm);
	emit_word(emitptr, imm);
}


/*-------------------------------------------------
    emit_op_modrm_mem_imm16 - emit an opcode with a
    memory modrm byte and a 16-bit immediate
-------------------------------------------------*/

INLINE void emit_op_modrm_mem_imm16(x86code **emitptr, UINT32 op, UINT8 opsize, UINT8 reg, DECLARE_MEMPARAMS, UINT16 imm)
{
	emit_op_modrm_mem(emitptr, op, opsize, reg, MEMPARAMS);
	emit_word(emitptr, imm);
}


/*-------------------------------------------------
    emit_op_modrm_reg_imm816 - emit an opcode with
    a register modrm byte and an 8-bit or 16-bit
    immediate
-------------------------------------------------*/

INLINE void emit_op_modrm_reg_imm816(x86code **emitptr, UINT32 op8, UINT32 op16, UINT8 opsize, UINT8 reg, UINT8 rm, UINT16 imm)
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


/*-------------------------------------------------
    emit_op_modrm_mem_imm816 - emit an opcode with
    a memory modrm byte and an 8-bit or 16-bit
    immediate
-------------------------------------------------*/

INLINE void emit_op_modrm_mem_imm816(x86code **emitptr, UINT32 op8, UINT32 op16, UINT8 opsize, UINT8 reg, DECLARE_MEMPARAMS, UINT16 imm)
{
	if ((INT8)imm == (INT16)imm)
	{
		emit_op_modrm_mem(emitptr, op8, opsize, reg, MEMPARAMS);
		emit_byte(emitptr, imm);
	}
	else
	{
		emit_op_modrm_mem(emitptr, op16, opsize, reg, MEMPARAMS);
		emit_word(emitptr, imm);
	}
}


/*-------------------------------------------------
    emit_op_modrm_reg_imm32 - emit an opcode with a
    register modrm byte and a 32-bit immediate
-------------------------------------------------*/

INLINE void emit_op_modrm_reg_imm32(x86code **emitptr, UINT32 op, UINT8 opsize, UINT8 reg, UINT8 rm, UINT32 imm)
{
	emit_op_modrm_reg(emitptr, op, opsize, reg, rm);
	emit_dword(emitptr, imm);
}


/*-------------------------------------------------
    emit_op_modrm_mem_imm32 - emit an opcode with a
    memory modrm byte and a 32-bit immediate
-------------------------------------------------*/

INLINE void emit_op_modrm_mem_imm32(x86code **emitptr, UINT32 op, UINT8 opsize, UINT8 reg, DECLARE_MEMPARAMS, UINT32 imm)
{
	emit_op_modrm_mem(emitptr, op, opsize, reg, MEMPARAMS);
	emit_dword(emitptr, imm);
}


/*-------------------------------------------------
    emit_op_modrm_reg_imm832 - emit an opcode with
    a register modrm byte and an 8-bit or 32-bit
    immediate
-------------------------------------------------*/

INLINE void emit_op_modrm_reg_imm832(x86code **emitptr, UINT32 op8, UINT32 op32, UINT8 opsize, UINT8 reg, UINT8 rm, UINT32 imm)
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


/*-------------------------------------------------
    emit_op_modrm_mem_imm832 - emit an opcode with
    a memory modrm byte and an 8-bit or 32-bit
    immediate
-------------------------------------------------*/

INLINE void emit_op_modrm_mem_imm832(x86code **emitptr, UINT32 op8, UINT32 op32, UINT8 opsize, UINT8 reg, DECLARE_MEMPARAMS, UINT32 imm)
{
	if ((INT8)imm == (INT32)imm)
	{
		emit_op_modrm_mem(emitptr, op8, opsize, reg, MEMPARAMS);
		emit_byte(emitptr, imm);
	}
	else
	{
		emit_op_modrm_mem(emitptr, op32, opsize, reg, MEMPARAMS);
		emit_dword(emitptr, imm);
	}
}



/***************************************************************************
    SIMPLE OPCODE EMITTERS
***************************************************************************/

INLINE void emit_nop(x86code **emitptr)    { emit_op_simple(emitptr, OP_NOP, OP_32BIT); }
INLINE void emit_int_3(x86code **emitptr)  { emit_op_simple(emitptr, OP_INT_3, OP_32BIT); }
INLINE void emit_ret(x86code **emitptr)    { emit_op_simple(emitptr, OP_RETN, OP_32BIT); }
INLINE void emit_cdq(x86code **emitptr)    { emit_op_simple(emitptr, OP_CDQ, OP_32BIT); }
INLINE void emit_clc(x86code **emitptr)    { emit_op_simple(emitptr, OP_CLC, OP_32BIT); }
INLINE void emit_stc(x86code **emitptr)    { emit_op_simple(emitptr, OP_STC, OP_32BIT); }
INLINE void emit_cmc(x86code **emitptr)    { emit_op_simple(emitptr, OP_CMC, OP_32BIT); }
INLINE void emit_pushf(x86code **emitptr)  { emit_op_simple(emitptr, OP_PUSHF_Fv, OP_32BIT); }
INLINE void emit_popf(x86code **emitptr)   { emit_op_simple(emitptr, OP_POPF_Fv, OP_32BIT); }
INLINE void emit_cpuid(x86code **emitptr)  { emit_op_simple(emitptr, OP_CPUID, OP_32BIT); }

#ifndef PTR64
INLINE void emit_pushad(x86code **emitptr) { emit_op_simple(emitptr, OP_PUSHA, OP_32BIT); }
INLINE void emit_popad(x86code **emitptr)  { emit_op_simple(emitptr, OP_POPA, OP_32BIT); }
INLINE void emit_lahf(x86code **emitptr)   { emit_op_simple(emitptr, OP_LAHF, OP_32BIT); }
INLINE void emit_sahf(x86code **emitptr)   { emit_op_simple(emitptr, OP_SAHF, OP_32BIT); }
#endif

#ifdef PTR64
INLINE void emit_cqo(x86code **emitptr)    { emit_op_simple(emitptr, OP_CQO, OP_64BIT); }
#endif



/***************************************************************************
    CALL/JUMP EMITTERS
***************************************************************************/

/*-------------------------------------------------
    resolve_link - resolve a link in a jump
    instruction
-------------------------------------------------*/

INLINE void resolve_link(x86code **destptr, const emit_link *linkinfo)
{
	INT64 delta = *destptr - linkinfo->target;
	if (linkinfo->size == 1)
	{
		assert((INT8)delta == delta);
		((INT8 *)linkinfo->target)[-1] = (INT8)delta;
	}
	else if (linkinfo->size == 2)
	{
		assert((INT16)delta == delta);
		((INT16 *)linkinfo->target)[-1] = (INT16)delta;
	}
	else
	{
		assert((INT32)delta == delta);
		((INT32 *)linkinfo->target)[-1] = (INT32)delta;
	}
}


/*-------------------------------------------------
    emit_call_*
-------------------------------------------------*/

INLINE void emit_call_link(x86code **emitptr, emit_link *linkinfo)
{
	emit_op_simple(emitptr, OP_CALL_Jz, OP_32BIT);
	emit_dword(emitptr, 0);
	linkinfo->target = *emitptr;
	linkinfo->size = 4;
}

INLINE void emit_call(x86code **emitptr, x86code *target)
{
	emit_link link;
	emit_call_link(emitptr, &link);
	resolve_link(&target, &link);
}


/*-------------------------------------------------
    emit_jmp_*
-------------------------------------------------*/

INLINE void emit_jmp_short_link(x86code **emitptr, emit_link *linkinfo)
{
	emit_op_simple(emitptr, OP_JMP_Jb, OP_32BIT);
	emit_byte(emitptr, 0);
	linkinfo->target = *emitptr;
	linkinfo->size = 1;
}

INLINE void emit_jmp_near_link(x86code **emitptr, emit_link *linkinfo)
{
	emit_op_simple(emitptr, OP_JMP_Jz, OP_32BIT);
	emit_dword(emitptr, 0);
	linkinfo->target = *emitptr;
	linkinfo->size = 4;
}

INLINE void emit_jmp(x86code **emitptr, x86code *target)
{
	INT32 delta = target - (*emitptr + 2);
	emit_link link;

	if ((INT8)delta == delta)
		emit_jmp_short_link(emitptr, &link);
	else
		emit_jmp_near_link(emitptr, &link);
	resolve_link(&target, &link);
}


/*-------------------------------------------------
    emit_jcc_*
-------------------------------------------------*/

INLINE void emit_jcc_short_link(x86code **emitptr, UINT8 cond, emit_link *linkinfo)
{
	emit_op_simple(emitptr, OP_JCC_O_Jb + cond, OP_32BIT);
	emit_byte(emitptr, 0);
	linkinfo->target = *emitptr;
	linkinfo->size = 1;
}

INLINE void emit_jcc_near_link(x86code **emitptr, UINT8 cond, emit_link *linkinfo)
{
	emit_op_simple(emitptr, OP_JCC_O_Jv + cond, OP_32BIT);
	emit_dword(emitptr, 0);
	linkinfo->target = *emitptr;
	linkinfo->size = 4;
}

INLINE void emit_jcc(x86code **emitptr, UINT8 cond, x86code *target)
{
	INT32 delta = *emitptr + 2 - target;
	emit_link link;

	if ((INT8)delta == delta)
		emit_jcc_short_link(emitptr, cond, &link);
	else
		emit_jcc_near_link(emitptr, cond, &link);
	resolve_link(&target, &link);
}


/*-------------------------------------------------
    emit_jecxz_*
-------------------------------------------------*/

INLINE void emit_jecxz_link(x86code **emitptr, emit_link *linkinfo)
{
	emit_op_simple(emitptr, OP_JrCXZ_Jb, OP_32BIT);
	emit_byte(emitptr, 0);
	linkinfo->target = *emitptr;
	linkinfo->size = 1;
}

INLINE void emit_jecxz(x86code **emitptr, x86code *target)
{
	emit_link link;
	emit_jecxz_link(emitptr, &link);
	resolve_link(&target, &link);
}

#ifdef PTR64

INLINE void emit_jrcxz_link(x86code **emitptr, emit_link *linkinfo)
{
	emit_op_simple(emitptr, OP_JrCXZ_Jb, OP_64BIT);
	emit_byte(emitptr, 0);
	linkinfo->target = *emitptr;
	linkinfo->size = 1;
}

INLINE void emit_jrcxz(x86code **emitptr, x86code *target)
{
	emit_link link;
	emit_jrcxz_link(emitptr, &link);
	resolve_link(&target, &link);
}

#endif


/*-------------------------------------------------
    emit_call/jmp_*
-------------------------------------------------*/

#ifndef PTR64
INLINE void emit_call_r32(x86code **emitptr, UINT8 dreg)		{ emit_op_modrm_reg(emitptr, OP_G5, OP_32BIT, 2, dreg); }
INLINE void emit_call_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 2, MEMPARAMS); }
INLINE void emit_jmp_r32(x86code **emitptr, UINT8 dreg)			{ emit_op_modrm_reg(emitptr, OP_G5, OP_32BIT, 4, dreg); }
INLINE void emit_jmp_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 4, MEMPARAMS); }
#else
INLINE void emit_call_r64(x86code **emitptr, UINT8 dreg)		{ emit_op_modrm_reg(emitptr, OP_G5, OP_32BIT, 2, dreg); }
INLINE void emit_call_m64(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 2, MEMPARAMS); }
INLINE void emit_jmp_r64(x86code **emitptr, UINT8 dreg)			{ emit_op_modrm_reg(emitptr, OP_G5, OP_32BIT, 4, dreg); }
INLINE void emit_jmp_m64(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 4, MEMPARAMS); }
#endif


/*-------------------------------------------------
    emit_ret_*
-------------------------------------------------*/

INLINE void emit_ret_imm(x86code **emitptr, UINT16 imm)
{
	emit_op_simple(emitptr, OP_RETN_Iw, OP_32BIT);
	emit_word(emitptr, imm);
}



/***************************************************************************
    PUSH/POP EMITTERS
***************************************************************************/

INLINE void emit_push_imm(x86code **emitptr, INT32 imm)
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

#ifndef PTR64

INLINE void emit_push_r32(x86code **emitptr, UINT8 reg)							{ emit_op_reg(emitptr, OP_PUSH_rAX + (reg & 7), OP_32BIT, reg); }
INLINE void emit_push_m32(x86code **emitptr, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 6, MEMPARAMS); }
INLINE void emit_pop_r32(x86code **emitptr, UINT8 reg) 							{ emit_op_reg(emitptr, OP_POP_rAX + (reg & 7), OP_32BIT, reg); }
INLINE void emit_pop_m32(x86code **emitptr, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_G1A_Ev, OP_32BIT, 0, MEMPARAMS); }

#else

INLINE void emit_push_r64(x86code **emitptr, UINT8 reg)							{ emit_op_reg(emitptr, OP_PUSH_rAX + (reg & 7), OP_32BIT, reg); }
INLINE void emit_push_m64(x86code **emitptr, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_G5, OP_32BIT, 6, MEMPARAMS); }
INLINE void emit_pop_r64(x86code **emitptr, UINT8 reg)							{ emit_op_reg(emitptr, OP_POP_rAX + (reg & 7), OP_32BIT, reg); }
INLINE void emit_pop_m64(x86code **emitptr, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_G1A_Ev, OP_32BIT, 0, MEMPARAMS); }

#endif



/***************************************************************************
    MOVE EMITTERS
***************************************************************************/

/*-------------------------------------------------
    emit_mov_r8_*
-------------------------------------------------*/

INLINE void emit_mov_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)
{
	emit_op_reg(emitptr, OP_MOV_AL_Ib | (dreg & 7), OP_32BIT, dreg);
	emit_byte(emitptr, imm);
}

INLINE void emit_mov_r8_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_MOV_Gb_Eb, OP_32BIT, dreg, sreg); }
INLINE void emit_mov_r8_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_MOV_Gb_Eb, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_mov_m8_r8(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)	{ emit_op_modrm_mem(emitptr, OP_MOV_Eb_Gb, OP_32BIT, sreg, MEMPARAMS); }
INLINE void emit_mov_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_op_modrm_mem_imm8(emitptr, OP_G11_Eb_Ib, OP_32BIT, 0, MEMPARAMS, imm); }


/*-------------------------------------------------
    emit_xchg_r8_*
-------------------------------------------------*/

INLINE void emit_xchg_r8_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg)
{
	if (dreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (sreg & 7), OP_32BIT, sreg);
	else if (sreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (dreg & 7), OP_32BIT, dreg);
	else
 		emit_op_modrm_reg(emitptr, OP_XCHG_Eb_Gb, OP_32BIT, dreg, sreg);
}


/*-------------------------------------------------
    emit_mov_r16_*
-------------------------------------------------*/

INLINE void emit_mov_r16_imm(x86code **emitptr, UINT8 dreg, UINT16 imm)
{
	emit_op_reg(emitptr, OP_MOV_rAX_Iv | (dreg & 7), OP_16BIT, dreg);
	emit_word(emitptr, imm);
}

INLINE void emit_mov_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg)							{ emit_op_modrm_reg(emitptr, OP_MOV_Gv_Ev, OP_16BIT, dreg, sreg); }
INLINE void emit_mov_r16_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_MOV_Gv_Ev, OP_16BIT, dreg, MEMPARAMS); }
INLINE void emit_mov_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT16 imm)					{ emit_op_modrm_mem_imm16(emitptr, OP_G11_Ev_Iz, OP_16BIT, 0, MEMPARAMS, imm); }
INLINE void emit_mov_m16_r16(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)					{ emit_op_modrm_mem(emitptr, OP_MOV_Ev_Gv, OP_16BIT, sreg, MEMPARAMS); }
INLINE void emit_movsx_r16_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Eb, OP_16BIT, dreg, sreg); }
INLINE void emit_movsx_r16_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Eb, OP_16BIT, dreg, MEMPARAMS); }
INLINE void emit_movzx_r16_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg) 						{ emit_op_modrm_reg(emitptr, OP_MOVZX_Gv_Eb, OP_16BIT, dreg, sreg); }
INLINE void emit_movzx_r16_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Eb, OP_16BIT, dreg, MEMPARAMS); }
INLINE void emit_cmovcc_r16_r16(x86code **emitptr, UINT8 cond, UINT8 dreg, UINT8 sreg) 			{ emit_op_modrm_reg(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_16BIT, dreg, sreg); }
INLINE void emit_cmovcc_r16_m16(x86code **emitptr, UINT8 cond, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_16BIT, dreg, MEMPARAMS); }


/*-------------------------------------------------
    emit_xchg_r16_*
-------------------------------------------------*/

INLINE void emit_xchg_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg)
{
	if (dreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (sreg & 7), OP_16BIT, sreg);
	else if (sreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (dreg & 7), OP_16BIT, dreg);
	else
 		emit_op_modrm_reg(emitptr, OP_XCHG_Ev_Gv, OP_16BIT, dreg, sreg);
}


/*-------------------------------------------------
    emit_mov_r32_*
-------------------------------------------------*/

INLINE void emit_mov_r32_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)
{
	emit_op_reg(emitptr, OP_MOV_rAX_Iv | (dreg & 7), OP_32BIT, dreg);
	emit_dword(emitptr, imm);
}

INLINE void emit_mov_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg)							{ emit_op_modrm_reg(emitptr, OP_MOV_Gv_Ev, OP_32BIT, dreg, sreg); }
INLINE void emit_mov_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_MOV_Gv_Ev, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_mov_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)					{ emit_op_modrm_mem_imm32(emitptr, OP_G11_Ev_Iz, OP_32BIT, 0, MEMPARAMS, imm); }
INLINE void emit_mov_m32_r32(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)					{ emit_op_modrm_mem(emitptr, OP_MOV_Ev_Gv, OP_32BIT, sreg, MEMPARAMS); }
INLINE void emit_movsx_r32_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Eb, OP_32BIT, dreg, sreg); }
INLINE void emit_movsx_r32_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Eb, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_movsx_r32_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Ew, OP_32BIT, dreg, sreg); }
INLINE void emit_movsx_r32_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)				{ emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Ew, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_movzx_r32_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg)
{
#ifndef PTR64
	if (sreg >= 4)
	{
		emit_op_modrm_reg(emitptr, OP_MOV_Gv_Ev, OP_32BIT, dreg, sreg);							// mov dreg,sreg
 		emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 4, dreg, 0xff); 	// and dreg,0xff
	}
	else
#endif
	emit_op_modrm_reg(emitptr, OP_MOVZX_Gv_Eb, OP_32BIT, dreg, sreg);
}
INLINE void emit_movzx_r32_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Eb, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_movzx_r32_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_MOVZX_Gv_Ew, OP_32BIT, dreg, sreg); }
INLINE void emit_movzx_r32_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)				{ emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Ew, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_cmovcc_r32_r32(x86code **emitptr, UINT8 cond, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_32BIT, dreg, sreg); }
INLINE void emit_cmovcc_r32_m32(x86code **emitptr, UINT8 cond, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_32BIT, dreg, MEMPARAMS); }


/*-------------------------------------------------
    emit_xchg_r32_*
-------------------------------------------------*/

INLINE void emit_xchg_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg)
{
	if (dreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (sreg & 7), OP_32BIT, sreg);
	else if (sreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (dreg & 7), OP_32BIT, dreg);
	else
 		emit_op_modrm_reg(emitptr, OP_XCHG_Ev_Gv, OP_32BIT, dreg, sreg);
}


/*-------------------------------------------------
    emit_mov_r64_*
-------------------------------------------------*/

#ifdef PTR64

INLINE void emit_mov_r64_imm(x86code **emitptr, UINT8 dreg, UINT64 imm)
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

INLINE void emit_mov_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg)							{ emit_op_modrm_reg(emitptr, OP_MOV_Gv_Ev, OP_64BIT, dreg, sreg); }
INLINE void emit_mov_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_MOV_Gv_Ev, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_mov_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)					{ emit_op_modrm_mem_imm32(emitptr, OP_G11_Ev_Iz, OP_64BIT, 0, MEMPARAMS, imm); }
INLINE void emit_mov_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)					{ emit_op_modrm_mem(emitptr, OP_MOV_Ev_Gv, OP_64BIT, sreg, MEMPARAMS); }
INLINE void emit_movsx_r64_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Eb, OP_64BIT, dreg, sreg); }
INLINE void emit_movsx_r64_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Eb, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_movsx_r64_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_MOVSX_Gv_Ew, OP_64BIT, dreg, sreg); }
INLINE void emit_movsx_r64_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)				{ emit_op_modrm_mem(emitptr, OP_MOVSX_Gv_Ew, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_movsxd_r64_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_MOVSXD_Gv_Ev, OP_64BIT, dreg, sreg); }
INLINE void emit_movsxd_r64_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)				{ emit_op_modrm_mem(emitptr, OP_MOVSXD_Gv_Ev, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_movzx_r64_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_MOVZX_Gv_Eb, OP_64BIT, dreg, sreg); }
INLINE void emit_movzx_r64_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Eb, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_movzx_r64_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_MOVZX_Gv_Ew, OP_64BIT, dreg, sreg); }
INLINE void emit_movzx_r64_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)				{ emit_op_modrm_mem(emitptr, OP_MOVZX_Gv_Ew, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_cmovcc_r64_r64(x86code **emitptr, UINT8 cond, UINT8 dreg, UINT8 sreg) 			{ emit_op_modrm_reg(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_64BIT, dreg, sreg); }
INLINE void emit_cmovcc_r64_m64(x86code **emitptr, UINT8 cond, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CMOV_O_Gv_Ev + cond, OP_64BIT, dreg, MEMPARAMS); }


/*-------------------------------------------------
    emit_xchg_r64_*
-------------------------------------------------*/

INLINE void emit_xchg_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg)
{
	if (dreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (sreg & 7), OP_64BIT, sreg);
	else if (sreg == REG_EAX)
		emit_op_reg(emitptr, OP_NOP | (dreg & 7), OP_64BIT, dreg);
	else
 		emit_op_modrm_reg(emitptr, OP_XCHG_Ev_Gv, OP_64BIT, dreg, sreg);
}

#endif


/***************************************************************************
    ARITHMETIC EMITTERS
***************************************************************************/

/*-------------------------------------------------
    emit_arith_r8_*
-------------------------------------------------*/

INLINE void emit_add_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)			{ emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 0, dreg, imm); }
INLINE void emit_add_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)	{ emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 0, MEMPARAMS, imm); }
INLINE void emit_add_r8_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg) 			{ emit_op_modrm_reg(emitptr, OP_ADD_Gb_Eb, OP_32BIT, dreg, sreg); }
INLINE void emit_add_r8_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_ADD_Gb_Eb, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_add_m8_r8(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_ADD_Eb_Gb, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_or_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)			{ emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 1, dreg, imm); }
INLINE void emit_or_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)		{ emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 1, MEMPARAMS, imm); }
INLINE void emit_or_r8_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg) 			{ emit_op_modrm_reg(emitptr, OP_OR_Gb_Eb, OP_32BIT, dreg, sreg); }
INLINE void emit_or_r8_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_OR_Gb_Eb, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_or_m8_r8(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_OR_Eb_Gb, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_adc_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)			{ emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 2, dreg, imm); }
INLINE void emit_adc_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)	{ emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 2, MEMPARAMS, imm); }
INLINE void emit_adc_r8_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg) 			{ emit_op_modrm_reg(emitptr, OP_ADC_Gb_Eb, OP_32BIT, dreg, sreg); }
INLINE void emit_adc_r8_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_ADC_Gb_Eb, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_adc_m8_r8(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_ADC_Eb_Gb, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_sbb_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)			{ emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 3, dreg, imm); }
INLINE void emit_sbb_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)	{ emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 3, MEMPARAMS, imm); }
INLINE void emit_sbb_r8_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg) 			{ emit_op_modrm_reg(emitptr, OP_SBB_Gb_Eb, OP_32BIT, dreg, sreg); }
INLINE void emit_sbb_r8_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_SBB_Gb_Eb, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_sbb_m8_r8(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_SBB_Eb_Gb, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_and_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)			{ emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 4, dreg, imm); }
INLINE void emit_and_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)	{ emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 4, MEMPARAMS, imm); }
INLINE void emit_and_r8_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg) 			{ emit_op_modrm_reg(emitptr, OP_AND_Gb_Eb, OP_32BIT, dreg, sreg); }
INLINE void emit_and_r8_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_AND_Gb_Eb, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_and_m8_r8(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_AND_Eb_Gb, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_sub_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)			{ emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 5, dreg, imm); }
INLINE void emit_sub_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)	{ emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 5, MEMPARAMS, imm); }
INLINE void emit_sub_r8_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg) 			{ emit_op_modrm_reg(emitptr, OP_SUB_Gb_Eb, OP_32BIT, dreg, sreg); }
INLINE void emit_sub_r8_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_SUB_Gb_Eb, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_sub_m8_r8(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_SUB_Eb_Gb, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_xor_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)			{ emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 6, dreg, imm); }
INLINE void emit_xor_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)	{ emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 6, MEMPARAMS, imm); }
INLINE void emit_xor_r8_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg) 			{ emit_op_modrm_reg(emitptr, OP_XOR_Gb_Eb, OP_32BIT, dreg, sreg); }
INLINE void emit_xor_r8_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_XOR_Gb_Eb, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_xor_m8_r8(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_XOR_Eb_Gb, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_cmp_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)			{ emit_op_modrm_reg_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 7, dreg, imm); }
INLINE void emit_cmp_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)	{ emit_op_modrm_mem_imm8(emitptr, OP_G1_Eb_Ib, OP_32BIT, 7, MEMPARAMS, imm); }
INLINE void emit_cmp_r8_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg) 			{ emit_op_modrm_reg(emitptr, OP_CMP_Gb_Eb, OP_32BIT, dreg, sreg); }
INLINE void emit_cmp_r8_m8(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_CMP_Gb_Eb, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_cmp_m8_r8(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_CMP_Eb_Gb, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_test_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)			{ emit_op_modrm_reg_imm8(emitptr, OP_G3_Eb, OP_32BIT, 0, dreg, imm); }
INLINE void emit_test_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)	{ emit_op_modrm_mem_imm8(emitptr, OP_G3_Eb, OP_32BIT, 0, MEMPARAMS, imm); }
INLINE void emit_test_r8_r8(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_TEST_Eb_Gb, OP_32BIT, sreg, dreg); }
INLINE void emit_test_m8_r8(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)	{ emit_op_modrm_mem(emitptr, OP_TEST_Eb_Gb, OP_32BIT, sreg, MEMPARAMS); }


/*-------------------------------------------------
    emit_arith_r16_*
-------------------------------------------------*/

INLINE void emit_add_r16_imm(x86code **emitptr, UINT8 dreg, UINT16 imm)			{ emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 0, dreg, imm); }
INLINE void emit_add_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT16 imm)	{ emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 0, MEMPARAMS, imm); }
INLINE void emit_add_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_ADD_Gv_Ev, OP_16BIT, dreg, sreg); }
INLINE void emit_add_r16_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_ADD_Gv_Ev, OP_16BIT, dreg, MEMPARAMS); }
INLINE void emit_add_m16_r16(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_ADD_Ev_Gv, OP_16BIT, sreg, MEMPARAMS); }

INLINE void emit_or_r16_imm(x86code **emitptr, UINT8 dreg, UINT16 imm)			{ emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 1, dreg, imm); }
INLINE void emit_or_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT16 imm)	{ emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 1, MEMPARAMS, imm); }
INLINE void emit_or_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg) 			{ emit_op_modrm_reg(emitptr, OP_OR_Gv_Ev, OP_16BIT, dreg, sreg); }
INLINE void emit_or_r16_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_OR_Gv_Ev, OP_16BIT, dreg, MEMPARAMS); }
INLINE void emit_or_m16_r16(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_OR_Ev_Gv, OP_16BIT, sreg, MEMPARAMS); }

INLINE void emit_adc_r16_imm(x86code **emitptr, UINT8 dreg, UINT16 imm)			{ emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 2, dreg, imm); }
INLINE void emit_adc_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT16 imm)	{ emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 2, MEMPARAMS, imm); }
INLINE void emit_adc_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_ADC_Gv_Ev, OP_16BIT, dreg, sreg); }
INLINE void emit_adc_r16_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_ADC_Gv_Ev, OP_16BIT, dreg, MEMPARAMS); }
INLINE void emit_adc_m16_r16(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_ADC_Ev_Gv, OP_16BIT, sreg, MEMPARAMS); }

INLINE void emit_sbb_r16_imm(x86code **emitptr, UINT8 dreg, UINT16 imm)			{ emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 3, dreg, imm); }
INLINE void emit_sbb_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT16 imm)	{ emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 3, MEMPARAMS, imm); }
INLINE void emit_sbb_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_SBB_Gv_Ev, OP_16BIT, dreg, sreg); }
INLINE void emit_sbb_r16_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_SBB_Gv_Ev, OP_16BIT, dreg, MEMPARAMS); }
INLINE void emit_sbb_m16_r16(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_SBB_Ev_Gv, OP_16BIT, sreg, MEMPARAMS); }

INLINE void emit_and_r16_imm(x86code **emitptr, UINT8 dreg, UINT16 imm)			{ emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 4, dreg, imm); }
INLINE void emit_and_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT16 imm)	{ emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 4, MEMPARAMS, imm); }
INLINE void emit_and_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_AND_Gv_Ev, OP_16BIT, dreg, sreg); }
INLINE void emit_and_r16_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_AND_Gv_Ev, OP_16BIT, dreg, MEMPARAMS); }
INLINE void emit_and_m16_r16(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_AND_Ev_Gv, OP_16BIT, sreg, MEMPARAMS); }

INLINE void emit_sub_r16_imm(x86code **emitptr, UINT8 dreg, UINT16 imm)			{ emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 5, dreg, imm); }
INLINE void emit_sub_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT16 imm)	{ emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 5, MEMPARAMS, imm); }
INLINE void emit_sub_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_SUB_Gv_Ev, OP_16BIT, dreg, sreg); }
INLINE void emit_sub_r16_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_SUB_Gv_Ev, OP_16BIT, dreg, MEMPARAMS); }
INLINE void emit_sub_m16_r16(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_SUB_Ev_Gv, OP_16BIT, sreg, MEMPARAMS); }

INLINE void emit_xor_r16_imm(x86code **emitptr, UINT8 dreg, UINT16 imm)			{ emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 6, dreg, imm); }
INLINE void emit_xor_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT16 imm)	{ emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 6, MEMPARAMS, imm); }
INLINE void emit_xor_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_XOR_Gv_Ev, OP_16BIT, dreg, sreg); }
INLINE void emit_xor_r16_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_XOR_Gv_Ev, OP_16BIT, dreg, MEMPARAMS); }
INLINE void emit_xor_m16_r16(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_XOR_Ev_Gv, OP_16BIT, sreg, MEMPARAMS); }

INLINE void emit_cmp_r16_imm(x86code **emitptr, UINT8 dreg, UINT16 imm)			{ emit_op_modrm_reg_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 7, dreg, imm); }
INLINE void emit_cmp_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT16 imm)	{ emit_op_modrm_mem_imm816(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_16BIT, 7, MEMPARAMS, imm); }
INLINE void emit_cmp_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_CMP_Gv_Ev, OP_16BIT, dreg, sreg); }
INLINE void emit_cmp_r16_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_CMP_Gv_Ev, OP_16BIT, dreg, MEMPARAMS); }
INLINE void emit_cmp_m16_r16(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_CMP_Ev_Gv, OP_16BIT, sreg, MEMPARAMS); }

INLINE void emit_test_r16_imm(x86code **emitptr, UINT8 dreg, UINT16 imm)		{ emit_op_modrm_reg_imm16(emitptr, OP_G3_Ev, OP_16BIT, 0, dreg, imm); }
INLINE void emit_test_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT16 imm)	{ emit_op_modrm_mem_imm16(emitptr, OP_G3_Ev, OP_16BIT, 0, MEMPARAMS, imm); }
INLINE void emit_test_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg)		{ emit_op_modrm_reg(emitptr, OP_TEST_Ev_Gv, OP_16BIT, sreg, dreg); }
INLINE void emit_test_m16_r16(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)	{ emit_op_modrm_mem(emitptr, OP_TEST_Ev_Gv, OP_16BIT, sreg, MEMPARAMS); }


/*-------------------------------------------------
    emit_arith_r32_*
-------------------------------------------------*/

INLINE void emit_add_r32_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 0, dreg, imm); }
INLINE void emit_add_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 0, MEMPARAMS, imm); }
INLINE void emit_add_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_ADD_Gv_Ev, OP_32BIT, dreg, sreg); }
INLINE void emit_add_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_ADD_Gv_Ev, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_add_m32_r32(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_ADD_Ev_Gv, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_or_r32_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 1, dreg, imm); }
INLINE void emit_or_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 1, MEMPARAMS, imm); }
INLINE void emit_or_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg) 			{ emit_op_modrm_reg(emitptr, OP_OR_Gv_Ev, OP_32BIT, dreg, sreg); }
INLINE void emit_or_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_OR_Gv_Ev, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_or_m32_r32(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_OR_Ev_Gv, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_adc_r32_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 2, dreg, imm); }
INLINE void emit_adc_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 2, MEMPARAMS, imm); }
INLINE void emit_adc_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_ADC_Gv_Ev, OP_32BIT, dreg, sreg); }
INLINE void emit_adc_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_ADC_Gv_Ev, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_adc_m32_r32(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_ADC_Ev_Gv, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_sbb_r32_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 3, dreg, imm); }
INLINE void emit_sbb_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 3, MEMPARAMS, imm); }
INLINE void emit_sbb_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_SBB_Gv_Ev, OP_32BIT, dreg, sreg); }
INLINE void emit_sbb_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_SBB_Gv_Ev, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_sbb_m32_r32(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_SBB_Ev_Gv, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_and_r32_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 4, dreg, imm); }
INLINE void emit_and_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 4, MEMPARAMS, imm); }
INLINE void emit_and_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_AND_Gv_Ev, OP_32BIT, dreg, sreg); }
INLINE void emit_and_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_AND_Gv_Ev, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_and_m32_r32(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_AND_Ev_Gv, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_sub_r32_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 5, dreg, imm); }
INLINE void emit_sub_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 5, MEMPARAMS, imm); }
INLINE void emit_sub_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_SUB_Gv_Ev, OP_32BIT, dreg, sreg); }
INLINE void emit_sub_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_SUB_Gv_Ev, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_sub_m32_r32(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_SUB_Ev_Gv, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_xor_r32_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 6, dreg, imm); }
INLINE void emit_xor_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 6, MEMPARAMS, imm); }
INLINE void emit_xor_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_XOR_Gv_Ev, OP_32BIT, dreg, sreg); }
INLINE void emit_xor_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_XOR_Gv_Ev, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_xor_m32_r32(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_XOR_Ev_Gv, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_cmp_r32_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 7, dreg, imm); }
INLINE void emit_cmp_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_32BIT, 7, MEMPARAMS, imm); }
INLINE void emit_cmp_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_CMP_Gv_Ev, OP_32BIT, dreg, sreg); }
INLINE void emit_cmp_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_CMP_Gv_Ev, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_cmp_m32_r32(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_CMP_Ev_Gv, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_test_r32_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)		{ emit_op_modrm_reg_imm32(emitptr, OP_G3_Ev, OP_32BIT, 0, dreg, imm); }
INLINE void emit_test_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm32(emitptr, OP_G3_Ev, OP_32BIT, 0, MEMPARAMS, imm); }
INLINE void emit_test_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg)		{ emit_op_modrm_reg(emitptr, OP_TEST_Ev_Gv, OP_32BIT, sreg, dreg); }
INLINE void emit_test_m32_r32(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)	{ emit_op_modrm_mem(emitptr, OP_TEST_Ev_Gv, OP_32BIT, sreg, MEMPARAMS); }


/*-------------------------------------------------
    emit_arith_r64_*
-------------------------------------------------*/

#ifdef PTR64

INLINE void emit_add_r64_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 0, dreg, imm); }
INLINE void emit_add_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 0, MEMPARAMS, imm); }
INLINE void emit_add_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_ADD_Gv_Ev, OP_64BIT, dreg, sreg); }
INLINE void emit_add_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_ADD_Gv_Ev, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_add_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_ADD_Ev_Gv, OP_64BIT, sreg, MEMPARAMS); }

INLINE void emit_or_r64_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 1, dreg, imm); }
INLINE void emit_or_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 1, MEMPARAMS, imm); }
INLINE void emit_or_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg) 			{ emit_op_modrm_reg(emitptr, OP_OR_Gv_Ev, OP_64BIT, dreg, sreg); }
INLINE void emit_or_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_OR_Gv_Ev, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_or_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_OR_Ev_Gv, OP_64BIT, sreg, MEMPARAMS); }

INLINE void emit_adc_r64_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 2, dreg, imm); }
INLINE void emit_adc_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 2, MEMPARAMS, imm); }
INLINE void emit_adc_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_ADC_Gv_Ev, OP_64BIT, dreg, sreg); }
INLINE void emit_adc_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_ADC_Gv_Ev, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_adc_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_ADC_Ev_Gv, OP_64BIT, sreg, MEMPARAMS); }

INLINE void emit_sbb_r64_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 3, dreg, imm); }
INLINE void emit_sbb_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 3, MEMPARAMS, imm); }
INLINE void emit_sbb_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_SBB_Gv_Ev, OP_64BIT, dreg, sreg); }
INLINE void emit_sbb_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_SBB_Gv_Ev, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_sbb_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_SBB_Ev_Gv, OP_64BIT, sreg, MEMPARAMS); }

INLINE void emit_and_r64_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 4, dreg, imm); }
INLINE void emit_and_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 4, MEMPARAMS, imm); }
INLINE void emit_and_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_AND_Gv_Ev, OP_64BIT, dreg, sreg); }
INLINE void emit_and_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_AND_Gv_Ev, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_and_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_AND_Ev_Gv, OP_64BIT, sreg, MEMPARAMS); }

INLINE void emit_sub_r64_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 5, dreg, imm); }
INLINE void emit_sub_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 5, MEMPARAMS, imm); }
INLINE void emit_sub_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_SUB_Gv_Ev, OP_64BIT, dreg, sreg); }
INLINE void emit_sub_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_SUB_Gv_Ev, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_sub_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_SUB_Ev_Gv, OP_64BIT, sreg, MEMPARAMS); }

INLINE void emit_xor_r64_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 6, dreg, imm); }
INLINE void emit_xor_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 6, MEMPARAMS, imm); }
INLINE void emit_xor_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_XOR_Gv_Ev, OP_64BIT, dreg, sreg); }
INLINE void emit_xor_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_XOR_Gv_Ev, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_xor_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_XOR_Ev_Gv, OP_64BIT, sreg, MEMPARAMS); }

INLINE void emit_cmp_r64_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)			{ emit_op_modrm_reg_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 7, dreg, imm); }
INLINE void emit_cmp_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_G1_Ev_Ib, OP_G1_Ev_Iz, OP_64BIT, 7, MEMPARAMS, imm); }
INLINE void emit_cmp_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_CMP_Gv_Ev, OP_64BIT, dreg, sreg); }
INLINE void emit_cmp_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_CMP_Gv_Ev, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_cmp_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) 	{ emit_op_modrm_mem(emitptr, OP_CMP_Ev_Gv, OP_64BIT, sreg, MEMPARAMS); }

INLINE void emit_test_r64_imm(x86code **emitptr, UINT8 dreg, UINT32 imm)		{ emit_op_modrm_reg_imm32(emitptr, OP_G3_Ev, OP_64BIT, 0, dreg, imm); }
INLINE void emit_test_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT32 imm)	{ emit_op_modrm_mem_imm32(emitptr, OP_G3_Ev, OP_64BIT, 0, MEMPARAMS, imm); }
INLINE void emit_test_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg)		{ emit_op_modrm_reg(emitptr, OP_TEST_Ev_Gv, OP_64BIT, sreg, dreg); }
INLINE void emit_test_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)	{ emit_op_modrm_mem(emitptr, OP_TEST_Ev_Gv, OP_64BIT, sreg, MEMPARAMS); }

#endif



/***************************************************************************
    SHIFT EMITTERS
***************************************************************************/

/*-------------------------------------------------
    emit_shift_reg_imm
-------------------------------------------------*/

INLINE void emit_shift_reg_imm(x86code **emitptr, UINT32 op1, UINT32 opn, UINT8 opsize, UINT8 opindex, UINT8 dreg, UINT8 imm)
{
	if (imm == 1)
		emit_op_modrm_reg(emitptr, op1, opsize, opindex, dreg);
	else
		emit_op_modrm_reg_imm8(emitptr, opn, opsize, opindex, dreg, imm);
}

INLINE void emit_shift_mem_imm(x86code **emitptr, UINT32 op1, UINT32 opn, UINT8 opsize, UINT8 opindex, DECLARE_MEMPARAMS, UINT8 imm)
{
	if (imm == 1)
		emit_op_modrm_mem(emitptr, op1, opsize, opindex, MEMPARAMS);
	else
		emit_op_modrm_mem_imm8(emitptr, opn, opsize, opindex, MEMPARAMS, imm);
}


/*-------------------------------------------------
    emit_shift_r8_*
-------------------------------------------------*/

INLINE void emit_rol_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 0, dreg, imm); }
INLINE void emit_rol_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 0, MEMPARAMS, imm); }
INLINE void emit_rol_r8_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 0, dreg); }
INLINE void emit_rol_m8_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 0, MEMPARAMS); }

INLINE void emit_ror_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 1, dreg, imm); }
INLINE void emit_ror_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 1, MEMPARAMS, imm); }
INLINE void emit_ror_r8_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 1, dreg); }
INLINE void emit_ror_m8_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 1, MEMPARAMS); }

INLINE void emit_rcl_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 2, dreg, imm); }
INLINE void emit_rcl_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 2, MEMPARAMS, imm); }
INLINE void emit_rcl_r8_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 2, dreg); }
INLINE void emit_rcl_m8_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 2, MEMPARAMS); }

INLINE void emit_rcr_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 3, dreg, imm); }
INLINE void emit_rcr_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 3, MEMPARAMS, imm); }
INLINE void emit_rcr_r8_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 3, dreg); }
INLINE void emit_rcr_m8_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 3, MEMPARAMS); }

INLINE void emit_shl_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 4, dreg, imm); }
INLINE void emit_shl_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 4, MEMPARAMS, imm); }
INLINE void emit_shl_r8_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 4, dreg); }
INLINE void emit_shl_m8_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 4, MEMPARAMS); }

INLINE void emit_shr_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 5, dreg, imm); }
INLINE void emit_shr_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 5, MEMPARAMS, imm); }
INLINE void emit_shr_r8_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 5, dreg); }
INLINE void emit_shr_m8_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 5, MEMPARAMS); }

INLINE void emit_sar_r8_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 7, dreg, imm); }
INLINE void emit_sar_m8_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Eb_1, OP_G2_Eb_Ib, OP_32BIT, 7, MEMPARAMS, imm); }
INLINE void emit_sar_r8_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Eb_CL, OP_32BIT, 7, dreg); }
INLINE void emit_sar_m8_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Eb_CL, OP_32BIT, 7, MEMPARAMS); }


/*-------------------------------------------------
    emit_shift_r16_*
-------------------------------------------------*/

INLINE void emit_rol_r16_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 0, dreg, imm); }
INLINE void emit_rol_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 0, MEMPARAMS, imm); }
INLINE void emit_rol_r16_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 0, dreg); }
INLINE void emit_rol_m16_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 0, MEMPARAMS); }

INLINE void emit_ror_r16_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 1, dreg, imm); }
INLINE void emit_ror_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 1, MEMPARAMS, imm); }
INLINE void emit_ror_r16_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 1, dreg); }
INLINE void emit_ror_m16_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 1, MEMPARAMS); }

INLINE void emit_rcl_r16_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 2, dreg, imm); }
INLINE void emit_rcl_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 2, MEMPARAMS, imm); }
INLINE void emit_rcl_r16_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 2, dreg); }
INLINE void emit_rcl_m16_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 2, MEMPARAMS); }

INLINE void emit_rcr_r16_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 3, dreg, imm); }
INLINE void emit_rcr_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 3, MEMPARAMS, imm); }
INLINE void emit_rcr_r16_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 3, dreg); }
INLINE void emit_rcr_m16_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 3, MEMPARAMS); }

INLINE void emit_shl_r16_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 4, dreg, imm); }
INLINE void emit_shl_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 4, MEMPARAMS, imm); }
INLINE void emit_shl_r16_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 4, dreg); }
INLINE void emit_shl_m16_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 4, MEMPARAMS); }

INLINE void emit_shr_r16_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 5, dreg, imm); }
INLINE void emit_shr_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 5, MEMPARAMS, imm); }
INLINE void emit_shr_r16_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 5, dreg); }
INLINE void emit_shr_m16_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 5, MEMPARAMS); }

INLINE void emit_sar_r16_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 7, dreg, imm); }
INLINE void emit_sar_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_16BIT, 7, MEMPARAMS, imm); }
INLINE void emit_sar_r16_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_16BIT, 7, dreg); }
INLINE void emit_sar_m16_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_16BIT, 7, MEMPARAMS); }


/*-------------------------------------------------
    emit_shift_r32_*
-------------------------------------------------*/

INLINE void emit_rol_r32_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 0, dreg, imm); }
INLINE void emit_rol_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 0, MEMPARAMS, imm); }
INLINE void emit_rol_r32_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 0, dreg); }
INLINE void emit_rol_m32_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 0, MEMPARAMS); }

INLINE void emit_ror_r32_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 1, dreg, imm); }
INLINE void emit_ror_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 1, MEMPARAMS, imm); }
INLINE void emit_ror_r32_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 1, dreg); }
INLINE void emit_ror_m32_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 1, MEMPARAMS); }

INLINE void emit_rcl_r32_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 2, dreg, imm); }
INLINE void emit_rcl_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 2, MEMPARAMS, imm); }
INLINE void emit_rcl_r32_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 2, dreg); }
INLINE void emit_rcl_m32_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 2, MEMPARAMS); }

INLINE void emit_rcr_r32_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 3, dreg, imm); }
INLINE void emit_rcr_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 3, MEMPARAMS, imm); }
INLINE void emit_rcr_r32_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 3, dreg); }
INLINE void emit_rcr_m32_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 3, MEMPARAMS); }

INLINE void emit_shl_r32_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 4, dreg, imm); }
INLINE void emit_shl_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 4, MEMPARAMS, imm); }
INLINE void emit_shl_r32_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 4, dreg); }
INLINE void emit_shl_m32_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 4, MEMPARAMS); }

INLINE void emit_shr_r32_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 5, dreg, imm); }
INLINE void emit_shr_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 5, MEMPARAMS, imm); }
INLINE void emit_shr_r32_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 5, dreg); }
INLINE void emit_shr_m32_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 5, MEMPARAMS); }

INLINE void emit_sar_r32_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 7, dreg, imm); }
INLINE void emit_sar_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_32BIT, 7, MEMPARAMS, imm); }
INLINE void emit_sar_r32_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_32BIT, 7, dreg); }
INLINE void emit_sar_m32_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_32BIT, 7, MEMPARAMS); }

INLINE void emit_shld_r32_r32_imm(x86code **emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm) 		{ emit_op_modrm_reg_imm8(emitptr, OP_SHLD_Ev_Gv_Ib, OP_32BIT, sreg, dreg, imm); }
INLINE void emit_shld_m32_r32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg, UINT8 imm)	{ emit_op_modrm_mem_imm8(emitptr, OP_SHLD_Ev_Gv_Ib, OP_32BIT, sreg, MEMPARAMS, imm); }
INLINE void emit_shld_r32_r32_cl(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_SHLD_Ev_Gv_CL, OP_32BIT, sreg, dreg); }
INLINE void emit_shld_m32_r32_cl(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)				{ emit_op_modrm_mem(emitptr, OP_SHLD_Ev_Gv_CL, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_shrd_r32_r32_imm(x86code **emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)			{ emit_op_modrm_reg_imm8(emitptr, OP_SHRD_Ev_Gv_Ib, OP_32BIT, sreg, dreg, imm); }
INLINE void emit_shrd_m32_r32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg, UINT8 imm)	{ emit_op_modrm_mem_imm8(emitptr, OP_SHRD_Ev_Gv_Ib, OP_32BIT, sreg, MEMPARAMS, imm); }
INLINE void emit_shrd_r32_r32_cl(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_SHRD_Ev_Gv_CL, OP_32BIT, sreg, dreg); }
INLINE void emit_shrd_m32_r32_cl(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)				{ emit_op_modrm_mem(emitptr, OP_SHRD_Ev_Gv_CL, OP_32BIT, sreg, MEMPARAMS); }


/*-------------------------------------------------
    emit_shift_r64_*
-------------------------------------------------*/

#ifdef PTR64

INLINE void emit_rol_r64_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 0, dreg, imm); }
INLINE void emit_rol_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 0, MEMPARAMS, imm); }
INLINE void emit_rol_r64_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 0, dreg); }
INLINE void emit_rol_m64_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 0, MEMPARAMS); }

INLINE void emit_ror_r64_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 1, dreg, imm); }
INLINE void emit_ror_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 1, MEMPARAMS, imm); }
INLINE void emit_ror_r64_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 1, dreg); }
INLINE void emit_ror_m64_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 1, MEMPARAMS); }

INLINE void emit_rcl_r64_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 2, dreg, imm); }
INLINE void emit_rcl_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 2, MEMPARAMS, imm); }
INLINE void emit_rcl_r64_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 2, dreg); }
INLINE void emit_rcl_m64_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 2, MEMPARAMS); }

INLINE void emit_rcr_r64_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 3, dreg, imm); }
INLINE void emit_rcr_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 3, MEMPARAMS, imm); }
INLINE void emit_rcr_r64_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 3, dreg); }
INLINE void emit_rcr_m64_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 3, MEMPARAMS); }

INLINE void emit_shl_r64_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 4, dreg, imm); }
INLINE void emit_shl_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 4, MEMPARAMS, imm); }
INLINE void emit_shl_r64_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 4, dreg); }
INLINE void emit_shl_m64_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 4, MEMPARAMS); }

INLINE void emit_shr_r64_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 5, dreg, imm); }
INLINE void emit_shr_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 5, MEMPARAMS, imm); }
INLINE void emit_shr_r64_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 5, dreg); }
INLINE void emit_shr_m64_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 5, MEMPARAMS); }

INLINE void emit_sar_r64_imm(x86code **emitptr, UINT8 dreg, UINT8 imm) 			{ emit_shift_reg_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 7, dreg, imm); }
INLINE void emit_sar_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm) 	{ emit_shift_mem_imm(emitptr, OP_G2_Ev_1, OP_G2_Ev_Ib, OP_64BIT, 7, MEMPARAMS, imm); }
INLINE void emit_sar_r64_cl(x86code **emitptr, UINT8 dreg)			 			{ emit_op_modrm_reg(emitptr, OP_G2_Ev_CL, OP_64BIT, 7, dreg); }
INLINE void emit_sar_m64_cl(x86code **emitptr, DECLARE_MEMPARAMS)			 	{ emit_op_modrm_mem(emitptr, OP_G2_Ev_CL, OP_64BIT, 7, MEMPARAMS); }

INLINE void emit_shld_r64_r64_imm(x86code **emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)			{ emit_op_modrm_reg_imm8(emitptr, OP_SHLD_Ev_Gv_Ib, OP_64BIT, sreg, dreg, imm); }
INLINE void emit_shld_m64_r64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg, UINT8 imm)	{ emit_op_modrm_mem_imm8(emitptr, OP_SHLD_Ev_Gv_Ib, OP_64BIT, sreg, MEMPARAMS, imm); }
INLINE void emit_shld_r64_r64_cl(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_SHLD_Ev_Gv_CL, OP_64BIT, sreg, dreg); }
INLINE void emit_shld_m64_r64_cl(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)				{ emit_op_modrm_mem(emitptr, OP_SHLD_Ev_Gv_CL, OP_64BIT, sreg, MEMPARAMS); }

INLINE void emit_shrd_r64_r64_imm(x86code **emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)			{ emit_op_modrm_reg_imm8(emitptr, OP_SHRD_Ev_Gv_Ib, OP_64BIT, sreg, dreg, imm); }
INLINE void emit_shrd_m64_r64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg, UINT8 imm)	{ emit_op_modrm_mem_imm8(emitptr, OP_SHRD_Ev_Gv_Ib, OP_64BIT, sreg, MEMPARAMS, imm); }
INLINE void emit_shrd_r64_r64_cl(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_SHRD_Ev_Gv_CL, OP_64BIT, sreg, dreg); }
INLINE void emit_shrd_m64_r64_cl(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)				{ emit_op_modrm_mem(emitptr, OP_SHRD_Ev_Gv_CL, OP_64BIT, sreg, MEMPARAMS); }

#endif



/***************************************************************************
    GROUP3 EMITTERS
***************************************************************************/

/*-------------------------------------------------
    emit_group3_r8_*
-------------------------------------------------*/

INLINE void emit_not_r8(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 2, dreg); }
INLINE void emit_not_m8(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 2, MEMPARAMS); }

INLINE void emit_neg_r8(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 3, dreg); }
INLINE void emit_neg_m8(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 3, MEMPARAMS); }

INLINE void emit_mul_r8(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 4, dreg); }
INLINE void emit_mul_m8(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 4, MEMPARAMS); }

INLINE void emit_imul_r8(x86code **emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 5, dreg); }
INLINE void emit_imul_m8(x86code **emitptr, DECLARE_MEMPARAMS) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 5, MEMPARAMS); }

INLINE void emit_div_r8(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 6, dreg); }
INLINE void emit_div_m8(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 6, MEMPARAMS); }

INLINE void emit_idiv_r8(x86code **emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 7, dreg); }
INLINE void emit_idiv_m8(x86code **emitptr, DECLARE_MEMPARAMS) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 7, MEMPARAMS); }


/*-------------------------------------------------
    emit_group3_r16_*
-------------------------------------------------*/

INLINE void emit_not_r16(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 2, dreg); }
INLINE void emit_not_m16(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 2, MEMPARAMS); }

INLINE void emit_neg_r16(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 3, dreg); }
INLINE void emit_neg_m16(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 3, MEMPARAMS); }

INLINE void emit_mul_r16(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 4, dreg); }
INLINE void emit_mul_m16(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 4, MEMPARAMS); }

INLINE void emit_imul_r16(x86code **emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 5, dreg); }
INLINE void emit_imul_m16(x86code **emitptr, DECLARE_MEMPARAMS) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 5, MEMPARAMS); }

INLINE void emit_div_r16(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 6, dreg); }
INLINE void emit_div_m16(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 6, MEMPARAMS); }

INLINE void emit_idiv_r16(x86code **emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_16BIT, 7, dreg); }
INLINE void emit_idiv_m16(x86code **emitptr, DECLARE_MEMPARAMS) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_16BIT, 7, MEMPARAMS); }


/*-------------------------------------------------
    emit_group3_r32_*
-------------------------------------------------*/

INLINE void emit_not_r32(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 2, dreg); }
INLINE void emit_not_m32(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 2, MEMPARAMS); }

INLINE void emit_neg_r32(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 3, dreg); }
INLINE void emit_neg_m32(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 3, MEMPARAMS); }

INLINE void emit_mul_r32(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 4, dreg); }
INLINE void emit_mul_m32(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 4, MEMPARAMS); }

INLINE void emit_imul_r32(x86code **emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 5, dreg); }
INLINE void emit_imul_m32(x86code **emitptr, DECLARE_MEMPARAMS) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 5, MEMPARAMS); }

INLINE void emit_div_r32(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 6, dreg); }
INLINE void emit_div_m32(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 6, MEMPARAMS); }

INLINE void emit_idiv_r32(x86code **emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_32BIT, 7, dreg); }
INLINE void emit_idiv_m32(x86code **emitptr, DECLARE_MEMPARAMS) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_32BIT, 7, MEMPARAMS); }


/*-------------------------------------------------
    emit_group3_r64_*
-------------------------------------------------*/

#ifdef PTR64

INLINE void emit_not_r64(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 2, dreg); }
INLINE void emit_not_m64(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 2, MEMPARAMS); }

INLINE void emit_neg_r64(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 3, dreg); }
INLINE void emit_neg_m64(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 3, MEMPARAMS); }

INLINE void emit_mul_r64(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 4, dreg); }
INLINE void emit_mul_m64(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 4, MEMPARAMS); }

INLINE void emit_imul_r64(x86code **emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 5, dreg); }
INLINE void emit_imul_m64(x86code **emitptr, DECLARE_MEMPARAMS) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 5, MEMPARAMS); }

INLINE void emit_div_r64(x86code **emitptr, UINT8 dreg)         { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 6, dreg); }
INLINE void emit_div_m64(x86code **emitptr, DECLARE_MEMPARAMS)  { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 6, MEMPARAMS); }

INLINE void emit_idiv_r64(x86code **emitptr, UINT8 dreg)        { emit_op_modrm_reg(emitptr, OP_G3_Ev, OP_64BIT, 7, dreg); }
INLINE void emit_idiv_m64(x86code **emitptr, DECLARE_MEMPARAMS) { emit_op_modrm_mem(emitptr, OP_G3_Ev, OP_64BIT, 7, MEMPARAMS); }

#endif



/***************************************************************************
    IMUL EMITTERS
***************************************************************************/

/*-------------------------------------------------
    emit_imul_r16_*
-------------------------------------------------*/

INLINE void emit_imul_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_IMUL_Gv_Ev, OP_16BIT, dreg, sreg); }
INLINE void emit_imul_r16_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_IMUL_Gv_Ev, OP_16BIT, dreg, MEMPARAMS); }
INLINE void emit_imul_r16_r16_imm(x86code **emitptr, UINT8 dreg, UINT8 sreg, INT16 imm) 		{ emit_op_modrm_reg_imm816(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_16BIT, dreg, sreg, imm); }
INLINE void emit_imul_r16_m16_imm(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS, INT16 imm)	{ emit_op_modrm_mem_imm816(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_16BIT, dreg, MEMPARAMS, imm); }

INLINE void emit_imul_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_IMUL_Gv_Ev, OP_32BIT, dreg, sreg); }
INLINE void emit_imul_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)					{ emit_op_modrm_mem(emitptr, OP_IMUL_Gv_Ev, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_imul_r32_r32_imm(x86code **emitptr, UINT8 dreg, UINT8 sreg, INT32 imm) 		{ emit_op_modrm_reg_imm832(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_32BIT, dreg, sreg, imm); }
INLINE void emit_imul_r32_m32_imm(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS, INT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_32BIT, dreg, MEMPARAMS, imm); }

#ifdef PTR64

INLINE void emit_imul_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg)						{ emit_op_modrm_reg(emitptr, OP_IMUL_Gv_Ev, OP_64BIT, dreg, sreg); }
INLINE void emit_imul_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 				{ emit_op_modrm_mem(emitptr, OP_IMUL_Gv_Ev, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_imul_r64_r64_imm(x86code **emitptr, UINT8 dreg, UINT8 sreg, INT32 imm) 		{ emit_op_modrm_reg_imm832(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_64BIT, dreg, sreg, imm); }
INLINE void emit_imul_r64_m64_imm(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS, INT32 imm)	{ emit_op_modrm_mem_imm832(emitptr, OP_IMUL_Gv_Ev_Ib, OP_IMUL_Gv_Ev_Iz, OP_64BIT, dreg, MEMPARAMS, imm); }

#endif



/***************************************************************************
    BIT OPERATION EMITTERS
***************************************************************************/

/*-------------------------------------------------
    emit_bswap_*
-------------------------------------------------*/

INLINE void emit_bswap_r32(x86code **emitptr, UINT8 dreg) 						{ emit_op_reg(emitptr, OP_BSWAP_EAX + (dreg & 7), OP_32BIT, dreg); }
INLINE void emit_bswap_r64(x86code **emitptr, UINT8 dreg) 						{ emit_op_reg(emitptr, OP_BSWAP_EAX + (dreg & 7), OP_64BIT, dreg); }


/*-------------------------------------------------
    emit_bsr/bsf_r16_*
-------------------------------------------------*/

INLINE void emit_bsf_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_BSF_Gv_Ev, OP_16BIT, dreg, sreg); }
INLINE void emit_bsf_r16_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_BSF_Gv_Ev, OP_16BIT, dreg, MEMPARAMS); }
INLINE void emit_bsr_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg) 		{ emit_op_modrm_reg(emitptr, OP_BSR_Gv_Ev, OP_16BIT, dreg, sreg); }
INLINE void emit_bsr_r16_m16(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_BSR_Gv_Ev, OP_16BIT, dreg, MEMPARAMS); }

INLINE void emit_bsf_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_BSF_Gv_Ev, OP_32BIT, dreg, sreg); }
INLINE void emit_bsf_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_BSF_Gv_Ev, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_bsr_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_BSR_Gv_Ev, OP_32BIT, dreg, sreg); }
INLINE void emit_bsr_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_BSR_Gv_Ev, OP_32BIT, dreg, MEMPARAMS); }

#ifdef PTR64
INLINE void emit_bsf_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_BSF_Gv_Ev, OP_64BIT, dreg, sreg); }
INLINE void emit_bsf_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_BSF_Gv_Ev, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_bsr_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_BSR_Gv_Ev, OP_64BIT, dreg, sreg); }
INLINE void emit_bsr_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_BSR_Gv_Ev, OP_64BIT, dreg, MEMPARAMS); }
#endif


/*-------------------------------------------------
    emit_bit_r16_*
-------------------------------------------------*/

INLINE void emit_bt_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_BT_Ev_Gv, OP_16BIT, sreg, dreg); }
INLINE void emit_bt_m16_r16(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_BT_Ev_Gv, OP_16BIT, sreg, MEMPARAMS); }
INLINE void emit_bt_r16_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)          { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 4, dreg, imm); }
INLINE void emit_bt_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)   { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 4, MEMPARAMS, imm); }

INLINE void emit_bts_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTS_Ev_Gv, OP_16BIT, sreg, dreg); }
INLINE void emit_bts_m16_r16(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTS_Ev_Gv, OP_16BIT, sreg, MEMPARAMS); }
INLINE void emit_bts_r16_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 5, dreg, imm); }
INLINE void emit_bts_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 5, MEMPARAMS, imm); }

INLINE void emit_btr_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTR_Ev_Gv, OP_16BIT, sreg, dreg); }
INLINE void emit_btr_m16_r16(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTR_Ev_Gv, OP_16BIT, sreg, MEMPARAMS); }
INLINE void emit_btr_r16_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 6, dreg, imm); }
INLINE void emit_btr_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 6, MEMPARAMS, imm); }

INLINE void emit_btc_r16_r16(x86code **emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTC_Ev_Gv, OP_16BIT, sreg, dreg); }
INLINE void emit_btc_m16_r16(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTC_Ev_Gv, OP_16BIT, sreg, MEMPARAMS); }
INLINE void emit_btc_r16_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 7, dreg, imm); }
INLINE void emit_btc_m16_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_16BIT, 7, MEMPARAMS, imm); }


/*-------------------------------------------------
    emit_bit_r32_*
-------------------------------------------------*/

INLINE void emit_bt_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_BT_Ev_Gv, OP_32BIT, sreg, dreg); }
INLINE void emit_bt_m32_r32(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_BT_Ev_Gv, OP_32BIT, sreg, MEMPARAMS); }
INLINE void emit_bt_r32_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)          { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 4, dreg, imm); }
INLINE void emit_bt_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)   { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 4, MEMPARAMS, imm); }

INLINE void emit_bts_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTS_Ev_Gv, OP_32BIT, sreg, dreg); }
INLINE void emit_bts_m32_r32(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTS_Ev_Gv, OP_32BIT, sreg, MEMPARAMS); }
INLINE void emit_bts_r32_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 5, dreg, imm); }
INLINE void emit_bts_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 5, MEMPARAMS, imm); }

INLINE void emit_btr_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTR_Ev_Gv, OP_32BIT, sreg, dreg); }
INLINE void emit_btr_m32_r32(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTR_Ev_Gv, OP_32BIT, sreg, MEMPARAMS); }
INLINE void emit_btr_r32_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 6, dreg, imm); }
INLINE void emit_btr_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 6, MEMPARAMS, imm); }

INLINE void emit_btc_r32_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTC_Ev_Gv, OP_32BIT, sreg, dreg); }
INLINE void emit_btc_m32_r32(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTC_Ev_Gv, OP_32BIT, sreg, MEMPARAMS); }
INLINE void emit_btc_r32_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 7, dreg, imm); }
INLINE void emit_btc_m32_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_32BIT, 7, MEMPARAMS, imm); }


/*-------------------------------------------------
    emit_bit_r64_*
-------------------------------------------------*/

#ifdef PTR64

INLINE void emit_bt_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg)         { emit_op_modrm_reg(emitptr, OP_BT_Ev_Gv, OP_64BIT, sreg, dreg); }
INLINE void emit_bt_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)  { emit_op_modrm_mem(emitptr, OP_BT_Ev_Gv, OP_64BIT, sreg, MEMPARAMS); }
INLINE void emit_bt_r64_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)          { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 4, dreg, imm); }
INLINE void emit_bt_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)   { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 4, MEMPARAMS, imm); }

INLINE void emit_bts_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTS_Ev_Gv, OP_64BIT, sreg, dreg); }
INLINE void emit_bts_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTS_Ev_Gv, OP_64BIT, sreg, MEMPARAMS); }
INLINE void emit_bts_r64_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 5, dreg, imm); }
INLINE void emit_bts_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 5, MEMPARAMS, imm); }

INLINE void emit_btr_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTR_Ev_Gv, OP_64BIT, sreg, dreg); }
INLINE void emit_btr_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTR_Ev_Gv, OP_64BIT, sreg, MEMPARAMS); }
INLINE void emit_btr_r64_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 6, dreg, imm); }
INLINE void emit_btr_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 6, MEMPARAMS, imm); }

INLINE void emit_btc_r64_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg)        { emit_op_modrm_reg(emitptr, OP_BTC_Ev_Gv, OP_64BIT, sreg, dreg); }
INLINE void emit_btc_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg) { emit_op_modrm_mem(emitptr, OP_BTC_Ev_Gv, OP_64BIT, sreg, MEMPARAMS); }
INLINE void emit_btc_r64_imm(x86code **emitptr, UINT8 dreg, UINT8 imm)         { emit_op_modrm_reg_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 7, dreg, imm); }
INLINE void emit_btc_m64_imm(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 imm)  { emit_op_modrm_mem_imm8(emitptr, OP_G8_Ev_Ib, OP_64BIT, 7, MEMPARAMS, imm); }

#endif



/***************************************************************************
    LEA EMITTERS
***************************************************************************/

INLINE void emit_lea_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_LEA_Gv_M, OP_32BIT, dreg, MEMPARAMS); }

#ifdef PTR64
INLINE void emit_lea_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_LEA_Gv_M, OP_64BIT, dreg, MEMPARAMS); }
#endif



/***************************************************************************
    SET EMITTERS
***************************************************************************/

/*-------------------------------------------------
    emit_setcc_*
-------------------------------------------------*/

INLINE void emit_setcc_r8(x86code **emitptr, UINT8 cond, UINT8 dreg)			{ emit_op_modrm_reg(emitptr, OP_SETCC_O_Eb + cond, OP_32BIT, 0, dreg); }
INLINE void emit_setcc_m8(x86code **emitptr, UINT8 cond, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_SETCC_O_Eb + cond, OP_32BIT, 0, MEMPARAMS); }



/***************************************************************************
    SIMPLE FPU EMITTERS
***************************************************************************/

#ifndef PTR64

INLINE void emit_fnop(x86code **emitptr)     { emit_op_simple(emitptr, OP_FNOP, OP_32BIT); }
INLINE void emit_fchs(x86code **emitptr)     { emit_op_simple(emitptr, OP_FCHS, OP_32BIT); }
INLINE void emit_fabs(x86code **emitptr)     { emit_op_simple(emitptr, OP_FABS, OP_32BIT); }
INLINE void emit_ftst(x86code **emitptr)     { emit_op_simple(emitptr, OP_FTST, OP_32BIT); }
INLINE void emit_fxam(x86code **emitptr)     { emit_op_simple(emitptr, OP_FXAM, OP_32BIT); }
INLINE void emit_fld1(x86code **emitptr)     { emit_op_simple(emitptr, OP_FLD1, OP_32BIT); }
INLINE void emit_fldl2t(x86code **emitptr)   { emit_op_simple(emitptr, OP_FLDL2T, OP_32BIT); }
INLINE void emit_fldl2e(x86code **emitptr)   { emit_op_simple(emitptr, OP_FLDL2E, OP_32BIT); }
INLINE void emit_fldpi(x86code **emitptr)    { emit_op_simple(emitptr, OP_FLDPI, OP_32BIT); }
INLINE void emit_fldlg2(x86code **emitptr)   { emit_op_simple(emitptr, OP_FLDLG2, OP_32BIT); }
INLINE void emit_fldln2(x86code **emitptr)   { emit_op_simple(emitptr, OP_FLDLN2, OP_32BIT); }
INLINE void emit_fldz(x86code **emitptr)     { emit_op_simple(emitptr, OP_FLDZ, OP_32BIT); }
INLINE void emit_f2xm1(x86code **emitptr)    { emit_op_simple(emitptr, OP_F2XM1, OP_32BIT); }
INLINE void emit_fyl2x(x86code **emitptr)    { emit_op_simple(emitptr, OP_FYL2X, OP_32BIT); }
INLINE void emit_fptan(x86code **emitptr)    { emit_op_simple(emitptr, OP_FPTAN, OP_32BIT); }
INLINE void emit_fpatan(x86code **emitptr)   { emit_op_simple(emitptr, OP_FPATAN, OP_32BIT); }
INLINE void emit_fxtract(x86code **emitptr)  { emit_op_simple(emitptr, OP_FXTRACT, OP_32BIT); }
INLINE void emit_fprem1(x86code **emitptr)   { emit_op_simple(emitptr, OP_FPREM1, OP_32BIT); }
INLINE void emit_fdecstp(x86code **emitptr)  { emit_op_simple(emitptr, OP_FDECSTP, OP_32BIT); }
INLINE void emit_fincstp(x86code **emitptr)  { emit_op_simple(emitptr, OP_FINCSTP, OP_32BIT); }
INLINE void emit_fprem(x86code **emitptr)    { emit_op_simple(emitptr, OP_FPREM, OP_32BIT); }
INLINE void emit_fyl2xp1(x86code **emitptr)  { emit_op_simple(emitptr, OP_FYL2XP1, OP_32BIT); }
INLINE void emit_fsqrt(x86code **emitptr)    { emit_op_simple(emitptr, OP_FSQRT, OP_32BIT); }
INLINE void emit_fsincos(x86code **emitptr)  { emit_op_simple(emitptr, OP_FSINCOS, OP_32BIT); }
INLINE void emit_frndint(x86code **emitptr)  { emit_op_simple(emitptr, OP_FRNDINT, OP_32BIT); }
INLINE void emit_fscale(x86code **emitptr)   { emit_op_simple(emitptr, OP_FSCALE, OP_32BIT); }
INLINE void emit_fsin(x86code **emitptr)     { emit_op_simple(emitptr, OP_FSIN, OP_32BIT); }
INLINE void emit_fcos(x86code **emitptr)     { emit_op_simple(emitptr, OP_FCOS, OP_32BIT); }
INLINE void emit_fucompp(x86code **emitptr)  { emit_op_simple(emitptr, OP_FUCOMPP, OP_32BIT); }
INLINE void emit_fclex(x86code **emitptr)    { emit_op_simple(emitptr, OP_FCLEX, OP_32BIT); }
INLINE void emit_finit(x86code **emitptr)    { emit_op_simple(emitptr, OP_FINIT, OP_32BIT); }
INLINE void emit_fcompp(x86code **emitptr)   { emit_op_simple(emitptr, OP_FCOMPP, OP_32BIT); }
INLINE void emit_fstsw_ax(x86code **emitptr) { emit_op_simple(emitptr, OP_FSTSW_AX, OP_32BIT); }

#endif



/***************************************************************************
    REGISTER-BASED FPU EMITTERS
***************************************************************************/

#ifndef PTR64

INLINE void emit_ffree_stn(x86code **emitptr, UINT8 reg)		{ emit_op_simple(emitptr, OP_FFREE_STn + reg, OP_32BIT); }
INLINE void emit_fst_stn(x86code **emitptr, UINT8 reg)			{ emit_op_simple(emitptr, OP_FST_STn + reg, OP_32BIT); }
INLINE void emit_fstp_stn(x86code **emitptr, UINT8 reg)			{ emit_op_simple(emitptr, OP_FSTP_STn + reg, OP_32BIT); }
INLINE void emit_fucomp_stn(x86code **emitptr, UINT8 reg)		{ emit_op_simple(emitptr, OP_FUCOMP_STn + reg, OP_32BIT); }

INLINE void emit_fadd_st0_stn(x86code **emitptr, UINT8 reg)		{ emit_op_simple(emitptr, OP_FADD_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fmul_st0_stn(x86code **emitptr, UINT8 reg)		{ emit_op_simple(emitptr, OP_FMUL_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fcom_st0_stn(x86code **emitptr, UINT8 reg)		{ emit_op_simple(emitptr, OP_FCOM_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fcomp_st0_stn(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FCOMP_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fsub_st0_stn(x86code **emitptr, UINT8 reg)		{ emit_op_simple(emitptr, OP_FSUB_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fsubr_st0_stn(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FSUBR_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fdiv_st0_stn(x86code **emitptr, UINT8 reg)		{ emit_op_simple(emitptr, OP_FDIV_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fdivr_st0_stn(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FDIVR_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fld_st0_stn(x86code **emitptr, UINT8 reg)		{ emit_op_simple(emitptr, OP_FLD_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fxch_st0_stn(x86code **emitptr, UINT8 reg)		{ emit_op_simple(emitptr, OP_FXCH_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fcmovb_st0_stn(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FCMOVB_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fcmove_st0_stn(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FCMOVE_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fcmovbe_st0_stn(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FCMOVBE_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fcmovu_st0_stn(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FCMOVU_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fcmovnb_st0_stn(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FCMOVNB_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fcmovne_st0_stn(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FCMOVNE_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fcmovnbe_st0_stn(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FCMOVNBE_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fcmovnu_st0_stn(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FCMOVNU_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fucomi_st0_stn(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FUCOMI_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fcomi_st0_stn(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FCOMI_ST0_STn + reg, OP_32BIT); }
INLINE void emit_fcomip_st0_stn(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FCOMIP_ST0_STn + reg, OP_32BIT); }

INLINE void emit_fadd_stn_st0(x86code **emitptr, UINT8 reg)		{ emit_op_simple(emitptr, OP_FADD_STn_ST0 + reg, OP_32BIT); }
INLINE void emit_fmul_stn_st0(x86code **emitptr, UINT8 reg)		{ emit_op_simple(emitptr, OP_FMUL_STn_ST0 + reg, OP_32BIT); }
INLINE void emit_fsubr_stn_st0(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FSUBR_STn_ST0 + reg, OP_32BIT); }
INLINE void emit_fsub_stn_st0(x86code **emitptr, UINT8 reg)		{ emit_op_simple(emitptr, OP_FSUB_STn_ST0 + reg, OP_32BIT); }
INLINE void emit_fdivr_stn_st0(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FDIVR_STn_ST0 + reg, OP_32BIT); }
INLINE void emit_fdiv_stn_st0(x86code **emitptr, UINT8 reg)		{ emit_op_simple(emitptr, OP_FDIV_STn_ST0 + reg, OP_32BIT); }
INLINE void emit_fucom_stn_st0(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FUCOM_STn_ST0 + reg, OP_32BIT); }
INLINE void emit_faddp_stn_st0(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FADDP_STn_ST0 + reg, OP_32BIT); }
INLINE void emit_fmulp_stn_st0(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FMULP_STn_ST0 + reg, OP_32BIT); }
INLINE void emit_fsubrp_stn_st0(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FSUBRP_STn_ST0 + reg, OP_32BIT); }
INLINE void emit_fsubp_stn_st0(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FSUBP_STn_ST0 + reg, OP_32BIT); }
INLINE void emit_fdivrp_stn_st0(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FDIVRP_STn_ST0 + reg, OP_32BIT); }
INLINE void emit_fdivp_stn_st0(x86code **emitptr, UINT8 reg)	{ emit_op_simple(emitptr, OP_FDIVP_STn_ST0 + reg, OP_32BIT); }

INLINE void emit_faddp(x86code **emitptr)						{ emit_faddp_stn_st0(emitptr, 1); }
INLINE void emit_fmulp(x86code **emitptr)						{ emit_fmulp_stn_st0(emitptr, 1); }
INLINE void emit_fsubrp(x86code **emitptr)						{ emit_fsubrp_stn_st0(emitptr, 1); }
INLINE void emit_fsubp(x86code **emitptr)						{ emit_fsubp_stn_st0(emitptr, 1); }
INLINE void emit_fdivrp(x86code **emitptr)						{ emit_fdivrp_stn_st0(emitptr, 1); }
INLINE void emit_fdivp(x86code **emitptr)						{ emit_fdivp_stn_st0(emitptr, 1); }

#endif



/***************************************************************************
    MEMORY FPU EMITTERS
***************************************************************************/

#ifndef PTR64

INLINE void emit_fadd_m32(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 0, MEMPARAMS); }
INLINE void emit_fmul_m32(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 1, MEMPARAMS); }
INLINE void emit_fcom_m32(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 2, MEMPARAMS); }
INLINE void emit_fcomp_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 3, MEMPARAMS); }
INLINE void emit_fsub_m32(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 4, MEMPARAMS); }
INLINE void emit_fsubr_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 5, MEMPARAMS); }
INLINE void emit_fdiv_m32(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 6, MEMPARAMS); }
INLINE void emit_fdivr_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_D8, OP_32BIT, 7, MEMPARAMS); }

INLINE void emit_fld_m32(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_D9, OP_32BIT, 0, MEMPARAMS); }
INLINE void emit_fst_m32(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_D9, OP_32BIT, 2, MEMPARAMS); }
INLINE void emit_fstp_m32(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_D9, OP_32BIT, 3, MEMPARAMS); }
INLINE void emit_fldenv_m(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_D9, OP_32BIT, 4, MEMPARAMS); }
INLINE void emit_fldcw_m16(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_D9, OP_32BIT, 5, MEMPARAMS); }
INLINE void emit_fstenv_m(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_D9, OP_32BIT, 6, MEMPARAMS); }
INLINE void emit_fstcw_m16(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_D9, OP_32BIT, 7, MEMPARAMS); }

INLINE void emit_fiadd_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 0, MEMPARAMS); }
INLINE void emit_fimul_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 1, MEMPARAMS); }
INLINE void emit_ficom_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 2, MEMPARAMS); }
INLINE void emit_ficomp_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 3, MEMPARAMS); }
INLINE void emit_fisub_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 4, MEMPARAMS); }
INLINE void emit_fisubr_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 5, MEMPARAMS); }
INLINE void emit_fidiv_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 6, MEMPARAMS); }
INLINE void emit_fidivr_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DA, OP_32BIT, 7, MEMPARAMS); }

INLINE void emit_fild_m32(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DB, OP_32BIT, 0, MEMPARAMS); }
INLINE void emit_fisttp_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DB, OP_32BIT, 1, MEMPARAMS); }
INLINE void emit_fist_m32(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DB, OP_32BIT, 2, MEMPARAMS); }
INLINE void emit_fistp_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DB, OP_32BIT, 3, MEMPARAMS); }
INLINE void emit_fld_m80(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DB, OP_32BIT, 5, MEMPARAMS); }
INLINE void emit_fstp_m80(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DB, OP_32BIT, 7, MEMPARAMS); }

INLINE void emit_fadd_m64(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 0, MEMPARAMS); }
INLINE void emit_fmul_m64(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 1, MEMPARAMS); }
INLINE void emit_fcom_m64(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 2, MEMPARAMS); }
INLINE void emit_fcomp_m64(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 3, MEMPARAMS); }
INLINE void emit_fsub_m64(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 4, MEMPARAMS); }
INLINE void emit_fsubr_m64(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 5, MEMPARAMS); }
INLINE void emit_fdiv_m64(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 6, MEMPARAMS); }
INLINE void emit_fdivr_m64(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DC, OP_32BIT, 7, MEMPARAMS); }

INLINE void emit_fld_m64(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DD, OP_32BIT, 0, MEMPARAMS); }
INLINE void emit_fisttp_m64(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DD, OP_32BIT, 1, MEMPARAMS); }
INLINE void emit_fst_m64(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DD, OP_32BIT, 2, MEMPARAMS); }
INLINE void emit_fstp_m64(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DD, OP_32BIT, 3, MEMPARAMS); }
INLINE void emit_frstor_m(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DD, OP_32BIT, 4, MEMPARAMS); }
INLINE void emit_fsave_m(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DD, OP_32BIT, 6, MEMPARAMS); }
INLINE void emit_fstsw_m16(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DD, OP_32BIT, 7, MEMPARAMS); }

INLINE void emit_fiadd_m16(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 0, MEMPARAMS); }
INLINE void emit_fimul_m16(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 1, MEMPARAMS); }
INLINE void emit_ficom_m16(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 2, MEMPARAMS); }
INLINE void emit_ficomp_m16(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 3, MEMPARAMS); }
INLINE void emit_fisub_m16(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 4, MEMPARAMS); }
INLINE void emit_fisubr_m16(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 5, MEMPARAMS); }
INLINE void emit_fidiv_m16(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 6, MEMPARAMS); }
INLINE void emit_fidivr_m16(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DE, OP_32BIT, 7, MEMPARAMS); }

INLINE void emit_fild_m16(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 0, MEMPARAMS); }
INLINE void emit_fisttp_m16(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 1, MEMPARAMS); }
INLINE void emit_fist_m16(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 2, MEMPARAMS); }
INLINE void emit_fistp_m16(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 3, MEMPARAMS); }
INLINE void emit_fbld_m80(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 4, MEMPARAMS); }
INLINE void emit_fild_m64(x86code **emitptr, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 5, MEMPARAMS); }
INLINE void emit_fbstp_m80(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 6, MEMPARAMS); }
INLINE void emit_fistp_m64(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_ESC_DF, OP_32BIT, 7, MEMPARAMS); }

#endif



/***************************************************************************
    GROUP16 EMITTERS
***************************************************************************/

INLINE void emit_ldmxcsr_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_G16, OP_32BIT, 2, MEMPARAMS); }
INLINE void emit_stmxcsr_m32(x86code **emitptr, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_G16, OP_32BIT, 3, MEMPARAMS); }



/***************************************************************************
    MISC SSE EMITTERS
***************************************************************************/

INLINE void emit_movd_r128_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_MOVD_Vd_Ed, OP_32BIT, dreg, sreg); }
INLINE void emit_movd_r128_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_MOVD_Vd_Ed, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_movd_r32_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_MOVD_Ed_Vd, OP_32BIT, sreg, dreg); }
INLINE void emit_movd_m32_r128(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)		{ emit_op_modrm_mem(emitptr, OP_MOVD_Ed_Vd, OP_32BIT, sreg, MEMPARAMS); }

#ifdef PTR64

INLINE void emit_movq_r128_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_MOVD_Vd_Ed, OP_64BIT, dreg, sreg); }
INLINE void emit_movq_r128_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_MOVD_Vd_Ed, OP_64BIT, dreg, MEMPARAMS); }
INLINE void emit_movq_r64_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_MOVD_Ed_Vd, OP_64BIT, sreg, dreg); }
INLINE void emit_movq_m64_r128(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)		{ emit_op_modrm_mem(emitptr, OP_MOVD_Ed_Vd, OP_64BIT, sreg, MEMPARAMS); }

#endif



/***************************************************************************
    SSE SCALAR SINGLE EMITTERS
***************************************************************************/

INLINE void emit_movss_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_MOVSS_Vss_Wss, OP_32BIT, dreg, sreg); }
INLINE void emit_movss_r128_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_MOVSS_Vss_Wss, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_movss_m32_r128(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)		{ emit_op_modrm_mem(emitptr, OP_MOVSS_Wss_Vss, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_addss_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_ADDSS_Vss_Wss, OP_32BIT, dreg, sreg); }
INLINE void emit_addss_r128_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ADDSS_Vss_Wss, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_subss_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_SUBSS_Vss_Wss, OP_32BIT, dreg, sreg); }
INLINE void emit_subss_r128_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_SUBSS_Vss_Wss, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_mulss_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_MULSS_Vss_Wss, OP_32BIT, dreg, sreg); }
INLINE void emit_mulss_r128_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_MULSS_Vss_Wss, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_divss_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_DIVSS_Vss_Wss, OP_32BIT, dreg, sreg); }
INLINE void emit_divss_r128_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_DIVSS_Vss_Wss, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_rcpss_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_RCPSS_Vss_Wss, OP_32BIT, dreg, sreg); }
INLINE void emit_rcpss_r128_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_RCPSS_Vss_Wss, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_sqrtss_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_SQRTSS_Vss_Wss, OP_32BIT, dreg, sreg); }
INLINE void emit_sqrtss_r128_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_SQRTSS_Vss_Wss, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_rsqrtss_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_RSQRTSS_Vss_Wss, OP_32BIT, dreg, sreg); }
INLINE void emit_rsqrtss_r128_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_RSQRTSS_Vss_Wss, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_comiss_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_COMISS_Vss_Wss, OP_32BIT, dreg, sreg); }
INLINE void emit_comiss_r128_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_COMISS_Vss_Wss, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_ucomiss_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_UCOMISS_Vss_Wss, OP_32BIT, dreg, sreg); }
INLINE void emit_ucomiss_r128_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_UCOMISS_Vss_Wss, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_cvtsi2ss_r128_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTSI2SS_Vss_Ed, OP_32BIT, dreg, sreg); }
INLINE void emit_cvtsi2ss_r128_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTSI2SS_Vss_Ed, OP_32BIT, dreg, MEMPARAMS); }

#ifdef PTR64
INLINE void emit_cvtsi2ss_r128_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTSI2SS_Vss_Ed, OP_64BIT, dreg, sreg); }
INLINE void emit_cvtsi2ss_r128_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTSI2SS_Vss_Ed, OP_64BIT, dreg, MEMPARAMS); }
#endif

INLINE void emit_cvtsd2ss_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTSD2SS_Vss_Wsd, OP_32BIT, dreg, sreg); }
INLINE void emit_cvtsd2ss_r128_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTSD2SS_Vss_Wsd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_cvtss2si_r32_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTSS2SI_Gd_Wss, OP_32BIT, dreg, sreg); }
INLINE void emit_cvtss2si_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_CVTSS2SI_Gd_Wss, OP_32BIT, dreg, MEMPARAMS); }

#ifdef PTR64
INLINE void emit_cvtss2si_r64_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTSS2SI_Gd_Wss, OP_64BIT, dreg, sreg); }
INLINE void emit_cvtss2si_r64_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_CVTSS2SI_Gd_Wss, OP_64BIT, dreg, MEMPARAMS); }
#endif

INLINE void emit_cvttss2si_r32_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTTSS2SI_Gd_Wss, OP_32BIT, dreg, sreg); }
INLINE void emit_cvttss2si_r32_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTTSS2SI_Gd_Wss, OP_32BIT, dreg, MEMPARAMS); }

#ifdef PTR64
INLINE void emit_cvttss2si_r64_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTTSS2SI_Gd_Wss, OP_64BIT, dreg, sreg); }
INLINE void emit_cvttss2si_r64_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTTSS2SI_Gd_Wss, OP_64BIT, dreg, MEMPARAMS); }
#endif

INLINE void emit_roundss_r128_r128_imm(x86code **emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)		{ emit_op_modrm_reg(emitptr, OP_ROUNDSS_Vss_Wss_Ib, OP_32BIT, dreg, sreg); emit_byte(emitptr, imm); }
INLINE void emit_roundss_r128_m32_imm(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS, UINT8 imm)	{ emit_op_modrm_mem(emitptr, OP_ROUNDSS_Vss_Wss_Ib, OP_32BIT, dreg, MEMPARAMS); emit_byte(emitptr, imm); }



/***************************************************************************
    SSE PACKED SINGLE EMITTERS
***************************************************************************/

INLINE void emit_movps_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_MOVAPS_Vps_Wps, OP_32BIT, dreg, sreg); }
INLINE void emit_movps_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_MOVAPS_Vps_Wps, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_movps_m128_r128(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)		{ emit_op_modrm_mem(emitptr, OP_MOVAPS_Wps_Vps, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_addps_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_ADDPS_Vps_Wps, OP_32BIT, dreg, sreg); }
INLINE void emit_addps_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ADDPS_Vps_Wps, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_subps_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_SUBPS_Vps_Wps, OP_32BIT, dreg, sreg); }
INLINE void emit_subps_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_SUBPS_Vps_Wps, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_mulps_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_MULPS_Vps_Wps, OP_32BIT, dreg, sreg); }
INLINE void emit_mulps_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_MULPS_Vps_Wps, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_divps_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_DIVPS_Vps_Wps, OP_32BIT, dreg, sreg); }
INLINE void emit_divps_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_DIVPS_Vps_Wps, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_rcpps_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_RCPPS_Vps_Wps, OP_32BIT, dreg, sreg); }
INLINE void emit_rcpps_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_RCPPS_Vps_Wps, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_sqrtps_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_SQRTPS_Vps_Wps, OP_32BIT, dreg, sreg); }
INLINE void emit_sqrtps_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_SQRTPS_Vps_Wps, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_rsqrtps_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_RSQRTPS_Vps_Wps, OP_32BIT, dreg, sreg); }
INLINE void emit_rsqrtps_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_RSQRTPS_Vps_Wps, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_andps_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_ANDPS_Vps_Wps, OP_32BIT, dreg, sreg); }
INLINE void emit_andps_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ANDPS_Vps_Wps, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_andnps_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_ANDNPS_Vps_Wps, OP_32BIT, dreg, sreg); }
INLINE void emit_andnps_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ANDNPS_Vps_Wps, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_orps_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_ORPS_Vps_Wps, OP_32BIT, dreg, sreg); }
INLINE void emit_orps_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ORPS_Vps_Wps, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_xorps_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_XORPS_Vps_Wps, OP_32BIT, dreg, sreg); }
INLINE void emit_xorps_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_XORPS_Vps_Wps, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_cvtdq2ps_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTDQ2PS_Vps_Wdq, OP_32BIT, dreg, sreg); }
INLINE void emit_cvtdq2ps_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTDQ2PS_Vps_Wdq, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_cvtpd2ps_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTPD2PS_Vps_Wpd, OP_32BIT, dreg, sreg); }
INLINE void emit_cvtpd2ps_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTPD2PS_Vps_Wpd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_cvtps2dq_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTPS2DQ_Vdq_Wps, OP_32BIT, dreg, sreg); }
INLINE void emit_cvtps2dq_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTPS2DQ_Vdq_Wps, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_cvttps2dq_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTTPS2DQ_Vdq_Wps, OP_32BIT, dreg, sreg); }
INLINE void emit_cvttps2dq_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_CVTTPS2DQ_Vdq_Wps, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_roundps_r128_r128_imm(x86code **emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)		{ emit_op_modrm_reg(emitptr, OP_ROUNDPS_Vdq_Wdq_Ib, OP_32BIT, dreg, sreg); emit_byte(emitptr, imm); }
INLINE void emit_roundps_r128_m128_imm(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS, UINT8 imm)	{ emit_op_modrm_mem(emitptr, OP_ROUNDPS_Vdq_Wdq_Ib, OP_32BIT, dreg, MEMPARAMS); emit_byte(emitptr, imm); }



/***************************************************************************
    SSE SCALAR DOUBLE EMITTERS
***************************************************************************/

INLINE void emit_movsd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_MOVSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
INLINE void emit_movsd_r128_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_MOVSD_Vsd_Wsd, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_movsd_m64_r128(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)		{ emit_op_modrm_mem(emitptr, OP_MOVSD_Wsd_Vsd, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_addsd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_ADDSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
INLINE void emit_addsd_r128_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ADDSD_Vsd_Wsd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_subsd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_SUBSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
INLINE void emit_subsd_r128_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_SUBSD_Vsd_Wsd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_mulsd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_MULSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
INLINE void emit_mulsd_r128_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_MULSD_Vsd_Wsd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_divsd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_DIVSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
INLINE void emit_divsd_r128_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_DIVSD_Vsd_Wsd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_sqrtsd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_SQRTSD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
INLINE void emit_sqrtsd_r128_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_SQRTSD_Vsd_Wsd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_comisd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_COMISD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
INLINE void emit_comisd_r128_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_COMISD_Vsd_Wsd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_ucomisd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_UCOMISD_Vsd_Wsd, OP_32BIT, dreg, sreg); }
INLINE void emit_ucomisd_r128_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_UCOMISD_Vsd_Wsd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_cvtsi2sd_r128_r32(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTSI2SD_Vsd_Ed, OP_32BIT, dreg, sreg); }
INLINE void emit_cvtsi2sd_r128_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTSI2SD_Vsd_Ed, OP_32BIT, dreg, MEMPARAMS); }

#ifdef PTR64
INLINE void emit_cvtsi2sd_r128_r64(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTSI2SD_Vsd_Ed, OP_64BIT, dreg, sreg); }
INLINE void emit_cvtsi2sd_r128_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTSI2SD_Vsd_Ed, OP_64BIT, dreg, MEMPARAMS); }
#endif

INLINE void emit_cvtss2sd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTSS2SD_Vsd_Wss, OP_32BIT, dreg, sreg); }
INLINE void emit_cvtss2sd_r128_m32(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTSS2SD_Vsd_Wss, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_cvtsd2si_r32_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTSD2SI_Gd_Wsd, OP_32BIT, dreg, sreg); }
INLINE void emit_cvtsd2si_r32_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_CVTSD2SI_Gd_Wsd, OP_32BIT, dreg, MEMPARAMS); }

#ifdef PTR64
INLINE void emit_cvtsd2si_r64_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTSD2SI_Gd_Wsd, OP_64BIT, dreg, sreg); }
INLINE void emit_cvtsd2si_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_CVTSD2SI_Gd_Wsd, OP_64BIT, dreg, MEMPARAMS); }
#endif

INLINE void emit_cvttsd2si_r32_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTTSD2SI_Gd_Wsd, OP_32BIT, dreg, sreg); }
INLINE void emit_cvttsd2si_r32_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTTSD2SI_Gd_Wsd, OP_32BIT, dreg, MEMPARAMS); }

#ifdef PTR64
INLINE void emit_cvttsd2si_r64_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTTSD2SI_Gd_Wsd, OP_64BIT, dreg, sreg); }
INLINE void emit_cvttsd2si_r64_m64(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTTSD2SI_Gd_Wsd, OP_64BIT, dreg, MEMPARAMS); }
#endif

INLINE void emit_roundsd_r128_r128_imm(x86code **emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)		{ emit_op_modrm_reg(emitptr, OP_ROUNDSD_Vsd_Wsd_Ib, OP_32BIT, dreg, sreg); emit_byte(emitptr, imm); }
INLINE void emit_roundsd_r128_m64_imm(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS, UINT8 imm)	{ emit_op_modrm_mem(emitptr, OP_ROUNDSD_Vsd_Wsd_Ib, OP_32BIT, dreg, MEMPARAMS); emit_byte(emitptr, imm); }



/***************************************************************************
    SSE PACKED DOUBLE EMITTERS
***************************************************************************/

INLINE void emit_movpd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_MOVAPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
INLINE void emit_movpd_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_MOVAPD_Vpd_Wpd, OP_32BIT, dreg, MEMPARAMS); }
INLINE void emit_movpd_m128_r128(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 sreg)		{ emit_op_modrm_mem(emitptr, OP_MOVAPD_Wpd_Vpd, OP_32BIT, sreg, MEMPARAMS); }

INLINE void emit_addpd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_ADDPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
INLINE void emit_addpd_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ADDPD_Vpd_Wpd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_subpd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_SUBPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
INLINE void emit_subpd_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_SUBPD_Vpd_Wpd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_mulpd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_MULPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
INLINE void emit_mulpd_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_MULPD_Vpd_Wpd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_divpd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_DIVPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
INLINE void emit_divpd_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_DIVPD_Vpd_Wpd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_sqrtpd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_SQRTPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
INLINE void emit_sqrtpd_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_SQRTPD_Vpd_Wpd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_andpd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_ANDPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
INLINE void emit_andpd_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ANDPD_Vpd_Wpd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_andnpd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_ANDNPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
INLINE void emit_andnpd_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ANDNPD_Vpd_Wpd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_orpd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_ORPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
INLINE void emit_orpd_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_ORPD_Vpd_Wpd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_xorpd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)				{ emit_op_modrm_reg(emitptr, OP_XORPD_Vpd_Wpd, OP_32BIT, dreg, sreg); }
INLINE void emit_xorpd_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)		{ emit_op_modrm_mem(emitptr, OP_XORPD_Vpd_Wpd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_cvtdq2pd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTDQ2PD_Vpd_Wq, OP_32BIT, dreg, sreg); }
INLINE void emit_cvtdq2pd_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTDQ2PD_Vpd_Wq, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_cvtps2pd_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTPS2PD_Vpd_Wq, OP_32BIT, dreg, sreg); }
INLINE void emit_cvtps2pd_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTPS2PD_Vpd_Wq, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_cvtpd2dq_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTPD2DQ_Vdq_Wpd, OP_32BIT, dreg, sreg); }
INLINE void emit_cvtpd2dq_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS)	{ emit_op_modrm_mem(emitptr, OP_CVTPD2DQ_Vdq_Wpd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_cvttpd2dq_r128_r128(x86code **emitptr, UINT8 dreg, UINT8 sreg)			{ emit_op_modrm_reg(emitptr, OP_CVTTPD2DQ_Vdq_Wpd, OP_32BIT, dreg, sreg); }
INLINE void emit_cvttpd2dq_r128_m128(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS) 	{ emit_op_modrm_mem(emitptr, OP_CVTTPD2DQ_Vdq_Wpd, OP_32BIT, dreg, MEMPARAMS); }

INLINE void emit_roundpd_r128_r128_imm(x86code **emitptr, UINT8 dreg, UINT8 sreg, UINT8 imm)		{ emit_op_modrm_reg(emitptr, OP_ROUNDPD_Vdq_Wdq_Ib, OP_32BIT, dreg, sreg); emit_byte(emitptr, imm); }
INLINE void emit_roundpd_r128_m128_imm(x86code **emitptr, UINT8 dreg, DECLARE_MEMPARAMS, UINT8 imm) { emit_op_modrm_mem(emitptr, OP_ROUNDPD_Vdq_Wdq_Ib, OP_32BIT, dreg, MEMPARAMS); emit_byte(emitptr, imm); }

#endif /* __X86EMIT_H__ */
