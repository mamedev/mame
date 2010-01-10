#pragma once

#ifndef __V810_H__
#define __V810_H__


CPU_GET_INFO( v810 );
#define CPU_V810 CPU_GET_INFO_NAME( v810 )

CPU_DISASSEMBLE( v810 );

enum
{
	V810_R0=1,
	V810_R1,
	V810_R2,  /* R2 - handler stack pointer */
	V810_SP,  /* R3 - stack pointer */
	V810_R4,  /* R4 - global pointer */
	V810_R5,  /* R5 - text pointer */
	V810_R6,
	V810_R7,
	V810_R8,
	V810_R9,
	V810_R10,
	V810_R11,
	V810_R12,
	V810_R13,
	V810_R14,
	V810_R15,
	V810_R16,
	V810_R17,
	V810_R18,
	V810_R19,
	V810_R20,
	V810_R21,
	V810_R22,
	V810_R23,
	V810_R24,
	V810_R25,
	V810_R26,
	V810_R27,
	V810_R28,
	V810_R29,
	V810_R30,
	V810_R31, /* R31 - link pointer */

	/* System Registers */
	V810_EIPC, /* Exception/interrupt  saving - PC */
	V810_EIPSW,/* Exception/interrupt  saving - PSW */
	V810_FEPC, /* Duplexed exception/NMI  saving - PC */
	V810_FEPSW,/* Duplexed exception/NMI  saving - PSW */
	V810_ECR,  /* Exception cause register */
	V810_PSW,  /* Program status word */
	V810_PIR,  /* Processor ID register */
	V810_TKCW, /* Task control word */
	V810_res08,
	V810_res09,
	V810_res10,
	V810_res11,
	V810_res12,
	V810_res13,
	V810_res14,
	V810_res15,
	V810_res16,
	V810_res17,
	V810_res18,
	V810_res19,
	V810_res20,
	V810_res21,
	V810_res22,
	V810_res23,
	V810_CHCW,  /* Cache control word */
	V810_ADTRE, /* Address trap register */
	V810_res26,
	V810_res27,
	V810_res28,
	V810_res29,
	V810_res30,
	V810_res31,

	V810_PC
};

#endif /* __V810_H__ */
