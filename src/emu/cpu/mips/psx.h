#pragma once

#ifndef __PSX_H__
#define __PSX_H__


#define PSXCPU_DELAYR_PC ( 32 )
#define PSXCPU_DELAYR_NOTPC ( 33 )

enum
{
	PSXCPU_PC = 1,
	PSXCPU_DELAYV, PSXCPU_DELAYR,
	PSXCPU_HI, PSXCPU_LO,
	PSXCPU_BIU,
	PSXCPU_R0, PSXCPU_R1,
	PSXCPU_R2, PSXCPU_R3,
	PSXCPU_R4, PSXCPU_R5,
	PSXCPU_R6, PSXCPU_R7,
	PSXCPU_R8, PSXCPU_R9,
	PSXCPU_R10, PSXCPU_R11,
	PSXCPU_R12, PSXCPU_R13,
	PSXCPU_R14, PSXCPU_R15,
	PSXCPU_R16, PSXCPU_R17,
	PSXCPU_R18, PSXCPU_R19,
	PSXCPU_R20, PSXCPU_R21,
	PSXCPU_R22, PSXCPU_R23,
	PSXCPU_R24, PSXCPU_R25,
	PSXCPU_R26, PSXCPU_R27,
	PSXCPU_R28, PSXCPU_R29,
	PSXCPU_R30, PSXCPU_R31,
	PSXCPU_CP0R0, PSXCPU_CP0R1,
	PSXCPU_CP0R2, PSXCPU_CP0R3,
	PSXCPU_CP0R4, PSXCPU_CP0R5,
	PSXCPU_CP0R6, PSXCPU_CP0R7,
	PSXCPU_CP0R8, PSXCPU_CP0R9,
	PSXCPU_CP0R10, PSXCPU_CP0R11,
	PSXCPU_CP0R12, PSXCPU_CP0R13,
	PSXCPU_CP0R14, PSXCPU_CP0R15,
	PSXCPU_CP2DR0, PSXCPU_CP2DR1,
	PSXCPU_CP2DR2, PSXCPU_CP2DR3,
	PSXCPU_CP2DR4, PSXCPU_CP2DR5,
	PSXCPU_CP2DR6, PSXCPU_CP2DR7,
	PSXCPU_CP2DR8, PSXCPU_CP2DR9,
	PSXCPU_CP2DR10, PSXCPU_CP2DR11,
	PSXCPU_CP2DR12, PSXCPU_CP2DR13,
	PSXCPU_CP2DR14, PSXCPU_CP2DR15,
	PSXCPU_CP2DR16, PSXCPU_CP2DR17,
	PSXCPU_CP2DR18, PSXCPU_CP2DR19,
	PSXCPU_CP2DR20, PSXCPU_CP2DR21,
	PSXCPU_CP2DR22, PSXCPU_CP2DR23,
	PSXCPU_CP2DR24, PSXCPU_CP2DR25,
	PSXCPU_CP2DR26, PSXCPU_CP2DR27,
	PSXCPU_CP2DR28, PSXCPU_CP2DR29,
	PSXCPU_CP2DR30, PSXCPU_CP2DR31,
	PSXCPU_CP2CR0, PSXCPU_CP2CR1,
	PSXCPU_CP2CR2, PSXCPU_CP2CR3,
	PSXCPU_CP2CR4, PSXCPU_CP2CR5,
	PSXCPU_CP2CR6, PSXCPU_CP2CR7,
	PSXCPU_CP2CR8, PSXCPU_CP2CR9,
	PSXCPU_CP2CR10, PSXCPU_CP2CR11,
	PSXCPU_CP2CR12, PSXCPU_CP2CR13,
	PSXCPU_CP2CR14, PSXCPU_CP2CR15,
	PSXCPU_CP2CR16, PSXCPU_CP2CR17,
	PSXCPU_CP2CR18, PSXCPU_CP2CR19,
	PSXCPU_CP2CR20, PSXCPU_CP2CR21,
	PSXCPU_CP2CR22, PSXCPU_CP2CR23,
	PSXCPU_CP2CR24, PSXCPU_CP2CR25,
	PSXCPU_CP2CR26, PSXCPU_CP2CR27,
	PSXCPU_CP2CR28, PSXCPU_CP2CR29,
	PSXCPU_CP2CR30, PSXCPU_CP2CR31
};

#define PSXCPU_IRQ0	( 0 )
#define PSXCPU_IRQ1	( 1 )
#define PSXCPU_IRQ2	( 2 )
#define PSXCPU_IRQ3	( 3 )
#define PSXCPU_IRQ4	( 4 )
#define PSXCPU_IRQ5	( 5 )

#define PSXCPU_BYTE_EXTEND( a ) ( (INT32)(INT8)a )
#define PSXCPU_WORD_EXTEND( a ) ( (INT32)(INT16)a )

#define INS_OP( op ) ( ( op >> 26 ) & 63 )
#define INS_RS( op ) ( ( op >> 21 ) & 31 )
#define INS_RT( op ) ( ( op >> 16 ) & 31 )
#define INS_IMMEDIATE( op ) ( op & 0xffff )
#define INS_TARGET( op ) ( op & 0x3ffffff )
#define INS_RD( op ) ( ( op >> 11 ) & 31 )
#define INS_SHAMT( op ) ( ( op >> 6 ) & 31 )
#define INS_FUNCT( op ) ( op & 63 )
#define INS_CODE( op ) ( ( op >> 6 ) & 0xfffff )
#define INS_CO( op ) ( ( op >> 25 ) & 1 )
#define INS_COFUN( op ) ( op & 0x1ffffff )
#define INS_CF( op ) ( op & 31 )
#define INS_BC( op ) ( ( op >> 16 ) & 1 )
#define INS_RT_REGIMM( op ) ( ( op >> 16 ) & 1 )

