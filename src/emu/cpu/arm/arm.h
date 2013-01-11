#pragma once

#ifndef __ARM_H__
#define __ARM_H__

/****************************************************************************************************
 *  INTERRUPT CONSTANTS
 ***************************************************************************************************/

#define ARM_IRQ_LINE    0
#define ARM_FIRQ_LINE   1

/****************************************************************************************************
 *  PUBLIC FUNCTIONS
 ***************************************************************************************************/

DECLARE_LEGACY_CPU_DEVICE(ARM, arm);
DECLARE_LEGACY_CPU_DEVICE(ARM_BE, arm_be);

enum
{
	ARM32_PC=0,
	ARM32_R0, ARM32_R1, ARM32_R2, ARM32_R3, ARM32_R4, ARM32_R5, ARM32_R6, ARM32_R7,
	ARM32_R8, ARM32_R9, ARM32_R10, ARM32_R11, ARM32_R12, ARM32_R13, ARM32_R14, ARM32_R15,
	ARM32_FR8, ARM32_FR9, ARM32_FR10, ARM32_FR11, ARM32_FR12, ARM32_FR13, ARM32_FR14,
	ARM32_IR13, ARM32_IR14, ARM32_SR13, ARM32_SR14
};

#endif /* __ARM_H__ */
