#pragma once

#ifndef __SSP1601_H__
#define __SSP1601_H__


enum
{
	/* general registers */
	SSP_R0,   SSP_X,     SSP_Y,    SSP_A,
	SSP_ST,   SSP_STACK, SSP_PC,   SSP_P,
	/* hardware stack */
	SSP_STACK0, SSP_STACK1, SSP_STACK2, SSP_STACK3, SSP_STACK4, SSP_STACK5,
	/* pointer registers */
	SSP_PR0, SSP_PR1, SSP_PR2, SSP_PR3, SSP_PR4, SSP_PR5, SSP_PR6, SSP_PR7
};

DECLARE_LEGACY_CPU_DEVICE(SSP1601, ssp1601);

extern unsigned dasm_ssp1601(char *buffer, unsigned pc, const UINT8 *oprom);

#endif /* __SSP1601_H__ */
