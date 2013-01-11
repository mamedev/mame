/* register names for apexc_get_reg & apexc_set_reg */
#pragma once

#ifndef __APEXC_H__
#define __APEXC_H__

enum
{
	APEXC_CR =1,    /* control register */
	APEXC_A,        /* acumulator */
	APEXC_R,        /* register */
	APEXC_ML,       /* memory location */
	APEXC_WS,       /* working store */
	APEXC_STATE,    /* whether CPU is running */

	APEXC_ML_FULL,  /* read-only pseudo-register for exclusive use by the control panel code
                    in the apexc driver : enables it to get the complete address computed
                    from the contents of ML and WS */
	APEXC_PC    /* doesn't actually exist; is there for the disassembler */
};

DECLARE_LEGACY_CPU_DEVICE(APEXC, apexc);

CPU_DISASSEMBLE( apexc );

#endif /* __APEXC_H__ */