#define GTE_OP( op ) ( ( op >> 20 ) & 31 )
#define GTE_SF( op ) ( ( op >> 19 ) & 1 )
#define GTE_MX( op ) ( ( op >> 17 ) & 3 )
#define GTE_V( op ) ( ( op >> 15 ) & 3 )
#define GTE_CV( op ) ( ( op >> 13 ) & 3 )
#define GTE_CD( op ) ( ( op >> 11 ) & 3 ) /* not used */
#define GTE_LM( op ) ( ( op >> 10 ) & 1 )
#define GTE_CT( op ) ( ( op >> 6 ) & 15 ) /* not used */
#define GTE_FUNCT( op ) ( op & 63 )

#define OP_SPECIAL ( 0 )
#define OP_REGIMM ( 1 )
#define OP_J ( 2 )
#define OP_JAL ( 3 )
#define OP_BEQ ( 4 )
#define OP_BNE ( 5 )
#define OP_BLEZ ( 6 )
#define OP_BGTZ ( 7 )
#define OP_ADDI ( 8 )
#define OP_ADDIU ( 9 )
#define OP_SLTI ( 10 )
#define OP_SLTIU ( 11 )
#define OP_ANDI ( 12 )
#define OP_ORI ( 13 )
#define OP_XORI ( 14 )
#define OP_LUI ( 15 )
#define OP_COP0 ( 16 )
#define OP_COP1 ( 17 )
#define OP_COP2 ( 18 )
#define OP_COP3 ( 19 )
#define OP_LB ( 32 )
#define OP_LH ( 33 )
#define OP_LWL ( 34 )
#define OP_LW ( 35 )
#define OP_LBU ( 36 )
#define OP_LHU ( 37 )
#define OP_LWR ( 38 )
#define OP_SB ( 40 )
#define OP_SH ( 41 )
#define OP_SWL ( 42 )
#define OP_SW ( 43 )
#define OP_SWR ( 46 )
#define OP_LWC0 ( 48 )
#define OP_LWC1 ( 49 )
#define OP_LWC2 ( 50 )
#define OP_LWC3 ( 51 )
#define OP_SWC0 ( 56 )
#define OP_SWC1 ( 57 )
#define OP_SWC2 ( 58 )
#define OP_SWC3 ( 59 )

/* OP_SPECIAL */
#define FUNCT_SLL ( 0 )
#define FUNCT_SRL ( 2 )
#define FUNCT_SRA ( 3 )
#define FUNCT_SLLV ( 4 )
#define FUNCT_SRLV ( 6 )
#define FUNCT_SRAV ( 7 )
#define FUNCT_JR ( 8 )
#define FUNCT_JALR ( 9 )
#define FUNCT_SYSCALL ( 12 )
#define FUNCT_BREAK ( 13 )
#define FUNCT_MFHI ( 16 )
#define FUNCT_MTHI ( 17 )
#define FUNCT_MFLO ( 18 )
#define FUNCT_MTLO ( 19 )
#define FUNCT_MULT ( 24 )
#define FUNCT_MULTU ( 25 )
#define FUNCT_DIV ( 26 )
#define FUNCT_DIVU ( 27 )
#define FUNCT_ADD ( 32 )
#define FUNCT_ADDU ( 33 )
#define FUNCT_SUB ( 34 )
#define FUNCT_SUBU ( 35 )
#define FUNCT_AND ( 36 )
#define FUNCT_OR ( 37 )
#define FUNCT_XOR ( 38 )
#define FUNCT_NOR ( 39 )
#define FUNCT_SLT ( 42 )
#define FUNCT_SLTU ( 43 )

/* OP_REGIMM */
#define RT_BLTZ ( 0 )
#define RT_BGEZ ( 1 )
#define RT_BLTZAL ( 16 )
#define RT_BGEZAL ( 17 )

/* OP_COP0/OP_COP1/OP_COP2 */
#define RS_MFC ( 0 )
#define RS_CFC ( 2 )
#define RS_MTC ( 4 )
#define RS_CTC ( 6 )
#define RS_BC ( 8 )
#define RS_BC_ALT ( 12 )

/* BC_BC */
#define BC_BCF ( 0 )
#define BC_BCT ( 1 )

/* OP_COP0 */
#define CF_TLBR ( 1 )
#define CF_TLBWI ( 2 )
#define CF_TLBWR ( 6 )
#define CF_TLBP ( 8 )
#define CF_RFE ( 16 )

typedef struct _DasmPSXCPU_state DasmPSXCPU_state;

struct _DasmPSXCPU_state
{
	UINT32 pc;
	int delayr;
	UINT32 delayv;
	UINT32 r[ 32 ];
};

extern unsigned DasmPSXCPU( DasmPSXCPU_state *state, char *buffer, UINT32 pc, const UINT8 *opram );

CPU_GET_INFO( psxcpu );
#define CPU_PSXCPU CPU_GET_INFO_NAME( psxcpu )

CPU_GET_INFO( cxd8661r );
#define CPU_CXD8661R CPU_GET_INFO_NAME( cxd8661r )

#endif /* __PSX_H__ */
